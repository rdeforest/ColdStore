// String - Coldstore dynamic strings
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: String.hh,v 1.22 2002/02/27 01:49:31 coldstore Exp $	

#ifndef STRING_HH
#define STRING_HH

#include <Data.hh>
#include <Vector.hh>

union Char;
union uChar;

/** Dynamic shared String
 *
 */
class String
    : public Data,
      public Vector<Char>
{
    /** disallowed: has no meaning */
    virtual Slot replace(const Slot &val);

public:
    /** construct an empty String of a given size
     * @param size pre-allocation size for this
     */
    String(size_t size = 0);

    /** construct a String from a prefix of a char*
     * @param str char* for initial value
     * @param size substring size (0 == rest)
     */
    String(const char *str, int len=-1);

    /** construct a String from a substring of a String*
     * @param str char* for initial value
     * @param size substring size (0 == rest)
     */
    String(const String *str, ssize_t start=0, int l=-1);

    /** construct a String from a substring of a String&
     * @param str char* for initial value
     * @param size substring size (0 == rest)
     */
    String(const String &str, ssize_t start=0, int l=-1);
  
    /** construct a String from a substring of a String* via a Tuple arg */
    String(const Slot& s);

    /** construct a string from a (usually) alloca'd TupleBase */
    //String(TupleBase<Char> *ts);

    /** destroy String */
    virtual ~String();

    /** check String integrity
     * 
     * Checks memory Guard (if any)
     * and that string content is entirely 
     * within its allocation
     */
    virtual void check(int=0) const;
    
    /** copy constructor
     *
     * Used as the destination of @ref mutate when a copy on write mutable
     * object is desired.
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual Data *clone(void *where = (void*)0) const;

    /** create a mutable copy
     * 
     * @param where the locale into which to clone this object
     * @return mutable copy of object     
     */
    virtual Data *mutate(void *where = (void*)0) const;
    
    /** the String's truth value
     *
     * We consider a String to be true if it's not empty
     * @return the object's truth value
     */
    bool truth() const;

    
    /** constructor arguments sufficient to recreate object
     *
     * String is a basic type: it is its own constructor
     * @return identity
     */
    Slot toconstruct() const;

    /** dump the String to a stream
     * @param ostream stream onto which to dump the object
     */
    virtual ostream &dump(ostream&) const;
    
    /** order two Strings
     * @param arg String to be ordered relative to this
     * @return 1,0,-1 depending on String order
     */
    virtual int order(const Slot &arg) const;

    /** equality operator
     * @param arg String to be compared to this
     * @return true/false depending on equality
     */
    virtual bool equal(const Slot &arg) const;
    
    /** predicate - is a String a sequence?
     * @return constant true
     */
    bool isSequence() const;

    /** String length
     * @return length of String
     */
    virtual int length() const;

    /** concatenate two Strings
     * @param arg String to concatenate to this
     * @return a String with arg concatenated to this
     */
    virtual Slot concat(const Slot &arg) const;

    virtual Slot add(const Slot &arg) const;

    /** substring of String
     *
     * Note that we use copy-on-write, so a substring
     * points into the same storage as the string it's a substring of.
     * @param start index of start of substring
     * @param len length of substring
     * @return the range of elements of this sequence 'from' for 'len'
     */
    virtual Slot slice(const Slot &start, const Slot &len) const;

    /** element of String
     *
     * Note that we use copy-on-write, so a substring
     * points into the same storage as the string it's a substring of.
     * @param start index of start of substring
     * @param len length of substring
     * @return a string containing the single character at start
     */
    virtual Slot slice(const Slot &start) const;


    /** replace a String subrange with a String
     * @param from replacement-range start index
     * @param len replacement-range length
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &from, const Slot &len, const Slot &value);

    /** replace a String element with a String
     * @param from replacement-range start index
     * @return a copy of this with the replacement performed
     */
    virtual Slot replace(const Slot &from, const Slot &val);

    /** insert a String into this String at a given position
     * @param value value to be inserted in this sequence
     * @param before element index before which insertion is performed
     * @return a copy of this with the replacement performed
     */
    virtual Slot insert(const Slot &from, const Slot &val);

    /** insert a String at the end of this String
     *
     * This is really redundant with @ref String::insert
     * @param value value to be inserted in this String
     * @return a copy of this with the replacement performed
     */
    virtual Slot insert(const Slot &val);

    
    /** delete a substring from this String
     * @param from del-range start index
     * @param len del-range length
     * @return a copy of this with the deletion performed
     */
    virtual Slot del(const Slot &from, const Slot &len);

    /** delete an element from this String
     * @param from del-element index
     * @return a copy of this with the deletion performed
     */
    virtual Slot del(const Slot &from);
    
    /** search for a matching substring
     * @param search element for which to search
     * @return the first of this String with search as a prefix
     */
    virtual Slot search(const Slot &search) const;

    /** Iterator over this sequence - identity
     * @return an Iterator
     */
    Slot iterator() const;

    /** convert String to sequence - identity
     */
    List *toSequence() const;

    /** predicate: is iterated sequence complete?
     * @return true iff iterator is not exhausted
     */
    bool More() const;

    /** advance iterator over sequence
     * @return next iterated element
     */
    Slot Next();

    // local
    /** convert to a C string */
    operator char *() const;

    /** encrypt String */
    String *crypt(Slot &key);

    /** convert String to upper case */
    String *uppercase();

    /** convert String to lower case */
    String *lowercase();

    /** TODO: substitute a list into the String */
    //virtual Slot multiply(const Slot &arg);	// dyadic `*', multiply

    // Split a String by substring
    Slot explode(const String *by) const;
    virtual Slot divide(const Slot &arg) const;

    /** null terminate a String
     */
    String *nullterm(char term = 0);
    virtual Slot positive();

    // prefix operations
    /** are we a prefix of the parameter? */
    bool isPrefix(const Slot &s);
    /** return everything after us in a string we are a prefix of */
    Slot killPrefix(const Slot& s);
    /** return largest common prefix */
    Slot commonPrefix(const Slot& s);
    /** return length of largest common prefix */
    int lenCommonPrefix(const Slot& s);
};

ostream& operator<< (ostream& out,  const String &str);

/** caseless @ref String
 *
 * UString is just like String, but all comparisons are
 *  performed without regard to case.
 */
class UString
    : public String
{
    
public:
    /** construct a String from a prefix of a char*
     * @param str char* for initial value
     * @param size substring size (0 == rest)
     */
    UString(const char *str, int len=0);

    /** construct a String from a substring of a String&
     * @param str char* for initial value
     * @param size substring size (0 == rest)
     */
    UString(const UString &str, ssize_t start = 0, ssize_t l = 0);

    /** destroy String */
    virtual ~UString();
    
    /** copy constructor
     *
     * Used as the destination of @ref mutate when a copy on write mutable
     * object is desired.
     * @param where the locale into which to clone this object
     * @return mutable copy of object
     */
    virtual Data *clone(void *where = (void*)0) const;
    
    /** order two Strings (ignoring case)
     * @param arg String to be ordered relative to this (ignoring case)
     * @return 1,0,-1 depending on String order
     */
    virtual int order(const Slot &arg) const;

    /** equality operator (ignoring case)
     * @param arg String to be compared to this (ignoring case)
     * @return true/false depending on equality
     */
    virtual bool equal(const Slot &arg) const;
    
    /** search for a matching substring ignoring case
     * @param search element for which to search
     * @return the first of this String with search as a prefix
     */
    virtual Slot search(const Slot &search) const;
};

#endif

