#include <sys/types.h>

//#define DEBUG

template <class T> class SkipEl;
template <class T> class SkipList;

const unsigned int cnmask = ~1;
/** locked smartpointer.
    lptr is a smartpointer which can be atomically locked against updates.
    This is effected by stealing the lowest order bit, and having it indicate
    that the pointer is locked against updates.  This should present no problem if
    we can guarantee that allocations are word-aligned (which we can.)
 */
template <class T>
union lptr
{
    /** pointer with lock.
        bit 0 of this pointer indicates whether its value is locked or not.
     */
    ulong lock_ptr;

    /// convenience rendering as a pointer
    SkipEl<T> *ptr;

    /// atomic compare-and-swap operation
    static bool CAS (ulong *p, ulong oldval, ulong newval);

public:
    /// construct an lptr (initially locked)
    lptr();

    /// lock this pointer
    void lock();

    /// unlock this pointer
    void unlock();

    /// return the pointer value of this lptr
    operator SkipEl<T> *();

    /// assign a pointer value to this (must hold lock)
    SkipEl<T> *operator = (SkipEl<T> *what);

    /// assign another lptr to this (must hold lock)
    SkipEl<T> *operator = (lptr what);

    /// predicate: is this lptr locked?
    bool locked();
};

/** Generic thread-safe sorted singly-linked list element
    This element can contain an arbitrary object and can be maintained in a sorted list,
    controlled by @see SkipList
 */
template <class T>
class SkipEl
{
    friend SkipList<T>;

    /// content of SkipEl element
    T key;
    lptr<T> garbage;
    
#ifdef DEBUG
    pthread_t locker[10];
#endif
    /// number of levels in this element
    short maxlevel;
    
    /// array of pointers to next SkipEl element in lists
    lptr<T> forward[10];

    //// comparison of contents
    virtual bool operator < (T &comp);

    //// comparison of contents
    bool operator > (T &comp) {
        return !(*this < comp) && !(*this == comp);
    }

    //// equality of contents
    virtual bool operator == (T &comp);

protected:
    /** search for an insertion point at a given level.
        
        @return (by reference) a pair of elements (prev,next) such that prev < searchKey <= next
        @param searchKey content to search for
        @param l level at which to search
        @param prev previous element in sort order
        @param next element in sort order
    */
    void WeakSearch(T searchKey, ushort l, SkipEl<T> *&prev, SkipEl<T> *&next);
    
    /** search for an insertion point at a given level, leaving it locked.

        @return (by reference) a pair of elements (prev,next) such that prev < searchKey <= next
        @param searchKey content to search for
        @param l level at which to search
        @param prev previous element in sort order
        @param next element in sort order
     */
    void StrongSearch(T searchKey, ushort l, SkipEl<T> *&prev, SkipEl<T> *&next);

    /** search for and return insertion point at a given level, leaving it locked.

        @return insertion point
        @param searchKey content to search for
        @param l level at which to search
     */
    SkipEl<T> *getLock(T searchKey, ushort l);
   

public:
    
    /** construct a SkipEl element with a given level.
        @param k contents of this element
        @param level level of this element
    */
    SkipEl(T &k, ushort level);
    
    /** construct an empty locked list element.
        @param level level of this element
    */
    SkipEl(ushort level);
    
    /** get content of element

        @return reference to content
     */
    operator T&();

    /** assign to content of SkipEl element
        Note: if this changes sort order, all bets are off
        @param val value to assign to element's content
        @return value assigned
     */
    T &operator = (T&val);
    
    /** lock SkipEl forward pointer at level l.
        @param l level to lock
    */
    void lock(ushort l);

    /** unlock SkipEl forward pointer at level l
        @param l level to lock
    */
    void unlock(ushort l);
};

/** Generic Thread-Safe Sorted Singly Linked List
 */
template <class T>
class SkipList
    : public SkipEl<T>	// list header
{
    //// comparison of contents
    virtual bool operator < (T &comp);

    //// equality of contents
    virtual bool operator == (T &comp);

    // stuff for random
    static const short BitsInRandom = 31; // how many bits does a call to random() yield?
    static uint randomBits;	// repository for random bits (saves re-calling random)
    static short randomsLeft;	// number of bits left in random repository

    /** randomLevel - choose a random level 0..maxLevel for a node.
        the distribution has the property that higher levels are geometrically less probable
        @return a random level within a distribution
    */
    ushort randomLevel();

public:

    /// construct an empty unlocked list
    SkipList(ushort level);

    /** insert content.
        
        @param searchKey content to insert
        @return true if new insertion, false if replacement
    */
    bool Insert(T searchKey);

    /** delete content from the skiplist.
        The returned node can be deleted any time after the completion of all
        searches/insertions/deletions that were in progress when the node was deleted.

        @param searchKey content to delete
        @return element deleted from list, NULL if no match
     */
    SkipEl<T> *Delete(T searchKey);

    /** search for a match at a given level.
        The result returned is guaranteed to have been true at some point during the execution of the search.

        @param searchKey content to search for
        @param l level at which to search
        @return matching element or NULL if none
     */
    SkipEl<T> *Search(T searchKey);
};
