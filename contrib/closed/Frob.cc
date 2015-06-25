// Tuple - proxy + data
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
//      $Id

#include "Data.hh"
#include "Frob.hh"
#include "Object.hh"

Slot Frob::slice(const Slot &name) const {
  return _object->slice(name);
};

Slot Frob::search(const Slot &name) const {
  return _object->search(name);
};

Slot Frob::del(const Slot &name)  {
  return _object->del(name);
};

Slot Frob::insert(const Slot &sym) {
  return _object->insert(sym);
};

Slot Frob::insert(const Slot &key, const Slot &value) {
  return _object->insert(key, value);
};

int Frob::order(const Slot &arg) const {
  return _object->order( arg );
};

Slot Frob::iterator(){
  return _object->iterator();
};


// Uses the copy constructor
Data *Frob::clone(void *where) const
{
    //  return new (_TupleBase<Slot>::Length(), where) Frob(*this);
    return new Frob(this);
}
 
// returns a mutable copy of this
Data *Frob::mutate(void *where) const
{
    return (Data*)this;
}
