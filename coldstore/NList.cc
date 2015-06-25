// NList.cc - Coldstore NTuple Lists
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: NList.cc,v 1.9 2001/10/29 09:16:43 coldstore Exp $";

#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"
#include "NList.hh"

#include "tSlot.th"
template union tSlot<NList>;


// Constructors
NList::NList(int arity, int size)
    : List(size * arity), _arity(arity)
{
}

// Fill out Segment that's too short
// NB: mutates the underlying List
void NList::roundup()
{
  while (length() & arity()) {
    vconcat((Data*)0);	// fill it with NULLs
  }
}


NList::NList(int arity_, const List *l, int start, int len)
    : List(l, start * arity_, (len<0)?len:(len * arity_)), _arity(arity_)
{

  assert(_arity);
  roundup();	// fill out Segment that's too short
}

// share substructure with an NList
NList::NList(const NList *l, int start, int len)
  : List(l, start * l->arity(),
         (len<0)?len:(len * l->arity())),
            _arity(l->arity())
{
  roundup();	// fill out Segment that's too short
}

NList::~NList()
{}

void NList::check(int) const
{
  assert(_arity > 0);
  List::check();
}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *NList::clone(void *where) const
{
  return new (where) NList(*this);
}

// returns a mutable copy of this
Data *NList::mutate(void *where) const
{
  if (refcount() > 1)
    return clone(where);
  else
    return (Data*)this;
}

/////////////////////////////////////
// object

// constructor args to recreate object
// an identity, for NLists
Slot NList::toconstruct() const
{
  Slot result = new List(this);
  return result->insert(0, arity());
}

// construct from constructor args
Slot NList::construct(const Slot &arg)
{
  return new NList(arg[0], (List*)arg, 1);
}

/////////////////////////////////////
// sequence

// length as sequence
int NList::length() const
{
  return Vector<Slot>::Length() / arity();
}

// concatenate two sequences
Slot NList::concat(const Slot &arg) const
{
  if (!arg)
    return this;

  NList *m = List::concat(arg);	// returns a mutable copy
  m->roundup();
  return m;
}

// triadic `[]', range
Slot NList::slice(const Slot &from, const Slot &len) const
{
  return new NList(this, (int)from, (int)len);
}

// dyadic `[]', item
Slot NList::slice(const Slot &from) const
{
  return new List(this, (int)from * _arity, _arity);
}

// replace subrange
Slot NList::replace(const Slot &from, const Slot &len, const Slot &val)
{
  NList *m = (NList*)mutate();

  // replace a slice with the value
  Slot o = val->toSequence();	// get the argument as a List
  List *other = (List*)o;
  if (other->length() % _arity) {
    throw new Error("conformant", val, "not a conformant array");
  }
  m->Vector<Slot>::Replace((int)from * _arity, (int)len * _arity,
			   other->content(), other->length());

  return m;
}

// replace item
Slot NList::replace(const Slot &from, const Slot &val)
{
  return replace(from, 1, val);
}

// replace subrange
Slot NList::replace(const Slot &)
{
    return unimpl("monadic replace on NList");
}

// insert item
Slot NList::insert(const Slot &from, const Slot &val)
{
  NList *m = (NList*)mutate();
  Slot o = val->toSequence();	// get the argument as a List
  List *other = (List*)o;
  if (other->length() % _arity) {
    throw new Error("conformant", val, "not a conformant array");
  }
  m->Vector<Slot>::vinsert((int)from * _arity, other->toSequence());
  return m;
}

// insert item
Slot NList::insert(const Slot &val)
{
  NList *m = (NList*)mutate();
  Slot o = val->toSequence();	// get the argument as a List
  List *other = (List*)o;
  if (other->length() % _arity) {
    throw new Error("conformant", val, "not a conformant array");
  }
  m->Vector<Slot>::vinsert(length() * _arity, other->toSequence());
  return m;
}

// delete subrange
Slot NList::del(const Slot &from, const Slot &len)
{
  NList *m = (NList*)mutate();

  // assigning NULL is equivalent to deletion
  m->Vector<Slot>::del((int)from * _arity, (int)len * _arity);
  return m;
}

// delete subrange
Slot NList::del(const Slot &from)
{
  NList *m = (NList*)mutate();

  // assigning NULL is equivalent to deletion
  m->Vector<Slot>::del((int)from * _arity, _arity);
  return m;
}

// dyadic `search', matching subrange
Slot NList::search(const Slot &search) const
{
  // TODO - search with regard to arity boundary
  return unimpl("NList::search - TODO");
}

// return an iterator
Slot NList::iterator() const
{
  return new NList(this);
}

// monadic `+', absolute value
Slot NList::qsort()
{
  NList *m = (NList*)mutate();
  m->Segment<Slot>::qsort();
  return m;
}

// monadic `-', negative absolute value
Slot NList::reverse()
{
  NList *m = (NList*)mutate();
  m->Segment<Slot>::reverse();
  return m;
}

