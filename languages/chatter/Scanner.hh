// Scanner - Scanner for smalltalk-like language
// based on source from "Brain"

// -*-C++-*-
#ifndef __SCANNER_HH
#define __SCANNER_HH

#include "Data.hh"
#include "Tape.hh"

class Scanner
{
private:

  Tape<String> *in;
  bool cached;       // return last token on read (used for peek)
  Slot token;
  unsigned extra;    // Extra data (ex regex flags)
  unsigned type;     // Type of current token

private:
  void clearComments ();
  void clearWhiteSpace();
  Slot readChar ();
  Slot readHex (unsigned);
  void readString ();
  void readOperator ();
  void readIdentifier ();
  void readNumber ();
  void readBaseNumber (unsigned);

  void error ( const Slot &errmsg );

  char inPeek();
  char inNext();

  void concat( const Slot &c );
  void concat( char c ) ;
  void Read();

public:

  virtual Slot Next();
  virtual bool More() const;
  virtual Slot Peek();
  virtual Slot Current();

  enum {STRING = 0, CHAR, SPECIAL, NAME, NUMBER,
        KEYWORD, OPERATOR, ARRAY, REGEX, SYMBOL};


public:

  Scanner ( const Slot &inProgram );

};


#endif /* __SCANNER_HH */
