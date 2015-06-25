// NList.cc - Coldstore NTuple Lists
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id $

#ifndef NLIST_HH
#define NLIST_HH

#include <List.hh>

/** dynamic ordinally indexed 2-dimensional ntuples of @ref Slot

    In representation, NList presents as a List of Lists of fixed size.

    NList implements a space-efficient storage for lists of Slot ntuples,
    e.g pairs, triples, etc.

    Each NList has an `arity' property which governs indexing operations.
    NList is really syntactic sugar which peforms index translation.
 */
class NList
    : public List
{
    int _arity;	// the arity of this NList
public:
    
    // Constructors
    /** cold construct an NList
     */
    NList(const Slot &init);

    /** construct an empty NList
     */
    NList(int arity, int size = 0);

    /** share substructure with a List
     */
    NList(int arity, const List *l, int start = 0, int len = -1);

    /** share substructure with an NList
     */
    NList(const NList *l, int start = 0, int len = -1);

    virtual ~NList();

    virtual void check(int=0) const;

    //Slot &List::operator [] (const Data &i) const;

public:
    ///////////////////////////////
    // ColdStore Interface

    // structural
    virtual Data *clone(void *store = (void*)0) const;
    virtual Data *mutate(void *store = (void*)0) const;

    // object
    virtual Slot toconstruct() const;
    static Slot construct(const Slot &arg);

    // sequence
    virtual int length() const;
    virtual Slot concat(const Slot &arg) const;
    virtual Slot slice(const Slot &from, const Slot &len) const;
    virtual Slot slice(const Slot &from) const;
    virtual Slot replace(const Slot &from, const Slot &to, const Slot &value);
    virtual Slot replace(const Slot &from, const Slot &val);
    virtual Slot replace(const Slot &);
    virtual Slot insert(const Slot &from, const Slot &val);
    virtual Slot insert(const Slot &val);
    virtual Slot del(const Slot &from);
    virtual Slot del(const Slot &from, const Slot &len);
    virtual Slot search(const Slot &search) const;
    
    // iterator
    virtual Slot iterator() const;
    
    // additional ops
    Slot qsort();
    Slot reverse();

    /** return the arity of an NList
     */
    int arity() const {
        return _arity;
    }

    /** round the underlying List up to arity
     */
    void roundup();
};

#endif

