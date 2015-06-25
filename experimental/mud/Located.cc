// Located.cc: builds the $located prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Frame.hh>
#include <Store.hh>
#include <Closure.hh>

#include "common.hh"

/** Move a located object
  * Queries destination's "acceptQuery" method for true/false
  * Then calls destination's "allowEnter" to allow destination to add
  *    to contents
  * Then changes location to match
  */
static Slot moveTo( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot destination = args[0];

  Slot location = self.location();

  // first, are we allowed into the new location?
  Slot enterable = destination.allowEnterQuery( self );

  if (!enterable)
    return false;

  // second, are we allowed to leave current location?
  if (location)  {
    Slot leaveable = location.allowLeaveQuery( self );

    if (!leaveable)
      return false;
    
    // third, remove us from current location
    Slot left = location.leave(self);
  }

  // fourth, ask current location to embrace us
  destination.enter( self );
  
  // fifth, set our new location
  self.location = destination;
  
  return true;
};

static Slot location( Frame *context )
{
  Slot self = context->_this;

  return self.location;
}

static Slot initialize( Frame *context )
{
  Slot self = context->_this;

  self.location = (Data*)NULL;

  return true;
}


Slot createLocated( Frame *context )
{
  Slot self = context->_this;

  Slot root = self.root;

  Slot located = root.create("located");
  located.initialize = &initialize;
  located.location = &location;
  located.moveTo = &moveTo;
  located.initialize();
  
  return located;
}
