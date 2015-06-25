// Container.cc: builds the $container prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <VectorSet.hh>
#include <String.hh>
#include <Frob.hh>
#include <Closure.hh>

#include "common.hh"

static Slot lookAt( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot names = store->Names;

  Slot desc = new String();

  desc = desc->concat( self.name() )->concat(new String("\n"));
  desc = desc->concat( self.description() )->concat(new String("\n"));
  
  Slot contents = self.contents;

  Slot c_names = contents.mapMethod( "name" );
  Slot c_string = c_names.englishJoin();
  desc = desc->concat( c_string )->concat(" are here.");
  
  return desc;
}

/** Respond to a request for entry on a container object
 */
static Slot allowEnterQuery( Frame *context )
{
  // default implementation just returns true for all objects
  return true;
};

/** Actually accept the entrance of an object into our contents
 */
static Slot enter( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot who = args[0];
  // append to contents
  Slot contents = self.contents;
  contents = contents->insert(who);
  self.contents = contents;

  return true;
};

/** Respond to a request to leave a container object
 */
static Slot allowLeaveQuery( Frame *context )
{
  // default implementation just returns true for all objects
  return true;
};

/** Actually accept the departure of an object from our contents
 */
static Slot leave( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot who = args[0];

  // append to contents
  Slot contents = self.contents;
  contents = contents->del(who);
  self.contents = contents;

  return true;
};

/** matchContentsByName: find all objects matching "name"
 *  inside this object.
 */
static Slot matchContentsByName( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Slot name = args[0];


  Slot matches = new VectorSet();

  Slot contents = self.contents();

  Slot c_it = contents->iterator();

  while (c_it->More()) {
    Slot obj = c_it->Next();
    Slot names = obj.names();
    if ( names->search( name ) )
      matches = matches->insert( obj );
  }

  return matches;
}

static Slot contents( Frame *context )
{
  Slot self = context->_this;

  return self.contents;
}

static Slot initialize( Frame *context ) 
{
  Slot self = context->_this;

  self.contents = new VectorSet();

  return true;
}

Slot createContainer( Frame *context )
{
  Slot self = context->_this;

  Slot thing = self.thing;
  Slot container = thing.create("container");
  container.setNames("generic container");
  container.initialize = &initialize;
  container.allowEnterQuery = &allowEnterQuery;
  container.enter = &enter;
  container.allowLeaveQuery = &allowLeaveQuery;
  container.leave = &leave;
  container.lookAt = &lookAt;
  container.contents = &contents;
  container.matchContentsByName = &matchContentsByName;

  // set up instance variables
  container.initialize();
  return container;
}
