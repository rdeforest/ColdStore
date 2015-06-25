// freonParser -- handwritten recursive descent parser for a c-like language
// freon language based on "c--" or "coldC" written by Greg Hudson for coldmud server (1991? 1992?
// copyright ryan daum, august 2001

#include <Data.hh>
#include <Store.hh>
#include <Object.hh>
#include <Message.hh>
#include <Error.hh>
#include <Closure.hh>
#include <List.hh>

#include <InterpreterNodes.hh>

#include "freonParser.hh"
#include "freonLexer.hh"

Slot freonParser::parseList( const Slot &closure, const Slot &start ) {
  Slot result = new listNode( start[2] );
  Slot token = lexer->Peek();
  while (lexer->More() && (!token[0].equal( freonLexer::CB))) {
    result->insert( parseAtom( closure , lexer->Next()));
    
    if (!lexer->More())
      break;

    token = lexer->Peek();    
    if (token[0].equal( freonLexer::CB ))
      break;
    else if (!token[0].equal( freonLexer::COMMA ))
      throw new Error("parse", token, "comma missing in list");

    lexer->Next();
    token = lexer->Peek();
    if (token[0].equal( freonLexer::CB))
      throw new Error("parse", token, "extra trailing comma in list");
  }
  if (!token[0].equal( freonLexer::CB ))
    throw new Error("parse", token, "unclosed list");

  lexer->Next();

  return result;
}

Slot freonParser::parseDict( const Slot &closure, const Slot &start ) {
  Slot result = new dictNode( start[2] );
  Slot token = lexer->Peek();
  while (lexer->More() && (!token[0].equal( freonLexer::CB))) {
    Slot key = parseAtom( closure , lexer->Next());

    result->insert(key);

    if (!lexer->More())
      throw new Error("parse", token, "truncated dict");

    token = lexer->Peek();

    if (!token[0].equal( freonLexer::COLON ))
      throw new Error("parse", token, "malformed pair in dict");

    lexer->Next();

    Slot value = parseAtom( closure, lexer->Next());

    result->insert( value );

    if (!lexer->More())
      break;

    token = lexer->Peek();    
    if (token[0].equal( freonLexer::CB ))
      break;
    else if (!token[0].equal( freonLexer::COMMA ))
      throw new Error("parse", token, "comma missing in dict");

    lexer->Next();
    token = lexer->Peek();
    if (token[0].equal( freonLexer::CB))
      throw new Error("parse", token, "extra trailing comma in dict");
  }
  if (!token[0].equal( freonLexer::CB ))
    throw new Error("parse", token, "unclosed dict");

  lexer->Next();

  return result;
}

Slot freonParser::parseBlock( const Slot &closure, const Slot &start ) {
  // create a child closure -- 

  // at compilation, the closure inherits the index bindings of
  // its parent closure -- this is to insure that variables declared
  // in the parent closure can be used in the child.  at actual
  // invocation time, the value storage is copied over to update
  // the values from the parent closure

  Slot sub_closure = new Closure( (Data*)NULL );
  ((Closure*)(Data*)sub_closure)->_binding = ((Closure*)(Data*)closure)->_binding;
  Slot statements = new blockNode();
  Slot token;
  while ( lexer->More() ) {
    token = lexer->Next();

    if (token[0].equal( freonLexer::CBLOCK)) {
      break;
    }
    Slot statement = parseStatement( sub_closure, token );
    if (statement)
      statements->insert( statement );
  }
  
  ((Closure*)(Data*)sub_closure)->_tree = statements;

  return sub_closure;
}

Slot freonParser::parseAtom( const Slot &closure, const Slot &start ) {
  Slot token = start;
  Slot atom = (Data*)NULL;

  if (token[0].equal(freonLexer::NUMBER))
    atom = token[1];
 
  if (token[0].equal(freonLexer::STRING))
    atom = token[1];

  if (token[0].equal(freonLexer::NAME))
    atom = new nameNode( token[2], token[1] );
  
  if (token[0].equal(freonLexer::START_DICT))
    atom = parseDict( closure, token );

  if (token[0].equal(freonLexer::THIS))
    atom =new thisNode( token[2] );

  if (token[0].equal(freonLexer::IDENT)) {
    Binding *binding = ((Closure*)(Data*)closure)->_binding;
    Slot var = binding->search( token->slice(1) );
    atom = var;
  }

  if (token[0].equal(freonLexer::OB))
    atom = parseList( closure, token );

  if (token[0].equal(freonLexer::SYMBOL))
    atom = new symbolNode( token[2], token[1] );

  if (token[0].equal(freonLexer::ERROR))
    atom = new errorNode( token[2], token[1] );

  if (token[0].equal(freonLexer::OP)) {
    atom = parseParentheticExpression( closure, token );
  }

  if (!atom)
    throw new Error("parse", token, "unknown atomic type");

  return atom;
}


Slot freonParser::parseMessage( const Slot &closure, const Slot &object, const Slot &start ) {

  Slot token = start;
  Slot selector;
  Slot arguments;

  if (token[0].equal( freonLexer::IDENT ))
    selector = token[1];
  else if (token[0].equal( freonLexer::OP )) 
    selector = parseParentheticExpression( closure, token );
  else
    throw new Error("parse", token, "invalid selector for message");

  checkForEnd();

  token = lexer->Next();

  if (!token[0].equal( freonLexer::OP ))
    throw new Error("parse", token, "arguments required for message invoke");

  checkForEnd();

  arguments = parseArgumentList( closure, token );

  return new Message( (Data*)NULL, object, selector, arguments );
}

Slot freonParser::parseArgumentList( const Slot &closure, const Slot &start ) {
  Slot result = new listNode( start[2] );
  Slot token = lexer->Peek();
  while (lexer->More() && (!token[0].equal( freonLexer::CP))) {
    result->insert( parseAtom( closure , lexer->Next()));
    
    if (!lexer->More())
      break;

    token = lexer->Peek();    
    if (token[0].equal( freonLexer::CP ))
      break;
    else if (!token[0].equal( freonLexer::COMMA ))
      throw new Error("parse", token, "comma missing in argument list");

    lexer->Next();
    token = lexer->Peek();
    if (token[0].equal( freonLexer::CP))
      throw new Error("parse", token, "extra trailing comma in argument list");
  }
  if (!token[0].equal( freonLexer::CP ))
    throw new Error("parse", token, "unclosed argument list");

  lexer->Next();

  return result;
}

Slot freonParser::parseParentheticExpression( const Slot &closure, const Slot &start ) {
  checkForEnd();

  // PUSH ")" AS END TOKEN
  ((Stack*)(Data*)endTokenS)->push( freonLexer::CP );
  Slot expr = parseExpression( closure, lexer->Next() );

  checkForEnd();

  Slot next = lexer->Next();

  if (!next[0].equal( freonLexer::CP ))
    throw new Error( "parse", next, "unclosed parenthetic expression");

  // POP ")" AS END TOKEN
  Slot x;
  ((Stack*)(Data*)endTokenS)->pop( x );
  if (!x.equal( freonLexer::CP ))
    throw new Error( "parse", lexer->Peek(), "internal end token stack error!  report bug to maintainer!");


  return expr;
}

Slot freonParser::getEndToken() {
  Slot endToken;
  int length = endTokenS->length();
  if (length)
    endToken = endTokenS->slice( length - 1 );
  else
    endToken = freonLexer::SEMICOLON;

  return endToken;
}

Slot freonParser::parseExpression( const Slot &closure,
				   const Slot &start ) {
  Slot token = start;
  Slot lhs = parseAtom( closure, token );

  // determine what kind of token ends the expression
  Slot endToken = getEndToken();
  // parse until no more to parse because of ending token ( ")" or ";" usually)
  for (token = lexer->Peek();!token[0].equal( endToken );token=lexer->Peek()) {
    checkForEnd();
    lhs = parseRExpression( closure, lhs, lexer->Next() );
  }

  return lhs;
}

Slot freonParser::parseRExpression( const Slot &closure,
				    const Slot &atom, const Slot &start ) {

  Slot token = start;
  Slot lhs = atom;
 
  // attempt unary
  Slot result = parseUnaryExpression( closure, lhs, token );
  if (result)
    return result;
    
  // not unary, try binary
  result = parseBinaryExpression( closure, lhs, token );
  if (result)
    return result;
    
  // not binary, try conditional
  result = parseConditionalExpression( closure, lhs, token );
  if (result)
    return result;

  // none of the above, probably an error
  throw new Error("parse", lexer->Current(), "syntax error");
}

Slot freonParser::parseUnaryExpression( const Slot &closure, const Slot &lhs,  const Slot &op ) {
  // unary increment, decrement need nodes
  if (op[0].equal( freonLexer::INCREMENT ) || op[0].equal( freonLexer::DECREMENT ))
    throw new Error("parse", op, "increment and decrement unary operators not implemented");

  return (Data*)NULL;
}

Slot freonParser::parseBinaryExpression( const Slot &closure,
					 const Slot &lhs,
					 const Slot &op ) {
  checkForEnd();

  if (op[0].equal( freonLexer::ASSIGN ))
    return new assignNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::EQ ))
    return new eqNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::LE ))
    return new leNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::GE ))
    return new geNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::NE ))
    return new neNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::LT ))
    return new ltNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::GT ))
    return new gtNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::AND ))
    return new andNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::OR ))
    return new orNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::MULTIPLY ))
    return new mulNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::DIVIDE ))
    return new divNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::MOD ))
    return new modNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::PLUS ))
    return new addNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::MINUS ))
    return new subNode( op[2], lhs, parseExpression( closure, lexer->Next() ) );

  if (op[0].equal( freonLexer::DOT )) 
    return parseMessage( closure, lhs, lexer->Next() );

  return (Data*)NULL;
}

Slot freonParser::parseConditionalExpression( const Slot &closure, const Slot &lhs, 
					      const Slot &op ) {
  checkForEnd();

  if (op[0].equal( freonLexer::QUESTION )) {

    checkForEnd();

    Slot cond1 = parseExpression( closure, lexer->Next() );
    
    checkForEnd();

    Slot next = lexer->Peek();

    if (!next[0].equal( freonLexer::PIPE ))
      throw new Error("parse", next, "syntax error in conditional");

    lexer->Next();

    checkForEnd();
   
    Slot cond2 = parseExpression( closure, lexer->Next() );

    return new conditionalNode( op[2], lhs, cond1, cond2 );
  }
  return (Data*)NULL;

}

void freonParser::checkForEnd() {
  if (!lexer->More()) 
    throw new Error("parse", lexer->Current(), "unexpected end of source");

  Slot next = lexer->Peek();
  if (next[0].equal( freonLexer::SEMICOLON ))
    throw new Error("parse", next, "syntax error");
}

Slot freonParser::parseIf( const Slot &closure, const Slot &start ) {
  checkForEnd();

  Slot next = lexer->Next();
  if (!next[0].equal( freonLexer::OP ))
    throw new Error("parse", next, "invalid conditional for if statement");

  Slot cond = parseParentheticExpression( closure, next );

  checkForEnd();

  Slot branch = parseStatement( closure, lexer->Next() );

  // has an else branch
  if (lexer->More() && lexer->Peek()[0].equal( freonLexer::ELSE )) {
    lexer->Next();
    return new ifElseNode( start[2], cond, branch, parseStatement( closure, lexer->Next() ));
  }
				          
  return new ifNode( start[2], cond, branch );
}

Slot freonParser::parseWhile( const Slot &closure, const Slot &start ) {
  checkForEnd();

  Slot next = lexer->Next();
  if (!next[0].equal( freonLexer::OP ))
    throw new Error("parse", next, "invalid conditional for while statement");

  Slot cond = parseParentheticExpression( closure, next );

  checkForEnd();

  Slot branch = parseStatement( closure, lexer->Next() );

			          
  return new whileNode( start[2], cond, branch );
}

Slot freonParser::parseTry( const Slot &closure, const Slot &start ) {
  checkForEnd();

  Slot branch = parseStatement( closure, lexer->Next() );

  checkForEnd();

  Slot next = lexer->Peek();

  if (!next[0].equal( freonLexer::CATCH ))
    throw new Error("parse", next, "missing catch branch for try statement");

  lexer->Next();

  return new tryCatchNode( start[2], branch, parseStatement( closure, lexer->Next() ));
}

Slot freonParser::parseFor( const Slot &closure, const Slot &start ) {
  checkForEnd();

  Slot var = parseAtom( closure, lexer->Next() );

  checkForEnd();

  Slot next = lexer->Peek();

  if (!next[0].equal( freonLexer::IN ))
    throw new Error("parse", next, "missing in statement in for statement");

  lexer->Next();

  next = lexer->Peek();
  Slot lst;
  if (next[0].equal( freonLexer::OP )) {
    lst = parseParentheticExpression( closure, lexer->Next() );
  } else if (next[0].equal( freonLexer::OB ))
    lst = parseList( closure, lexer->Next() );
  else
    throw new Error("parse", next, "range argument must be list or parenthetic expression");

  checkForEnd();

  return new forListNode( start[2], var, lst, parseStatement( closure, lexer->Next() ));
}



Slot freonParser::parseStatement( const Slot &closure, const Slot &start ) {
  Slot stmt = (Data*)NULL;

  if (start[0].equal( freonLexer::RETURN)) {
    stmt = new returnNode( start[2], parseExpression( closure, lexer->Next() ));
    Slot semi = lexer->Next();
    if (!semi[0].equal(freonLexer::SEMICOLON))
      throw new Error("parse", semi, "missing semicolon at end of return");
  } else if (start[0].equal( freonLexer::VAR)) {
    Binding *binding = ((Closure*)(Data*)closure)->_binding;

    Slot token = lexer->Next();

    if (!token[0].equal( freonLexer::IDENT ))
      throw new Error("parse", token, "invalid variable name in variable declaration");

    binding->insert( token[1] );
    ((Closure*)(Data*)closure)->_locals = ((Closure*)(Data*)closure)->_locals + Slot(1);

    stmt = noopNode::noop; 

    Slot semi = lexer->Next();
    if (!semi[0].equal(freonLexer::SEMICOLON))
      throw new Error("parse", semi, "missing semicolon at end of var declaration");

  } else if (start[0].equal( freonLexer::ARG)) {
    Binding *binding = ((Closure*)(Data*)closure)->_binding;

    Slot token = lexer->Next();

    if (!token[0].equal( freonLexer::IDENT ))
      throw new Error("parse", token, "invalid variable name in argument declaration");

    stmt = new assignNode( 0, 
			   binding->insert( token[1] ),
			   new nextArgumentNode( token[2] ) );

    Slot semi = lexer->Next();
    if (!semi[0].equal(freonLexer::SEMICOLON))
      throw new Error("parse", semi, "missing semicolon at end of arg declaration");
  } else if (start[0].equal( freonLexer::BREAK)) {
    stmt = new breakNode( start[2] );
    Slot semi = lexer->Next();
    if (!semi[0].equal(freonLexer::SEMICOLON))
      throw new Error("parse", semi, "missing semicolon at end of break");
  } else if (start[0].equal( freonLexer::CONTINUE)) {
    stmt = new continueNode( start[2] );
    Slot semi = lexer->Next();
    if (!semi[0].equal(freonLexer::SEMICOLON))
      throw new Error("parse", semi, "missing semicolon at end of continue");
  } else if (start[0].equal( freonLexer::COMMENT))
    stmt = new commentNode( start[2], start[1] );
  else if (start[0].equal(freonLexer::OBLOCK))
    stmt = parseBlock( closure, start );
  else if (start[0].equal( freonLexer::IF))
    stmt = parseIf( closure, start );
  else if (start[0].equal( freonLexer::WHILE))
    stmt = parseWhile( closure, start );
  else if (start[0].equal( freonLexer::TRY))
    stmt = parseTry( closure, start );
//   else if (start[0].equal( freonLexer::switch))
//     return parseSwitch( closure, start );
  else if (start[0].equal( freonLexer::FOR))
    stmt = parseFor( closure, start );
  else {
    stmt = parseExpression( closure, start );
    Slot semi = lexer->Next();
    if (!semi[0].equal(freonLexer::SEMICOLON))
      throw new Error("parse", semi, "missing semicolon at end of expression");
  }
  return stmt;
}

Slot freonParser::parseStatements( const Slot &closure ) {
  Slot statements = new blockNode();
  Slot token;
  while (lexer->More() ) {
    Slot statement = parseStatement( closure, lexer->Next() );
    if (statements)
      statements->insert( statement );
  }
  ((Closure*)(Data*)closure)->_tree = statements;
  return closure;
}

Slot freonParser::parse(  const Slot &definer, const Slot &program ) {
  lexer = new freonLexer( program );

  endTokenS = new List();
  return parseStatements( new Closure( definer ) );
}


