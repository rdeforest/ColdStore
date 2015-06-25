// Binding - Binding object
// Copyright (C) 2000,2001 Colin McCormack, Ryan Daum
//
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// $Id

//#define DEBUGLOG
#include "Data.hh"
#include "Tuple.hh"
#include "List.hh"
#include "Dict.hh"
#include "Error.hh"
#include "Binding.hh"
#include "Closure.hh"

#include "Frame.hh"

Binding::Binding() 
  : Tuple(2),
    _variables(new List()),
    _map(new Dict())
{}

Binding::Binding( const Slot &binding ) 
  : Tuple(2),
    _variables(new List()),
    _map(new Dict())
{}

Binding::Binding( const Binding *copy )
  : Tuple(2),
    _variables( copy->_variables ),
    _map( new Dict() )
{
  // for some reason, it appears that mutable copy of dict
  // can't work... this is gross, but necessary for now
  Slot it = copy->_map->iterator();
  while (it->More()) {
    Slot o = it->Next();
    _map->insert( o[0], o[1] );
  }
}

ostream &Binding::dump(ostream& out) const
{
  out << "Binding<";
  out.form("[0x%08x]:", (Data*)_map);
  out << _map << ": " << _variables << ">" << endl;
  return out;
}

// Uses the copy constructor
Data *Binding::clone(void *where) const
{
    return new Binding(this);
}

// returns a mutable copy of this
Data *Binding::mutate(void *where) const
{
    return (Data*)this;
}

/////////////////////////////////////
// variable handling

Slot Binding::resolve(const Slot &var, Frame *context ) const
{
    Slot index = var[1];
    return _variables[index];
}

Slot Binding::set(const Slot &var, const Slot &value)
{
    Slot index = var[1];
    _variables = _variables->replace(index, value);
    return value;
}

// search returns the identNode, while slice returns the value
// python module depends on this
Slot Binding::search(const Slot &name) const
{
  int varID = _map->slice(name);   
  return new Variable( -1, name, varID );
}

Slot Binding::insert(const Slot &name) {
    int varID = _variables->length();
    _map->insert( name, varID );
    //    _variables = _variables->add( ((Data*)NULL) );
    Slot idNode = new Variable( -1, name, varID );
    return idNode;
}

Slot Binding::insert(const Slot &name, const Slot &value) {
    Slot node = insert ( name );
    replace( node, value );
    return node;
}

// there's no such thing as deleting a variable
// just let it pass out of scope, or set it to
// null
Slot Binding::del(const Slot &name) {
    replace( slice( name), ((Data*)NULL) );
    return this;
}

void Binding::pushLocals( const Slot &count ) {
  int amt = count;
  while (amt) {
    _variables->insert( (Data*)NULL );
    amt--;
  }
}

void Binding::popLocals( const Slot &count ) {
  int amt = count;
  Slot nul;
  while (amt) {
    ((Stack*)(Data*)_variables)->pop( nul );
    amt--;
  }
}

//////////////////////////////
// Variable implementation

void Variable::reduce( Slot &inexpr, Frame *context ) {
  Node *that = prepare_edit(inexpr);


  if (!context->_closure)
    throw new Error("illegal", this, "attempt to reduce variable without closure assigned to frame");
  
  // get the current binding from the current closure
  Binding *binding = ((Closure*)(Data*)context->_closure)->_binding;
  
  // resolve the value from the binding
  inexpr = binding->resolve( that, context );
}

Slot Variable::replace( const Slot &inValue, const Slot &context ) {
  Closure *closure = ((Closure*)(Data*) ((Frame*)(Data*)context)->_closure);

  // get the current binding from the current closure
  Binding *binding = closure->_binding;

  return binding->set( this, inValue );
}
