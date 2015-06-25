// Capability - capability wrapper
// map calls on Capability under the virtual protocol to another object's vp via a capability filter
// Copyright (C) 1998,1999,2000 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef CAP_HH
#define CAP_HH

class Capability
    : public Tuple
{
    Slot object;	// the capability's object
    Slot qualifiers;	// the capability's qualifiers
};

#endif


