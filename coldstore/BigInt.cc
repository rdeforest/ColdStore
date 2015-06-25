// Integer.cc - ColdStore interface to Integer
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: BigInt.cc,v 1.8 2002/02/09 05:09:51 coldstore Exp $";

#include <stdlib.h>
#include <stdio.h>

#include "Data.hh"
#include "Store.hh"

#include "Vector.hh"
#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Integer.hh"
#include "BigInt.hh"
#include "Real.hh"

#include "tSlot.th"
template union tSlot<BigInt>;

BigInt::BigInt(long value)
{
    mpz_init_set_si(_value, value);
    DEBLOG(cerr << "BigInt " << this << " value " << _value << '\n');
}

BigInt::BigInt(const Slot &value)
{
    if (value.isEncInt()) {
        mpz_init_set_si(_value, (long)value);
    } else if (AKO(value,Integer)) {
        mpz_init_set_si(_value, (long)value);
    } else if (AKO(value,BigInt)) {
        mpz_set(_value, (mpz_t)((BigInt*)value));
    } else if (AKO(value,Real)) {
        mpz_set_f(_value, ((Real*)value)->asGMP());
    } else {
        // last chance - try treating it as a string
        ((String*)value)->nullterm();
        mpz_init_set_str(_value, (char*)value, 0);
    }
    DEBLOG(cerr << "BigInt " << this << " value " << _value << '\n');
}

BigInt::BigInt(const char *value)
{
    mpz_init_set_str(_value, value, 0);
    DEBLOG(cerr << "BigInt " << this << " value " << _value << '\n');
}

BigInt::BigInt (const mpz_t &value)
{
    mpz_set(_value, value);
}

BigInt::BigInt(const mpf_t &value)
{
    mpz_set_f(_value, value);
}

BigInt::~BigInt()
{
    DEBLOG(cerr << "Destroy BigInt " << this << '\n');
    mpz_clear (_value);
}

Data *BigInt::clone(void *where) const
{
    return new (where) BigInt(_value);
}

void BigInt::check(int) const {}

// 1,0,-1 depending on order
int BigInt::order(const Slot &arg) const
{
    if (AKO(arg,BigInt)) {
        // BigInt argument
        return mpz_cmp(_value, ((BigInt*)arg)->_value);
    } else if (arg.isNumeric()) {
        // Integer argument
        return mpz_cmp_ui(_value, (long)arg);
    } else {
        // try to construct it
        BigInt div = BigInt(arg);
        return mpz_cmp(_value, div._value);
    }
}

// the object's truth value
bool BigInt::truth() const
{
    return mpz_sgn(_value) != 0;
}

// constructor args to recreate object
Slot BigInt::toconstruct() const
{
    int size = mpz_sizeinbase (_value, 10);
    char *buf = (char*)alloca(size + 2);
    
    return new String(mpz_get_str(buf, 10, _value));
}

ostream &BigInt::dump(ostream& out) const
{
    int size = mpz_sizeinbase (_value, 10);
    char *buf = (char*)alloca(size + 2);
    mpz_get_str(buf, 10, _value);
    out << buf;
    return out;
}

// construct from constructor args
Slot BigInt::construct(const Slot &arg)
{
    if (arg.isNumeric()) {
        return new BigInt((int)(arg[0]));
    } else
        return new BigInt((const char *)(arg[0]));
}

// equality predicate
bool BigInt::equal(const Slot &arg) const
{
    return order(arg) == 0;
}


// arithmetic
// monadic `+', absolute value
Slot BigInt::positive() const
{
    BigInt *result = new BigInt;
    mpz_abs(result->_value, _value);
    return result;
}

// monadic `-', negative absolute value
Slot BigInt::negative() const
{
    BigInt *result = new BigInt;
    mpz_neg(result->_value, _value);
    return result;
}

// dyadic `+', add
Slot BigInt::add(const Slot &arg) const
{
    BigInt *result = new BigInt;

    if (AKO(arg,BigInt)) {
        // BigInt argument
        mpz_add(result->_value, _value, ((BigInt*)arg)->_value);
    } else if (arg.isNumeric()) {
        // Integer argument
        long addend = (long)arg;
        if (addend < 0) {
            mpz_sub_ui(result->_value, _value, ::abs(addend));
        } else {
            mpz_add_ui(result->_value, _value, addend);
        }
        return result;

    } else {
        // try to construct it
        BigInt addend = BigInt(arg);
        mpz_add(result->_value, _value, addend._value);
    }
    return result;
}

// dyadic `-', subtract
Slot BigInt::subtract(const Slot &arg) const
{
    BigInt *result = new BigInt;
    
    if (AKO(arg,BigInt)) {
        // BigInt argument
        mpz_sub(result->_value, _value, ((BigInt*)arg)->_value);
    } else if (arg.isNumeric()) {
        // Integer argument
        long sub = (long)arg;
        if (sub < 0) {
            mpz_add_ui(result->_value, _value, ::abs(sub));
        } else {
            mpz_sub_ui(result->_value, _value, sub);
        }
        return result;

    } else {
        // try to construct it
        BigInt sub = BigInt(arg);
        mpz_sub(result->_value, _value, sub._value);
    }
    return result;
}

// dyadic `*', multiply
Slot BigInt::multiply(const Slot &arg) const
{
    BigInt *result = new BigInt;
    
    if (AKO(arg,BigInt)) {
        // BigInt argument
        mpz_mul(result->_value, _value, ((BigInt*)arg)->_value);
    } else if (arg.isNumeric()) {
        // Integer argument
        long mult = (long)arg;
        mpz_mul_ui(result->_value, _value, ::abs(mult));
        if (mult < 0) {
            return result->negative();
        } else {
            return result;
        }
    } else {
        // try to construct it
        BigInt mult = BigInt(arg);
        mpz_mul(result->_value, _value, mult._value);
    }
    return result;
}

// dyadic '/', divide
Slot BigInt::divide(const Slot &arg) const
{
    BigInt *result = new BigInt;
    
    if (AKO(arg,BigInt)) {
        // BigInt argument
        mpz_tdiv_q(result->_value, _value, ((BigInt*)arg)->_value);
    } else if (arg.isNumeric()) {
        // Integer argument
        long div = (long)arg;
        mpz_tdiv_q_ui(result->_value, _value, ::abs(div));
        if (div < 0) {
            return result->negative();
        } else {
            return result;
        }
    } else {
        // try to construct it
        BigInt div = BigInt(arg);
        mpz_tdiv_q(result->_value, _value, div._value);
    }
    return result;
    // TODO: handle arithmetic exceptions
}

// dyadic '%', modulo
Slot BigInt::modulo(const Slot &arg) const
{
    BigInt *result = new BigInt;
    if (arg.isNumeric()) {
        // Integer argument
        long div = (long)arg;
        div = mpz_mod_ui(result->_value, _value, ::abs(div));
        if (div < 0) {
            return -div;
        } else {
            return div;
        }
    } else if (AKO(arg,BigInt)) {
        // BigInt argument
        mpz_mod(result->_value, _value, ((BigInt*)arg)->_value);
    } else {
        // try to construct it
        BigInt div(arg);
        mpz_mod(result->_value, _value, div._value);
    }
    return result;
    // TODO: handle arithmetic exceptions
}

// bitwise
// unary '~', invert
Slot BigInt::invert() const
{
    BigInt *result = new BigInt;
    mpz_com(result->_value, _value);
    return result;
}

// dyadic '&', bitwise and
Slot BigInt::and(const Slot &arg) const
{
    BigInt *result = new BigInt;

    if (AKO(arg,BigInt)) {
        // BigInt argument
        mpz_and(result->_value, _value, ((BigInt*)arg)->_value);
    } else {
        // try to construct it
        BigInt addend = BigInt(arg);
        mpz_and(result->_value, _value, addend._value);
    }
    return result;
}

// dyadic '^', bitwise xor
Slot BigInt::xor(const Slot &arg) const
{
    return or(arg)->and(and(arg)->invert());
}

// dyadic '|', bitwise or
Slot BigInt::or(const Slot &arg) const
{
    BigInt *result = new BigInt;

    if (AKO(arg,BigInt)) {
        // BigInt argument
        mpz_ior(result->_value, _value, ((BigInt*)arg)->_value);
    } else {
        // try to construct it
        BigInt addend = BigInt(arg);
        mpz_ior(result->_value, _value, addend._value);
    }
    return result;
}

// dyadic '<<', left shift
Slot BigInt::lshift(const Slot &arg) const
{
    BigInt *result = new BigInt;
    
    mpz_mul_2exp(result->_value, _value, (int)arg);
    return result;
}

// dyadic '>>', right shift
Slot BigInt::rshift(const Slot &arg) const
{
    BigInt *result = new BigInt;
    
    mpz_tdiv_q_2exp(result->_value, _value, (int)arg);
    return result;
}

Slot BigInt::hamming(const Slot &arg)
{
    if (AKO(arg,BigInt)) {
        // BigInt argument
        return mpz_hamdist(_value, ((BigInt*)arg)->_value);
    } else {
        // try to construct it
        BigInt addend = BigInt(arg);
        return mpz_hamdist(_value, addend._value);
    }
}

// misc BigInt transforms to implement Builtins
BigInt *BigInt::random()
{
    Data::Unimpl("Stub - return a random integer");
    return (BigInt*)0;
}

Integer *BigInt::asInteger()
{
    size_t size = mpz_sizeinbase(_value, 16);
    if (size > 8) {
        throw new Error("precision", *this, "too large to convert to Integer");
    }
    return new Integer(mpz_get_si(_value));
}

int BigInt::asint()
{
    size_t size = mpz_sizeinbase(_value, 16);
    if (size > 8) {
        throw new Error("precision", *this, "too large to convert to Integer");
    }
    return mpz_get_si(_value);
}

// Allocation interface to gmp.
void *BigInt::GmpAlloc(size_t alloc_size)
{
    void *mem = malloc(alloc_size);
    DEBLOG(cerr << "GmpAlloc(" << alloc_size
         << ") ->" << mem
         << " ( actual size: " << Memory::size(mem) << ")"
         << '\n');
    return mem;
}

void *BigInt::GmpReAlloc(void *ptr, size_t old_size, size_t new_size)
{
    void *nptr = malloc(new_size);
    DEBLOG(cerr << "GmpReAlloc(" << ptr
         << ", " << old_size
         << ", " << new_size 
         << ") -> " << nptr
         << " ( actual size: " << Memory::size(nptr) << ")"
         << '\n');
    memcpy(nptr, ptr, old_size);
    return nptr;
#if 0
    cerr << "GmpReAlloc(" << ptr
         << ", " << old_size
         << ", " << new_size 
         << ")\n";
    if (new_size == 0)
        new_size = 16;
    return realloc(ptr, new_size);
#endif
}

void BigInt::GmpDeAlloc(void *ptr, size_t size)
{
    DEBLOG(cerr << "GmpDeAlloc(" << ptr
         << ", " << size
         << ")\n");
    free(ptr);
}

// Local Variables ***
// c-default-style:gnu ***
