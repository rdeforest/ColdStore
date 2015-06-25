// Real - ColdStore interface GNU gmp arbitrary precision floats
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: Real.hh,v 1.7 2002/04/08 02:32:05 coldstore Exp $

#ifndef REAL_HH
#define REAL_HH

#include <gmp.h>

/** Real - arbitrary precision floats via the gnu gmp library
 */
class Real
    : public Data
{
protected:
    mpf_t _value;

public:
    Real(long value);
    Real(double value = 0.0);
    Real(const Slot &value);
    Real(const char *value);
    Real(const mpz_t &value);
    Real(const mpf_t &value);

    virtual ~Real();

    //operator mpf_t const &() const {return _value;}
    mpf_t const &asGMP() const {return _value;}
    BigInt *asBigInt();
    Integer *asInteger();
    int asint();

    // structural
    virtual Data *clone(void * = 0) const;
    virtual void check(int=0) const;	// check an instance of Data
    
    // object
    virtual bool truth() const;		// the object's truth value
    virtual Slot toconstruct() const;	// constructor args to recreate object
    virtual ostream &dump(ostream& out) const;	// dump the object
    
    static Slot construct(const Slot &arg);	// construct from constructor args
    virtual int order(const Slot &arg) const;	// 1,0,-1 depending on order
    virtual bool equal(const Slot &arg) const;	// equality predicate
    
    // arithmetic
    virtual Slot positive() const;		// monadic `+', absolute value
    virtual Slot negative() const;		// monadic `-', negative absolute value
    virtual Slot add(const Slot &arg) const;	// dyadic `+', add
    virtual Slot subtract(const Slot &arg) const;	// dyadic `-', subtract
    virtual Slot multiply(const Slot &arg) const;	// dyadic `*', multiply
    virtual Slot divide(const Slot &arg) const;		// dyadic '/', divide
    virtual Slot modulo(const Slot &arg) const;		// dyadic '%', modulo

    // local
    static void precision(unsigned long prec);
};

#endif
// Local Variables ***
// c-default-style:gnu ***
