// Ribbon.cc - Coldstore Ribbons
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Ribbon.cc,v 1.9 2001/10/29 09:16:43 coldstore Exp $";

//#define DEBUGLOG
#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Ribbon.hh"

#include <stdarg.h>

#include "tSlot.th"
template union tSlot<Ribbon>;

// Constructors
Ribbon::Ribbon(int clen, int size, ...)
        : List(clen + (size>0)?size:-size),
          prefix(clen)
{
    if (size < 0) {
        Slot* p = 1 + (Slot*)&size;
        for(int i=0;i<size;i++) {
            Slot& s = operator [] (i);
            s=*p;
            p--;
        }
        DEBLOG(cerr << "Sized Ribbon construction and init " << Length()
               << ' ' << *this
               << '\n');
    }
}

Ribbon::Ribbon(const Slot &sequence)
        : List(sequence[0]->toSequence()->concat(sequence[1]->toSequence())),
          prefix(sequence[0]->toSequence()->length())
{
}

Ribbon::Ribbon(const Ribbon *ribbon)
        : List((List*)ribbon),
          prefix(ribbon->prefix)
{
    DEBLOG(cerr << "Ribbon Copy Pair construction: " << prefix << '\n');
    DEBLOG(cerr << "from: " << ribbon->dump(cerr) << '\n');
    DEBLOG(cerr << "-> " << dump(cerr) << '\n');
}

Ribbon::Ribbon(const Slot &colours, const Slot &children)
        : List(colours->toSequence()->concat(children->toSequence())),
          prefix(colours->toSequence()->length())
{
    DEBLOG(cerr << "Ribbon List Pair construction: " << prefix << '\n');
    DEBLOG(cerr << "colours: " << colours << '\n');
    DEBLOG(cerr << "children: " << children << '\n');
    DEBLOG(cerr << "-> " << dump(cerr) << '\n');
}

Ribbon::~Ribbon()
{}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Ribbon::clone(void *where) const
{
  return new Ribbon(this);
}

// returns a mutable copy of this
Data *Ribbon::mutate(void *where) const
{
    Data *result;
    if (refcount() > 1) {
        result = clone(where);
        DEBLOG(cerr << "mutate: clone "
               << "from " << dump(cerr)
               << " to " << ((Ribbon*)result)->dump(cerr) << '\n');
    } else {
        result = (Data*)this;
        DEBLOG(cerr << "mutate: dup " << dump(cerr));
    }
    int l1 = length();
    int l2 = result->length();
    assert(l1 == l2);	// catch a bonehead bug
    
    return result;
}

/////////////////////////////////////
// object

// constructor args to recreate object
// an identity, for Ribbons
Slot Ribbon::toconstruct() const
{
  return toSequence();
}

ostream &Ribbon::dump(ostream& out) const
{
    if (this != (Ribbon*)0) {
        out << "[prefix:" << prefix << ']';	// just prepend prefix to dump
        List::dump(out);
    } else {
        out << "[NULL]";
    }
    return out;
}

/////////////////////////////////////
// sequence

// length as sequence
int Ribbon::length() const
{
    return List::length() - prefix;
}


// dyadic `[]', item
Slot Ribbon::slice(const Slot &from) const
{
    int index = (int)from;
    return List::slice(index + prefix);
}

// dyadic `[]', item
Slot Ribbon::slice(const Slot &from, const Slot &len) const
{
    int index = (int)from;
    return List::slice(index + prefix, len);
}

// replace subrange
Slot Ribbon::replace(const Slot &from, const Slot &len, const Slot &val)
{
    int index = (int)from;
    return List::replace(index + prefix, len, val);
    
}

// replace item
Slot Ribbon::replace(const Slot &from, const Slot &val)
{
    int index = (int)from;
    return List::replace(index + prefix, val);
}

// insert
Slot Ribbon::insert(const Slot &from, const Slot &val)
{
    int index = (int)from;
    return List::insert(index + prefix, val);
}

// delete subrange
Slot Ribbon::del(const Slot &from, const Slot &len)
{
    int index = (int)from;
    return List::del(index + prefix, len);
}

// delete item
Slot Ribbon::del(const Slot &from)
{
    int index = (int)from;
    return List::del(index + prefix);
}

// dyadic `in', matching subrange
Slot Ribbon::search(const Slot &search) const
{
    if (search->isSequence()) {
        List *other = search->toSequence();
        Slot *found = Slot::search(content() + prefix,
                                   other->content(),
                                   other->length(),
                                   Length());
        if (found)
            return found - content() - prefix;
    } else {
        // singleton search
        Slot *found = Slot::search(content() + prefix, &search, 1, 0);
        if (found)
            return found - content() - prefix;
    }
    return (Data*)0;	// NULL result
}

// return the implementation Segment
List *Ribbon::toSequence() const
{
    Slot colours = new List((List*)this, 0, prefix);
    Slot contents = new List((List*)this, prefix);
    List *result = new List(-2, (Data*)colours, (Data*)contents);
    return result;
}

// return an iterator
Slot Ribbon::iterator() const
{
    return new List((List*)this, prefix);
    //return mutate();
  // was: return new List(this); but I don't think it needs to be dup'd
}
