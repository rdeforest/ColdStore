// HasVerbs.cc: builds the $has_verbs prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

/** An has_verbs is a parent for any object that wishes to match verbs.
 */

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Frame.hh>
#include <Store.hh>
#include <Closure.hh>
#include <List.hh>

#include "common.hh"

/** addVerb: verb 
 *  @param verb the verb object associated with the verb
 */
static Slot addVerb( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot verb = args[0];

  Slot verbs = self.verbs;
  verbs = verbs->insert( verb );
  self.verbs = verbs;

  return true;
}

/** removeVerb: verb object
 *  @param verb the verb object associated with the verb
 */
static Slot removeVerb( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot verbs = self.verbs;
  verbs = verbs->del( verbs->search(args[0]) );
  self.verbs = verbs;

  return true;
}

/** return a list of verbs
 */
static Slot verbs( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  return self.verbs;
}

/** matchVerbsByName: name
 *  search for all verbs with name
 */
static Slot matchVerbsByName( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot name = args[0];
  Slot matches = new List();

  Slot verbs = self.verbs();
  Slot verb_it = verbs->iterator();
  while (verb_it->More()) {
    Slot verb = verb_it->Next();
    Slot names = verb.names();
    if ( names->search(name) )
       matches = matches->insert( verb );
  }

  return matches;
}

static Slot initialize( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  self.verbs = new List();

  return true;
}

Slot createHasVerbs( Frame *context )
{
  Slot self = context->_this;

  Slot root = self.root;

  Slot has_verbs = root.create("has_verbs");

  has_verbs.initialize = &initialize;
  has_verbs.matchVerbsByName = &matchVerbsByName;
  has_verbs.addVerb = &addVerb;
  has_verbs.removeVerb = &removeVerb;
  has_verbs.verbs = &verbs;

  has_verbs.initialize();
  
  return has_verbs;
}
