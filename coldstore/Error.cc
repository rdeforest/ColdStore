// Error - ColdStore interface to Error
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Error.cc,v 1.8 2002/02/27 01:49:31 coldstore Exp $";

#include "Data.hh"
#include "Store.hh"

#include "Tuple.hh"
#include "String.hh"
#include "Dict.hh"
#include "Symbol.hh"
#include "Error.hh"


#include "tSlot.th"
template union tSlot<Error>;

Error::Error(const Error *err)
  : Tuple(3),
    _err(err->_err),
    _value(err->_value),
    _explanation(err->_explanation)
{
}

Error::Error(const Slot &err)
  : Tuple(3),
    _err(err[0]),
    _value(err[1]),
    _explanation(err[2])
{
}

Error::~Error()
{
}

extern void onError(const char *type = NULL);

Error::Error(const char *err, const Slot value, Slot expl)
    : Tuple(3),	// Overlay the Tuple
      //_err(store->Errors->mkSym(err)),
      _value(value),
      _explanation(expl)
{
    const char *errtype = value->typeId();	// useful for type error debugging
    onError(errtype);	// breakpoint opportunity
    _err = store->Errors->insert(err, this); //mkSym
}

// structural

// Uses the copy constructor
Data *Error::clone(void *where) const
{
    return new (where) Error(this);
}

// returns a mutable copy of this
Data *Error::mutate(void *where) const
{
    return clone(where);
}

// object's truth value
bool Error::truth() const
{
    return false;
}

static void recursive_error ()
{
}

ostream &Error::dump(ostream& out) const
{
    try {
        char* sym;
        if(!_err)
	  {
	    sym = "[null]";
	  }
        else
	  {
	    sym = ((Symbol*)_err)->name();
	  }
        out << '~' << sym << " {";
        _value.Dump(out);
        out << "} ";
        if ((bool)_explanation) {
            _explanation->dump(out);
#ifdef notdef
            char *expl = _explanation;
            out << expl;
#endif
        }
    } catch (Error *e) {
        out << "recursive error: ";
        recursive_error();
    }

    return out;
}
