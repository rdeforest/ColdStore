// start.cc: start the mud!
// Copyright (C) 200!, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

#include <assert.h>
#include <stdio.h>

#include <elf.hh>
#include <Data.hh>
#include <Slot.hh>
#include <List.hh>
#include <Frame.hh>
#include <Object.hh>
#include <Store.hh>
#include <Error.hh>
#include <VectorSet.hh>
#include <String.hh>
#include <Frob.hh>
#include <Closure.hh>

#include <strstream>

#include "Event.hh"
#include "common.hh"

#include "server.hh"

// forward creates for loaders
extern Slot createContainer(Frame *context);
extern Slot createLocated(Frame *context);
extern Slot createReceiver(Frame *context);
extern Slot createBroadcaster(Frame *context);
extern Slot createHasVerbs(Frame *context);
extern Slot createRoom(Frame *context);
extern Slot createConsoleObserver(Frame *context);
extern Slot createActor(Frame *context);
extern Slot createThing(Frame *context);
extern Slot createNamed(Frame *context);
extern Slot createDescribed(Frame *context);
extern Slot createConnection(Frame *context);
extern Slot createUserDictionary( Frame *context );
extern Slot createParser( Frame *context );
extern Slot createVerb( Frame *context );
extern Slot createCommand( Frame *context );

/** given an object, insert it (using its id as key)
 *  into the global namespace as well as adding a
 *  local instance variable for it.
 */
static Slot installPrototype( Frame *context ) {
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot prot = args[0];

  store->Names->insert( prot.id(), prot );
  self->insert( new IVKey( prot.id(), self ), prot );

  cerr << "installed $" << (char*)prot.id() << endl;

  return true;
}

/** construct the root prototypes for key components 
 */
static Slot buildPrototypes( Frame *context ) {
  Slot self = context->_this;
  Slot args = context->_arguments;

  self.installPrototype( self.createReceiver() );
  self.installPrototype( self.createBroadcaster() );
  self.installPrototype( self.createNamed() );
  self.installPrototype( self.createDescribed() );
  self.installPrototype( self.createLocated() );
  self.installPrototype( self.createCommand() );
  self.installPrototype( self.createVerb() );
  self.installPrototype( self.createHasVerbs() );
  self.installPrototype( self.createThing() );
  self.installPrototype( self.createContainer() );
  self.installPrototype( self.createRoom() );
  self.installPrototype( self.createParser() );
  self.installPrototype( self.createActor() );
  self.installPrototype( self.createConsoleObserver() );
  self.installPrototype( self.createConnection() );
  self.installPrototype( self.createUserDictionary() );

  cerr << "prototypes built." << endl;
  return true;
}  

/** build a demonstration world.
 */
static Slot buildDemonstration( Frame *context ) {
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot thing = self.thing;
  Slot room = self.room;
  Slot actor = self.actor;
  Slot console_observer = self.console_observer;
  Slot user_dictionary = self.user_dictionary;
  Slot verb = self.verb;

  // to test "thing" with
  Slot testObject = thing.create("test_object");
  testObject.setNames("my test object", "test object");

  Slot getFVerb = verb.create("getFrom");
  getFVerb.setNames("get", "take");
  getFVerb.setPrepMatch("from");
  getFVerb.setDobjMatch("object");
  getFVerb.setIobjMatch("this");
  testObject.addVerb( getFVerb );
  cerr << "Added verb " << getFVerb.id() << endl;

  // to test rooms with
  Slot testDest = room.create("test_room");
  testDest.setNames("my test room", "test room", "room");

  // starting room global
  store->Names->insert( "starting_room", testDest );

  // to test movement between rooms with
  Slot testDest2 = room.create("test_room2");
  testDest2.setNames("my other room", "other room");
  
  // now create a user
  Slot user = actor.create("ryan");
  user.setNames("Ryan", "Karl");
  
  user_dictionary.addUser_withName( user, "Ryan" );
  user_dictionary.addUser_withName( user, "Karl" );


  user.moveTo( testDest ); 
  console_observer.moveTo( testDest );
  testObject.moveTo( testDest );

  // PING!
  Slot myEvent = new Event( user, user, "ping" );

  testDest.broadcastEvent( myEvent );
   
  cerr << "DEMO WORLD BUILT" << endl;

 
  return true;  
}


static Slot initialize( Frame *context )
{
  Slot self = context->_this;

  self.root = store->Names->slice("root")[2];

  self.buildPrototypes();
  self.buildDemonstration();

  return true;
}

/** create the world object so that the server may bootstrap it
 *  later
 */
void initElf(Elf *elf)
{
  Frame *context = (Frame*)NULL;

  try {
    Slot root = store->Names->slice("root")[2];

    if (store->Names->search("world")) {
      cerr << "resuming..." << endl;
      return; 
    }
    Slot world = root.create("world");

 
    world.installPrototype = &installPrototype;
    world.createReceiver = &createReceiver;
    world.createBroadcaster = &createBroadcaster;
    world.createNamed = &createNamed;
    world.createDescribed = &createDescribed;
    world.createLocated = &createLocated;
    world.createThing = &createThing;
    world.createHasVerbs = &createHasVerbs;
    world.createContainer = &createContainer;
    world.createRoom = &createRoom;
    world.createActor = &createActor;
    world.createConsoleObserver = &createConsoleObserver;
    world.createConnection = &createConnection;
    world.createUserDictionary = &createUserDictionary;
    world.createParser = &createParser;
    world.createVerb = &createVerb;
    world.createCommand = &createCommand;

    world.buildPrototypes = &buildPrototypes;
    world.buildDemonstration = &buildDemonstration;

    world.initialize = &initialize;

    store->Names->insert("world", world);
    cerr << "added world object..." << endl;

  } catch (Error *e) {
    e->upcount();
    cerr << "Error " <<  ((Symbol*)(Data*)e)->name() << "\n";
    e->dncount();
  } catch (Frame *e) {
    e->upcount();
    cerr << (char*)e->traceback() << endl;
    e->dncount();
  }
}
