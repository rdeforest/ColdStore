// Room.cc: builds the $room prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <Closure.hh>

#include "Event.hh"

#include "common.hh"

static Slot lookVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  Slot on = args[1];
  Slot command = args[0];
  Slot actor = command.actor();
  Slot dobj = command.dobj();

  actor.tell( on.lookAt() );

  return true;
}
static Slot sayVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot command = args[0];
  Slot actor = command.actor();
  Slot line = command.argstr();
    
  Slot location = actor.location();

  Slot myEvent = new Event( actor, location, "say" );

  Slot text;
  if (line->slice(0)->equal("\""))
    text = line->slice(1,-1);
  else {
    if (line->length()>3)
      text = line->slice(4,-1);
    else
      text = "";
  }
  myEvent->insert("text", text);

  location.broadcastEvent( myEvent );
    
  return true;

}

static Slot emoteVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot command = args[0];
  Slot actor = command.actor();
  Slot line = command.argstr();
    
  Slot location = actor.location();

  Slot myEvent = new Event( actor, location, "emote" );

  Slot text;
  if (line->slice(0)->equal(":"))
    text = line->slice(1,-1);
  else {
    if (line->length()>3)
      text = line->slice(6,-1);
    else
      text = "";
  }
  myEvent->insert("text", text);

  location.broadcastEvent( myEvent );
    
  return true;

}

/** Enter the object -- our version (as opposed to Container's)
  * also adds the object to our event subscription list.
  */
static Slot enter( Frame *context )
{
    Slot self = context->_this;
    Slot args = context->_arguments;

    Slot who = args[0];

    // pass message to parent to handle actual object addition
    self.pass();
     
    self.subscribe( who );
    // construct the event
    Slot myEvent = new Event( self, who, "enter" );
    myEvent->insert("who", who );

    // broadcast it
    self.broadcastEvent( myEvent );

    return true;
};

/** Actually accept the departure of an object from our contents
 *  our version (as opposed to Container's) unsubscribes the leaving
 *  party
 */
static Slot leave( Frame *context )
{
    Slot self = context->_this;
    Slot args = context->_arguments;

    Slot who = args[0];

    // Let the world know who is leaving
    Slot myEvent = new Event( self, who, "leave" );
    self.broadcastEvent( myEvent );

    // pass message to parent to handle object removal
    self.pass();

    // Call unsubscribe
    self.unsubscribe( who );

    return true;
};

Slot createRoom(Frame *context)
{
  Slot self = context->_this;

  Slot container = self.container;
  Slot broadcaster = self.broadcaster;

  Slot room = container.create("room");
  room.addParent( broadcaster );
  room.setNames("generic room");
  room.enter = &enter;
  room.leave = &leave;

  Slot verb = self.verb;
  Slot sayVerb = verb.create("sayVerb");
  sayVerb.setNames("say");
  sayVerb.setDobjMatch("string");
  sayVerb.setPrepMatch("none");
  sayVerb.setIobjMatch("none");
  sayVerb.invoke_on = &sayVerbInvoke;
  room.addVerb( sayVerb );

  Slot emoteVerb = verb.create("emoteVerb");
  emoteVerb.setNames("emote");
  emoteVerb.setDobjMatch("string");
  emoteVerb.setPrepMatch("none");
  emoteVerb.setIobjMatch("none");
  emoteVerb.invoke_on = &emoteVerbInvoke;
  room.addVerb( emoteVerb );

  Slot lookVerb = verb.create("lookVerb");
  lookVerb.setNames("look");
  lookVerb.setDobjMatch("none");
  lookVerb.setPrepMatch("none");
  lookVerb.setIobjMatch("none");
  lookVerb.invoke_on = &lookVerbInvoke;
  room.addVerb( lookVerb );

  return room;
}
