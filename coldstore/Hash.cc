// Hash.cc - Coldstore Hashed collections
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Hash.cc,v 1.16 2001/10/29 09:16:43 coldstore Exp $";

//#define DEBUGLOG
#include "Data.hh"
#include "Store.hh"
#include "Integer.hh"
#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"
#include "Hash.hh"

#include "tSlot.th"
template union tSlot<Hash>;

#define MALLOC_DELTA			 5
#define HASHTAB_STARTING_SIZE		(32 - MALLOC_DELTA)

Slot &Hash::key(int i) const {
    return (*this)[i*2];
}

Slot &Hash::value(int i) const {
    return (*this)[i*2+1];
}

// hardline hash function - it's the address!!  Get it?
inline unsigned int Hash::data_hash(const Slot &d) const
{
  if (d.isEncInt() || dynamic_cast<Integer*>((Data*)d)) {
    DEBLOG(cerr << "data_hash enc int: " << d << '\n');
    return (int)d;
  } else {
    DEBLOG(cerr << "data_hash: " << d << '\n');
    return (int)(Data*)d;
  }
}

void Hash::insert_key(int i)
{
  int ind;

  ind = data_hash(key(i)) % size;
  links[i] = hashtab[ind];
  hashtab[ind] = i;
  DEBLOG(cerr << "insert_key: links[" << i 
         << "] <- " << hashtab[ind] <<'\n');
  DEBLOG(cerr << "insert_key: hashtab[" << ind
         << "] <- " << i <<'\n');
}

int Hash::hsearch(Data &_key) const
{
  int ind, i;
    
  ind = data_hash(_key) % size;
  DEBLOG(cerr << "hsearch: Looking for " << ind << ". ");
  for (i = hashtab[ind]; i != -1; i = links[i]) {
    DEBLOG(cerr << "at "<< i << ": " << _key << "/" << key(i));
    if (key(i)->equal(_key)) {
      DEBLOG(cerr << " EQUAL\n");
      return i;
    }
  }
  DEBLOG(cerr << "NOT EQUAL\n");

  return -1;
}

// dyadic `search', matching subrange
Slot Hash::search(const Slot &what) const
{
  int pos = hsearch(what);
  if (pos == -1)
    return (Data*)0;
  else {
    return value(pos);
  }
}

void Hash::double_size()
{
  int oldsize = size;
  size = size * 2 + MALLOC_DELTA;
    
  int *oldlinks = links;
  links = new int[size];
  memcpy(links, oldlinks, oldsize * sizeof(int));
  delete [] oldlinks;
    
  int *oldhashtab = hashtab;
  hashtab = new int[size];
  memcpy(hashtab, oldhashtab, oldsize * sizeof(int));
  delete [] oldhashtab;

  for (int i = 0; i < size; i++) {
    links[i] = -1;
    hashtab[i] = -1;
  }
  for (int i = 0; i < length(); i++)
    insert_key(i);
}

void Hash::init()
{
  // make sure the Lists are refcounted
  //keys->upcount();
  //values->upcount();
  /* Calculate initial size of chain and hash table. */
  size = HASHTAB_STARTING_SIZE;
  while (size < length())
    size = size * 2 + MALLOC_DELTA;

  /* Initialize chain entries and hash table. */
  links = new int[size];
  hashtab = new int[size];
  for (int i = 0; i < size; i++) {
    links[i] = -1;
    hashtab[i] = -1;
  }

  /* Insert the keys into the hash table, eliminating duplicates. */
  int i = 0;
  while (i < length()) {
    if (hsearch(key(i)) == -1) {
      insert_key(i++);
    } else {
      List::del(i*2,2);
    }
  }
}

Hash::Hash(Hash *hash)
  :List((List *)hash)
{
  init();
}

Hash::~Hash()
{
  delete [] links;
  delete [] hashtab;
}

#if 0
Hash::Hash(Slot _keys, Slot _values)
  : keys(_keys), values(_values)
{
  if (keys->length() != values->length()) {
    throw new Error("range", keys, "key and value lists differ in size");
  }
  init();
}
#endif

Hash::Hash(void)
{
  init();
}

Hash::Hash(const Slot &arg)
{
  DEBLOG(cerr << "Hash "); DEBLOG(arg->dump(cerr) << '\n');
  Slot seq = arg->toSequence();
  int l = seq->length();
  DEBLOG(seq->dump(cerr) << " toSeq\n");
  
  if (l%2 != 0) {
    throw new Error("range", seq, "Hash constructor sequence uneven");
  }
  for (int i = 0; i < l; i++) {
      push(seq[i]);
  }
  DEBLOG(cerr << "Hash " << dump(cerr)
         << '\n');
  init();    /* Slices were all valid; return new hash. */
}

Slot Hash::replace(const Slot &key, const Slot &value)
{
  /* Just replace the value for the key if it already exists. */
  int pos = hsearch(key);
  if (pos != -1) {
    Hash *hash = (Hash*)mutate();
    Slot protect(hash);
    hash->List::replace(pos*2+1, value);
    return hash;
  } else {
    throw new Error("dictnf", key, "key not found");
  }
}

Slot Hash::insert(const Slot &key, const Slot &value)
{
  Hash *hash = (Hash*)mutate();
  Slot protect(hash);

  /* Just replace the value for the key if it already exists. */
  int pos = hash->hsearch(key);
  if (pos != -1) {
    hash->List::replace(pos*2+1, value);
    return hash;
  }

  /* Add the key and value to the list. */
  hash->List::insert(key);
  hash->List::insert(value);
  
  /* Check if we should resize the hash table. */
  if (hash->length() >= hash->size)
    hash->double_size();
  else
    hash->insert_key(hash->length()-1);
  
  return hash;
}

/* Error-checking is the caller's responsibility; this routine assumes that it
 * will find the key in the hashionary. */
Slot Hash::del(const Slot &key)
{
    Hash *hash = (Hash*)mutate();
    Slot protect(hash);
    
    /* Search for a pointer to the key, either in the hash table entry or in
     * the chain links. */
    int *ip, i = -1;
    int ind = data_hash(key) % hash->size;
    for (ip = &hash->hashtab[ind];; ip = &hash->links[*ip]) {
      i = *ip;
      if (hash->key(i)->equal(key))
        break;
    }

    /* Delete the element from the keys and values lists. */
    hash->List::del(i*2,2);

    /* Replace the pointer to the key index with the next link. */
    *ip = hash->links[i];

    /* Copy the links beyond i backward. */
    memmove(hash->links + i, hash->links + i + 1, hash->length() - i);
    hash->links[hash->length()] = -1;

    /* Since we've renumbered all the elements beyond i, we have to check
     * all the links and hash table entries.  If they're greater than i,
     * decrement them.  Skip this step if the element we removed was the last
     * one. */
    if (i < hash->length()) {
      for (int j = 0; j < hash->length(); j++) {
        if (hash->links[j] > i)
          hash->links[j]--;
      }
      for (int j = 0; j < hash->size; j++) {
        if (hash->hashtab[j] > i)
          hash->hashtab[j]--;
      }
    }

    return hash;
}

Slot Hash::slice(const Slot &_key) const
{
  int pos = hsearch(_key);
  if (pos == -1)
    throw new Error("dictnf", _key, "key not found");
  return new (2) Tuple(-2, (Data*)key(pos), (Data*)value(pos));
}

/////////////////////////////////////
// map

// return map as sequence
List *Hash::toSequence() const
{
  DEBLOG(cerr << "Hash toSequence " << dump(cerr)
         << '\n');

  // a range of items
  return new List(this);
}

Slot Hash::Keys() const
{
    return new List(this);
}

int Hash::length() const
{
    return List::length()/2;
}

bool Hash::equal(const Slot &arg) const
{
  Hash *hash2 = dynamic_cast<Hash*>((Data*)arg);
  if (!hash2 || !List::equal((List*)hash2))
    return false;
  else
    return true;
}

int Hash::order(const Slot &arg) const
{
    int cmp = typeOrder(arg);
    if (cmp)
      return cmp;

    Hash *hash2 = dynamic_cast<Hash*>((Data*)arg);
    return List::order((List*)hash2);
}

void Hash::check(int i) const
{
    //FIXME: there's probably something we could do here
}

Data *Hash::mutate(void *where) const
{
  if (refcount() > 1)
    return clone(where);
  else
    return (Data*)this;
}

Data *Hash::clone(void *) const
{
  return new Hash(this);
}

Slot Hash::toconstruct(void) const
{
  return toSequence();
}

Slot Hash::concat(Slot const &other) const
{
  Data *result = clone();

  Slot seq = other->toSequence();
  int l = seq->length();
    
  if (l%2 != 0) {
    throw new Error("range", other, "Hash constructor sequence uneven");
  }
  for (int i = 0; i < l; i+=2) {
    result = result->insert(seq[i], seq[i+1]);
  }

  return result;
}

// return an iterator
Slot Hash::iterator() const
{
    List *result = new List(length());
    Slot protect(result);
    for (int i = 0; i < length(); i++) {
        result->push(new (2) Tuple(-2, (Data*)key(i), (Data*)value(i)));
    }
    return result;
}
