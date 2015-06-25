// Event.cc: implementation of the Event object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Dict.hh>
#include <Error.hh>
#include <List.hh>

#include "Event.hh"

Event::Event()
{
    source = 0;
    id = 0;
}

// exists only to satisfy Data::New
Event::Event(const Slot &from )
{
    source = 0;
    id = 0;
}

Event::Event( const Slot &inSource, const Slot &inCause, const Slot &inId )
{
    source = inSource;
    id = inId;
    cause = inCause;
}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Event::clone(void *where) const
{
    //  return new (_TupleBase<Slot>::Length(), where) Event(*this);
    return new Event(this);
}

// returns a mutable copy of this
Data *Event::mutate(void *where) const
{
    return (Data*)this;
}

