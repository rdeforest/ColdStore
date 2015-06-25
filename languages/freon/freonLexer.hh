// freonNodes - Declarations for lexer class
// Copyright (C) 2000 Ryan Daum, based on token.h from coldmud by Greg Hudson
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// 

#ifndef LEXER_HH
#define LEXER_HH

#include <stdio.h>

#include <Data.hh>
#include <Slot.hh>
#include <Hash.hh>
#include <Error.hh>
#include <Frame.hh>
#include <Closure.hh>
#include <Tape.hh>

static struct {
    int start;
    int num;
} starting[128];

typedef struct {
  char *word;
  int token;
} _words;


class freonLexer
{
private:
  Tape<String> *in;
  static _words reserved_words[];
  int cur_line;
  Slot token;

public:

  // tokens for types and reserved words
  enum {STRING = 0, IDENT, NAME, NUMBER, COMMENT,
        ERROR, SYMBOL, ANY, ARG, BREAK, CASE, CATCH, 
	CONTINUE, DEFAULT, ELSE,
	FOR, FORK, IF, IN, LOCK, PASS, RETURN, SWITCH, THIS, TO,
	TRY, VAR, WHILE, WITH, CRITLEFT, PROPLEFT, PROPRIGHT,
	LE, UPTO, CRITRIGHT, OR, START_DICT, START_BUFFER, AND,
	EQ, NE, GE, SEMICOLON, OP, CP, OB, CB, PLUS, MINUS, DOT,
        MULTIPLY, DIVIDE, COMMA, COLON, OBLOCK, CBLOCK, MOD,
	ASSIGN, INCREMENT, DECREMENT, LT, GT, QUESTION, PIPE };
  
  freonLexer( const Slot &program );
  virtual ~freonLexer();

  void clearWhiteSpace();

private:
  Slot readHex (unsigned n);
  Slot readChar();
  Slot readString ( const Slot &start );
  Slot readReserved( const Slot &start );
  Slot readIdentifier( const Slot &start );
  Slot readSymbol( const Slot &start );
  Slot readComment( const Slot &start );
  Slot readNumber ( const Slot &start );

public:
  Slot Next();
  bool More() const;
  Slot Peek();
  Slot Current();

  virtual Slot make_token( int type, const Slot &value );
  virtual void error( const Slot &str, const Slot &value );

};

#endif

 
