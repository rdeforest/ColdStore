#ifndef NEW_HH
#define NEW_HH

/// extent placement default array new operator using store
void *operator new[] (size_t size, void *where, MemArena *arena = NULL);

/// extent placement default new operator using store
void *operator new (size_t size, void *where, MemArena *arena = NULL);

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

MemArena *MemArena::default_arena = NULL;
void *MemArena::default_locale = NULL;

#endif
