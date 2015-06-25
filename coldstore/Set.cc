// Set - Coldstore Sets common functionality
// Copyright 2001 Matthew Toseland
// See LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include "Set.hh"
#include "List.hh"
#include "Error.hh"
#include "Symbol.hh" // for inversion
#include "VectorSet.hh"

#include "tSlot.th"
template union tSlot<Set>;

Data* Set::mutate(void* where) const
{
   Data *result;
    if (refcount() > 1) {
        result = clone(where);
        DEBLOG(cerr << "mutate: clone "
               << "from " << this
               << " to " << result << '\n');
    } else {
        result = (Data*)this;
        DEBLOG(cerr << "mutate: dup " << this);
    }
    assert(length() == result->length());	// catch a bonehead bug
    
    return result;
}

bool Set::truth() const
{
  return length()>0;
}

Slot Set::factor(const List *l) const
{
  return factor(new VectorSet(l));
};

Slot Set::factor(const Set *l) const
{
  Slot f = new List; // FIXME: should be Tuple
  Slot d = (Data*)NULL;
  Slot f0 = create(d);
  Slot f1 = create(d);
  Slot f2 = create(d);
  factor(l,f0,f1,f2);
  f->insert(f0);
  f->insert(f1);
  f->insert(f2);
  return f;
}

void Set::factor(const Set* pl, Slot& lme, Slot& lboth, Slot& lhim) const
{
  ((VectorSet*)toVectorSet())->factor((pl->toVectorSet()),lme,lboth,lhim);
}

Slot Set::toVectorSet() const
{
  return this;
}

Slot Set::modulo(const Slot &s) const
{
  if(AKO(s,Set)) {return factor((Set*)(Data*)s);}
  else if(AKO(s,List)) {return factor((List*)s);};
  throw new Error("set",s,"factoring a bad type");
  return (Data*)NULL;
}

Slot Set::and(const Slot& arg) const
{
  if(AKO(arg,Set))
    {
      Slot d = (Data*)NULL;
      Slot n = d;
      Slot p = create(n);
      factor((Set*)(Data*)arg,n,p,n);
      return p;
    }
  else if(AKO(arg,List))
    {
      Slot s = create(arg);
      return Set::and(s);
    }
  else 
    {
      throw new Error("set",arg,"invalid argument to and");
    }
}

Slot Set::or(const Slot& arg) const
{
  if(AKO(arg,Set))
     {
       Slot d = (Data*)NULL;
       Slot p = create(d);
       // p is mutable because p is new
       factor((Set*)(Data*)arg,p,p,p);
       return p;
     }
  else if(AKO(arg,List))
    {
      Slot s = create(arg);
      return or(s);
      // FIXME: alternative implementation: concat and sortuniq
    }
  else throw new Error("set",arg,"invalid argument to or");
}

Slot Set::xor(const Slot& arg) const
{
  if(AKO(arg,Set))
    {
      Slot d = (Data*)NULL;
      Slot n = d;
      Slot p = create((Data*)NULL);
      factor((Set*)(Data*)arg,p,n,p);
      return p;
    }
  else if(AKO(arg,List))
    {
      Slot s = create(arg);
      return xor(s);
      // can't concat and sortuniq because can have dupes within the List, must eliminate BEFORE xor'ing
    }
  else throw new Error("set",arg,"invalid argument to xor");
}

int Set::order(const Slot& arg) const
{
  if(!AKO(arg,Set))
    {
      return typeOrder(arg);
    }
  else return ((Set*)toVectorSet())->order(((Set*)arg)->toVectorSet());
}

bool Set::equal(const Slot& arg) const
{
  if(!AKO(arg,Set))
    {
      return false;
    }
  else return toVectorSet()->equal(((Set*)arg)->toVectorSet());
}

Slot Set::invert() const
{
  Slot ns = new Namespace();
  Slot it = iterator();
  while (it->More()) {
    Slot el = it->Next();
    ns->insert(el->slice(1), el->slice(0));
  }
  return ns;
}

