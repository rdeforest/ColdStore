// Latch and shared memory
// Copyright (C) 1998,1999 Phillipe Hebrais,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
/* $Id: mmap.cc,v 1.8 2001/04/22 06:15:22 coldstore Exp $ */
 
/* Latches and shared memory.

 Shared memory: allow any number of forks (processes) to access a
 shared memory-mapped database.  At this level we only deal with
 mapping the memory and allocating pages (no deallocation).

*/
#include <assert.h>
#include <signal.h>
#include <fcntl.h>
#include <iostream.h>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

#include "qvmm.h"
#include "store.hh"

int errno;
extern void *coldroot;
bool Mmap::initialized = false;		// predicate: is Mmap initialized?

void Latch::lock( void* target)
{
    size_t i, j, n;
    char r;

    /* look for a free spot to lock:
       start at (target%size)
       advance (mod size) until you find the spot.
       beware of infinite loops when table is full.
    */

    for (n=i=((size_t)target) % size; ; i = j) {

        // find two consecutive free entries
        if ((j=i+1) == size) j = 0;
        if (j == n) { qvmm_throw("Too many latches __FILE__ __LINE__."); }

        // look for a first one
        r = 1;
        asm volatile ("xchgb (%2),%0" : "=r"(r) : "0"(r),
                      "r"(latch+i));
        if (r) continue;

        // look for a second
        r = 1;
        asm volatile ("xchgb (%2),%0" : "=r"(r) : "0"(r),
                      "r"(latch+j));
        if (!r) break;

        // we don't have the second.  release the first and keep looking
        latch[i] = 0;
        if (++j == size) j = 0;
        if (j == n) { qvmm_throw("Too many latches __FILE__ __LINE__."); }
    }

    // got two consecutive entries.  release the second.  `i' is our spot
    latch[j] = 0;

    /* wait for the target lock:
       start from our spot.
       wait for previous (mod size) entry to be free.
       lock previous and unlock current.
       repeat starting from previous until we have the target lock
    */

    while (i != n) {
        j = i ? i - 1 : size - 1;

        // wait for previous lock
        for (r = 1;;) {
            asm volatile ("xchgb (%2),%0" : "=r"(r) : "0"(r),
                          "r"(latch+j));
            if (!r) break;
            usleep(timeout);
        }

        // release current lock
        latch[i] = 0;
        i = j;
    }
}

/** unlock a latch
 */
void Latch::unlock( void* target) {
    latch[((size_t)target)%size]=0;
}

/** open the Mmap to a given file
    @param fd file descriptor of the file to mmap
    @return the size of the Mmap'ed region
*/
size_t Mmap::open( int fd)
{
    void   *ptr;

    /* prevent large core dumps (the dump facility is not bright
       enough to skip unused but mapped memory). */
    {
        rlimit rlim;
        rlim.rlim_cur = 1024;
        rlim.rlim_max = 1024;
        setrlimit( RLIMIT_CORE, &rlim);
    }

    assert(!check(static_cast<void *>(sbrk(0)))
           || !"Initial program data overlaps store");
    
    assert(!check(static_cast<void *>(&ptr))
           || !"Initial stack overlaps store");

    // try to recover the content of the file
    size_t sz = lseek( fd, 0, SEEK_END);
    
    // grow the file to its maximum size
    if (sz < max) {
        lseek( fd, max-1, SEEK_SET);
        if (write( fd, "", 1) != 1) {
            qvmm_throw("Can't write to the image file");
        }
    }

    // mmap the file in
    if ((ptr = ::mmap( (caddr_t)base, max,
                       PROT_READ|PROT_WRITE|PROT_EXEC, 
                       MAP_SHARED|MAP_FILE|MAP_FIXED|MAP_NOSYNC,
                       fd, 0)) != base) {
        if (ptr) munmap( (caddr_t)ptr, max);
        qvmm_throw("Can't map image to memory");
    }
    if (sz == 0) {
        extern int coldmagic;

        // freshly opened store - calculate the size
        DEBLOG(cerr << "New store:\n "
               << "\tinitialized:\t" << &initialized << '\n'
               << "\tcoldmagic:\t" << &coldmagic << '\n'
               << "\tend:\t" << &end << '\n'
               << "\tstore:\t" << &store << '\n'
               << "\tbase:\t" << base << '\n' );

        size = (char *)&store - (char*)base;

        DEBLOG(cerr << "\tsize: " << size << '\n');
    } else {
        DEBLOG(cerr << "Old store: "
               << &store << ' ' << ' ' << base << ' ' << size << '\n');
    }

#ifndef DEBUG
    if (mprotect( (caddr_t)base + size, max - size, PROT_NONE))
    {
        close( fd);
        qvmm_throw("Can't mprotect shared memory");
    }
#endif

    if (sz == 0) {
        // freshly opened store - create some globals
        extern BTree BTreeRoot;
        BTree::tree = &BTreeRoot;
    }

    initialized = true;		// signal completion of initialization

    return size;
};

int Mmap::open(char *file)
{
    int     fd;
    size_t size;
    fd = ::open(file, O_RDWR|O_CREAT, 0777);
    if (fd < 0)
        qvmm_throw("__FILE__ __LINE__: can't open store file");
    size = open(fd);
    DEBLOG(cerr << "qvmm store size: " << size << '\n');
    return fd;
}

/** close the Mmap
    @param fd file to close
*/
void Mmap::close( int fd)
{
    size_t sz = size;		// grab actual size before munmapping
    initialized = false;	// signal closure of Mmap
#if 0
    // we let the system unmap our memory
    caddr_t bs = (caddr_t)base;
    size_t mx = max;
    munmap( bs, mx);
#endif
    ftruncate( fd, sz);
}

/** allocate a contiguous free region and advance the upper Mmap limit
    @param size size of allocation
    @return a contiguous free region of the Mmap
*/
void* Mmap::alloc( size_t _size)
{
    void* ret;
    _size = align(_size);
    Latch::lock(coldroot);	// obtain a lock on the Mmap
    ret = (char*)base + size;	// get the next free location
    size += _size;		// advance the mmap upper limit
    Latch::unlock(coldroot);	// release the Mmap lock

#ifndef DEBUG
    // remove protection on the newly allocated region
    if (mprotect( (caddr_t)ret, _size, PROT_READ|PROT_WRITE|PROT_EXEC))
    {
        qvmm_throw("Can't un-mprotect memory");
    }
#endif

    return ret;
};

