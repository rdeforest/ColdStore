// Scanner - Scanner for smalltalk-like language
// based on source from "Brain"

#include "Data.hh"
#include "Tape.hh"
#include "String.hh"
#include "Scanner.hh"
#include "Tuple.hh"
#include "Error.hh"

#include <ctype.h>

template class Tape<String>;

char Scanner::inPeek() {
  try { 
    if (!in->More())
      return 0;

    return ((char*)(in->Peek()))[0];
  } catch (Error *e) {
    assert(0);
  }

}

char Scanner::inNext() {
  try {
    return ((char*)(in->Next()))[0];
  } catch (Error *e) {
    assert(0);
  }

}

Slot cts( char c ) {
  char str[2];
  str[0] = c;
  str[1] = 0;
  return Slot(str);
}

void Scanner::concat( char c ) {
  char str[2];
  str[0] = c;
  str[1] = 0;
  token = token->concat( Slot(str) );
}

void Scanner::concat( const Slot &c ) {
  token = token->concat( c );
}

Scanner::Scanner ( const Slot &inProgram ) {
  token   = new String();
  in      = new Tape<String>( inProgram );
  cached  = false;
  extra   = 0;
  type    = 0;
}

void Scanner::error ( const Slot &errmsg ) {
  throw new Error( "syntax", "", errmsg );
}


void Scanner::clearWhiteSpace() {
  Slot c = in->Peek();
  while (c->equal(" ")) {
    if (!in->More()) return;
    in->Next();
    c = in->Peek();
  }
}

void Scanner::clearComments () {
  for (;;) {
    clearWhiteSpace();
    if (in->Follows("#!"))
      while (in->More() && inNext() != '\n');
    else if (in->Follows("(*")) {
      while (!in->Follows("*)")) {
        if (!in->More())
          error("Unterminated comment");
        in->Next();
      }
      in->Next();
      in->Next();
    } else
      return;
  }
}

Slot Scanner::readHex (unsigned n) {
  char result = 0;
  while (n--) {
    char c = inNext();
    result <<= 4;
    if (c >= '0' && c <= '9')
      result += c - '0';
    else if (c >= 'a' && c <= 'f')
      result +=c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      result += c - 'A' + 10;
    else
      error ("Invalid hex escape sequence");
  }
  return cts(result);
}

Slot Scanner::readChar () {
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

void Scanner::readString () {
  type = STRING;
  for (;;) {
    if (in->Follows("\"\"")) {
      in->Next();
      in->Next();
      while (!in->Follows("\"\"\"")) {
        if (!in->More())
          error("Unterminated string constant");
        else if (in->Peek().equal("\n"))
          concat(in->Next());
        else
          concat(readChar());
      }
      in->Next();
      in->Next();
      in->Next();
    } else {
      for (char c = inPeek(); c != '"'; c = inPeek()) {
        if (!in->More() || c == '\n')
          error ("Unterminated string constant");
        concat(readChar());
      }
      in->Next();
    }
    clearComments();
    if (inPeek() == '"')
      in->Next();
    else
      return;
  }
}

void Scanner::readOperator () {
  char c = inPeek();
  while (strchr("+-/*^><=?@~!$%&\\", c)) {
    concat(in->Next());
    c = inPeek();
  }

  if (token.equal("="))
    type = SPECIAL;
  else
    type = OPERATOR;
}

void Scanner::readIdentifier () {
  for (;;) {
    char c = inPeek();
    if (isalnum(c) || c == '_' || c == '-' || c == ':') {
      in->Next();
      concat(c);
      if (c == ':') {
        type = KEYWORD;
        return;
      }
    } else
      return;
  }
}

void
Scanner::readNumber () {
  char c = inPeek();

  if (c == '-')
    concat(in->Next());

  c = inPeek();
  if (isdigit(c))
    readBaseNumber(10);

  c = inPeek();
  if (c == '.') {
    in->Mark();
    in->Next();
    c = inPeek();
    if (isdigit(c)) {
      concat('.');
      readBaseNumber(10);
    } else {
      in->Reset();
      return;
    }
  }
  c = inPeek();
  if (c == 'e' || c == 'E') {
    concat(in->Next());
    c = inPeek();
    if (c == '-' || c == '+')
      concat(in->Next());
    c = inPeek();
    if (isdigit(c)) {
      concat(in->Next());
      readBaseNumber(10);
    } else
      error("Invalid numerical constant");
  }
}


void Scanner::Read () {
  if (cached) {
    cached = false;
    return;
  }

  clearComments();
  token = "";

  if (!in->More()) {
    type = SPECIAL;
    return;
  }

  char c = inNext();
  // Parse identifiers/messages/keywords
  if (isalpha(c) || c == '_') {
    type = NAME;
    concat(c);
    readIdentifier();

    if (token.equal("self"))
      type = SPECIAL;
  }
  // Parse numbers
  else if (c == '.' || c == '-' || isdigit(c)) {
    unsigned base = 0;
    concat(c);
    type = NUMBER;

    if (c == '.') {
      c = inPeek();
      if (isdigit(c))
        readNumber();
      else {
        type = SPECIAL;
      }
      //readOperator();
      return;
    } else if (c == '-') {
      c = inPeek();
      if (c == '.') {
        concat(in->Next());
        c = inPeek();
        if (!isdigit(c))
          readOperator();
        else
          readNumber();
        return;
      }

      c = inPeek();
      if (!isdigit(c))
        readOperator();
    } else
      base = c - '0';

    for (int i = 0; i < 3; i++) {
      c = inPeek();
      if (isdigit(c)) {
        base *= 10;
        base += c - '0';
        concat(in->Next());
      } else if (c == '#') {
        if (base > 36 || base < 2)
          error("Number literal base must be between 2-36");
        concat(in->Next());
        if (!isalnum(inPeek()))
          error("Invalid base number literal");

        readBaseNumber(base);
        return;
      }
    }

    readNumber();
    return;
  }
  // Parse operators
  else if (strchr("+-/*^><=.?@~!$%&\\", c)) {
    concat(c);
    readOperator();
    return;
  }
  // Parse delimiters/specials
  else if (strchr("(){}[],;|", c)) {
    type = SPECIAL;
    concat(c);
    return;
  }
  // Parse strings
  else if (c == '"') {
    readString();
    return;
  }
  // Parse characters
  else if (c == '\'') {
    type = CHAR;

    if (inPeek() == '\'')
      error ("Invalid character constant");
    concat(readChar());
    if (inNext() != '\'')
      error ("Invalid character constant");
    return;
  }
  //
  else if (c == '#') {
    c = inPeek();
    // Parse specials
    if (c == '(' || c == '[' || c == '{') {
      in->Next();
      concat('#');
      concat(c);
      type = SPECIAL;
      return;
    } else
      error("Unexpected character found after `#'");
  } else if (c == ':') {
    c = inPeek();
    if (strchr("+-/*^><=?@~!$%&\\", c))
      readOperator();
    else if (isalpha(c) || c == '_') {
      while (isalpha(c) || c == '_') {
        readIdentifier();
        c = inPeek();
      }
    } else if (c == '"') {
      in->Next();
      readString();
    } else if (c == '(') {
      in->Next();
      token = token->concat("(");
      c = inNext();
      if (c == ')')
        token = token->concat(")");
      else
        error("Invalid symbol constant");
    } else
      error("Invalid symbol constant");
    type = SYMBOL;
    return;
  } else
    throw new Error("syntax", c, "unexpected character found");
}


Slot Scanner::Peek() {
  in->Mark();
  Slot value = Next();
  in->Reset();
  return value;
}

Slot Scanner::Current() {
  Slot res = new Tuple(2);
  res->replace(0, type);
  res->replace(1, token);
  return res;
  
}

Slot Scanner::Next() {
  Slot res = new Tuple(2);
  Read();
  res->replace(0, type);
  res->replace(1, token);
  return res;
}

bool Scanner::More () const {
  return in->More();
}
    
