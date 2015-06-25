// OrderedSet - Coldstore Sets common functionality
// Copyright 2001 Matthew Toseland
// See LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

//#define DEBUGLOG
#include "OrderedSet.hh"
#include "List.hh"
#include "Error.hh"
#include "Slot.hh"

#include "tSlot.th"
template union tSlot<OrderedSet>;

Slot OrderedSet::factor(const OrderedSet *l) const
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

void OrderedSet::factor(const OrderedSet* pl, Slot& lme, Slot& lboth, Slot& lhim) const
{
  if(!length())
    {
      if(lhim) lhim=lhim->concat(pl->toSequence());
    }
  else if(!pl->length())
    {
      if(lme) lme=lme->concat(toSequence());
    }
  else
    {
      Slot i = iterator();
      Slot j = pl->iterator();
      Slot s = i->Next();
      Slot p = j->Next();
      while(true)
	{
	  DEBLOG(cout << "Entering OrderedSet::factor::while\n");
	  DEBLOG(cout << "s = ");
	  DEBLOG(s->dump(cout));
	  DEBLOG(cout << "\np = ");
	  DEBLOG(p->dump(cout));
	  DEBLOG(cout << "\n");
	  int x = s->order(p);
	  if(x<0) x = -1; else if (x>0) x = 1;
	  switch(x)
	    {
	    case 0:
	      {
		// put on [1], advance both
		if(lboth) lboth=lboth->insert(s);
		if(i->More())
		  s = i->Next();
		else s = (Data*)NULL;
		if(j->More())
		  p = j->Next();
		else p = (Data*)NULL;
		break;
	      }
	    case -1:
	      {
		// put on [0], advance lower
		if(lme) lme=lme->insert(s);
		if(i->More())
		  s = i->Next();
		else s = (Data*)NULL;
		break;
	      }
	    case 1:
	      {
		// put on [2], advance lower i.e. p
		if(lhim) lhim=lhim->insert(p);
		if(j->More())
		  p = j -> Next();
		else p = (Data*)NULL;
		break;
	      }
	    default:
	      throw new Error("set",this,"Fatal order consistency error in Set::factor");
	    }
	  if(p && (!s))
	    {
	      // put rest of target on [2]
	      if(lhim)
		{
		  while(j->More())
		    {
		      DEBLOG(cout << "Inserting p = ");
		      DEBLOG(p.Dump(cout));
		      DEBLOG(cout << '\n');
		      lhim = lhim->insert(p);
		      p=j->Next();
		    }
		  lhim=lhim->insert(p);
		}
	      return;
	    }
	  if(s && !p)
	    {
	      // put rest of me on [0]
	      if(lme)
		{
		  while(i->More())
		    {
		      DEBLOG(cout << "Inserting s = ");
		      DEBLOG(s.Dump(cout));
		      DEBLOG(cout << '\n');
		      lme = lme->insert(s);
		      s=i->Next();
		    }
		  lme = lme->insert(s);
		}
	      return;
	    }
	}
    }
  return;
}

int OrderedSet::order(const Slot& arg) const
{
  if(!AKO(arg,OrderedSet))
    {
      return typeOrder(arg);
    }
  else
    {
      if(length() > arg->length())
	{
	  return 1;
	}
      else if(arg->length() > length())
	{
	  return -1;
	}
      else
	{
	  if(!length()) return 0;
	  Slot s = iterator();
	  Slot p = arg->iterator();
	  for(;s->More();)
	    {
	      int x = (s->Next())->order(p->Next());
	      if(x) return x;
	    }
	  return 0;
	}
    }
}

bool OrderedSet::equal(const Slot& arg) const
{
  if(!AKO(arg,OrderedSet))
    {
      return false;
    }
  else
    {
      if(length() != arg->length())
	{
	  return false;
	}
      else
	{
	  if(!length()) return true;
	  Slot s = iterator();
	  Slot p = arg->iterator();
	  for(;s->More();)
	    {
	      int x = (s->Next())->order(p->Next());
	      if(x) return false;
	    }
	  return true;
	}
    }
}
