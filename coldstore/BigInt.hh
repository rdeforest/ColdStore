// BigInt - GNU gmp integer interface
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: BigInt.hh,v 1.6 2002/04/08 02:32:05 coldstore Exp $	

#ifndef BIGINT_HH
#define BIGINT_HH

#include <gmp.h>

/** BigInt - arbitrary precision integer via the gnu gmp library
 */
class BigInt
    : public Data
{
protected:
    mpz_t _value;
    int base;

public:
    BigInt(long value = 0);
    BigInt(const Slot &value);
    BigInt(const char *value);
    BigInt (const mpz_t &value);
    BigInt(const mpf_t &value);
    
    virtual ~BigInt();

    Integer *asInteger();
    //operator mpz_t const &() const {return _value;}
    mpz_t const &asGMP() const {return _value;}
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
    
    // bitwise
    virtual Slot invert() const;			// unary '~', invert
    virtual Slot and(const Slot &arg) const;		// dyadic '&', bitwise and
    virtual Slot xor(const Slot &arg) const;		// dyadic '^', bitwise xor
    virtual Slot or(const Slot &arg) const;		// dyadic '|', bitwise or
    virtual Slot lshift(const Slot &arg) const;	// dyadic '<<', left shift
    virtual Slot rshift(const Slot &arg) const;	// dyadic '>>', right shift
    
    // misc BigInt transforms to implement Builtins
    static BigInt *random();		// rand()

    Slot hamming(const Slot &arg);

    // Allocation interface to gmp.
    static void *GmpAlloc(size_t alloc_size);
    static void *GmpReAlloc(void *ptr, size_t old_size, size_t new_size);
    static void GmpDeAlloc(void *ptr, size_t size);
};

#endif
// Local Variables ***
// c-default-style:gnu ***
