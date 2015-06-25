// Symbol.cc - global symbols
// Copyright (C) 1998,1999 Colin McCormack
// Copyright (C) 2000,2001 Matthew Toseland
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))= "$Id: Symbol.cc,v 1.60 2002/04/10 02:34:09 coldstore Exp $";
//#define DEBUGLOG
#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Dict.hh"
#include "List.hh"
#include "Symbol.hh"
#include "NamedNamespace.hh"
#include "Directory.hh"
#include "VectorSet.hh"

#include "tSlot.th"
template union tSlot<Symbol>;
template union tSlot<Namespace>;

Name::Name()
  : Tuple(2),
    _namespace((Data*)0),
    _name((Data*)0)
{
    DEBLOG(dump(cerr) << " New Name\n");
}

Name::Name(unsigned short len)
  : Tuple(len),
    _namespace((Data*)0),
    _name((Data*)0)
{
    DEBLOG(dump(cerr) << " New Name\n");
}

Symbol::Symbol()
  : Name(3),
    _value((Data*)0)
{
    DEBLOG(dump(cerr) << " New Name\n");
}

Name::Name(const Slot &tuple)
  : Tuple(2),
    _namespace(tuple[0]),
    _name(tuple[1])
{
    //throw new Error("illegal", this, "Symbol without a Namespace");
    DEBLOG(dump(cerr) << " New Name\n");
}

Name::Name(unsigned short len, const Slot &tuple)
  : Tuple(len),
    _namespace(tuple[0]),
    _name(tuple[1])
{
    //throw new Error("illegal", this, "Symbol without a Namespace");
    DEBLOG(dump(cerr) << " New Symbol\n");
}

#if 0
Symbol::Symbol(const Slot& name)
  : Name(3, name),
    _value(name[2])
{}
#endif

Name::Name(const Slot &name, const Data /*Namespace*/ *nspace)
    : Tuple(2),
      _namespace(dynamic_cast<const Namespace*>(nspace)),
      _name(name)
{
  //if (_namespace) _namespace->upcount();
  // we are part of the namespace - we don't contribute to its refcount
  DEBLOG(dump(cerr) << "New Name\n");
}

Name::Name(unsigned short len, const Slot &name, const Data /*Namespace*/ *nspace)
    : Tuple(len),
      _namespace(dynamic_cast<const Namespace*>(nspace)),
      _name(name)
{
  //if (_namespace) _namespace->upcount();
  // we are part of the namespace - we don't contribute to its refcount
  DEBLOG(dump(cerr) << "New Name\n");
}

Symbol::Symbol(const Slot& name, const Data /*Namespace*/ *nspace)
  : Name(3, name,nspace),
    _value((Data*)0)
{}

Symbol::Symbol(const Slot &name, const Data /*Namespace*/ *nspace, const Slot &value)
    : Name(3, name,nspace),
      _value(value)
{
    DEBLOG(dump(cerr) << "New Symbol\n");
}

Name::Name(const Name &sym)
  : Tuple(2),
    _namespace(sym._namespace),
    _name(sym._name)
{
    DEBLOG(dump(cerr) << "New Name\n");
    //throw new Error("illegal", &sym, "Copying a Symbol");
}

Name::Name(unsigned short len, const Name &sym)
  : Tuple(len),
    _namespace(sym._namespace),
    _name(sym._name)
{
    DEBLOG(dump(cerr) << "New Name\n");
    //throw new Error("illegal", &sym, "Copying a Symbol");
}

Symbol::Symbol(const Symbol &sym)
  : Name(3,(Name&)sym),
    _value(sym._value)
{
    DEBLOG(dump(cerr) << "New Symbol\n");
}

Name::~Name()
{
  DEBLOG(dump(cerr) << " Destroy Name\n");
  // Clean up the Namespace's reference to this
  //((Data*)space())->del(this);
  /* toad says, "yup...but a symbol won't need to remove itself from a Namespace, because it will always have >=1 refcount from being in the Namespace; so you can only delete from top down - hence need for parent ptrs" */
  /*if (_namespace)
    _namespace->dncount();*/
}

// Uses the copy constructor
Data *Name::clone(void *where)const
{
  throw new Error("name", this, "An attempt was made to duplicate a Symbol");
  return (Data*)0;	// Symbols are unique
}

// returns a mutable copy of this
Data *Name::mutate(void *where) const
{
  throw new Error("name", this, "An attempt was made to mutate a Symbol");
  return (Data*)0;	// Symbols are unique
}

ostream &Symbol::dump(ostream& out) const
{
  Name::dump(out) << "=";
  out << ((Data *)_value)->typeId();
  out << '{' << (void*)((Data *)_value) << '}';
  out << "(...)";
  return out;
  // return _value->dump(out);
}

ostream &Name::dump(ostream& out) const
{
    out << typeId();
    out.form(" [{0x%08x}]: ", this);
    _name.Dump(out);
    out.form("{0x%08x}", _namespace);

#if 0
    if (_namespace)
        _namespace->dump(out);
    else {
        out << "{No Namespace}";
    }
#endif

    return out;
}

int Name::order(Slot const &arg) const
{
    int result;
    if (AKO(arg,Name))
      {
        if ((Data*)this == (Data*)arg)
	  {
	    result = 0;
	  }
	else
	  {
	    Name* p = (Name*)arg;
	    if(p == this)
	      result = 0;
	    else
	       result = _name -> order(p -> _name);
	    // Names sort by their names. Names need to compare between
	    // different Namespaces
	  }
      }
    else
      {
	result = typeOrder(arg);
	// Names do not compare with their values
	// Otherwise x < y !=> y > x
      }
      DEBLOG(dump(cerr) << " order "
 	<< arg.Dump(cerr) << " -> "
	<< result << '\n');

      return result;
}

// Here's the big payoff - comparison is numeric
bool Name::equal(Slot const &arg) const
{
  bool result = false;
  if (AKO(arg,Name))
    {
      if((((Name*)arg)->_namespace) == _namespace)
	{
	  if((Data*)this == (Data*)arg)
	      result = true;
	  else
	      result = _name->equal(((Name*)arg)->_name); /* temporaries (see search()) will have namespace = namespace created in, even though they are in the Namespace */

	  DEBLOG(dump(cerr) << " equal in same Namespace ");
	  DEBLOG(arg.Dump(cerr) << " -> "
		 << result << '\n');
	}
      else
	  result = _name->equal(((Name*)arg)->_name); /* temporaries (see search()) will have namespace = namespace created in, even though they are in the Namespace */
    };
  DEBLOG(dump(cerr) << " equal ");
  DEBLOG(arg.Dump(cerr) << " -> "
	 << result << '\n');
  return result;
}

// Symbol assign - note this can be overridden
Slot Symbol::replace(const Slot &value)
{
  DEBLOG(dump(cerr) << " replace " << value.Dump(cerr));
  _value = value;
  return value;
}

// replace subrange
Slot Name::replace(const Slot &, const Slot &)
{
    return unimpl("dyadic replace on Symbol");
}
Slot Name::replace(const Slot &, const Slot &, const Slot &)
{
    return unimpl("triadic replace on Symbol");
}

// Local

// return Namespace of Symbol
const Data /*Namespace*/ *Name::space() const
{
  if (_namespace)
    return _namespace;
  else
    return store->Names;
}

Slot Symbol::operator=(const Slot &value)
{
    replace(value);
    return value;
}

Slot Name::toconstruct() const
{
  // this cannot be dual with the constructor, because we need a Namespace
  // toconstruct() on Namespace should be dual with Namespace(Slot&)
  // this could also be solved by "real" serialization w/ objids
  return _name;
}

Slot Symbol::toconstruct() const
{
  return new(2) Tuple (-2, (Data*)_name, (Data*)_value);
}

RefCount *Name::upcount( void) const
{
    //DEBLOG(cerr << "Name ");
    //DEBLOG(dump(cerr) << " UP " << _refcount << ".\n");
  RefCount* p = Data::upcount();
  if(refcount() == 2)
    if(space())
      {
	space() -> upcount();
      }
  return p;
}

void Name::dncount( void) const
{
    //DEBLOG(cerr << "Name ");
    //DEBLOG(dump(cerr) << " DN " << _refcount << ".\n");
  Data::dncount();
  if(refcount() == 1)
    if(space())
      {
	space() -> dncount();
      }
}

void Name::changeName(const Slot& s)
{ // if you want to see what these null checks are for, check Directory::insertPrefix
  Namespace* p = (Namespace*)_namespace;
  if(p) p -> drop(_name);
  _name = s;
  if(p)
    {
      p -> Namespace::insert(this);
    }
}

void Name::changeSpace(Namespace* to)
{
  if(AKO(((Name*)this),NamedNamespace))
    {
      if((Namespace*)(NamedNamespace*)this == to)
	{
	  DEBLOG(cerr << (void*)((Namespace*)(NamedNamespace*)this) << " " << (void*)to << "\n");
	  throw new Error("name",new List(2,to,this),"inserting me into myself!");
	}
    }
  Namespace* p = (Namespace*)_namespace;
  if(p) p -> drop (_name);
  _namespace = to;
  if(to) to -> Namespace::insert(this);
}

Slot Name::dirName(const Directory* parent) const
{
  Slot n = _name;
  const Namespace* p = (Namespace*)space();
  while(true)
    {
      if(!p) return n;
      if(p==parent) return n;
      if(!dynamic_cast<const Directory*>(p))
	{
	  return n;
	}
      else
	{
	  const Directory* pd = (Directory*)p;
	  DEBLOG(n -> dump(cerr));
	  n = pd -> prependPrefix(n);
	  DEBLOG(n -> dump(cerr));
	  p = (const Namespace*)(pd -> space());
	}
    }
}

void Name::check(int) const
{
  // TODO: check Name for consistency
}

//////////////////////
/// Namespace

Namespace::Namespace()
    : tree(new VectorSet)
{
}

Namespace::Namespace(const Slot &init)
    : tree(new VectorSet)
{
  int l = init->length();
  for (int i = 0; i < l; i++) {
    insert(init[i]);
  }
}

Namespace::Namespace(Namespace &n)
{
    throw new Error("illegal", n, "Can't copy construct a Namespace");
}

Namespace::~Namespace()
{
}

Data *Namespace::clone(void *where) const
{
    return (Data*)this;	// namespaces are inherently mutable
}

Data *Namespace::mutate(void *where) const
{
    DEBLOG(dump(cerr) << " mutate\n");
    return (Data*)this;
}

// dyadic `[]', item
Slot Namespace::slice(const Slot &from) const
{
  Name* result = search(from);
  if (result)
    return result;
  else {
    // unknown symbol - throw
    throw new Error("dictnf", from, "Symbol not found");
  }
}

// replace Name with a new Name
Slot Namespace::replace(const Slot &from)
{
  DEBLOG(dump(cerr)
           << " replace " << from->dump(cerr)
           << '\n');
    // a single item
    if(dynamic_cast<Name*>((Data*)from))
      {
	tSlot<Name> found = search(from);
	if (found){
	  // bind new value
	  if(dynamic_cast<Symbol*>((Name*)found))
	    {
	       tSlot<Symbol> f = (Name*)found;
	       f->replace(((Symbol*)from)->value());
	      return f;
	    }
	  else
	    {
	      del(*found);
	      tree=tree->insert(from);
	      if(from->refcount()>1) upcount();
	      return from;
	    }
	} else {
	  // unknown symbol - throw
	  throw new Error("dictnf", from, "Symbol not found");
	}
      } else {
	throw new Error("namespace",this,"monadic replace without Name");
      }
}

// replace association
// In a Namespace, assignment has the effect of assigning a value to a Symbol
// If the Symbol doesn't exist, it is created.  Only Namespace can construct 
// Symbols.
Slot Namespace::replace(const Slot &from, const Slot &value)
{
  DEBLOG(dump(cerr)
           << " replace " << from->dump(cerr)
           << " with " << value->dump(cerr)
           << '\n');

    // a single item
    tSlot<Name> found = search(from);
    if (found){
        // bind new value
        if(dynamic_cast<Symbol*>((Name*)found))
	  {
	    tSlot<Symbol> f = (Name*)found;

	    f->replace(value);
	    return f;
	  }
	else
	  {
	    del(*found);
	    Name* p = new (this) Symbol(from, this, value);
	    tree=tree->insert(p);
	    return p;
	  }
    } else {
        // unknown symbol - throw
        throw new Error("dictnf", from, "Symbol not found");
    }
}

// insert association
// In a Namespace, assignment has the effect of assigning a value to a Symbol
// If the Symbol doesn't exist, it is created.  Only Namespace can construct 
// Symbols.
Slot Namespace::insert(const Slot &sym)
{
  DEBLOG(dump(cerr)
           << " insert " << sym->dump(cerr)
           << '\n');

    // a single item
    Name *found = search(sym);    // sym has to be a Symbol
    if (found){return found;}
    // unknown symbol - add it
    if (dynamic_cast<Name*>((Data*)sym))
      {
	tree=tree->insert(sym);
	if(sym->refcount()>1) upcount();
	return sym;
      } else {
	// Namespace may only contain Names
	Name *dsym = new (this) Name(sym, this);
	tree=tree->insert(dsym);
	return sym;
      }
}

// dyadic `search', matching subrange
Slot Namespace::search(const Slot &what) const
{
  // Explanation in Name::order
  Name* result;
  DEBLOG(cerr << "Searching for: ");
  DEBLOG(what->dump(cerr) << '\n');
  if(!AKO(what,Name))
    {
      DEBLOG(cerr << "Not a Name\n");
      Name n (what, this);
      Slot s = &n;
      DEBLOG(cerr << "Search Name: ");
      DEBLOG(s->dump(cerr) << '\n');
      result = tree->search(s);
    }
  else
    {
      result = tree->search(what);
    }
  if (result)
    {
      DEBLOG(cerr << "FOUND: \n");
      DEBLOG(result->dump(cerr));
      DEBLOG(cerr << "\n");
      return result;
    }
  else
    {
      DEBLOG(cerr << "NOT FOUND\n");
      return (Data *)0;
    }
}

// insert association
// In a Namespace, assignment has the effect of assigning a value to a Symbol
// If the Symbol doesn't exist, it is created.  Only Namespace can construct 
// Symbols.
Slot Namespace::insert(const Slot &key, const Slot &value)
{
  DEBLOG(dump(cerr));
  DEBLOG(cerr << "Inserting:\n");
  DEBLOG(cerr << "Key: ");
  DEBLOG(key.Dump(cerr));
  DEBLOG(cerr << "\nValue: ");
  DEBLOG(value.Dump(cerr));
  DEBLOG(cerr << "\n");
    // a single item
    Name *found = search(key);
    if (found){
        // bind new value
        if(dynamic_cast<Symbol*>(found))
	  {
	    ((Symbol*)found)->replace(value);
	    DEBLOG(cerr << "Rebound existing Symbol\n");
	    return found;
	  }
	else
	  {
	    DEBLOG(cerr << "Deleting old Symbol ");
	    DEBLOG(found->dump(cerr));
	    DEBLOG(cerr << "\n");
	    del(*found);
	    DEBLOG(dump(cerr));
	    Name* p = new (this) Symbol(key, this, value);
	    tree=tree->insert(p);
	    DEBLOG(cerr << "Inserted new Symbol ");
	    DEBLOG(dump(cerr));
	    DEBLOG(cerr << "\n");
	    return p;
	  }
   } else {
        // unknown symbol - create and add it
        DEBLOG(cerr << "Adding completely new Symbol in insert\n");
	DEBLOG(dump(cerr));
        Slot s = new (this) Symbol(key, this, value);
        tree=tree->insert(s);
	DEBLOG(dump(cerr));
        return s;
    }
}

Slot Namespace::replace(const Slot &, const Slot &, const Slot &)
{
    return unimpl("triadic replace on Namespace");
}

// delete subrange
Slot Namespace::del(const Slot &from)
{
    // TODO? put some permissions stuff here to stop people deleting bindings
    // of Symbols with non-zero external references.
    DEBLOG(dump(cerr)
           << " delete " << from->dump(cerr));
    Name *found = search(from);
    if (!found)
        throw new Error("name", this, "deleting non-existant Namespace element");
    DEBLOG(cerr << "refcount deleting Name: " << found->refcount() << "\n");
    //if(refcount() > 1) throw new Error("namespace",this,"refcount != 1 in del");
    tree=tree->del(found); // refcount -> 0, Name gets deleted
    return this;
}

// delete subrange
Slot Namespace::del(const Slot &, const Slot &)
{
    return unimpl("dyadic del on Namespace");    
}

// 1,0,-1 depending on order
int Namespace::order(const Slot &arg) const
{
  if(!AKO(arg,Namespace))
    {
      return typeOrder(arg);
    }
  else
    {
      return tree->order(((Namespace*)arg)->tree);
    }
}

// constructor args to recreate object
Slot Namespace::toconstruct() const
{
  return tree->toconstruct();
}

int Namespace::length() const
{
  return tree->length();
}

Slot Namespace::multiply(const Slot& arg) const
{
  return tree->multiply(arg);
}

Slot Namespace::divide(const Slot& arg) const
{
  return tree->divide(arg);
}

Slot Namespace::lshift(const Slot& arg) const
{
  return tree->lshift(arg);
}

Slot Namespace::rshift(const Slot& arg) const
{
  return tree->rshift(arg);
}

RefCount *Namespace::upcount( void) const
{
  //DEBLOG(cerr << "Namespace " << (void*)this << " UP " << _refcount << ".\n");
  return Data::upcount();
}

void Namespace::dncount( void) const
{
  //DEBLOG(cerr << "Namespace " << (void*)this << " DN " << _refcount << ".\n");
  return Data::dncount();
}

ostream &Namespace::dump(ostream &o) const
{
  if(this)
    {
      o << '[' << length() << ']';
      o << "{";
      bool b = true;
      Slot s;
      Slot i = tree -> iterator();
      for(;i->More();)
	{
	  s = i ->Next();
	  if(!b) {o << ", ";} else {b = false;};
	  s.Dump(o);
	}
      o << "}";
    } else {
      o << "{NULL}";
    }
  return o;
}

Slot Namespace::drop(const Slot& s)
{
  DEBLOG(dump(cerr)
	 << " drop " << s->dump(cerr));
  Name *found = search(s); // found will dncount when BigBTree removes
  if(found->refcount()>1) dncount();
  tree=tree->del(found);
  return this;
}

Slot Namespace::iterator() const
{
  return tree->iterator();
}

Slot Namespace::invert() const
{
  return tree->invert();
}
