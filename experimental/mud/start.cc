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

#include <strstream>

#include "Event.hh"

#include "common.hh"
#include "Parser.hh"

#include "server.hh"



void pre_eval_bkpt() {}

void yac_break()
{}

static Slot initRoots(Frame *context)
{
  
  cerr << "Bootstrapping DB with core objects..." << endl;
  Slot Receiver = defineReceiver( context );
  cerr << "Defined $" << (char*)Receiver.id << endl;
  Slot Broadcaster = defineBroadcaster( context );
  cerr << "Defined $" << (char*)Broadcaster.id << endl;
  Slot Named = defineNamed( context );
  cerr << "Defined $" << (char*)Named.id << endl;
  Slot Described = defineDescribed( context );
  cerr << "Defined $" << (char*)Described.id << endl;
  Slot Located = defineLocated( context );
  cerr << "Defined $" << (char*)Located.id << endl;
  Slot Thing = defineThing( context );
  cerr << "Defined $" << (char*)Thing.id << endl;
  Slot Container = defineContainer( context );
  cerr << "Defined $" << (char*)Container.id << endl;
  Slot Room = defineRoom( context );
  cerr << "Defined $" << (char*)Room.id << endl;
  Slot Parser = defineParser( context );
  cerr << "Defined $" << (char*)Parser.id << endl;
  Slot Dictionary = defineDictionary( context );
  cerr << "Defined $" << (char*)Dictionary.id << endl;
  Slot Verb = defineVerb( context );
  cerr << "Defined $" << (char*)Verb.id << endl;
  Slot Reactor = defineReactor( context );
  
  cerr << "Defined $" << (char*)Reactor.id << endl;
  Slot Actor = defineActor( context );
  cerr << "Defined $" << (char*)Actor.id << endl;
  Slot ConsoleObserver = defineConsoleObserver( context );
  cerr << "Defined $" << (char*)ConsoleObserver.id << endl;
  Slot GlobalVerbs = defineGlobalVerbs( context );
  cerr << "Defined global verbs." << endl;
  Slot Connection = defineConnection( context );
  cerr << "Defined $" << (char*)Connection.id << endl;
  
  Slot testObject = Thing.create_child("test_object");
  testObject.set_names("my test object", "test object");
  
  Slot testDest = Room.create_child("test_room");
  testDest.set_names("my test room", "test room", "room");
  names.starting_room = testDest;
  
  Slot testDest2 = Room.create_child("test_room2");
  testDest2.set_names("my other room", "other room");
  
  // now create a user
  Slot user = Actor.create_child("ryan");
  user.set_names("Ryan", "Karl");
  
  Slot userDict = new Dict();
  userDict = userDict->insert("Ryan", user );
  userDict = userDict->insert("ryan", user );
  names.user_dictionary = userDict;
  
  user.moveTo( testDest ); 
  ConsoleObserver.moveTo( testDest );
  testObject.moveTo( testDest );
  
  // PING!
  Slot myEvent = new Event( user, user, "ping" );
  
  testDest.broadcastEvent( myEvent );
  
  
  return true;
    
}


void initElf(Elf *elf)
{
  Frame *context = (Frame*)0;

  try {
    
    names = store->Names; 
    Slot Root = names.root;

    Slot World = Root.create_child("world");
    World.initialize = &initRoots;
    names.world = World;
    cerr << "Added world object..." << endl;

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
