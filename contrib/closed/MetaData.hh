// MetaData - MetaObject for exposing layer1 facilities via messages.  singleton
// Copyright (C) 2001 Ryan Daum
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

// define one thing only: pointer to the MetaObject instance which can be used
// as a metaclass for access to layer1 vp

#ifndef METACLASS_HH
#define METACLASS_HH

#include "Data.hh"
#include "Frame.hh"
#include "Message.hh"
#include "Object.hh"

cold class MetaData
  : public Object
{
public:
  static Slot instance;

public:
  virtual Slot receive( Message *msg, Frame *context );
};

#endif
