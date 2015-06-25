// freonParser for smalltalk-like language

// -*-C++-*-
#ifndef __PARSER_HH
#define __PARSER_HH

#include "Data.hh"
#include "freonLexer.hh"

class freonParser
{
private:
  freonLexer *lexer;
  Slot endTokenS;
  
private:
  // (a, b, c)
  Slot parseArgumentList( const Slot &closure, const Slot &start ); 

  // o.m(arglist)
  Slot parseMessage( const Slot &closure, const Slot &object, const Slot &start );

  // for var in (pexpr) statement
  virtual Slot parseFor( const Slot &closure, const Slot &start );

  // if (pexpr) statement [else statement]
  virtual Slot parseIf( const Slot &closure, const Slot &start );
 
  // while (expr) statement
  virtual Slot parseWhile( const Slot &closure, const Slot &start );

  // try statement catch statement
  virtual Slot parseTry( const Slot &closure, const Slot &start );

  // #[ ... ]
  virtual Slot parseDict( const Slot &closure, const Slot &start );

  // { ... }
  virtual Slot parseBlock( const Slot &closure, const Slot &start );

  // [ ... ]
  virtual Slot parseList( const Slot &closure, const Slot &start );  

  // ( . )
  virtual Slot parseParentheticExpression( const Slot &closure, const Slot &start );

  // atom
  virtual Slot parseAtom( const Slot &closure, const Slot &start );

  // unary:  i.e.  a++
  virtual Slot parseUnaryExpression( const Slot &closure, 
				     const Slot &lhs, 
				     const Slot &op );

  // binary: i.e.  a + b
  virtual Slot parseBinaryExpression( const Slot &closure, 
				     const Slot &lhs, 
				     const Slot &op );

  // conditional: i.e.  a ? b | c
  virtual Slot parseConditionalExpression( const Slot &closure, 
				     const Slot &lhs, 
				     const Slot &op );

  // right hand side of expression
  virtual Slot parseRExpression( const Slot &closure, const Slot &lhs,
				 const Slot &start );

  // expression
  virtual Slot parseExpression( const Slot &closure, const Slot &start );

  // statement
  virtual Slot parseStatement( const Slot &closure, const Slot &start );

  // statement
  virtual Slot parseStatements( const Slot &closure );

  void checkForEnd();
  Slot getEndToken();

public:

  /** parse - parse the method and return a closure object
   *  @param name name of the method
   *  @param definer definer of the method
   *  @param program string containing the program
   *  @returns a closure object
   */
  Slot parse( const Slot &definer, const Slot &program );
  
};

#endif /* PARSER_HH */
