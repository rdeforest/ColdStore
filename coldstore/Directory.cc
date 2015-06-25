// Directory
//#define DEBUGLOG
#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Dict.hh"
#include "List.hh"
#include "Symbol.hh"
#include "Slot.hh"
#include "Data.hh"
#include "NamedNamespace.hh"
#include "Directory.hh"
#include "Pool.th"

template class Pool<String>;
template class gPool<String>;
template class Pooled<String>;
template class Q< gPool<String> >;
template class Qh< gPool<String> >;

#include "tSlot.th"
template union tSlot<Directory>;

Directory::Directory()
{
  throw new Error("Directory",(Data*)NULL,"Directory cannot be created empty");
}

Directory::Directory(Directory& d)
{
}

Directory::Directory(const Slot& name, const Data* parent)
  : NamedNamespace(name,parent?parent:(Namespace*)this)
{
}

Directory::Directory(const Slot& s)
  : NamedNamespace(s)
{
}

Directory::~Directory()
{
};

Slot Directory::search(const Slot &key) const
{
  DEBLOG(cerr << "searching for ");
  DEBLOG(key->dump(cerr));
  Slot s = NamedNamespace::search(key);
  if(s) return s;
  Slot lp = NamedNamespace::lshift(key);
  if(lp)
    {
      Slot lpn = ((Name*)lp) -> name();
      Slot kn; // Namespace::search will accept either, so we must too
      if(AKO(key,String)) {kn = key;}
      else if(AKO(key,Name)) {kn = ((Name*)key)->name();}
      else { throw new Error("search",key,"not a String or a Name");}
      if(lp && isPrefix(lpn,kn))
	{
	  if(AKO(lp,NamedNamespace))
	    {
	      return ((NamedNamespace*)(Name*)lp) -> search(killPrefix(lpn,kn));
	    }
	}
    }
  return (Data*)0;
}

ostream& Directory::dump(ostream& out) const
{
  return NamedNamespace::dump(out);
}

Slot Directory::insert(const Slot& key, const Slot& value)
{
  // this is long term storage; anything that actually goes in the tree should *not* be Pooled; everything else should be
  // NamedNamespaces don't reallocate Pooled<String>
  Slot s = search(key);
  if(AKO(key,NamedNamespace))
    {
      if(AKO(key,Directory))
	{
	}
      else
	{
	  throw new Error("error",key,"inserting a non-Directory Namespace into a Directory");
	  return (Data*)NULL;
	}
    }
  if (s)
    {
      if(AKO(s,NamedNamespace))
	{
	  Slot n = "";
	  if(value)
            return s -> insert(n, value);
	  else
            return s -> insert(n);
	}
      else
	{
	  Slot n = key;
	  n = Pooled<String>::unPool(n,this);
	  if(value)
	    return Namespace::insert(key,value);
	  else 
	    return Namespace::insert(key);
	}
    };
  Slot lp = NamedNamespace::lshift(key);
  if(lp)
    {
      Slot lpn = ((Name*)lp) -> name();
      DEBLOG(cerr << "Found ");
      DEBLOG(lpn.Dump(cerr));
      Slot y = commonPrefix(lpn,key);
      if(y == lpn) // only the prior item can be a prefix
	{
	  if(AKO(lp,NamedNamespace))
	    {
	      Directory* p = (Directory*)(NamedNamespace*)(Name*)(Data*)lp;
	      DEBLOG(cerr << "inserting symbol " << key << "\n");
	      Slot s = killPrefix(lpn,key);
	      return p -> insert(s,value);
	    }
	}
      if(y && y->length())
	{
	  Slot s = killPrefix(y,key);
	  Slot space = insertPrefix(y);
	  DEBLOG(dump(cerr));
	  Slot sym = space -> insert(s,value);
	  DEBLOG(dump(cerr));
	  DEBLOG(sym -> dump(cerr));
	  return sym;
	}
    }
  else
    {
      lp = NamedNamespace::rshift(key);
      if(lp)
	{
	  Slot lpn = ((Name*)lp)->name();
	  DEBLOG(cerr << "Found ");
	  DEBLOG(lpn.Dump(cerr));
	  Slot y = commonPrefix(lpn,key);
	  if(y && y->length())
	    {
	      Slot s = killPrefix(y,key);
	      DEBLOG(cerr << "inserting symbol " << s << ", " << key << "\n");
	      return insertPrefix(y) -> insert(s,value);
	    }
	}
    }
  Slot n = key;
  n = Pooled<String>::unPool(n,this);
  return value ? NamedNamespace::insert(n, value) : NamedNamespace::insert(n);
}

Slot Directory::insert(const Slot& key)
{
  return Directory::insert(key,(Data*)NULL);
}

Slot Directory::replace(const Slot& key)
{
  return Directory::replace(key,(Data*)NULL);
}

Slot Directory::replace(const Slot& key, const Slot& value)
{
  Slot s = search(key);
  if(AKO(key,NamedNamespace))
    {
      if(AKO(key,Directory))
	{
	}
      else
	{
	  throw new Error("error",key,"replacing a non-Directory Namespace into a Directory");
	  return (Data*)NULL;
	}
    }
  if (s)
    {
      if(AKO(s,NamedNamespace))
	{
	  // this can't happen in the default Directory (without deletions)
	  // because the bottom level is always leaves
	  Slot n = "";
	  if(value)
	    return s -> replace(n, value);
	  else
	    return s -> replace(n);
	}
      else
	{
	  if(value)
	    return Namespace::replace(key,value);
	  else 
	    return Namespace::replace(key);
	}
    };
  Slot lp = NamedNamespace::lshift(key);
  if(lp)
    {
      Slot lpn = ((Name*)lp) -> name();
      if(isPrefix(lpn,key))
	{
	  if(AKO(lp,NamedNamespace))
	    {
	      if(value)
		return ((Directory*)(NamedNamespace*)(Name*)lp) -> replace(killPrefix(lpn,key),value);
	      else
		return ((Directory*)(NamedNamespace*)(Name*)lp) -> replace(killPrefix(lpn,key));
	    }
	}
    }
  return (Data*)0;
}

Slot Directory::del(const Slot& key)
{
  Slot s = Namespace::search(key);
  if (s)
    {
      Namespace::del(key);
      return (Data*)(Namespace*)this;
    };
  Slot lp = NamedNamespace::lshift(key);
  if(lp)
    {
      Slot lpn = ((Name*)lp) -> name();
      if(isPrefix(lpn,key))
	{
	  if(AKO(lp,NamedNamespace))
	    {
	      ((Namespace*)(NamedNamespace*)(Name*)lp) -> del(killPrefix(lpn,key));
	    }
	}
    }
  return (Namespace*)this;
}

int Directory::length() const
{
  int l = 0;
  Slot s;
  for(Slot i = ((Directory*)this)->Namespace::iterator();i->More();)
    {
      s = i -> Next();
      if(AKO((Name*)s,NamedNamespace))
	{
	  l += ((Namespace*)(NamedNamespace*)(Name*)(Data*)s) -> length();
	}
      else
	{
	  l++;
	}
    }
  return l;
}

List* Directory::toSequence(const Slot& prefix) const
{
  List* l = new List;
  for(Slot i = ((Directory*)this)->Namespace::iterator();i -> More();)
    {
      Slot s = i -> Next();
      if(AKO((Name*)s,NamedNamespace))
	{
	  Directory* d = (Directory*)(NamedNamespace*)(Name*)(Data*)s;
	  l -> concat(d -> toSequence(prefix -> concat(((Name*)s) -> name())));
	}
      else
	{
	  l -> add(prefix -> concat(((Name*)s) -> name()));
	  l -> add(s);
	}
    }
  return l;
}

Slot Directory::concat(const Slot& arg, const Slot& prefix) const
{
  Directory* p = dynamic_cast<Directory*>(mutate());
  if(!AKO((Namespace*)(Data*)arg,Directory)) { return unimpl("Directory concat with non-Directory"); }
  for(Slot i = ((Directory*)arg)->Namespace::iterator();i -> More();)
    {
      Slot s = i -> Next();
      if(AKO((Name*)s,NamedNamespace))
	{
	  Directory* d = (Directory*)(NamedNamespace*)(Name*)(Data*)s;
	  concat((Namespace*)d, prefix -> concat(((Name*)s) -> name()));
	}
      else
	{
	  if(AKO((Name*)s,Symbol))
	    {
	      p -> insert(prefix -> concat(((Name*)s) -> name()),((Symbol*)s) -> value());
	    }
	  else
	    {
	      return unimpl("Directory concat including non-Symbol leafs");
	    }
	}
    }
  return (Namespace*)p;
}

bool Directory::isPrefix(const Slot& prefix, const Slot& name) const
{
  if(!AKO(prefix,String))
    {
      throw new Error("Directory",prefix,"not a string");
    }
  if(!AKO(name,String))
    {
      throw new Error("Directory",name,"not a string");
    }
  DEBLOG(cerr << "isPrefix(" << prefix->dump(cerr) << "," << name->dump(cerr) << ".\n");
  return ((String*)prefix)->isPrefix(name);
}

Slot Directory::killPrefix(const Slot& prefix, const Slot& name) const
{
  if(!AKO(prefix,String))
    {
      throw new Error("Directory",prefix,"not a string");
    }
  if(!AKO(name,String))
    {
      throw new Error("Directory",name,"not a string");
    }
  DEBLOG(cerr << "killPrefix(");
  DEBLOG(prefix->dump(cerr));
  DEBLOG(cerr << ", ");
  DEBLOG(name->dump(cerr));
  DEBLOG(cerr << ")\n");
  int x = prefix->length();
  Slot xs(x);
  Slot xl(name->length()-x);
  MAKE_TEMP_LIST(l,3,(Data*)name,(Data*)xs,(Data*)xl);
  Slot s = (gPool<String>::Get(l));
  DEBLOG(s->dump(cerr));
  DEBLOG(cerr << "\n");
  return s;
}

Slot Directory::prependPrefix(const Slot& suffix) const
{
  if(suffix)
    return name() -> concat(suffix); else return name();
}

Slot Directory::insertPrefix(const Slot& prefix)
{
  DEBLOG(prefix->dump(cerr));
  DEBLOG(cerr << " inserting new prefix " << prefix << "\n");
  // strictly a local operation - within *our* namespace
  // massively thread-unsafe
  Slot move = NamedNamespace::search(prefix);
  // special case: inserting a NamedNamespace called "joe" when we already have a Symbol called "joe"
  if(move && (((Name*)move)->name() == prefix))
    {
      move -> upcount();
      ((Name*)move) -> changeSpace(NULL);
    }
  else {move = NamedNamespace::rshift(prefix);};
  DEBLOG(cerr << "move: ");
  if(move)
     {
  	DEBLOG(move -> dump(cerr));
     }
  Slot s = createSubspace(prefix);
  DEBLOG(cerr << "subspace: ");
  DEBLOG(s -> dump(cerr));
  Slot name;
  //DEBLOG(dump(cerr));
  if(!move) return s;
  for(;move && isPrefix(prefix,name = ((Name*)move)->name());move = NamedNamespace::rshift(name))
    {
      DEBLOG(cerr << "Moving " << name << " into " << prefix << ".\n");
      Slot n = killPrefix(prefix,name);
      DEBLOG(cerr << "New name: " << n << ".\n");
      ((Name*)move) -> changeSpace(NULL);
      if(AKO((String*)n,Pooled<String>))
	n = new (this) String ((String*)n);
      n = Pooled<String>::unPool(n,this);
      ((Name*)move) -> changeName(n);
      ((Name*)move) -> changeSpace(s);
      //DEBLOG(dump(cerr));
    };
  //DEBLOG(dump(cerr));
  DEBLOG(cerr << " inserted new prefix " << prefix << "\n");
  return s;
}

Slot Directory::createSubspace(const Slot& subspace)
{
  Slot s = subspace;
  s = Pooled<String>::unPool(s,this);
  Slot p = (Name*)(new (this) Directory(s,(Namespace*)this));
  Namespace::insert(p);
  DEBLOG(p -> dump(cerr));
  p = (Namespace*)(NamedNamespace*)(Name*)p;
  DEBLOG(p -> dump(cerr));
  return p;
}

Slot Directory::commonPrefix(const Slot& name1, const Slot& name2) const
{
  if(!AKO(name1,String))
    {
      throw new Error("Directory",name1,"not a string");
    }
  if(!AKO(name2,String))
    {
      throw new Error("Directory",name2,"not a string");
    }
  DEBLOG(cerr << "commonPrefix(");
  DEBLOG(name1->dump(cerr));
  DEBLOG(cerr << ",");
  DEBLOG(name2->dump(cerr));
  DEBLOG(cerr << ".\n");
  String* p = (String*)name1;
  int x = p -> lenCommonPrefix(name2);
  Slot xs(x);
  Slot x0(0);
  MAKE_TEMP_LIST(a,3,(Data*)name1,(Data*)x0,(Data*)xs);
  Slot s = (gPool<String>::Get(a));
  DEBLOG(cerr << "commonPrefix returning ");
  DEBLOG(s->dump(cerr));
  DEBLOG(cerr << "\n");
  return s;
}

Slot Directory::lshift(const Slot& arg) const
{
  // arg can be a String or a Name
  DEBLOG(cerr << "Directory::lshift " << this << " " << arg << "\n");
  if(!arg || AKO(arg,Name))
    {
      const Name* p = (Name*)arg;
      tSlot<NamedNamespace> ps = p ? (const Namespace*)(p -> space()) : this;
      const Name* o = (Name*)NULL;
l1:   o = (Name*)(ps -> Namespace::lshift(p));
      DEBLOG(cerr << "got ");
      if(o) DEBLOG(o -> dump(cerr));
      DEBLOG(cerr << "\n");
      if(!o)
	{
	  if((const Namespace*)(ps -> space()) == ps || !(ps -> space())) return (Data*)NULL;
	  if(!AKO((Namespace*)(ps->space()),NamedNamespace)) return (Data*)NULL;
	  p = ps;
	  ps = toType(ps -> space(), NamedNamespace);
	  goto l1;
	}
      if(AKO(o,const NamedNamespace))
	{
	  return ((NamedNamespace*)o) -> lshift((Data*)NULL);
	}
      return o;
    }
  else if(AKO(arg,String))
    {
      const Name* p = search(arg);
      if(p)
	return lshift(p);
      else
	{
	  Slot prefix = NamedNamespace::lshift(arg);
	  if(prefix && isPrefix(prefix,arg))
	    {
	      return ((const NamedNamespace*)(Name*)prefix)->lshift(killPrefix(((Name*)prefix)->name(),arg));
	    }
	}
      return (Data*)NULL;
    }
  else throw new Error("directory",arg,"not a string or a name");
}

Slot Directory::rshift(const Slot& arg) const
{
  // arg can be a String or a Name
  DEBLOG(cerr << "Directory::rshift " << this << " " << arg << "\n");
  if(!arg || AKO(arg,Name))
    {
      const Name* p = (Name*)arg;
      tSlot<NamedNamespace> ps = p ? (const Namespace*)(p -> space()) : this;
      const Name* o = (Name*)NULL;
l1:   o = (Name*)(ps -> Namespace::rshift(p));
      DEBLOG(cerr << "got ");
      if(o) DEBLOG(o -> dump(cerr));
      DEBLOG(cerr << "\n");
      if(!o)
	{
	  if((const Namespace*)(ps -> space()) == ps || !(ps -> space())) return (Data*)NULL;
	  if(!AKO((Namespace*)(ps->space()),NamedNamespace)) return (Data*)NULL;
	  p = ps;
	  ps = toType(ps -> space(), NamedNamespace);
	  goto l1;
	}
      if(AKO(o,const NamedNamespace))
	{
	  return ((NamedNamespace*)o) -> rshift((Data*)NULL);
	}
      return o;
    }
  else if(AKO(arg,String))
    {
      const Name* p = search(arg);
      if(p)
	return rshift(p);
      else
	{
	  Slot prefix = NamedNamespace::rshift(arg);
	  if(prefix && isPrefix(prefix,arg))
	    {
	      return ((const NamedNamespace*)(Name*)prefix)->rshift(killPrefix(((Name*)prefix)->name(),arg));
	    }
	}
      return (Data*)NULL;
    }
  else throw new Error("directory",arg,"not a string or a name");
}

Slot Directory::modulo(const Slot& arg) const
{
  if(!AKO(arg,Name)) throw new Error("directory",arg,"modulo a non-Name");
  return ((Name*)arg) -> dirName(this);
}
