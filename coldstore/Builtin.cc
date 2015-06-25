// Builtin.cc - ColdStore interface to C++ functions
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Builtin.cc,v 1.17 2002/04/01 13:50:33 coldstore Exp $";

#include <iostream.h>
#include <elf.hh>

#include "Data.hh"
#include "Store.hh"
//#include "Task.hh"

#include "String.hh"
#include "Tuple.hh"
#include "List.hh"
#include "Error.hh"

#include "Builtin.hh"

#include "tSlot.th"
template union tSlot<Builtin>;

// create a Builtin from a Csym
Builtin::Builtin(Csym *_sym)
    : String(_sym->symname()), elf(_sym->in), sym(_sym)
{}

// construct from constructor args
// expects a 2Tuple or List of form (builtin name, file-name)
Builtin::Builtin(const char *_sym, const char *elf_name)
    : String(_sym)
{
    if (!elf_name) {
        elf_name = COLDSTORE;
    }
    
    // locate the elf descriptor for this name
    elf = Elf::findElf(elf_name);
    if (!elf)
        throw new Error("dictnf", elf_name, "Can't find .so file");

    // locate the symbol for this name
    sym = elf->findSym(_sym);
    if (!sym)
        throw new Error("dictnf", _sym, "No such symbol.");
}

// construct from constructor args
// expects a 2Tuple or List of form (builtin name, file-name)
Builtin::Builtin(const Slot &arg)
{
#ifdef notdef
    // check task permissions 
    if (!currTask()->isSys()) {
        throw new Error("illegal", arg, "Constructing Builtin is privileged");
    }
#endif
    
    // locate the elf descriptor for this name
    const char *elf_name = (int)(arg->length()) == 2
                       ?(char*)(arg[1])
                       : COLDSTORE;
    elf = Elf::findElf(elf_name);
    if (!elf)
        throw new Error("dictnf", elf_name, "Can't find .so file");

    // locate the symbol for this name
    sym = elf->findSym(arg[0]);
}

// constructor args to recreate object
Slot Builtin::toconstruct() const
{
    if (!strcmp(fname(), COLDSTORE)) {
        return new String(name());
    } else {
        return new (2) Tuple(-2, new String(sym->symname()),
                             new String(elf->name()));
    }
}

Builtin::~Builtin()
{}

//////////////////////////////////////////////////
// Builtin static construction

//////////////////////////////////////////////////
// Builtin accessors

const char *Builtin::name() const {
    return sym->symname();
}

void *Builtin::address() const {
    return sym->address();
}

const char *Builtin::fname() const {
    return elf->name();
}

const char *Builtin::rname() const {
   return sym->demangle();
}

const char *Builtin::rname_full() const {
   return sym->demangle_full();
}

bool Builtin::isLoaded() const {
    return sym->isLoaded();
}

//////////////////////////////////////////////////
// Builtin feature extractors

///////////////////////////////
// Coldmud Interface

// structural

// Identity functions
Data *Builtin::clone(void *store) const
{
    return (Data*)this;
}

Data *Builtin::mutate(void *where) const
{
    return (Data*)this;
}

// Is the Builtin loaded?
bool Builtin::truth() const
{
    // handle will be non-0 if Builtin's been loaded.
    return sym->isLoaded();
}

// Order by the string order of the file and builtin names
int Builtin::order(const Slot &arg) const
{
    int cmp = typeOrder(arg);
    if (cmp)
        return cmp;
    
    Builtin *other = (Builtin*)arg;

    cmp = strcmp(fname(), other->fname());
    if (!cmp)
        cmp = strcmp(name(), other->name());
    return cmp;
}

// sequence

// get C++ symbol as Builtin
Slot Builtin::slice(const Slot &from)
{
    // give access to builtin names
    int val = (int)from;
    switch(val) {
    case -1:
        return name();
    case -2:
        return fname();
    case -3:
        return isLoaded()?1:0;
    case -4:
        return sym->len();
    case -5: {
        switch (sym->type()) {
        case Csym::FUNCTION:
            return "function";
        case Csym::DATA:
            return "data";
        case Csym::VTBL:
            return "vtbl";
        case Csym::UNKNOWN:
            return "unknown";
        default:
            return "impossible";
        }
    }
    case -6:
        return (int)address();

    case -7:
      return rname();

    case -8:
      return rname_full();

    default:
	return String::slice(val);
        //throw new Error("range", from, "Builtin has no such entry");
    }
}

const void *Builtin::vtbl() const {
    try {
        return sym->vtype();
    } catch (char *err) {
        throw new Error("builtin", this, err);
    }
}

/////////////////////////////////////
// functional

// call Builtin new
Slot Builtin::new_call(const int nargs, ...)
{
    // GCC 2.96 has a parser bug, and doesn't like pointer types in va_args
    typedef Data* tmpdataptr;
    va_list args;
    tmpdataptr datum;

    //cerr << "Builtin::new_call(int,...): " << nargs << "\n";
    if (!sym)
        throw new Error("builtin", this, "no associated Csym");
    if (sym->type() != Csym::FUNCTION)
        throw new Error("builtin", this, "Csym not FUNCTION type");

    try {
        va_start(args, nargs);
        for (int size = nargs; size > 0; size--) {
            datum = va_arg(args, tmpdataptr);
            __asm__ __volatile__ ("pushl (%0)"
                                  : : "r" (&datum) : "cc", "memory");
        }
        va_end(args);
        __asm__ __volatile__ ("pushl (%0)"
                              : : "r" (&nargs) : "cc", "memory");
        __asm__ __volatile__ ("pushl (%0)"
                              : : "r" (&sym) : "cc", "memory");
        
        __asm__ __volatile__ ("call %0"
                              : : "r" (Csym::callit) : "cc", "memory");
        __asm__ __volatile__ ("movl %%eax,%0"
                              : "=r" (datum) :
                              : "eax", "cc", "memory");
        return datum;
        //return (Data*)(sym->call(nargs));
    } catch (char *err) {
        throw new Error("builtin", this, err);
    }
}

// call Builtin
Slot Builtin::call(Slot &args)
{
    if (!sym)
        throw new Error("builtin", this, "no associated Csym");
    //if (sym->type() != Csym::FUNCTION)
    //    throw new Error("builtin", this, "Csym not FUNCTION type");
    void *(*fn)(...) = (void *(*)(...))(sym->functional());
    void **datum = (void **)(((List*)args)->content());
    return (Data*)(sym->caller(fn, datum, args->length()));

#if 0
    try {
        int nargs = args->length();
        //cerr << "Builtin::call(Slot&): " << nargs << " (" << args.Dump(cerr) << ")\n";

        for (int size = nargs; size > 0; size--) {
            Slot datum = args->slice(size - 1);
            __asm__ __volatile__ ("pushl (%0)"
                                  : : "r" (&datum) : "cc", "memory");
        }
        __asm__ __volatile__ ("pushl (%0)"
                              : : "r" (&nargs) : "cc", "memory");
        __asm__ __volatile__ ("pushl (%0)"
                              : : "r" (&sym) : "cc", "memory");
        
        __asm__ __volatile__ ("call %0"
                              : : "r" (Csym::callit) : "cc", "memory");
        //return (Data*)(sym->call(nargs));
    } catch (char *err) {
        throw new Error("builtin", this, err);
    }
#endif
}
Slot caller(void *(*what)(...), void *args, int count);

Slot Builtin::mcall(Slot that, Slot &args)
{
    if (!sym)
        throw new Error("builtin", this, "no associated Csym");

    Data *datum;
    int len = args->length();
    for (int size = len; size > 0; size--) {
      datum = args[size-1];
      __asm__ __volatile__ ("pushl (%0)"
			    : : "r" (&datum) : "cc", "memory");
    }
    __asm__ __volatile__ ("pushl (%0)"
			  : : "r" (&that) : "cc", "memory");    
    len++;
    __asm__ __volatile__ ("pushl (%0)"
			  : : "r" (&len) : "cc", "memory");
    __asm__ __volatile__ ("pushl (%0)"
			  : : "r" (&sym) : "cc", "memory");
        
    __asm__ __volatile__ ("call %0"
			  : : "r" (Csym::callit) : "cc", "memory");
    __asm__ __volatile__ ("movl %%eax,%0"
			  : "=r" (datum) :
			  : "eax", "cc", "memory");
    return datum;
}

void Builtin::mvcall(Slot that, Slot &args)
{
    if (!sym)
        throw new Error("builtin", this, "no associated Csym");

    Data *datum;
    int len = args->length();
    for (int size = len; size > 0; size--) {
      datum = args[size-1];
      __asm__ __volatile__ ("pushl (%0)"
			    : : "r" (&datum) : "cc", "memory");
    }
    __asm__ __volatile__ ("pushl (%0)"
			  : : "r" (&that) : "cc", "memory");    
    len++;
    __asm__ __volatile__ ("pushl (%0)"
			  : : "r" (&len) : "cc", "memory");
    __asm__ __volatile__ ("pushl (%0)"
			  : : "r" (&sym) : "cc", "memory");
        
    __asm__ __volatile__ ("call %0"
			  : : "r" (Csym::callit) : "cc", "memory");
    __asm__ __volatile__ ("movl %%eax,%0"
			  : "=r" (datum) :
			  : "eax", "cc", "memory");
}

ostream &Builtin::dump(ostream& out) const
{
    char *ty;
    out << typeId() << ':';
    String::dump(out);

    switch (sym->type()) {
    case Csym::FUNCTION:
        ty = "FUNCTION"; break;
    case Csym::DATA:
        ty = "DATA"; break;
    case Csym::VTBL:
        ty = "VTBL"; break;
    case Csym::UNKNOWN:
        ty = "UNKNOWN"; break;
    default:
        ty = "ERRONEOUS"; break;
    }

    out << " = " << sym->symname()
        << " type: " << ty;

    return out;
}

