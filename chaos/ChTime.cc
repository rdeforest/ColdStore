// Chaos time routines
// Copyright (C) 2002 Colin McCormack, Robert de Forest
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

//#define DEBUGLOG 1
#include "semantics.hh"
#include <BigInt.hh>
#include <sys/time.h>
#include <time.h>

void Chaos::ChGetTimeOfDay()
{
    timeval now;
    
    gettimeofday(&now, 0);

    tSlot<BigInt> sec = new BigInt(now.tv_sec), 
                 usec = new BigInt(now.tv_usec);

    tSlot<List> ret = new List(2);

    ret->push(sec);
    ret->push(usec);
    
    ChPush(ret);
}
