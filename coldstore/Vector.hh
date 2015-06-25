// templated Vector class for dynamic arrays of elements of arbitrary type
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: Vector.hh,v 1.11 2001/07/11 14:08:30 amphibian Exp $	

#ifndef VECTOR_HH
#define VECTOR_HH

#include <typeinfo>
#include <stdlib.h>
#include <assert.h>

#include "Segment.hh"

class ostream;

/** Templated Vector class
   A Vector is a pair of Segments, one (Vector's parent) delineates
   the active subrange of the second, (called allocation)
   allocation is a TupleBase array allocated by QVMM, it encloses the active subrange
*/
template <class T>
class Vector
    : public Segment<T>	// the active region of a Vector
{
public:
    // static vars controlling memory allocation internal overhead

    /** a extra proportion of allocation (3 == 133%)
     */
    static unsigned int extra_allocation;

    /** minimum proportional occupancy before shrinking
     */
    static unsigned int minimum_occupancy;

    /** the minimum size of an allocation in bytes
     */
    static unsigned int minimum_size;

    /** calculate an allocation based on the above parameters
     */
    static int actual_allocation(int newlen);

protected:
#if 1
    /** dump Vector to file
     */
    friend ostream& operator<< <T> (ostream& out, const Vector<T> & vec);
#endif

    /** Vector's allocation is managed with rlloc, resize and shrink
     */

    /** the allocated space for this Vector
     */
    TupleBase<T> *allocation;

    /** accessor for @ref allocation
     */
    TupleBase<T> *Allocation() const {
        return allocation;
    }

    /** reallocate Tuple
        create a new allocation of actual_allocation(len) size
        @param newlen new length Vector allocation
     */
    void rlloc(int newlen);

    /** realloc Tuple to accomodate new growth

        ensure Vector has at least len elements, reallocating as needed
        @param newlen new length Vector allocation
    */
    void resize(int newlen);
    
    /** reallocate Tuple if shrunk below minimum utilization
        reduce an allocation if it's fallen below the low-water-mark
        defined for this Vector type
    */
    void shrink();

    /** set the Segment of this Vector
     */
    Segment<T>::set;

    // Make a clean copy of the Vector
    bool Mutate();

    /** fix up start (for insert etc) */
    static inline void adjust_index_start(int& _start, int rlen);

    /** fix up index parameters, throw on error */
    inline void adjust_indices_here(int& _start, int& _length);

    /** fix up index parameters, with no object so no length. Throw on error.*/
    static inline void adjust_indices(int& _start, int& _length, int rlen);

public:

    /////////////////////////////
    // Simple constructors

    /** Null Vector constructor
     */
    Vector();

    /** construct with pre-allocation
     */
    Vector(unsigned int size);

    /** destructor - does nothing
     */
    virtual ~Vector();

    /** destroy contents
     */
    void destroy(int len = 1);

    /** consistency check
     */
    void check(int i = 1) const;

    
    /////////////////////////////////////
    // accessors

    /** Segment array accessor
     */
    T &operator[](int offset);

    /** hoist @ref Segment element accessor
     */
    Segment<T>::element;

    /** hoist @ref Segment empty predicate
     */
    Segment<T>::empty;

    /** hoist @ref Segment boolean cast
     */
    Segment<T>::operator bool;


    /////////////////////////////////////
    // copy constructors of sundry kinds

    /** Copy T* array of a given size
     */
    Vector(const T *contentT, unsigned int size);

    /** Copy Segment
     */
    Vector(const Segment<T> &contentT);

    /** Make singleton T& Vector
     */
    Vector(const T &contentT);

    /** Repeat T& as array
     */
    Vector(const T &contentT, unsigned int repetition);

    /** Reference Tuple
     */
    Vector(TupleBase<T> *t);

    /** Reference Tuple
     */
    Vector(TupleBase<T> &t);

    /** Reference SubTuple
     @param _start start of subrange
     @param _len length of subrange
     */
    Vector(TupleBase<T> *t, int _start, int _len=-1);

    /** Reference SubTuple
     @param _start start of subrange
     @param _len length of subrange
     */
    Vector(TupleBase<T> &t, int _start, int _len=-1);

    /** Reference Vector<T>*
     */
    Vector(const Vector<T> *v);

    /** Reference Vector<T>&
     */
    Vector(const Vector<T> &v);

    /** Reference subrange Vector<T>*
     @param _start start of subrange
     @param _len length of subrange
     */
    Vector(const Vector<T> *v, int _start, int _len=-1);

    /** Reference subrange Vector<T>&
     @param _start start of subrange
     @param _len length of subrange
     */
    Vector(const Vector<T> &v, int _start, int _len=-1);

    /** union: v1 + v2
     */
    Vector(const Vector<T> *v1, const Vector<T> *v2);

    /** union: v1 + v2
     */
    Vector(const Vector<T> &v1, const Vector<T> &v2);
    
private:
    /** doDel - delete but don't shrink (used by replace)
     */
    Vector<T> *doDel(int where, int what);

public:

    /** mutator - remove range from Vector
     */
    Vector<T> *del(int where = 0, int what=-1);


    /** mutator - append values to right
     */
    Vector<T> *vconcat(const T *addT, int additional);


    /** mutator - append values to right
     */
    Vector<T> *vconcat(const T *addT) {
        return vconcat(addT, T::findNull(addT));
    }

    /** mutator - append values to right
     */
    Vector<T> *vconcat(const T &addT) {
        return vconcat(&addT, 1);
    }

    /** mutator - append values to right
     */
    Vector<T> *vconcat(const Segment<T> *addT) {
        return vconcat(addT->content(), addT->Length());
    }

    /** mutator - append values to right
     */
    Vector<T> *vconcat(const Segment<T> &addT) {
        return vconcat(addT.content(), addT.Length());
    }

    /** mutator - append values to right
     */
    Vector<T> *vconcat(int count, ...);

    /** mutator - insert values before point
     */
    Vector<T> *vinsert(int where, const T *addT, int what);

    /** mutator - insert values before point
     */
    Vector<T> *vinsert(int where, const T *addT) {
        return vinsert(where, addT, T::findNull(addT));
    }

    /** mutator - insert values before point
     */
    Vector<T> *vinsert(int where, const Segment<T> *addT) {
        return vinsert(where, addT->content(), addT->Length());
    }

    /** mutator - insert values before point
     */
    Vector<T> *vinsert(int where, const Segment<T> &addT) {
        return vinsert(where, addT.content(), addT.Length());
    }

    /** mutator - replace range with array
     */
    Vector<T> *Replace(int where, int what, const T *addT, int additional);

    /** mutator - replace range with array
     */
    Vector<T> *Replace(int where, int what, const T *addT) {
        return Replace(where, what, addT, T::findNull(addT));
    }

    /** mutator - replace range with array
     */
    Vector<T> *Replace(int where, int what, const Segment<T> *addT) {
        return Replace(where, what, addT->content(), addT->Length());
    }

    /** mutator - replace range with array
     */
    Vector<T> *Replace(int where, int what, const Vector<T> &addT) {
        return Replace(where, what, addT.content(), addT.Length());
    }

    ////////////////////////////////////
    // mutator - set operations on Vector
    /** add an element to the set, true if added
     */
    bool set_add(const T &d);

    /** delete an element from the set, true if deleted
     */
    bool set_remove(const T &d);

    /** convert Vector to set
     */
    void toset();
};

#if 1
// dump Vector to file
template <class T> ostream& operator<< (ostream& out,  const Vector<T> & vec);
#endif

#endif
