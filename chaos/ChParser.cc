// Chaos parser
// Copyright (C) 2002 Colin McCormack, Robert de Forest
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

//#define DEBUGLOG 1
#include "semantics.hh"
#include <Real.hh>
#include <BigInt.hh>
#include <Builtin.hh>
#include <errno.h>

pcre *Chaos::ws;
pcre *Chaos::number;
pcre *Chaos::token;

void Chaos::init_parse()
{
    int options = PCRE_EXTRA;
    const char *errmsg;
    int erroff;

    ws = pcre_compile("^(\\s|\\0|(/[*]([*][^/]|[^*])*[*]/))+", options, &errmsg, &erroff, NULL);
    if (!ws) {
	cerr << errmsg << ' ' << erroff << '\n';
    }
    number = pcre_compile("^-?\\d+([.](\\d+([Ee]?\\d+)?)?)?$", options, &errmsg, &erroff, NULL);
    if (!number) {
	cerr << errmsg << ' ' << erroff << '\n';
    }
    token = pcre_compile("^((?:\\\\.|[^[\\](){}\\s])+)", options, &errmsg, &erroff, NULL);
    if (!token) {
	cerr << errmsg << ' ' << erroff << '\n';
    }
    
    // set up primitives
    for (int idx = 0; primitives[idx].name; idx++) {
	tSlot<Builtin> bi(new Builtin(primitives[idx].builtin_name, "chaos.so"));
	DEBLOG(cerr << "Make Prim: " << primitives[idx].name
	       << ' ' << primitives[idx].name << '\n');
	store->Names->insert(primitives[idx].name, bi);
    }   
}

void Chaos::ChParse()
{
  Slot s = ChPop();
  DEBLOG(cerr << "ChParse: " << (char*)s << " --> ");
  //ChPushStack();
  s = parse(s);
  //ChPopStack();
  DEBLOG(cerr << (char*)s << '\n');
  ChPush(s);
}

Slot Chaos::parse(Slot s)
{
  const size_t mveclen=54;
  int matches[mveclen+1];
  int match;

  while (s->length()) {
    // remove leading whitespace and comments
    if (0 < (match = pcre_exec(ws, NULL, s, s->length(), 0, 0, matches, mveclen))) {
      s = s->del(0, matches[1]);
      if (!s->length()) {
        return s;
      }
    }
    // parse out tokens
    char prefix = ((char*)s)[0];
    if (prefix == '/' && ((char*)s)[1] == '*') {
      // Incomplete comment, wait for rest
      return s;
    }
    switch (prefix) {
      case '"': {
        // we're starting a double-quote string
        char *dquote_str = (char*)malloc(s->length());
        if (! dquote_str) {
          cerr << "DQUOTE ERR: malloc failed" << '\n';
          throw new Error("parse", s, "Error mallocing Dquote buffer");
        }
        memcpy((void *)dquote_str, (void*)(char*)s, s->length());

        int i = 1;  // Place in string we are looking at
        int j = 0;  // Place in string we are overwriting

        // de-quote the string
        for (; i < s->length(); i++) {
            char copychar = dquote_str[i];
            if (copychar == '\\') {
                i++;
                dquote_str[j] = dquote_str[i];
                j++;
            } else if (copychar == '"') {
                ChPush(new String(dquote_str, j));
                s = s->del(0, i + 1);               // i + 1 because i is a base 0 index into s
                i = s->length();                    // exit for loop
                j = 0;                              // string end found indicator
            } else {
                dquote_str[j] = copychar;
                j++;
            }
        }
        free(dquote_str);
        if (j) { return s; }
      } break;
      
      case '[': { ChVar("["); s=s->del(0); break; } 
      case ']': { ChVar("]"); s=s->del(0); break; } 
      case '(': { ChVar("("); s=s->del(0); break; } 
      case ')': { ChVar(")"); s=s->del(0); break; } 

      case '}': {
        return s;
      } break;
      
      case '{': {
        s = s->del(0);
        tSlot<Chaos> c = new Chaos(s);
        s = c->parse(s);
        if (s->length() && ((char*)s)[0] == '}') {
          // got matching close brace
          ChPush(c);
          s=s->del(0);
        } else {
          // got an open brace without a matching close, can do no more.
          return s->insert(0, "{");
        }
      } break;

      default: {
        // Find out where the token ends
        match = pcre_exec(token, NULL,
                          s, s->length(), 0, 0,
                          matches, mveclen);

        if (match < 2) {
          // Whatever's left of this string doesn't match anything we
          // can recognize.  This should never happen.  We are
          // looking for anything other than {}[]() and whitespace,
          // and all those should have been seen above.  Throw an
          // exception.
          
          throw new Error("parse", s, "Impossible parse situation: no words left in non-null string.");
        }

        int token_end = matches[1];
        char *token_str = (char*)malloc(token_end);
        if (! token_str) {
          cerr << "PARSE ERR: token_str malloc failed" << '\n';
          throw new Error("parse", s, "Error mallocing token_str buffer");
        }
        memcpy((void*)token_str, (void *) (char *)s, token_end);
        token_str[token_end] = '\0';

                if (prefix == '\'') {
          ChPush(token_str + 1);
        } else if (prefix == '$') {
          ChPush(token_str + 1);
          ChVar("$");
        } else if ((match = pcre_exec(number, NULL, s, token_end, 0, 0, matches, mveclen)) > 1) {
          // match > 1 means ([.]...) sub expression matched
          ChPush(new Real(token_str));
        } else if (match > 0) {
          // match > 0 means just -\d+ matched
          long n = strtol(token_str, NULL, 0);
          if (errno == ERANGE) {
            ChPush(new BigInt(token_str));
          } else {
            ChPush(n);
          }
        } else {
          // It must be a word (function/procedure/whatever)
          ChPush(token_str);
          ChVar("$");
          ChVar("exec");
        }
        s = s->del(0, token_end);
        free(token_str);
      } // end default case
    } // end switch
  } // while s->length()
  return s;
}
