#include <iostream>
#include <stdio.h>
#include <string>
#include "SlotClass.h"

#include <mop.h>
#include <types.h>
#include <ptree-core.h>
#include <env.h>
#include <typeinfo.h>
#include <member.h>

bool SlotClass::Initialize()
{
    //ChangeDefaultMetaclass("SlotClass");
    RegisterMetaclass("SlotMeta", "SlotClass");
    return TRUE;
}

Ptree* SlotClass::TranslateFunctionCall(Environment* env,
            Ptree* object, Ptree* arglist )
{
  Class *sl = env->LookupClassMetaobject( Ptree::Make("SlotClass"));
  
  // if we _Are_ slot (not an instance) leave it alone!!
  if (Ptree::Equal(object, Ptree::Make("Slot")))
    return Class::TranslateFunctionCall( env, object, arglist );
  

  // construct the arguments list.. but inside another function...
  Ptree *tmpvar = Ptree::GenSym();
  int arglength = Ptree::Length( arglist ) / 2;
  Ptree *vardecl = Ptree::Make("Slot %p = new Tuple(%d); ", tmpvar, arglength );

  InsertDeclaration(env, vardecl);

  int counter = 0;
  Ptree *varset = Ptree::Make("%p", tmpvar);
  for(PtreeIter i = Ptree::Second(arglist); !i.Empty(); i++) {
    varset = Ptree::Make("%p->replace(%d, %p)", varset, counter, *i);
    counter++;
    if (!i.Empty())
      i++;
  }

  
  return Ptree::Make("%p->call( %p )\n", object, varset );
}

Ptree* SlotClass::TranslateMemberCall(Environment* env, Ptree* object,
                                      Ptree* op, Ptree* member, Ptree* arglist)
{
     if(!op->Eq('.')) {
#if 0
         // translate call
         if (member->Eq("call")) {
             int nargs = (arglist->Second()->Length() + 1)/2;
             if (nargs > 0) {
                 arglist = Ptree::Make("( %p )",
                                       Ptree::Make("%d , %p",
                                                   nargs, arglist->Second()));              }
#if 0
             cout << "Slot: " << nargs << ' ';
             member->Display();
             arglist->Display();
#endif
         }
#endif
	 return Class::TranslateMemberCall(env, object, op, member, arglist);
     }

    // if it's something actually defined by Slot, handle it as normal
    if (LookupMember(member)) {
      return Class::TranslateMemberCall(env, object, op, member, arglist);
    }

    // construct the arguments list..
    Ptree *tmpvar = Ptree::GenSym();
    Ptree *tmpvar2 = tmpvar;
    int counter = 0;
    for(PtreeIter i = Ptree::Second(arglist); !i.Empty(); i++) {
      tmpvar = Ptree::Make("%p->replace(%d, %p)", tmpvar, counter, TranslateExpression(env, *i));
      counter++;
      if (!i.Empty()) {
        i++;

      }
    }

    Ptree *arguments = Ptree::Make("Slot %p = new Tuple(%d);\n", tmpvar2, counter);
    InsertDeclaration(env, arguments );

    // retrieve the method
    //    Ptree *method = TranslateMemberRead( env, object, op, member );

    // do the call

    Ptree *ret = Ptree::Make("\n%p->receive( new Message( context, %p, \"%p\", %p ), context )\n", object, object, member, tmpvar );
    return ret;
}

Ptree* SlotClass::TranslateMemberRead(Environment* env, Ptree* object,
			   Ptree* op, Ptree* member)
{
  // if it's not being accessed by '.', handle it as normal
  if (!op->Eq('.'))
    return Class::TranslateMemberRead(env, object, op, member);

  member = TranslateExpression(env, member);
  object = TranslateExpression(env, object);

  
  // if it's something actually defined by Slot, handle it as normal
  if (LookupMember(member))
    return Class::TranslateMemberRead(env, object, op, member);

  // now it's something that isn't on Slot, so write the code to look
  // it up

  // now slice by an IVKey.
  return Ptree::Make("%p->slice( new IVKey( \"%p\", ((Closure*)(Data*)context->_closure)->_definer ) )[2]\n", object, member );
}

Ptree* SlotClass::TranslateMemberWrite(Environment* env, Ptree* object,
            Ptree* op, Ptree* member, Ptree* assign_op, Ptree* expr )
{
  // if it's not being accessed by '.', handle it as normal
  if (!op->Eq('.'))
    return Class::TranslateMemberWrite(env, object, op, member, assign_op, expr);

  expr = TranslateExpression( env, expr );
  
  // if it's something actually defined by Slot, handle it as normal
  if (LookupMember(member)) {
    return Class::TranslateMemberWrite(env, object, op, member, assign_op, expr);
  }

  // we merely get the member name and use ->insert for it

  // find out its type.  if it's a function, create a Closure for it, and we add it different
  TypeInfo t;
  Ptree *x = Class::TranslateExpression ( env, expr, t );
  if ( t.IsFunction() )
     return Ptree::Make("%p->insert(\"%p\", new Closure( %p, %p ) )", object, member, object, x );
  
  // it's not a method, so its name is actually ( Name, Definer ), where Definer is got
  // from our closure

  return Ptree::Make("%p->insert( new IVKey( \"%p\", ((Closure*)(Data*)context->_closure)->_definer ), %p )\n", object, member, x );
}

/// Local Variables:
/// mode:c++
/// mode:font-lock
/// End:
