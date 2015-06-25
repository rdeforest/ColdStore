// ChExpr.cc - ChExpr functionals
// Copyright (C) 2002 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#include "ChExpr.hh"

template union tSlot<ChExpr>;

ChExpr::ChExpr(Slot _expr, tSlot<Chaos> _chaos)
    : Tuple(2), expr(_expr), chaos(_chaos)
{}

ChExpr::ChExpr(const Slot &args)
    : Tuple(2), expr(args[0])
{
    if (args->length() > 1) {
	chaos = args[1];
    } else {
	chaos = new Chaos();
    }
}

ChExpr::~ChExpr()
{}

Slot ChExpr::call(Slot &args)
{
    DEBLOG(cerr << "call\n");
  
    // if object's Chaos evaluator is NULL, then create a temporary/local Chaos
    tSlot<Chaos> local_chaos = chaos;
    if (!local_chaos)
	local_chaos = new Chaos();
    
    for (int i = 0; i < args->length(); i++) {
	Slot datum = args[i];
	if (AKO(datum, Symbol)) {
	    // symbols' values are interpreted
	    local_chaos->ChPush(datum[2]);
	    local_chaos->ChEval();
	} else {
	    // everything else is just pushed
	    local_chaos->ChPush(datum);
	}
    }
    
    va_end(ap);
    
    local_chaos->ChPush(expr);
    local_chaos->ChEval();
    return local_chaos->ChPop();
}

