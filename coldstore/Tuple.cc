// Tuple.cc - ColdStore interface to Tuple
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#include "Data.hh"
#include "Store.hh"

#include "Vector.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"

#include <stdarg.h>

#include "TupleBase.th"
template class TupleBase<Slot>;
template class ATupleBase<Slot>;

#include "tSlot.th"
template union tSlot<Tuple>;

// construct a Tuple
Tuple::Tuple(int size, ...)
  : _TupleBase<Slot>(size<0?-size:size)
{
    // GCC 2.96 has a parser bug and doesn't like pointer types in va_arg
    typedef Data*va_type;
    if (size < 0) {
        va_list ap;
        va_start(ap, size);
        Slot *body = content();
        assert(body);
        size = -size;
        for (int i = 0; i < size; i++) {
            Data *thing = va_arg(ap, va_type);
            DEBLOG(cerr << "tuple from: " << thing << '\n');
            body[i] = thing;
        }
        va_end(ap);
    }
}

// construct a Tuple from a stack
Tuple::Tuple(int *size)
  : _TupleBase<Slot>((Slot*)(size+1), *size)
{}

// construct a Tuple from a Segment<Slot>
Tuple::Tuple(List *seg)
  : _TupleBase<Slot>(seg->Length())
{
  Slot *body = content();
  for (int i = 0; i < seg->Length(); i++) {
    body[i] = seg->content()[i];
  }
}

Tuple::~Tuple()
{}

Range::Range(const Slot &start, const Slot &finish)
  : Tuple(-2, (Data*)start, (Data*)finish)
{
}

Range::~Range()
{}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Tuple::clone(void *where) const
{
  return new (_TupleBase<Slot>::Length(), where) Tuple(*this);
}

// returns a mutable copy of this
Data *Tuple::mutate(void *where) const
{
  if (refcount() > 1) {
    return clone(where);
  } else {
    return (Data*)this;
  }
}

/////////////////////////////////////
// object

// the object's truth value
bool Tuple::truth() const
{
  return (!this || length());
}

// constructor args to recreate object
Slot Tuple::toconstruct() const
{
  return new List(this);
}

ostream &Tuple::dump(ostream& out) const
{
    return Data::dump(out);
    // this can't be simple inheritance because
    // Symbol and Error AKO Tuple needs special handling
}

// construct from constructor args
Slot Tuple::construct(const Slot &arg)
{
  Slot other = arg->toSequence();
  return new (other->length()) Tuple((List*)other);
}

// 1,0,-1 depending on order
int Tuple::order(const Slot &arg) const
{
  int cmp = typeOrder(arg);
  if (cmp)
    return cmp;

  Slot argIt = arg->iterator();
  Slot it = iterator();

  // traverse both sequences using Iterators
  for (;it->More() && argIt->More();) {
      if ((cmp = it->Next()->order(argIt->Next())))
	return cmp;
    }

  return length() - (arg->length());
}

//  Predicate: are lists equal?
bool Tuple::equal(const Slot &arg) const
{
  // typed equality
  if (typeOrder(arg))
    return false;

  Tuple *l = arg;
  if (this == l)
    return true;	// == is a reflexive relation

  // Tuples can only be equal if they're of the same length.
  int len = _TupleBase<Slot>::Length();
  if (l->_TupleBase<Slot>::Length() != len)
    return false;

  // See if any elements differ.
  for (int i = 0; i < len; i++) {
    if (!const_element(i)->equal(l->const_element(i)))
      return false;
  }

  return true;
}

/////////////////////////////////////
// sequence

// length as sequence
int Tuple::length() const
{
  return _TupleBase<Slot>::Length();
}

// concatenate two sequences
// convert tuple to List, let List do the hard work
Slot Tuple::concat(const Slot &arg) const
{
  if (!arg)
    return this;
  Slot l = new List(this);
  l->concat(arg);
  return l;
}

// triadic `[]', index
Slot Tuple::slice(const Slot &from, const Slot &len) const
{
  return new List(this, (int)from, (int)len);
}

// dyadic `[]', index
Slot Tuple::slice(const Slot &from) const
{
  return const_element((int)from);
}

// dyadic `search', matching subrange
Slot Tuple::search(const Slot &seq) const
{
  return Segment<Slot>((TupleBase<Slot>*)this).search(seq);
}

// replace subrange
Slot Tuple::replace(const Slot &from, const Slot &len, const Slot &value)
{
  Tuple *m = (Tuple*)mutate();
  Slot o = value->toSequence();	// get the argument as a Segment
  List *other = (List*)o;
  m->_TupleBase<Slot>::Replace(from, len, other->content(), other->Length());
  return m;
}

// replace subrange
Slot Tuple::replace(const Slot &from, const Slot &value)
{
  Tuple *m = (Tuple*)mutate();
  m->_TupleBase<Slot>::Replace(from, 1, &value, 1);
  return m;
}

// replace subrange
Slot Tuple::replace(const Slot &value)
{
  Tuple *m = (Tuple*)mutate();
  Slot o = value->toSequence();	// get the argument as a Segment
  List *other = (List*)o;
  m->_TupleBase<Slot>::Replace(0, other->Length(), other->content(), other->Length());
  return m;
}

#if 0
// Can't delete in a Tuple
// delete subrange
Slot Tuple::delSlice(const Slot &from, const Slot &len)
{
  Tuple *m = (Tuple*)mutate();
  m->TupleBase<Slot>::Replace(from, len, (Data*)0, 1);
  return m;
}

// delete item
Slot Tuple::del(const Slot &from)
{
  return delSlice(from, 1);
}
#endif

// return the implementation Segment
List *Tuple::toSequence() const
{
  return new List(this);
}

Slot Tuple::iterator() const
{
  return new List(this);	// List is its own iterator
}

Slot Tuple::call(Slot &args)
{
  return Data::call(args);
}

ostream& operator<< (ostream& out, const Tuple & tup)
{
  if (&tup) {
    out.form("[0x%08x,%d] ",
	     tup.content(), tup.Length());
    out << "{";
    const Slot *contents = tup.content();
    for (int i = 0; i < tup.Length(); i++)
        out << contents[i] << ", ";
    out << "}";
  } else {
    out << "[NULL]";
  }
  return out;
}
