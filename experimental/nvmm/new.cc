#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>

#include "new.hh"

#ifdef WRAP_MALLOC

// Wrappers for standard allocation functions.
// These displace the standard functions when Mmap is up.
// specify -wrap malloc and -wrap free on the linker line
extern "C"
{
    void *__real_malloc(size_t size);
    void *__real_realloc(void *ptr, size_t size);
    void __real_free (void *allocation);
}

void *__builtin_new(size_t size) throw();
void __builtin_delete (void *allocation);

void *__wrap_malloc (size_t size)
{
    // round size up to longword
    size = (size + 2*sizeof(void*) - 1) & ~(sizeof(void*)-1);
    void *result;
    if (MemArena::default_arena) {
        // Mmap's up: allocate in the Mmap
#ifdef GUARD_ALLOC
        result = MemArena::default_arena
            ->allocate(size + sizeof(Guard)*2, MemArena::default_locale);
        Guard *start = new (result)
            Guard(MemArena::default_locale, result, size);
        new (start->endGuard()) Guard(MemArena::default_locale, result, size);
        return result;
#else
        result = MemArena::default_arena
            ->allocate(size, MemArena::default_locale);
#endif
    } else {
        // Mmap's not yet up: allocate normally
        result = __real_malloc (size);
    }
    return result;
}

void *__wrap_realloc(void *ptr, size_t size)
{
    if (MemArena::default_arena) {
        // allocated in the Mmap
        return MemArena::default_arena->realloc(ptr, size);
    } else {
        // allocated by the standard malloc
        return __real_realloc (ptr, size);
    }
}

void __wrap_free(void *allocation)
{
    if (!allocation)
        return;

    if (MemArena::default_arena) {
        // allocated in the Mmap
        freeme(allocation);
    } else {
        // allocated by the standard malloc
        __real_free (allocation);
    }
}
#endif

void *operator new(size_t size) throw()
{
    if (MemArena::default_arena) {
        // Mmap's up: allocate in the Mmap
        return MemArena::default_arena
            ->allocate(size, MemArena::default_locale);
    } else {
        // Mmap's not yet up: allocate normally
        return __builtin_new(size);
    }
}

void operator delete(void *allocation)
{
    if (!allocation)
        return;

    if (MemArena::default_arena) {
        // allocated in the Mmap
        freeme(allocation);
    } else {
        // allocated by the standard malloc
        __builtin_delete (allocation);
    }
}

void *operator new(size_t size, void *where, MemArena *arena)
{
    return arena->allocate(size, where);
}

void *operator new[](size_t size, void *where, MemArena *arena)
{
    return arena->allocate(size,where);
}

// Global new[] and delete[]
void *operator new[] (size_t size) throw (std::bad_alloc)
{
    return operator new(size);
}

#if 0
void *operator new[] (size_t size, void *where) throw()
{
    return operator new(size);
}

void operator delete[] (void *what) throw()
{
    operator delete(what);
}
#endif

