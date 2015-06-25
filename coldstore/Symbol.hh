// Symbol - global name/value binding
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// $Id: Symbol.hh,v 1.29 2002/04/07 04:27:43 coldstore Exp $

#ifndef SYMBOL_HH
#define SYMBOL_HH

class Namespace;

/** A carrier for associations within a @ref Namespace
 *
 * Name is the carrier for associations between names within a Namespace.
 *
 * Names are unique within a @ref Namespace.  A Name is a (Namspace*,Slot)
 * pair.
 *
 * Names compare by address and not by value.
 * They are therefore an excellent choice for storage in @ref Hash tables
 * and such.
 *
 * Names may only be constructed in the context of a Namespace.
 */
class Directory;

#include "Tuple.hh"

class Name
    : public Tuple
{
  friend Directory;
protected: // Symbol needs
    // these are disallowed - will abort

    /** disallowed: coldstore constructor
     *
     * Symbols can only exist in the context of a @ref Namespace
     * so it makes no sense to create a Symbol without a @ref Namespace
     */
    Name(const Slot &name);
    Name(unsigned short len, const Slot &name);

    /** disallowed - cloning a symbol
     * Symbols are unique within a @ref Namespace
     * so it makes no sense to copy a Symbol
     */
    virtual Data *clone(void *where = (void*)0)const;
    
    /** disallowed - mutating a symbol
     * Symbols are inherently mutable,
     * and only mutable from @ref Namespace
     * that this has been called suggests a protocol violation
     */
    virtual Data *mutate(void *where = (void*)0) const;

    /** disallowed: it's meaningless
     */
    virtual Slot replace(const Slot &, const Slot &, const Slot &);

    /** disallowed: it's meaningless
     */
    virtual Slot replace(const Slot &, const Slot &);

protected:
    
    // These values are accessible via Tuple

    /** @ref Namespace within which this is defined */
    const Data /*Namespace*/ *_namespace;
    // this backward reference isn't a Slot because of circularity and refcounting

    /** name of Symbol (can be any type, usually a string */
    Slot _name;
    // Symbols can only be constructed by a Namespace
    friend class Namespace;
    virtual void changeName(const Slot& s);
    // change a Symbol's Namespace - WARNING: DANGEROUS IF USED ACROSS TREES
    virtual void changeSpace(Namespace* p);

public:

    /**
     *
     * Names can only exist in the context of a @ref Namespace
     * and need a name, so it makes no sense to create a default Symbol
     */
    Name();
    Name(unsigned short len);

    /** disallowed: copy constructor
     *
     * Names are unique within a @ref Namespace
     * so it makes no sense to copy a Name
     */
    Name(const Name &sym);
    Name(unsigned short len, const Name &sym);

    /** create a Name in a @ref Namespace
     * @param name Name's name
     * @param nspace Name's namespace
     */
    Name(const Slot &name, const Data /*Namespace*/ *nspace);
    Name(unsigned short len, const Slot &name, const Data /*Namespace*/ *nspace);


    /** destroy a Name
     */
    virtual ~Name();
    
    /** dump the object to a stream
     * @param ostream stream onto which to dump the object
     */
    virtual ostream &dump(ostream&) const;
    
    /** order two Names
     *
     * Names are always interned within some @ref Namespace.
     * This means they're unique, and immobile.
     *
     * They must still be compared by their names.
     *
     * @param arg Name to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual int order(const Slot &arg) const;

    /** compare two Names for strong equality
     *
     * Names are always interned within some @ref Namespace.
     * This means they're unique, and immobile.
     *
     * Therefore means they can be compared for strong equality on the
     * basis of their address, not their content.  This is fast.
     *
     * @param arg Name to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual bool equal(const Slot &arg) const;
    
    /** check an instance of Symbol for coherence
     *  (whatever that means to the datatype)
     * @param num number of contiguous instances in an array
     * @return void, but could be changed to return a bool
     */
    virtual void check(int num=0) const;	// check an instance of Data

    /** the namespace within which this Name is defined
     * @return defining Namespace
     */
    const Data /*Namespace*/ *space() const;

    /** this Name's name
     * convenience accessor
     * @return Name's name
     */
    Slot name() const {return _name;}

    /** this Name's name in the Directory (if there is one)
     * @param parent Directory to stop unparsing at
     * @return Name's hierarchical name
     */
    Slot dirName(const Directory* parent) const;

    /** generate constructor arguments sufficient to recreate this
     * does not include Namespace, as Namespace's toconstruct should include us
     * @return a primitive type (pickled) representation of this object
     */
    virtual Slot toconstruct() const;
  
  /** add a reference to this
   */
  virtual RefCount *upcount( void) const;
            
  /** remove a reference to this (possibly delete)
   */
  virtual void dncount( void) const;
};

class Symbol : public Name
{
protected:
    /** disallowed: default constructor
     *
     * Names can only exist in the context of a @ref Namespace
     * and need a name, so it makes no sense to create a default Symbol
     */
    Symbol();

    /** disallowed: construct from name
     *
     * Names can only exist in the context of a @ref Namespace
     * and need a name, so it makes no sense to create a default Symbol
     */
  //Symbol(const Slot &name);

    /** disallowed: copy constructor
     *
     * Names are unique within a @ref Namespace
     * so it makes no sense to copy a Name
     */
    Symbol(const Symbol &sym);
    /** Symbol's value (any type of Data) */
    Slot _value;
public:
    /** create a Symbol in a @ref Namespace
     * @param name Symbol's name
     * @param nspace Symbol's namespace
     * @param value Symbol's value
     */
    Symbol(const Slot &name, const Data /*Namespace*/ *nspace, const Slot &value);
    
    /** create a valueless Symbol in a @ref Namespace
     * @param name Symbol's name
     * @param nspace Symbol's namespace
     */
    Symbol(const Slot &name, const Data /*Namespace*/ *nspace);

    /** dump the object to a stream
     * @param ostream stream onto which to dump the object
     */
    virtual ostream &dump(ostream&) const;

    /** assign to Symbol
     * @param value value to be assigned
     * @return a copy of this with the assignment performed
     */
    virtual Slot replace(const Slot &value);

    /** this Symbol's value
     * convenience accessor
     * @return Symbol's value
     */
    Slot value() {return _value;}

#if 0
  WTF is this?  CMC
    /** reduction of Symbol is its value */
    virtual void reduce(Slot &inexpr, Slot &context) { 
      Node::Reduce( _value, context); 
      inexpr = _value; 
    };
#endif

    /** Assign a value to this Symbol
     * convenience mutator - equivalent to @ref Symbol::replace()
     * @return Symbol's new value
     */
    Slot operator=(const Slot &vale);

    /** generate constructor arguments sufficient to recreate this
     * does not include Namespace, as Namespace's toconstruct should include us
     * @return a primitive type (pickled) representation of this object
     */
    virtual Slot toconstruct() const;
};

/** associations between @ref Symbol names and Symbols
 */
class Namespace
    : public Data
{
    friend Name;
    Slot tree; // points to a Set

    /** disallowed:
     * it makes very little sense to consider a range of Symbols.
     *
     * Having said that, it could be used to clear a group of Symbols
     * I've seen no compelling use for it, so at the moment it's out.
     */
    virtual Slot replace(const Slot &, const Slot &, const Slot&);

    /** disallowed:
     * I don't really want to allow the deletion of a range of Symbols,
     * it just seems too powerful.
     */
    virtual Slot del(const Slot &, const Slot &);

    /** disallowed: copy connstruction of a Namespace */
    Namespace(Namespace &n); 

    /** clone this Namespace
     * Namespaces are inherently mutable, and their contents 
     * are specific to the Namespace,
     * so it makes no sense to clone one unless to duplicate all
     * Symbols ... which's a lot of work to no obvious purpose.
     */
    virtual Data *clone(void *where = (void*)0)const;

    /** drop a Name from this Namespace without deleting it
     */
    virtual Slot drop(const Slot& from);

public:
    /** construction of a Namespace */
    Namespace(); 

    /** construction of a Namespace from an association list, or map
     *
     * Namespaces can be constructed from a list of name,value pairs.
     */
    Namespace(const Slot &init); 

    /** generate constructor arguments sufficient to recreate this
     * @return a primitive type (pickled) representation of this object
     */
    virtual Slot toconstruct() const;

    /** destruction of a Namespace
     *
     * this will inevitably lead to the destruction of all Symbols
     * in the Namespace (won't it?)
     */
    virtual ~Namespace();

    /** make this mutable
     * Namespaces are inherently mutable anyway.
     * @return an identity - this.
     */
    virtual Data *mutate(void *where = (void*)0) const;

    /** return a Symbol's value
     * @param key the association key to be searched for
     * @return the value of any association found
     * @exceptions ~dictnf
     */
    virtual Slot slice(const Slot &key) const;

    /** assign a Symbol's value in this Namespace
     *
     * Throws if the Symbol's not already defined
     * @param key the Symbol's name
     * @param value the Symbol's value
     * @return the Symbol's new value
     * @exception ~dictnf
     */
    virtual Slot replace(const Slot &key, const Slot &value);

    /** zero out a symbol
     *
     * it makes very little sense to NULL out a @ref Symbol,
     * which is what this would seem to do. but we do it anyway.
     */
    virtual Slot replace(const Slot &);

    /** search for a symbol, returning NULL if it's not present
     *
     * @param search value to be found
     * @return value of association found, or NULL
     */
    Slot search(const Slot &what) const;

    /** create/assign a Symbol's value in this Namespace
     *
     * Creates a new Symbol with the given value.
     * Will change the value of an existing Symbol/Name.
     * @param key the Symbol's name
     * @param value the Symbol's value
     * @return the new Symbol
     */
    virtual Slot insert(const Slot &key, const Slot &value);

    /** create a Symbol in this Namespace
     *
     * Creates a new Symbol with the given value.
     * Does not change the value of an existing Symbol/Name.
     * @param key the Symbol's name
     * @return the Symbol's value
     */
    virtual Slot insert(const Slot &key);

    /** order two objects
     * @param arg object to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual int order(const Slot &arg) const;

    /** delete a Symbol from this Namespace
     *
     * Symbol must exist, or ~dictnf is thrown
     * @param from Symbol to delete
     * @return Symbol's value
     * @exception ~dictnf
     */
    virtual Slot del(const Slot &from);

    /** get length of collection
     * 
     * @return length of Namespace
     */
    virtual int length() const;

    /** get Symbol with rank
     * 
     * @param rank
     * @return item with rank
     */
    virtual Slot multiply(const Slot& arg) const;

    /** get rank of name
     *
     * @param name
     * @return rank of name
     */
    virtual Slot divide(const Slot& arg) const;
  
    /** get last Symbol whose name is less than the argument
     * 
     * @param name to compare with
     * @return found Symbol
     */
    virtual Slot lshift(const Slot& arg) const;
  
    /** get first Symbol whose name is greater than the argument
     * 
     * @param name to compare with
     * @return found Symbol
     */
    virtual Slot rshift(const Slot& arg) const;

    /** C++ extent placement new
        @param size size of allocation in bytes
        @param where neighborhood into which to allocate
        @return storage allocated as requested
     */

    void* operator new( size_t size, void* where=(void*)0) {
        return Memory::operator new(size, where);
    }

    /** C++ extent additional placement new
        @param size size of allocation in bytes
        @param extra extra allocation in bytes
        @param where neighborhood into which to allocate
        @return storage allocated as requested
     */
    void* operator new( size_t size, size_t extra, void* where=(void*)0) {
        return Memory::operator new(size, extra, where);
    }

    /** C++ object deletion
        @param ptr a pointer returned by @ref operator new()
        @param extra extra allocation in bytes
        @param where neighborhood into which to allocate
        @return storage allocated as requested
     */
    void operator delete(void* ptr) {
        Memory::operator delete(ptr);
    }

  /** add a reference to this
   */
  virtual RefCount *upcount( void) const;
            
  /** remove a reference to this (possibly delete)
   */
  virtual void dncount( void) const;

  /** dump the object to a stream
   * @param ostream stream onto which to dump the object
   */
  virtual ostream &dump(ostream&) const;

  /** make iterator for Namespace
   */
  virtual Slot iterator() const;

  virtual bool isMap() const { return true; };

  /** unary '~', invert the Namespace's Set
   */
  virtual Slot invert() const;

};

#endif
