// semantics of Chaos
// Copyright (C) 1998,1999 Colin McCormack, Jordan B. Baker
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms
// 	$Id

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream.h>
#include <strstream.h>
#include <typeinfo>
#include <dlfcn.h>
#include <stdlib.h>

#include <Data.hh>
#include <Store.hh>
#include <Tuple.hh>
#include <Segment.hh>
#include <String.hh>
#include <List.hh>
#include <Dict.hh>
#include <Symbol.hh>
#include <Error.hh>
#include <pcre.h>

cold class Chaos;

// See
// http://www.cerfnet.com/~mpcline/C++-FAQs-Lite/pointers-to-members.html#[30.5]
// for an explination of this define

typedef void (Chaos::*ChaosPrimitive)(void);

struct PrimitiveEntry {
    char *name;
    char *builtin_name;
};

//metaclass ColdClass Chaos;
cold class Chaos
  : public Stack
{
protected:
  // object / callstack variables
  tSlot<Stack> scopes;         // TOP is current scope, UNDER is outer, etc.
  tSlot<Namespace> base_scope; // TOP is current scope, UNDER is outer, etc.
  tSlot<Stack> stacks;         // TOP is previous this, UNDER is before that, etc
  tSlot<Chaos> parent;
  tSlot<String> buffer;	// unparsed input
  tSlot<Stack> rstack;	// return stack

protected:
  static PrimitiveEntry primitives[];

  // regexps used in parsing
  static pcre *ws;
  static pcre *number;
  static pcre *token;

public:
    /** construct Chaos from Slot args
     * @param args - TODO
     */
    Chaos();
    Chaos(Slot const &args);
    Chaos(Slot string, Chaos *parent);
    void ChExit();
    virtual ~Chaos();

    void check(int) const;

    /** connection indication/reuest
     * @param to indication's source
     * @return 
     */
    virtual Slot connect(const Slot to);

    /** diconnection indication/request
     * @param from indication's source
     * @return 
     */
    virtual Slot disconnect(const Slot from);
    
    /** incoming available indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot incoming(const Slot from);

    /** ready-for-outgoing indication/request
     * @param from indication's source
     * @return where to send future indications - NULL to disable
     */
    virtual Slot outgoing(const Slot to);

    /* Chaos is mutable */
    virtual Data *mutate(void *store = (void*)0) const;

    virtual ostream &dump(ostream&) const;

    /** call Chaos
     */
    virtual Slot call(Slot &args);

    /** no_error - this is pushed by ChCatch to indicate no error
     */
    static tSlot<Error> no_error;

    void ChPush(Slot el);
    Slot ChPop();

    void ChPrint();
    void ChEcho();

    tSlot<Symbol> Lookup(const char *var);
    void ChLookup();
    void ChVar(const char *);

    void ChChr();        // ASCII number -> char  (65 -> "A")
    void ChOrd();        // ASCII char -> number  ("A"-> 65 )
    void ChFetch();
    void ChScopes();     // push namespace stack onto regular stack
    void ChPushScope();  // push namespace onto namespace stack
    void ChPopScope();   // pop namespace off namespace stack
    void ChPushStack();  // push current stack onto "stack stack"
    void ChPopStack();   // pop previous stack from "stack stack" and
                         // push current stack onto that as a list
    void ChRPush();	// push top onto ReturnStack
    void ChRPop();	// pop ReturnStack to stack
    void ChThat();	// push whole ReturnStack

    void ChThis();       // push a pointer to ourselves
    void ChSpawn();      // push a child of this
    void ChEval();       // convert TOP to code and execute it
    void ChExec();       // convert TOP to code and execute it
    void ChCatch();      // convert TOP to code and execute it, pushing an error instead of dumping it.
    void ChThrow();
    void ChInvoke();     // invoke a builtin

    void ChPrecision();
    void ChDup();
    void ChDrop();
    void ChSwap();
    void ChView();
    void ChDump();
    void ChNames();
    void ChBuiltins();
    void ChIf();
    void ChWhile();
    void ChSplice();

    void ChNew();       // Bind a new builtin type
    void ChNew2();       // Bind a new builtin type from a specific .so file
    void ChPushNull();
    void ChSet();
    void ChNamedNamespace();
    void ChTrue();
    void ChSum();
    void ChSub();
    void ChMult();
    void ChDiv();
    void ChOrder();
    void ChMod();
    void ChSetVar();
    void ChPositive();
    void ChNegative();
    void ChInvert();
    void ChAnd();
    void ChXor();
    void ChOr();
    void ChLShift();
    void ChRShift();
    void ChType();
    void ChTypes();
    void ChSequence();
    void ChIsSequence();
    void ChLength();
    void ChConcat();
    void ChSlice();
    void ChElement();
    void ChSearch();
    void ChRReplace();
    void ChReplace();
    void ChSReplace();
    void ChInsert();
    void ChMInsert();
    void ChRDel();
    void ChDel();
    void ChIterator();
    void ChIsIterator();
    void ChMore();
    void ChNext();
    void ChCall();
    void ChMCall();
    void ChOpEq();
    void ChOpNEq();
    void ChOpGEq();
    void ChOpGT();
    void ChOpLEq();
    void ChOpLT();
    void ChParse();
    void ChShutDown();

    Slot parse(Slot);
    void invoke(tSlot<Builtin>);
    void init_parse();
    void bootstrap();

    void ChGetTimeOfDay();
};

