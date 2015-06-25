// Segment - weak Vector
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: Segment.hh,v 1.5 2001/05/13 05:33:53 coldstore Exp $	

#ifndef SEGMENT_HH
#define SEGMENT_HH

#include <string.h>
//#include <qvmm.h>
#include <assert.h>

template <class T>
class TupleBase;

/** Segment<T> templated class for weak @ref Vector

    A Segment<T> is a pair (T *start, length) designed to implement a
    weak Vector<T> (that is, one which doesn't manage its own storage)

    Segment may be constructed from a length, an explicit (start,len) pair,
    or another Segment

    Segment may only be constructed by a derived type.

    Segment performs no content manipulation in its operation,
    nor bounds checking, but defines the predicate segEncloses
    for that purpose in a derived type.
*/
template <class T>
class Segment
{
protected:
  /** start of segment (or (T*)0)
   */
  T *start;

  /** length of segment (or 0)
   */
  int len;
  
  /** explicitly set the segment's value
   */
  void set(T* t, int size=0);
  
public:
  //////////////////////////////////////////////
  // Simple Constructors
  
  /** default constructor - NULL segment
   */
  Segment();

  /** construct a singleton Segment
   */
  Segment(T &thing);

  /** construct a Segment conforming to a @ref TupleBase
   */
  Segment(TupleBase<T> &t);

  /** construct a Segment conforming to a @ref TupleBase
   */
  Segment(TupleBase<T> *t);

  //////////////////////////////////////////////
  // Storage operators

  /** allocate an array of T's
   */
  static T *alloc(int nelem, void *allocator = (void*)0);

  /** Deallocate storage pointed at by Segment
   */
  void dealloc();

  //////////////////////////////////////////////
  // Constructors
#if 0
  // allocate a new Segment of a given size (possibly NULL)
  Segment(int _len, void *allocator = (void*)0)
    : start((T*)((_len > 0)? alloc(_len, allocator) :(T*)0)), len(_len)
    {
      assert(len >= 0);
    }
#endif

  /** construct a Segment from an address/len pair
   */
  Segment(T *_start, int _len);
    
  /** Copy Constructor: construct a prefix sub-Segment of another Segment
   */
  Segment(const Segment<T> &seg, int _len = 0, int offset = 0);
    
#if 0
  Segment<T> operator=(T *location) {		// Segment assignment
    start = location;
    len = 0;
    return *this;
  }

  Segment<T> operator=(Segment<T> *location) {	// Segment assignment
    start = location->start;
    len = location->len;
    return *this;
  }
#endif

  /** Segment assignment
   */
  Segment<T> operator=(const Segment<T> &location);

  //////////////////////////////////////////////
  // Limit mutators

  /** advance lower edge of Segment by amount
   */
  Segment *operator+=(int advance);

  /** advance lower edge of Segment by 1
   */
  Segment *operator++(int);
    
public:
    
  virtual ~Segment();
  

  /** find occurrence in Segment
   */
  int search(const T* what, int whatlen, int offset = 0) const;

  /** find occurrence in Segment
   */
  int search(const T &what, int offset = 0) const;

  /** find occurrence in Segment
   */
  int search(const T *what, int offset = 0) const;

  /** find occurrence in Segment
   */
  int search(const Segment<T> &what, int offset = 0) const;

  /** find occurrence in Segment
   */
  int search(const Segment<T>* what, int offset = 0) const;

  //////////////////////////////////////////////
  // Element accessors

  /** Segment array accessor
   */
  T &operator[](int offset) const;

  /** return in-bounds const element
   */
  const T &const_element(int index) const;

  /** return in-bounds const element
   */
  T &element(int index) const;

  /** Segment length
   */
  int Length() const;

  /** Segment content
   */
  T* content() const;

  /** Segment to basetype conversion
   */
  operator T* () const;
    
  ////////////////////////////////////////
  // convenience accessors
    
  /** pointer to start of Segment
   */
  T *segFirst() const;

  /** ref to first in Segment
   */
  const T &first() const;

  /** pointer to end of Segment
   */
  T *segLast() const;

  /** ref to last in Segment
   */
  const T &last() const;

  //////////////////////////////////////////////
  //	predicates
  
  /** Predicate: Segment encloses a given range
   */
  bool segEncloses(const T * const _start, int _len) const;

  /** Predicate: Segment encloses another segment
   */
  bool segEncloses(Segment<T> const &seg) const;
    
  /** return the offset of T *t in content
   */
  int in(T *t, int tlen = 0) const;

  /** predicate:  Segment is empty
   */
  bool empty() const;

  /** predicate:  Segment is empty
   */
  operator bool () const;

  //////////////////////////////////////////////
  // comparison operators
  static int cmp(const T *l, const T *r);
  int order(const Segment<T> &that) const;
  bool operator==(const Segment<T> &d) const;
  bool operator!=(const Segment<T> &d) const;
  bool operator>=(const Segment<T> &d) const;
  bool operator> (const Segment<T> &d) const;
  bool operator<=(const Segment<T> &d) const;
  bool operator< (const Segment<T> &d) const;
    
  //////////////////////////////////////////////
  // element ordering operations

  /** qsort Segment contents
   */
  Segment<T> *qsort();

  /** reverse Vector
   */
  Segment<T> *reverse();

  /** bsearch sorted Vector
   */
  bool tsearch(const T &key, int &idx) const;

  ////////////////////////////////////
  // set operations on Segment
  
  /** Predicate: is an element a member of the set?
   */
  bool isMember(const T &d);
    
protected:
  //////////////////////////////////////////////
  // accessory operators

  /** argument range modifier
   */
  int endRelative(int where) const;

  /** duplicate a range of basetype
   */
  T *dup(T* to, const T* from, int range);
};

#endif
