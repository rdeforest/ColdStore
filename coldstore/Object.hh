#ifndef __OBJECT_HH
#define __OBJECT_HH

#include "Data.hh"
#include "Tuple.hh"
#include "Symbol.hh"



cold class Object
  : public Namespace
{
public:
  Object();
  Object( const Slot& init );
  virtual Slot toconstruct() const;
  ~Object();

private:
  Slot member_cache; // set of found members.
  Slot parent_list;  // Set of parents
  Slot child_set;    // Set of children

public:
  // Required details for the operation of Object name resolution.

  /** slice by key through inheritance
   *  @param key key to identify the member sought
   *  @return match either in self or parents
   *  @exception dictnf 
   */
  virtual Slot slice( const Slot &name ) const;

  /** search by key through inheritance with visitation filter
   *  @param key key to identify the member sought
   *  @param recurse true if inheritance search required
   *  @param context list of previously visited objects
   *  @return match either in self or parents or null if not found
   */
  virtual Slot search_graph( const Slot &name, const Slot &context ) const ;

  /** construct an instance of a subclass from the constructor args given
   * @param arg constructor arguments, derived from @ref toconstruct
   * @return instance constructed from args
   */
  virtual Slot create( const Slot &arg ) const;

  /** dump string representation ofthe object to output stream.
   *  overrides namespace's dump, so as to strip out dumping of symbols
   *  shows only: # of symbols contained, address, and identifier, if possible
   */
  virtual ostream &dump(ostream&) const;

  virtual int order(const Slot &arg) const;
public:
  /** create a child of this Object
   *  @return a child of this Object
   */
  virtual Slot createChild() const ;

  /** add an Object to the children list
   *  @param child child to add
   *  @return copy of self modified with change
   */
  virtual Slot addChild(const Slot& child);

  /** add a parent to the parents list
   *  @param parent parent to add
   *  @return copy of self modified with change
   */
  virtual Slot addParent(const Slot& parent);
  
  /** remove a child
   *  @param child child to delete
   *  @return childrent list minus child
   */
  virtual Slot delChild(const Slot& child);

  /** delete a child
   *  @param parent parent to delete
   *  @return parents list minus parent
   */
  virtual Slot delParent(const Slot& parent);

  /** return a list of parents
   *  @return list of parents
   */
  virtual Slot parents() const;

  /** return a list of children
   *  @return list of children
   */
  virtual Slot children() const;

};

// an IVKey is "instance variable key" -- it is meant to be used in place of
// the name string field in Symbol specifically for instance variables, and
// holds both definer and name, rather than just the name.  The reason we use
// this instead of just a plain tuple (which in most respects it is) is because
// this also provides us with a) easy constructor and ** b) a dump which won't
// recurse infinitely down the definer path like Tuple's would  ** (** obsolete)
cold class IVKey
  : public Tuple 
{
private:
  Slot _name;
  Slot _definer;

public:
  IVKey();
  IVKey( const Slot &name, const Slot &definer );
  IVKey( const Slot & init );
  virtual Slot toconstruct() const;
  ~IVKey();

  virtual ostream &dump(ostream&) const;

  // structural
  virtual Data *clone(void *store = (void*)0) const;
  virtual Data *mutate(void *where) const;
};

#endif
