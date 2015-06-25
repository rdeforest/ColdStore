#include <mop.h>

// MetaType generates a counting call() method
class SlotClass : public Class {
public:
    static bool Initialize();

    Ptree* TranslateMemberRead(Environment* env, Ptree* object,
            Ptree* op, Ptree* member);
 
    Ptree* TranslateMemberWrite(Environment* env, Ptree* object,
            Ptree* op, Ptree* member, Ptree* assign_op, Ptree* expr);
 
    Ptree* TranslateMemberCall(Environment* env, Ptree* object,
            Ptree* op, Ptree* member, Ptree* arglist);

    Ptree* TranslateFunctionCall(Environment* env,
            Ptree* object, Ptree* args);
};
