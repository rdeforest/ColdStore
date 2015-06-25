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

#include "freonLexer.hh"
#include "freonParser.hh"

void initElf(Elf *elf)
{
  Slot testList = "return [ 1, \"string\", 2, $name, 'noggin, 3 ];";
  Slot testDict = "return #[ 1:2, \"string\": 4 ];";
  Slot testBinary = "return 5 == 4;";
  Slot testParen = "return ((3 * 2 + 3) - (1 + 2 / 4)) / 4;";
  Slot testConditional = "return (1 + 2) ? 2 | 3;";
  Slot testIf = "if (1) return 2;";
  Slot testIfElse = "if (1) return $bob; else return 3;";
  Slot testWhile = "while (1) return 5;";
  Slot testTry = "try { return 5; } catch return 5;";
  Slot testArg = "arg a; return a;";
  Slot testVar = "var a; a = 4; return a;";
  Slot testComment = "// test this line\nreturn 4;";
  Slot testFor = "var x; for x in [1, 2, 3, 4] { return x; }";
  Slot testMessage = "return $bob.create(5).open().close();";

  freonParser *p = new freonParser();
  try {
    //     cerr << p->parse( new Object(), testList ) << endl;
//     cerr << p->parse( new Object(), testDict ) << endl;
//     cerr << p->parse( new Object(), testBinary ) << endl;
//     cerr << p->parse( new Object(), testConditional ) << endl;
//    cerr << p->parse( new Object(), testIf ) << endl;
//    cerr << p->parse( new Object(), testIfElse ) << endl;
//    cerr << p->parse( new Object(), testWhile ) << endl;
//    cerr << p->parse( new Object(), testTry ) << endl;
//    cerr << p->parse( new Object(), testFor ) << endl;  
//    cerr << p->parse( new Object(), testArg ) << endl;  
//    cerr << p->parse( new Object(), testComment ) << endl;  
//    cerr << p->parse( new Object(), testMessage ) << endl;  
//    cerr << p->parse( new Object(), testParen ) << endl;  
    Frame *context = (Frame*)0;
    Slot x = new Object();
    Slot y = p->parse( x, "var a; a = 5; if (1) {return a;}") ;
    cerr << y << endl;
    x->insert("y", y);
    cerr << x.y() << endl;


    
  } catch (Error *e) {
    e->upcount();
    e->dump(cerr) << '\n';
    e->dncount();
  }
} 
