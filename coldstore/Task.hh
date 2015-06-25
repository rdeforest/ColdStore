// cTask - task class - vestigal as yet
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
//	$Id: Task.hh,v 1.2 2000/04/12 04:29:34 skeptopotamus Exp $

#ifndef TASK_HH
#define TASK_HH

class cTask
  : public Data
//    public Task
{

protected:
  virtual ~cTask();

public:
  static Slot object;
  bool isSys();		// predicate:  is current object System?
  Data *currObj();	// operative Object in Task

  // Task operator Builtins - all stubs
  Slot opTask_id(Slot&);	//
  Slot opCancel(Slot&);		// (admin)
  Slot opSuspend(Slot&);	// (admin)
  Slot opResume(Slot&);		// (admin)
  Slot opPause(Slot&);		// (admin)
  Slot opTasks(Slot&);		// (admin)
  Slot opCallers(Slot&);	// (admin)
  Slot opTick(Slot&);		//
  Slot opTicks_left(Slot&);	//
};

inline cTask *currTask()
{
  return (cTask*)NULL;
}

inline Data *currObj()
{
  return (Data*)0;
  //  return ((cTask*)(Tram::task()))->currObj();
}

inline bool isSys()
{
    return false;
    //return ((cTask*)(Tram::task()))->isSys();
}

#endif
