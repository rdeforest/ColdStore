// ColdStore Mmap manager
// Copyright (C) 1998,1999 Phillipe Hebrais,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// $Id: qvmm.h,v 1.13 2002/03/15 12:49:25 coldstore Exp $

#ifndef QVMM_H
#define QVMM_H
#include <sys/types.h>
#include <stdlib.h>
#include "options.h"

void qvmm_throw(char *message);
void onError(const char *type = NULL);

// DEBUGLOG turns on traces
#ifdef DEBUGLOG
#define DEBLOG(x) (x);
#else
#define DEBLOG(x)
#endif

#define AKO(inst, typ) ((Data*)inst && dynamic_cast<typ *>((Data*)inst))
//arg->isAKO(typeid(Dict))
#define toType(inst, typ) (dynamic_cast<typ *>((Data*)inst))

// GDB interface, technique courtesy GNU Nana 
#ifdef _NANA_FILTER_
#define GDBCALL(comd) \
  @@break @__FILE__:__LINE__@@ \
  @@command@@ \
  @@silent@@ \
  @@comd@@ \
  @@cont@@ \
  @@end@@
#else
#define GDBCALL(c) asm("nop")
#endif

#ifdef linux
#define MAP_NOSYNC 0
#endif

/** ColdStore Mmap manager
 * @author Phillipe Hebrais
 *
 * Mmap maintains the store, providing
 * @li store information,
 * @li new page allocation, and
 * @li @ref Latch services</ul> to the application.
 */
class Mmap
{
    // space for a store's magic number and Global region size
    /** actual size of store */
    static size_t size;

    /** store's persistent header */
    static int header[];

    /** end of header */
    static int end[];

    /** first byte of non-header store (R/O!!!) */
    static int store[];

  public:
    /** store mmap'd base address (page aligned) */
    static const void *const base = QVMM_BASE;

    /** maximum size of store (page aligned) */
    static const size_t max = QVMM_SIZE;

    /** underlying page size of store (power of 2) */
    static const size_t page_size = 4096;

    /** flag: is Mmap initialized? */
    static bool initialized;
    
    // Initialize and uninit;

    /** Mmap the file as a store.
     * @param fd file descriptor of open store
     * @return current size of store
     */
    static size_t open( int fd);

    /** Open and Mmap the named file as a store.
     * @param file file name
     * @return file descriptor of open store
     */
    static int open(char *file);

    /** close the Mmap
     * @param fd file descriptor of open store
     */
    static void close( int fd);
    
    /** Allocate a range of pages in the store
     * @param size size of allocation
     * @return allocated region
     */
    static void* alloc( size_t size);

    // Munge pointers and sizes

    /** check the validity of a pointer
     * @param ptr pointer to be checked
     * @return true if the pointer is in the Mmap
     */
    static int check( void* ptr)
        { return ptr >= base && (char*)ptr < (char*)base + max; }

    /** align a value to a page boundary
     * @param size value to align
     * @return page-aligned value
     */
    static size_t align( size_t size)
        { return (size+page_size-1) & ~(page_size-1); }

    /** return the page which contains a pointer
     * @param ptr pointer to align
     * @return page-aligned pointer
     */
    static void* pageof( void* ptr)
        { return (void*)(((size_t)ptr) & ~(page_size-1)); }
    
    // Latch ops;
    static void* locked( void);
    static void lock( void* l);
    static void unlock( void);
};

/** cheap limited deadlock- & starvation-free interthread spinlock mutex
 * @author Phillipe Hebrais
 *
 * Latches provide simple mutual exclusion between forks.  Each fork
 * may hold at most one Latch at a time.  Page allocation uses
 * this latch, so it must be free or the allocation will fail.
 *
 * Use Latches only for very short term locking.
 *
 * The goal is to let as many forks as possible access concurrently the
 * database so that while one fork page faults, the others can continue
 * using the cpu.  To avoid excessive serialization, we must reduce to a
 * minimum the amount of global locking required.
 *
 * Latches are meant to be very cheap (but limited) mutex.
 *
 * It avoids global locking by not using a wait queue.
 * Instead, a global hash table is used as rendez-vous point for 
 * latch requests and forks wait by yielding and looping.
 *
 * The latching algorithm garantees deadlock and starvation freeness.
*/
class Latch
{
    /** prime number > 2*n_forks */
    static size_t const size    = 4079;

    /** uSec to sleep while busy waiting for the spinlock */
    static size_t const timeout = 1000;
    
    /** set of latches of given size
     */
    static char latch[size];

  public:
    /** lock a latch
     * @param target address to lock
     */
    static void lock( void* target);
    
    /** unlock a latch
     * @param target locked address
     */
    static void unlock( void* target);
};

/** Entity allocated relative to an @ref Mmap
 * @author Phillipe Hebrais
 *
 * Memory instances are the basic currency of ColdStore,
 * providing interfaces to the store's features
 * @li extent-based (de&re)allocation
 * @li interface to @ref Latch locking of addresses
 * @li per-allocation information: @ref Memory::mySize @ref Memory::myAllocator
 * @li atomic low-level inc/dec ops: @ref Memory::inc @ref Memory::dec
 */
class Memory
{
  public:
    /** the `unallocated' allocator */
    static const void * const no_alloc = (void*)-1;
#ifdef GUARD_ALLOC
    static const int quantum = 80;
#else
    static const int quantum = 40;
#endif
    /** lock this object
     */
    void lock(void);

    /** unlock this object
     */
    void unlock(void);

#if 0    
    /** Atomic counter increment
     */
    static void inc( size_t& val) {
        __asm__ __volatile__ ("incl (%0)" : : "r" (&val) : "cc", "memory");
    };

    /** Atomic counter decrement
     */
    static int dec( size_t& val) {
        int ret = 0;
        __asm__ __volatile__ ("decl (%2);jz 1f;incl %0;1:"
                              : "=r" (ret) : "0" (ret), "r" (&val) : "cc", "memory");
        return ret;
    }
#endif

    static __inline__ void inc(volatile size_t &v)
        {
            __asm__ __volatile__(
                "lock; incl %0"
                :"=m" (v)
                :"m" (v)
                );
        }

    static __inline__ bool dec(volatile size_t &v)
        {
            unsigned char retval;
            
            __asm__ __volatile__(
                "lock ; decl %0; sete %1"
                :"=m" (v), "=qm" (retval)
                :"m" (v)
                );
            return retval == 0;
        }

    // C++ interface - new with optional allocator
    /** C++ extent placement new
        @arg size size of allocation in bytes
        @arg where neighborhood into which to allocate
        @returns storage allocated as requested
    */
    void* operator new( size_t size, void* where=(void*)0);

    /** C++ extent additional placement new
        @arg size size of allocation in bytes
        @arg extra extra allocation in bytes
        @arg where neighborhood into which to allocate
        @returns storage allocated as requested
    */
    void* operator new( size_t size, size_t extra, void* where=(void*)0);

    /** C++ object deletion
        @arg ptr a pointer returned by @ref operator new()
        @arg extra extra allocation in bytes
        @arg where neighborhood into which to allocate
        @returns storage allocated as requested
    */
    void  operator delete( void* ptr);

    /** C++ extent placement array new
        @arg size size of allocation in bytes
        @arg where neighborhood into which to allocate
    */
    void* operator new[] (size_t size, void* where=(void*)0);

    /** C++ array object deletion
        @arg ptr a pointer returned by @ref operator new[]()
        @arg extra extra allocation in bytes
        @arg where neighborhood into which to allocate
    */
    void  operator delete[] ( void* ptr);
    
    // C interface
    /** C storage reallocation
        @arg addr a pointer to previously alloc'd storage
        @arg size size of new allocation in bytes
        @arg allocator neighborhood into which to allocate
        @returns storage allocated as requested
    */
    static void*  realloc( void* addr, size_t size,
                           void* allocator=(void*)no_alloc);

    /** C storage allocation
        @arg size size of new allocation in bytes
        @arg allocator neighborhood into which to allocate
        @returns storage allocated as requested
    */
    static void*  alloc( size_t size, void* allocator);

    /** C storage deletion
        @arg ptr a pointer returned by @ref alloc()
    */
    static void   free( void* ptr);

    // utility accessors
    /** external size of allocation
        @arg ptr a pointer returned by @ref alloc()
        @returns size occupied by the allocation of ptr
    */
    static size_t size(const void* ptr);

    /** internal size of some allocation
     */
    size_t itsSize(const void* ptr) const;

    /** internal size of allocation surrounding this
     */
    size_t mySize() const;

    /** allocator of given region
        @arg ptr a pointer to allocated storage
        @returns the neighborhood with which ptr is associated
    */
    static void*  allocator( void* ptr);

    /** allocator of instance
        @returns the neighborhood with which this is associated
    */
    void *myAllocator() {
        return allocator(this);
    }

    // Health checks
    /** predicate: BTree is healthy
     */
    static int check(void);

    /** Assert health of BTree
     */
    static void Assert( void);

#ifdef GUARD_ALLOC
    /** Check Guards' health 
     */
    static void *checkGuard(void *ptr, bool damage=false);

  protected:
    /** construct a pair of Guards around the allocation to
     * detect overwriting and double-deletion.
     */
    static void *guard(void *ptr, void *alloc, size_t size);
    
    /** Non-destructively Validate Guards of this Memory instance
     */
    void validGuard() const {
        Memory::Assert();
        checkGuard((void*)this);
    }
#else
    // no Guarding - hopefully these will be optimised out
    static void *checkGuard(void *ptr, bool damage=false) {return ptr;}
  protected:
    static void *guard(void *ptr, void *alloc, size_t size) {return ptr;}
    void validGuard() const {}
#endif
};

// Global new[] and delete[] with Allocator placement
/** extent placement default array new operator using store
 */
void* operator new[] (size_t size, Memory* where);

/** extent placement default new operator using store
 */
void* operator new (size_t size, Memory* where);

#ifdef WRAP_MALLOC
// This permits use to wrap the standard allocation functions
// -wrap malloc -wrap free must appear in the ld linker command
extern "C"
{
    void *__wrap_malloc (size_t size);
    void __wrap_free (void *allocation);
    void *__wrap_realloc(void *ptr, size_t size);
}
#endif

#endif // QVMM_H
