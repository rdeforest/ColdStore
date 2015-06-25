#include "Tuple.hh"
#include "Error.hh"
#include "NamedNamespace.hh"
#include "List.hh"

#include "tSlot.th"
template union tSlot<NamedNamespace>;

NamedNamespace::NamedNamespace()
{
  throw new Error("illegal","","Name without a Namespace");
}

NamedNamespace::NamedNamespace(NamedNamespace& n)
{
  Slot s((Namespace*)&n);
  throw new Error("illegal",s,"copy construction of a NamedNamespace");
}

NamedNamespace::NamedNamespace(const Slot& list)
  : Name(list[0],list[1])/*, 
			   Namespace(list[2])*/
{
}

NamedNamespace::NamedNamespace(const Slot& name, const Data* nspace)
  : Name(name,nspace)
{
}

NamedNamespace::NamedNamespace(const Slot& name, const Data* nspace, const Slot& init)
  : Name(name,nspace),
    Namespace(init)
{
}

NamedNamespace::~NamedNamespace()
{
  DEBLOG(cerr << "CALLED ~NamedNamespace\n");
  
}

Slot NamedNamespace::toconstruct() const
{
  Slot t = new (2) Tuple(-2,(Data*)Name::toconstruct(),(Data*)Namespace::toconstruct());
  return t;
}

Data* NamedNamespace::clone(void*where) const
{
  throw new Error("unimpl",(Data*)(Name*)this,"An attempt was made to duplicate a NamedNamespace");
  return NULL;
} // dont

Slot NamedNamespace::replace(const Slot & s1, const Slot & s2, const Slot & s3)
{
  throw new Error("unimpl",(Data*)(Namespace*)this,"replace on NamedNamespace");
  return Slot((Data*)NULL);
}

Slot NamedNamespace::replace(const Slot &s1, const Slot &s2)
{
  throw new Error("unimpl",(Data*)(Namespace*)this,"replace on NamedNamespace");
  return Slot((Data*)NULL);
}

Slot NamedNamespace::replace(const Slot & s)
{
  throw new Error("unimpl",(Data*)(Namespace*)this,"replace on NamedNamespace");
  return Slot((Data*)NULL);
}

ostream& NamedNamespace::dump(ostream& o) const
{
  ostream & o1 = Name::dump(o);
  return Namespace::dump(o1);
}

RefCount *NamedNamespace::upcount( void) const
{
  return Name::upcount();
}

void NamedNamespace::dncount( void) const
{
  Name::dncount();
}

Slot NamedNamespace::unimpl(const char* s) const
{
  return Namespace::unimpl(s);
}
