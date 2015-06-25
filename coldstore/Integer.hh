// Integer - ColdStore interface to 32 bit integer type
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: Integer.hh,v 1.7 2001/10/05 16:35:42 coldstore Exp $	

#ifndef INTEGER_HH
#define INTEGER_HH
#include <Data.hh>

class iPool;		// opaque type for cells of pools
struct PoolCell;	// opaque type for cells of pools

/** Pool allocated 32 bit signed integers
 */
class Integer
    : public Data
{
protected:
    /** the 32 bit signed integer value of this Integer */
    long _value;

    /** set of all iPool */
    static iPool **_pool;

public:
    Integer(long value = 0);
    Integer(const Slot &init);
    virtual ~Integer();
    static Integer *getInteger(long value = 0);

    long operator = (long value);
    long value();
  
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
    
    // misc Integer transforms to implement Builtins
    static Integer *abs(const Integer*);	// abs()
    static Integer *random();		// rand()

#if 0
    static void Junk() {
        asm(".globl _sizeof"); asm(".size _sizeof,0"); asm(".set _sizeof,100");
    }
#endif
};
#if 0
    static void Junk() {
        asm(".globl _sizeof; .size _sizeof,0; .set _sizeof,%0",
            : "=n" (sizeof(Integer)));
    }
#endif

#endif
// Local Variables ***
// c-default-style:gnu ***
