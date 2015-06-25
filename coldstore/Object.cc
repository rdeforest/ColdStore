// Object - object scope
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
//      $Id 

#include "Data.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Symbol.hh"
#include "VectorSet.hh"
#include "List.hh"
//#include "Frame.hh"
//#include "Message.hh"
#include "Object.hh"

#include "tSlot.th"
template union tSlot<Object>;

Object::Object():
	Namespace(),
	parent_list(new List()),
	child_set(new List())
{}

Slot Object::toconstruct() const {
  return new Object();
}

Object::~Object() {}

Slot Object::slice( const Slot& name ) const {
  Slot result = search_graph( name,  new VectorSet() );
  if (result)
    return result;
  else {
    // unknown symbol - throw
    throw new Error("dictnf", name, "Symbol not found");
  }
}

Slot Object::search_graph( const Slot& name, const Slot& context ) const {

  // Search for self in context
  // If we're there, end.
  Slot iret = context->search(this);
  if (iret)
    return (Data*)NULL;

  // Insert self into context
  context->insert( (Data*)this );

  // Search for the symbol locally
  Slot retval = search( name );

  // Found it...
  if (retval) 
    return retval;

  // Check parents...
  Slot iterator = parent_list->iterator();
  while (iterator->More()) {
    Object* parent = (Object*)(Data*)iterator->Next();
    retval = parent->search_graph(name, context);
    if (retval) 
      return retval;
  }

  // Unfound, returns NULL
  return retval;

}

Slot Object::createChild() const {
  Slot child = new Object();
  ((Object*)(Data*)child)->addParent( this );
  return child;
}

Slot Object::addParent(const Slot& parent) {
  Data* pobj = (Data*)parent;
  if (AKO(pobj,Namespace)) {
    parent_list = parent_list->insert(parent);
    if (dynamic_cast<Object*>(pobj)) {
      ((Object*)pobj)->addChild((Data*)this);
    }
  }
  return (Data*)this;
}

Slot Object::addChild(const Slot& child) {
  child_set->insert(child);
  return (Data*)this;
}

Slot Object::delParent(const Slot& parent) {
  Data* pobj = (Data*)parent;
  if (AKO(pobj,Object)) {
    ((Object*)pobj)->delChild(this);
  }
  return parent_list->del( parent_list->search(parent));
}

Slot Object::delChild(const Slot& child) {
  
  return child_set->del( child_set->search(child));
}

Slot Object::create(const Slot &arg) const
{
  // we ignore constructor args for now, 
  // until we can find something to do with them
  return createChild();
}

Slot Object::parents() const {
  return parent_list;
}

Slot Object::children() const {
  return child_set;
}

ostream &Object::dump(ostream& out) const
{
  out.form("%s [%d]{0x%08x}", this->typeId(), length(), this);
  return out;
}

int Object::order( const Slot &arg ) const
{
  void *self = ((void*)(Data*)this);
  void *other = ((void*)(Data*)arg);

  if (self > other)
    return -1;
  else if (self == other)
    return 0;
  else 
    return 1;

}

IVKey::IVKey( const Slot &name, const Slot &definer ) :
  Tuple(2),
  _name(name),
  _definer(definer)
{}

IVKey::IVKey( const Slot &init ) :
  Tuple(2),
  _name((Data*)NULL),
  _definer((Data*)NULL)
{}

IVKey::IVKey() :
  Tuple(2),
  _name((Data*)NULL),
  _definer((Data*)NULL)
{}

 
Slot IVKey::toconstruct() const {
  return new IVKey();
}

IVKey::~IVKey() {}

// Uses the copy constructor
Data *IVKey::clone(void *where) const
{
    return new IVKey(this);
}

// returns a mutable copy of this
Data *IVKey::mutate(void *where) const
{
    return (Data*)this;
}



ostream &IVKey::dump(ostream& out) const
{
   
  out << typeId() << "<";
  if (_definer)
    out << _definer;
  else
    out << "[null]";
  out << "::" ;
  if (_name)
    out << _name;
  else
    out << "[null]";
  out << ">" << endl;
  return out;
}
