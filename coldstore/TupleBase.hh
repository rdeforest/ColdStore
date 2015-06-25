// TupleBase - templated class for a static array of elements of arbitrary type
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: TupleBase.hh,v 1.10 2002/02/27 01:49:31 coldstore Exp $	

#ifndef TUPLEBASE_HH
#define TUPLEBASE_HH

#include <typeinfo>
#include <stdlib.h>
#include <stddef.h>
#include <iostream.h>
#include <assert.h>

#include "Vector.hh"

class ostream;

/** a templated arbitrary- but fixed- size collection of some basetype

    TupleBase is implemented as an immobile allocation which contains by
    inclusion, not reference.

    _TupleBase is a simpler type which doesn't destroy its allocation contents
    when it is destroyed, and is therefore suitable for use by Tuple.
    
    Note: C++ Children of _TupleBase have their internal class data members
    exposed to indexing by the _content[0] array data member.
*/
template <class T>
/*coldT*/ class _TupleBase
  : public Data
{

protected:

  virtual ~_TupleBase();

protected:
    /** dump Tuple to file
     */
    friend ostream& operator<< <T> (ostream& out, const _TupleBase<T> & vec);
    
    /** length of Tuple
     */
    int _length;
    
    /** Tuple's contents (overlayed on whatever follows)
     */
    mutable T _content[];
    
public:
    
    /////////////////////////////
    // Simple constructors
    /** construct a Tuple with pre-allocation of storage
     */
    _TupleBase(int size);

    /** consistency check
     */
    virtual void check(int i = 1) const;

    /** allocate space for Tuple of given byte size
     */
    void *operator new (unsigned int size, void *allocator = (void*)0);

    /** allocate space for Tuple of given size, with extra overhead
     */
    void *operator new (unsigned int size,
                        int extra, void *allocator = (void*)0);

    /////////////////////////////////////
    // copy constructors of sundry kinds

    /** Copy T* array of a given size
     */
    _TupleBase(const T *contentT, int size);

    /** Copy null-terminated T* array
     */
    _TupleBase(const T *contentT);

    /** Copy Segment
     */
    _TupleBase(const Segment<T> &contentT, int size=0);

    /** Copy Vector
     */
    _TupleBase(const Vector<T> *contentT, int size = 0);

    /** Repeat T& as array
     */
    _TupleBase(const T &contentT, int repetition = 1);

    /** Copy a Tuple<T>*'s contents
     */
    _TupleBase(const _TupleBase<T> *contentT);

    /** Copy a Tuple<T>&'s contents
     */
    _TupleBase(const _TupleBase<T> &contentT);

    /** Copy subrange Tuple<T>*
     */
    _TupleBase(const _TupleBase<T> *contentT, int _start, int l);

    /** Copy subrange Tuple<T>&
     */
    _TupleBase(const _TupleBase<T> &contentT, int _start, int l);

    /** union - v1 + v2
     */
    _TupleBase(const _TupleBase<T> *v1, const _TupleBase<T> *v2);

    /** union - v1 + v2
     */
    _TupleBase(const _TupleBase<T> &v1, const _TupleBase<T> &v2);

    /** argument range modifier
     */
    int endRelative(int where) const {
        // negative where is end-relative
        return (where < 0)? _length + where: where;
    }

public:
    T *content() const {
        return &(_content[0]);
    }

    int Length() const {
        return _length;
    }

    T &operator[](int offset) const {	// array accessor
        assert(offset >= 0);
        return _content[offset];
    }

    T &element(int offset) {	// array accessor
        assert(offset >= 0);
        return _content[offset];
    }

    const T &const_element(int offset) const {	// array accessor
        assert((offset >= 0) && (offset < _length));
        return _content[offset];
    }

    operator T*() const {	// array accessor
        return _content;
    }

    bool encloses(const Segment<T> &subrange) {
        return (subrange.content() >= _content)
            && ((_content + _length)
                >= (subrange.content() + subrange.Length()));
    }

    const T *last() const {	// ref to last in Segment
        return _content + _length - 1;
    }

    int bounds() const {
        return (Memory::mySize() - offsetof(_TupleBase<T>, _content))
            / sizeof(T);
    }

    // replace range with array
    _TupleBase<T> *Replace(int where, int what, const T *addT, int additional);
    inline _TupleBase<T> *Replace(int where, int what, const T *addT) {
        return Replace(where, what, addT, T::findNull(addT));
    }
    
    inline _TupleBase<T> *Replace(int where, int what, const T &addT,
                                 int repeat=1) {
        return Replace(where, what, &addT, repeat);
    }
    inline _TupleBase<T> *Replace(int where, int what, const Segment<T> *addT) {
        return Replace(where, what, addT->content(), addT->Length());
    }
    inline _TupleBase<T> *Replace(int where, int what,
                                 const _TupleBase<T> &addT) {
        return Replace(where, what, addT.content(), addT.Length());
    }

    // ColdStore interface
    virtual Data *clone(void *where) const;	// Uses the copy constructor
    virtual Data *mutate(void *where) const;	// return mutable copy of this

    /** throw an error - shouldn't be called
     */
    virtual int order(Slot const &) const {
        unimpl("operator order undefined on _TupleBase<>");
        return 0;
    }

    /** call object
     * 
     * treat object as functional and call it with arg
     * @param args the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot call(Slot &args);	// call object
};

template <class T> ostream& operator<< (ostream& out,  const _TupleBase<T> & vec);

template <class T>
/*coldT*/ class TupleBase
  : public _TupleBase<T>
{

protected:

  virtual ~TupleBase();

public:
    /////////////////////////////
    // Simple constructors

    /** construct a Tuple with pre-allocation of storage
     */
    TupleBase(int size)
            : _TupleBase<T>(size) {}

    /////////////////////////////////////
    // copy constructors of sundry kinds

    /** Copy a Tuple<T>*'s contents
     */
    TupleBase(const TupleBase<T> *contentT)
            :_TupleBase<T>(contentT) {}

    /** Copy T* array of a given size
     */
    TupleBase(const T *contentT, int size)
            :_TupleBase<T>(contentT, size) {}

    /** Copy null-terminated T* array
     */
    TupleBase(const T *contentT)
            : _TupleBase<T>(contentT) {}

    /** Repeat T& as array
     */
    TupleBase(const T &contentT, int repetition = 1)
            :_TupleBase<T>(contentT, repetition) {}
    
    /** Copy Vector
     */
    TupleBase(const Vector<T> *contentT, int size = 0)
            :_TupleBase<T>(contentT, size) {}

    /** Copy Vector
     */
    TupleBase(const Vector<T> &contentT, int size = 0)
            :_TupleBase<T>(contentT, size) {}

    /** Copy Segment
     */
    TupleBase(const Segment<T> &contentT, int size=0)
            :_TupleBase<T>(contentT, size) {}

    /** Copy a Tuple<T>&'s contents
     */
    TupleBase(const TupleBase<T> &contentT)
        :_TupleBase<T>(contentT) {}

    /** Copy subrange Tuple<T>*
     */
    TupleBase(const TupleBase<T> *contentT, int _start, int l)
            :_TupleBase<T>(contentT, _start, l) {}

    /** Copy subrange Tuple<T>&
     */
    TupleBase(const TupleBase<T> &contentT, int _start, int l)
            :_TupleBase<T>(contentT, _start, l) {}

    /** union - v1 + v2
     */
    TupleBase(const TupleBase<T> *v1, const TupleBase<T> *v2)
            :_TupleBase<T>(v1, v2) {}

    /** union - v1 + v2
     */
    TupleBase(const TupleBase<T> &v1, const TupleBase<T> &v2)
            :_TupleBase<T>(v1, v2) {}

public:
    // ColdStore interface
    virtual Data *clone(void *where) const;	// Uses the copy constructor
    virtual Data *mutate(void *where) const;	// return mutable copy of this

    /** call object
     * 
     * treat object as functional and call it with arg
     * @param args the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot call(Slot &args);	// call object
};

template <class T>
class ATupleBase
  : public TupleBase<T>
{
public:
    /** ATupleBase new
        @param size length of the main allocation
        @param location alloca()d location of storage
        @param extra extra allocation in bytes
        @return storage allocated as requested
    */
    void *operator new(size_t size, void *location) {
        memset(location, '\0', size);
        return location;
    };

    /** ATupleBase delete
        @param ptr a pointer returned by @ref new()
        @param extra extra allocation in bytes
        @param where neighborhood into which to allocate
    */
    void operator delete(void* ptr) {}

    virtual void dncount( void) const;

    // Some constructors for ATupleBase
    /** Copy T* array of a given size
     */
    ATupleBase(const T *contentT, int size)
            : TupleBase<T>(contentT, size) {}
    
    /** Copy null-terminated T* array
     */
    ATupleBase(const T *contentT)
            : TupleBase<T>(contentT) {}
    
    /** Copy Segment
     */
    ATupleBase(const Segment<T> &contentT, int size=0)
            : TupleBase<T>(contentT, size) {}
    
    /** Copy Vector
     */
    ATupleBase(const Vector<T> *contentT, int size = 0)
            : TupleBase<T>(contentT, size) {}
    
    /** Repeat T& as array
     */
    ATupleBase(const T &contentT, int repetition = 1)
            : TupleBase<T>(contentT, repetition) {}
#if 0    
    /** Copy a Tuple<T>*'s contents
     */
    ATupleBase(const TupleBase<T> *contentT)
            : TupleBase<T>(contentT) {}
#endif
    /** Copy a Tuple<T>&'s contents
     */
    ATupleBase(const TupleBase<T> &contentT)
            : TupleBase<T>(contentT) {}
    
    /** Copy subrange Tuple<T>*
     */
    ATupleBase(const TupleBase<T> *contentT, int _start, int l)
            : TupleBase<T>(contentT, _start, l) {}
    
    /** Copy subrange Tuple<T>&
     */
    ATupleBase(const TupleBase<T> &contentT, int _start, int l)
            : TupleBase<T>(contentT, _start, l) {}
    
    /** union - v1 + v2
     */
    ATupleBase(const TupleBase<T> *v1, const TupleBase<T> *v2)
            : TupleBase<T>(v1, v2) {}
    
    /** union - v1 + v2
     */
    ATupleBase(const TupleBase<T> &v1, const TupleBase<T> &v2)
            : TupleBase<T>(v1, v2) {}
    
    virtual ~ATupleBase() {}
};
#endif
