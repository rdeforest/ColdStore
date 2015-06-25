#ifndef STL_MMAP_H
#define STL_MMAP_H

#include <memory>
#include "mmap.hh"

/** Mmap_alloc matchines the interface required for allocators under STL
    with the interface defined for Mmap;
*/

//Note:  I could put a template around this, which would refer to the filehandle these operations.

class mmap_alloc {
public:
  static size_t open(int fd) { return Mmap::open(fd);} ;
  static size_t close(int fd) { return Mmap::close(fd);} ;
  static void *allocate(size_t n) { return Mmap::alloc(n);};
  static void deallocate(void *p,size_t n) { Mmap::free(p);};  //What's the equivalent for Mmap?
  static void *reallocate(void *p,size_t old_size,size_t new_size) { 
    Mmap::realloc(p,new_size);
    return
  }
}

template<class _Tp,int inst>
struct _Alloc_traits <_Tp,mmap_alloc>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<Tp,mmap_alloc> _Alloc_type;
  typedef simple_alloc<Tp,mmap_alloc> allocator_type;
}

//TODO:  Create a set of typedefs with partially specialized versions of the
//major collections using the coldstore mmap allocator.

#endif STL_MMAP_H
