// VectorSet - Coldstore Sets, implemented using sorted vectors
// Copyright 2000 Matthew Toseland
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

//#define DEBUGLOG
#include <values.h>
#include "List.hh"
#include "Error.hh"
#include "TupleBase.hh"
#include "VectorSet.hh"

template union tSlot<VectorSet>;

ostream& operator<< (ostream&, const _TupleBase<Slot> &);

// Uses the copy constructor
Data *VectorSet::clone(void *where) const
{
  return new (where) VectorSet((Vector<Slot>*)this, 0, length());
}

VectorSet::VectorSet(unsigned int size)
  : Vector<Slot>(size)
{
}

VectorSet::VectorSet(unsigned int size, ...)
  : Vector<Slot>(size)
{
  if (size<0) size=-size;
  va_list ap;
  va_start(ap, size);
  if(size>MAXINT) throw new Error("set",this,"impossible size");
  for (int i = 0; i < (int)size; i++) {
    Slot datum = va_arg(ap, Slot);
    this->Vector<Slot>::vconcat(&datum, 1);
  }
  va_end(ap);
  /*DEBLOG(cerr << "Sized VectorSet construction and init " << Length()
	 << ' ' << size
	 << ' ' << *this
	 << '\n');*/
  Vector<Slot>::toset();
}

VectorSet::VectorSet(const Slot &sequence)
  : Vector<Slot>((Data*)sequence?(unsigned int)sequence->length():0)
{
  if((Data*)sequence)
    {
      int x = sequence -> length();
      for(int i=0;i<x;i++)
	{
	  Slot s = sequence[i];
	  this->Vector<Slot>::vconcat(&s, 1);
	}
      Vector<Slot>::toset();
    }
}

VectorSet::VectorSet(const List* contentT, int start, int l)
  : Vector<Slot>((Vector<Slot>*)contentT,start,l)
{
  Vector<Slot>::toset();
}

VectorSet::VectorSet(const List& contentT, int start, int l)
  : Vector<Slot>((Vector<Slot>*)&contentT,start,l)
{
  Vector<Slot>::toset();
}

VectorSet::VectorSet(const Tuple* contentT, int start, int l)
  : Vector<Slot>((TupleBase<Slot>*)contentT,start,l)
{
  Vector<Slot>::toset();
}

VectorSet::VectorSet(const VectorSet& contentT)
  : Vector<Slot>((Vector<Slot>*)&contentT)
{
}

VectorSet::VectorSet(const VectorSet* contentT)
  : Vector<Slot>((Vector<Slot>*)contentT)
{
}

VectorSet::VectorSet(const VectorSet& contentT, Slot start, Slot end)
  : Vector<Slot>((Vector<Slot>*)(VectorSet*)contentT.slice(start,end))
{
}

VectorSet::VectorSet(const VectorSet* contentT, Slot start, Slot end)
  : Vector<Slot>((Vector<Slot>*)(VectorSet*)contentT->slice(start,end))
{
}

VectorSet::VectorSet(TupleBase<Slot> *contentT)
  : Vector<Slot>(contentT)
{
  Vector<Slot>::toset();
}

VectorSet::VectorSet(TupleBase<Slot> *contentT, unsigned int size, ...)
  : Vector<Slot>(contentT)
{
  if (size<0) size=-size;
  va_list ap;
  va_start(ap, size);
  if(size>MAXINT) throw new Error("set",this,"impossible size");
  for (int i = 0; i < (int)size; i++) {
    Slot datum = va_arg(ap, Slot);
    this->Vector<Slot>::vconcat(&datum, 1);
  }
  va_end(ap);
  /*DEBLOG(cerr << "Sized VectorSet construction and init " << Length()
	 << ' ' << size
	 << ' ' << *this
	 << '\n');*/
  Vector<Slot>::toset();
}

VectorSet::~VectorSet()
{
}

void VectorSet::check(int x) const
{
  Slot s((Data*)NULL);
  Slot p((Data*)NULL);
  for(int i=0;i<length();i++)
    {
      p = multiply(i);
      if((i || p) && p <= s)
	{
	  assert(!"VectorSet sorting inconsistency");
	}
      s = p;
    }
  Vector<Slot>::check();
}

Slot VectorSet::concat(const Slot &arg) const
{
  /* if is a List, make into a VectorSet then tail recurse
   * if is a VectorSet,...
   *
   * allocate sizeof smaller extra on bigger->mutate
   * interlace smaller and larger into new allocated thing
   *
   * because of copies, loss on concat(4, 10k) isn't that much
   * the mutate'd stretched alloc may well be the same; we go from low end 
   * to high end so no problem
   */
  
  if(!length())
    {
      return arg;
    }
  else if(!arg->length())
    {
      return this;
    }
  else
    {
      if(!AKO(arg,Set))
	{
	  if(!AKO(arg,List))
	    {
	      throw new Error("set",arg,"concatenating a non-List non-Set");
	    }
	  else
	    {
	      return concat(new VectorSet((List*)arg));
	    }
	}
      if(!AKO(arg,VectorSet))
	{
	  return concat(((Set*)arg)->toVectorSet());
	}
      // We need new VectorSet List with length length()+arg.length()
      // FIXME: if small, insert one by one. How small ?
      const VectorSet* pl1 = this;
      const VectorSet* pl2 = (const VectorSet*)arg;
      if((pl1->length()) < (pl2->length()))
	{ const VectorSet* pp = pl1; pl1=pl2; pl2=pp; };
      VectorSet* pt;
      if((((VectorSet*)pl1)->Segment<Slot>::segFirst())-(&(*(((VectorSet*)pl1)->Vector<Slot>::Allocation())[0]))>=(pl2->length()))
	{
	  pt = (VectorSet*)pl1->mutate();
	  pt->Mutate();
	  pt->vinsert(0,NULL,pl2->length());
	}
      else
	{
	  pt = new VectorSet;
	  pt -> resize(pl2->length()+pl1->length());
	}
      int i = 0;
      int j = 0;
      int k = 0;
      while(i<(pl1->length()) && j < (pl2->length()))
	{
	  Slot s = pl1 -> element(i);
	  Slot p = pl2 -> element(j);
	  Slot& q = pt -> element(k);
	  int x = cmp(&s,&p);
	  if(x>0) x=1;
	  if(x<0) x=-1;
	  switch(x)
	    {
	    case 0:
	      {
		i++;
		j++;
		q = s;
		k++;
		break;
	      }
	    case -1:
	      {
		i++;
		q = s;
		k++;
		break;
	      }
	    case 1:
	      {
		j++;
		q = p;
		k++;
		break;
	      }
	    default:
	      throw new Error("set",this,"Fatal order consistency error in Set::factor");
	    }
	};
      if(i==pl1->length() && j!=pl2->length())
	{
	  // put rest of target on [2]
	  for(;j<(pl2->length());j++)
	    {
	      pt->element(k) = pl2->element(j);
	      k++;
	    }
	}
      else if(j==pl2->length())
	{
	  // put rest of me on [0]
	  for(;i<(pl1->length());i++)
	    {
	      pt->element(k) = pl1->element(i);
	      k++;
	    }
	}
      pt->resize(k);
      return pt;
    }
}

Slot VectorSet::insert(const Slot &val)
{
  // never use concat() because we are inserting the Slot
  // we do *not* dereference a Set, we just shove it in
  int x = isearch(val);
  if(x>-1) return this;
  x = (-x)-1;
  VectorSet* p = (VectorSet*)mutate();
  p -> Vector<Slot>::vinsert(x,&val,1);
  return p;
}

Slot VectorSet::lshift(const Slot &arg) const
{ // return [y] : [y] < x, max y
  if(!length()) return (Data*)NULL;
  int x = isearch(arg);
  if(x==0)
    {
      return (Data*)NULL;
    }
  else if(x>0)
    {
      // exact match
      return element(x-1);
    }
  else
    {
      x = -x;
      x--;
      // insert before x
      x--;
      if(x<0) return (Data*)NULL;
      return element(x);
    }
}

Slot VectorSet::rshift(const Slot &arg) const
{ // return [y] : [y] > x, min y
  if(!length()) return (Data*)NULL;
  int x = isearch(arg);
  if(x==(length()-1))
    {
      return (Data*)NULL;
    }
  else if(x>-1)
    {
      // exact match
      return element(x+1);
    }
  else
    {
      x = -x;
      x--;
      // insert before x
      if(x==length())
	return (Data*)NULL;
      return element(x);
    }
}

Slot VectorSet::slice(const Slot& start, const Slot& end) const
{
  if(!length()) return new VectorSet;
  if(end <= start) return new VectorSet;
  int x = isearch(start);
  int y = isearch(end);
  // from start to end, including ends and everything in between
  if(x==(length()-1))
    {
      return new VectorSet;
    }
  else if(x>-1)
    {
      // exact match
    }
  else
    {
      x=(-x)-1;
    }
  if(y==0 || y==-1)
    {
      return new VectorSet;
    }
  else if(y>-1)
    {
    }
  else
    {
      y=(-y)-2;
    }
  return new VectorSet((Vector<Slot>*)this,x,y-x+1);
}

Slot VectorSet::search(const Slot& find) const
{
  DEBLOG(cerr << "Searching for: ");
  DEBLOG(find->dump(cerr) << '\n');
  int x = isearch(find);
  DEBLOG(cerr << "Found at index " << x << "\n");
  if(x>-1) return element(x);
  return (Data*)NULL;
}

Slot VectorSet::slice(const Slot& find) const
{
  int x = isearch(find);
  if(x>-1) return element(x);
  throw new Error("VectorSet",this,"not found");
}

Slot VectorSet::replace(const Slot& from, const Slot& val)
{
  if(slice(from))
    {
      return del(from) -> insert(val);
    }
  return this;
}

Slot VectorSet::del(const Slot& from)
{
  int x = isearch(from);
  if(x>-1)
    {
      VectorSet* p = (VectorSet*)mutate();
      p -> Vector<Slot>::del(x,1);
      return p;
    }
  else
    {
      throw new Error("set",this,"deletion out of range");
      return (Data*)NULL;
    }
}

Slot VectorSet::multiply(const Slot& arg) const
{
  int x = (int)arg;
  if(x<0 || !length()) return (Data*)NULL;
  if(x>=length()) return (Data*)NULL;
  return element(x);
}

Slot VectorSet::divide(const Slot& arg) const
{
  int x = isearch(arg);
  if(x>=0) return x;
  return -1-x;
}

Slot VectorSet::iterator() const
{
  return clone();
}

bool VectorSet::More() const
{
  return (length()>0);
}

Slot VectorSet::Next()
{
  if (truth()) {
      Slot retval = element(0);
    (*this)++;	// advance lower edge of Segment
    return retval;
  } else {
    throw new Error("break", this, "Iterator exhaustion");
  }
}

bool VectorSet::isIterator() const
{
  return true;
}

int VectorSet::isearch(const Slot& arg) const
{
  if(length()<8)
    {
      int i;
      if(length()==0) return -1;
      for(i=0;i<length();i++)
	{
	  Slot s = element(i);
	  DEBLOG(cerr << " isearch comparing [" << i << "]: ");
	  DEBLOG(s.Dump(cerr) << " with ");
	  DEBLOG(arg.Dump(cerr) << '\n');
	  if(s==arg) return i;
	  if(s>arg) return -(i+1);
	}
      return -(i+1);
    }
  else
    {
      int idx;
      bool b = tsearch(arg,idx);
      if(b) return idx;
      // [idx] < arg
      if(element(idx) < arg) return -(idx+2);
      else return -(idx+1);
    }
}

List* VectorSet::toSequence() const
{
  return new List((const Vector<Slot>*)this);
}

Slot VectorSet::toconstruct() const
{
  return toSequence();
}

Slot VectorSet::toVectorSet() const
{
  return this;
}

int VectorSet::order(const Slot& arg) const
{
  if(!AKO(arg,Set))
    {
      return typeOrder(arg);
    }
  else
    {
      if(AKO(arg,VectorSet))
	return Segment<Slot>::order(*(Segment<Slot>*)(VectorSet*)arg);
      else
	return order(((Set*)arg)->toVectorSet());
    }
}

bool VectorSet::equal(const Slot& arg) const
{
  if(!AKO(arg,Set))
    {
      return false;
    }
  else
    {
      if(AKO(arg,VectorSet))
	return Segment<Slot>::order(*(Segment<Slot>*)(VectorSet*)arg)==0;
      else
	return equal(((Set*)arg)->toVectorSet());
    }
}

Slot VectorSet::create(const Slot& arg) const
{
  return construct(arg);
}

Slot VectorSet::construct(const Slot& arg)
{
  return new VectorSet(arg);
}
