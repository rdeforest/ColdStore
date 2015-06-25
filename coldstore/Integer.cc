// Integer - ColdStore interface to 32 bit integer type
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Integer.cc,v 1.16 2002/02/09 05:09:51 coldstore Exp $";
//#define DEBUGLOG 1
#include <stdlib.h>
#include <stdio.h>

#include <qvmm.h>
#include <Q.hh>
#include "Data.hh"
#include "Store.hh"

#include "Vector.hh"
#include "String.hh"
#include "Tuple.hh"
#include "List.hh"
#include "Error.hh"
#include "Integer.hh"
#include "Pool.th"
#include "BigInt.hh"
#include "Real.hh"


template class Pool<Integer>;
template class gPool<Integer>;
template class Pooled<Integer>;
template class Q< gPool<Integer> >;
template class Qh< gPool<Integer> >;

#include "tSlot.th"
template union tSlot<Integer>;

Integer::Integer(const Slot &init)
    : _value(((Integer*)init)->value())
{
    DEBLOG(cerr << "Integer " << this
           << " value " << _value << '\n');
}

Integer::Integer(long value)
    : _value(value)
{
    DEBLOG(cerr << "Integer " << this
           << " value " << _value << '\n');
}

Integer::~Integer()
{
    DEBLOG(cerr << "Destroy Integer " << this << '\n');
}

Integer *Integer::getInteger(long value)
{
    Integer *result = gPool<Integer>::Get();
    if (!result) {
        throw "Can't get Pool Integer";
    }
    result->_value = value;
    result->upcount();
    return result;
}

long Integer::operator = (long value)
{
    return _value = value;
}

long Integer::value()
{
    return _value;
}

Data *Integer::clone(void *where) const
{
    DEBLOG(cerr << "clone Integer " << this
           << " value " << _value << '\n');
    return new (where) Integer(_value);
}

void Integer::check(int) const {}

// 1,0,-1 depending on order
int Integer::order(const Slot &arg) const
{
    if(AKO(arg,Integer)) {
        int argi = (int)arg;
        return _value - argi;
    } else if (AKO(arg,BigInt)) {
        return -(arg->order(this));
    } else if (AKO(arg,Real)) {
        return -(arg->order(this));
    } else {
        return typeOrder(arg);
    }
}

// the object's truth value
bool Integer::truth() const
{
    return _value != 0;
}

// constructor args to recreate object
Slot Integer::toconstruct() const
{
    char buffer[32];
    sprintf(buffer, "%ld", _value);
    return new String(buffer);
}

extern bool DumpVerbose;
ostream &Integer::dump(ostream& out) const
{
    if (DumpVerbose)
        out << "Integer: " << _value;
    else
        out << _value;
    return out;
}

// equality predicate
bool Integer::equal(const Slot &arg) const
{
    //cerr << _value << " == " << (int)arg << '\n';
    if (arg.isNumeric()) {
        return _value == (int)arg;
    } else {
        return false;
    }
}


// arithmetic
// monadic `+', absolute value
Slot Integer::positive() const
{
    return abs(this);
}

// monadic `-', negative absolute value
Slot Integer::negative() const
{
    return -_value;
}

// dyadic `+', add
Slot Integer::add(const Slot &arg) const
{
    return _value + (int)arg;
}

// dyadic `-', subtract
Slot Integer::subtract(const Slot &arg) const
{
    return _value - (int)arg;
}

// dyadic `*', multiply
Slot Integer::multiply(const Slot &arg) const
{
    return _value * (int)arg;
}

// dyadic '/', divide
Slot Integer::divide(const Slot &arg) const
{
    return _value / (int)arg;
}

// dyadic '%', modulo
Slot Integer::modulo(const Slot &arg) const
{
    return _value % (int)arg;
}

// bitwise
// unary '~', invert
Slot Integer::invert() const
{
    return ~_value;
}

// dyadic '&', bitwise and
Slot Integer::and(const Slot &arg) const
{
    return _value & (int)arg;
}

// dyadic '^', bitwise xor
Slot Integer::xor(const Slot &arg) const
{
    return _value ^ (int)arg;
}

// dyadic '|', bitwise or
Slot Integer::or(const Slot &arg) const
{
    return _value | (int)arg;
}

// dyadic '<<', left shift
Slot Integer::lshift(const Slot &arg) const
{
    return _value << (int)arg;
}

// dyadic '>>', right shift
Slot Integer::rshift(const Slot &arg) const
{
    return _value >> (int)arg;
}


// misc Integer transforms to implement Builtins
Integer *Integer::random()
{
    Data::Unimpl("Stub - return a random integer");
    return (Integer*)0;
}

Integer *Integer::abs(const Integer*)
{
    Data::Unimpl("Stub - return absolute value of integer");
    return (Integer*)0;
}

Slot Integer::construct(const Slot& arg)
{
  return new Integer(arg);
}
