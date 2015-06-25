// Copyright (C) 2000 Ryan Daum, Licensed under the GNU Public License
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
//
// $Id: InterpreterNodes.hh,v 1.1 2002/01/26 05:56:45 coldstore Exp $

#ifndef NODES_HH
#define NODES_HH

#include "Slot.hh"
#include "Node.hh"
//#include "Frame.hh"

class idListNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class commentNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class noopNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
  static const noopNode *noop;
};
 
class exprNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class compoundNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class blockNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class ifElseNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class forRangeNode
: public Node {
public:
      
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class forRangeCNode
: public Node {
public:
      
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class forListNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class forListCNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class whileNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class whileCNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class forkNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};


class lengthNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class switchNode
: public Node {
public:
      
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class breakNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class continueNode
: public Node {
public:
      
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class returnNode
  : public Node {
public:
  
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class tryCatchNode
: public Node {
public:
      
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class ifNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class caseNode
: public Node {
public:
      
  virtual void reduce(Slot &inexpr, Slot &context) ;
};


class passNode
: public Node {
public:
      
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class symbolNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class errorNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class nameNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class assignNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class instanceVariableNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class nextArgumentNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class remainingArgumentsNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class bufferNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class frobNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class indexNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class mulNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class powerNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class addNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class inNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class leNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class ltNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class geNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class gtNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class neNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class eqNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class subNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class divNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class modNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class andNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class orNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class conditionalNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class criticalNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class propagateNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class spliceNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class rangeNode
: public Node {
public:
    
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class lockNode
: public Node
{
public:
  
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class listNode
: public Node
{
public:
  
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class dictNode
: public Node
{
public:
  
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

class thisNode
: public Node
{
public:
  
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

/** a CFunctionNode is a Node with an extra color which is a pointer to
 *  a C function.  When reduced, it reduces to the return value of the
 *  function.
 */
cold class CFunctionNode
  : public Node
{
private:
  Slot (*function)( Slot &context );
 
public:
  /** constructor which takes a C function pointer as argument
   *  to allow easy construction
   */
  CFunctionNode( int lineNumber, Slot (*inFunction)( Slot &context ) );
 
  virtual void reduce(Slot &inexpr, Slot &context) ;
};

#endif
