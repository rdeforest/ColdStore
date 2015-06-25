// Root.cc: builds the $root prototype
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <Closure.hh>
#include <Tuple.hh>
#include <Error.hh>
#include <VectorSet.hh>

#include "common.hh"

/** add instance variable, with value NULL
 */
static Slot addVar( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot name = args[0];

  self->insert( new IVKey( name, self ), 0 );

  return true;
}

/** remove instance variable
 */
static Slot delVar( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot name = args[0];

  self->del( new IVKey( name, self ) );

  return true;
}

/** add method
 */
static Slot addMethod( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot name = args[0];
  Slot closure = args[1];

  self->insert( name, closure );

  return true;
}

/** del method
 */
static Slot delMethod( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot name = args[0];
  Slot closure = args[1];

  self->del( name );

  return true;
}

/** add parent
 */
static Slot addParent( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  return ((Object*)(Data*)self)->addParent(args[0]);
}

/** del parent
 */
static Slot delParent( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  return ((Object*)(Data*)self)->delParent(args[0]);
}

/** list children
 */
static Slot children( Frame *context )
{
  Slot self = context->_this;

  return ((Object*)(Data*)self)->children();
}

/** list parents
 */
static Slot parents( Frame *context )
{
  Slot self = context->_this;

  return ((Object*)(Data*)self)->parents();
}

/** create a child of this (option first arg = id string
 */
static Slot create( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  Slot child = self->create("");
  child.setId( args[0] );

  return child;
}

/** return id of the object
 */
static Slot id( Frame *context )
{
  Slot self = context->_this;

  return self.id;
}

/** set id of the object
 */
static Slot setId( Frame *context)
{
  Slot self = context->_this;
  Slot new_id = context->_arguments[0];

  self.id = new_id;

  return true;
}

/** pass a message up to parent
 */
static Slot pass( Frame *context )
{
  Slot self = context->_this;


  Frame *prev = ((Frame*)(Data*)context->_caller);

  Slot definer = ((Closure*)(Data*)prev->_closure)->_definer;
  Slot parents = definer.parents();
  
  Slot f_parent;
  if (parents->length()) {
    f_parent = parents->slice( 0 );
  } else
    throw new Error("pass", parents, "no parents for pass");

  Message *n = new Message( context, self, prev->_selector, prev->_arguments, prev->_metaobjects );
  return f_parent->receive( n, context );
}

/** root object defines an id (unique identifier)
 *  as well as createChild, addParent, delParent, etc.
 */
Slot defineRoot( Frame *context )
{
  Slot Root = new Object();

  Root.addVar = &addVar;
  Root.delVar = &delVar;
  Root.addMethod = &addMethod;
  Root.delMethod = &delMethod;
  Root.create = &create;
  Root.parents = &parents;
  Root.children = &children;
  Root.addParent = &addParent;
  Root.delParent = &delParent;
  Root.id = &id;
  Root.setId = &setId;
  Root.pass = &pass;

  Root->insert( new IVKey("id", Root), "root" );

  store->Names->insert("root", Root );

  return Root;
}
