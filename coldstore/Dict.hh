// Dict - ordered mapping of key to value
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef DICT_HH
#define DICT_HH

#include "BTree.hh"
/** pair of objects used for storage in Dict's BigBTree
 */

class SlotPair
    : public Memory
{
    
protected:
    Slot _key;
    mutable Slot _value;

public:
    bool operator == (const SlotPair &p) const;
    bool operator != (const SlotPair &p) const;
    bool operator < (const SlotPair &p) const;
    bool operator <= (const SlotPair &p) const;
    bool operator >= (const SlotPair &p) const;
    bool operator > (const SlotPair &p) const;
    int order(const SlotPair *arg) const;
    
    // Constructors
    
    // used to initialize to NULL - UGLY
    SlotPair()
        : _key((Data*)0), _value((Data*)0) {}

    // a valueless pair - used to search
    SlotPair(const Slot &k)
        : _key(k), _value((Data*)0) {}

    SlotPair(const SlotPair &p)	// Copy Constructor
        : _key(p._key), _value(p._value) {}

    SlotPair(const Slot &k, const Slot &v)
        : _key(k), _value(v) {}

    const SlotPair &operator =(const SlotPair &p);
    const int operator =(const int i);

    // accessors
    Slot &key() {
        return _key;
    }

    Slot &value() {
        return _value;
    }
};

class SlotPair;	// opaque type for storage of Slot pairs in Dict

class Dict
  : public Data
{
private:
    /** unimplemented
     */
    virtual Slot replace(const Slot &value);

protected:
    virtual ~Dict();
    friend class DictIt;
    BigBTree<SlotPair> tree;

public:
    /** construct a Dict of a given order
     * @param order order of each BigBTree node
     */
    Dict(/*int order=10*/);

    /** construct a Dict from an even List of key value pairs
     *
     * Note that the List must be even length, and the members 
     * are treated pairwise.
     *  {key val key val ...} not {{key val} {key val}...}
     * @param pairs List of key value pairs
     */
    Dict(const Slot &pairs);	// cold construct a Dict

    /** construct a Dict of a given order from an even List of key value pairs
     *
     * Not terribly useful, just a wrinkle, slated for removal.
     *
     * Note that the List must be even length, and the members 
     * are treated pairwise.
     *  {key val key val ...} not {{key val} {key val}...}
     * @param pairs List of key value pairs
     */
  //Dict(const Slot &pairs/*,int order*/);

    /** copy construct a Dict
     *
     * @param o dict to copy
     * @param order order of created dict
     */
    Dict(const Dict &o/*,int order=10*/);

    /** copy constructor @ref Data::clone
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual Data *clone(void *where = (void*)0)const;	// Uses the copy constructor

    /** create a mutable copy @ref Data::mutate
     * @param where the locale into which to clone this object
     * @return mutable copy of object     
     */
    virtual Data *mutate(void *where = (void*)0) const;

    /** Dictionary's truth value
     *
     * We hold that a dictionary is `true' if it has content
     * @return the object's truth value
     */    
    virtual bool truth() const;

    /** generate constructor arguments sufficient to recreate this
     * @return a primitive type (pickled) representation of this object
     */
    virtual Slot toconstruct() const;

    /** order two Dicts
     *
     * Dicts compare as their flattened ordered association list.
     * @param arg object to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual int order(const Slot &arg) const;


    /** equality operator
     * Dicts compare for equality as their flattened ordered association list.
     * @param arg object to be compared to this
     * @return true/false depending on equality
     */
    virtual bool equal(const Slot &arg) const;	// depending on equality
    
    /** length of Dict
     *
     * A Dict's length is the number of associations it contains
     */
    virtual int length() const;

    /** concatenate two Dicts, or a Dict and an association list
     *
     * All associations in the argument are added to this.
     * Associations in both do not replace those in this
     * ... perhaps they should?
     *
     * @param arg Dict or association list
     * @return a Dictionary containing the merge of this and arg
     */    
    virtual Slot concat(const Slot &arg) const;

    /** return a range of associations
     *
     * If no associations exist in the nominated range,
     * an empty list is returned.
     * @param from key of beginning of association range
     * @param to key of end of association range
     * @return an association list containing associations in from..to
     */    
    virtual Slot slice(const Slot &from, const Slot &to) const;


    /** return the value of an association
     * @param key the association key to be searched for
     * @return the value of any association found
     * @exceptions ~dictnf
     */
    virtual Slot slice(const Slot &key) const;

    /** replace a range of keys with values in a sequence
     * @param from slice-range start key
     * @param to slice-range end key
     * @param value sequence of values to be replaced in this key range
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &from, const Slot &to, const Slot &value);

    /** replace an association's value
     * @param key key of association to be replaced
     * @param value value to be stored
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &key, const Slot &value);

    /** search for an association, returning NULL if it's not present
     *
     * search differs from @ref Dict::slice() in two ways:
     * @li search doesn't throw ~dictnf if the association's absent
     * @li search returns the value, whereas slice returns the association
     * (ie: key,value pair)
     * 
     * search could, should, (but doesn't) return the element less than
     * the requested element, because this isn't possible in the 
     * BigBTree implementation chosen.
     *
     * @param search value to be found
     * @return value of association found, or NULL
     */
    virtual Slot search(const Slot &search) const;

    /** an iterator over this sequence
     * @return a DictIt over this
     */
    virtual Slot iterator() const;	// return an iterator
    
    /** insert a value into this
     *
     * insert differs from @ref Dict::replace in that it will create a 
     * new association if one with a matching key does not exist.
     *
     * @param key key of association to be inserted in this sequence
     * @param value value of association to be inserted
     * @return a copy of this with the insertion performed
     */
    virtual Slot insert(const Slot &key, const Slot &value);

    /** insert a (key,NULL) into this
     *
     * insert differs from @ref Dict::replace in that it will create a 
     * new association if one with a matching key does not exist.
     *
     * @param key key of association to be inserted in this sequence
     * @return a copy of this with the insertion performed
     */
    virtual Slot insert(const Slot &key);

    /** delete a subrange from Dict
     *
     * All associations from <= key <= to are removed
     * @param from slice-range start key
     * @param to slice-range end key
     * @return a copy of this with the deletion performed
     */
    virtual Slot del(const Slot &from, const Slot &to);

    /** delete an association from Dict
     *
     * Throws ~dictnf if association is not present.
     *
     * To remove a possibly absent element, one could use a range 
     * whose terminals are identical.
     * @param key key of association to be deleted
     * @return a copy of this with the deletion performed
     */
    virtual Slot del(const Slot &key);
    
    /** make an association list from this
     * @return an even length List whose even elements are keys,
     * and whose odd elements are their corresponding values.
     */
    virtual List *toSequence() const;
    virtual bool isMap() const {return true;};

protected:
    /** add the association between a pair of slots
     * @param key which the associated value may be found
     * @param value value associated with key
     * @return true if the addition was successful
     */
    bool Add(const Slot &key, const Slot &value);

    /** find a partial association with key
     *
     * This is used to test the presence of an association 
     * independantly of setting its value, as for example when
     * one wishes to add an association only if it does not clash
     *
     * @param key for which to search
     * @return opaque SlotPair uniquely identifying the association
     */
    SlotPair *Find(const Slot &key) const;

    /** find an association with key, throw if no match
     *
     * This is used to test the presence of an association 
     * independantly of setting its value, as for example when
     * one wishes to add an association only if it does not clash
     *
     * @param key for which to search
     * @return opaque SlotPair uniquely identifying the association
     */
    SlotPair *FindEQ(const Slot &key) const;

    /** replace the value of an association 
     *
     * @param entry returned by a @ref Dict::Find
     * @param replacement value value for association
     */
    void Replace(SlotPair *entry, const Slot &value);

    /** find the association with the given key and pass back its value
     *
     * Will only replace an existing association, use @ref Dict::Add
     * to create a new association.
     *
     * @param key for which to search
     * @param replacement value value for association
     * @return true iff the replacement was successful
     */
    bool FindV(const Slot &key, Slot &value) const;
};

class DictIt
    : public Data,
      public BigBTreeIterator<SlotPair>
{
    Slot curr;		// last result from Next()
    
protected:
    friend class Dict;
    virtual ~DictIt();
    Slot dict;		// dict from which this is constructed
    Slot min;		// the place to start (STUB)
    Slot max;		// the place to end (STUB)
    
public:
    DictIt(const Slot &init);	// cold construct a DictIt
    DictIt(const Dict *o);
    DictIt(const Dict &o);
    DictIt(const DictIt &oit);

    void *operator new(size_t size, void *allocator=(void*)0) {
        return Data::operator new(size, allocator);
    }
    void operator delete(void *allocation) {
        Data::operator delete(allocation);
    }
    
    // structural
    virtual Data *clone(void *where = (void*)0)const;	// Uses the copy constructor
    virtual Data *mutate(void *where = (void*)0) const;	// returns a mutable copy of this
    
    // object
    virtual bool truth() const;		// the object's truth value
    virtual Slot toconstruct() const;		// constructor args to recreate object
    
    static Slot construct(const Slot &arg);	// construct from constructor args
    virtual int order(const Slot &arg) const;	// 1,0,-1 depending on order
    virtual bool equal(const Slot &arg) const;	// depending on equality
    
    // iterator
    virtual bool isIterator() const;			// predicate - is object an iterator
    virtual bool More() const;
    virtual Slot Next();
};

#endif
