// Memory - fundamental qvmm allocation class
// Copyright (C) 1998,1999 Phillipe Hebrais,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: memory.cc,v 1.16 2002/04/01 08:48:59 coldstore Exp $";
//#define DEBUGLOG
#include <assert.h>
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <iostream.h>
#include <new>
#include "qvmm.h"
#include "store.hh"

void qvmm_throw(char *message)
{
    onError(message);
    throw runtime_error(message);
}

/** Lock the Memory
 */
void Memory::lock(void)
{
    Latch::lock(this);
}

/** Unlock the Memory - permitting another task to lock it
 */
void Memory::unlock(void)
{
    Latch::unlock(this);
}

int Memory::check( void)
{
    int ret;

    if (!BTree::tree->root) return 1;
    ret = BTree::check( BTree::tree->depth, BTree::tree->root);
    return ret;
}

void Memory::Assert( void)
{
    if (BTree::tree->root) {
        BTree::Assert( BTree::tree->depth, BTree::tree->root);
    }
}



size_t Memory::size(const void* addr)
{
    void* allocator;
    Leaf  found[2];

    if (!addr) return 0;

#ifdef GUARD_ALLOC
    addr = ((Guard*)addr)-1;
#endif
    allocator = *--(void**&)addr;

    if (!BTree::find( allocator, (void*)addr, found)) {
        // check that addr was in fact allocated by qvmm
        qvmm_throw("Sizing bad pointer or mangled cell");
    }

    return found[0].size;
}

size_t Memory::itsSize(const void* ptr) const {
#ifdef GUARD_ALLOC
    return Memory::size((char*)ptr)
        - sizeof(void*) - (2*sizeof(Guard));
#else
    return Memory::size(ptr) - sizeof(void*);
#endif
}


size_t Memory::mySize() const {
    return itsSize(this);
}


void* Memory::allocator( void* addr)
{
#ifdef GUARD_ALLOC
    addr = ((Guard*)addr)-1;
#endif
    return *--(void**&)addr;
}



void* Memory::realloc( void* addr, size_t new_size, void* allocator)
{
    Leaf   found[2];

    if (!addr) return alloc( new_size, allocator);

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif
    // round up requested size
    //new_size = (new_size + 2*sizeof(void*) - 1) & ~(sizeof(void*)-1);
    new_size += sizeof(void*); // locality pointer
    new_size = ((new_size + quantum - 1)/quantum)*quantum;
    
    // get the original internal and external sizes
    // nb: esize will include Guard overhead (if any)
    // nb: isize will exclude Guard and *allocator overhead
    size_t esize = size(addr);	// this will throw if addr wasn't allocated
#ifdef GUARD_ALLOC
    size_t isize = esize - sizeof(void*) - (2*sizeof(Guard));
#else
    size_t isize = esize - sizeof(void*);
#endif
    int shrink = isize - new_size;	// how much are we shrinking/-growing?
    DEBLOG(cerr << "realloc " << addr << " from " << esize << " to " << new_size << " isize: " << isize << " shrinkage " << shrink << '\n');
    // real_addr is the actual start of the allocation
    void *real_addr = checkGuard(addr, false);
    --(void**&)real_addr;

    // get the original allocator
    if (allocator == no_alloc)
        allocator = *(void**)real_addr;
    void *real_allocator = *(void**)real_addr;

    if (isize == new_size) {
        // allocating the same sized space, leave it alone
        if (allocator != real_allocator) {
            // same size, changed allocator
            BTree::del(real_allocator, real_addr);
            BTree::add(allocator, real_addr, esize);
            return guard((void **)real_addr + 1, allocator, esize); // remake guards
        } else {
            return addr;	// same size, leave it alone
        }
    } else if (isize > new_size) {
        // reallocating a smaller space, shrink in place
        if (allocator != real_allocator) {
            BTree::del( *(void**)real_addr, real_addr);	// delete the original allocation
            BTree::add( allocator, real_addr, new_size);	// create new allocation
        }
        void *shrinkage = (char *)real_addr + esize - shrink;	// what are we shrinking?
        BTree::unallocate(shrinkage, shrink);	// trim tail
        return guard((void**)real_addr + 1, allocator, esize - isize + new_size);
    } else if (BTree::find(no_alloc, (char*)real_addr + esize, found)
               && (isize + found[0].size) > new_size) {
        // found abutting unallocated space - nb: shrink == -growth
        BTree::del(*(void**)real_addr, real_addr);	// delete old region
        BTree::del((void*)no_alloc, (char*)real_addr + esize);	// delete abutting region
        *(void**)real_addr = allocator;			// establish new allocator
        int gsize = esize - shrink;			// external size has increased
        BTree::add( allocator, real_addr, gsize);	// add region to new allocator
        
        BTree::unallocate((char*)real_addr + gsize, esize + found[0].size - gsize);
        return guard((void**)real_addr + 1, allocator, gsize);
    } else {
        // no abutting region - allocate anew
        void *new_addr = alloc( new_size, allocator);	// standard allocation
        memcpy(new_addr, addr, isize);	// copy the contents of the original allocation
        Memory::free(addr);	// we're done with the original allocation
        return new_addr;			// return the adjusted new allocation
    }
}


#ifdef GUARD_ALLOC
Guard::Guard(void *_allocator, void *_address, size_t _size)
        : magic(Magic),
          allocator(_allocator),
          address(_address),
          size(_size),
          checksum(0)
{
    // cerr << "sizeof Guard " << sizeof(Guard) << '\n';
    // calculate checksum
    for (unsigned int i = 0; i < sizeof(Guard)/sizeof(size_t) - 1; i++)
        checksum ^= ((int*)this)[i];
}

// endGuard - given a pointer to a start Guard, find the endGuard
Guard *Guard::endGuard()
{
    return ((Guard*)((char*)address - sizeof(void*) + size)) - 1;
    // Guard abutting end region
}

// check that a guard appears to be conformant.
bool Guard::check()
{
    // check magic
    if (magic != Magic) {
        cerr << "\n**bad Magic "
             << magic << "!=" << Magic
             << " at " << &magic
             << " object allocated at " << (char*)&magic + sizeof(Guard)
             << '\n';
        onError("bad Magic");
        return false;
    }

    // compare checksums
    size_t ck = 0;
    for (unsigned int i = 0; i < sizeof(Guard)/sizeof(size_t) - 1; i++)
        ck ^= ((int*)this)[i];
    if (ck != checksum) {
        cerr << "bad checksum\n";
        onError("bad checksum");
    }

    return ck == checksum;
}

void *Memory::guard(void *ptr, void *alloc, size_t size)
{
    assert(ptr);
    DEBLOG(cerr << "Guard: " << ptr
           << " ... " << (void*)((char*)ptr + size)
           << " in " << alloc
           << " of " << size
           << '\n');
    Guard *start = new (ptr) Guard(alloc, ptr, size);
    Guard *end = new (start->endGuard()) Guard(alloc, ptr, size);
    ptr = (void*)(start + 1);

    // check that the guards reflect reality (and vica versa :)
    assert ((*end == *start)
	    && "Guards don't match");
#if 0
    assert((Memory::size(ptr) == start->size)
	   && "Sizes don't match");
#endif
    assert ((Memory::allocator(ptr) == start->allocator)
	    && "Allocators don't match");

    DEBLOG(cerr << "allocated: " << ptr << " size: " << size << '\n');
#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif
    return ptr;
}

void *Memory::checkGuard(void *ptr, bool damage)
{
    // the pointer here is the one passed in by the user
    Guard *start = (Guard*)ptr;
    start--;	// adjust for user space

    // Check the opening Guard
    if (!start->check()) {
        cerr << "Offending Start Guard address " << start
             << " in " << ptr
             << '\n';
        if (start->magic == ~Guard::Magic)
            assert(!"Deallocating Twice");
        else
            assert(!"Underwritten Guard");
    }

    // Check the closing Guard
    Guard *end = start->endGuard();
    if (!end->check()) {
        cerr << "Offending End Guard address " << end
             << " in " << ptr
             << " starts at " << start
             << '\n';
        if (end->magic == ~Guard::Magic)
            assert(!"Probably Overwritten Guard");
        else
            assert(!"Overwritten Guard");
    }

    assert ((*end == *start) &&
            "Guards don't match");
#if 0
    assert((Memory::size(ptr) == start->size)
	   && "Sizes don't match");
#endif
    assert ((Memory::allocator(ptr) == start->allocator)
            && "Allocators don't match");

    // damage the guards so we know we've deallocated
    if (damage) {
        DEBLOG(cerr << "Damage Guard: " << start << '\n');
        start->damage();
        end->damage();
    }

    return (void*)start;
}

#endif

//DEBLOG(extern int nProfileLookupCount);

void* Memory::alloc( size_t size, void* alloc)
{
    //DEBLOG(nProfileLookupCount=0);
    void* ret;

    if (alloc == no_alloc || !size) {
      return 0;
    }

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

#ifdef GUARD_ALLOC
    size += 2 * sizeof(Guard);	// extra space for start and end Guards
#endif
    //size = (size + 2*sizeof(void*) - 1) & ~(sizeof(void*)-1); // round size up
    size += sizeof(void*); // locality pointer
    size = ((size + quantum - 1)/quantum)*quantum;

    if (alloc) {
        /* parallel scan of no_alloc and alloc's cell lists */
        Leaf   found[2];
        void   *ptr;
        size_t tsize;

        for (ptr=0,tsize=0;;) {
            // ptr/size describes a free block we want to consider
            BTree::find( alloc, ptr, found);
            if (found[0].alloc == alloc
                && tsize >= size
                && (Mmap::pageof( (char*)found[0].addr+found[0].size)
                    == Mmap::pageof( ptr))) {
                ret = BTree::allocate( alloc, ptr, size);
                return guard(ret, alloc, size);
            } else if (found[1].alloc != alloc) {
                break;
            } else if (tsize >= size
                       && (Mmap::pageof( found[1].addr)
                           == Mmap::pageof( (char*)ptr+tsize))) {
                ret = BTree::allocate( alloc, (char*)ptr+tsize-size, size);
                return guard(ret, alloc, size);
            } else {
                ptr = found[1].addr;
                tsize = found[1].size;
            }

            // ptr/size describes a target position
            // for when looking for a free cell
            BTree::find( (void*)no_alloc, ptr, found);
            if (found[0].alloc == no_alloc
                && found[0].size >= size
                && (Mmap::pageof( (char*)found[0].addr+found[0].size)
                    == Mmap::pageof( ptr))) {
                ret = BTree::allocate(alloc,
                                      (char*)found[0].addr+found[0].size-size,
                                      size);
                return guard(ret, alloc, size);
            } else if (found[1].alloc != no_alloc) {
                break;
            } else if (found[1].size >= size
                       && (Mmap::pageof( found[1].addr)
                           == Mmap::pageof( (char*)ptr+tsize))) {
                ret = BTree::allocate( alloc, found[1].addr, size);
                return guard(ret, alloc, size);
            } else {
                ptr = found[1].addr;
                tsize = found[1].size;
            }
        }
    }

    if (alloc) {
        /* look for something close to the allocator */
        Leaf found[2];
        BTree::find( (void*)no_alloc, alloc, found);
        if (found[0].alloc == no_alloc
            && found[0].size >= size
            && (Mmap::pageof( (char*)found[0].addr+found[0].size)
                == Mmap::pageof( alloc)))
        {
            ret = BTree::allocate(alloc,
                                  (char*)found[0].addr+found[0].size-size,
                                  size);
            return guard(ret, alloc, size);
        } else if (found[1].alloc == no_alloc
                   && found[1].size >= size
                   && (Mmap::pageof( found[1].addr)
                       == Mmap::pageof( alloc))) {
            ret = BTree::allocate( alloc, found[1].addr, size);
            return guard(ret, alloc, size);
        }
    }

    { /* look for the rightmost acceptable block */
        Leaf found[2];
        found[0].addr = 0;
        for (;;) {
            BTree::find( (void*)no_alloc, (char*)found[0].addr-1, found);
            if (found[0].alloc != no_alloc) break;
            if (found[0].size >= size) {
                ret = BTree::allocate(alloc,
                                      (char*)found[0].addr+found[0].size-size,
                                      size);
                return guard(ret, alloc, size);
            }
        }
    }

    { /* resort to growing the memory */
        void*  ptr;
        size_t tsize;
        //DEBLOG(cerr << nProfileLookupCount << "\n");
        tsize = Mmap::align( size);
	tsize *= quantum;
        if (!(ptr = Mmap::alloc( tsize))) {
	  cerr << "Out of Memory" << '\n';
            return 0;	// out of memory
        }
        BTree::unallocate( ptr, tsize);
        ret = BTree::allocate( alloc, ptr, size);
        return guard(ret, alloc, size);
    }
}

void Memory::free( void* addr)
{
    void* alloc;
    Leaf found[2];

    assert(!(((int)addr) & 0x3));

#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif

    addr = checkGuard(addr, true);	// check and remove Guards, if enabled
    alloc = *--((void**&)addr);
    if (!BTree::find( alloc, addr, found)) {
        qvmm_throw("Freeing bad pointer or mangled cell");
    }

    DEBLOG(cerr << "free " << addr
           << " Leaf: " << found[0].addr
           << ", " << found[0].size
           << '\n');

    BTree::del( alloc, addr);
    BTree::unallocate( addr, found->size);
    DEBLOG(cerr << "QVMM free: " << addr << '\n');
#ifdef BTREE_INTEGRITY
    Memory::Assert();
#endif
}

void* Memory::operator new( size_t size, void* where=(void*)0)
{
    return memset(alloc(size,where), 0, size);
}

void* Memory::operator new( size_t size, size_t extra, void* where=(void*)0)
{
    return memset(alloc(size + extra, where), 0, size + extra);
}

void  Memory::operator delete( void* ptr)
{
    DEBLOG(cerr << "Memory::delete(" << ptr << ")\n");
    free(ptr);
}

void* Memory::operator new[] (size_t size, void* where=(void*)0)
{
    return memset(alloc(size,where), 0, size);
}

void  Memory::operator delete[] ( void* ptr)
{
    DEBLOG(cerr << "Memory::delete(" << ptr << ")\n");
    free(ptr);
};

void* operator new( size_t size, Memory* where)
{
    return memset(Memory::alloc(size,where), 0, size);
}

void* operator new[]( size_t size, Memory* where)
{
    return memset(Memory::alloc(size,where), 0, size);
}

//extern "C"
//{
void __builtin_delete (void *allocation);
//}

void operator delete(void *allocation)
{
    if (!allocation)
        return;
    if (Mmap::check(allocation)) {
        // allocated in the Mmap
        Memory::free(allocation);
    } else {
        // allocated by the standard malloc
        __builtin_delete (allocation);
    }
}

void operator delete[] (void* what) throw()
{
    operator delete(what);
}

#ifdef WRAP_MALLOC

// Wrappers for standard allocation functions.
// These displace the standard functions when Mmap is up.
// specify -wrap malloc and -wrap free on the linker line
extern "C"
{
    void *__real_malloc(size_t size);
    void *__real_realloc(void *ptr, size_t size);
    void __real_free (void *allocation);

    void *__real___libc_free(void *allocation);
    void *__real___libc_malloc(size_t size);
}
void *__builtin_new(size_t size) throw();
void __builtin_delete (void *allocation);


void *__wrap_malloc (size_t size)
{
    if (Mmap::initialized) {
        // Mmap's up: allocate in the Mmap
      void *allocation = Memory::alloc(size, (void*)0);
      assert(allocation || !size);
      return allocation;
    } else {
        // Mmap's not yet up: allocate normally
        return __real_malloc (size);
    }
}

void __wrap_free (void *allocation)
{
    if (Mmap::check(allocation)) {
        // allocated in the Mmap
        Memory::free(allocation);
    } else {
        // allocated by the standard malloc
        __real_free (allocation);
    }
}

void *__wrap_realloc(void *ptr, size_t size)
{
    if (Mmap::check(ptr)) {
        // allocated in the Mmap
        return Memory::realloc(ptr, size);
    } else {
        // allocated by the standard malloc
        return __real_realloc (ptr, size);
    }
}

void *__wrap___libc_malloc (size_t size)
{
    if (Mmap::initialized) {
        // Mmap's up: allocate in the Mmap
        return Memory::alloc(size, (void*)0);
    } else {
        // Mmap's not yet up: allocate normally
        return __real___libc_malloc (size);
    }
}

void __wrap___libc_free (void *allocation)
{
    if (Mmap::check(allocation)) {
        // allocated in the Mmap
        Memory::free(allocation);
    } else {
        // allocated by the standard malloc
        __real___libc_free (allocation);
    }
}

void *operator new(size_t size) throw()
{
    if (Mmap::initialized) {
        // Mmap's up: allocate in the Mmap
        return Memory::alloc(size, (void*)0);
    } else {
        // Mmap's not yet up: allocate normally
        return __builtin_new (size);
    }
}

// Global new[] and delete[]
//
// XXX - these conflict with libstdc++
//
//void* operator new[] (size_t size) throw (std::bad_alloc)
//{
//    return operator new(size);
//}

//void* operator new[] (size_t size, void* where) throw()
//{
//    return operator new(size);
//}

#endif
