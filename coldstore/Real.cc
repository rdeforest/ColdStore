// Real - ColdStore interface GNU gmp arbitrary precision floats
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Real.cc,v 1.11 2002/02/09 05:09:51 coldstore Exp $";

#include <stdlib.h>
#include <stdio.h>

//#define DEBUGLOG
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
template union tSlot<Real>;

Real::Real(const Slot &value)
{
    if (value.isEncInt()) {
        mpf_init_set_si(_value, (long)value);
    } else if (AKO(value,Integer)) {
        mpf_init_set_si(_value, (long)value);
    } else if (AKO(value,BigInt)) {
        mpf_init(_value);
        mpf_set_z(_value, ((BigInt*)value)->asGMP());
    } else if (AKO(value,Real)) {
        mpf_init_set(_value, ((Real*)value)->_value);
    } else {
        // last chance - try treating it as a string
        ((String*)value)->nullterm();
        mpf_init_set_str(_value, (char*)value, 0);
    }
    DEBLOG(cerr << "Real " << this << " value " << _value << '\n');
}

Real::Real(long value)
{
    mpf_init_set_si(_value, value);
    DEBLOG(cerr << "Real " << this << " value " << _value << '\n');
}

Real::Real(double value)
{
    mpf_init_set_d(_value, value);
    DEBLOG(cerr << "Real " << this << " value " << _value << '\n');
}

Real::Real(const char *value)
{
    mpf_init_set_str(_value, value, 0);
    DEBLOG(cerr << "Real " << this << " value " << _value << '\n');
}

Real::Real (const mpf_t &value)
{
    mpf_set(_value, value);
}

Real::Real (const mpz_t &value)
{
    mpf_set_z(_value, value);
}

Real::~Real()
{
    DEBLOG(cerr << "Destroy Real " << this << '\n');
    mpf_clear (_value);
}

Data *Real::clone(void *where) const
{
    return new (where) Real(_value);
}

void Real::check(int) const {}

// 1,0,-1 depending on order
int Real::order(const Slot &arg) const
{
    if (AKO(arg,Real)) {
        // Real argument
        return mpf_cmp(_value, ((Real*)arg)->_value);
    } else if (arg.isNumeric()) {
        // Integer argument
        return mpf_cmp_ui(_value, (long)arg);
    } else {
        // try to construct it
        Real div = Real(arg);
        return mpf_cmp(_value, div._value);
    }
}

// the object's truth value
bool Real::truth() const
{
    return mpf_sgn(_value) != 0;
}

// constructor args to recreate object
Slot Real::toconstruct() const
{
    mp_exp_t exp;
    char *mantissa = mpf_get_str(NULL, &exp, 10, 0, _value);
    char *ebuf = (char *)alloca(strlen(mantissa) + 16);
    sprintf(ebuf, "%c.%sE%d", mantissa[0], mantissa + 1, (int)exp - 1);
    String *b = new String(ebuf);
    free(mantissa);
    return b;
}

ostream &Real::dump(ostream& out) const
{
    mp_exp_t exp;
    char *mantissa = mpf_get_str(NULL, &exp, 10, 0, _value);
    out << mantissa[0] << '.' << (mantissa + 1) << 'E' << (exp - 1)
        << '#' << mpf_get_prec(_value);
    free(mantissa);
    return out;
}

// construct from constructor args
Slot Real::construct(const Slot &arg)
{
    if (arg.isNumeric()) {
        return new Real((long)(arg[0]));
    } else
        return new Real((const char *)(arg[0]));
}

// equality predicate
bool Real::equal(const Slot &arg) const
{
    return order(arg) == 0;
}


// arithmetic
// monadic `+', absolute value
Slot Real::positive() const
{
    Real *result = new Real;
    mpf_abs(result->_value, _value);
    return result;
}

// monadic `-', negative absolute value
Slot Real::negative() const
{
    Real *result = new Real;
    mpf_neg(result->_value, _value);
    return result;
}

// dyadic `+', add
Slot Real::add(const Slot &arg) const
{
    Real *result = new Real;

    if (AKO(arg,Real)) {
        // Real argument
        mpf_add(result->_value, _value, ((Real*)arg)->_value);
    } else if (AKO(arg,BigInt)) {
        // BigInt argument
        Real addend((BigInt*)arg);
        mpf_add(result->_value, _value, addend._value);
        return result;
    } else if (arg.isNumeric()) {
        // Integer argument
        long addend = (long)arg;
        if (addend < 0) {
            mpf_sub_ui(result->_value, _value, ::abs(addend));
        } else {
            mpf_add_ui(result->_value, _value, addend);
        }
        return result;

    } else {
        // try to construct it
        Real addend = Real(arg);
        mpf_add(result->_value, _value, addend._value);
    }
    return result;
}

// dyadic `-', subtract
Slot Real::subtract(const Slot &arg) const
{
    Real *result = new Real;
    
    if (AKO(arg,Real)) {
        // Real argument
        mpf_sub(result->_value, _value, ((Real*)arg)->_value);
    } else if (AKO(arg,BigInt)) {
        // BigInt argument
        Real sub((BigInt*)arg);
        mpf_sub(result->_value, _value, sub._value);
        return result;
    } else if (arg.isNumeric()) {
        // Integer argument
        long sub = (long)arg;
        if (sub < 0) {
            mpf_add_ui(result->_value, _value, ::abs(sub));
        } else {
            mpf_sub_ui(result->_value, _value, sub);
        }
        return result;

    } else {
        // try to construct it
        Real sub = Real(arg);
        mpf_sub(result->_value, _value, sub._value);
    }
    return result;
}
 
// dyadic `*', multiply
Slot Real::multiply(const Slot &arg) const
{
    Real *result = new Real;
    
    if (AKO(arg,Real)) {
        // Real argument
        mpf_mul(result->_value, _value, ((Real*)arg)->_value);
    } else if (AKO(arg,BigInt)) {
        // BigInt argument
        Real mult((BigInt*)arg);
        mpf_mul(result->_value, _value, mult._value);
        return result;
    } else if (arg.isNumeric()) {
        // Integer argument
        long mult = (long)arg;
        mpf_mul_ui(result->_value, _value, ::abs(mult));
        if (mult < 0) {
            return result->negative();
        } else {
            return result;
        }
    } else {
        // try to construct it
        Real mult = Real(arg);
        mpf_mul(result->_value, _value, mult._value);
    }
    return result;
}

// dyadic '/', divide
Slot Real::divide(const Slot &arg) const
{
    Real *result = new Real;
    
    if (AKO(arg,Real)) {
        // Real argument
        mpf_div(result->_value, _value, ((Real*)arg)->_value);
    } else if (AKO(arg,BigInt)) {
        // Integer argument
        Real div((BigInt*)arg);
        mpf_div(result->_value, _value, div._value);
        return result;
    } else if (arg.isNumeric()) {
        // Integer argument
        long div = (long)arg;
        mpf_div_ui(result->_value, _value, ::abs(div));
        if (div < 0) {
            return result->negative();
        } else {
            return result;
        }
    } else {
        // try to construct it
        Real div = Real(arg);
        mpf_div(result->_value, _value, div._value);
    }
    return result;
    // TODO: handle arithmetic exceptions
}

// dyadic '%', modulo
Slot Real::modulo(const Slot &arg) const
{
    return unimpl("modulo on Real");
}

BigInt *Real::asBigInt()
{
    return new BigInt(_value);
}

Integer *Real::asInteger()
{
    return new Integer(mpf_get_d(_value));
}

int Real::asint()
{
    return (int)mpf_get_d(_value);
}

void Real::precision(unsigned long prec)
{
    mpf_set_default_prec(prec);
}

// Local Variables ***
// c-default-style:gnu ***
