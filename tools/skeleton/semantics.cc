// semantics of Chaos
// Copyright (C) 1998,1999 Colin McCormack, Jordan B. Baker
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

extern void bkpt();
static char *id __attribute__((unused))="$Id: semantics.cc,v 1.1 2001/06/20 02:06:00 amphibian Exp $";
// #define DEBUGLOG 1
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <Data.hh>
#include <Store.hh>
#include <Tuple.hh>
#include <Segment.hh>
#include <String.hh>
#include <List.hh>
#include <Dict.hh>
#include <Hash.hh>
#include <Tree.hh>
#include <Symbol.hh>
#include <Error.hh>
#include <Builtin.hh>
#include <BigInt.hh>
#include <Real.hh>
#include <Integer.hh>
#include <Regexp.hh>
#include <NamedNamespace.hh>
#include <File.hh>
#include <VectorSet.hh>

#include <elf.hh>
// #include <assert.h>
#include "semantics.hh"

Data* Skeleton::clone(void* where) const
{
  throw new Error("skeleton",this,"can't clone a skeleton");
  return (Data*)NULL;
};

int Skeleton::order(const Slot& arg) const
{
  return typeOrder(arg);
};

/** called when Elf has loaded this module
 */
void initElf(Elf *elf)
{
    //cerr << "chaos's initElf called\n";

    Skeleton *skeleton;
    Slot sym;

    // register our Builtins with ColdStore
#if 0
    try {
        RegisterElf(elf);
    } catch (Error *e) {
        e->dump(cerr) << '\n';
        throw e;
    }
#endif

    sym = store->Names->search("Skeleton");
    if (sym) {
        skeleton = (Skeleton*)(Data*)((Symbol *)sym)->value();
        cerr << "resuming skeleton\n";
        DEBLOG(cerr << "Skeleton " << skeleton->typeId()
               << ' ' << skeleton
               << '\n');
        //skeleton->reset();
    } else {
        skeleton = new Skeleton();
        cerr << "new skeleton\n";
        DEBLOG(cerr << "Skeleton " << skeleton->typeId()
               << ' ' << skeleton
               << '\n');
        store->Names->insert("Chaos", skeleton);
        //store->Names->insert("PS1", "> ");
    }

    //lexer->dump(cerr) << '\n';
    //Memory::Assert();

}

