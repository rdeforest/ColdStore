#include <mop.h>

// MetaType generates an isAKO(typeinfo&) predicate for comparing the given type
class MetaType : public Class {
public:
    void TranslateClass(Environment* env);
    Ptree *FinalizeInstance() {
        return Class::FinalizeInstance();
    }

#ifdef notdef
    static bool Initialize() {
        ChangeDefaultMetaclass("MetaType");
    }
#endif
};
