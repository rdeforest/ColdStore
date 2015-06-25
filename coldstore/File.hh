// File - class for local file access
// Copyright (C) 2000,2001 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef FILE_HH
#define FILE_HH

#include <fstream.h>
#include <Data.hh>
#include <Q.hh>

/** File access
 *
 */
class File
    : public Data,
    public Q<File>
{
protected:
    Slot name;
    int mode;
    int prot;
    mutable fstream *stream;
    mutable Slot lastread;
    static Qh<File> all;	// set of all Files (need to be destroyed :)

    /** destroy File instance
     */
    virtual ~File();			// destruction via dncount(), please
    
public:
    
    File(const Slot &file);
    File(const char *path, int mode, int prot);
    File(fstream &f);
    ////////////////////////////////////
    // structural

protected:
    /** copy constructor
     *
     * Used as the destination of @ref mutate when a copy on write mutable
     * object is desired.
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual File *clone(void *where = (void*)0) const;
    
public:
    /** create a mutable copy
     * 
     * Depending upon whether the object is considered a mutable instance or
     * a copy-on-write instance, vtbl magic is used to redirect this call to
     * one of @ref identity or @ref clone
     *
     * @param where the locale into which to clone this object
     * @return mutable copy of object     
     */
    virtual File *mutate(void *where = (void*)0)const;

    ////////////////////////////////////
    // object

    /** the object's truth value
     *
     * mapping of all objects onto true/false
     * @return the object's truth value
     */
    virtual bool truth() const;

    /** constructor arguments sufficient to recreate object
     * @return a primitive type (pickled) representation of this object
     */
    virtual Slot toconstruct() const;

    /** dump the object to a stream
     * @param ostream stream onto which to dump the object
     */
    virtual ostream &dump(ostream& output) const;

    /** construct an instance of File from the constructor args given
     * @param arg constructor arguments, derived from @ref toconstruct
     * @return instance constructed from the args
     */
    static Slot construct(const Slot &arg);

    /** order two objects
     * @param arg object to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual int order(const Slot &arg) const;

    /** equality operator
     * @param arg object to be compared to this
     * @return true/false depending on equality
     */
    virtual bool equal(const Slot &arg) const;

#if 0
    ////////////////////////////////////
    // arithmetic

    /** monadic `+', absolute value
     */
    virtual Slot positive() const;

    /** monadic `-', negative absolute value
     */
    virtual Slot negative() const;

    /** dyadic `+', add
     */
    virtual Slot add(const Slot &arg) const;

    /** dyadic `-', subtract
     */
    virtual Slot subtract(const Slot &arg) const;

    /** dyadic `*', multiply
     */
    virtual Slot multiply(const Slot &arg) const;

    /** dyadic '/', divide
     */
    virtual Slot divide(const Slot &arg) const;

    /** dyadic '%', modulo
     */
    virtual Slot modulo(const Slot &arg) const;
  
    ////////////////////////////////////
    // bitwise

    /** unary '~', invert
     */
    virtual Slot invert() const;

    /** dyadic '&', bitwise and
     */
    virtual Slot and(const Slot &arg) const;

    /** dyadic '^', bitwise xor
     */
    virtual Slot xor(const Slot &arg) const;

    /** dyadic '|', bitwise or
     */
    virtual Slot or(const Slot &arg) const;

    /** dyadic '<<', left shift
     */
    virtual Slot lshift(const Slot &arg) const;

    /** dyadic '>>', right shift
     */
    virtual Slot rshift(const Slot &arg) const;
#endif

    /** length as sequence
     */
    virtual int length() const;

    ////////////////////////////////////
    // Return range

    /** return a range
     * @param from index of start of range
     * @param len length of range
     * @return the range of elements of this sequence 'from' for 'len'
     */
    virtual Slot slice(const Slot &from, Slot &len) const;

    /** return an element
     * @param element element index
     * @return the nominated element of this sequence
     */
    virtual Slot slice(const Slot &element) const;

    ////////////////////////////////////
    // Replacement of range

    /** replace an element with a value
     * @param value value to be replaced in this sequence
     * @param element element index to be replaced
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &element, const Slot &value);

    /** replace this with a value
     * @param value value to be replaced in this sequence
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &value);

    ////////////////////////////////////
    // iterator

    /** an iterator over this sequence
     * @return an Iterator
     */
    virtual Slot iterator() const ;

    /** predicate - is object an iterator
     */
    virtual bool isIterator() const;

    /** predicate: is iterated sequence complete?
     */
    virtual bool More() const;

    /** advance iterator over sequence
     */
    virtual Slot Next();


    ////////////////////////////////////
    // iterator

    /** slurp in the whole file as a List
     */
    Slot read_file();
};

#endif
