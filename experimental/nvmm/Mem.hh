#ifndef MEM_HH
#define MEM_HH

/// virtual base class for all arenas

// A MemArena is an allocator for MemSegments
class MemArena
{
public:
    /** allocate a region of the given size in the given locale
     * @param size minimum internal size of region to allocate
     * @param locale meaningless cookie, argument to underlying implementation, hint, whatever
     * @return void * to start of internal space of at least size.
     */
    virtual void *allocate(size_t size, void *locale = NULL) = 0;
    
    /// return an allocated region to the arena
    virtual void unallocate(void *ptr) = 0;
    
    /** is MemArena guarded?
     * @return true iff a Guard pair is laminated onto each allocation
     */
    virtual bool isGuarded();
    
    /// reallocate a region
    virtual void *realloc(void *addr, size_t new_size, void *locale = NULL);
    
    /// arena within which standard malloc() occurs
    static MemArena *default_arena;
    
    /// locale for standard malloc()
    static void *default_locale;
};

/// generic QV memory allocation segment

// MemSegment represents some chunk of memory which has been allocated by some MemArena

// Having been allocated, a MemSegment can be unallocated(), and can report the following
// characteristics: Start() address, Size() in bytes, Arena() from which it was allocated,
// Locale() of Arena() in which it was allocated, and the address-range Page(s) it occupies
class MemSegment
{
public:
    /// remove this Segment from its arena
    virtual void unallocate() = 0;
    
    /// start of region
    virtual MemSegment **Start() = 0;

    /// size of region
    virtual size_t Size() = 0;

    /// arena containing region
    virtual MemArena *Arena() = 0;

    /// locale of region
    virtual void *Locale() = 0;
    
    /// page of the start of this region
    char *startPage();

    /// page of the end of this region
    char *endPage();
    
    /** is this in a given page
        @param page the page to compare against
    */
    bool inPage(void *page);

protected:
    virtual ~MemSegment() {}

    /// install a pointer to the MemSegment controller in the region specified by this MemSegment.
    void *install_ptr();
};

#endif
