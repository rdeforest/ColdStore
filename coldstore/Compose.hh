// Compose.hh - Compose functionals
// Copyright (C) 2002 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef COMPOSE_HH
#define COMPOSE_HH

#include "Data.hh"
#include "Tuple.hh"
#include <tSlot.hh>

class Compose
  : public Tuple
{
  // NB: Tuple overlays the following
  /** the first element composed */
  Slot first;
  
  /** the second element composed */
  Slot second;
  
public:
  /** create a Compose
   * @param _first first element
   * @param _second second element
   */
  Compose(Slot _first, Slot _second);

  /** create a Compose
   * @param both sequence of [first,second]
   */
  Compose(const Slot &both);
  
  virtual ~Compose();
  
public:
    virtual Slot call(Slot &args);	// call object

};

#endif

