// Broadcaster.cc: builds the $broadcaster prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Object.hh>
#include <Frame.hh>
#include <Error.hh>
#include <Store.hh>
#include <VectorSet.hh>
#include <Closure.hh>
#include "Event.hh"

#include "common.hh"

/** subscribe to a broadcaster
 */
static Slot subscribe( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  if ( args->length() != 1 )
    throw new Error("invarg", args, "invalid number of arguments");
  
  Slot subscribers = self.subscribers;
  subscribers = subscribers->insert( args[0] );
  self.subscribers = subscribers;

  return true;
}

/** unsubscribe from a broadcaster
 */
static Slot unsubscribe( Frame *context )
{
    Slot self = context->_this;
    Slot args = context->_arguments;

    if ( args->length() != 1 )
        throw new Error("invarg", args, "invalid number of arguments");

    Slot who = args[0];

    Slot subscribers = self.subscribers;
    subscribers = subscribers->del( who );
    self.subscribers = subscribers;

    return true;
}


/** broadcast an event to subscribers
 */
static Slot broadcastEvent( Frame *context )
{
    Slot self = context->_this;
    Slot args = context->_arguments;

    if ( args->length() != 1 )
        throw new Error("invarg", args, "invalid number of arguments");

    Slot event = args[0];

    if ( ! (AKO((Data*)(event), Event )) )
        throw new Error("type", event, "argument is not an event" );

    Slot subscribers = self.subscribers;

    Slot it = subscribers->iterator();
    Slot selector = ((Event*)(Data*)event)->getID()->concat("Reaction");

    while (it->More()) {
      Slot who = it->Next();
      try {
	Message *call = new Message( context, who, selector, args );
	who->receive( call, context );
      } catch (Error *e) {
	try {
	  who.receiveEvent( event );
	} catch (Error *e) {
	}
      }
    }

    return true;
}

static Slot initialize( Frame *context )
{
  Slot self = context->_this;
  
  self.subscribers = new VectorSet();
  
  return true;
}

static Slot subscribers( Frame *context )
{
  Slot self = context->_this;
  
  return self.subscribers;
}

Slot createBroadcaster( Frame *context )
{
  Slot self = context->_this;
  
  Slot root = self.root;
  Slot broadcaster = root.create("broadcaster");
  broadcaster.initialize = &initialize;
  broadcaster.subscribers = &subscribers;
  broadcaster.subscribe = &subscribe;
  broadcaster.unsubscribe = &unsubscribe;
  broadcaster.broadcastEvent = &broadcastEvent;
  
  broadcaster.initialize();
  
  return broadcaster;
}
