// Closure - Closure object 
// Copyright (C) 2000,2001 Ryan Daum, Colin McCormack
// 
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// $Id 

#ifndef CLOSURE_HH
#define CLOSURE_HH

#include "Data.hh"
#include "Tuple.hh"
#include "Frame.hh"
#include "Symbol.hh"
#include "InterpreterNodes.hh"
#include "Binding.hh"

// Holgate says, "sounds like a Woody Allen film."
// Holgate says, "see, we need Closure."

/** Closure
 *
 * Closure stores: definer, variable binding, syntax nodes, source
 */

cold class Closure
  : public Tuple
{
public:
  Slot _definer;		/* definer for the closure if there is one, else
				   it must be either unbound or a child of another
				   closure */

  Slot _tree;			/* node tree of the actual closure */
  Binding *_binding;		/* the environment (variable stack) */
  Slot _locals;			/* number of local variables */
  Slot _source;			/* original (text) source for the closure */

public:

  /** Clone a closure
   */
  Closure( const Closure *from );

  /** construct a blank Closure with definer 
   *  @param definer the closure definer
   *  @param name the name of the closure
   */
  Closure( const Slot &definer );

  /** construct Closure with a node tree and source
   *  @param definer the closure definer
   *  @param source the source of the method
   */
  Closure( const Slot &definer, const Slot &tree, const Slot source );

  /** construct Closure from a C function pointer and a definer
   *  @param definer the closure definer
   *  @param function the C function pointer for the closure
   */
  Closure( const Slot &definer,  Slot (*function)(Frame *context ) );

  /** construct Closure with no definer, just
   *  a C function.
   *  @param function the c function pointer
   */
  Closure( Slot (*function)(Frame *context) );

  ///////////////////////////////
  // Node behaviours
  virtual bool isAtomic() const { return true; };

  ///////////////////////////////
  // Object behaviours
  virtual Slot receive( Message *msg, Frame *context );

  ///////////////////////////////
  // Structural
  virtual Data *clone(void *store = (void*)0) const;
  virtual Data *mutate(void *where) const;  

  /////////////////////

  virtual ostream &dump(ostream&) const;

private:
  // called on closure entry
  void prepare( Frame *context);

  // called on closure exit
  void finish( Frame *context );

};

#endif

