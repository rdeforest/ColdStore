// Promise.cc - Promise functionals
// Copyright (C) 2002 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#define DEBUGLOG
#include <List.hh>
#include <Error.hh>

#include <Promise.hh>

template union tSlot<Promise>;

Promise::Promise(size_t n)
  : Tuple(n)
{}

#if 0
Promise::Promise(const Slot &args)
{
  throw new Error("unimplemented", NULL, "Promise is a virtual class");
}
#endif

Promise::~Promise()
{}

Slot Promise::call(Slot &args)
{
  if ((Data*)this != (Data*)args)
    throw Error("unimplemented", (Data*)0, "Promise called naked");
  return (Data*)0;
}
