// Pool - maintain an allocation pool of templated type
// Copyright (C) 2000 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id $

#ifndef ALLOCA_HH
#define ALLOCA_HH

template <class T>
class Alloca
    : public T
{
public:
    /** Alloca new
        @param size length of the main allocation
        @param location alloca()d location of storage
        @param extra extra allocation in bytes
        @return storage allocated as requested
    */
    void *operator new(size_t size, void *location) {
        memset(location, '\0', size);
        return location;
    };

    /** Alloca delete
        @param ptr a pointer returned by @ref new()
        @param extra extra allocation in bytes
        @param where neighborhood into which to allocate
    */
    void operator delete(void* ptr) {}

    // Some constructors for normal Data
    Alloca() {}
    Alloca(Slot&s):
            T(s) {}
    virtual ~Alloca() {}
    void dncount(void) const {
        Data::dncount();
        if (!refcount()) {
            this->~Alloca();
        }
    }

    T *enclosed() {
        return dynamic_cast<T*>(this);
    }
};

#endif ALLOCA_HH
