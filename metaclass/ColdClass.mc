#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <strstream>
#include "ColdClass.h"

bool ColdClass::Initialize()
{
    //ChangeDefaultMetaclass("ColdClass");
    RegisterMetaclass("cold", "ColdClass");
    return TRUE;
}

Ptree *ColdClass::FinalizeInstance()
{
    static bool done = false;	// we only want to emit this once
    ClassArray a;

    if (done)
        return Class::FinalizeInstance();
    else
        done = true;

    Ptree *result = Ptree::qMake("asm(\".section .gnu.linkonce.sizeof,\\\"ax\\\",@progbits\");\n");
    
    //string extra("asm(\".section .gnu.linkonce.sizeof,\\\"ax\\\",@progbits\");\n");
    for (int n = InstancesOf("ColdClass", a);      // get the number of elements
         n;
         n--) {
        string extra;
        Class *c = a[n-1];
        char *name = c->Name()->ToString();

        char nname[256];
        sprintf(nname, "%s", name);
        
        char sname[256];
        sprintf(sname, "%d%s", strlen(name), name);
        result = Ptree::Append(result, Ptree::Make("size_t %s::__sizeof __attribute__ ((section (\".gnu.linkonce.d._%s.__sizeof\"))) = sizeof(%s);\n",
                                                   nname, sname, nname));
    }
    
    for (int n = InstancesOf("ColdClass", a);      // get the number of elements
         n;
         n--) {
        string extra;
        Class *c = a[n-1];
        char *name = c->Name()->ToString();

        char sname[256];
        sprintf(sname, "%d%s", strlen(name), name);

        char cname[256];
        sprintf(cname, "%s", name);

        result = Ptree::Append(result, Ptree::Make("__attribute__ ((section (\".gnu.linkonce.d._%s_dopeV\"))) ", sname));
        result = Ptree::Append(result, Ptree::Make("_DopeV %s_dopeV(\"%s\", (void*)__vt_%s, sizeof(__vt_%s), sizeof(%s));\n",
                    cname, cname, sname, sname, cname));

#if 0
        result = Ptree::Append(result, Ptree::Make("_DopeV %s::__dopeV __attribute__ ((section (\".gnu.linkonce.d._%s.__dopeV\"))) \
	= {(DopeV*)0, \"%s\", (void*)__vt_%s, sizeof(__vt_%s), sizeof(%s)};\n",
                    cname, sname, cname, sname, sname, cname));

        result = Ptree::Append(result, Ptree::Make("__attribute__ ((constructor, section (\".gnu.linkonce.t._%sdopeV\"))) void %s_dopeV()  \
	{ DopeV::doDV(&%s::__dopeV); }\n",
                    sname, cname, cname));
#endif
    }

    return result;
}

static string genBase(Class *c)
{
    string expr("(arg == typeid(");
    expr += c->Name()->ToString();
    expr += "))";
    Class *k;
	for (int i = 0; (k = c->NthBaseClass(i)); i++) {
        expr += " || " + genBase(k);
    }
    return expr;
}

void ColdClass::TranslateClass(Environment* env)
{
    //cerr << MetaclassName() << ": "; Name()->Display();

    {
        // add a _sizeof symbol to each ColdClass
#if 0
        Ptree *expr = Ptree::qMake("sizeof(`Name()`)");
        int size = 0;
        expr->Reify((unsigned)size);
#endif
        Ptree *body = Ptree::qMake("public: static size_t __sizeof;\n");
        //body->Display();
        AppendMember(body);
    }

#if 0
    {
        // add a __dopeV data member to each ColdClass
        Ptree *body = Ptree::qMake("public: static DopeV __dopeV;\n");
        //body->Display();
        AppendMember(body);
    }

    {
        // add a __dopeV virtual to each ColdClass
        Ptree *body = Ptree::qMake("public: virtual DopeV *_dopeV() {return &__dopeV;}\n");
        //body->Display();
        AppendMember(body);
    }
#endif
    //Name()->Display(); BaseClasses()->Display();
}

/// Local Variables:
/// mode:c++
/// mode:font-lock
/// End:
