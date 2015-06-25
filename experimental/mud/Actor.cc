// Actor.cc: builds the $actor prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Frame.hh>
#include <Store.hh>
#include <String.hh>
#include <Closure.hh>
#include <Error.hh>

#include "Event.hh"

#include "common.hh"

#include <sys/socket.h>
#include <unistd.h>

#include "../../languages/chatter/Parser.hh"

#include <strstream>

#include <stdio.h>

Slot repr( Slot what ) {
  ostrstream out;
  out << what << ends; 
  return out.str();
}

static Slot evalVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot on = args[1];
  Slot argstr = args[0].argstr();

  Slot text = argstr->slice(5, -1);

  if (!text->slice(0).equal("^"))
    text = text->insert(0, "^ ");

  Parser *p = new Parser();
  Slot x = p->parse( on, text );

  on->insert("tmp_eval", x );

  on.tell( Slot("=> ")->concat(repr( on.tmp_eval())) );

  return true;
}

static Slot quitVerbInvoke( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot actor = args[0].actor();

  actor.tell("** Disconnected **");

  actor.disconnect();
  
  
  return true;
}

static Slot tell( Frame *context)
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Slot line = args[0];
  
  Slot conn = self.connection();
  if (conn) 
    try {   
      conn.tell( line );
    } catch (Error *e) {
      cerr << (char*) line << endl;
    }
  
  return true;
}

static Slot connection( Frame *context )
{
  Slot self = context->_this;
  
  return self.connection;
}

static Slot setConnection( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  self.connection = args[0];
  
  return true;
}

static Slot receiveLine( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments; 
  
  Slot line = args[0];

  try {
    self.parseLine( line ) ;
  } catch (Frame *e_context) {
    Error *e = e_context->_error;
    if (e->slice(0)->slice(1)->equal("parse"))
      self.tell(e->slice(2));
    else
      self.tell(e_context->traceback());
  }

  return true;
};

static Slot getReaction( Frame *context )
{
    Slot self = context->_this;
    Slot arguments = context->_arguments;
 
    Event *event = ((Event*)(Data*)arguments[0]);
    
    Slot what = event->getCause();
	
    if (event->getSource().equal(self))
    {
	Slot msg = "You pick up ";
	msg=msg->concat(what.name())->concat(".");
	self.tell(msg );   
    } else {
	Slot msg = event->getSource().name()->concat(" picks up ")->concat(what.name())->concat(".");
	self.tell( msg );
    }   
  
    return 1;
}

static Slot dropReaction( Frame *context )
{
    Slot self = context->_this;
    Slot arguments = context->_arguments;
 
    Event *event = ((Event*)(Data*)arguments[0]);
    
    Slot what = event->getCause();
	
    if (event->getSource().equal(self)) {
      Slot msg = "You drop ";
      msg=msg->concat(what.name())->concat(".");
      self.tell(msg);
    } else {
      Slot msg = event->getSource().name()->concat(" drops ")->concat(what.name())->concat(".");
      self.tell(msg);
    }
  
    return 1;
}   

static Slot sayReaction( Frame *context )
{
  Slot self = context->_this;
  Slot arguments = context->_arguments;
  
  Event *event = ((Event*)(Data*)arguments[0]);
  
  Slot what = event->getCause();
  
  Slot from = event->getSource();

  cerr << "EVENT SAY " << from.id() << endl;

  if (event->getSource().equal(self))
    {
      Slot msg = "You say, \"";
      msg=msg->concat( event->slice("text") )->concat("\"");
      self.tell(msg);
    } else {
      Slot msg = event->getSource().name()->concat(" says, \"")->concat(event->slice("text"))->concat("\"");
      self.tell(msg);
    }   
  
  return 1;
}  

static Slot emoteReaction( Frame *context )
{
    Slot self = context->_this;
    Slot arguments = context->_arguments;
 
    Event *event = ((Event*)(Data*)arguments[0]);
    
    Slot what = event->getCause();
	
    Slot msg = event->getSource().name()->concat(" ")->concat(event->slice("text"));
    self.tell(msg);
  
    return 1;
} 

static Slot connectReaction( Frame *context )
{
  
  Slot self = context->_this;
  Slot arguments = context->_arguments;
  
  Event *event = ((Event*)(Data*)arguments[0]);
  
  Slot what = event->getCause();
  Slot who = event->getSource();

  if (who.equal(self))
    {
      Slot msg = "** Connected **";
      self.tell(msg);
      
      Slot location = self.location();
      self.tell( location.lookAt() );
    } else {
      Slot msg = who.name()->concat(" has connected.");
      self.tell(msg);
    }   

  return 1;
} 

static Slot disconnectReaction( Frame *context )
{
    Slot self = context->_this;
    Slot arguments = context->_arguments;
 
    Event *event = ((Event*)(Data*)arguments[0]);
    
    Slot what = event->getCause();
	
    if (!event->getSource().equal(self)) {
      Slot msg = event->getSource().name()->concat(" has disconnected.");
      self.tell(msg);
    }   
  
    return 1;  // if we're in programming mode, spool input
} 

static Slot disconnect( Frame *context )
{
    Slot self = context->_this;
    Slot arguments = context->_arguments;

    Slot location = self.location();
    location.broadcastEvent( new Event( self, self, "disconnect" ) );
    
    Slot conn = self.connection();   
    self.setConnection( (Data*)NULL );
    conn.setUser( (Data*)NULL );
    conn.disconnect();
    
    return 1;
} 

Slot createActor( Frame *context )
{
  Slot self = context->_this;

  Slot container = self.container;
  Slot parser = self.parser;

  Slot actor = container.create("actor");
  actor.addParent( parser );
  actor.setNames("generic actor");
    
  actor.connection = &connection;
  actor.setConnection = &setConnection;
  actor.disconnect = &disconnect;
  actor.getReaction = &getReaction;
  actor.dropReaction = &dropReaction;
  actor.sayReaction = &sayReaction;
  actor.emoteReaction = &emoteReaction;
  actor.connectReaction = &connectReaction;
  actor.disconnectReaction = &disconnectReaction;
  actor.receiveLine = &receiveLine;
  actor.tell = &tell;

  Slot verb = self.verb;
  Slot quitVerb = verb.create("quitVerb");
  quitVerb.setNames("quit", "@quit");
  quitVerb.setPrepMatch("none");
  quitVerb.setDobjMatch("none");
  quitVerb.setIobjMatch("none");
  quitVerb.invoke_on = &quitVerbInvoke;
  actor.addVerb( quitVerb );

  Slot evalVerb = verb.create("evalVerb");
  evalVerb.setNames("eval", "@eval");
  evalVerb.setPrepMatch("none");
  evalVerb.setDobjMatch("string");
  evalVerb.setIobjMatch("none");
  evalVerb.invoke_on = &evalVerbInvoke;
  actor.addVerb( evalVerb );
  
  actor.setConnection( (Data*)NULL );

  return actor;
}
