// Data - class from which all ColdStore entities must derive
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef DATA_HH
#define DATA_HH

#include <typeinfo>
#include <qvmm.h>
#include "Segment.hh"
#include "Slot.hh"

#ifndef __opencxx
#define cold
#define coldT
#endif

inline int MIN(int a, int b)
{
#ifdef __opencxx
    if (a<b)
        return a;
    else
        return b;
#else
    return a <? b;
#endif
}

inline int MAX(int a, int b)
{
#ifdef __opencxx
    if (a>b)
        return a;
    else
        return b;
#else
    return a >? b;
#endif
}


class Builtin;		// Builtin handles dynamic loading of Data subtypes
class Error;
class DopeV;

class RefCount
  : public Memory
{
protected:
  /** number of references to this Memory
   */
  mutable size_t _refcount;
#ifdef REFMONITOR
    mutable bool initialized;
#endif
public:
  /** Construct RefCount Memory
      note: Memory starts with 0 refcount
  */
  RefCount(void)
    : _refcount(0)
#ifdef REFMONITOR
      , initialized(true)
#endif
    {}

  virtual ~RefCount() 
    {}
  
  /** add a reference to this
   */
  virtual RefCount *upcount( void) const;
            
  /** remove a reference to this (possibly delete)
   */
  virtual void dncount( void) const;
  
  /** return the refcount of this
   */
  size_t refcount( void) const { return _refcount; }
};

/** Refcounted Persistent Objects under the coldstore virtual protocol
 *
 * Data implements the coldstore virtual protocol, which provides a set of
 * semantic operations derived from Python's C API, sufficient to support
 * Arithmetic, Sequence, Map, Iterator and Functional types.
 */
cold class Data
    : public RefCount	// Data instances are RefCounted under qvmm
{
protected:
  friend RefCount;
  
    /** destroy Data instance
     * protected to encourage the use of RefCount::dncount()
     * in our copy-on-write reference-counted environment
     */
    virtual ~Data();			// destruction via dncount(), please
    
    /** throw an `unimplemented' Error
     * A place holder for unimplemented virtual protocol
     * elements
     * @param message message associated with the Error
     * @return nothing useful
     */
    Slot unimpl(const char *message) const;
    static Slot Unimpl(const char *message);

public:
    Data();
    DopeV *dopeV();

    //void destroy() {dncount();}
    
    /** check an instance of Data for coherence
     *  (whatever that means to the datatype)
     * @param num number of contiguous instances in an array
     * @return void, but could be changed to return a bool
     */
    virtual void check(int num=0) const;	// check an instance of Data
    
    /** the typeID for this type - derived from rtti
     */
    void *type() const;
    
    /** make an instance of a named type
     */
    static Slot New(const char *name, Slot &args, const char *so = NULL);

    /** compare types of Slots
     *
     * Types are given an arbitrary but fixed order to enable @ref Data::order
     *  between disparate types
     */
    int typeOrder(const Slot &arg) const;
    
    /** predicate: is this of that Builtin type?
     */
    //bool typeEQ(const Builtin *m) const;
    
    /** return the name of the type of this object, using rtti
     */
    const char *typeId() const;

    /** make this instance mutable
     */
    void mkMutable();

    /** make this instance COW
     */
    void mkCOW();

    ////////////////////////////////////
    // structural
protected:
    /** copy constructor
     *
     * Used as the destination of @ref mutate when a copy on write mutable
     * object is desired.
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual Data *clone(void *where = (void*)0) const=0;
    
    /** dynamic mutable copy
     *
     * Used as the destination of @ref mutate when the object is mutable.
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual Slot identity(void *where = (void*)0) const;

public:
    /** create a mutable copy
     * 
     * Depending upon whether the object is considered a mutable instance or
     * a copy-on-write instance, vtbl magic is used to redirect this call to
     * one of @ref identity or @ref clone
     *
     * @param where the locale into which to clone this object
     * @return mutable copy of object     
     */
    virtual Data *mutate(void *where = (void*)0)const;

    ////////////////////////////////////
    // object

    /** the object's truth value
     *
     * mapping of all objects onto true/false
     * @return the object's truth value
     */
    virtual bool truth() const;

    /** constructor arguments sufficient to recreate object
     * @return a primitive type (pickled) representation of this object
     */
    virtual Slot toconstruct() const;

    /** dump the object to a stream
     * @param ostream stream onto which to dump the object
     */
    virtual ostream &dump(ostream& output) const;

    /** construct an instance of Data from the constructor args given
     * @param arg constructor arguments, derived from @ref toconstruct
     * @return instance constructed from the args
     */
    static Slot construct(const Slot &arg);

    /** construct an instance of a subclass from the constructor args given
     * @param arg constructor arguments, derived from @ref toconstruct
     * @return instance constructed from args
     */
    virtual Slot create(const Slot &arg) const;


    /** order two objects
     * @param arg object to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual int order(const Slot &arg) const=0;

    /** equality operator
     * @param arg object to be compared to this
     * @return true/false depending on equality
     */
    virtual bool equal(const Slot &arg) const;

    ////////////////////////////////////
    // arithmetic

    /** monadic `+', absolute value
     */
    virtual Slot positive() const;

    /** monadic `-', negative absolute value
     */
    virtual Slot negative() const;

    /** dyadic `+', add
     */
    virtual Slot add(const Slot &arg) const;

    /** dyadic `-', subtract
     */
    virtual Slot subtract(const Slot &arg) const;

    /** dyadic `*', multiply
     */
    virtual Slot multiply(const Slot &arg) const;

    /** dyadic '/', divide
     */
    virtual Slot divide(const Slot &arg) const;

    /** dyadic '%', modulo
     */
    virtual Slot modulo(const Slot &arg) const;
  
    ////////////////////////////////////
    // bitwise

    /** unary '~', invert
     */
    virtual Slot invert() const;

    /** dyadic '&', bitwise and
     */
    virtual Slot and(const Slot &arg) const;

    /** dyadic '^', bitwise xor
     */
    virtual Slot xor(const Slot &arg) const;

    /** dyadic '|', bitwise or
     */
    virtual Slot or(const Slot &arg) const;

    /** dyadic '<<', left shift
     */
    virtual Slot lshift(const Slot &arg) const;

    /** dyadic '>>', right shift
     */
    virtual Slot rshift(const Slot &arg) const;


    ////////////////////////////////////
    // sequence

    /** addressable Sequence as a List
     */
    virtual List *toSequence() const;

    /** predicate - is object a sequence
     */
    virtual bool isSequence() const;

    virtual bool isMap() const { return false; }

    /** length as sequence
     */
    virtual int length() const;

    /** concatenate two sequences
     */
    virtual Slot concat(const Slot &arg) const;

    ////////////////////////////////////
    // Return range

    /** return a range
     * @param from index of start of range
     * @param len length of range
     * @return the range of elements of this sequence 'from' for 'len'
     */
    virtual Slot slice(const Slot &from, const Slot &len) const;

    /** return an element
     * @param element element index
     * @return the nominated element of this sequence
     */
    virtual Slot slice(const Slot &element) const;

    /** search for a matching element
     * @param search element for which to search
     * @return the first matching element of this sequence
     */
    virtual Slot search(const Slot &search) const;

    ////////////////////////////////////
    // Replacement of range

    /** replace a range with a value
     * @param value value to be replaced in this sequence
     * @param from slice-range start index
     * @param len slice-range length
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &from, const Slot &len, const Slot &value);

    /** replace an element with a value
     * @param value value to be replaced in this sequence
     * @param element element index to be replaced
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &element, const Slot &value);

    /** replace this with a value
     * @param value value to be replaced in this sequence
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &value);

    
    /** insert a value into this sequence
     * @param value value to be inserted in this sequence
     * @param before element index before which insertion is performed
     * @return a copy of this with the replacement performed
     */
    virtual Slot insert(const Slot &before, const Slot &value);

    /** insert a value at the end of this sequence
     * @param value value to be inserted in this sequence
     * @return a copy of this with the replacement performed
     */
    virtual Slot insert(const Slot &value);

    ////////////////////////////////////
    // Deletion of range

    /** delete a subrange from sequence
     * @param from slice-range start index
     * @param len slice-range length
     * @return a copy of this with the deletion performed
     */
    virtual Slot del(const Slot &from, const Slot &len);

    /** delete an item from sequence
     * @param element index of element to be deleted
     * @return a copy of this with the deletion performed
     */
    virtual Slot del(const Slot &from);

    /** return a map's domain
     * @return the a sequence of keys forming a map's domain
     */
    virtual Slot domain() const;

    ////////////////////////////////////
    // iterator

    /** an iterator over this sequence
     * @return an Iterator
     */
    virtual Slot iterator() const ;

    /** predicate - is object an iterator
     */
    virtual bool isIterator() const;

    /** predicate: is iterated sequence complete?
     */
    virtual bool More() const;

    /** advance iterator over sequence
     */
    virtual Slot Next();

    //////////////////////////
    // Reduction

    /** reduce object ( Frame )
     *
     *  returns result of semantic reduction on object
     */
    virtual void reduce(Slot &inexpr, Slot &context);

    ////////////////////////////////////
    // functional call

    /** call object
     * 
     * treat object as functional and call it with arg
     * @param args the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot call(Slot &args);	// call object

#if 0
    /** call object (Method, Builtin)
     */
    virtual Slot call(int nargs, ...);
#endif

    ////////////////////////////////////
    // Network primitives

    /** connection indication/reuest
     * @param to indication's source
     * @return 
     */
    virtual Slot connect(const Slot to);

    /** diconnection indication/request
     * @param from indication's source
     * @return 
     */
    virtual Slot disconnect(const Slot from);
    
    /** incoming available indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot incoming(const Slot from);

    /** ready-for-outgoing indication/request
     * @param from indication's source
     * @return where to send future indications - NULL to disable
     */
    virtual Slot outgoing(const Slot to);

    ////////////////////////////////////
    // comparisons defined on virtual `order'
    bool operator==(const Slot &d) const {
        return order(d) == 0;
    }
    bool operator!=(const Slot &d) const {
        return order(d) != 0;
    }
    bool operator>=(const Slot &d) const {
        return order(d) >= 0;
    }
    bool operator>(const Slot &d) const {
        return order(d) > 0;
    }
    bool operator<=(const Slot &d) const {
        return order(d) <= 0;
    }
    bool operator<(const Slot &d) const {
        return order(d) < 0;
    }
};

ostream& operator << (ostream &out, const Data *);

#include <DopeV.hh>
#endif
