// store.hh - low level allocation via BTree
// Copyright (C) 1998,1999 Phillipe Hebrais,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: store.cc,v 1.1 2000/07/30 02:49:35 coldstore Exp $";

#include <assert.h>
#include <string.h>
#include <iostream.h>
#include <stdexcept>
#include "qvmm.h"
#include "store.hh"

struct Node;

bool Leaf::operator < (Leaf &l) {
    if ((unsigned)alloc > (unsigned)(l.alloc))
        return false;
    else if ((addr < l.addr) && (((char*)addr + size) <= l.addr))
        return true;
    else
        return false;
}


struct Branch
{
    void* alloc;
    void* addr;
    union { size_t leaf; Node* internal; } value;

    bool operator < (Branch &b) {
        if (!(b.addr))
            return true;
        else if ((unsigned)alloc < (unsigned)(b.alloc))
            return true;
        else if (addr < b.addr)
            return true;
        else
            return false;
    }
};

class Node
{
private:
    size_t size;

public:
    static const size_t branches = ((Mmap::page_size - sizeof(size_t))
                                    / sizeof(Branch));

    Branch branch[branches];

    size_t getSize() {
        assert(size <= branches); return size;
    }
    void setSize(size_t s);
    size_t &refSize() {return size;}

    void* operator new( size_t size);
    void  operator delete( void* ptr);

    bool operator < (Node &n) {
        return branch[size-1] < n.branch[0];
    }

    void check();
};

void Node::setSize(size_t s) {
    assert(s <= branches);
    size = s;
}

void Node::check()
{
    assert(Mmap::check(this));
    assert(size && (size <= branches));
    for (size_t i = 0; i < (size - 1); i++) {
        assert(branch[i] < branch[i+1]);
    }
}


struct Found
{
    const void* alloc;
    void*  addr;
    struct Nodes {
        Node*  node;
        size_t index;
    } node[2];

    void check();
};

void Found::check()
{
    if (node[0].node)
        node[0].node->check();
    if (node[1].node)
        node[1].node->check();
}

struct Edit
{
    size_t depth, size;
    Branch branch[3];

    void check() {
        assert(size <= 3);
    }
};

/** allocate a new BTree Node

   Free Tree nodes are chained in a list called `free'.
   When the list is empty, we allocate a page from Mmap,
   produce as many nodes as possible from it, and chain them
   to the free list.
*/
void* Node::operator new( size_t size)
{
    Node* ptr;
    int n;

    assert( size == sizeof(Node));
    if (!BTree::tree->free) {
        ptr = (Node*) Mmap::alloc( Mmap::page_size);
        if (!ptr) throw runtime_error("Mmap::alloc returned NULL");
        n = Mmap::page_size / size;
        while (n--) {
            *(void**)(ptr+n) = BTree::tree->free;
            BTree::tree->free = (void*)(ptr+n);
        }
    }
    assert( BTree::tree->free);
    ptr = (Node*)BTree::tree->free;
    BTree::tree->free = *(void**)BTree::tree->free;
    return ptr;
}


void Node::operator delete( void* ptr)
{
    if (!ptr) return;
    *(void**)ptr = BTree::tree->free;
    BTree::tree->free = ptr;
}

/**
   lookup( state)

   Search for (alloc,addr) in state[0].node.
   Set state[0] to refer to the right most entry less than or equal to
   (alloc,addr).  Set state[1] to refer to the left most entry greater
   than (alloc,addr).

   lookup ignores the initial content of state[1].node, unless the matched
   entry is the right most entry of state[0].node.

   @param state state[0].node: node to search in state[1].node: right-sibling of state[0].node
   
   output:
   state[0].node->branch[state[0].index]: found low
   state[1].node->branch[state[1].index]: found high
   
   notes:
   it is an error to look for a key to the left of the first entry.
*/
void BTree::lookup( Found* state)
{
    Node*  node = state->node[0].node;
    size_t low, mid, high;

#ifdef BTREE_INTEGRITY
    node->check();
    Memory::Assert();
#endif

    // invalid lookup
    assert( (state->alloc == node->branch[0].alloc)
            ? state->addr >= node->branch[0].addr
            : state->alloc >= node->branch[0].alloc);

    // divide and conquer
    for (low=0,high=node->getSize(); high>low+1; ) {
        mid = (high + low) / 2;
        if (state->alloc < node->branch[mid].alloc) high = mid;
        else if (state->alloc > node->branch[mid].alloc) low = mid;
        else if (state->addr < node->branch[mid].addr) high = mid;
        else low = mid;
    }
    // low and high refer to the matched nodes

    // write back the state
    state->node[0].index = low;
    if (high == node->getSize())
        state->node[1].index = 0; // state->node[1].node is already set (or 0)
    else {
        state->node[1].node = node;
        state->node[1].index = high;
    }

#ifdef BTREE_INTEGRITY
    state->check();
    Memory::Assert();
    //assert(state->node[0] < state->node[1]);
#endif
}

//////////////////////////////////////////////////////////////////////
// doins( state, alloc, addr, size)
//
// Insert before the first entry in the tree.
//
// intput:
//  alloc,addr,size: key and value of entry to add.
//  state->depth: depth of edit-node tree (as if Edit was a node)
//  state->size:  1 or 2.
//  state->branch[0..1]: search result (we add to branch[0]).
//
// output:
//  state->size: original_state->size or original_state->size+1
//  state->branch[0..1]: edited subtree
//
/////////////////////////////////////////////////////////////////////
void BTree::doins( Edit* state, void* alloc, void* addr, size_t size)
{
    Edit reg;

#ifdef BTREE_INTEGRITY
    state->check();
    Memory::Assert();
#endif

    // invalid insertion
    assert( state->size == 1 );
    assert( (alloc == state->branch[0].alloc)
            ? addr < state->branch[0].addr
            : alloc < state->branch[0].alloc);

    // inserting in a leaf is easy
    if (!state->depth) {
        state->branch[1] = state->branch[0];
        state->branch[0].alloc = alloc;
        state->branch[0].addr  = addr;
        state->branch[0].value.leaf = size;
        state->size = 2;

#ifdef BTREE_INTEGRITY
        //assert(state->branch[0] < state->branch[1]);
        state->check();
        Memory::Assert();
#endif

        return;
    }

    // insert in the first node
    reg.depth = state->depth - 1;
    reg.size = 1;
    reg.branch[0] = state->branch[0].value.internal->branch[0];
    doins( &reg, alloc, addr, size);

#ifdef BTREE_INTEGRITY
    state->check();
    Memory::Assert();
#endif

    // replace the first node
    -- reg.size;
    state->branch[0].value.internal->branch[0]
        = reg.branch[reg.size];
    //assert(state->branch[0] < state->branch[1]);

    if (!reg.size)
        /* nothing */;

    // there is room on this node
    else if (state->branch[0].value.internal->getSize() < Node::branches) {
        memmove( state->branch[0].value.internal->branch + 1,
                 state->branch[0].value.internal->branch,
                 state->branch[0].value.internal->getSize() * sizeof(Branch));
        state->branch[0].value.internal->branch[0] = reg.branch[0];
        ++ state->branch[0].value.internal->refSize();

        // split in two
    } else {
        Node* node = new Node;
        if (!node) throw runtime_error("Can't create Node __FILE__ __LINE__");
        node->setSize((Node::branches + 1) / 2);
        state->branch[0].value.internal->setSize(Node::branches / 2 + 1);
        memmove( node->branch,
                 state->branch[0].value.internal->branch + Node::branches/2,
                 node->getSize() * sizeof(Branch));
        memmove( state->branch[0].value.internal->branch + 1,
                 state->branch[0].value.internal->branch,
                 state->branch[0].value.internal->getSize() * sizeof(Branch));
        state->branch[0].value.internal->branch[0] = reg.branch[0];

#ifdef BTREE_INTEGRITY
        node->check();
        Memory::Assert();
#endif

        state->branch[1].value.internal = node;
        state->size = 2;
    }

    // refresh state
    if (state->size == 2) {
        state->branch[1].alloc
            = state->branch[1].value.internal->branch[0].alloc;
        state->branch[1].addr
            = state->branch[1].value.internal->branch[0].addr;
    }
    state->branch[0].alloc = state->branch[0].value.internal->branch[0].alloc;
    state->branch[0].addr  = state->branch[0].value.internal->branch[0].addr;

#ifdef BTREE_INTEGRITY
    state->check();
    Memory::Assert();
    //assert(state->branch[0] < state->branch[1]);
#endif
}

/////////////////////////////////////////////////////////////////////
// doadd( state, alloc, addr, size)
//
// Add (or update) an entry in the tree.
//
// intput:
//  alloc,addr,size: key and value of entry to add.
//  state->depth: depth of edit-node tree (as if Edit was a node)
//  state->size:  1 or 2.
//  state->branch[0..1]: search result (we add to branch[0]).
//
// output:
//  state->size: original_state->size or original_state->size+1
//  state->branch[0..1]: edited subtree
//
// note:
//  adding before the first key will not work.
/////////////////////////////////////////////////////////////////////
void BTree::doadd( Edit* state, void* alloc, void* addr, size_t size)
{
    Found found;
    Edit  reg;

    // invalid addition
    assert( state->size == 1 || state->size == 2);
    assert( (alloc == state->branch[0].alloc)
            ? addr >= state->branch[0].addr
            : alloc > state->branch[0].alloc);

    // adding to a leaf is easy
    if (!state->depth) {
        if (alloc == state->branch[0].alloc
            && addr == state->branch[0].addr) {
            state->branch[0].value.leaf = size;
            return;
        }
        if (state->size == 2) state->branch[2] = state->branch[1];
        state->branch[1].alloc = alloc;
        state->branch[1].addr  = addr;
        state->branch[1].value.leaf = size;
        //assert(state->branch[0] < state->branch[1]);
        //if (state->size == 2) assert(state->branch[1] < state->branch[2]);
        ++ state->size;
        return;
    }

    // search for a suitable subtree
    found.alloc = alloc;
    found.addr  = addr;
    found.node[0].node = state->branch[0].value.internal;
    if (state->size == 1) found.node[1].node = 0;
    else found.node[1].node = state->branch[1].value.internal;
    lookup( &found);

    // add to the subtree
    reg.depth = state->depth - 1;
    reg.size = 1;
    reg.branch[0] = found.node[0].node->branch[found.node[0].index];
    if (found.node[1].node) {
        reg.size = 2;
        reg.branch[1] = found.node[1].node->branch[found.node[1].index];
    }
    doadd( &reg, alloc, addr, size);

#if 0
    if (reg.size >= 2)
        assert(reg.branch[0] < reg.branch[1]);
    if (reg.size >= 3)
        assert(reg.branch[1] < reg.branch[2]);
#endif

    // replace the edit-node into the subtree

    // replace the sibling
    if (found.node[1].node) {
        -- reg.size;
        found.node[1].node->branch[found.node[1].index] = reg.branch[reg.size];
    }

    // replace the matched entry
    -- reg.size;
    found.node[0].node->branch[found.node[0].index] = reg.branch[reg.size];

    if (!reg.size)
        /* nothing */;

    // simple case: the new entry fits
    else if (found.node[0].node->getSize() < Node::branches) {
        memmove( found.node[0].node->branch + found.node[0].index + 1,
                 found.node[0].node->branch + found.node[0].index,
                 (found.node[0].node->getSize() - found.node[0].index)
                 * sizeof(Branch));
        found.node[0].node->branch[found.node[0].index] = reg.branch[0];
        ++ found.node[0].node->refSize();

#ifdef BTREE_INTEGRITY
        found.check();
        Memory::Assert();
#endif

        // no node to overflow to, split in two
    } else if (state->size == 1) {
        Node* node = new Node;
        if (!node) throw runtime_error("Can't create Node __FILE__ __LINE__");
        DEBLOG(cerr << "Node::branches: " << Node::branches
             << " found.node[0].index: " << found.node[0].index
             << '\n');

        node->setSize(Node::branches - found.node[0].index);
        memmove( node->branch,
                 found.node[0].node->branch + found.node[0].index,
                 node->getSize() * sizeof(Branch));
        found.node[0].node->branch[found.node[0].index] = reg.branch[0];
        found.node[0].node->setSize(found.node[0].index + 1);
        state->branch[1].value.internal = node;
        state->size = 2;

#ifdef BTREE_INTEGRITY
        node->check();
        //assert (found.node[0].node->branch[found.node[0].index] < found.node[0].node->branch[found.node[0].index + 1]);
#endif

        // sibling has room, rebalance and then add
    } else if (state->branch[1].value.internal->getSize() < Node::branches) {
        size_t move = (Node::branches + state->branch[1].value.internal->getSize()) / 2 + 1;
        move = (found.node[0].index<move) ? Node::branches-move+1 : Node::branches-move;
        memmove( state->branch[1].value.internal->branch + move,
                 state->branch[1].value.internal->branch,
                 state->branch[1].value.internal->getSize() * sizeof(Branch));
        memmove( state->branch[1].value.internal->branch,
                 found.node[0].node->branch + Node::branches - move,
                 move * sizeof(Branch));
        found.node[0].node->refSize() -= move;
        state->branch[1].value.internal->refSize() += move;
        if (found.node[0].index > found.node[0].node->getSize()) {
            found.node[0].index -= found.node[0].node->getSize();
            found.node[0].node = state->branch[1].value.internal;
        }

        move = found.node[0].node->getSize() - found.node[0].index;
        memmove( found.node[0].node->branch + found.node[0].index + 1,
                 found.node[0].node->branch + found.node[0].index,
                 move * sizeof(Branch));
        found.node[0].node->branch[found.node[0].index] = reg.branch[0];
        ++ found.node[0].node->refSize();

#ifdef BTREE_INTEGRITY
        found.check();
#endif

        // allocate a third node and redistribute the entries equally
    } else {
        size_t sz = (2 * Node::branches + 1) / 3;
        Node* node = new Node;
        if (!node) throw runtime_error("Can't create Node __FILE__ __LINE__");
        node->setSize(2 * (Node::branches - sz));
        state->branch[1].value.internal->refSize() -= node->getSize();
        memmove( node->branch,
                 (state->branch[1].value.internal->branch
                  + state->branch[1].value.internal->getSize()),
                 node->getSize() * sizeof(Branch));
        memmove( (state->branch[1].value.internal->branch
                  + sz - state->branch[1].value.internal->getSize()),
                 state->branch[1].value.internal->branch,
                 state->branch[1].value.internal->getSize() * sizeof(Branch));
        memmove( state->branch[1].value.internal->branch,
                 found.node[0].node->branch + sz,
                 (Node::branches - sz) * sizeof(Branch));
        found.node[0].node->setSize(sz);
        state->branch[1].value.internal->setSize(sz);
        state->size = 3;
        state->branch[2].value.internal = node;
        if (found.node[0].index <= found.node[0].node->getSize())
            node = found.node[0].node;
        else if ((found.node[0].index -= found.node[0].node->getSize())
                 <= state->branch[1].value.internal->getSize())
            node = state->branch[1].value.internal;
        else
            found.node[0].index -= state->branch[1].value.internal->getSize();
        memmove( node->branch + found.node[0].index + 1,
                 node->branch + found.node[0].index,
                 (node->getSize() - found.node[0].index) * sizeof(Branch));
        node->branch[found.node[0].index] = reg.branch[0];
        ++ node->refSize();

#ifdef BTREE_INTEGRITY
        node->check();
#endif
    }

    // This would be a good central place to install an allocator pointer in
    // the leaf/allocated region.

    // refresh the state
    if (state->size == 3) {
        state->branch[2].alloc
            = state->branch[2].value.internal->branch[0].alloc;
        state->branch[2].addr
            = state->branch[2].value.internal->branch[0].addr;
    }
    if (state->size >= 2) {
        state->branch[1].alloc
            = state->branch[1].value.internal->branch[0].alloc;
        state->branch[1].addr
            = state->branch[1].value.internal->branch[0].addr;
    }
    state->branch[0].alloc = state->branch[0].value.internal->branch[0].alloc;
    state->branch[0].addr  = state->branch[0].value.internal->branch[0].addr;
}


////////////////////////////////////////////////////////////////
// dodel( state, alloc, addr);
//
// Remove the entry (if any) matching the key (alloc,addr).
//
// input:
//  alloc, addr: key for the search.
//  state->depth: depth of edit-node tree (as if Edit was a node)
//  state->size:  1 or 2.
//  state->branch[0..1]: search result (we delete from branch[0]).
//
// output:
//  state->size: original_state->size or original_state->size-1
//  state->branch[0..1]: edited suBTree
//
// note:
//  deleting before the first key will not work (lets be positive now:
//  deleting before the first key will crash).
////////////////////////////////////////////////////////////////
void BTree::dodel( Edit* state, void* alloc, void* addr)
{
    Found found;
    Edit  reg;

    // invalid deletion
    assert( state->size == 1 || state->size == 2);
    assert( (alloc == state->branch[0].alloc)
            ? addr >= state->branch[0].addr
            : alloc > state->branch[0].alloc);

    // deleting from a leaf is easy
    if (!state->depth) {
        if (alloc != state->branch[0].alloc) return;
        if (addr  != state->branch[0].addr) return;
        if (state->size == 2) state->branch[0] = state->branch[1];
        -- state->size;
        return;
    }

    // Look for an appropriate subtree
    found.alloc = alloc;
    found.addr  = addr;
    found.node[0].node = state->branch[0].value.internal;
    if (state->size == 1) found.node[1].node = 0;
    else found.node[1].node = state->branch[1].value.internal;
    lookup( &found);

#ifdef BTREE_INTEGRITY
    found.check();
#endif

    // perform dodel() in that subtree
    reg.depth = state->depth - 1;
    reg.size = 1;
    reg.branch[0] = found.node[0].node->branch[found.node[0].index];
    if (found.node[1].node) {
        reg.size = 2;
        reg.branch[1] = found.node[1].node->branch[found.node[1].index];
    }
    dodel( &reg, alloc, addr);
    // ok, replace this edited subtree into my own subtree

    // start by replacing found.node[1] with the right-most branch
    if (found.node[1].node) {
        -- reg.size;
        found.node[1].node->branch[found.node[1].index] = reg.branch[reg.size];
    }

    // we might not really have to delete anything
    if (reg.size) {
        found.node[0].node->branch[found.node[0].index] = reg.branch[0];

        // the simple case: we just move all the entries down
    } else if (state->size == 1
               || found.node[0].node->getSize() > Node::branches/2) {
        -- found.node[0].node->refSize();
        memmove( found.node[0].node->branch + found.node[0].index,
                 found.node[0].node->branch + found.node[0].index + 1,
                 (found.node[0].node->getSize()
                  - found.node[0].index)*sizeof(Branch));

        // if this was the last entry, free the root
        if (!found.node[0].node->getSize()) {
            assert( state->size == 1);
            delete found.node[0].node;
            state->size = 0;
            return;
        }

        // merge the two nodes into one
    } else if (found.node[0].node->getSize()
               + state->branch[1].value.internal->getSize()
               <= Node::branches + 1) {
        -- found.node[0].node->refSize();
        memmove( found.node[0].node->branch + found.node[0].index,
                 found.node[0].node->branch + found.node[0].index + 1,
                 (found.node[0].node->getSize() - found.node[0].index)
                 *sizeof(Branch));
        memmove( found.node[0].node->branch + found.node[0].node->getSize(),
                 state->branch[1].value.internal->branch,
                 state->branch[1].value.internal->getSize() * sizeof(Branch));
        found.node[0].node->refSize()
            += state->branch[1].value.internal->getSize();
        delete state->branch[1].value.internal;
        state->size = 1;

#ifdef BTREE_INTEGRITY
        found.check();
#endif

        // rebalance the two nodes
    } else {
        size_t move = (state->branch[1].value.internal->getSize()
                       - found.node[0].node->getSize()) / 2 + 1;
        assert( move);
        -- found.node[0].node->refSize();
        memmove( found.node[0].node->branch + found.node[0].index,
                 found.node[0].node->branch + found.node[0].index + 1,
                 (found.node[0].node->getSize() - found.node[0].index)
                 *sizeof(Branch));
        memmove( found.node[0].node->branch + found.node[0].node->getSize(),
                 state->branch[1].value.internal->branch,
                 move * sizeof(Branch));
        found.node[0].node->refSize() += move;
        state->branch[1].value.internal->refSize() -= move;
        memmove( state->branch[1].value.internal->branch,
                 state->branch[1].value.internal->branch + move,
                 state->branch[1].value.internal->getSize()*sizeof(Branch));

#ifdef BTREE_INTEGRITY
        found.check();
#endif
    }

    // refresh the state
    if (state->size == 2) {
        state->branch[1].alloc
            = state->branch[1].value.internal->branch[0].alloc;
        state->branch[1].addr
            = state->branch[1].value.internal->branch[0].addr;
    }
    state->branch[0].alloc = state->branch[0].value.internal->branch[0].alloc;
    state->branch[0].addr  = state->branch[0].value.internal->branch[0].addr;
}

int BTree::check( size_t depth, Node* node)
{
    size_t i;

    if (!Mmap::check( node))
        return 0;
    if (!node->getSize() || node->getSize() > Node::branches)
        return 0;
    for (i=0; i<node->getSize(); i++) {
        if (!Mmap::check( node->branch[i].addr))
            return 0;
        if (i < node->getSize()-1) {
            if (node->branch[i].alloc > node->branch[i+1].alloc)
                return 0;
            else if (node->branch[i].alloc == node->branch[i+1].alloc
                     && node->branch[i].addr >= node->branch[i+1].addr)
                return 0;
            else if (!depth && node->branch[i].alloc
                     == node->branch[i+1].alloc
                     && ((char*)node->branch[i].addr
                         + node->branch[i].value.leaf
                         > (char*)node->branch[i+1].addr))
                return 0;
        }
        if (depth) {
            if (!check( depth-1, node->branch[i].value.internal))
                return 0;
            if (node->branch[i].alloc
                != node->branch[i].value.internal->branch[0].alloc)
                return 0;
            if (node->branch[i].addr
                != node->branch[i].value.internal->branch[0].addr)
                return 0;
        }
    }
    return 1;
}


void BTree::Assert( size_t depth, Node* node)
{
    size_t    i;

    if (!node)
        return;

    assert (Mmap::check( node));
    assert (node->getSize() && (node->getSize() <= Node::branches));

    for (i=0; i<node->getSize(); i++) {
        assert (Mmap::check( node->branch[i].addr));

        if (i < node->getSize()-1) {
            assert (node->branch[i] < node->branch[i+1]);
            if (!depth && (node->branch[i].alloc == node->branch[i+1].alloc))
                assert (((char*)node->branch[i].addr
                         + node->branch[i].value.leaf)
                        <= (char*)node->branch[i+1].addr);
        } else {
#if 0
            if (!depth && (node->branch[i].alloc == node->branch[i+1].alloc))
                assert (((char*)node->branch[i].addr
                         + node->branch[i].value.leaf)
                        <= (char*)(tree->root));
#endif
        }

        // check the integrity of this allocation
        if (!depth && (node->branch[i].alloc != Memory::no_alloc)) {
            // assert(*(void**)(node->branch[i].addr) == node->branch[i].alloc);
#ifdef GUARD_ALLOC
            if (((unsigned int*)(node->branch[i].addr))[1] == Guard::Magic) {
                //cerr << "Checking allocation " << node->branch[i].addr << '\n';
                Memory::checkGuard((char*)(node->branch[i].addr)
                                   + sizeof(void*) + sizeof(Guard));
            }
#endif
        }

        if (depth) {
            // recurse through tree
            Assert( depth-1, node->branch[i].value.internal);
            assert (node->branch[i].alloc
                    == node->branch[i].value.internal->branch[0].alloc);
            assert (node->branch[i].addr
                    == node->branch[i].value.internal->branch[0].addr);
        }
    }
}

////////////////////////////////////////////////////////////////
// bool = find( alloc, addr, marks)
//
// find the entry matched by (alloc,addr).  Return true if found, 0 otherwise.
// Place the low and high existing entries in marks[0] and marks[1]
// respectively, unless marks is NULL.  Note that marks is set whether the
// search is successful or not; in fact, it is MORE useful for failed searches.
//
// input:
//  marks: address of Leaf[2] for matched entries or 0.
//  alloc,addr: search key
//
// output:
//  marks[0]: the right most entry less than or equal to (alloc,addr)
//  marks[1]: the left most entry greater than (alloc,addr)
//  return: true or false depending on wether the search was successful.
////////////////////////////////////////////////////////////////
int BTree::find( const void* const alloc, void* addr, Leaf* marks)
{
    Node* node = tree->root;
    Found found;
    size_t    i;

    // lookup in an empty tree
    if (!tree->root) {
        if (marks) {
            marks[0].alloc = 0;
            marks[0].addr = 0;
            marks[0].size = 0;
            marks[1] = marks[0];
        }
        return 0;
    }

#ifdef BTREE_INTEGRITY
    node->check();
    Memory::Assert();
#endif

    // lookup before the first key
    if ((alloc == node->branch[0].alloc)
        ? addr < node->branch[0].addr
        : alloc < node->branch[0].alloc) {
        for (i=0; i<tree->depth; i++) {
            node = node->branch[0].value.internal;

#ifdef BTREE_INTEGRITY
            node->check();
            Memory::Assert();
#endif
        }
        if (marks) {
            marks[0].alloc = 0;
            marks[0].addr = 0;
            marks[0].size = 0;
            marks[1].alloc = node->branch[0].alloc;
            marks[1].addr  = node->branch[0].addr;
            marks[1].size  = node->branch[0].value.leaf;
        }
        return 0;
    }

    // search through the tree
    found.alloc = alloc;
    found.addr  = addr;
    found.node[0].node = tree->root;
    found.node[1].node = 0;

#ifdef BTREE_INTEGRITY
    found.check();
    Memory::Assert();
#endif

    for (i=0; i < tree->depth; i++) {
        lookup( &found);

#ifdef BTREE_INTEGRITY
        found.check();
        Memory::Assert();
#endif

        found.node[0].node = (found.node[0].node
                              ->branch[found.node[0].index].value.internal);
        if (found.node[1].node)
            found.node[1].node
                = (found.node[1].node
                   ->branch[found.node[1].index].value.internal);

#ifdef BTREE_INTEGRITY
        found.check();
        Memory::Assert();
#endif
    }
    lookup( &found);

#ifdef BTREE_INTEGRITY
    found.check();
    Memory::Assert();
#endif

    // write back the search results
    if (marks) {
        marks[0].alloc = found.node[0].node->branch[found.node[0].index].alloc;
        marks[0].addr = found.node[0].node->branch[found.node[0].index].addr;
        marks[0].size
            = found.node[0].node->branch[found.node[0].index].value.leaf;
        if (found.node[1].node) {
            marks[1].alloc
                = found.node[1].node->branch[found.node[1].index].alloc;
            marks[1].addr
                = found.node[1].node->branch[found.node[1].index].addr;
            marks[1].size
                = (found.node[1].node->branch[found.node[1].index].value.leaf);
            //assert(marks[0] < marks[1]);
        } else {
            marks[1].alloc = 0;
            marks[1].addr = 0;
            marks[1].size = 0;
        }
    }

    return (alloc == found.node[0].node->branch[found.node[0].index].alloc
            && addr  == found.node[0].node->branch[found.node[0].index].addr);
}


////////////////////////////////////////////////////////////////////
// tree = tree.add( alloc, addr, size)
//
// Add (or update) and entry in the tree.
//
// input:
//  alloc,addr,size: key and value for the addition.
//
// output:
//
////////////////////////////////////////////////////////////////////
void BTree::add( void* alloc, void* addr, size_t size)
{
    Node* node = tree->root;
    Edit  reg;

    // adding in an empty tree is easy
    if (!node) {
        node = new Node;
        if (!node) throw runtime_error("Can't create Node __FILE__ __LINE__");
        node->setSize(1);
        node->branch[0].alloc = alloc;
        node->branch[0].addr  = addr;
        node->branch[0].value.leaf = size;
        tree->depth = 0;
        tree->root = node;

#ifdef BTREE_INTEGRITY
        Memory::Assert();
#endif
        return;
    }

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    // setup the recursive addition
    reg.depth = tree->depth + 1;
    reg.size = 1;
    reg.branch[0].alloc = node->branch[0].alloc;
    reg.branch[0].addr  = node->branch[0].addr;
    reg.branch[0].value.internal = node;

    // if adding before the first entry, transmute to insertion
    if ((alloc == node->branch[0].alloc)
        ? addr < node->branch[0].addr
        : alloc < node->branch[0].alloc)
        doins( &reg, alloc, addr, size);
    else
        doadd( &reg, alloc, addr, size);

    // tree did not grow
    if (reg.size == 1)
        tree->root = reg.branch[0].value.internal;

    // tree grows
    else {
        node = new Node;
        if (!node) throw runtime_error("Can't create Node __FILE__ __LINE__");
        node->setSize(2);
        memmove( node->branch, reg.branch, 2*sizeof(Branch));
        tree->root = node;
        ++ tree->depth;
    }

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    return;
}


///////////////////////////////////////////////////////////////////
// tree = tree.del( alloc, addr)
//
// Remove the entry (if any) that matches (alloc,addr).
//
// input:
//  alloc,addr: search key
//
// output:
//
///////////////////////////////////////////////////////////////////
void BTree::del( void* alloc, void* addr)
{
    Node *node = tree->root;
    Edit reg;

    // deleting from an empty tree
    if (!node) return;

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    // deleting before the first key
    if ((alloc == node->branch[0].alloc)
        ? addr < node->branch[0].addr
        : alloc < node->branch[0].alloc)
        return;

    // delete recursively
    reg.depth = tree->depth + 1;
    reg.size = 1;
    reg.branch[0].alloc = node->branch[0].alloc;
    reg.branch[0].addr  = node->branch[0].addr;
    reg.branch[0].value.internal = node;
    dodel( &reg, alloc, addr);

    // deleted last entry
    if (!reg.size) {
        tree->root = 0;
        return;
    }

    // tree shrinks
    if (tree->depth && reg.branch[0].value.internal->getSize() == 1) {
        tree->root = reg.branch[0].value.internal->branch[0].value.internal;
        delete reg.branch[0].value.internal;
        -- tree->depth;
        return;
    }

    tree->root = reg.branch[0].value.internal;

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    return;
}

// move region (addr of size) into unallocated region
// abutting unallocate regions are coalesced
void BTree::unallocate( void* addr, size_t size)
{
    Leaf found[2];

    if (!size) return;

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    BTree::find( (void*)Memory::no_alloc, addr, found);
    if (found[0].alloc == Memory::no_alloc
        && (char*)found[0].addr + found[0].size == (char*)addr) {
        //  found[0]: an unallocated region immediately before addr - coalesce
        BTree::del( (void*)Memory::no_alloc, found[0].addr);	// remove it
        addr = found[0].addr;	// adjust unallocation start down
        size += found[0].size;	// adjust unallocation size up
    }
    if (found[1].alloc == Memory::no_alloc
        //  found[1]: an unallocated region immediately after addr - coalesce
        && (char*)addr + size == (char*)found[1].addr) {
        BTree::del( (void*)Memory::no_alloc, found[1].addr);
        size += found[1].size;
    }
    // add addr back into tree as unallocated
    BTree::add( (void*)Memory::no_alloc, addr, size);

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif
}

void* BTree::allocate( void* alloc, void* addr, size_t size)
{
    Leaf found[2];

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    assert( alloc != Memory::no_alloc);	// we can't allocate to `unallocated'

    BTree::find( (void*)Memory::no_alloc, addr, found);	// get the unallocated region
    assert( found[0].alloc == Memory::no_alloc);	// found unallocated region
    assert( (char*)found[0].addr + found[0].size >= (char*)addr + size); // right size
    BTree::del( (void*)Memory::no_alloc, found[0].addr);	// remove the unallocation
    BTree::add( alloc, addr, size);				// claim allocation for allocator
    unallocate( found[0].addr, (char*)addr - (char*)found[0].addr);	// trim prolog
    unallocate( (char*)addr+size,
                (char*)found[0].addr+found[0].size-(char*)addr-size);	// trim epilog

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    *((void**&)addr)++ = alloc;	// adjust to the internal allocation
    return addr;
}
