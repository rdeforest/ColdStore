// 
#include "new.hh"

struct Leaf
    : public MemSegment
{
    const void *locale;	/// locale of region - key1
    char *addr;		/// start of region - key2
    size_t size;	/// size of region
    MemArena *arena;	/// allocation arena of region

public:
    Leaf(MemArena *_arena, const void *_locale,
         char *_addr = NULL, size_t _size = 0);
    Leaf();

    bool operator < (const Leaf &l) const;
    bool operator == (const Leaf &l) const;
    bool operator > (const Leaf &l) const;

    /// remove this Leaf from its arena
    virtual void unallocate();
    
    MemArena *Arena() {
        return arena;
    }
    
    void *Locale() {
        return (void *)locale;
    }
    
    MemSegment **Start() {
        return (MemSegment**)addr;
    }
    
    size_t Size() {
        return size;
    }
};

/** An Allocation Arena which attempts to maximise Spatial Locality of Reference
    by allocating within locales (which are collection of pages containing similar allocations)
*/
class QVmm
    : public MemArena,
      SkipList<Leaf>
{
    /// locale for unallocated regions
    static const void * no_alloc;

    /** construct a QVmm arena */
    QVmm();
    
    /** shrink in situ or remove a Leaf, adding new allocation in its place.
        @param leaf unallocated leaf to replace/divide
        @param locale locale of the new allocation
        @param size size of the new allocation
        @return new allocation Leaf
     */
    Leaf *shrink(Leaf &leaf, void *locale, size_t size);
    
    /** lock and return first element allocated in the given locale.
        @param locale locale to search for
        @return locked region's Leaf element or NULL
    */
    SkipEl<Leaf> *First(const void *locale);
    
    /** allocate a region somewhere on a given page.
        @param locale locale of region
        @param page page on which to allocate
        @param size size of allocation
        @return region allocated or NULL
    */
    Leaf *near_alloc(void *locale, char *page, size_t size);

    /** allocate the first acceptable free block.
        @param size size of region to allocate
        @param locale locale to assign to allocated region
        @return region allocated or NULL
     */
    Leaf *first_alloc(size_t size, void *locale);
    
    /** allocate a region collocated on a page with a aomne other locale neighbor.
        @param size size of region to allocate
        @param locale locale to assign to allocated region
        @return region allocated or NULL
     */
    Leaf *locale_alloc(size_t size, void *locale);
    
    /** resort to growing the memory.
        @param size size of region to allocate
        @param locale locale to assign to allocated region
        @return region allocated or NULL
     */
    Leaf *map_alloc(size_t size, void *locale);
    
    /** allocate a given sized block to a given locale.
        The following allocation tactics are used, in order:
        1) allocate on a page already occupied by something in this locale,
        2) allocate near the locale itself,
        3) allocate first fit,
        4) allocate on grown heap.
        @param size size of region to allocate
        @param locale locale to assign to allocated region
        @return region allocated or NULL
     */
    void *allocate(size_t size, void *locale = NULL);
    
    /** return a block to the heap
        @param pointer to a location which was previously allocated in this arena
     */
    void unallocate(void *ptr);
};

const void *QVmm::no_alloc = (void*)-1;
