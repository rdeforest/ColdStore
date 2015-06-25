#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>

#include "mmap.hh"
#include "SkipList.hh"
#include "QVmm.hh"

Leaf *QVmm::shrink(Leaf &leaf, void *locale, size_t size)
{
    assert(leaf.locale == no_alloc);
    assert(leaf.size >= size);
    leaf.size -= size;	// reduce size of freespace allocation

    // add allocation leaf
    Leaf allocated(this, locale, leaf.addr + leaf.size, size);
    SkipEl<Leaf> *result;
    Insert(allocated, &result);
    
    if (!leaf.size)
        Delete(leaf);	// remove leaf from arena
    return (Leaf*)result;
}

SkipEl<Leaf> *QVmm::First(const void *locale)
{
    Leaf probe(this, locale);	// null address, specified locale
    SkipEl<Leaf> *found = Find(probe);
    if (found) {
        // found <= Leaf < found->next
        if ((found = found->Advance())) {
            if ((*found)->locale == locale) {
                return found;
            }
        }
    }
    return NULL;
}

Leaf *QVmm::near_alloc(void *locale, char *page, size_t size)
{
    Leaf probe(this, no_alloc, page);
    for (SkipEl<Leaf> *found = Find(probe);
         found
             && ((*found)->locale == no_alloc);
         found = found->Advance())
    {
        if (((*found)->size >= size)
            && (*found)->inPage(page))
            return shrink(*found, locale, size);
    }
    return NULL;
}

Leaf *QVmm::first_alloc(size_t size, void *locale)
{
    for (SkipEl<Leaf> *found = First(no_alloc);
         found && ((*found)->locale == no_alloc);
         found = found->Advance())
    {        
        if ((*found)->size >= size) {
            return shrink(*found, locale, size);
        }
    }
    return NULL;
}

Leaf *QVmm::locale_alloc(size_t size, void *locale)
{
    // search for first allocated in locale
    for (SkipEl<Leaf> *found = First(locale);
         found && ((*found)->locale == locale);)
    {
        // search for a no_alloc on same page as start of found region
        char *page = (*found)->startPage();
        Leaf *result = near_alloc(locale, page, size);
        if (result) {
            found->Unlock();
            return result;
        }
        
        // search for a no_alloc on same page as end of found region
        if (page != (*found)->endPage()) {
            page = (*found)->endPage();
            if ((result = near_alloc(locale, page, size))) {
                found->Unlock();
                return result;
            }
        }
        
        // skip along found list ignoring neighbors on same page
        while ((found = found->Advance())
               && ((*found)->locale == locale)
               && ((*found)->startPage() == page));
        
    }
    
    return NULL;
}

Leaf *QVmm::map_alloc(size_t size, void *locale)
{
    // carve out a chunk of new allocation from the Mmap
    size_t tsize = Mmap::align(size);
    char *ptr = Mmap::alloc(tsize);

    if (!ptr) {
        return NULL;	// out of memory
    }

    // feed the new block(s) to this arena
    SkipEl<Leaf> *result;
    Leaf newblock(this, locale, ptr, size);
    Insert(newblock, &result);
    int over = tsize - size;
    if (over) {
        Leaf newspace(this, no_alloc, ptr + size, over);
        Insert(newspace);
    }
    return (Leaf*)result;
}

void *QVmm::allocate(size_t size, void *locale = NULL)
{
    Leaf *result;
    // try each of the allocation tactics in order of desirability
    bool done = (result = locale_alloc(size, locale))
        || (result = near_alloc(locale, (char*)locale, size))
        || (result = first_alloc(size, locale))
        || (result = map_alloc(size, locale));
    
    if (done)
        return (void*)result->install_ptr();
    else
        return NULL;
}

void QVmm::unallocate(void *ptr)
{
    Leaf *leaf = (Leaf*)ptr;
    leaf--;
    assert(leaf->arena == this);

    char *addr = leaf->addr;
    size_t size = leaf->size;
    SkipEl<Leaf> *found = Delete(*leaf);
    if (found) {
        (*found)->locale = no_alloc;
        (*found)->addr = addr;
        (*found)->size = size;
        Insert(*found);
    }
}

Leaf::Leaf() {}

Leaf::Leaf(MemArena *_arena, const void *_locale,
         char *_addr, size_t _size)
        : locale(_locale), addr(_addr), size(_size), arena(_arena)
{}

bool Leaf::operator < (const Leaf &l) const
{
    return locale < l.locale
        || ((locale == l.locale) && (addr < l.addr));
}

bool Leaf::operator == (const Leaf &l) const
{
    return locale == l.locale
        && (addr == l.addr);
}

bool Leaf::operator > (const Leaf &l) const
{
    return locale > l.locale
        || ((locale == l.locale) && (addr > l.addr));
}

#include "SkipList.th"

template class SkipEl<Leaf>;
template class SkipList<Leaf>;
template union lptr<Leaf>;
