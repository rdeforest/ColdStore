// start.cc: start the browser
// Copyright (C) 200!, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <elf.hh>
#include <Data.hh>
#include <Slot.hh>
#include <Tuple.hh>
#include <Symbol.hh>
#include <Error.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <Object.hh>


#include <gnome--/main.h>

#include "coldBrowser.hh"

void pre_eval_bkpt() {}

void yac_break()
{}

void initElf(Elf *elf)
{

  try {
    Frame *context = (Frame*)0;
    
    Slot names = store->Names;
    Slot freon_compiler = names.freon_compiler;
    Slot method = new List();
    method->insert("var a;");
    method->insert("a=1;");
    method->insert("while (a<1000)");
    method->insert(" a=a+1;");
    method->insert("return a;");

    Slot test_object = new Object();
    test_object.id = "test_object";
    names.test_object=test_object;   
    
    Slot meth = freon_compiler.compile( method, test_object, "test_method");
    test_object.test_method = meth; 
    test_object.bob = (Data*)NULL;
    
    Slot x = new Namespace();
    x.test = 5;
    Slot sym = x->slice("test");
    x->del(sym);
    
    int argc = 1;
    char *argv[1];
    argv[0] = "test";
    char **cargv = (char**)argv;

    Gnome::Main m("coldBrowser", "0.1", argc, cargv);

    Slot objects = new List();
    Slot it = names->iterator();
    while (it->More())
      objects->insert(it->Next()[2]);


    manage(new class coldBrowser( objects ));
    m.run();


  } catch (Error *e) {
    e->upcount();
    cerr << "Error " << e << "\n";
    e->dncount();
  }
}
