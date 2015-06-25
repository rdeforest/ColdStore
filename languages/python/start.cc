// start.cc: invoke the python interpreter
// Copyright (C) 2001, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

#include <assert.h>
#include <stdio.h>
extern "C" 
{
    
#include <Python.h>
};


#include <Data.hh>
#include <Slot.hh>
#include <List.hh>
#include <elf.hh>
#include <Tuple.hh>
#include <Error.hh>
#include <Node.hh>
#include <Symbol.hh>
#include <Tuple.hh>
#include <Store.hh>
#include <Dict.hh>
#include <String.hh>

void prePyBrk ()
{}

void _cleanup();

void initElf(Elf *elf)
{
  Slot sym = store->Names->search("Python");
  
  if (sym) {
    cerr << "resuming python session\n"; 
  } else {
    Slot pythonDict = new Dict();
    cerr << "new python session\n";
    store->Names->insert("Python", pythonDict);
  }
  try {
    prePyBrk();
    PyRun_InteractiveLoop(stdin, "cold-session"); 
    

  } catch (Error *e) {
      e->upcount();
      cerr << "Error: " << e << "\n\n";
      e->dncount();
  }
}
