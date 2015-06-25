// List.cc - Coldstore Lists
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: List.cc,v 1.48 2002/05/09 13:31:16 coldstore Exp $";

//#define DEBUGLOG
#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"

#include "Segment.th"
template class Segment<Slot>;

#include "TupleBase.th"
template class _TupleBase<Slot>;

#include "Vector.th"
template class Vector<Slot>;

unsigned int Vector<Slot>::minimum_size = 4;
unsigned int Vector<Slot>::extra_allocation = 4;
unsigned int Vector<Slot>::minimum_occupancy = 4;

#include "tSlot.th"
template union tSlot<List>;
template union tSlot<Stack>;

#include <stdarg.h>

void stdJunk(int size, Slot *to, va_list from)
{
    typedef Data *tmpdataptr;
    //tmpdataptr datum;
    for (int i = 0; i < size; i++)
        to[i] = va_arg(from, tmpdataptr);
}

// Constructors
List::List(int size, ...)
//    : Vector<Slot>(&first, size)
      : Vector<Slot>(size<0?-size:size)
{
    if (size < 0) {
        va_list ap;
        va_start(ap, size);
        size = -size;
        for (int i = 0; i < size; i++) {
            Slot datum = va_arg(ap, Slot);
            this->Vector<Slot>::vconcat(&datum, 1);
        }
        va_end(ap);
        DEBLOG(cerr << "Sized List construction and init " << Length()
                   << ' ' << size
                   << ' ' << *this
                   << '\n');
    }
}

// Constructors
List::List(TupleBase<Slot>* contentT, int size, ...)
//    : Vector<Slot>(&first, size)
      : Vector<Slot>(contentT)
{
    if (size < 0) {
        va_list ap;
        va_start(ap, size);
        size = -size;
        for (int i = 0; i < size; i++) {
            Slot datum = va_arg(ap, Slot);
            this->Vector<Slot>::vconcat(&datum, 1);
        }
        va_end(ap);
        DEBLOG(cerr << "Sized List construction and init " << Length()
                   << ' ' << size
                   << ' ' << *this
                   << '\n');
    }
}

List::List(const List *contentT, int start, int l)
  : Vector<Slot>(contentT, start, l)
{
}

List::List(const List &contentT, int start, int l)
  : Vector<Slot>(contentT, start, l)
{
}

List::List(const Vector<Slot>* contentT, int start, int l)
  : Vector<Slot>(contentT, start, l)
{
}

List::List(const Vector<Slot>& contentT, int start, int l)
  : Vector<Slot>(&contentT, start, l)
{
}

List::List(const Tuple *contentT, int start, int l)
  : Vector<Slot>((TupleBase<Slot>*)contentT, start, l)
{
}

List::List(TupleBase<Slot> *contentT)
  : Vector<Slot>(contentT)
{
}

List::List(const Slot &sequence)
        : Vector<Slot>(sequence->toSequence())
{
    DEBLOG(cerr << "Slot List construction " << Length()
           << ' ' << *this
           << '\n');
}


List::~List()
{}

/* factor
 * returns a list of 3 lists: [elements only of list1, intersection, elements only of list2]
 */
Slot List::factor(const List *l) const {
  Slot dl1 = new List(this);
  Slot inter = new List((int)length() + (int)(l->length()));
  Slot dl2 = new List(l);
    
#ifndef EDEBUG
  Vector<Slot>::check();
  ((List*)l)->Vector<Slot>::check();
#endif

  for (int i = dl2->length(); i >= 0; i--) {
    Slot pos = dl1->search(dl2[i]);
    if (pos) {
      // in intersection - move it and remove it
      int p = pos;
      inter->add(dl2->slice(i));
      ((List*)dl1)->Vector<Slot>::del(p);
      ((List*)dl2)->Vector<Slot>::del(i);
    }
  }

  // format up the return value
  Slot factor = new List(3);
  factor->add(dl1);
  factor->add(inter);
  factor->add(dl2);

  return factor;
}
  
/////////////////////////////////////
// structural

// Uses the copy constructor
Data *List::clone(void *where) const
{
  return new (where) List(*this, 0, Length());
}

// returns a mutable copy of this
Data *List::mutate(void *where) const
{
    Data *result;
    if (refcount() > 1) {
        result = clone(where);
        DEBLOG(cerr << "mutate: clone "
               << "from " << this
               << " to " << result << '\n');
    } else {
        result = (Data*)this;
        DEBLOG(cerr << "mutate: dup " << this);
    }
    assert(length() == result->length());	// catch a bonehead bug
    
    return result;
}


void List::check(int) const
{
  validGuard();
  Vector<Slot>::check();
}

/////////////////////////////////////
// object

// the object's truth value
bool List::truth() const
{
  return !(Vector<Slot>::empty());
}

// constructor args to recreate object
// an identity, for Lists
Slot List::toconstruct() const
{
  return mutate();
}

// construct from constructor args
Slot List::construct(const Slot &arg)
{
  return new List((List*)arg);
}

extern bool DumpVerbose;
ostream &List::dump(ostream& out) const
{
    if (this != (List*)0) {
        if (DumpVerbose)
            out << '[' << Length() << ']';
        Slot i(iterator());
        out << "{";
        while (i->More()) {
            i->Next().Dump(out);
            if (i->More()) {
                out << ", ";
            }
        }
        out << "}";
    } else {
        out << "[NULL]";
    }
    return out;
}

ostream &Stack::dump(ostream& out) const
{
    return List::dump(out);
}

// 1,0,-1 depending on order
int List::order(const Slot &arg) const
{
    if (AKO(arg, Tuple) || (AKO(arg, List))) {
#ifdef EDEBUG
        check();
        arg->check();
#endif

        // Lists can only be equal if they're of the same length.
        int len = (int)MIN((arg->length()), (int)(length()));

        // See if any elements differ.
	// To make Tree compare correctly, we use slice on both sides
        for (int i = 0; i < len; i++) {
            int diff = slice(i)->order(arg->slice(i));
            if (diff != 0)
                return diff;
        }

        // they're equal to the end of one list, which is longer?
        return length() - (arg->length());
    }

  return typeOrder(arg);
}

//  Predicate: are lists equal?
bool List::equal(const Slot &arg) const
{
    if (AKO(arg, Tuple) || (AKO(arg, List))) {
        // test for strong equality
        if (dynamic_cast<void*>((Data*)arg) == dynamic_cast<void*>((Data*)this))
	    return true;
        // Lists can only be equal if they're of the same length.
        int len = length();
        if (arg->length() != len)
            return false;

        // See if any elements differ.
	// To make Tree compare correctly, we use slice on both
        for (int i = 0; i < len; i++) {
            if (!slice(i)->equal(arg->slice(i)))
                return false;
        }
        return true;
    }
    // typed equality
    if (typeOrder(arg))
        return false;
    throw new Error("list",arg,"equal types but not equal types");
}

/////////////////////////////////////
// arithmetic

// monadic `+', absolute value
Slot List::qsort() const
{
  List *m = (List*)mutate();
  m->Segment<Slot>::qsort();
  return m;
}

// monadic `-', negative absolute value
Slot List::reverse() const
{
  Slot m(mutate());
  size_t len;
  if ((len = m->length())) {
      for (unsigned int i = 0; i < len / 2; i++) {
          Slot tmp(m->slice(i));
          m = m->replace(i, m->slice(len - i - 1));
          m = m->replace(len - i - 1, tmp);
      }
  }
  return m;
}

Slot List::sortuniq() const
{
  List* m = (List*)qsort();
  int l = m -> length();
  if(l<2) return m;
  int iShifted = 0;
  Slot s, p;
  for(int iAt=0;iAt<l;iAt++)
    {
      s = m -> slice(iAt);
      iAt++;
      p = m -> slice(iAt);
      if(s == p)
	{
	  iShifted++;
	}
      else
	{
	  (*m)[iAt-iShifted] = (*m)[iAt];
	}
    }
  m->resize(l-iShifted);
  return m;
}

Slot List::join(const Slot sep = " ") const
{
  Slot str = "";
  Slot it = iterator();
  while (it->More()) {
    str = str->concat( it->Next() );
    if (it->More())
      str = str->concat( sep );
  }

  return str;
}

// dyadic `+', add
Slot List::add(const Slot &arg) const
{
    return concat(arg);
}


#if 0
// dyadic `-', subtract
Slot List::subtract(const Slot &arg) const
{
}

// dyadic `*', multiply
Slot List::multiply(const Slot &arg) const
{
}

// dyadic '/', divide
Slot List::divide(const Slot &arg) const
{
}

// dyadic '%', modulo
Slot List::modulo(const Slot &arg) const
{
}
  
/////////////////////////////////////
// bitwise

// unary '~', invert
Slot List::invert() const
{
}

// dyadic '&', bitwise and
Slot List::and(const Slot &arg) const
{
}

// dyadic '^', bitwise xor
Slot List::xor(const Slot &arg) const
{
}

// dyadic '|', bitwise or
Slot List::or(const Slot &arg) const
{
}

// dyadic '<<', left shift
Slot List::lshift(const Slot &arg) const
{
}

// dyadic '>>', right shift
Slot List::rshift(const Slot &arg) const
{
}
#endif

/////////////////////////////////////
// sequence

// predicate - is object a sequence
bool List::isSequence() const
{
  return true;
}

// length as sequence
int List::length() const
{
  return Vector<Slot>::Length();
}

// concatenate two sequences
Slot List::concat(const Slot &arg) const
{
  List *m = (List*)mutate();
  if (arg && arg->isSequence()) {
      m->Vector<Slot>::vconcat(arg->toSequence());
  } else {
      m->Vector<Slot>::vconcat(&arg, 1);
  }
  return m;
}

// dyadic `[]', range
Slot List::slice(const Slot &from, const Slot &len) const
{
  if (empty()) {
    if (((bool)len && (((int)len) != 0)) || ((int)from != 0)) {
      throw new Error("range", this, "Taking non-empty slice of an empty list");
    } else {
        return new List();
    }
  }

  return new List(this, from, len);
}

// dyadic `[]', item
Slot List::slice(const Slot &from) const
{
  if (empty()) {
      throw new Error("range", this,
                      "Taking slice of an empty list");
  }
  int f = from;
  if(f<0) {
    f += length();
  }

  return element(f);
}

// replace subrange
Slot List::replace(const Slot &from, const Slot &len, const Slot &val)
{
   List *m = (List*)mutate();
   Slot other = val->toSequence();	// get the argument as a List
   m->Vector<Slot>::Replace((int)from,
                            len,
                            ((List*)other)->content(),
                            other->length());
   return m;
}

// replace item
Slot List::replace(const Slot &from, const Slot &val)
{
  // replace a slice with the value
  List *m = (List*)mutate();
  DEBLOG(dump(cerr) << " mut: " << m->dump(cerr) << " replace " << val.Dump(cerr) << '\n');
  m->Vector<Slot>::Replace((int)from, 1, &val, 1);
  return m;
}

// replace subrange
Slot List::replace(const Slot &)
{
    return unimpl("monadic replace on List");
}

// insert
Slot List::insert(const Slot &from, const Slot &val)
{
  List *m = (List*)mutate();
  m->Vector<Slot>::vinsert((int)from, &val, 1);
  return m;
}

// insert
Slot List::insert(const Slot &val)
{
  List *m = (List*)mutate();
  m->Vector<Slot>::vconcat(&val, 1);
  return m;
}

// delete subrange
Slot List::del(const Slot &from, const Slot &len)
{
  // assignment to a null length range is insertion or deletion
  List *m = (List*)mutate();
  m->Vector<Slot>::del(from, len);
  return m;
}

// delete item
Slot List::del(const Slot &from)
{
  List *m = (List*)mutate();
  m->Vector<Slot>::del(from, 1);
  return m;
}

// dyadic `in', matching subrange
Slot List::search(const Slot &search) const
{
  if (search->isSequence()) {
      List *other = search->toSequence();
      Slot *found = Slot::search(content(),
                                 other->content(),
                                 other->length(),
                                 Length());
    if (found)
      return found - content();
  } else {
    // singleton search
    Slot *found = Slot::search(content(), &search, 1, 0);
    if (found)
      return found - content();
  }
  return (Data*)0;	// NULL result
}

// return the implementation Segment
List *List::toSequence() const
{
    Data *result = mutate();
    return dynamic_cast<List*>(result);
    //was: new List(this, start, len); but I don't think it needs to be dup'd
}

// return an iterator
Slot List::iterator() const
{
    return clone();
    //return mutate();
  // was: return new List(this); but I don't think it needs to be dup'd
}

// List can act as an Iterator on List
bool List::More() const
{
  return truth();
}

Slot List::Next()
{
  if (truth()) {
      Slot retval = slice(0);
    (*this)++;	// advance lower edge of Segment
    return retval;
  } else {
    throw new Error("break", this, "Iterator exhaustion");
  }
}

List *List::pop(Slot &val)
{
    if (length() < 1) {
        throw new Error("underflow", this, "stack underflow");
    }
    List *that = (List*)mutate();
    val = (*that)[length() - 1];
    return that->del(-1, 1);
}

List *List::push(const Slot &val)
{
    List *that = (List*)mutate();
    //    cout << "push: stack before length is " << stack->Length() << "\n";
    that->vconcat(&val,1);
    //    cout << "push: stack after length is " << stack->Length() << "\n";
    return that;
}

Slot List::Mutate()
{
    Slot that(mutate());
    ((List*)that)->Vector<Slot>::Mutate();
    return that;
}


Slot List::call(Slot &args)
{
  return Data::call(args);
}

Slot Stack::call(Slot &args)
{
  return List::call(args);
}

List *Stack::pop(Slot &val)
{
    if (length() < 1) {
        throw new Error("underflow", this, "stack underflow");
    }

    val = (*this)[length() - 1];
    Vector<Slot>::del(-1, 1);
    return this;
}

Slot &Stack::top()
{
  return element(length() - 1);
}

List *Stack::push(const Slot &val)
{
    //    cout << "push: stack before length is " << stack->Length() << "\n";
    vconcat(&val,1);
    //    cout << "push: stack after length is " << stack->Length() << "\n";
    return this;
}

////////////////////////////////////
// Network primitives

Slot Stack::connect(const Slot to)
{
    unimpl("Stack doesn't implement connect");
}

Slot Stack::disconnect(const Slot from)
{
    unimpl("Stack doesn't implement disconnect");
}
    
Slot Stack::incoming(const Slot from)
{
    unimpl("Stack doesn't implement incoming");
}

Slot Stack::outgoing(const Slot to)
{
    unimpl("Stack doesn't implement outgoing");
}

Slot List::connect(const Slot to)
{
    unimpl("List doesn't implement connect");
}

Slot List::disconnect(const Slot from)
{
    unimpl("List doesn't implement disconnect");
}
    
Slot List::incoming(const Slot from)
{
    unimpl("List doesn't implement incoming");
}

Slot List::outgoing(const Slot to)
{
    unimpl("List doesn't implement outgoing");
}

// Some utility List functions
void Push(Slot &list, const Slot &val)
{
    List *l = dynamic_cast<List*>((Data*)list);
    if (l) {
        list = l->push(val);
    } else {
        throw new Error("type", list, " not a list");
    }
}

Slot Pop(Slot &list)
{
    List *l = dynamic_cast<List*>((Data*)list);
    if (l) {
        Slot val;
        list = l->pop(val);
        return val;
    } else {
        throw new Error("type", list, " not a list");
    }
}

Slot Append(Slot &list, const Slot &val)
{
    List *l = dynamic_cast<List*>((Data*)list);
    if (l) {
        list = l->insert(-1, val);
    } else {
        throw new Error("type", list, " not a list");
    }
    return list;
}

