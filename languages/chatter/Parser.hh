// Parser for smalltalk-like language

// -*-C++-*-
#ifndef __PARSER_HH
#define __PARSER_HH

#include "Data.hh"
#include "Scanner.hh"

class Parser
{
public:
  Scanner *scanner;

private:
  Slot parseList( const Slot &closure );
  Slot parseBlock( const Slot &closure );
  Slot parseStatements( const Slot &closure );
  Slot parseVariablesDecl( const Slot &closure, const Slot &token_left );
  Slot parseParentheses( const Slot &closure, const Slot &left );
  Slot parseAtom( const Slot &closure, const Slot &token );
  Slot parseObject( const Slot &closure, const Slot &token );
  Slot parseBinarySelector( const Slot &closure, const Slot &object, const Slot &name );
  Slot parseKeywordSelector( const Slot &closure, const Slot &object, const Slot &name );
  Slot parseMessage( const Slot &closure, const Slot &object, const Slot &token );
  Slot parseExpression( const Slot &closure, const Slot &left );
  Slot parseStatement( const Slot &closure, const Slot &left );

  bool expressionEnd( const Slot &token );

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
