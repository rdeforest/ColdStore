// Copyright (C) 2000 Ryan Daum, licensed under the GNU Public License see
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

// $Id: InterpreterNodes.cc,v 1.1 2002/01/26 05:56:45 coldstore Exp $

static char *id __attribute__((unused))="$Id: InterpreterNodes.cc,v 1.1 2002/01/26 05:56:45 coldstore Exp $";

#include "Data.hh"
#include "Dict.hh"
#include "List.hh"
#include "Store.hh"
#include "Tuple.hh"
#include "Symbol.hh"
#include "Error.hh"
//#include "Frob.hh"
//#include "Closure.hh"
#include "InterpreterNodes.hh"

void idListNode::reduce(Slot &inexpr, Slot &context)
{
  //  Node *that = prepare_edit(inexpr);
  //  DEBLOG(cerr << "IDLIST " << that << "\n");
  inexpr = noopNode::noop;
}


void commentNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void nextArgumentNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "NEXT ARGUMENT " << that << "\n");

  //STUB:  inexpr = context->nextArgument();
}

void remainingArgumentsNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "REMAINING ARGUMENTS " << that << "\n");
  //STUB: inexpr = context->getArguments();
}

void exprNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  DEBLOG(cerr << "EXPR " << that << "\n");
  that->inplace(context, 0);
  DEBLOG(cerr << "EXPR=>" << that << "\n");
  inexpr = that->slice(0);
}

void compoundNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void blockNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  inexpr = that;
  DEBLOG(cerr << "STMT " << that << "\n");

  while (that && that->length()) {
    Slot &e = (*that)[0];	// get the next element
    DEBLOG(cerr << "STMT PART " << e << "\n");

    Reduce(e, context);

    that = dynamic_cast<Node*>((Data*)(that->del(0)));
  }
  inexpr = noopNode::noop;
}

void ifElseNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  DEBLOG(cerr << "IFELSE " << that << "\n");
  that->inplace(context, 0);
  if (that->slice(0)->truth()) {
    DEBLOG(cerr << "IF TRUE " << that << "\n");
    inexpr = that->slice(1);
  } else {
    DEBLOG(cerr << "IF FALSE " << that << "\n");
    inexpr = that->slice(2);
  }
}

void ifNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  DEBLOG(cerr << "IF " << that << "\n");
  that->inplace(context, 0);
  if (that->slice(0)->truth()) {
    DEBLOG(cerr << "IF TRUE " << that << "\n");
    inexpr = that->slice(1);
  } else {
    DEBLOG(cerr << "IF FALSE " << that << "\n");
    inexpr = noopNode::noop;
  }
}

void whileNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "WHILE " << this << "\n");
  inexpr = new whileCNode(_lineNumber, slice(0), slice(1), slice(0), slice(1));
}

void whileCNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "WHILEC " << this << "\n");

  // execute pending continuations
  inplace(context, 0);
  if (slice(0)->truth()) {
    try {
      inplace(context, 1); // reduce existing continuation in-place
    } catch (breakNode *br) {
      // the loop is broken
      inexpr = noopNode::noop;
      return;
    } catch (continueNode *co) {
      // do nothing
    }
  } else {
    // the loop is exhausted
    inexpr = noopNode::noop;
    return;
  }

  // set up continuations for next time around
  replace(0, slice(2));
  replace(1, slice(3));
}

void forListNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "FOR " << this << "\n");
  // first time through - reduce the list argument
  inplace(context, 1);	 // reduce list in-place
  inexpr = new forListCNode(_lineNumber, slice(0), slice(1)->toSequence(), slice(2), noopNode::noop);
}

void forListCNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "FORC " << this << '\n');
  Slot var = slice(0);
  Slot stmt = slice(2);

  try {
    inplace(context, 3); // reduce existing continuation in-place
    replace(3, noopNode::noop);
  } catch (breakNode *br) {
    // the loop is broken
    inexpr = noopNode::noop;
    return;
  } catch (continueNode *co) {
    replace(3, noopNode::noop);
  }

  if (slice(1) && slice(1)->length()) {
    var->replace( slice(1)->slice(0), context ); // set the variable
    replace(1, slice(1)->del(0));	// remove list head
    replace(3, stmt);		// place continuation
  } else {
    inexpr = noopNode::noop;
  }
}
void forkNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "FORK " << this << "\n");
  inexpr = noopNode::noop;
}

void forRangeNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "FOR RANGE" << this << "\n");
  // first time through - reduce the list argument
  inplace(context, 1);	 // reduce start in-place 
  slice(0)->replace(slice(1), context );
  inplace(context, 2);	 // reduce upto in-place

  inexpr = new forRangeCNode(_lineNumber, slice(0), slice(2), slice(3), 0);
}

void forRangeCNode::reduce(Slot &inexpr, Slot &context)
{
  DEBLOG(cerr << "FOR RANGE C " << this << '\n');
  Slot var = slice(0);
  Slot stmt = slice(2);

  try {
    inplace(context, 3); // reduce existing continuation in-place
    replace(3, noopNode::noop);
  } catch (breakNode *br) {
    // the loop is broken
    inexpr = noopNode::noop;
    return;
  } catch (continueNode *co) {
    replace(3, noopNode::noop);
  }
  int counter = (int)(  ((Symbol*)(Data*)var)->value()) + 1;
  if (counter <= (int)(slice(1))) {
    var->replace( counter );
    replace(3, stmt);		// place continuation
  } else {
    inexpr = noopNode::noop;
  }
}


void switchNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void breakNode::reduce(Slot &inexpr, Slot &context)
{
  cerr << "BREAK " << inexpr << "\n";
  throw this;
}

void continueNode::reduce(Slot &inexpr, Slot &context)
{
  cerr << "CONTINUE " << inexpr << "\n";
  throw this;
}

void returnNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  DEBLOG(cerr << "RETURN " << that << "\n");
  Slot x(that->slice(0));
  Reduce( x, context );
  // that->inplace(context, 0);
  DEBLOG(cerr << "RETURN=> " << that << "\n");
  returnNode *retval = new returnNode(_lineNumber, x );
  retval->upcount();	// necessary to upcount this for catcher
  throw retval;
}

// arguments: blockNode, errorType, errorVarName, branch (blockNode)
void tryCatchNode::reduce(Slot &inexpr, Slot &context)
{
  Slot tryBranch = slice(0);
  Slot errorType = slice(1);
  Slot errorIdent = slice(2);
  Slot catchBranch = slice(3);

  try {
    Reduce(tryBranch, context);

  } catch ( Frame *context) {
    Error *ret = context->_error;
    if ( ret->slice(0) != errorType ) {
      throw;
    } else {
      Node::Reduce( catchBranch, context );
      return;

    }
  }
  inexpr = noopNode::noop;

}

void caseNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}


void passNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}


void lengthNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  inexpr = that->slice(0)->length();
}

void symbolNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void errorNode::reduce(Slot &inexpr, Slot &context)
{
  if ( length() == 1)
    inexpr = new Error( slice(0), _lineNumber, "" );
  else if ( length() == 2)
    inexpr = new Error( slice(0), slice(1), "" );
  else
    inexpr = new Error( slice(0), slice(1), slice(2) );
}

void instanceVariableNode::reduce( Slot &inexpr, Frame *context )
{
  Node *that = prepare_edit(inexpr);

  // an instance variable node is the Symbol returned from the nameNode
  // lookup.  its value is inside
  inexpr = that->slice(0)[2];
}

void nameNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);

  Slot name = that->slice(0);

  // try instance variables, first 
  try {
    Slot self = context->_this;
    Slot definer = ((Closure*)(Data*)context->_closure)->_definer;
    Slot result = self->slice( new IVKey( name, definer ) );
    inexpr = new instanceVariableNode( _lineNumber, result );
  } catch (Error *e) {
    Symbol *w = store->Names->slice( name );
    inexpr = w->value();
  }

}

void assignNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);

  Slot ident = that->slice(0);
  Slot value = that->inplace(context, 1);

  if (!ident) 
    throw new Error("illegal", this, "assignment to non-existent identifier");

  if ( AKO((Data*)ident, Variable )) {
    DEBLOG(cerr << "ASSIGN: " << ident << "=" << value << "\n");
    ident = ident->replace(value, context );
    inexpr = ident;
    return;
  }

  // not a variable?  reduce the ident once only
  ident->reduce( ident, context );

  // if it's an instance variable node, do the replace on the symbol therein
  if ( AKO((Data*)ident, instanceVariableNode) ) {
    Slot node = ident->slice(0);
    node->replace(value);
    inexpr = value;
    return;
  }

  // assignment can only happen to Variables and Instance Variables

  throw new Error("illegal", this, "assignment to non-variable");
}


void bufferNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void frobNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot obj = that->inplace(context, 0);
  Slot repr = that->inplace(context, 1);
  inexpr = new Frob(obj, repr);
}

void indexNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot expr = that->inplace( context, 0 );
  Slot indx = that->inplace( context, 1 );
  inexpr = expr->slice(indx);
}

void conditionalNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot expr = that->inplace( context, 0 );
  if (expr->truth())
    inexpr = that->inplace( context, 1 );
  else
    inexpr = that->inplace( context, 2 );
}

void criticalNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void propagateNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void spliceNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = noopNode::noop;
}

void rangeNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  Slot range = new List();
  for (;op0<op1;op0->add(1))
  {
    range->insert(op0);
  }
  inexpr = range;
}

void mulNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0->multiply(op1);
}

void divNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0->divide(op1);
}

void modNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0->modulo(op1);
}

void addNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0->add(op1);
}

void powerNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0 ^ op1;
}

void subNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0->subtract(op1);
}

void eqNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0 == op1;
}

void neNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0 != op1;
}

void gtNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0 > op1;
}

void geNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0 >= op1;
}

void ltNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  DEBLOG(cerr << "LT " << that << "\n");
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);
  inexpr = op0 < op1;
}

void leNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0 <= op1;
}

void andNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0->and(op1);
}

void orNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);

  inexpr = op0->or(op1);
}

void inNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  
  Slot op0 = that->inplace(context, 0);
  Slot op1 = that->inplace(context, 1);
  inexpr = op1->search(op0);
}

// unfinished.  stub for now.
void noopNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = (Data*)NULL;
}

// unfinished.  stub for now.
void lockNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = (Data*)NULL;
}

void thisNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = context->_this;
}

void listNode::reduce(Slot &inexpr, Slot &context)
{
  Slot that = prepare_edit( inexpr );
  Slot out = new List();
  while (that->More()) {
    Slot x = that->Next();
    Reduce( x, context );
    out->insert( x );
  }
  inexpr = out;
}

void dictNode::reduce(Slot &inexpr, Slot &context)
{
  Node *that = prepare_edit(inexpr);
  Slot list = that->inplace(context, 0);

  Slot it = list->iterator();
  Slot outList = new List();
  while (it->More())
    {
      Slot el = it->Next();
      Reduce( el, context );
      outList = outList->insert( el );
    }
  inexpr = new Dict(outList);

}

CFunctionNode::CFunctionNode( int lineNumber, Slot (*inFunction)( Frame *context ) )
{
  _lineNumber = lineNumber;
  function = inFunction;
}
 
void CFunctionNode::reduce(Slot &inexpr, Slot &context)
{
  inexpr = function( context );
}
