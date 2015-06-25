// tSlot - Coldstore templated smartpointers
// Copyright (C) 2001 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: tSlot.hh,v 1.5 2002/04/11 03:16:32 coldstore Exp $	

#ifndef TSLOT_HH
#define TSLOT_HH

union Slot;
class Data;

// a child of Slot which enforces the type of its referent
template <class T>
union tSlot
{
protected:
    mutable T *data_;

public:
    tSlot(const Slot &s_);
    tSlot(const Data *d = 0);
    tSlot(const T *d);
    tSlot(const tSlot<T>& f);	// copy constructor - dups reference
#if 0
    /** assignment operator.  Initializes tSlot
        @param f tSlot& to which to assign this 
    */
    tSlot<T>& operator= (const tSlot<T>& f);
#endif

    /** assignment operator.  Initializes tSlot
        @param d Data* to which to assign this
     */
    tSlot<T>& operator= (T *d);

    /** assignment operator.  Initializes tSlot
        @param d Slot to which to assign this
     */
    tSlot<T>& operator= (Slot d);

#if 0
    // dereference smartpointer
    operator Slot() const;
#endif
    // dereference smartpointer
    operator T*() const;

    // dereference smartpointer
    T* value() const;

    // dereference smartpointer - returns const
    T *operator->() const;

  //operator Slot() const;
};
#endif
