// start.cc: start the chatter
// Copyright (C) 200!, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <elf.hh>
#include <Data.hh>
#include <Slot.hh>
#include <Tuple.hh>
#include <Store.hh>
#include <Error.hh>
#include <String.hh>
#include <Tape.hh>
#include <List.hh>
#include <Symbol.hh>
#include <Message.hh>
#include <Frame.hh>
#include <Object.hh>
#include <Binding.hh>
#include <String.hh>

#include <string>

#include "Parser.hh"


void initElf(Elf *elf)
{
  //  Parser *p = new Parser();
  Scanner *s = new Scanner("^ #( 1. 2. 3. 4. \"5 4 3 2 \\n \". ).");

  try {
    while (s->More()) {
      cerr << s->Next() << endl;

    }
    //    cerr << p->parse(new Object(), "^ #( 1. \x1. 2. 3. 4. \"5 4 3 2 \\n \". ).");
    
  } catch (Error *e) {
    e->upcount();
    e->dump(cerr) << '\n';
    e->dncount();
  }
} 
