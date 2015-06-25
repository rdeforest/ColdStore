// Slot - Coldstore smartpointers
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: Slot.hh,v 1.28 2002/05/09 13:31:16 coldstore Exp $	

#ifndef SLOT_HH
#define SLOT_HH

#include <iostream.h>
#include <string.h>
#include <assert.h>
#include <tSlot.hh>

class Data;	// that to which a Slot maps
class UString;
class List;
class NList;
class Tuple;
class String;
class Range;
class Hash;
class NList;
class Dict;
class DictIt;
class Builtin;
class Symbol;
class Namespace;
class Name;
class Error;
class Integer;
class BigInt;
class Real;
class Object;
class NamedNamespace;
class Directory;
//class Frame;
class Message;
class Closure;
class Binding;
class Node;
class Set;
class DivideFunc;
class VectorSet;
class Connection;
class Server;

const unsigned long MARK_DATA = 0;
const unsigned long MARK_INTEGER = 1;
const unsigned long MARK_COMPLEX = 2;
const unsigned long MASK_DATA = (MARK_INTEGER | MARK_COMPLEX);

// [16.21] How do I provide reference counting with copy-on-write 
// semantics for a hierarchy of classes?

// Slot::Data is now the root of a hierarchy of classes,
// which probably cause it to have some virtual functions.
// Note that class Slot itself will still not have any virtual functions. 

// The Virtual Constructor Idiom is used to make copies of the Slot::Data objects.
// To select which derived class to create, the sample code below uses 
// the Named Constructor Idiom, but other techniques are possible 
// (a switch statement in the constructor, etc).
// Methods in the derived classes are unaware of the reference counting.

/** Slot class - a smart pointer to Data
    which provides for copy-on-write of the Data it manages

    Additionally, Slot makes use of pointer alignment to encode additional
    forms into a pointer: encoded Integers (31 bit integer values) and one
    additional form which will be used for `immediate' values such as Promise
    and Future.

    Note: can't make arrays of Slot - no default constructor
*/
SlotMeta union Slot
{
private:
    /** friend method to permit printing of Slot
     */
    friend ostream& operator<< (ostream&, const Slot &);

    /** Slot representation of Data*

        Invariant: data_ is never NULL when being operated on
    */
    mutable Data *data_;

    /** Slot representation of encoded Integer
     */
    long integer_;

protected:
    /** initialize Slot to NULL
     */
    void init() {
        data_ = (Data*)0;
    }
    
  /** convert an encoded Data into a plain Data
   */
  Data *mkData() const;

  /** if this is a complex Slot, simplify it
   */
  Data *simplify() const;

public:

    /** predicate: is Slot an encoded Integer?
     */
    inline bool isEncInt() const {
        return (integer_ & MARK_INTEGER) == MARK_INTEGER;
    }
    static inline bool isEncInt(const Data *d) {
        return ((((int)d) & MARK_INTEGER) == MARK_INTEGER);
    }

    /** predicate: is Slot a Continuation?
     */
    inline bool isComplex() const {
        return !isEncInt() && ((integer_ & MARK_COMPLEX) == MARK_COMPLEX);
    }
    static inline bool isComplex(const Data *d) {
        return !isEncInt(d) && ((((int)d) & MARK_COMPLEX) == MARK_COMPLEX);
    }

    /** predicate: is Slot a normal Data?
     */
    inline bool isData() const {
        return ((integer_ & MASK_DATA) == MARK_DATA);
    }
    static inline bool isData(const Data *d) {
        return ((((int)d) & MASK_DATA) == MARK_DATA);
    }

    ////////////////////////////////////
    // Slot construction

    /** default constructor - construct NULL Slot
     */
    Slot(Data *d = NULL);

    /** construct from const Data*
     */
    Slot(const Data *d);

    /** Integer constructor - construct Slot from integer
     */
    Slot(int l);

    /** reference constructor - construct Slot from Data
     */
    Slot(Data &d);

    /** reference constructor
     */
    //Slot(const Data &d);

    /** construct Slot from a string
        @param str string to assign to this
     */
    Slot(const char *str);

    /** copy constructor - dups reference
        @param f const Slot& to which to assign this
     */
    Slot(const Slot& f);

    /** copy constructor - dups reference
        @param s Slot& to which to assign this
     */
    Slot(Slot &s);

    /** assignment operator.  Initializes Slot
        @param f Slot& to which to assign this 
    */
    Slot& operator= (const Slot& f);

    /** assignment operator.  Initializes Slot
        @param d Data* to which to assign this
     */
    Slot& operator= (Data *d);

    /** assignment operator.  Initializes Slot
        @param i integer to which to assign this
     */
    Slot& operator= (int i);

    template <typename T>
    Slot(tSlot<T> &t)
	: data_((T*)t) {
	if (!isEncInt() && (T*)t)
	    mkData()->upcount();
	
	DEBLOG(cerr << "Converting tSlot " << (Data*)t
	       << " to Slot " << this << '\n');
    }

    ~Slot();

    ////////////////////////////////////
    // Dereferencing Slot

    /** dereference smartpointer
     */
    Data *operator->() const;

    /** dereference smartpointer
     */
    operator Data*() const;

    /** dereference smartpointer
     */
    operator Data&() const;

    /** dereference smartpointer as sequence
     */
    Slot operator [](int index) const;
    
    /** dereference smartpointer accessing underlying sequence
     */
    Slot operator [] (const Slot &) const;

    ////////////////////////////////////
    // Typed Dereference - checks rtti type and throws if non-conformant type

    operator String *() const;
    operator UString *() const;
    operator List *() const;
    operator NList *() const;
    operator Tuple *() const;
    operator Dict *() const;
    operator DictIt *() const;
    operator Symbol *() const;
    operator Name *() const;
    operator Namespace *() const;
    operator NamedNamespace *() const;
    operator Error *() const;
    operator Builtin *() const;
    operator Integer *() const;
    operator BigInt *() const;
    operator Real *() const;
    operator Set *() const;
    operator VectorSet *() const;
    operator Directory *() const;
    operator Connection *() const;
    operator Server *() const;
    operator int() const;
    operator long() const;
    operator char*() const;

    /** return the type pointed to
     */
    const char *typeId() const;

    /** non-NULL predicate:  convert Slot to a bool
     */
    inline operator bool () const {
        return data_ != (Data*)0;
    }

    /** Data ordering over Slot
     */
    int order(const Slot& d) const;

    /** possible reduction in strength for ==
     */
    bool equal(const Slot &d) const;

    ////////////////////////////////////
    // type predicates
    bool isNumeric() const;
    bool isList() const;
    bool isString() const;

    ////////////////////////////////////
    // comparisons are defined on Data virtuals `order' and `equal'

    bool operator==(const Slot &d) const;
    bool operator!=(const Slot &d) const;
    bool operator>=(const Slot &d) const;
    bool operator>(const Slot &d) const;
    bool operator<=(const Slot &d) const;
    bool operator< (const Slot &d) const;

  ////////////////////////////////////
  
  Slot operator+(const Slot& d) const;
  Slot operator-(const Slot& d) const;
  Slot operator*(const Slot& d) const;
  Slot operator/(const Slot& d) const;
  Slot operator%(const Slot& d) const;
  Slot operator ~() const;
  Slot operator&(const Slot& d) const;
  Slot operator^(const Slot& d) const;
  Slot operator|(const Slot& d) const;
  Slot operator<<(const Slot& d) const;
  Slot operator>>(const Slot& d) const;
  

    ////////////////////////////////////
    // Vector support (for List)
    
    /** consistency check on underlying Data array
     */
    static void check(const Slot *from, size_t len = 1);


    /** compare two Slots
     * used by @ref Vector::qsort and @ref Vector::bsearch
     */
    static int cmp(const Slot *l, const Slot *r) {
        return l->order(*r);
    }

    /** find offset of leftmost NULL element in range 
     */
    static int findNull(const Slot *from);
    
    /** move a range of Slots without touching them
     */
    static Slot *move(Slot *to, const Slot *from, size_t range);

    /** duplicate a range of Slots
        perform necessary reference counting
    */
    static Slot *dup(Slot *to, const Slot *from, size_t range);
    
    /** destroy the elements of the Slot range
     */
    static void destroy(Slot *from, size_t range);

    /** initialise default elements in Slot range
     */
    static void init(Slot *from, size_t range);

    /** fill a segment of slots with a slot
     */
    static void segFill(Slot *from, const Slot &, int);

    /** find a single matching T
     */
    static Slot *find(Slot *from, const Slot &_c, size_t range);

    /** find needle in haystack O(n^2)
     */
    static Slot *search(Slot *from, const Slot *needle, size_t len, size_t range);

    /** Data ordering over Slot
     */
    static int order(const Slot& d1, const Slot& d2);

    /** dump this Slot to a stream
     */
    ostream &Dump(ostream& out) const;
};

//ostream& operator<< (ostream& out, const Slot & slot);

#endif
