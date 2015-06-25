#ifndef __FROB_HH
#define __FROB_HH
 
#include "Data.hh"
#include "Tuple.hh"
#include "Object.hh"

cold class Frob
  : public Tuple
{
protected:
  Slot _object;
  Slot _data;

public:
  /////////////////
  // Construction

  Frob() : Tuple(2) {};

  Frob( const Slot &from ) : Tuple(2) {};

  /** construct a Frob from an object and a piece of data
   */
  Frob( const Slot &object, const Slot &data ) : 
    Tuple(2),
    _object(object),
    _data(data)
  {};

public:
  ///////////////////////////////
  // Object wrappers

  virtual Slot slice(const Slot& name) const;
  virtual Slot search( const Slot &name) const;

  virtual Slot del(const Slot &); 

  virtual Slot insert(const Slot &key, const Slot &value);
  virtual Slot insert(const Slot &sym);

  virtual Slot data() {
    return _data; 
  };

  virtual int order(const Slot &arg) const;
  virtual Slot iterator();

  Data *Frob::clone(void *where) const;
  Data *Frob::mutate(void *where) const;


};

#endif
  
