// Data - class from which all ColdStore entities must derive
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Data.cc,v 1.54 2002/05/09 13:31:16 coldstore Exp $";

//#define DEBUGLOG
#include <ctype.h>
#include <typeinfo>
#include <stdlib.h>
#include <iostream.h>
#include <stdarg.h>
#include "options.h"

#include "Data.hh"
#include "Store.hh"
#include "Builtin.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"
//#include "Message.hh"
//#include "MetaData.hh"

//static Data *dummy = (Data*)0;

Data::Data()
{}

Data::~Data()
{
    // if we're destroying something that's not unreferenced, it's an error
    assert(refcount() == 0 || ((void*)this > ( (char*)QVMM_BASE + QVMM_SIZE )));
}

// returns a mutable copy of this
Data *Data::mutate(void *) const
{
    return (Data*)this;		// by default - objects are mutable
}

// returns a mutable copy of this
Slot Data::identity(void *) const
{
    return this;		// by default - objects are mutable
}

void Data::check(int) const
{
    validGuard();
    //unimpl("No check operator defined");
}

// type - return the address of this type's RTTI method
// it's assumed this address will be unique, and will uniquely
// characterise the instance.
void *Data::type() const
{
    assert("NULL passed to Data::type()");
    //    throw new Error("illegal", NULL, "NULL passed to Data::type()");
    return this->_vptr;
}

const char *Data::typeId() const
{
    return typeid(*this).name();
}

const char *__type(const Data *d)
{
    return typeid(*d).name();
}

#if 0
bool Data::typeEQ(const Builtin *m) const
{
    void **modrtti = (void **)(m->vtbl()); // pointer to RTTI function

    DEBLOG(cerr << "typeEQ: type()=" << type()
           << ", modrtti=" << modrtti
           << '\n');
    return type() == modrtti;
}
#endif

Slot Data::unimpl(const char *message) const
{
    throw new Error("unimplemented", this, message);
    return this;
}

Slot Data::Unimpl(const char *message)
{
    throw new Error("unimplemented", (Data*)NULL, message);
    return (Data*)NULL;
}

// the object's truth value
bool Data::truth() const
{
    return true;	// Slot will convert this to an encoded Integer
}

ostream &Data::dump(ostream& out) const
{
    if (this != NULL) {
        // out << "[untyped] ";
        out << '{' << typeId() << '}';
        Slot s = toconstruct();
	s.Dump(out);
    } else {
        out << "[NULL]";
    }
    return out;
}

//  constructor args to recreate object
Slot Data::toconstruct() const
{
    return unimpl("No constructor rendering defined");
}

//  1,0,-1 depending on order
int Data::order(const Slot &arg) const
{
    return (Data*)this - (Data *)arg;	// default strong equality
  // Note: Slot will have converted this to an encoded Integer
}

//  equality predicate
bool Data::equal(const Slot &arg) const
{
    return order(arg) == 0;	// default is strong equality
}

//  1,0,-1 depending on order of Type
int Data::typeOrder(const Slot &arg) const
{
    // compare the object type
    return (int)type() - (int)(arg->type());
    //  return strcmp(typeid(this).name(), typeid((Data*)arg).name());
}

//  unary '+', absolute value
Slot Data::positive() const
{
    return unimpl("No monadic `+' operator defined");
}

//  unary '-', negative absolute value
Slot Data::negative() const
{
    return unimpl("No monadic `-' operator defined");
}

//  dyadic `+', add
Slot Data::add(const Slot &arg) const
{
    return unimpl("No dyadic `+' operator defined");
}

//  dyadic `-', subtract
Slot Data::subtract(const Slot &arg) const
{
    return unimpl("No dyadic `-' operator defined");
}

//  dyadic `*', multiply
Slot Data::multiply(const Slot &arg) const
{
    return unimpl("No dyadic `*' operator defined");
}

//  dyadic '/', divide
Slot Data::divide(const Slot &arg) const
{
    return unimpl("No dyadic `/' operator defined");
}

//  dyadic '%', modulo
Slot Data::modulo(const Slot &arg) const
{
    return unimpl("No dyadic `%' operator defined");
}

// bitwise
//  unary '~', invert
Slot Data::invert() const
{
    return unimpl("No monadic `~' operator defined");
}

//  dyadic '&', bitwise and
Slot Data::and(const Slot &arg) const
{
    return unimpl("No dyadic `&' operator defined");
}

//  dyadic '^', bitwise xor
Slot Data::xor(const Slot &arg) const
{
    return unimpl("No dyadic `^' operator defined");
}

//  dyadic '|', bitwise or
Slot Data::or(const Slot &arg) const
{
    return unimpl("No dyadic `|' operator defined");
}

//  dyadic '<<', left shift
Slot Data::lshift(const Slot &arg) const
{
    return unimpl("No dyadic `<<' operator defined");
}

//  dyadic '>>', right shift
Slot Data::rshift(const Slot &arg) const
{
    return unimpl("No dyadic `>>' operator defined");
}

// predicate - is object a sequence
bool Data::isSequence() const
{
    return false;
}

//  length as sequence
int Data::length() const
{
    return unimpl("No monadic `length' operator defined");
}

//  concatenate two sequences
Slot Data::concat(const Slot &arg) const
{
    return unimpl("No dyadic `concat' operator defined");
}

//  dyadic `[]', index
Slot Data::slice(const Slot &index) const
{
    Slot tmp = (Data*)0;
    return slice(index, tmp);
}

//  dyadic `[]', index
Slot Data::slice(const Slot &from, const Slot &len) const
{
    return unimpl("No dyadic `[]' operator defined");
}

//  dyadic `in', matching subrange
Slot Data::search(const Slot &search) const
{
    return unimpl("No dyadic `search' operator defined");
}

Slot Data::domain() const
{
    return unimpl("No monadic `domain' operator defined");
}

Slot Data::del(Slot const &what)
{
    return unimpl("No diadic `del' operator defined");
}

Slot Data::del(Slot const &from, Slot const &len)
{
    return unimpl("No triadic `del' operator defined");
}

Slot Data::insert(Slot const &before, Slot const &value)
{
    return unimpl("No triadic `insert' operator defined");
}

Slot Data::insert(Slot const &value)
{
    return unimpl("No dyadic `insert' operator defined");
}

//  replace subrange
Slot Data::replace(const Slot &from, const Slot &len, const Slot &value)
{
    return unimpl("No quadradic `[]=' operator defined");
}

//  replace item
Slot Data::replace(const Slot &from, const Slot &value)
{
    return unimpl("No triadic `[]=' operator defined");
}

//  replace
Slot Data::replace(const Slot &value)
{
    return unimpl("No dyadic `[]=' operator defined");
}

//  return an iterator
Slot Data::iterator() const
{
    return unimpl("No iterator operator defined");
}

// return an iterable Sequence as a List
List *Data::toSequence() const
{
    // has to be done the with an iterator
  List *result = (List*)0;
  if (isSequence()) {
    result = new List(length());
    Slot it = iterator();
    
    while (it->More()) {
        result->concat(it->Next());
    }
  } else {
    result = new List(-1, this);
  }
  result->upcount();
  return result;
}

// predicate - is object an iterator?
bool Data::isIterator() const
{
    return false;
}


bool Data::More() const
{
    unimpl("No More operator defined");
    return false;
}

Slot Data::Next()
{
    return unimpl("No Next operator defined");
}

#if 0
Slot Data::call(int nargs, ...)
{
  va_list ap;
  va_start(ap, nargs);
  Slot result = this;
  for (int i = 0; i < nargs; i++) {
    Slot datum = va_arg(ap, Slot);
    result = result->slice(datum);
  }
  va_end(ap);
  return result;
}
#endif

Slot Data::call(Slot &args)
{
  Slot result = this;
  for (int i = 0; i < args->length(); i++) {
    result = result->slice(args[i]);
  }
  return result;
}

// evaluate the object
void Data::reduce(Slot &inexpr, Slot &context)
{
    inexpr = this;
}

Slot Data::connect(const Slot to)
{
  return this;
  //unimpl("No connect operator defined");
}

Slot Data::disconnect(const Slot from)
{
  return this;
  //unimpl("No disconnect operator defined");
}

Slot Data::incoming(const Slot from)
{
    unimpl("No incoming operator defined");
}

Slot Data::outgoing(const Slot to)
{
    unimpl("No outgoing operator defined");
}

/** make this instance mutable
 */
void Data::mkMutable()
{

//    if (_dopeV()->m_vptr) {
//        this->_vptr = (int (**)(...))_dopeV()->m_vptr;
//    } else {
        unimpl("This type's mutability is fixed");
//    }
}

DopeV *Data::dopeV()
{
    DopeV *reflect = (((DopeV*)this->_vptr)-1);
    return reflect;    
}

/** make this instance COW
 */
void Data::mkCOW()
{
    //this->_vptr = (int (**)(...))_dopeV()->vptr;
}

Slot Data::create(const Slot& arg) const
{
  return construct(arg);
}

Slot Data::construct(const Slot& arg)
{
  throw new Error("data",arg,"cannot construct a Data");
}

ostream& operator << (ostream &out, const Data *datum)
{
  if (datum) {
    datum->dump(out);
  } else {
    out << "[NULL]";
  }
  return out;
}

#include <elf.hh>
Slot Data::New(const char *tname, Slot &args, const char *so)
{
    DEBLOG(cerr << "New\n");
    char name[256];
    sprintf(name, "_%d%s.__sizeof", strlen(tname), tname);
    size_t *sizep = (size_t *)Elf::sym_search(name);
    DEBLOG(cerr << "searched " << name << '\n');
    if (!sizep) {
        throw new Error("unimplemented", tname, "This type has no __sizeof property");
    }
    DEBLOG(cerr << name << ": is " << *sizep << " bytes long\n");

    sprintf(name, "__%d%sRC4Slot", strlen(tname), (char*)tname);
    DEBLOG(cerr << "Looking for Builtin " << name << '\n');
    //Builtin *b = ((Symbol*)(store->Registered->slice(name)))->value();
    tSlot<Builtin> b = new Builtin(name, so);
    if (b) {
        int size = (((sizeof(Slot) - 1) + *sizep) / sizeof(Slot));
        Data *space = new (size) Tuple(size);
        // found our type
        DEBLOG(cerr << "calling " << (Data*)b
             << " with " << (Data*)args
             << " at " << (void*)(Data*)space
             << '\n');
        Slot result = b->new_call(2, space, &args);
        //result->upcount();
        DEBLOG(cerr << "result: " << result << '\n');
        return result;
    } else {
        throw new Error("type", tname, "no such type known");
    }
}
