#include <iostream>
#include <string>
#include "MetaType.h"

void MetaType::TranslateClass(Environment* env)
{
    string fn("inline bool isAKO(const type_info& arg) {\nreturn ");
    fn += "(typeid(";
        fn += ") == arg)";
    
    Ptree *p;
    for (int i = 0; p = NthBaseClassName(i); i++) {
        fn += " || ";
        fn += p->ToString();
        fn += "::isAKO(arg)";
    }
    
    fn += ";\n}";
    char *buf = (char*)alloca(fn.length()+1);
    strcpy(buf, fn.data());
    Ptree *body = Ptree::Make(buf);
    AppendMember(body);
}

/// Local Variables ***
/// c++-mode:t ***
/// End: ***
