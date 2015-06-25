// Parser.cc - Infocom-esque command parser
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

// Originally based on Parser from Python Universe Builder 6/01/98 by JJS

#include <Data.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <String.hh>
#include <Integer.hh>
#include <List.hh>
#include <Closure.hh>
#include <Error.hh>
#include <Message.hh>

#include "common.hh" 
#include <string.h>


static Slot parseVerb( Frame *context ) {
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot words = self.words;

  Slot verb_word = words->Next();
  self.words = words;

  Slot env = self.environment();
  Slot env_it = env->iterator();

  Slot matches = new List();
  while (env_it->More()) {
    Slot o = env_it->Next();
    Slot m = o.matchVerbsByName( verb_word );
    m->iterator();
    while (m->More()) {
      Slot who = m->Next();
      Slot pair = new Tuple(2);
      matches = matches->insert( pair->replace(0, who)->replace(1, o));
    }
  }

  if (!matches->length())
    throw new Error("parse", verb_word, "I don't understand that.");

  return matches;
}

static Slot expandSpecials( Frame *context ) {
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot argstr = args[0];

  if (argstr->slice(0).equal("\"")) {
    argstr = argstr->del(0);
    argstr = argstr->insert(0, "say ");
  } else if (argstr->slice(0).equal(":")) {
    argstr = argstr->del(0);
    argstr = argstr->insert(0, "emote ");
  } else if (argstr->slice(0).equal(";")) {
    argstr = argstr->del(0);
    argstr = argstr->insert(0, "eval ");
  }
  return argstr;
}

static Slot parseLine( Frame *context ) {
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot argstr = args[0];

  argstr = self.expandSpecials( argstr );

  Slot words = argstr.explodeWords();
  self.words = words;

  Slot cmd_r = store->Names->slice("command")[2];
  Slot cmd = cmd_r.create("tmp_command");

  cmd.setActor( self );
  
  cmd.setEnvironment( self.environment() );
  cmd.setWords( words );
  cmd.setArgstr( argstr );

  // POP VERB
  Slot verbs = self.parseVerb();

  // FILTER VERBS
  Slot v_it = verbs->iterator();
  Slot filt = new List();
  while (v_it->More()) {
    Slot v_e = v_it->Next();
    Slot verb = v_e[0];
    Slot on = v_e[1];
    if (verb.canHandle_on( cmd, on )) {
      filt = filt->insert( v_e );
    }
  }

  if (filt->length() > 1)
    throw new Error("parse", filt, "Which verb did you mean?");

  if (!filt->length())
    throw new Error("parse", argstr, "I don't understand that.");

  Slot v_e = filt->Next();
  Slot verb = v_e[0];
  Slot on = v_e[1];
  verb.invoke_on( cmd, on );

  return true;
}

static Slot initialize( Frame *context )
{
  Slot self = context->_this;

  // CREATE COMMAND
  Slot cmd = store->Names->slice("command")[2].create("tmp_command");
  self.cmd = cmd;

  return true;
}


Slot createParser( Frame *context )
{
  Slot self = context->_this;

  Slot root = self.root;

  Slot parser = root.create("parser");
  parser.initialize = &initialize;
  parser.parseVerb = &parseVerb;
  parser.parseLine = &parseLine;
  parser.expandSpecials = &expandSpecials;

  parser.initialize();

  return parser;
}

