// Startup - Server Initialization
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Store.cc,v 1.13 2002/04/08 02:37:31 coldstore Exp $";

#include <iostream.h>
#include <assert.h>
#include <fcntl.h>

#include <elf.hh>
#include "Data.hh"
#include "Store.hh"

#include "Vector.hh"
#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Builtin.hh"
#include "Dict.hh"
#include "Hash.hh"
#include "Symbol.hh"
#include "List.hh"
#include "Integer.hh"
#include "BigInt.hh"
#include "Set.hh"
#include "OrderedSet.hh"
#include "VectorSet.hh"

extern void shutdown();	// the default shutdown function

/** Create a bootstrap symbol table of Builtins for this Elf
 */
static Builtin **traverse(Elf *elf, Csym *sym, int cnt = 0)
{
    Builtin **table = NULL;
    if (sym != elf->symQ()) {
        table = traverse(elf, (Csym*)(sym->next()), cnt + 1);
        table[cnt] = new Builtin(sym);
        table[cnt]->upcount();

        DEBLOG(cerr << cnt
               << " traverse " << sym->symname()
               << " at: " << sym
               << " --> " << table[cnt]
               << '\n');
    } else {
        // this is the last in Q.
        table = (Builtin**)malloc(sizeof(Builtin*) * (1 + cnt));
        table[cnt] = 0;
        table[cnt-1] = new Builtin(sym);
        table[cnt-1]->upcount();
    }
    return table;
}

/** register any new Builtins in the bootstrap symbol table.
 */
static Set *registerElf(Builtin **table, Set *ns)
{
  for (Builtin **bpp = table; *bpp; bpp++) {
    try {
      //cerr << "builtin: " << (*bpp)->name() << '\n';
      ns->insert(*bpp);
    } catch (Error *e) {
      // Note: we can't replace existing registered Builtins.
      // since Csym can't be destroyed, any Builtin (once created)
      // will reference an existing Csym, and so is valid forever
      
      // Only way to reclaim a registered Builtin is by rebuilding
      // the store
    }
  }
  return ns;
}

/** called to register Csyms as Builtins for a foreign module
 */
Set *RegisterElf(Elf *elf)
{
    Set *ns = store->Registered;
    cerr << "RegisterElf " << elf << "\n";
    if (!elf->symQ()) {
        DEBLOG(cerr << elf << "empty\n");
        return NULL;
    }
    Builtin **table = traverse(elf, (Csym*)(elf->symQ()->next()));
    ns = registerElf(table, ns);
    free(table);
    //cerr << "RegisterElf " << elf << " done\n";
    return ns;
}

/** a utility to dump a Csym
 */
void dump_csym(Elf *elf, Csym *sym)
{
    cerr << "dump " << sym->symname()
         << "at: " << sym
         << " next: " << sym->next()
         << " in: " << sym->in
         << " elf: " << elf
         << " symQ " << sym->in->symQ()
         << '\n';
}

#if 0
/** find the Builtin/VTBL implementing this class in a bootstrap symbol table
 */
static Builtin *findV(Builtin **table, char *class_name, bool chuck=true)
{
    char name[256];
    sprintf(name, "__vt_%d%s", strlen(class_name), class_name);
    for (Builtin **bpp = table; *bpp; bpp++) {
        DEBLOG(cerr << "findV " << name << ", " << **bpp << '\n');
        if (!strcmp(name, (char *)**bpp)) {
            (*bpp)->upcount();	// maintain this Builtin if only for Store
            DEBLOG(cerr << (*bpp)->dump(cerr) << " FOUND\n");
            return *bpp;
        }
    }
    if (chuck) {
        sprintf(name, "Can't find vtbl for %s", class_name);
        throw name;
    }
    return NULL;
}
#endif

// initialize Errors namespace with the errors we throw
// note: this can be overriden in the executable,
// just make sure it's got at least these and ends in NULL
char *ColdErrors[] = {
    "dictnf",
    "type",
    "args",
    "break",
    "builtin",
    "conformant",
    "exhausted",
    "illegal",
    "nil",
    "parse",
    "range",
    "symbol",
    "unimplemented",
    "value",
    "underflow",
    NULL
};

// init - initialize subsystems
Store::Store()
{
    // perform once-only initializations

    // Errors namespace: the errors coldstore throws
    Errors = new Namespace();
    Errors->upcount();		// Must protect Namespace

    // Registered Builtin namespace (aka symbol table)
    Registered = new VectorSet();
    Registered->upcount();	// Must protect Namespace store-Registered

    // Coldstore global Namespace
    Names = new Namespace();
    Names->upcount();		// Must protect Namespace

    // Coldstore type Namespace
    Types = new Namespace();
    Types->upcount();		// Must protect Namespace

    initialized = true;
}

void cold_process()
{
    cerr << "Called ColdStore default process\n";
}

/** called when Elf has loaded this module
 */
void initElf(Elf *elf)
{
#ifdef DEBUGLOG
    cerr << "Store's initElf called\n";
    elf->forallSym(dump_csym);
#endif
    try {

        // create type mapping to Csym::VTBL syms
        if (!store) {
            if (!elf->symQ()) {
                cerr << elf << "empty\n";
                return;
            }
            store = new Store();
            Builtin **table = traverse(elf, (Csym*)(elf->symQ()->next()));
            assert((store != NULL) && "NULL Store - can't proceed");

            // initialize Errors namespace with the errors we throw
            for (char **errs = ColdErrors; *errs; errs++) {
                //const Slot err(new String(*errs));
                String *err = new String(*errs);
                store->Errors->insert(err, err); //mkSym
            }
#ifdef DEBUGLOG
            store->Errors->dump(cerr);
#endif
	    
            // initialize Types namespace with the errors we throw
            while (DopeV::all && DopeV::all->More()) {
 	tSlot<DopeV> dv =DopeV::all->Next(); 
                store->Types->insert(dv->name, dv); //mkSym
            }
            DEBLOG(store->Types->dump(cerr) << " store Type namespace\n");

            // Register all the Builtins
            //store->Registered = registerElf(table, store->Registered);
	    free(table);
        } else {
            // Register any new Builtins
            RegisterElf(elf);
        }
    } catch (Error *e) {
        e->dump(cerr) << '\n';
    }

    // replace gmp's allocation functions
    mp_set_memory_functions(BigInt::GmpAlloc,
                            BigInt::GmpReAlloc,
                            BigInt::GmpDeAlloc);
#ifdef DEBUGLOG
    cerr << "ColdStore initElf() complete\n";
#endif
    return;

    try {
        //cerr << init() << '\n';
        cold_process();
    }
    catch (Error *e) {
        cerr << "\nUncaught Error: ";
        ((Data*)e)->dump(cerr);
        cerr << '\n';
    }

    shutdown();
}

Data *Store::clone(void *where = (void*)0) const
{
    throw new Error("unimpl", this, "clone not implemented");
}

int Store::order(const Slot &d) const
{
    throw new Error("unimpl", this, "order not implemented");
}

//Store * __attribute__ ((weak)) store = NULL;
