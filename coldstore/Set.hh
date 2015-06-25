// Set - Coldstore Sets common functionality
// Copyright 2001 Matthew Toseland
// See LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#ifndef BASESET_HH
#define BASESET_HH

#include "Data.hh"
/* Set - implements common Set functionality, abstract base class
 */
class Set
  : public Data
{
public:
  virtual Data* mutate(void* where=(Data*)NULL) const;
  virtual bool truth() const;
  // toVectorSet converts any Set to a VectorSet
  virtual Slot toVectorSet() const = 0;
  /** factor Set into: [list1-only, intersection, list2-only]
   */
  // default hilariously slow and unscalable factors using toVectorSet
  virtual Slot factor(const List *l) const;
  virtual Slot factor(const Set* l) const;
  virtual void factor(const Set* pl, Slot&, Slot&, Slot&) const;
  // other name for factor
  virtual Slot modulo(const Slot& s) const;
  virtual Slot and(const Slot& arg) const; // intersection
  virtual Slot or(const Slot& arg) const; // union
  virtual Slot xor(const Slot& arg) const; // xor - everything which is in only one set
  // default hilariously slow and unscalable operators using toVectorSet
  virtual int order(const Slot& arg) const;
  virtual bool equal(const Slot& arg) const;

  /** unary '~', invert the Set
   */
  virtual Slot invert() const;
};

#endif
