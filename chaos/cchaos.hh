#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define DEBUGLOG 1
struct PrimitiveEntry {
  char *name;
  char *fn;
};

PrimitiveEntry chPrimitives[] = {
  {"positive",         "ChPositive"},
  {"negative",         "ChNegative"},
  {"add",              "ChSum"},
  {"+",                "ChSum"},
  {"subtract",         "ChSub"},
  {"-",                "ChSub"},
  {"multiply",         "ChMult"},
  {"*",                "ChMult"},
  {"div",              "ChDiv"},
  {"/",                "ChDiv"},
  {"mod",              "ChMod"},
  {"%",                "ChMod"},
  {"invert",           "ChInvert"},
  {"~",                "ChInvert"},
  {"and",              "ChAnd"},
  {"&",                "ChAnd"},
  {"xor",              "ChXor"},
  {"^",                "ChXor"},
  {"or",               "ChOr"},
  {"|",                "ChOr"},
  {"lshift",           "ChLShift"},
  {"<<",               "ChLShift"},
  {"rshift",           "ChRShift"},
  {">>",               "ChRShift"},
  {"type",             "ChType"},
  {"sequence",         "ChSequence"},
  {"issequence",       "ChIsSequence"},
  {"len",              "ChLength"},
  {"length",           "ChLength"},
  {"concat",           "ChConcat"},
  {"slice",            "ChSlice"},
  {"element",          "ChElement"},
  {"search",           "ChSearch"},
  {"rreplace",         "ChRReplace"},
  {"replace",          "ChReplace"},
  {"sreplace",         "ChSReplace"},
  {"insert",           "ChInsert"},
  {"append",           "ChMInsert"},
  {"rdelete",          "ChRDel"},
  {"delete",           "ChDel"},
  {"iterator",         "ChIterator"},
  {"isiterator",       "ChIsIterator"},
  {"more",             "ChMore"},
  {"next",             "ChNext"},
  {"call",             "ChCall"},
  {"==",               "ChOpEq"},
  {"!=",               "ChOpNEq"},
  {">=",               "ChOpGEq"},
  {">",                "ChOpGT"},
  {"<=",               "ChOpLEq"},
  {"<",                "ChOpLT"},
  {"true",             "ChTrue"},
  {"!",                "ChSetVar"},
  {"set",              "ChSetVar"},
  {"scope",            "ChScope"},
  {"proc",             "ChProc"},
  {"cond",             "ChCond"},
  {"splice",           "ChSplice"},
  {"if",               "ChIf"},
  {"while",            "ChWhile"},
  {"eval",             "ChEval"},
  {"dup",              "ChDup"},
  {"drop",             "ChDrop"},
  {"swap",             "ChSwap"},
  {".",                "ChPrint"},
  {"echo",             "ChEcho"},
  {"view",             "ChView"},
  {"dump",             "ChDump"},
  {"new",              "ChNew"},
  {"new2",             "ChNew2"},
  {"Null",             "ChPushNull"},
  {"list",             "ChList"},
  {"dict",             "ChDict"},
  {"tree",             "ChTree"},
  {"regexp",           "ChRegexp"},
  {"hash",             "ChHash"},
  {"file",		 "ChFile"},
  {"names",            "ChNames"},
  {"this",             "ChThis"},
  {"builtins",         "ChBuiltins"},
  {"precision",        "ChPrecision"},
  {"nnspace",          "ChNamedNamespace"},
  {"nset",		 "ChSet"},
  {"order",            "ChOrder"},
  {NULL,               NULL}
};

class CChaos
  : public yyFlexLexer
{
  // lexing variables
  char *string;
  int  strl;
  char *sp;
  int  paren;
  
  PrimitiveEntry *primitives;
  
public:
  CChaos(istream *infile)
    : yyFlexLexer(infile),
      string(NULL),
      strl(0),
      sp(string),
      paren(0)
  {
    primitives = chPrimitives;
  }
  
  virtual ~CChaos() {}
  
  char *LookupPrimitive(const char *name) {
    int idx;
    
    for (idx = 0; primitives[idx].name; idx++) {
      if (strcasecmp(primitives[idx].name, name) == 0) {
	return primitives[idx].fn;
      }
    }

    return NULL;
  }

  void String(const char *string) {
    int l = strlen(string)+1;
    char *sdup = (char *)alloca(l);
    memcpy (sdup, string, l);
    sdup[strlen(sdup)-1] = '\0';
    cout << "\tChPush(\"" << 1+sdup << "\");\n";
  }

  void BString(const char *string) {
    cout << "\tChPush(\"" << string << "\");";
  }

  void Fn(const char *name) {
    char *fn = LookupPrimitive(name);
    if (fn) {
      cout << '\t' << fn << "();\n";
    } else {
      cout << "\tChVar(\"" << name << "\");"
	   << "ChEval();\n";
    }
  }

  void Var(const char *name) {
    cout << "\tChVar(\"" << name << "\");\n";
  }

  void FnPop() {
    cout << "\tChFn(ChPop());\n";
  }

  void VarPop() {
    cout << "\tChVar(ChPop())\n";
  }

  void Real(const char *digits) {
    cout << "\tChPush(new Real(\"" << digits << "\"));\n";
  }

  void Digit(const char *digits) {
    long n = strtol(digits, NULL, 0);
    if (errno == ERANGE) {
      // value's too big
      cout << "\tChPush(new BigInt(\"" << digits << "\"));\n";
    } else {
      cout << "\tChPush(Slot(" << n << "));\n";
    }
  }

  int yywrap(void)
  {
    return 1;
  }

  int yylex();	// defined by flex
  void reset();
};

extern "C" {
    int yywrap()
    {
        return 1;
    }
}

int main(int argc, char **argv)
{
  CChaos cchaos(&cin);

  //cchaos.set_debug(1);
  if (argc < 2) {
    cout << "void Chaos::bootstrap(void) {\n";
  } else {
    cout << argv[1] << "{\n";
  }
  cchaos.yylex();
  cout << "}\n";
}
