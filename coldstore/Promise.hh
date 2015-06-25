// Promise - class from which all ColdStore entities must derive
// Copyright (C) 2001 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef PROMISE_HH
#define PROMISE_HH

#include "Data.hh"
#include "Tuple.hh"
#include "Null.hh"

/** Promise - lazy evaluation wrapper
 *
 */
class Promise
  : public Tuple
{
protected:
  /** destroy Promise instance
   * protected to encourage the use of RefCount::dncount()
   * in our copy-on-write reference-counted environment
   */
  virtual ~Promise();			// destruction via dncount(), please

  /** constructor - this is a virtual class, it merely allocates a tuple
   */
  Promise(size_t n);

public:

  ////////////////////////////////////
  // structural
protected:

public:
  /** constructor arguments sufficient to recreate object
   * @return a primitive type (pickled) representation of this object
   */
  virtual Slot toconstruct() const {
    return new Null;
  }

  /** call object
   * 
   * treat object as functional and call it with arg
   * @param args the argument to the function call
   * @return whatever the underlying function returns
   */
  virtual Slot call(Slot &args);	// call object
};

#endif
