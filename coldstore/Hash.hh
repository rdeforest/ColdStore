// Hash.cc - Coldstore Hash table
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id $

#ifndef HASH_HH
#define HASH_HH

#include "List.hh"

/** Hash - a hash table for interned keys.  Intended mainly for internal/system use.

    CAUTION:  While most Hash tables flatten/traverse their keys and run some 
    kind of hash function over them, this one expects interned keys,
    and so it does a very quick comparison on address.

    What this means in practice is that you really can only use a very few types as keys.
    It's intended that Hash be used to hash Symbols allocated from a single Namespace
    to values.  That way, each Symbol is guaranteed unique and the hash value calculated
    identifies the Symbol quite well.

    While this is a surprising and unusual approach, in most cases where a language is
    trying to map Symbol->value, it's already got the Symbol in an interned form.
    In the case of interpreted OO languages, the lookup will occur on potentially
    many tables (each parent's tables) and the expensive flat-hash would have to be
    calculated on each potential collision.  Take it up with QV, anyway :)
    
    For the normal, userHash kind of thing, with nice hash() values calculated slowly,
    recommend you write a child of hash which flattens its arguments (using toconstruct()
    or similar) and then traverses the resultant List to come up with some magic hash values.
    Call it UHash or UserHash or something.
 */
class Hash
  : public List
{
protected:
  int *links;
  int *hashtab;
  int size;

private:
  void init();
  virtual unsigned int data_hash(const Slot &d) const;
  Hash *prepare_to_modify(Hash *hash);
  void insert_key(int i);
  int hsearch(Data &key) const;
  void double_size();

protected:
  Hash(const List &k, int *l, int *h, int s);	// quick constructor
  Slot &key(int i) const;
  Slot &value(int i) const;

public:
    
  // Constructors
  Hash(Hash *hash);

  //Hash(Slot _keys, Slot _values);
  Hash(void);

  /** cold construct an Hash
   */
  Hash(const Slot &init);

  virtual ~Hash();

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
  virtual bool equal(const Slot &hash2) const;
  virtual int order(const Slot &arg) const;
  
  // sequence
  virtual int length() const;
  virtual Slot concat(const Slot &arg) const;	// combine two sequences
  virtual Slot slice(const Slot &from) const;
  virtual Slot replace(const Slot &from, const Slot &val);
  virtual Slot insert(const Slot &from, const Slot &val);
  virtual Slot del(const Slot &from);
  virtual Slot search(const Slot &search) const;
  virtual List *toSequence() const;

  // iterator
  virtual Slot iterator() const;
  
  // Hash locals
  Slot Hash::Keys() const;
};

#endif

