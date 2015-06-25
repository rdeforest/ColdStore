// Described.cc: builds the $named prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <Closure.hh>

#include "common.hh"

static Slot setDescription( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot desc = args[0];

  self.description = desc;

  return true;
}

static Slot description( Frame *context )
{
  Slot self = context->_this;

  return self.description;
}

Slot createDescribed( Frame *context )
{
  Slot self = context->_this;

  Slot root = self.root;

  Slot described = root.create("described");
  described.setDescription = &setDescription;
  described.description = &description;

  described.setDescription("You see nothing special.");
  
  return described;
}
