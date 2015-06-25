#include <iostream>
#include <string>
#include "temptest.h"

void temptest::TranslateClass(Environment* env)
{
    cerr << "Translate: ";
    Name()->Display();
    Ptree *decl = Ptree::Make("public: bool isAKO(const type_info& arg);");
    AppendMember(decl);
}

/// Local Variables:
/// mode:c++
/// mode:font-lock
/// End:
