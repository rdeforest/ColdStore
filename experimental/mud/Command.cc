// Command.cc: builds the $command prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <Closure.hh>
#include <Message.hh>

#include "common.hh"


static Slot environment( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  return self.environment;
}

static Slot setEnvironment( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  self.environment = args[0];

  return true;
}

static Slot words( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  return self.words;
}

static Slot setWords( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  self.words = args[0];

  return true;
}

static Slot iobj( Frame *context ) { Slot self = context->_this; return self.iobj; return true; }
static Slot dobj( Frame *context ) { Slot self = context->_this; return self.dobj; return true; }
static Slot iobjstr( Frame *context ) { Slot self = context->_this; return self.iobjstr; return true; }
static Slot dobjstr( Frame *context ) { Slot self = context->_this; return self.dobjstr; return true; }
static Slot prep( Frame *context ) { Slot self = context->_this; return self.prep; return true; }
static Slot argstr( Frame *context ) { Slot self = context->_this; return self.argstr; return true; }
static Slot iobjlength( Frame *context ) { Slot self = context->_this; return self.iobjlength; return true; }
static Slot dobjlength( Frame *context ) { Slot self = context->_this; return self.dobjlength; return true; }

static Slot setIobj( Frame *context ) { Slot self = context->_this; self.iobj = context->_arguments[0]; return true; } 
static Slot setDobj( Frame *context ) { Slot self = context->_this; self.dobj = context->_arguments[0]; return true; } 
static Slot setIobjstr( Frame *context ) { Slot self = context->_this; self.iobjstr = context->_arguments[0]; return true; } 
static Slot setDobjstr( Frame *context ) { Slot self = context->_this; self.dobjstr = context->_arguments[0]; return true; } 
static Slot setPrep( Frame *context ) { Slot self = context->_this; self.prep = context->_arguments[0]; return true; } 
static Slot setArgstr( Frame *context ) { Slot self = context->_this; self.argstr = context->_arguments[0]; return true; } 
static Slot setIobjLength( Frame *context ) { Slot self = context->_this; self.iobjlength = context->_arguments[0]; return true; } 
static Slot setDobjLength( Frame *context ) { Slot self = context->_this; self.dobjlength = context->_arguments[0]; return true; } 
static Slot setActor( Frame *context ) { Slot self = context->_this; self.actor = context->_arguments[0]; return true; } 
static Slot actor( Frame *context ) { Slot self = context->_this; return self.actor; }

static Slot initialize( Frame *context )
{
  Slot self = context->_this;

  self.words = new List();
  self.environment = new List();

  self.iobj = (Data*)NULL;
  self.dobj = (Data*)NULL;
  self.iobjstr = (Data*)NULL;
  self.dobjstr = (Data*)NULL;
  self.prep = (Data*)NULL;
  self.argstr = (Data*)NULL;
  self.iobjlength = (Data*)NULL;
  self.dobjlength = (Data*)NULL;

  return true;
}


Slot createCommand(Frame *context)
{
  Slot self = context->_this;

  Slot root = self.root;
  Slot command = root.create("command");

  command.initialize = &initialize;
  command.words = &words;
  command.setWords = &setWords;
  command.environment = &environment;
  command.setEnvironment = &setEnvironment;

  command.iobj = &iobj;
  command.dobj = &dobj;
  command.iobjstr = &iobjstr;
  command.dobjstr = &dobjstr;
  command.prep = &prep;
  command.argstr = &argstr;
  command.setIobj = &setIobj;
  command.setDobj = &setDobj;
  command.setIobjstr = &setIobjstr;
  command.setDobjstr = &setDobjstr;
  command.setPrep = &setPrep;
  command.setArgstr = &setArgstr;
  command.dobjlength = &dobjlength;
  command.iobjlength = &iobjlength;
  command.setDobjLength = &setDobjLength;
  command.setIobjLength = &setIobjLength;
  command.setActor = &setActor;
  command.actor = &actor;

  command.initialize();

  return command;
}

