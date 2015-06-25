// Compose.cc - Compose functionals
// Copyright (C) 2002 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#define DEBUGLOG
#include <List.hh>
#include <Compose.hh>

template union tSlot<Compose>;

Compose::Compose(Slot _first, Slot _second)
  : Tuple(2), first(_first), second(_second)
{
  DEBLOG(cerr << "Compose construction " << *this << '\n');
}

Compose::Compose(const Slot &both)
  : Tuple(2), first(both[0]), second(both[1])
{
  DEBLOG(cerr << "Compose construction " << *this << '\n');
}

Compose::~Compose()
{}

Slot Compose::call(Slot &args)
{
  DEBLOG(cerr << "Compose call: "
	 << *this
	 << " args: " << args
	 << '\n');
  Slot result = new List(second->call(args));
  DEBLOG(cerr << "Compose call: " << *this
	 << " args: " << args
	 << " apply second: " << result << '\n');
  DEBLOG(cerr << "calling first: " << first 
	 << " of type " << first->typeId()
	 << " with " << result << '\n');
  result = first->call(result);
  DEBLOG(cerr << "Compose call: " << *this
	 << " apply first: " << result << '\n');
  return result;
}
