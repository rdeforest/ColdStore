// VectorSet - Coldstore Sets, implemented using sorted vectors
// Copyright 2000 Matthew Toseland
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#ifndef SET_HH
#define SET_HH

#include "Vector.hh"
#include "OrderedSet.hh"
/* Vector-based Set - implements a Set using an ordered dynamic array
 */
/* coldstore Sets are ordered with no dupes
   Set always sorts and orders
*/
class VectorSet
  : private Vector<Slot>,
    public OrderedSet
{
public:
    virtual Data* clone(void* where=(Data*)NULL) const;
    static Slot construct(const Slot& arg);
    virtual Slot create(const Slot& arg) const;
    // Constructors
    /** construct a VectorSet of given size (default 0)
     */
    VectorSet(unsigned int size = 0);

    /** construct and init a VectorSet of given size (default 0)
     */
    VectorSet(unsigned int size, ...);

    /** construct a VectorSet from a sequence Slot
     */
    VectorSet(const Slot &sequence);

    /** copy construct a subVectorSet
     */
    VectorSet(const List *contentT, int start=0, int l=-1);
    VectorSet(const VectorSet *contentT);
    VectorSet(const VectorSet *contentT, Slot startcmp, Slot endcmp);
    VectorSet(const Set *contentT);
    VectorSet(const Set *contentT, Slot startcmp, Slot endcmp);

    /** copy construct a subVectorSet
     */
    VectorSet(const List &contentT, int start = 0, int l = -1);
    VectorSet(const VectorSet &contentT);
    VectorSet(const VectorSet &contentT, Slot startcmp, Slot endcmp);
    VectorSet(const Set &contentT);
    VectorSet(const Set &contentT, Slot startcmp, Slot endcmp);

    /** construct a VectorSet from a subTuple
     */
    VectorSet(const Tuple *contentT, int start = 0, int l = -1);

    /** construct a VectorSet from a TupleBase<Slot>
     */
    VectorSet(TupleBase<Slot> *contentT);
    
    VectorSet(TupleBase<Slot> *contentT, unsigned int size, ...);

    virtual ~VectorSet();

    virtual void check(int x=0) const;

    virtual int order(const Slot& arg) const;
    virtual bool equal(const Slot& arg) const;

    virtual Slot concat(const Slot &arg) const;
    virtual Slot insert(const Slot &val);
    virtual Slot lshift(const Slot &arg) const;
    virtual Slot rshift(const Slot &arg) const; // same meanings as in Namespace
    virtual Slot slice(const Slot &start, const Slot &end) const;
    virtual Slot slice(const Slot &from) const;
    virtual Slot replace(const Slot &from, const Slot &val);
    virtual Slot del(const Slot& from); // delete a single item
    // equivalent to operator[] - see Namespace for derivation
    virtual Slot multiply(const Slot& arg) const;
    // like search
    virtual Slot divide(const Slot& arg) const;
    // iterator
    virtual Slot iterator() const ;    
    // VectorSet is its own iterator
    virtual bool More() const;
    virtual Slot Next();
    virtual bool isIterator() const;
    
    void* operator new( size_t size, void* where=(void*)0) {
        return Memory::operator new(size, where);
    }

    /** C++ extent additional placement new
        @param size size of allocation in bytes
        @param extra extra allocation in bytes
        @param where neighborhood into which to allocate
        @return storage allocated as requested
     */
    void* operator new( size_t size, size_t extra, void* where=(void*)0) {
        return Memory::operator new(size, extra, where);
    }

    /** C++ object deletion
        @param ptr a pointer returned by @ref operator new()
        @param extra extra allocation in bytes
        @param neighborhood into which to allocate
        @return storage allocated as requested
     */
  static void operator delete(void* ptr)
    { return Memory::operator delete(ptr);}

    // hoist
    virtual int length() const {return Segment<Slot>::Length();};
    virtual List *toSequence() const;
    virtual Slot toconstruct() const;
    const Slot* content() const;
    const Slot* content(Slot startcmp, Slot endcmp) const;
    virtual Slot toVectorSet() const;
    virtual Slot search(const Slot& arg) const;
protected:
    virtual int isearch(const Slot& arg) const; // returns +x = match at x, -(x+1) = insert before x (x=length() if greater than end)

};

#endif
