// Null - ColdStore interface to nothing
// Copyright (C) 2001 Robert de Forest
//

static char *id __attribute__((unused))="$Id: Null.cc,v 1.3 2001/10/29 09:16:43 coldstore Exp $";

#include <stdlib.h>
#include <stdio.h>

#include <qvmm.h>
#include <Q.hh>
#include "Data.hh"
#include "Store.hh"
#include "Error.hh"

#include "String.hh"
#include "Null.hh"

#include "tSlot.th"
template union tSlot<Null>;

Null::Null() {}
Null::~Null() {}

Data *Null::clone(void *where) const
{
    return new (where) Null();
}

void Null::check(int) const {};

int Null::order(const Slot &arg) const
{
    return typeOrder(arg);
}

bool Null::truth() const
{
    return false;
}

Slot Null::toconstruct() const
{
    return new String("NULL");
}

ostream &Null::dump(ostream& out) const
{
    out << "NULL";
    return out;
}

bool Null::equal(const Slot &arg) const
{
    return typeOrder(arg) == 0;
}

Slot Null::construct(const Slot &arg)
{
    return new Null();
}

