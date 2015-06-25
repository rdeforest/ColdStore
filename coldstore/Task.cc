// cTask - task class - vestigal as yet
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused)) = "$Id: Task.cc,v 1.2 2000/04/12 04:29:34 skeptopotamus Exp $";

#include "Data.hh"
#include "Store.hh"
#include "Task.hh"

Slot cTask::object = 0;

// operative Object in Task
Data *cTask::currObj()
{
  return (Data*)this;
}

// predicate:  is current object System?
bool cTask::isSys()
{
  extern Store *store;
  return (Data*)(store->Root) == currObj();
}
