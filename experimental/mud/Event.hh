// Event - Event class defines
// Copyright (C) 2000,2 001 Colin McCormack, Ryan Daum 

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// $Id 

#ifndef EVENT_HH
#define EVENT_HH

#include <List.hh>
#include <Dict.hh>

/** an Event is basic storage for occurences in the mud
 *
 * Event stores: source, id, and a dict of params
 */
cold class Event
  : public Dict
{

private:
  Slot source;          // source Event of the message
  Slot id;              // unique identifier of the event
  Slot cause;           // who caused the event

public:
  /////////////////
  // Construction

  /** cold construct a Event
   */
  Event(const Slot &source);

  /** construct a blank Event
   */
  Event();

  /** construct an event with source, id, and cause
   */
  Event( const Slot &source, const Slot &cause, const Slot &id );

public:
  ///////////////////
  // Call interface
  
  virtual Slot getSource() const { return source; };
  virtual void setSource(const Slot &inSource) {  source = inSource; };

  virtual Slot getCause() const { return cause; };
  virtual void setCause(const Slot &inCause) {  source = inCause; };

  virtual Slot getID() const { return id;} ;
  virtual void setID(const Slot &inID) { id = inID; } ;

public:
  ///////////////////////////////
  // Coldmud Interface

  // structural
  virtual Data *clone(void *store = (void*)0) const;
  virtual Data *mutate(void *where) const;

};

#endif

