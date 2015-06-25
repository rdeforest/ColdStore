#include <mop.h>

// MetaType generates an isAKO(typeinfo&) predicate for comparing the given type
class ColdClass : public Class {
public:
    //void TranslateClass(Environment* env);
    static bool Initialize();
    Ptree *FinalizeInstance();
    void TranslateClass(Environment* env);
};

void CollectSlotMember(vector<string>& slotnames, Member& item);
void CollectSlotMemberFromClass(vector<string>& slotnames, Class* clazz);
