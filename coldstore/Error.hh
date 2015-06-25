// Error.hh - ColdStore interface to Error
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef ERROR_HH
#define ERROR_HH

#include "Data.hh"
#include "Tuple.hh"

class Error
    : public Tuple
{
    // NB: Tuple overlays the following
    /** the error id */
    Slot _err;

    /** the value in error (or nil) */
    Slot _value;

    /** an explanation of the error (free-form) */
    Slot _explanation;
    
public:
    /** create an Error
     * @param err error identifier
     * @param value value occasioning error
     * @param expl textual explanation of error
     */
    Error(const char *err, const Slot value, Slot expl=(Data*)0);

    /** duplicate an error
     */
    Error(const Error *err);
    Error(const Slot &err);

    virtual ~Error();
    
public:
    /** copy constructor
     *
     * Used as the destination of @ref mutate when a copy on write mutable
     * object is desired.
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual Data *clone(void *store = (void*)0) const;

    /** create a mutable copy
     * 
     * Depending upon whether the object is considered a mutable instance or
     * a copy-on-write instance, vtbl magic is used to redirect this call to
     * one of @ref identity or @ref clone
     *
     * @param where the locale into which to clone this object
     * @return mutable copy of object     
     */
    virtual Data *mutate(void *store = (void*)0) const;

    /** dump the object to a stream
     * @param ostream stream onto which to dump the object
     */
    virtual ostream &dump(ostream&) const;	// dump the object to a stream

    /** the object's truth value
     *
     * Errors are always considered to be false
     * @return false
     */
    virtual bool truth() const;

    /** cast the Error to its Symbolic identifier */
    operator Symbol *() {
        return _err;
    }
};

#endif

