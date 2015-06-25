// DopeV - Coldstore metadata
// Copyright (C) 2002 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: DopeV.cc,v 1.2 2002/04/13 03:53:47 coldstore Exp $	

//#define DEBUGLOG
#include "DopeV.hh"
#include "Store.hh"
#include "Symbol.hh"
#include "Error.hh"
#include "List.hh"

#include "tSlot.th"
template union tSlot<DopeV>;

_DopeV::_DopeV(char *n, void *v, size_t vs, size_t cs)
    : inited(inited?inited:0)
{
    // only create a real DopeV if it doesn't already exist
    if (!inited) {
	DEBLOG(cerr << "_DopeV: " << (void*)this
	       << ' ' << n
	       << '\n');
	inited++;
	new DopeV(n,v,vs,cs);
    }
}

DopeV::DopeV(char *n, void *v, size_t vs, size_t cs)
    :name(n),
     vptr(v),
     vptr_size(vs),
     class_size(cs)
{
    DEBLOG(cerr << "DopeV: " << (void*)this
	 << ' ' << name
	 << '\n');

    DopeV *reflect = (((DopeV*)vptr)-1);
    reflect = this;

    if (store) {
	DEBLOG(cerr << "prestore DopeV " << this
	     << " rc: " << refcount()
	     <<"\n");
	store->Types->insert(name, this);
	DEBLOG(cerr << "poststore DopeV " << this
	     << " rc: " << refcount()
	     <<"\n");
    } else {
	if (!all) {
	    all = new List();
	}
	all = all->insert(this);
    }
}


Data *DopeV::clone(void *where = (void*)0) const
{
    throw new Error("unimpl", this, "clone not implemented");
}

int DopeV::order(const Slot &d) const
{
    if(!AKO(d,DopeV)) {
	int cmp = typeOrder(d);
	if (cmp)
	    return cmp;
    }
    tSlot<DopeV> that = (Slot &)d;
    return strcmp(name, that->name);
}

ostream &DopeV::dump(ostream& out) const
{
    out << "{Dope Vector " << (void*)this
	<< ' ' << name
	<< "}";
    return out;
}
