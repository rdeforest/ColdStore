// Tuple.hh - ColdStore interface to TupleBase
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef TUPLE_HH
#define TUPLE_HH

#include "Data.hh"
#include "TupleBase.hh"

/** a fixed but arbitrary length collection of @ref Slot
 *
 * Tuple resembles List in all but its inability to have 
 * elements inserted into or deleted from it.
 *
 * Tuple is implemented as an immobile allocation which contains by inclusion
 * not reference.
 *
 * Note: C++ Children of Tuple have their internal class data members exposed
 * by Tuple to indexing from ColdStore.  This is usually desireable, as for
 * Error or other classes which are represented by ntuples.
 */
cold class Tuple
    : public _TupleBase<Slot>
{
    /** dump Tuple
     */
    friend ostream& operator<< (ostream& out, const Tuple& vec);

    
protected:
    virtual ~Tuple();
    
public:
    // Constructors

    /** cold construct a Tuple
     */
    Tuple(const Slot *init);

    /** construct a Tuple from a size and a variable sized set of @ref Slot
     */
    Tuple(int size, ...);

    /** construct a Tuple from a stack
     */
    Tuple(int *size);

    /** construct a Tuple from a Segment
     */
    Tuple(List *segment);
    
    // structural
    virtual Data *clone(void *where = (void*)0) const;
    virtual Data *mutate(void *where = (void*)0) const;
    
    // object
    virtual bool truth() const;
    virtual Slot toconstruct() const;
    virtual ostream &dump(ostream&) const;
    
    static Slot construct(const Slot &arg);
    virtual int order(const Slot &arg) const;
    virtual bool equal(const Slot &arg) const;
    
    // sequence
    virtual int length() const;
    virtual Slot concat(const Slot &arg) const;
    virtual Slot slice(const Slot &from, const Slot &len) const;
    virtual Slot slice(const Slot &from) const;
    virtual Slot replace(const Slot &from, const Slot &len, const Slot &value);
    virtual Slot replace(const Slot &from, const Slot &value);
    virtual Slot replace(const Slot &value);
    
    virtual Slot search(const Slot &search) const;
    virtual Slot iterator() const;
    virtual List *toSequence() const;

    /** call object
     * 
     * treat object as functional and call it with arg
     * @param args the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot call(Slot &args);	// call object
};

// class Range - a duple
//
// Usually constructed with the dyadic :-operator, thus `from:to'
//
class Range
    : public Tuple
{
    
protected:
    virtual ~Range();
    
public:
    Range(const Slot &range);
    Range(size_t range);
    Range(const Slot &start, const Slot &end);
};

#endif

