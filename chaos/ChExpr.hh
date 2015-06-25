// ChExpr.hh - ChExpr functionals
// Copyright (C) 2002 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef CHEXPR_HH
#define CHEXPR_HH
#include "semantics.hh"
#include <tSlot.hh>

/** ChExpr is a Chaos expression as an object.
 * 
 * ChExpr responds to the ::call vp element by pushing call's arguments
 * followed by its own expression, then evaluating, and returning the top of stack
 * 
 * This can be used to encapsulate a Chaos expression, e.g. for Compose and Promise
 */
class ChExpr
    : public Tuple
{
    // NB: Tuple overlays the following
    
    /** the expression to be evaluated */
    Slot expr;
    
    /** the evaluator to use */
    tSlot<Chaos> chaos;
    
public:
    /** create a ChExpr
     * @param _first first element
     * @param _second second element
     */
    ChExpr(Slot _expr, tSlot<Chaos> _chaos);
    
    /** create a ChExpr
     * @param args arguments to create
     */
    ChExpr(const Slot &args);
    
    virtual ~ChExpr();
    
public:
    
    /** call object
     */
    virtual Slot call(Slot &args);
};

#endif

