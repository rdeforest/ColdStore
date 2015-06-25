// store.hh - low level allocation via BTree
// Copyright (C) 1998,1999 Phillipe Hebrais,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// $Id: store.hh,v 1.1.1.1 2000/04/01 00:00:38 opiate Exp $
#ifndef STORE_H
#define STORE_H

class Node;
struct Found;
struct Edit;
struct Leaf;

struct Leaf
{
    void*  alloc;
    void*  addr;
    size_t size;

    bool operator < (Leaf &l);
};

/** BTree class for extent-based allocation of regions
 */
struct BTree
{
    size_t depth;
    Node*  root;
    void*  free;

    static BTree* tree;	// global root of BTree

    static void lookup( Found* state);
    static void doins(  Edit* state, void* alloc, void* addr, size_t sz);
    static void doadd(  Edit* state, void* alloc, void* addr, size_t sz);
    static void dodel(  Edit* state, void* alloc, void* addr);

    static int  check( size_t depth, Node* root);
    static void Assert( size_t depth, Node* root);
    static int  find( const void* const alloc, void* addr, Leaf* found);
    static void add(  void* alloc, void* addr, size_t size);
    static void del(  void* alloc, void* addr);

    static void  unallocate( void* addr, size_t size);
    static void* allocate( void* alloc, void* addr, size_t size);
};

#ifdef GUARD_ALLOC
/** Guard/wall structure laminated onto each Memory allocation,
    checked on free.
*/
class Guard
{
    friend class Memory;
    size_t magic;		// an arbitrary value
    void *allocator;		// region's allocator
    void *address;		// region's start address
    size_t size;		// region's external size
    size_t checksum;		// checksum of Guard fields
    
public:
    static const size_t Magic = 0x87654321;

    // placement storage allocation is a NOOP
    void *operator new (size_t s, void *address) {return address;}
    void operator delete(void *allocation) {}

    Guard(void *_allocator, void *_address, size_t _size);// Constructor

    bool check();		// check health of this Guard

    bool operator != (Guard &g) {  // inequality between Guards
        return magic != g.magic
            || allocator != g.allocator
            || address != g.address
            || size != g.size
            || checksum != g.checksum;
    }
    
    bool operator == (Guard &g) {	// equality between Guards
        return !(*this != g);
    }
    
    // damage - damage the magic so the next check will fail
    void damage() {magic = ~magic;}
    
    // endGuard - given a pointer to a start Guard, find the endGuard
    Guard *endGuard();
};
#endif
#endif // STORE_H
