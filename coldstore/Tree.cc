// Tree.cc - Coldstore Trees
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Tree.cc,v 1.9 2001/01/03 02:46:18 amphibian Exp $";

//#define DEBUGLOG
#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Tree.hh"

#include <stdarg.h>
// Constructors
Tree::Tree(int clen, int size, ...)
        : List(clen + (size>0)?size:-size),
          prefix(clen)
{
    // GCC 2.96 has a parser bug, and doesn't like pointer types in va_args
    typedef Data* tmpdataptr;
    va_list args;
    Vector<Slot> &v = (Vector<Slot>&)*this;	// treat this as a vector
    va_start(args, size);
    for (int _size = size; _size > 0; _size--) {
        v[clen + size - _size] = va_arg(args, tmpdataptr);
    }
    va_end(args);
}

Tree::Tree(const Slot &sequence)
        : List(sequence[0]->toSequence()->concat(sequence[1]->toSequence())),
          prefix(sequence[0]->toSequence()->length())
{
}

Tree::Tree(const Tree *tree)
        : List((List*)tree),
          prefix(tree->prefix)
{
    DEBLOG(cerr << "Tree Copy Pair construction: " << prefix << '\n');
    DEBLOG(cerr << "from: " << tree->dump(cerr) << '\n');
    DEBLOG(cerr << "-> " << dump(cerr) << '\n');
}

Tree::Tree(const Slot &colours, const Slot &children)
        : List(colours->toSequence()->concat(children->toSequence())),
          prefix(colours->toSequence()->length())
{
    DEBLOG(cerr << "Tree List Pair construction: " << prefix << '\n');
    DEBLOG(cerr << "colours: " << colours << '\n');
    DEBLOG(cerr << "children: " << children << '\n');
    DEBLOG(cerr << "-> " << dump(cerr) << '\n');
}

Tree::~Tree()
{}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Tree::clone(void *where) const
{
  return new Tree(this);
}

// returns a mutable copy of this
Data *Tree::mutate(void *where) const
{
    Data *result;
    if (refcount() > 1) {
        result = clone(where);
        DEBLOG(cerr << "mutate: clone "
               << "from " << dump(cerr)
               << " to " << ((Tree*)result)->dump(cerr) << '\n');
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
// an identity, for Trees
Slot Tree::toconstruct() const
{
  return toSequence();
}

ostream &Tree::dump(ostream& out) const
{
    if (this != (Tree*)0) {
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
int Tree::length() const
{
    return List::length() - prefix;
}


// dyadic `[]', item
Slot Tree::slice(const Slot &from) const
{
    int index = (int)from;
    return List::slice(index + prefix);
}

// dyadic `[]', item
Slot Tree::slice(const Slot &from, const Slot &len) const
{
    int index = (int)from;
    return List::slice(index + prefix, len);
}

// replace subrange
Slot Tree::replace(const Slot &from, const Slot &len, const Slot &val)
{
    int index = (int)from;
    return List::replace(index + prefix, len, val);
    
}

// replace item
Slot Tree::replace(const Slot &from, const Slot &val)
{
    int index = (int)from;
    return List::replace(index + prefix, val);
}

// insert
Slot Tree::insert(const Slot &from, const Slot &val)
{
    int index = (int)from;
    return List::insert(index + prefix, val);
}

// delete subrange
Slot Tree::del(const Slot &from, const Slot &len)
{
    int index = (int)from;
    return List::del(index + prefix, len);
}

// delete item
Slot Tree::del(const Slot &from)
{
    int index = (int)from;
    return List::del(index + prefix);
}

// dyadic `in', matching subrange
Slot Tree::search(const Slot &search) const
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
List *Tree::toSequence() const
{
    Slot colours = new List((List*)this, 0, prefix);
    Slot contents = new List((List*)this, prefix);
    List *result = new List(2, (Data*)colours, (Data*)contents);
    return result;
}

// return an iterator
Slot Tree::iterator() const
{
    return new List((List*)this, prefix);
    //return mutate();
  // was: return new List(this); but I don't think it needs to be dup'd
}
