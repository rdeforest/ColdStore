// freonNodes - Nodes used by freon for interpretation
// Copyright (C) 2000 Ryan Daum,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

// $Id: freonNodes.cc,v 1.1 2000/12/02 16:54:53 opiate Exp $  
// $Log: freonNodes.cc,v $
// Revision 1.1  2000/12/02 16:54:53  opiate
// Moved old stack-based system to "stack-experiment".
//
// Revision 1.31  2000/11/15 08:31:16  coldstore
// Replaced local version of openc++ with openc++ 2.5.10,
// debian for the updated version is here:
//  ftp://coldstore.sourceforge.net/pub/coldstore/debs/openc++_2.5.10-1_i386.deb
//
// Revision 1.30  2000/11/13 02:22:54  coldstore
// Freon-t: made stmt contain a List preparatory to making Node ako Tuple.
//
// Revision 1.29  2000/11/12 04:01:52  coldstore
// Node tree Reduction - works for simple while-test
//
// Revision 1.28  2000/11/09 17:38:53  opiate
// Changed Frame::prepareStack to be Node::toPostfix
// Changed Node::unfold to Node::isTerminal
// Changed whileNode::eval to actually work
// Changed interpretStack, ifNode, etc. to use toPostfix
//
// Revision 1.27  2000/11/09 05:56:36  opiate
// String::add copied from List::add
// Frame::prepareStack and addStack (nonreversing)
// broken freonNodes::whileNode
//
// Revision 1.26  2000/11/08 22:06:19  opiate
// Added CVS log and version entries.
// 

static char *id __attribute__((unused))="$Id: freonNodes.cc,v 1.1 2000/12/02 16:54:53 opiate Exp $";



#include <Data.hh>
#include <Dict.hh>
#include <List.hh>
#include <Tuple.hh>
#include <Symbol.hh>


#include "freonNodes.hh"

void initNodes()
{
    if (!noopNode::noop) {
       noopNode::noop = new noopNode;
       noopNode::noop->upcount();
    }
}

Frame *methodNode::eval( Frame *context )  {

  cout << "finished methodNode\n";
  return context;
}

Frame *overrideableNode::eval( Frame *context )  {

  Slot node;
  context->_valStack->pop(node);
  cout << "method overridable: " << node << "\n";
  return context;
}

Frame *varNode::eval( Frame *context ) { 

  while (context->_valStack->length()) {
    Slot varName;
    context->_valStack->pop(varName);
    context->_namespace->insert(varName, 0);
  }
  return context;
}

Frame *argDeclNode::eval( Frame *context ) {

  while (context->_valStack->length()) {
    Slot name, value;
    context->_valStack->pop(name);
    context->_arguments->pop(value);
    context->_namespace->insert(name, value);
  }
  return context;
}

Frame *idListNode::eval( Frame *context ) {

  return context;
}

Frame *commentNode::eval( Frame *context ) { 

 return context;
}

Frame *noopNode::eval( Frame *context ) {

  return context;
}

Frame *exprNode::eval( Frame *context ) { 
  Slot node;
  context->_valStack->pop(node);
  return context;
}

Frame *compoundNode::eval( Frame *context ) { 

  return context;
}

Frame *stmtNode::eval( Frame *context ) { 
  // if there's anything on the stack pop it off, statements
  // shouldn't leave anything behind
  while (context->_valStack->length()) {
    Slot node;
    context->_valStack->pop(node);
  }
  return context;
}

Frame *ifElseNode::eval( Frame *context ) { 
  return context;
}

Frame *forRangeNode::eval( Frame *context ) {
  return context;
}

Frame *forListNode::eval( Frame *context ) {
  return context;
}

Frame *whileNode::eval( Frame *context ) {
  Slot conditional, todo;
  Frame *doFrame;

  context->_valStack->pop(conditional);
  context->_valStack->pop(todo);
  if (conditional->truth()) {
    Slot cond;

    context->_opStack = new Stack(context->_opStack->concat( this->toPostfix() )) ;
    context->_opStack->push(todo);

    Slot topNode = todo->slice(0);
    doFrame = context->next(topNode);
    doFrame->_opStack = new Stack( ((Node*)(Data*)topNode)->toPostfix() );
    return doFrame;
  }
  return context;
}

Frame *switchNode::eval( Frame *context ) {
  return context;
}

Frame *breakNode::eval( Frame *context ) {
  return context->_caller;
}

Frame *continueNode::eval( Frame *context ) {
  return context;
}

Frame *returnNode::eval( Frame *context ) {
  Slot node;
  context->_valStack->pop(node);
  context->_status = TASK_COMPLETED;
  cout << "return value: " << node << "\n";
  return context;
}

Frame *catchNode::eval( Frame *context ) {
  return context;
}

Frame *ifNode::eval( Frame *context ) {
  Slot conditional, doNodes;

  context->_valStack->pop(conditional);
  context->_valStack->pop(doNodes);
  doNodes = doNodes->slice(0);

  if (conditional->truth()) {
    context = context->next( doNodes );
    context->_opStack = ((Stack*)(Data*)  ((Node*)(Data*)doNodes)->toPostfix());
  }
  return context;
}

Frame *frameNode::eval( Frame *context ) {
  context->_valStack->push(this);
  return context;
}

Frame *caseNode::eval( Frame *context ) {
  return context;
}

Frame *functionCallNode::eval( Frame *context ) {
  return context;
}

Frame *assignNode::eval( Frame *context ) {
  Slot ident, value, retval;
  context->_valStack->pop(value);
  context->_valStack->pop(ident);
  //cout << "assign: " << ident << "=" << value << "\n";  
  retval = context->_namespace->replace(ident, value);

  context->_valStack->push( ((Symbol*)(Data*)retval)->value());

  return context;
}

Frame *passNode::eval( Frame *context ) {
  return context;
}

Frame *messageNode::eval( Frame *context ) {
  return context;
}

Frame *exprMessageNode::eval( Frame *context ) {
  return context;
}

Frame *dbrefNode::eval( Frame *context ) {
  return context;
}

Frame *symbolNode::eval( Frame *context ) {
  return context;
}

Frame *errorNode::eval( Frame *context ) {
  return context;
}

Frame *nameNode::eval( Frame *context ) {
  return context;
}

Frame *identNode::eval( Frame *context ) {
  Slot ident;
  context->_valStack->pop(ident);
  Slot value = context->_namespace->search(ident);

  context->_valStack->push( ((Symbol*)(Data*)value)->value());
  return context;
}

Frame *listNode::eval( Frame *context ) {
  List *l = new List();
  while (context->_valStack->length()) {
    Slot node;
    context->_valStack->pop(node);
    l = l->push(node);
  }
  context->_valStack->push(l);
  return context;
}

Frame *listElementNode::eval( Frame *context ) {
  return context;
}

Frame *dictNode::eval( Frame *context ) {
  Slot d = new Dict();
  while (context->_valStack->length()) {
    Slot key, value;
    context->_valStack->pop(value);
    context->_valStack->pop(key);
    cout << "insert: " << key << " " << value << "\n";
    d = d->insert(key, value);
  }
  context->_valStack->push(d);
  return context;
}

Frame *dictElementNode::eval( Frame *context ) {
  return context;
}

Frame *bufferNode::eval( Frame *context ) {
  return context;
}

Frame *frobNode::eval( Frame *context ) {
  return context;
}

Frame *indexNode::eval( Frame *context ) {
  return context;
}

Frame *unaryNode::eval( Frame *context ) {
  return context;
}




Frame *conditionalNode::eval( Frame *context ) {
  return context;
}

Frame *criticalNode::eval( Frame *context ) {
  return context;
}

Frame *propagateNode::eval( Frame *context ) {
  return context;
}

Frame *spliceNode::eval( Frame *context ) {
  return context;
}

Frame *rangeNode::eval( Frame *context ) {
  return context;
}

Frame *mulNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 * a1 );
  return context;
}

Frame *divNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 / a1 );
  return context;
}

Frame *modNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 % a1 );
  return context;
}

Frame *addNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 + a1 );
  return context;
}

Frame *subNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 - a1 );
  return context;
}

Frame *eqNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 == a1 );
  return context;
}

Frame *neNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 != a1 );
  return context;
}

Frame *gtNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 > a1 );
  return context;
}

Frame *geNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 >= a1 );
  return context;
}

Frame *ltNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 < a1 );
  return context;
}

Frame *leNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 <= a1 );
  return context;
}

Frame *andNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 && a1 );
  return context;
}

Frame *orNode::eval( Frame *context ) {
  Slot a1, a2;

  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( a2 || a1 );
  return context;
}

// unfinished.  stub for now. 
Frame *inNode::eval( Frame *context ) {
  Slot a1, a2;
  context->_valStack->pop(a1);
  context->_valStack->pop(a2);
  context->_valStack->push( -1 );
  return context;
}


// unfinished.  stub for now. 
Frame *lockNode::eval( Frame *context ) {
  return context;
}
