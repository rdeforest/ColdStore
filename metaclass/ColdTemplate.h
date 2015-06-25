#include <mop.h>

class ColdTemplate : public TemplateClass
{
public:
    static bool Initialize();
    Ptree *FinalizeInstance();
    void TranslateClass(Environment* env);
//    Ptree* TemplateClass::TranslateInstantiation(Environment*, Ptree* spec)
//    Ptree* TemplateDefinition() // in mop
        //metaobject->AcceptTemplate()
};
