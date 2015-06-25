// Startup - Server Initialization
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
//	$Id: Store.hh,v 1.5 2002/04/07 04:27:43 coldstore Exp $

#ifndef STORE_HH
#define STORE_HH
class Builtin;
class Namespace;

class Object;

// Put the roots of the Store here
class Store
    : public Data
{
    bool initialized;
public:
    // Well Known Objects
    tSlot<Object> System;
    tSlot<Object> Root;
    tSlot<Namespace> Names;
    tSlot<Namespace> Errors;	// distinguished namespace for Errors
    tSlot<Namespace> Types;	// distinguished namespace for all Data types
    tSlot<Set> Registered;	// registered Builtins
    Store();

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
};

/** Initialization called after Elf loads the Coldstore library
 */
class Elf;
void initElf(Elf *elf);

/** Function to register Csyms as Builtins in the Registered Namespace
 */
Set *RegisterElf(Elf *elf);

extern Store *store;

#endif
