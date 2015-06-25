// Closure - Closure object
// Copyright (C) 2000,2001 Ryan Daum, Colin McCormack
//
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// $Id

//#define DEBUGLOG
#include "Data.hh"
#include "Closure.hh"
#include "InterpreterNodes.hh"
#include "Error.hh"
#include "Node.hh"

Closure::Closure( const Closure *from ) 
  : Tuple(4),
    _definer( from->_definer ),
    _tree( Slot(from->_tree) ),
    _binding( from->_binding ),
    _locals( from->_locals ),
    _source( from->_source )
{}

Closure::Closure( const Slot &definer ) 
  : Tuple(4),
    _definer( definer ),
    _tree( noopNode::noop ),
    _binding( new Binding() ),
    _locals( 0 ),
    _source( (Data*)NULL )
{}


Closure::Closure( const Slot &definer, const Slot &tree, const Slot source ) 
  : Tuple(4),
    _definer( definer ),
    _tree( tree ),
    _binding( new Binding() ),
    _locals( 0 ),
    _source( source )
{}


Closure::Closure( const Slot &definer, Slot (*function)(Frame *context ) )
  : Tuple(4),
    _definer( definer ),
    _tree( new CFunctionNode( 0, function ) ),
    _binding( (Binding*)NULL ),
    _locals( 0 ),
    _source( (Data*)NULL )
{}

Closure::Closure( Slot (*function)(Frame *context ) )
  : Tuple(4),
    _definer( (Data*)NULL ),
    _tree( new CFunctionNode(0, function) ),
    _binding( (Binding*)NULL ),
    _locals( 0 ),
    _source( (Data*)NULL )
{}

/** prepare() called to prepare the closure for execution
 * 
 *  get pointer to the binding from the parent (calling)
 *  closure, then push empty slots onto it for our_locals
 *  trust that the compiler generated the indexes into
 *  the binding correctly.
 */    
void Closure::prepare( Frame *context ) {
  Frame *caller = ((Frame*)(Data*)context->_caller);
  if (caller)
    cerr << " ... " << endl;

  if (caller && caller->_closure) {
    Closure *parent = ((Closure*)(Data*)caller->_closure);
    cerr << "inheriting from " << parent->_binding << endl;
    _binding = parent->_binding;
    _binding->pushLocals(_locals );
  } else {
    _binding = new Binding();
    _binding->pushLocals(_locals);
  }
}

/** finish() called to clean up closure after execution
 *
 *  pop the_locals from the binding and then free up our
 *  local reference to it.
 */
void Closure::finish( Frame *context ) {
  _binding->popLocals(_locals );
  _binding = (Binding*)NULL;
}

// receive message
Slot Closure::receive( Message *msg, Frame *context )
{
  // do reduction but trap returnNode to return values.  called from Object, usually
  if ( msg->_selector.equal("invoke") ) {
    // copy over binding from parent
    prepare( context );
    try {
      Slot copy(_tree);
      Node::Reduce( copy, context );
      finish( context );
      return copy;
    } catch (returnNode *ret) {
      DEBLOG(cerr << "Returned: " << ret << '\n');
      finish( context );
      return ret->slice(0);
      ret->dncount();
    }

  } 

  // does reduce, but allows the returnNode and errors to propagate up
  // called when closure is a kind of block thingy
  if ( msg->_selector.equal("reduce") ) {
    prepare( context );
    Slot copy(_tree);
    Node::Reduce( copy, context );
    finish( context );
    return copy;
  }    

  return this->Data::receive( msg, context );
}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Closure::clone(void *where) const
{
    //  return new (_TupleBase<Slot>::Length(), where) Closure(*this);
    return new Closure(this);
}

// returns a mutable copy of this
Data *Closure::mutate(void *where) const
{
    return (Data*)this;
}

ostream &Closure::dump(ostream& out) const
{
   
  out << "<" << typeId() << " {";
  out.form("0x%08x", (Data*)this );
  out << ">: ( definer: ";
  if (_definer)
     out.form("{0x%08x}", (Data*)_definer );
   else
    out << "[NULL]";

  out << " binding: "<< _binding;
  out << " locals: " << _locals;
  out << " tree: " << _tree;
  out << " source: " << _source;
  out << " )" ;
  return out;
}
