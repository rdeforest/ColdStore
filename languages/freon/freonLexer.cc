// freonLexer.cc: Convert text into tokens for yyparse().
// Copyright (C) 2000 Ryan Daum, portions adapted from coldmud's token.c by Greg Hudson
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms


static char *id __attribute__((unused))="$Id: freonLexer.cc,v 1.18 2001/08/26 17:35:15 opiate Exp $";

#include <ctype.h>

#include <Data.hh>
#include <String.hh>
#include <Tuple.hh>
#include <Real.hh>
#include <Integer.hh>

#include "freonLexer.hh"

// Macros for measuring the number of words
// and subscripting one of them, respectively
#define NUM_RESERVED_WORDS (sizeof(reserved_words) / sizeof(*reserved_words))
#define SUBSCRIPT(c) ((c) & 0x7f)

// all the reserved words.
_words freonLexer::reserved_words[] = { /*fold00*/
  { "any",			ANY },
  { "arg",			ARG },
  { "break",			BREAK },
  { "case",			CASE },
  { "catch",			CATCH },
  { "continue",		        CONTINUE },
  { "default",		        DEFAULT },
  { "else",			ELSE },
  { "for",			FOR },
  { "fork",			FORK },
  { "if",			IF },
  { "in",			IN },
  { "lock",			LOCK },
  { "pass",			PASS },
  { "return",		        RETURN },
  { "switch",		        SWITCH },
  { "this",			THIS },
  { "to",			TO },
  { "try",                      TRY }, 
  { "var",			VAR },
  { "while",			WHILE },
  { "with",			WITH },
  { "(|",			CRITLEFT },
  { "(>",			PROPLEFT },
  { "<)",			PROPRIGHT },
  { "<=",			LE },
  { "<",			LT },
  { ".",                        DOT },
  { "..",			UPTO },
  { "||",			OR },
  { "|",			PIPE },
  { "|)",			CRITRIGHT },
  { "#[",			START_DICT },
  { "`[",			START_BUFFER },
  { "&&",			AND },
  { "==",			EQ },
  { "=",			ASSIGN },
  { "!=",			NE },
  { ">=",			GE },
  { ">",			GT },
  { ";",                        SEMICOLON },
  { "(",                        OP },
  { ")",                        CP },
  { "[",                        OB },
  { "]",                        CB },
  { "+",                        PLUS },
  { "++",			INCREMENT},
  { "-",                        MINUS },
  { "--",                       DECREMENT },
  { "*",                        MULTIPLY },
  { "/",                        DIVIDE },
  { ",",                        COMMA },
  { ":",			COLON },
  { "{",                        OBLOCK },
  { "}",                        CBLOCK },
  { "%",			MOD },
  { "?",			QUESTION },
};

/** initialize the reserved words table
 */
freonLexer::freonLexer( const Slot &program ) /*fold00*/
{
    unsigned int i, c;

    for (i = 0; i < 128; i++)
	starting[i].start = -1;
    
    i = 0;
    while (i < NUM_RESERVED_WORDS) {
      c = SUBSCRIPT(*reserved_words[i].word);
      starting[c].start = i;
      starting[c].num = 1;
      for (i++; i < NUM_RESERVED_WORDS && *reserved_words[i].word == c; i++)
	starting[c].num++;
    }
    in = new Tape<String>( program );
}

Slot cts( char c ) { /*fold00*/
  char str[2];
  str[0] = c;
  str[1] = 0;
  return Slot(str);
}


Slot freonLexer::make_token( int type, const Slot &value ) { /*fold00*/
  Slot token = new Tuple(3);
  return token->replace( 0, type )->replace( 1, value )->replace( 2, cur_line );
}


void freonLexer::error( const Slot &str, const Slot &value ) { /*fold00*/
  char line[10];
  sprintf(line, "%d", cur_line);
  Slot msg = str->concat(" (line: ")->concat(line)->concat(")");
  throw new Error( "lex", value, msg );
}

Slot freonLexer::readHex (unsigned n) { /*fold00*/
  char result = 0;
  while (n--) {
    char c = ((char*)in->Next())[0];
    result <<= 4;
    if (c >= '0' && c <= '9')
      result += c - '0';
    else if (c >= 'a' && c <= 'f')
      result +=c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      result += c - 'A' + 10;
    else
      error ("invalid hex escape sequence", in->Peek());
  }
  return cts(result);
}
 /*fold00*/
/** retrieve a character from the tape
 */
Slot freonLexer::readChar () { /*fold00*/
  Slot c = in->Next();

  if (!c.equal("\\"))
    return c;

  c = in->Next();
  // FIXME: support stuff like \ESC \NIL ...

  if (c.equal("b"))
    return "\b";
  if (c.equal("t"))
    return "\t";
  if (c.equal("n"))
    return "\n";
  if (c.equal("f"))
    return "\f";
  if (c.equal("r"))
    return "\r";
  if (c.equal("e"))
    return "\e";
  if (c.equal("0"))
    return "\e"; // FIXME: Octals
  if (c.equal("x"))
    return readHex(2);

    //    case 'u': FIXME: Later have unicode escapes?

  return c;
}
 /*fold00*/
/** retrieve a string from the tape
 */
Slot freonLexer::readString ( const Slot &start ) { /*fold00*/
  if (!start.equal("\""))
    return (Data*)NULL;

  Slot result = new String();
  for (Slot c = in->Peek(); (!c.equal("\"")); c = in->Peek()) {
    if (!in->More() || c.equal("\n"))
      error ("unterminated string constant", result);
    result = result->concat(readChar());
  }
  in->Next();
  return make_token( STRING, result );
}

void freonLexer::clearWhiteSpace() { /*fold00*/
  Slot c = in->Peek();
  while (c->equal(" ") || c->equal("\n")) {
    if (!in->More()) return;
    if (c->equal("\n"))
      cur_line++;

    in->Next();
    c = in->Peek();
  }
}

Slot freonLexer::readReserved( const Slot &begin ) { /*fold00*/

  // convert to char, compare first character
  char *s = (char*)begin;
  int start = starting[SUBSCRIPT(*s)].start;
  
  // first character match, compare rest of word
  if (start != -1) {
    char *word;
    char nxt;
    for (int index = start; index < start + starting[SUBSCRIPT(*s)].num; index++) {
      word = reserved_words[index].word;

      // Mark position in stream
      in->Mark();
      int counter ;
      int length = strlen(word);

      // Scan until no match or end of word
      for (counter = 1; counter < length && in->More(); counter++) {
	nxt = ((char*)in->Next())[0];
	if (nxt != word[counter])
	  break;
      }
      
      // we didn't scan the whole world.  no match
      if (word[counter]) {
	in->Reset();
	continue;
      }

      // get our marker off the stack
      in->Pop();

      return make_token( reserved_words[index].token, reserved_words[index].word);
    }
  }
  return (Data*)NULL;
}
  
Slot freonLexer::readIdentifier( const Slot &start ) { /*fold00*/
  Slot token = (Data*)NULL;
  Slot identifier = start;

  if (isalpha( ((char*)start)[0]) || (start.equal("_"))) {
    while (1) {
      Slot nxt = in->Peek();
      char s = ((char*)nxt)[0];
      if ( isalnum(s) || nxt.equal("_") ) {
	identifier = identifier->concat( nxt );
	in->Next();
      } else {
	break;
      }
    }

    token = make_token( IDENT, identifier );
  }
  return token;
}

Slot freonLexer::readSymbol( const Slot &start ) { /*fold00*/
  Slot token = (Data*)NULL;
  if (start.equal("$") || start.equal("'") || start.equal("~")) {
    token = new Tuple(2);

    Slot next = in->Next();
    if (next.equal("\""))
      token = readString( next );
    else
      token = readIdentifier( next );
    
    if (start.equal("$"))
      token = token->replace(0, NAME);
    else if (start.equal("'"))
      token = token->replace(0, SYMBOL);
    else if (start.equal("~"))
      token = token->replace(0, ERROR);   
  }
  return token;
}

Slot freonLexer::readComment( const Slot &start ) { /*fold00*/
  Slot token = (Data*)NULL;
  if (start.equal("/") && in->Peek().equal("/")) {
    in->Next();
    Slot result = new String();
    while (in->More() && (!in->Peek().equal("\n")))
      result = result->concat( in->Next() );

    token = make_token( COMMENT, result );
  }
  return token;
}



Slot freonLexer::readNumber ( const Slot &start ) { /*fold00*/
  Slot token = (Data*)NULL;

  if (start.equal("-") || isdigit( ((char*)start)[0] )) {
    Slot number = start;
    bool decimal = false;

    Slot c = in->Peek();

    // if it's signed, the character immediately after must be a number,
    // otherwise the - is something else
    if (start.equal("-") && !(isdigit(((char*)c)[0])))
      return (Data*)NULL;

    if (start.equal("0") && c.equal("x")) {
      // read a hex number sequence instead
      error("hex numbers unimplemented in lexer", start->concat(c));
    }
      
    while ((c.equal(".") || isdigit( ((char*)c)[0] )) && in->More() ) {
      if (c.equal(".")) {

	// only one . allowed
	if (decimal)
	  break;

	decimal = true;
      }
      number = number->concat( in->Next() );
      c = in->Peek();
    }

    Slot value;
    if (decimal)
      value = new Real( strtod( (char*)number, NULL ) );
    else
      value = new Integer( strtol( (char*)number, NULL, 10 ) );

    token = make_token( NUMBER, value );
  }
  return token;
}
 /*fold00*/
// the meat of the monster, big function that needs to be refactored into
// many smaller ones
Slot freonLexer::Next() /*fold00*/
{
  Slot r_c;

  clearWhiteSpace();

  // read next character
  r_c = in->Next();

  // READ COMMENT
  token = readComment( r_c);
  if (token)
    return token;

  // RESERVED WORD
  token = readReserved( r_c );
  if (token)
    return token;
  
  
  // READ IDENTIFIER
  token = readIdentifier( r_c );
  if (token)
    return token;


  // READ NUMBER
  token = readNumber( r_c );
  if (token)
    return token;

  // READ STRING
  token = readString( r_c );
  if (token)
    return token;

  // READ SYMBOL  'symbol, $object, ~error
  token = readSymbol( r_c );
  if (token)
    return token;


  // none of the above -- 
  error("unexpected character", r_c);
}

bool freonLexer::More () const { /*fold00*/
  return in->More();
}

Slot freonLexer::Peek() { /*fold00*/
  in->Mark();
  Slot value = Next();
  in->Reset();
  return value;
}

Slot freonLexer::Current() { /*fold00*/
  return token;
}

freonLexer::~freonLexer() { /*fold00*/
}
