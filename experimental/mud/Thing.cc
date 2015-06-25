// Thing.cc: builds the $thing prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <String.hh>
#include <Closure.hh>
#include <VectorSet.hh>
#include "Event.hh"

#include "common.hh"

static Slot lookVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot command = args[0];
  Slot actor = command.actor();
  Slot dobj = command.dobj();

  actor.tell( dobj.lookAt() );

  return true;
}

static Slot getVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot command = args[0];

  Slot dobj = command.dobj();
  Slot actor = command.actor();

  Slot obj_loc = dobj.location();
  Slot a_loc = actor.location();

  if (obj_loc.equal(actor)) {
    actor.tell("You already have it.");
    return true;
  }
  if (dobj.equal(actor)) {
    actor.tell("You can't pick yourself up!");
    return true;
  }
  dobj.moveTo( actor );

  Slot myEvent = new Event( actor, dobj, "get");
  obj_loc.broadcastEvent(myEvent);

  return true;
}


static Slot dropVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot command = args[0];

  Slot dobj = command.dobj();
  Slot actor = command.actor();

  Slot obj_loc = dobj.location();
  Slot a_loc = actor.location();

  if (!obj_loc.equal(actor)) {
    actor.tell("You don't have it.");
    return true;
  }
  if (dobj.equal(actor)) {
    actor.tell("You can't drop yourself!");
    return true;
  }
  dobj.moveTo( a_loc );

  Slot myEvent = new Event( actor, dobj, "drop");
  a_loc.broadcastEvent(myEvent);

  return true;
}

// return location, contents, and location's contents
// in a list
static Slot environment( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot env = new VectorSet();

  Slot contents = self.contents();
  Slot location = self.location();
  Slot loc_cont = location.contents();

  env = env->concat( contents );
  env = env->concat( loc_cont );
  env = env->insert( location );

  return env;
}

/** return a string description of this thing
 */
static Slot lookAt( Frame *context )
{
    Slot self = context->_this;

    Slot desc = new String();
    desc->concat(self.name());
    desc->concat(new String("\n"));
    desc->concat(self.description());
    desc->concat(new String("\n"));

    return desc;
}

/** create the prototype for $thing
 */
Slot createThing(Frame *context)
{
  Slot self = context->_this;

  Slot named = self.named;
  Slot located = self.located;
  Slot described = self.described;
  Slot receiver = self.receiver;
  Slot has_verbs = self.has_verbs;

  Slot thing = named.create("thing");
  thing.addParent(located);
  thing.addParent(described);
  thing.addParent(receiver);
  thing.addParent(has_verbs);
  
  thing.setNames("generic thing");

  thing.lookAt = &lookAt;
  thing.environment = &environment;

  Slot verb = self.verb;
  Slot getVerb = verb.create("getVerb");
  getVerb.setNames("get", "take");
  getVerb.setPrepMatch("none");
  getVerb.setDobjMatch("this");
  getVerb.setIobjMatch("none");
  getVerb.invoke_on = &getVerbInvoke;
  thing.addVerb( getVerb );

  Slot dropVerb = verb.create("dropVerb");
  dropVerb.setNames("drop", "take");
  dropVerb.setPrepMatch("none");
  dropVerb.setDobjMatch("this");
  dropVerb.setIobjMatch("none");
  dropVerb.invoke_on = &dropVerbInvoke;
  thing.addVerb( dropVerb );

  Slot lookVerb = verb.create("lookVerb");
  lookVerb.setNames("look", "take");
  lookVerb.setPrepMatch("none");
  lookVerb.setDobjMatch("this");
  lookVerb.setIobjMatch("none");
  lookVerb.invoke_on = &lookVerbInvoke;
  thing.addVerb( lookVerb );

  Slot lookAtVerb = verb.create("lookAtVerb");
  lookAtVerb.setNames("look", "take");
  lookAtVerb.setPrepMatch("at");
  lookAtVerb.setDobjMatch("none");
  lookAtVerb.setIobjMatch("this");
  lookAtVerb.invoke_on = &lookVerbInvoke;
  thing.addVerb( lookAtVerb );

  return thing;
}
