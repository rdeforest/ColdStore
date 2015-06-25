// OrderedSet - Coldstore Ordered Sets common functionality
// Copyright 2001 Matthew Toseland
// See LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#ifndef ORDEREDSET_HH
#define ORDEREDSET_HH
#include "Set.hh"

/*
OrderedSet can be iterated in contents order
It supports operator lshift, operator rshift
Logic operators are implemented based on the iterators
*/

class OrderedSet : public Set
{
public:
  /** factor Set into: [list1-only, intersection, list2-only]
   */
  Slot factor(const OrderedSet* pl) const;
  // Set's factor converts Set -> OrderedSet by default
  void factor(const OrderedSet* pl, Slot&, Slot&, Slot&) const;
  // default massively slow operators using iterators
  virtual int order(const Slot& arg) const;
  virtual bool equal(const Slot& arg) const;
};

#endif
