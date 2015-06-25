// ChPromise - class from which all ColdStore entities must derive
// Copyright (C) 2001 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef CHPROMISE_HH
#define CHPROMISE_HH

#include <Promise.hh>
#include "semantics.hh"

/** ChPromise - Chaos lazy evaluation wrapper
 *
 */
class ChPromise
    : public Promise
{
    tSlot<Chaos> evaluator;
    Slot expression;
    
protected:
    /** destroy ChPromise instance
     * protected to encourage the use of RefCount::dncount()
     * in our copy-on-write reference-counted environment
     */
    virtual ~ChPromise();			// destruction via dncount(), please
    
    /** constructor - this is a virtual class, it merely allocates a tuple
     */
    ChPromise(tSlot<Chaos> ev, Slot &ex)
	: Promise(2), evaluator(ev), expression(ex)
	{}
    
    /** constructor - this is a virtual class, it merely allocates a tuple
     */
    ChPromise(const Slot &clos)
	: Promise(2), evaluator(clos[0]), expression(clos[1])
	{}
    
public:
    
    ////////////////////////////////////
    // structural
protected:
    
public:
    /** constructor arguments sufficient to recreate object
     * @return a primitive type (pickled) representation of this object
     */
    virtual Slot toconstruct() const;
    
    //////////////////////////
    // Reduction
    
  /** call object
   * 
   * treat object as functional and call it with arg
   * @param args the argument to the function call
   * @return whatever the underlying function returns
   */
  virtual Slot call(Slot &args);	// call object
};

#endif
