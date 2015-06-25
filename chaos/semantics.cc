// semantics of Chaos
// Copyright (C) 1998,1999,2000,2001,2002:
//   Colin McCormack, 
//   Jordan B. Baker, 
//   Robert de Forest
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

extern void bkpt();
static char *id __attribute__((unused))="$Id: semantics.cc,v 1.99 2002/05/09 13:38:38 coldstore Exp $";

//#define DEBUGLOG 1
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <qvmm.h>
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
#include <Null.hh>
#include <Connection.hh>
#include <tSlot.hh>

#include <elf.hh>
// #include <assert.h>
#include "semantics.hh"

#include <tSlot.th>
template union tSlot<Chaos>;

#define callPrimitive(object,ptrToPrimitive)  ((object).*(ptrToPrimitive))

PrimitiveEntry Chaos::primitives[] = {
    {"positive",            "ChPositive__5Chaos"},
    {"negative",            "ChNegative__5Chaos"},
    {"add",                      "ChSum__5Chaos"},
    {"+",                        "ChSum__5Chaos"},
    {"subtract",                 "ChSub__5Chaos"},
    {"-",                        "ChSub__5Chaos"},
    {"multiply",                "ChMult__5Chaos"},
    {"*",                       "ChMult__5Chaos"},
    {"div",                      "ChDiv__5Chaos"},
    {"/",                        "ChDiv__5Chaos"},
    {"mod",                      "ChMod__5Chaos"},
    {"%",                        "ChMod__5Chaos"},
    {"invert",                "ChInvert__5Chaos"},
    {"~",                     "ChInvert__5Chaos"},
    {"and",                      "ChAnd__5Chaos"},
    {"&",                        "ChAnd__5Chaos"},
    {"xor",                      "ChXor__5Chaos"},
    {"^",                        "ChXor__5Chaos"},
    {"or",                        "ChOr__5Chaos"},
    {"|",                         "ChOr__5Chaos"},
    {"lshift",                "ChLShift__5Chaos"},
    {"<<",                    "ChLShift__5Chaos"},
    {"rshift",                "ChRShift__5Chaos"},
    {">>",                    "ChRShift__5Chaos"},
    {"type",                    "ChType__5Chaos"},
    {"types",                  "ChTypes__5Chaos"},
    {"sequence",            "ChSequence__5Chaos"},
    {"issequence",        "ChIsSequence__5Chaos"},
    {"len",                   "ChLength__5Chaos"},
    {"length",                "ChLength__5Chaos"},
    {"concat",                "ChConcat__5Chaos"},
    {"slice",                  "ChSlice__5Chaos"},
    {"element",              "ChElement__5Chaos"},
    {"search",                "ChSearch__5Chaos"},
    {"rreplace",            "ChRReplace__5Chaos"},
    {"replace",              "ChReplace__5Chaos"},
    {"sreplace",            "ChSReplace__5Chaos"},
    {"insert",                "ChInsert__5Chaos"},
    {"append",               "ChMInsert__5Chaos"},
    {"rdelete",                 "ChRDel__5Chaos"},
    {"delete",                   "ChDel__5Chaos"},
    {"iterator",            "ChIterator__5Chaos"},
    {"isiterator",        "ChIsIterator__5Chaos"},
    {"more",                    "ChMore__5Chaos"},
    {"next",                    "ChNext__5Chaos"},
    {"call",                    "ChCall__5Chaos"},
    {"mcall",                  "ChMCall__5Chaos"},
    {"==",                      "ChOpEq__5Chaos"},
    {"!=",                     "ChOpNEq__5Chaos"},
    {">=",                     "ChOpGEq__5Chaos"},
    {">",                       "ChOpGT__5Chaos"},
    {"<=",                     "ChOpLEq__5Chaos"},
    {"<",                       "ChOpLT__5Chaos"},
    {"true",                    "ChTrue__5Chaos"},
    {"!",                     "ChSetVar__5Chaos"},
    {"$",                      "ChFetch__5Chaos"},
    {"set",                   "ChSetVar__5Chaos"},
    {"chr",                      "ChChr__5Chaos"},
    {"ord",                      "ChOrd__5Chaos"},
    {"scopes",                "ChScopes__5Chaos"},
    {"(",                  "ChPushScope__5Chaos"},
    {")",                   "ChPopScope__5Chaos"},
    {"[",                  "ChPushStack__5Chaos"},
    {"]",                   "ChPopStack__5Chaos"},
    {"rpush",                  "ChRPush__5Chaos"},
    {"rpop",                    "ChRPop__5Chaos"},
    {"that",                    "ChThat__5Chaos"},
    {"splice",                "ChSplice__5Chaos"},
    {"if",                        "ChIf__5Chaos"},
    {"while",                  "ChWhile__5Chaos"},
    {"eval",                    "ChEval__5Chaos"},
    {"exec",                    "ChExec__5Chaos"},
    {"catch",                  "ChCatch__5Chaos"},
    {"throw",                  "ChThrow__5Chaos"},
    {"parse",                  "ChParse__5Chaos"},
    {"dup",                      "ChDup__5Chaos"},
    {"drop",                    "ChDrop__5Chaos"},
    {"swap",                    "ChSwap__5Chaos"},
    {".",                      "ChPrint__5Chaos"},
    {"echo",                    "ChEcho__5Chaos"},
    {"view",                    "ChView__5Chaos"},
    {"dump",                    "ChDump__5Chaos"},
    {"new",                      "ChNew__5Chaos"},
    {"new2",                    "ChNew2__5Chaos"},
    {"Null",                "ChPushNull__5Chaos"},
    {"nset",                     "ChSet__5Chaos"},
    {"names",                  "ChNames__5Chaos"},
    {"this",                    "ChThis__5Chaos"},
    {"builtins",            "ChBuiltins__5Chaos"},
    {"precision",          "ChPrecision__5Chaos"},
    {"nnspace",       "ChNamedNamespace__5Chaos"},
    {"order",                  "ChOrder__5Chaos"},
    {"shutdown",            "ChShutDown__5Chaos"},
    {"gettimeofday",    "ChGetTimeOfDay__5Chaos"},
    {NULL,                                  NULL}
};

extern void onError(const char *type = NULL);

void Chaos::ChPush(Slot el)
{
#ifdef DEBUGLOG
  if (el) {
    el->dump(cerr) << " Push / " << (1 + length()) << '\n';
  } else {
    cerr << "Push NULL\n";
  }
#endif
    if (el) {
        push(el);
    } else {
        push(new Null);
    }
}

Slot Chaos::ChPop()
{
    Slot el;
    pop(el);	// this is a Stack, it'll modify itself
#ifdef DEBUGLOG
    if (el) {
      el->dump(cerr) << " Pop / " << length() <<'\n';
    } else {
      cerr << "NULL Pop\n";
    }
#endif
    return el;
}

tSlot<Symbol> Chaos::Lookup(const char *var)
{
    DEBLOG(cerr << "Lookup: " << var << " in scopes ");
    DEBLOG(scopes->dump(cerr) << " => \n");

    Slot name= new String(var);
    tSlot<Symbol> sym;
    for (int scopeidx = scopes->length();
            scopeidx;
            scopeidx--) {
      Slot el(scopes->element(scopeidx-1));
      DEBLOG(cerr << "Lookup: " << var << " in Namespace " << el << '\n');
      tSlot<Namespace> scope(el);
      sym = scope->search(name);
      if (sym) { 
	  DEBLOG(sym->dump(cerr) << " Found\n");
	  return sym;
      } else {
	  DEBLOG(cerr << " not found\n");
      }
    }

    // last resort: global (store-wide) namespace
    DEBLOG(cerr << " global search: \n");
    sym = store->Names->search(name);
    if (!sym) {
	DEBLOG(cerr << " NOT FOUND\n");
	throw new Error("dictnf", name, "key not found");
    }
    DEBLOG(sym->dump(cerr) << " Found\n");

    return sym;
}

void Chaos::ChVar(const char *var)
{
    tSlot<Symbol> sym = Lookup(var);
    ChPush(sym->value());
}

void Chaos::ChLookup()
{
    Slot name = ChPop();
    DEBLOG(cerr << "Lookup " << (char *)(name) << '\n');    
    tSlot<Symbol> sym = Lookup((char*)name);
    ChPush(sym);
}

void Chaos::ChFetch()
{
    Slot name = ChPop();
    DEBLOG(cerr << "ChFetch " << (char *)(name) << '\n');
    ChVar((char*)name);
    //cerr << "ChFetch " << (char *)(name) << "->";
    //top()->dump(cerr);
    //cerr << '\n';
}

void Chaos::ChSetVar()
{
    // < slot name -- >

    Slot name = ChPop();
    Slot val = ChPop();
    DEBLOG(cerr << "SetVar " << (char*)name
           << " = " << val
	   << " in scope " << scopes->top()
	   << '\n');    
    scopes->top()->insert(name, val);
}

void Chaos::ChType()
{
    // < slot -- type >

    DEBLOG(cerr << "Type\n");
    Slot arg = ChPop();
    ChPush(arg.typeId());
}

void Chaos::ChTypes()
{
    DEBLOG(cerr << "Types\n");
    Slot t  = store->Types;
    ChPush(t);
}

void Chaos::ChNamedNamespace()
{
    // < keylist valuelist -- nnspace >
    // NNs typically contain other NNs, so construct empty
    Slot name = ChPop();
    Slot nspace = ChPop();
    Namespace* n = (Namespace*)nspace;
    if(dynamic_cast<Namespace*>(n)!=n) {throw new Error("chaos",nspace,"not a namespace");}
    NamedNamespace* result = new NamedNamespace(name,nspace);
    DEBLOG(cerr << "Created NN at " << (void*)result << ".\n");
    n -> insert(Slot((Data*)(Name*)result));
    ChPush((Data*)(Namespace*)result);
}

// create a new instance of named type, pass the args to its constructor
void Chaos::ChNew()
{
    DEBLOG(cerr << "New\n");
    Slot tname = ChPop();
    Slot args = ChPop();
    ChPush(Data::New(tname, args));
}

// create a new instance of named type, pass the args to its constructor
void Chaos::ChNew2()
{
    DEBLOG(cerr << "New2\n");
    Slot so = ChPop();
    Slot tname = ChPop();
    Slot args = ChPop();
    ChPush(Data::New(tname, args, so));
}

void Chaos::ChPrecision()
{
    // Set default precision for all floating-point operations
    // < longint -- >
    DEBLOG(cerr << "Precision\n");
    Slot digits = ChPop();
    long n = ((Integer *)digits)->value();
    if (n >= 0) {
        Real::precision((unsigned long) n);
    } else {
        throw new Error("range", n, "wanted positive integer");
    }
}

void Chaos::ChChr()
{
    Slot num = ChPop();
    char chr[2] = {0, 0};
    chr[0] = (char) (((int) num) & 0xff);

    ChPush(new String(chr));
}

void Chaos::ChOrd()
{
    Slot chr = ((String *)ChPop())->nullterm();
    ChPush(new Integer((int) (((char *) chr)[0])));
}

void Chaos::ChSplice()
{
    // < list -- list[0] list[1] ... list[n] >
    concat(ChPop());
}

void Chaos::ChThrow ()
{ 
    throw ChPop();
    // tSlot<Error> err = ChPop(); 
    // throw (Error *) err; 
}

void Chaos::ChEval()
{
    ChCatch();		// 
    
    Slot error = ChPop();	// collect the error

    if (error != no_error) {
	// display the Error, if any
	error->dump(cerr) << '\n';
    }
}

void Chaos::invoke(tSlot<Builtin> bi)
{
    DEBLOG(cerr << "call Builtin "); DEBLOG(bi->dump(cerr) << '\n');

    if (bi->sym->type() == Csym::FUNCTION) {
	Slot arg = new List(-1, this);
	bi->mvcall(this, arg);
    } else {
	throw new Error("unimpl", "I don't know how to invoke that");
    }
}

void Chaos::ChInvoke() {
    tSlot<Builtin> builtin = ChPop();

    invoke(builtin);
}

Slot Chaos::call(Slot &args)
{
    DEBLOG(cerr << "Chaos::call ");
    DEBLOG(args->dump(cerr) << '\n');

    for (int i = 0; i < args->length(); i++) {
	Slot datum = args[i];
	if (AKO(datum, Builtin)) {
	    try {
		invoke(toType(datum, Builtin));
	    } catch (...) {
		ChPush(args->slice(i + 1, -1));   // Push whatever's left for resumption.
		throw;
	    }
	} else {
	    // everything else is just pushed
	    ChPush(datum);
	}
    }
    
    return this;
}

void Chaos::ChCall()
{
    DEBLOG(cerr << "Call\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg1->call(arg2));
}

void Chaos::ChExec()
{
    // < code -- result >
    if (AKO(top(), Builtin)) {
        ChInvoke();
    } else if (AKO(top(), List)) {
        ChPush(this);
        ChCall();
        ChPop();
    }
}

void Chaos::ChCatch()
{
    // < ExprStr -- >
    // Whatever code is contained in ExprStr is executed, so the stack may
    // grow or shrink accordingly
    // TOS will always be the Error (or the global Chaos::no_error)
  
    DEBLOG(cerr << "ChCatch: ");
  
    try {
        ChExec();
	// no error occurred
	ChPush(no_error);
    } catch (Error *e) {
	e->upcount();
	ChPush(e);
	e->dncount();
    } catch (exception e) {
	ChPush(new Error("stdexcept", e.what()));
    } catch (char *str) {
	ChPush(new Error("unknown", str));
    } catch (...) {
	ChPush(new Error("unknown", "unknown"));
    }
}

void Chaos::ChIf()
{
    // < ifExpr elseExpr testExpr -- [results of eval] >
    Slot falseCase = ChPop();
    Slot trueCase = ChPop();

    DEBLOG(cerr << "If\n");
    //cout << "If: "; ChView();
    if (ChPop()->truth()) {
        ChPush(trueCase);
    } else {
        ChPush(falseCase);
    }
    ChExec();
}

void Chaos::ChWhile()
{
    // < bodyExpr testExpr -- ... >
    DEBLOG(cerr << "While\n");
    Slot test = ChPop();
    Slot body = ChPop();
    DEBLOG(cerr << test->typeId() << " " << test->dump(cerr) << " test0\n");
    ChPush(test);
    ChExec();
    Slot tvar = ChPop();
    DEBLOG(cerr << tvar->typeId() << " " << tvar->dump(cerr) << " val\n");
    while (tvar && tvar->truth()) {
        DEBLOG(cerr << body->typeId() << " " << body->dump(cerr) << " body\n");
        ChPush(body);
        ChExec();
        DEBLOG(cerr << test->typeId() << " " << test->dump(cerr) << " test\n");
        ChPush(test);
        ChExec();
        tvar = ChPop();
        DEBLOG(cerr << tvar->typeId() << " " << tvar->dump(cerr) << " val\n");
    }
}

void Chaos::ChTrue()
{
    // Converts one arg to a raw truth value
    // < Slot -- bool >

    DEBLOG(cerr << "True\n");
    ChPush(ChPop()->truth());
}

void Chaos::ChSum()
{
    // < a b -- a+b >
    DEBLOG(cerr << "Sum\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->add(arg2));
}

void Chaos::ChSub()
{ 
    // < a b -- a-b >
    DEBLOG(cerr << "Sub\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->subtract(arg2));
}

void Chaos::ChMult()
{
    // < a b -- a*b >
    DEBLOG(cerr << "Mult\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->multiply(arg2));
}

void Chaos::ChDiv()
{
    // < a b -- a/b >
    DEBLOG(cerr << "Div\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->divide(arg2));
}

void Chaos::ChOrder()
{
    // < a b -- a/b >
    DEBLOG(cerr << "Div\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->order(arg2));
}


void Chaos::ChMod()
{
    // < a b -- a%b >
    DEBLOG(cerr << "Div\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->modulo(arg2));
}

void Chaos::ChDup()
{
    // < a -- a a >
    DEBLOG(cerr << "Dup\n");
    Slot arg1 = ChPop();
    ChPush(arg1);
    ChPush(arg1);
}

void Chaos::ChSwap()
{
    // < a b -- b a >
    DEBLOG(cerr << "Swap\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg1);
    ChPush(arg2);
}

void Chaos::ChDrop()
{
    // < a -- >
    DEBLOG(cerr << "Drop\n");
    ChPop();
}

void Chaos::ChEcho()
{
    // < a -- >
    Slot arg1 = ChPop();

    if (arg1.isString()) {
        Slot terminated = ((String *) arg1)->nullterm();
        cout << (char *) terminated;
    } else {
        arg1->dump(cout) << '\n';
    }
}

void Chaos::ChPrint()
{
    // < a -- >
    DEBLOG(cerr << "Print\n");
    ChPop().Dump(cerr) << '\n';
    //<< " (" << sizeof(typeof(*(Data*)arg1)) << ")\n";
}

void Chaos::ChView()
{
    // < -- >
    DEBLOG(cerr << "View\n");
    //    dump(cout) << '\n';

    for (int len = Length() - 1; len >= 0; len --) {
        slice(len)->dump(cout) << "\n";
    }
}

void Chaos::ChDump()
{
    // < -- >
    DEBLOG(cerr << "Dump\n");
    dump(cout) << '\n';
}

void Chaos::ChNames()
{
    // < -- Namespace >
    DEBLOG(cerr << "Names\n");
    ChPush(store->Names);
}

void Chaos::ChThis()
{
    // < -- copyOfStack >
    DEBLOG(cerr << "This\n");
    //ChPush((Chaos *)clone());
    ChPush(this);
}

void Chaos::ChBuiltins()
{
    // < -- Namespace >
    DEBLOG(cerr << "Builtins\n");
    //    store->Registered->dump(cout) << '\n';
    ChPush(store->Registered);
}

void Chaos::ChScopes()
{
    DEBLOG(cerr << " ChScopes " << scopes->typeId() << '\n');
    //DEBLOG(scopes->dump(cerr) << " ChScopes\n");
    ChPush(scopes);
}

void Chaos::ChPushScope()
{
    /* < namespace -- >
     * Pushes TOP onto the scope stack.
     */
    DEBLOG(cerr << "ChPushScope\n");
    tSlot<Namespace> scope = ChPop();
    scopes->push(scope);
}

void Chaos::ChPopScope()
{
    /* < -- >
     * Pops current namespace off the scope stack.
     */
    DEBLOG(cerr << "ChPopScope\n");
    Slot junk;
    scopes = (Stack*)(scopes->pop(junk));
}

void Chaos::ChPushStack()
{
    /* < ... -- >
     * Pushes current stack onto this.stacks and empties this.  Used
     * to begin a List.
     */
    Slot saved = slice(0, length());
    stacks->push(saved);
    if (length()) {
        del(0, length());
    }
}

void Chaos::ChPopStack()
{
    /* < ... -- ,,, [ ... ]>
     * Pops previous stack from this.stacks and pushes current stack
     * as a list.  Used to end a List.
     */
    Slot list = slice(0, length());
    Slot prev;
    stacks->pop(prev);
    replace(0, length(), prev);
    ChPush(list);
}

void Chaos::ChRPush()
{
    // push top onto ReturnStack
    DEBLOG(cerr << "ChRPush\n");
    rstack = (Stack*)(rstack->push(ChPop()));
}

void Chaos::ChRPop()
{
    // pop ReturnStack to stack
    Slot tmp;
    DEBLOG(cerr << "ChRPop\n");
    rstack = (Stack*)(rstack->pop(tmp));
    ChPush(tmp);
}

void Chaos::ChThat()
{
    // pop whole ReturnStack to stack
    DEBLOG(cerr << "ChThat\n");
    push(rstack);
}

void Chaos::ChPushNull()
{
    push(new Null);
}

void Chaos::ChSet()
{
    Slot contents = ChPop();
    Set* result = new VectorSet(contents);
    ChPush(result);
}    

void Chaos::ChPositive()
{
    DEBLOG(cerr << "Positive\n");
    Slot arg1 = ChPop();
    ChPush(arg1->positive());
}

void Chaos::ChNegative()
{
    DEBLOG(cerr << "Negative\n");
    Slot arg1 = ChPop();
    ChPush(arg1->negative());
}

void Chaos::ChInvert()
{
    DEBLOG(cerr << "Invert\n");
    Slot arg1 = ChPop();
    ChPush(arg1->invert());
}

void Chaos::ChAnd()
{
    DEBLOG(cerr << "And\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg1->and(arg2));
}

void Chaos::ChXor()
{
    DEBLOG(cerr << "Xor\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg1->xor(arg2));
}

void Chaos::ChOr()
{
    DEBLOG(cerr << "Or\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg1->or(arg2));
}

void Chaos::ChLShift()
{
    DEBLOG(cerr << "LShift\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->lshift(arg2));
}

void Chaos::ChRShift()
{
    DEBLOG(cerr << "RShift\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->rshift(arg2));
}

void Chaos::ChSequence()
{
    DEBLOG(cerr << "Sequence\n");
    Slot arg1 = ChPop();
    ChPush(arg1->toSequence());
}

void Chaos::ChIsSequence()
{
    DEBLOG(cerr << "IsSequence\n");
    Slot arg1 = ChPop();
    ChPush(arg1->isSequence());
}

void Chaos::ChLength()
{
    DEBLOG(cerr << "Length\n");
    Slot arg1 = ChPop();
    ChPush(arg1->length());
}

void Chaos::ChConcat()
{
    DEBLOG(cerr << "Concat\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->concat(arg2));
}

void Chaos::ChSlice()
{
    DEBLOG(cerr << "Slice\n");
    Slot arg3 = ChPop();
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->slice(arg2, arg3));
}

void Chaos::ChElement()
{
    DEBLOG(cerr << "Element\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->slice(arg2));
}

void Chaos::ChSearch()
{
  // < STR haystack STR needle -- INT index > (if found)
  // < STR haystack STR needle -- ~false > (if not found)
  // XXX FIXME: need support for ~foo symbols
  DEBLOG(cerr << "Search\n");
  Slot needle   = ChPop();
  Slot haystack = ChPop();
  
  ChPush(haystack->search(needle));
}

void Chaos::ChRReplace()
{
    DEBLOG(cerr << "RReplace\n");
    Slot arg4 = ChPop();
    Slot arg3 = ChPop();
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->replace(arg2, arg3, arg4));
}

void Chaos::ChReplace()
{
    DEBLOG(cerr << "Replace\n");
    Slot arg3 = ChPop();
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->replace(arg2, arg3));
}

void Chaos::ChSReplace()
{
    DEBLOG(cerr << "SReplace\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->replace(arg2));
}

void Chaos::ChInsert()
{
    DEBLOG(cerr << "Insert\n");
    Slot arg3 = ChPop();
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->insert(arg2, arg3));
}

void Chaos::ChMInsert()
{
    DEBLOG(cerr << "MInsert\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->insert(arg2));
}

void Chaos::ChRDel()
{
    DEBLOG(cerr << "RDel\n");
    Slot arg3 = ChPop();
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->del(arg2, arg3));
}

void Chaos::ChDel()
{
    DEBLOG(cerr << "Del\n");
    Slot arg2 = ChPop();
    Slot arg1 = ChPop();
    ChPush(arg1->del(arg2));
}

void Chaos::ChIterator()
{
    DEBLOG(cerr << "Iterator\n");
    Slot arg1 = ChPop();
    ChPush(arg1->iterator());
}

void Chaos::ChIsIterator()
{
    DEBLOG(cerr << "IsIterator\n");
    Slot arg1 = ChPop();
    ChPush(arg1->isIterator());
}

void Chaos::ChMore()
{
    DEBLOG(cerr << "More\n");
    Slot arg1 = ChPop();
    ChPush(arg1->More());
}

void Chaos::ChNext()
{
    DEBLOG(cerr << "Next\n");
    Slot arg1 = ChPop();
    ChPush(arg1->Next());
}

void Chaos::ChMCall()
{
    DEBLOG(cerr << "MCall\n");
    tSlot<Builtin> meth(ChPop());
    Slot args = ChPop();
    Slot that = ChPop();
    
    ChPush(meth->mcall(that, args));
}

void Chaos::ChOpEq()
{
    DEBLOG(cerr << "OpEq\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg1 == arg2);
}

void Chaos::ChOpNEq()
{
    DEBLOG(cerr << "OpNEq\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg1 != arg2);
}

void Chaos::ChOpGEq()
{
    DEBLOG(cerr << "OpGEq\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg2 >= arg1);
}

void Chaos::ChOpGT()
{
    DEBLOG(cerr << "OpGT\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg2 > arg1);
}

void Chaos::ChOpLEq()
{
    DEBLOG(cerr << "OpLEq\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg2 <= arg1);
}

void Chaos::ChOpLT()
{
    DEBLOG(cerr << "OpLT\n");
    Slot arg1 = ChPop();
    Slot arg2 = ChPop();
    ChPush(arg2 < arg1);
}

void chaos_bkpt ();

Chaos::Chaos()
  : scopes(new Stack()),
    base_scope(new Namespace()),
    stacks(new Stack()),
    rstack(new Stack())
{
    DEBLOG(cerr << "Chaos " << this
           << '\n');
    base_scope->insert("scopes", scopes);

    base_scope->insert("stacks", stacks);
    scopes->push(base_scope);
}

Chaos::Chaos(Slot const &args)
  : scopes(new Stack()),
    base_scope(new Namespace()),
    stacks(new Stack()),
    rstack(new Stack())
{
    DEBLOG(cerr << "Chaos" << this
           << " from args " << args
           << '\n');
    base_scope->insert("scopes", scopes);
    base_scope->insert("stacks", stacks);
    scopes->push(base_scope);
    chaos_bkpt();
}

Chaos::Chaos(Slot command, Chaos *p)
    : scopes(new Stack()),
    base_scope(p->base_scope),
    stacks(new Stack()),
    parent(p),
    rstack(new Stack())
{
    DEBLOG(cerr << "Chaos" << this
	   << "Child of " << p
	   << '\n');
    base_scope->insert("scopes", scopes);
    base_scope->insert("stacks", stacks);
    scopes->push(base_scope);
    chaos_bkpt();
}

Slot Chaos::incoming(Slot from)
{
    //cerr << "INCOMING\n";
    try {
	ChPush(from);
	ChVar("incoming");
	ChExec();
    } catch (Error *e) {
	e->upcount();
	cerr << "ChCatch got Error ";
	e->dump(cerr);
	cerr << '\n';
	e->dncount();
    } catch (exception e) {
	cerr << "ChCatch got stdexcept " << e.what() << '\n';
    } catch (char *str) {
	cerr << "ChCatch got string " << str << '\n';
    } catch (Chaos *c) {
	cerr << "ChCatch got suspend " << '\n';
    } catch (...) {
	cerr << "ChCatch got unknown " << '\n';
	ChPush(new Error("unknown", "unknown"));
    }

    return this;
}

Slot Chaos::outgoing(Slot from)
{
    //cerr << "OUTGOING\n";
    return (Data*)NULL;	// uninterested in outgoing
    ChPush(from);
    ChVar("outgoing");
    ChExec();
    return ChPop();
}

Slot Chaos::connect(Slot from)
{
    //cerr << "CONNECT\n";
    ChPush(from);
    ChVar("connected");
    ChExec();
    return this;
}

Slot Chaos::disconnect(Slot from)
{
    //cerr << "DISCONNECT\n";
    ChPush(from);
    ChVar("disconnected");
    ChExec();
    return this;
}

#if 0
Slot Chaos::connect(Slot from)
{
    return this;
}

Slot Chaos::disconnect(Slot from)
{
    return this;
}

Slot Chaos::incoming(Slot from)
{
    // append a bufferful of incoming to our buffer
    tSlot<List> input = from->slice(-1);
    if (buffer && buffer->length())
	buffer = buffer->concat(input->join());
    else
	buffer = input->join();
    
    DEBLOG(cerr << "incoming: ");
    DEBLOG(buffer->dump(cerr) << '\n');
    
    while (buffer->length()) {
	
	tSlot<Chaos> ch = new Chaos();
	
	// parse the buffer
	ch->ChPush(buffer);
	ch->ChParse();
	DEBLOG(cerr << "parse: ");
	DEBLOG(ch->dump(cerr) << '\n');
	
	// see if parsing was able to use any input
	tSlot<String> remains = ch->ChPop();
	if (remains != buffer) {
	    // parsing consumed some input - more remains
	    buffer = remains;
	} else {
	    // parsing has stalled
	    break;
	}
	
	// evaluate the parsed expression
	if (ch->length()) {
	    ChPush(ch);
	    ChCatch();
	    Slot err = ChPop();
	    if (err != no_error) {
		cerr << "Err: ";
		err->dump(cerr) << '\n';
	    }
	}
    }

    // if (buffer && buffer->length()) {
    //      cout << "[BUF]: " << buffer->toconstruct();
    // }
    
    return this;	// remain interested in incoming
}

Slot Chaos::outgoing(Slot to)
{
    return (Data*)NULL;	// uninterested in outgoing
}
#endif

Chaos::~Chaos()
{}

Data *Chaos::mutate(void *store) const
{
    return identity();
}

void ChExit()
{
}

#if 0
String *Chaos::operator += (String *b)
{
    cerr << "BINGO\n";
    if (buffer) {
        buffer = buffer->concat(b);
    } else {
        buffer = b;
    }
    return buffer;
}
#endif

void Chaos::bootstrap()
{
    tSlot<File> f = new File("boot.Ch", ios::in, 0);
    tSlot<Chaos> ch = new Chaos();
    Slot buffer;
    Slot nl("\n");
    while (f->More()) {
	Slot line = f->Next();
	if (buffer && buffer->length())
	    buffer = buffer->concat(line)->concat(nl);
	else
	    buffer = line->concat(nl);
    }
    buffer = ch->parse(buffer);
    if (buffer->length()) {
        cerr << "Parser leftovers: " << buffer << '\n';
    }
    
    // evaluate the parsed expression
    if (ch->length()) {
	ChPush(ch);
	ChCatch();
	Slot err = ChPop();
	if (err != no_error) {
	    cerr << "Err: ";
	    err->dump(cerr) << '\n';
	}
    }
}

void Chaos::ChShutDown()
{
    exit(0);
//    throw "Shutdown";
}

void DumpVerbosely(bool);
#ifdef PROFILING
extern "C" {
    void pct_on (char **argv);
}
#endif

/** called when Elf has loaded this module
 */
void initElf(Elf *elf)
{
    //cerr << "chaos's initElf called\n";
    //DumpVerbosely(true);	// change this to true to get addresses in output

    Chaos *lexer;
    chaos_bkpt ();		// breakpoint opportunity
#if 0
    for (DopeV *dv = DopeV::all; dv; dv=dv->next) {
	cerr << dv->name
	     << ' ' << dv->vptr 
	     << ' ' << dv->vptr_size
	     << ' ' << dv->class_size << '\n';
    }
#endif
    // register our Builtins with ColdStore
#if 0
    try {
        RegisterElf(elf);
    } catch (Error *e) {
        e->dump(cerr) << '\n';
        throw e;
    }
#endif

    Slot sym = store->Names->search("Chaos");
    if (sym) {
        lexer = (Chaos*)(Data*)((Symbol *)sym)->value();
        cerr << "resuming lexer\n";
        DEBLOG(cerr << "Chaos " << lexer->typeId()
               << ' ' << lexer
               << '\n');
    } else {

        lexer = new Chaos();
        Chaos::no_error = new Error("none", "No Error");
        lexer->init_parse();

        cerr << "new lexer\n";
        DEBLOG(cerr << "Chaos " << lexer->typeId()
               << ' ' << lexer
               << '\n');
        store->Names->insert("Chaos", lexer);
        //store->Names->insert("PS1", "> ");
    }

    if (!sym) {
	// stdio stuff
	try {
	    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
	    LConnection *c_in = new LConnection(fileno(stdin),(Data*)lexer, (Data*)NULL);
	    lexer->ChPush(c_in);
	    lexer->ChPush(new String("cin"));
	    lexer->ChSetVar();
	    c_in->connect(lexer);
	} catch (Error *e) {
	    e->upcount();
	    cerr << "\nI/O error: ";
	    e->dump(cerr) << '\n';
	    e->dncount();
	}
	
	try {
	    fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
	    BConnection *c_out = new BConnection(fileno(stdout),(Data*)NULL,(Data*)lexer);
	    lexer->ChPush(c_out);
	    lexer->ChPush(new String("cout"));
	    lexer->ChSetVar();
	    c_out->connect(lexer);
	} catch (Error *e) {
	    e->upcount();
	    cerr << "\nI/O error: ";
	    e->dump(cerr) << '\n';
	    e->dncount();
	}
    } else {
	fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
	fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
    }

    // perform bootstrapping - loaded by cchaos
    if (!sym) {
      try {
	lexer->bootstrap();
      } catch (Error *e) {
	e->upcount();
	cerr << "\nBootstrap error: ";
	e->dump(cerr) << '\n';
	e->dncount();
      } catch (exception e) {
	cerr << "\nbootstrap error: " << e.what() << '\n';
      }
    }


#ifdef PROFILING
    extern char **argv_glob;
    pct_on(argv_glob);
#endif

    //lexer->dump(cerr) << '\n';
    //Memory::Assert();

    //yyin = stdin;
    while(1) {
        try {
	  if (!Endpoint::handle_io_events(-1))
	    break;
        }
        catch (Error *e) {
	    e->upcount();
	    e->dump(cerr) << '\n';
	    e->dncount();
	    //lexer->ChPush((Symbol*)*e);
	    // cerr << "stack underflow\n";
        } catch (char *err) {
            cerr << "ERROR: " << err << '\n';
            onError();
        }
    }
    onError();
    //delete lexer;
}

void Chaos::check(int) const
{
}

extern bool DumpVerbose;
ostream &Chaos::dump(ostream& out) const
{
    if (this) {
        if (DumpVerbose)
            out << '[' << Length() << ']';
        Slot i(iterator());
        out << "{";
        while (i->More()) {
	    Slot n = i->Next();
	    if (n != this) {
		n.Dump(out);
	    } else {
		out << "**THIS**";
	    }
            if (i->More()) {
                out << ", ";
            }
        }
        out << "}";
    } else {
        out << "[NULL]";
    }
    return out;
}
