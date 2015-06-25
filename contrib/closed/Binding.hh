// Binding - Binding object 
// Copyright (C) 2000,2001 Colin McCormack, Ryan Daum 
// 
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// $Id 

#ifndef BINDING_HH
#define BINDING_HH

#include "List.hh"
#include "Tuple.hh"
#include "Dict.hh"
#include "Node.hh"


/** Binding - a class to allow lightweight variable storage by indexes
 */
 
cold class Binding
  : public Tuple
{

public:
  Slot _variables;	// list of variable values
  Slot _map;		// mapping of variable names -> indexes

public:
  /////////////////
  // Construction

  Binding();
  Binding( const Slot &constructor );
  Binding( const Binding *copy );

  virtual ostream &dump(ostream& out) const  ;

  ///////////////////////////////
  // Structural
  virtual Data *clone(void *store = (void*)0) const;
  virtual Data *mutate(void *where) const; 
  
public:
  ///////////////////////////////
  // Variable operations

  /** return the Variable representing a variable
   *  @param name the variable name
   *  @return an Variable with the index and name of the variable
   */
  virtual Slot search(const Slot &name) const;  
  
  /** define a variable, with value
   *  @param name the variable name
   *  @param value the variable's initial value
   *  @return an Variable with index and name of the variable
   */
  virtual Slot insert(const Slot &name, const Slot &value);
  
  /** define a variable, null value
   *  @param name the variable name
   *  @return the Variable with index and name of the variable
   */
  virtual Slot insert(const Slot &name);
  
  /** set the value of a variable through an Variable
   *  @param ident an Variable with which to lookup the variable
   *  @param value the new value of the variable
   */
  virtual Slot set(const Slot &ident, const Slot &value);

  /** retrieve the value of a variable via its node
   *  @param ident an Variable with which to look up the value
   *  @param context a Frame containing context information
   *  @return the value of the variable
   */
  virtual Slot resolve(const Slot &ident, Frame *context ) const;

  /** remove a variable from the binding
   *  @param name the variable name
   *  @return a copy of this with the deletion performed
   */
  virtual Slot del(const Slot &name);

public:
  virtual void pushLocals( const Slot &count );
  virtual void popLocals( const Slot &count );

};

/** Variable - a node representing an index into a Binding - reduces to its value
 *  as stored in the binding
 */
cold class Variable
  : public Node
{
public:
  /** reduce the node to its value
   *  @param inexpr copy of the node to replace
   *  @param context the current frame context
   */
  virtual void reduce( Slot &inexpr, Frame *context ) ;

  /** set the value current held by the variable
   *  @param inValue the new value of the variable
   *  @param context context of the frame to evaluate in
   *  @return modified copy 
   */
  virtual Slot replace( const Slot &inValue, const Slot &context );
};

#endif

