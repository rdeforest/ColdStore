// GlobalVerbs.cc: adds to the global verb dictionary
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <List.hh>
#include <String.hh>

#include "Event.hh"
#include "common.hh"
#include "Parser.hh"

#include <sys/socket.h>

#include <strstream>

static Slot get( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Slot command = args[0];
  Slot who = command[0];
  
  Slot old_loc = who.location;
  if (who.location.equal(command->actor)) {
    command->actor.tell("You already have it.");
    return false;
  }
  
  if (who.equal(command->actor)) {
    command->actor.tell("You cannot get yourself.");
    return false;
  }
   
  who.moveTo( command->actor );
  
  Slot myEvent = new Event( command->actor, who, "get");
  old_loc.broadcastEvent(myEvent);
  
  return true;
}

/** implementation of the common "get" verb
 */
static Slot drop_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Command *command = ((Command*)(Data*)args[0]);
  
  Slot who = command->dirobj;

  if  (!who) {
    command->actor.tell("Drop what?");
    return false;
  }
  
  if (who->equal(0))
    {
      command->actor.tell("I don't see that here.");
      return false;
    }
  
  Slot location = command->actor.location;
  Slot who_location = who.location;
  
  if ( command->actor != who_location)
    {
      command->actor.tell("You don't have that.");
      return false;
    }

  

  who.moveTo(location);

  Slot myEvent = new Event( command->actor, who, "drop");
  location.broadcastEvent( myEvent );
  return true;
}

/** implementation of the common "look" verb
 */
static Slot look_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Command *command = ((Command*)(Data*)args[0]);
  
  Slot who = command->dirobj;
  
  if (!who)
    // no direct object == look at location
    {
      who = command->actor.location;
    }
  if (who->equal(0))
    {
      command->actor.tell("I don't see that here.");
      return false;
    }
  
  try {
    Slot desc = who.lookAt();
    command->actor.tell(desc);
  } catch (Error *e) {
    command->actor.tell("You can't look at that!");
  }

  return true;
}

static Slot say_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Command *command = ((Command*)(Data*)args[0]);
  Slot line = args[1];
    
  Slot location = command->actor.location;
    
  Slot myEvent = new Event( command->actor, location, "say" );
    
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

static Slot emote_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Command *command = ((Command*)(Data*)args[0]);
  Slot line = args[1];

  Slot location = command->actor.location;
  
  Slot myEvent = new Event( command->actor, location, "emote" );
  
  Slot text;
  if (line->slice(0)->equal(":"))
    text = line->slice(1,-1);
  else {
    if (line->length()>5)
      text = line->slice(6,-1);
    else
      text = "";
  }

       
  myEvent->insert("text", text);

  location.broadcastEvent( myEvent );
  return true;
}

extern Slot string_to_words( Slot pStr, Slot garbage = new List() );   

static Slot start_programming_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;  

  Command *command = ((Command*)(Data*)args[0]);

  Slot words = string_to_words( args[1] );
  if (words->length()!=3) {
    command->actor.tell("@program object method");
    return false;
  }
  
  Slot obj_name = words[1];
  if (!(obj_name[0].equal("$"))) {
    command->actor.tell("first argument must be an object");
    return false;
  }

  Slot program_info = new Tuple(2);
  program_info->replace(0, obj_name );
  program_info->replace(1, words[2] );
  
  command->actor.program_info = program_info;

  command->actor.tell("Entering programming mode.");

  command->actor.programming_mode = 1;

  return true;
}

static Slot list_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;  
  
  Command *command = ((Command*)(Data*)args[0]);

  Slot words = string_to_words( args[1] );
  if (words->length()!=3) {
    command->actor.tell("@list object method");
    return false;
  }
  
  Slot obj_name = words[1];
  if (!(obj_name[0].equal("$"))) {
    command->actor.tell("first argument must be an object");
    return false;
  }

  obj_name = obj_name->slice(1,-1);
  Slot object = store->Names->slice(obj_name)[2];
  Slot method_name = words[2];

  Frame *method = ((Frame*)(Data*)object->slice(method_name)[2]);

  Slot source = ((Closure*)(Data*)method->getClosure())->getSource();
  if (source) {
    Slot label = new String("Source for ");
    label=label->concat(obj_name)->concat(".")->concat(method_name)->concat(":");
    command->actor.tell( label );

    Slot it = source->iterator();
    while (it->More()) {
      Slot line = it->Next();
      command->actor.tell( line );
    }
  } else {
    command->actor.tell("Source not available.");
  }
  command->actor.tell("---");
      
  return true;
}

static Slot quit_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;  

  Command *command = ((Command*)(Data*)args[0]);

  command->actor.tell("** Disconnected **");

  Slot conn = command->actor.conn;
  int sockfd = conn.sock;
  shutdown(sockfd, 2);
  command->actor.conn = (Data*)NULL;

  command->actor.disconnect();



  return true;
}

static Slot who_run( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;  

  Command *command = ((Command*)(Data*)args[0]);

  Slot actor = command->actor;
  
  actor.tell("Connected users:");
  Slot world = names.world;
  Slot connections = world.connections();
  Slot c_it = connections->iterator();
  while (c_it->More()) {
    Slot conn = c_it->Next();
    Slot user = conn.user;
    if (user)
      actor.tell( user.name() );
  }
  actor.tell("---");

  return true;
}

const Slot defineGlobalVerbs( Frame *context )
{
  

  Slot Verb = names.verb;
  Slot Dictionary = names.dictionary;

  Slot verbs = Dictionary.verbs;

  Slot getVerb = Verb.create_child("get_verb");
  getVerb.set_names("get", "take");
  getVerb.run = &get_run;
  names.get_verb = getVerb;
  
  Slot dropVerb = Verb.create_child("drop_verb");
  dropVerb.set_names("drop");
  dropVerb.run = &drop_run;
  names.drop_verb = dropVerb;
  
  Slot lookVerb = Verb.create_child("look_verb");
  lookVerb.set_names("look");
  lookVerb.run = &look_run;
  names.look_verb = lookVerb;
  
  Slot evalVerb = Verb.create_child("eval_verb");
  evalVerb.set_names("eval");
  evalVerb.run = &eval_run;
  names.eval_verb = evalVerb;
  
  Slot sayVerb = Verb.create_child("say_verb");
  sayVerb.set_names("say");
  sayVerb.run = &say_run;
  names.say_verb = sayVerb;
  
  Slot emoteVerb = Verb.create_child("emote_verb");
  emoteVerb.set_names("emote");
  emoteVerb.run = &emote_run;
  names.emote_verb = emoteVerb;
  
  Slot quitVerb = Verb.create_child("quit_verb");
  quitVerb.set_names("quit");
  quitVerb.run = &quit_run;
  names.quit_verb=quitVerb;
  
  Slot programVerb = Verb.create_child("program_verb");
  programVerb.set_names("@program");
  programVerb.run = &start_programming_run;
  names.program_verb = programVerb;
  
  Slot listVerb = Verb.create_child("list_verb");
  listVerb.set_names("@list");
  listVerb.run = &list_run;
  names.list_verb = listVerb;

  Slot whoVerb = Verb.create_child("who_verb");
  whoVerb.set_names("@who");
  whoVerb.run = &who_run;
  names.who_verb = whoVerb;
  
  verbs = verbs->insert( getVerb );
  verbs = verbs->insert( dropVerb );
  verbs = verbs->insert( lookVerb );
  verbs = verbs->insert( evalVerb );
  verbs = verbs->insert( sayVerb );
  verbs = verbs->insert( emoteVerb );
  verbs = verbs->insert( quitVerb );
  verbs = verbs->insert( programVerb );
  verbs = verbs->insert( listVerb );
  verbs = verbs->insert( whoVerb );

  Dictionary.verbs = verbs;
  
  return true;
}
