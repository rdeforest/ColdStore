// ConsoleObserver.cc: watches events and announces them to the console
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>
#include <Closure.hh>
#include "Event.hh"

#include "common.hh"

/** receives an event and dumps it to stderr
 */
static Slot receiveEvent( Frame *context )
{
  Slot self = context->_this;
  Slot arguments = context->_arguments;
  
  Event *event = ((Event*)(Data*)arguments[0]);
  
  cerr << "ConsoleObserver EVENT: " << event->getID() << " SOURCE: $" << (char*)event->getSource().id() << " CAUSE: $" << (char*)event->getCause().id() << endl;
  return true;

}

Slot createConsoleObserver( Frame *context )
{
  Slot self = context->_this;
  
  Slot thing = self.thing;
  
  Slot ConsoleObserver = thing.create("console_observer");
  ConsoleObserver.setNames("console observer", "observer", "console");

  // add the reception method
  ConsoleObserver.receiveEvent = &receiveEvent;

  return ConsoleObserver;
}
