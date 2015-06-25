// Parse.cc: main code loop
// Copyright (C) 2000, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

// $Id: Parse.cc,v 1.1 2000/12/02 16:54:53 opiate Exp $  
// $Log: Parse.cc,v $
// Revision 1.1  2000/12/02 16:54:53  opiate
// Moved old stack-based system to "stack-experiment".
//
// Revision 1.25  2000/11/12 04:01:52  coldstore
// Node tree Reduction - works for simple while-test
//
// Revision 1.24  2000/11/09 17:38:53  opiate
// Changed Frame::prepareStack to be Node::toPostfix
// Changed Node::unfold to Node::isTerminal
// Changed whileNode::eval to actually work
// Changed interpretStack, ifNode, etc. to use toPostfix
//
// Revision 1.23  2000/11/09 05:56:36  opiate
// String::add copied from List::add
// Frame::prepareStack and addStack (nonreversing)
// broken freonNodes::whileNode
//
// Revision 1.22  2000/11/08 22:06:19  opiate
// Added CVS log and version entries.
// 

#include <assert.h>
#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <Data.hh>
#include <Slot.hh>
#include <List.hh>
#include <elf.hh>
#include <Tuple.hh>
#include <Error.hh>
#include <Node.hh>
#include <Symbol.hh>
#include <Tuple.hh>
#include <Frame.hh>
#include <String.hh>

#include "freonLexer.hh"

void pre_eval_bkpt() {}

Frame *interpretStack(Frame *frame) {

  frame->_cookie = 0;
  while (frame) {
    Slot node;
    frame->_opStack->pop(node);

    //    for (int i=0; i<(int)frame->_cookie; i++)
    //  cout << " ";
    // cout << "Popped: " << node->typeId() << "\n";

    frame = node->eval(frame);
    
    if ( (!frame->_opStack->length() || (!frame->_status) ))
      frame = frame->_caller;  
  }

  return frame;
}

Slot read_it() {
  // variables for line reading
  
  Slot method_code = new List(); 
  char *str;
  Slot ret;
  
  while (1) {
    // input the line
    if (!(str = readline("")))
      break;    
    
    add_history ( str ); 
    ((List*)(Data*)method_code)->push( str );
  }


  freonLexer lexer;
  lexer.start(method_code);

  lexer.yyparse();

  Node *top = ((Node*)(Data*)lexer.prog);
  cout << '\n' << top->toLisp() << '\n';

  Slot SlotF = new Frame();
  Frame *frame = dynamic_cast<Frame*>((Data*)SlotF);
  frame->_arguments = new Stack();
  frame->_arguments->push(1);
  frame->_arguments->push(2);
  frame->_arguments->push(3);
  frame->_namespace = new Namespace();
  frame->_valStack = new Stack();
  frame->_opStack = new Stack();
  frame->_status = 1;

  // top frame, make sure its caller is set to 0
  frame->_caller = 0;

  frame = dynamic_cast<Frame*>((Data*)SlotF);

  frame->_opStack = new Stack( ((Node*)(Data*)lexer.prog)->toPostfix() );

  frame = interpretStack(frame);

  //->_valStack->dump(cout) << "\n";

  //  cout << "CALLING FRAME\n";
  //  cout << frame->call("test",2,3,4);
  //  cout << '\n';
}

void initNodes();

void initElf(Elf *elf)
{
  /*  try {
    RegisterElf(elf);
  } catch (Error *e) {
    e->dump(cerr) << '\n';
    throw e;
  }*/
  try {
      initNodes();
    read_it();
    
   } catch (Error *e) {
    cout << "Error " ;
    e->dump(cout) << "\n\n";
  }
}
