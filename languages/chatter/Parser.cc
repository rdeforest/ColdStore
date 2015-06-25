// Parser for smalltalk-like language

#include <Data.hh>
#include <Store.hh>
#include <InterpreterNodes.hh>

#include "Scanner.hh"
#include "Parser.hh"
#include "Object.hh"
#include "Message.hh"
#include "Error.hh"
#include "Closure.hh"


Slot Parser::parseBlock( const Slot &closure ) {
  // create a child closure -- 

  // at compilation, the closure inherits the index bindings of
  // its parent closure -- this is to insure that variables declared
  // in the parent closure can be used in the child.  at actual
  // invocation time, the value storage is copied over to update
  // the values from the parent closure

  Slot sub_closure = new Closure( (Data*)NULL );
  Binding *binding = ((Closure*)(Data*)closure)->_binding;  
  Binding *c_binding = new Binding(binding);
  ((Closure*)(Data*)sub_closure)->_binding = c_binding;
  Slot statements = new blockNode();
  Slot token;
  while ( scanner->More() ) {
    token = scanner->Next();

    if (token->slice(0).equal(Scanner::SPECIAL) && token->slice(1).equal("}")) {
      break;
    }
    statements->insert( parseStatement( sub_closure, token ) );
  }

  ((Closure*)(Data*)sub_closure)->setTree( statements );

  return sub_closure;
}

Slot Parser::parseList( const Slot &closure ) {
  Slot list = new listNode();
  while (scanner->More() ) {
    Slot token = scanner->Next();
    Slot result = parseStatement( closure, token );
    if (result)
      list->insert(result);
    Slot next = scanner->Current();
    if (next->slice(0).equal(Scanner::SPECIAL) && next->slice(1).equal(")")) {
      break;
    }    
  }
  return list;
}

Slot Parser::parseAtom( const Slot &closure, const Slot &token ) {

  if ( token->slice(0).equal(Scanner::NUMBER) ) {
    // must convert to integer.  leave as string for now
    char *s = token->slice(1);
    return strtol( s, NULL, 10 );
    return token->slice(1);
  }

  if (token->slice(0).equal(Scanner::STRING) )
    return token->slice(1);

  
  // is it an object?
  if (token->slice(0).equal(Scanner::NAME) ) {
    try {
      Binding *binding = ((Binding*)(Data*) ((Closure*)(Data*)closure)->getBinding());      
      Slot var = binding->search( token->slice(1) );
      return var;
    } catch (Error *e) {
      return new nameNode( 0, token->slice(1) );
    }
  }

  // specials
  if ( token->slice(0).equal(Scanner::SPECIAL) ) {

    // block enclosure
    if (token->slice(1).equal("{"))
      return parseBlock( closure );

    if (token->slice(1).equal("#("))
      return parseList( closure );
    // self
    if ( token->slice(1).equal("self") )
      return new thisNode( 0 );
   
  }

  throw new Error("syntax", token, "unknown atomic");
}

Slot Parser::parseParentheses( const Slot &closure, const Slot &left ) {
  Slot token = scanner->Next();

  // parse what is inside
  Slot exp = parseExpression( closure, token );

  // better be closed parentheses at end
  Slot next = scanner->Current();
  if ( !next->slice(0).equal( Scanner::SPECIAL ) &&
       !next->slice(1).equal(")") )
    throw new Error("syntax", next, "unclosed parentheses");

  return exp;
}

Slot Parser::parseObject( const Slot &closure, const Slot &token ) {

  // is the object the result of an expression?
  if ( (token->slice(0).equal(Scanner::SPECIAL))
       && (token->slice(1).equal("(" )))
    return parseParentheses( closure, token );
  
  // if not, it's an atom
  return parseAtom( closure, token );
}

// query to check for termination of an expression
// possible ends are statement end ("."), 
//                   parenthetic statement end (")")
//                   block end ("}")
bool Parser::expressionEnd( const Slot &token ) {
  if ( token->slice(0).equal( Scanner::SPECIAL )
       && (token->slice(1).equal(".") || 
	   (token->slice(1).equal(")") ) ) )
    return true;
  else
    return false;
}

Slot Parser::parseBinarySelector( const Slot &closure, const Slot &object, const Slot &name ) {
  Slot arguments = new List();
  arguments->insert( parseObject( closure, scanner->Next() ) );


  // if there's more in the scanner, it'd better be '.'
  // otherwise it a message to the results of this message
  if (scanner->More()) {
    Slot next = scanner->Next();
    if ( !expressionEnd( next ) )
      return parseMessage( closure, new Message( (Data*)NULL, object, name, arguments), next );
  }

  return new Message( (Data*)NULL, object, name, arguments );
}

Slot Parser::parseKeywordSelector( const Slot &closure, const Slot &object, const Slot &name ) {
  Slot arguments = new List();
  Slot keyword = name->slice(0, -2);
  Slot s_name = "";

  while ( scanner->More() ) {
    s_name = s_name->concat( keyword );
    Slot n = scanner->Next();
    arguments->insert( parseObject( closure, n ) );
    if ( scanner->More() ) {
      Slot next = scanner->Next();
      // break from loop when we get to a end of statement
      if ( next->slice(0).equal( Scanner::SPECIAL )
	   && next->slice(1).equal(".") )
	break;
      
      if (!next->slice(0).equal(Scanner::KEYWORD))
	throw new Error("syntax", next->slice(1), "keyword argument required");

      keyword = next->slice(1)->slice(0,-2)->insert(0,"_");
    }
  }


  return new Message( (Data*)NULL, object, s_name, arguments );
}



Slot Parser::parseMessage( const Slot &closure, const Slot &object, const Slot &token ) {
  Slot next;

  // unary expression because impending end of
  // statement
  if ( expressionEnd( scanner->Peek() ) ) {
    scanner->Next();
    return new Message( (Data*)NULL, object, token->slice(1), new List() );
  }

  if ( token->slice(0).equal( Scanner::KEYWORD ) ) 
    return parseKeywordSelector( closure, object, token->slice(1) );
  
  if ( token->slice(0).equal( Scanner::SPECIAL) && token->slice(1).equal("=") )
    return new assignNode( 0, object, parseExpression( closure, scanner->Next() ) );

  if ( token->slice(0).equal( Scanner::NAME ) || token->slice(0).equal( Scanner::OPERATOR ) )
    return parseBinarySelector( closure, object, token->slice(1) );

  throw new Error( "syntax", token, "invalid selector name");
}

// variable declarations kinda _look_ like a list, but the 
// only thing you can stick
// in them is variables and arguments
Slot Parser::parseVariablesDecl( const Slot &closure, const Slot &left ) {
  Slot assign_list = new blockNode( 0 );
  Binding *binding = ((Binding*)(Data*) ((Closure*)(Data*)closure)->getBinding());

  Slot token;
  while ( scanner->More() ) {
    token = scanner->Next();

     // is it the end of the var list?
    if ( token->slice(0).equal( Scanner::SPECIAL ) &&
	 token->slice(1).equal( "|" ) )
      break;


    // is it a symbol? are we in var decl mode? then it's an argument
    if (token->slice(0).equal( Scanner::SYMBOL ) ) 
      assign_list->insert( new assignNode( 0, 
					   binding->insert( token->slice(1) ),
					   new nextArgumentNode(0) ) );
    
    // is it a name? then it's a variable
    else if (token->slice(0).equal( Scanner::NAME ) ) {
      assign_list->insert( noopNode::noop );
      binding->insert(token->slice(1));
    }
    // otherwise, it's invalid
    else
      throw new Error("syntax", token, "invalid variable name" );
  }
  // last token should have been a "|"
  if ( !(token->slice(0).equal( Scanner::SPECIAL ) &&
	 token->slice(1).equal( "|" ) ))
    throw new Error("syntax", token, "variable declaration never closed");
  
  return assign_list;
}

Slot Parser::parseExpression( const Slot &closure, const Slot &token ) {
  if ( token->slice(0).equal( Scanner::SPECIAL ) &&
       token->slice(1).equal( "|" ) )
    return parseVariablesDecl( closure, token );

  Slot object = parseObject( closure, token );

  // atomic
  Slot next;
  if ( !scanner->More() || expressionEnd( next = scanner->Next() ) )
    return object;
  
  return parseMessage( closure, object, next );
}

Slot Parser::parseStatement( const Slot &closure, const Slot &token ) {
 
  // return
  if ( token->slice(0).equal(Scanner::OPERATOR) &&
       token->slice(1).equal("^") ) {
    return new returnNode(0, parseExpression( closure, scanner->Next() ) );
  }

  // null statement
  if (expressionEnd( token ) )
    return noopNode::noop;

  return parseExpression( closure, token );
}

Slot Parser::parseStatements( const Slot &closure ) {
  Slot statements = new blockNode();
  Slot token;
  while (scanner->More() ) {
    statements->insert( parseStatement( closure, scanner->Next() ) );
  }
  ((Closure*)(Data*)closure)->setTree( statements );
  return closure;
}

Slot Parser::parse(  const Slot &definer, const Slot &program ) {
  scanner = new Scanner( program );
  return parseStatements( new Closure( definer ) );
}


