#include "Mem.hh"

// turns any void* into a MemSegment*, and frees it.
static void freeme(void *allocation)
{
#ifdef GUARD_ALLOC
    allocation = checkGuards(allocation, true);
#endif

    allocation = ((void**)allocation)-1;	// get the MemSegment
    MemSegment *segment = dynamic_cast<MemSegment*>((MemSegment*)allocation);
    if (!segment)
        throw runtime_error("freeing unallocated segment");

    segment->unallocate();	// finally free the thing.
}

void *MemArena::realloc(void *addr, size_t new_size, void *locale)
{
    if (!addr)
        return allocate(new_size, locale);
    if (!new_size) {
        freeme(addr);
        return NULL;
    }

    void *allocator = addr;

#ifdef GUARD_ALLOC
    allocator = checkGuards(addr, true);
#endif

    allocator = ((void**)allocator)-1;	// get the MemSegment

    MemSegment *segment = dynamic_cast<MemSegment*>((MemSegment*)allocator);
    if (!segment)
        throw runtime_error("reallocing unallocated segment");

    // round up requested size
    new_size = (new_size + 2*sizeof(void*) - 1) & ~(sizeof(void*)-1);
    void *result = allocate(new_size, locale);

    // get changed locale
    if (!locale)
        locale = segment->Locale();

    // discover true internal size of allocation
    int old_size = segment->Size() - sizeof(void*);
#ifdef GUARD_ALLOC
    old_size -= sizeof(Guard) * 2;	// adjust for Guards
#endif

    memcpy(result, addr, old_size);
    return result;
}

const size_t Mmap::page_size = getpagesize();

/// install a pointer to the MemSegment controller in the region specified by this MemSegment.
void *MemSegment::install_ptr()
{
    MemSegment **lp = Start();
    *lp++ = this;
    return (void*)lp;
}

char *MemSegment::startPage()
{
    return (char*)Mmap::pageof(Start());
}

char *MemSegment::endPage()
{
    return (char*)Mmap::pageof(Start() + Size());
}

bool MemSegment::inPage(void *ptr)
{
    return Mmap::pageof(ptr) == Mmap::pageof(Start());
}
