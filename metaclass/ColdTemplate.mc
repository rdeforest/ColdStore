#include <iostream>
#include <string>
#include "ColdTemplate.h"

bool ColdTemplate::Initialize()
{
    RegisterMetaclass("coldT", "ColdTemplate"); 
    return TRUE;
}

Ptree *ColdTemplate::FinalizeInstance()
{
    return NULL;
}

void ColdTemplate::TranslateClass(Environment* env)
{
}

/// Local Variables:
/// mode:c++
/// mode:font-lock
/// End:
