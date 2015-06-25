/* -*- C++ -*- */
#include <typeinfo>
#include <qvmm.h>
#include "Symbol.hh"

#ifndef NAMEDNAMESPACE_HH
#define NAMEDNAMESPACE_HH

/*
a Namespace is detached (refcounted as normal)
a Name is committed to its parent
the entire tree structure is refcounted as one great lump
so a NamedNamespace must be committed
*/
/* all NamedNamespace's have parent Namespaces */
class NamedNamespace : public Name , public Namespace
{
public:
  /* constructors similar to Name constructors */
  NamedNamespace();
  NamedNamespace(const Slot& name);
  NamedNamespace(const Slot& name, const Data* nspace);
  /* construction from list or map */
  NamedNamespace(const Slot& name, const Data* nspace, const Slot& init);
  /* destructor - recursively kill contained symbols */
  virtual ~NamedNamespace();
  virtual ostream &dump(ostream& o) const;
  virtual int order(const Slot& arg) const // as Name
    {return Name::order(arg);}
  virtual bool equal(const Slot& arg) const
    {return Name::equal(arg);}
  virtual Slot insert(const Slot& key, const Slot& value)
    {return Namespace::insert(key,value);}
  virtual Slot insert(const Slot& key)
    {return Namespace::insert(key);}
  virtual Slot del(const Slot& key)
    {return Namespace::del(key);}
  virtual Slot search(const Slot &what) const
    {return Namespace::search(what);}
  // no need to override del,insert,search
  virtual Slot toconstruct() const;
    void* operator new( size_t size, void* where=(void*)0) {
        return Memory::operator new(size, where);
    }
  static Slot construct(const Slot& arg) { return (Data*)(Namespace *)(new NamedNamespace(arg)); }

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
  static void operator delete(void* ptr)
    { return Memory::operator delete(ptr);}

  /** add a reference to this
   */
  virtual RefCount *upcount( void) const;
            
  /** remove a reference to this (possibly delete)
   */
  virtual void dncount( void) const;
  
  virtual Slot lshift(const Slot& arg) const
    { return Namespace::lshift(arg); }
  virtual Slot rshift(const Slot& arg) const
    { return Namespace::rshift(arg); }

protected:
  NamedNamespace(NamedNamespace& n);
  /** disallowed - cloning a symbol
   * NamedNamespaces are unique within a @ref Namespace
   * so it makes no sense to copy a NamedNamespace
   */
  virtual Data *clone(void *where = (void*)0) const;

  /** disallowed - mutating a symbol
   * Names are inherently mutable,
   * and only mutable from @ref Namespace
   * that this has been called suggests a protocol violation
   */
  virtual Data *mutate(void *where = (void*)0) const
    {return (Data*)(Namespace*)this;}
  
  /** disallowed: it's meaningless
   */
  virtual Slot replace(const Slot & s1, const Slot & s2, const Slot & s3);
  
  /** disallowed: it's meaningless
   */
  virtual Slot replace(const Slot &s1, const Slot &s2);
  
  /** disallowed: it's meaningless and ambiguous
   */
  virtual Slot replace(const Slot & s);
  /** multiple inheritance helper
   */
  Slot unimpl(const char* message) const;
};

#endif
