// Receiever.cc: builds the $receiver prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <Message.hh>
#include <Closure.hh>

#include "common.hh"

/** default receive an event - just an interface.  ignore.
 */
static Slot receiveEvent( Frame *context )
{
  Slot self = context->_this;

  return true;
}

Slot createReceiver(Frame *context)
{
  Slot self = context->_this;

  Slot root = self.root;

  Slot receiver = root.create("receiver");
  receiver.receiveEvent = &receiveEvent;

  return receiver;
}
