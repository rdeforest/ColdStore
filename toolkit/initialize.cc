// initialize.cc - initalizes the prototypes defined in the toolit
// Copyright (C) 2000,2001 Ryan Daum 
// 
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 

#include <elf.hh>
#include <Data.hh>
#include <Slot.hh>
#include <List.hh>
#include <Frame.hh>
#include <Object.hh>
#include <Store.hh>
#include <Error.hh>
#include <Closure.hh>

#include "common.hh"

extern Slot defineRoot( Frame *context);
extern Slot defineExtraSequenceMethods(Frame *context);
extern Slot defineExtraStringMethods(Frame *context);

void initElf(Elf *elf)
{
  try {
    Frame *context = (Frame*)0;

    Slot root = defineRoot( context );
    Slot toolkit = root.create("toolkit");

    // adds "join", "englishJoin", among others
    toolkit.addExtraSequenceMethods = &defineExtraSequenceMethods;
    toolkit.addExtraSequenceMethods();

    // adds "explode" and "explodeWords"
    toolkit.addExtraStringMethods = &defineExtraStringMethods;
    toolkit.addExtraStringMethods();
    
  } catch (Error *e) {
    e->upcount();
    cerr << "Error " <<  ((Symbol*)(Data*)e)->name() << "\n";
    e->dncount();
  } catch (Frame *e) {
    e->upcount();
    cerr << (char*)e->traceback() << endl;
    e->dncount();
  }
}
