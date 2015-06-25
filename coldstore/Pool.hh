// Pool - maintain an allocation pool of templated type
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id $

#ifndef POOL_HH
#define POOL_HH

// extensive/expensive check of Pools
#define POOL_CHECK 1

// instantiate templates
#include <Q.hh>
template <class T> class Pool;

/** Pooled wrapper around types which is used to allocate them from a Pool
 * primary utility is to permit delete() on Pooled objects
 */
template <class T>
class Pooled
    : public T
{
protected:
    friend class Pool<T>;
    void *operator new(size_t size, void *location);

    union {
        Pool<T> *pool;
        int next;	// next Pooled in Pool
    };
    
public:
    Pooled();
    Pooled(Slot&);
    virtual ~Pooled();
    T *enclosed() {
        return dynamic_cast<T*>(this);
    }
    /** C++ object Pool deletion
        @param ptr a pointer returned by @ref Pool#Get
        @param extra extra allocation in bytes
        @param where neighborhood into which to allocate
        @return storage allocated as requested
    */
    void operator delete(void* ptr);
    T* unPool(void* loc);
    static T* unPool(T* p, void* loc=NULL);
};

/** Pool allocator
 * Pools are arrays of space for objects of a given type, allocation and release of objects is quite fast
*/
template <class T>
class Pool
    : public Memory
{
public:
    static const size_t def_size = (Mmap::page_size / sizeof(Pooled<T>))-1;

protected:
    Pooled<T> content[def_size];

    int count;
    int first;

    friend class Pooled<T>;
    void release(Pooled<T> *el);

public:
    /** construct a Pool of a given size
     * @param size the number of elements of Pool's type to reserve space for
     */
    Pool(size_t size = Pool::def_size);

    /** destroy a Pool
     * it is an error to destroy a non-empty Pool
     */
    virtual ~Pool();

    /** get an instance of the Pooled type
     * @return an instance, via default constructor, NULL if there's no more space
     */
    virtual T* get();

    /** get an instance of the Pooled type
     * @return an instance, via Slot&, NULL if there's no more space
     */
    virtual T* get(Slot&);
    bool check(int=0) const;
};

/** Global Pool allocator
 * Global Pools for a given type grow to accomodate more instances
*/
template <class T>
class gPool
    : public Pool<T>,
      public Q< gPool<T> >
{
    static Qh< gPool<T> > all;	// Q of all Pools of Integer
public:
    gPool();
    virtual ~gPool();

    /** get an instance of the Pooled type
     * @return an instance, via default constructor
     */
    static T* Get();
    /** get an instance of the Pooled type
     * @return an instance, via Slot& constructor
     */
    static T* Get(Slot&);
};

#endif

