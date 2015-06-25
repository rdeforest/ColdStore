// DopeV - Coldstore metadata
// Copyright (C) 2002 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: DopeV.hh,v 1.2 2002/04/13 03:53:47 coldstore Exp $	

#ifndef DOPEV_HH
#define DOPEV_HH
#include "Data.hh"

/** _DopeV is a precursor to DopeV, which is used to transmit metadata
    from OCC to the store.
*/
class _DopeV
{
    int inited;
public:
    _DopeV(char *name, void *vptr, size_t vtable_size, size_t class_size);
};

/** DopeV is a container for class metadata 
 */
class DopeV
    : public Data
{
public:
    char *name;
protected:
    void *vptr;
    size_t vptr_size;
    size_t class_size;

    /** _refcount shadows RefCount
	because DopeV can be instantiated multiple times
    */
    mutable size_t _refcount;

public:
    DopeV(char *n, void *v, size_t vs, size_t cs);

    /** list of all DopeVs used before Store is up
     */
    static tSlot<List> all;

    /** copy constructor
     *
     * Used as the destination of @ref mutate when a copy on write mutable
     * object is desired.
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual Data *clone(void *where = (void*)0) const;

    /** order two objects
     * @param arg object to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual int order(const Slot &arg) const;

    /** dump the object to a stream
     * @param ostream stream onto which to dump the object
     */
    virtual ostream &dump(ostream& output) const;

};

#endif
