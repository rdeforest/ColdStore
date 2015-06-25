// Named.cc: builds the $named prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <List.hh>
#include <VectorSet.hh>
#include <Frame.hh>
#include <Closure.hh>

#include "common.hh"

/** return the primary name: i.e. the first name in the set
 */
static Slot name( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Slot names = self.names;
  
  return names->rshift( (Data*)NULL );
}

/** set the names of the object.  arguments
 *  are received as a list, so we copy them
 *  to a VectorSet instead
 */
static Slot setNames( Frame *context ) 
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot names = new VectorSet();
  Slot n_it = args->iterator();
  while (n_it->More())
    names->insert( n_it->Next() );

  self.names = names;

  return 1;
}

/** return all names
 */
static Slot names( Frame *context )
{
  Slot self = context->_this;

  return self.names;
}
  
Slot createNamed(Frame *context)
{
  Slot self = context->_this;

  Slot root = self.root;
  Slot named = root.create("named");

  named.setNames = &setNames;
  named.name = &name;
  named.names = &names;

  named.setNames("generic named object");

  return named;
}
