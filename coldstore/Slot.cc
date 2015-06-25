// Slot - Coldstore smartpointers
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Slot.cc,v 1.30 2002/04/15 06:59:37 coldstore Exp $";

//#define DEBUGLOG
#define DEBDETAIL 0

#include <typeinfo>
#include<iostream>

#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Integer.hh"
#include "BigInt.hh"
#include "Real.hh"
#include "List.hh"
#include "NList.hh"
#include "Builtin.hh"
#include "Dict.hh"
#include "Symbol.hh"
#include "Set.hh"
#include "VectorSet.hh"
#include "Directory.hh"
#include "Connection.hh"
#include "Server.hh"
//#include "Frame.hh"

RefCount *RefCount::upcount( void) const
{
    validGuard();	// validate the allocation Guards
    inc(_refcount);

#if DEBDETAIL > 1
    DEBLOG(cerr << this
	   << "->upcount() " << _refcount
	   << "\n");
#endif

#ifdef REFMONITOR
    if (initialized) {
	initialized = false;
    } else if (!_refcount) {
	// the refcount has fallen to 0
	throw new Error("refcount", "Object has been deleted");
    }
#endif

    return (RefCount*)this;
}

void RefCount::dncount( void) const
{
    assert(_refcount >= 0);
    validGuard();	// validate the allocation Guards

    RefCount *victim = (RefCount*)(dynamic_cast<const void*>(this));
    if (!_refcount || !dec(_refcount)) {
        DEBLOG(cerr << this
	       << '/' << victim
	       << "->dncount() DELETING\n");
        if((void*)this < (char*)QVMM_BASE+QVMM_SIZE)
	  delete victim;
    } else {
#if DEBDETAIL > 1
        DEBLOG(cerr << this
	       << '/' << victim
	       << "->dncount() " << _refcount
	       << "\n");
#endif
    }
}


// mkData - convert an encoded integer
//	into an instance of Integer
Data *Slot::mkData() const
{
    if (isEncInt()) {
        // we have to create an Integer from the pool of free Integers
        //data_ = Integer::getFree(integer_ >> 1);
        data_ = Integer::getInteger(integer_ >> 1);
    } else if (isComplex()) {
      // we have to clean up the complex pointer
      union {
	Data *data_;
	long integer_;
      } mask;	// small/quick temp
      mask.integer_ = integer_ & MASK_DATA;
      DEBLOG(cerr << this << "->mkData() " << data_ << " -> " << mask.data_ << '\n');
      return mask.data_;
    }
    // return it as-is
    return data_;
}

Data *Slot::simplify() const
{
  switch (integer_ & MASK_DATA) {
  case MARK_COMPLEX:	// this is a complex value e.g. Promise
    // simplify the _data in-place
  DEBLOG(cerr << " simplify complex: " << data_
	 << '\n');
    data_ = mkData()->call(const_cast<Slot&>(*this));
    break;

  case MARK_INTEGER:	// this is an encoded integer
  case MASK_DATA:		// this is also an encoded integer
    // convert encoded integer to Data
    DEBLOG(cerr << " simplify encoded integer: " << data_
	 << '\n');
    data_ = mkData();
    break;

  case MARK_DATA:		// this is fully decoded Data
    // do nothing
    break;

  default:			// this is flatly impossible
    // complain loudly
    assert (!"This Slot contains an impossible TAG");
  }
  return data_;
}

Slot::~Slot()
{
    if (!isEncInt() && data_)
	mkData()->dncount();
}

Slot::Slot(Data *d)
    : data_(d)
{
    if (!isEncInt() && d)
        mkData()->upcount();

    DEBLOG(cerr << "Converting Data pointer " << (void*)d
           << " to Slot " << this << '\n');
}

Slot::Slot(int l)
    : integer_(l<<1 | 1)	// encode the integer
{
    DEBLOG(cerr << "Converted int " << l
           << " to Slot " << this << '\n');

    // we must check to see if l can be encoded
    if ((integer_ >> 1) != l) {
        // we're losing a bit of information ... don't encode
        data_ = Integer::getInteger(l);
    }
}

// reference constructor
Slot::Slot(Data &d)
    : data_(&d)
{
    if (!isEncInt())
        d.upcount();

    DEBLOG(cerr << "Converted Data reference " << &d
           << " to Slot " << this << '\n');
}

// We're really cheating here - we take a const, but we cast it away.
Slot::Slot(Data const *d)
    : data_((Data*)d)
{
    if (!isData()) {
        throw new Error("type", "Non Data passed as Data*");
    }

    if (d && !isEncInt())
        d->upcount();

    DEBLOG(cerr << "Converted Data const pointer " << (void*)d
           << " to Slot " << this << '\n');
}

Slot::Slot(char const *str)
    : data_(new String(str))
{
    data_->upcount();	// we can guarantee this isn't Complex
}

// copy constructor - dups reference
Slot::Slot(const Slot& f)
    : data_(f.data_)
{
    if (!isEncInt() && data_)
        mkData()->upcount();

    DEBLOG(cerr << "Duplicating Slot reference " << &f
           << " to Slot " << this << '\n');
}

Slot::Slot(Slot &s)
    : data_(s.data_)
{
    if (!isEncInt() && data_)
        mkData()->upcount();
    DEBLOG(cerr << "Copying Slot [" << &s
	   << "] '" << s
                   << "' to Slot [" << (void*)this
	   << " rc: " << data_->refcount()
	   <<  "]\n");
}

// assignment operator.  Initializes Slot
Slot &Slot::operator= (const Slot& f)
{
    // DO NOT CHANGE THE ORDER OF THESE STATEMENTS!
    // (This order properly handles self-assignment)
    if (!f.isEncInt() && f.data_) {
        f.mkData()->upcount();
    }

    // clean up the Data we own, if its refcount drops
    if (data_ && !isEncInt())
        mkData()->dncount();
    
    data_ = f.data_;	// copy it as Complex
    return *this;
}

// assignment operator.  Initializes Slot
Slot& Slot::operator= (Data *d)
{
  if (!isData(d)) {
        throw new Error("type", "Non Data passed as Data*");
  }
    // DO NOT CHANGE THE ORDER OF THESE STATEMENTS!
    // (This order properly handles self-assignment)
    if (d) {
	d->upcount();
    }

    // clean up the Data we own, if its refcount drops
    if (data_ && !isEncInt())
      mkData()->dncount();
    
    data_ = d;
    return *this;
}

// assignment operator.  Initializes Slot
Slot& Slot::operator= (int i)
{
    DEBLOG(cerr << " operator Data=: " << data_ << ' ' <<  i << '\n');

    // clean up the Data we own, if its refcount drops
    if (data_ && !isEncInt())
      mkData()->dncount();
    
    integer_ = (i<<1) | 1;	// encode the integer
    
    // we must check to see if l can be encoded
    if ((integer_ >> 1) != i) {
        // we're losing a bit of information ... don't encode
        data_ = Integer::getInteger(i);
    }
    
    return *this;
}

// dereference smartpointer
Slot::operator Data*() const
{
    DEBLOG(cerr << " operator Data*: " << data_ << '\n');
  if (!isEncInt() && isComplex()) {
    throw new Error("error", data_, "Making Data* from Complex-Data");
  } else
    return mkData();
}

Slot::operator Data&() const
{
    DEBLOG(cerr << " operator Data&: " << data_ << '\n');
  if (isComplex()) {
    throw new Error("error", data_, "Making Data& from Complex-Data");
  } else
    return *mkData();
}

Data *Slot::operator->() const
{
  return simplify();
}

Slot Slot::operator[](int index) const
{
  return simplify()->slice(index);
}

Slot Slot::operator[](Slot const &index) const
{
  return simplify()->slice(index);
}

Slot::operator UString *() const
{
    if (!this || !data_)
        return (UString *)0;

    if (!isEncInt()) {
      simplify();
      DEBLOG(cerr << "Converting Slot " << this
	     << " to UString " << data_ << '\n');
      if ((AKO(data_,UString)
	   || AKO(data_,String) ))
        return dynamic_cast<UString*>(data_);
    }
    throw new Error("type", *this, "expected UString");
}

bool Slot::isString() const
{
    return data_ && !isEncInt()
        && AKO(simplify(),String);
}

Slot::operator String *(void) const
{
    DEBLOG(cerr << "Converting Slot " << this
           << " to String " << data_ << '\n');

    if (!this || !data_)
        return (String *)0;

    if (isString())
        return dynamic_cast<String*>(data_);
    else
        throw new Error("type", *this, "expected String");
}

Slot::operator List *() const
{
    if (!this || !data_)
        return (List *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,List))
        return dynamic_cast<List*>(data_);
    }
    throw new Error("type", *this, "expected List");
}

Slot::operator NList *() const
{
    if (!this || !data_)
        return (NList *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,NList))
        return dynamic_cast<NList*>(data_);
    }
    throw new Error("type", *this, "expected NList");
}

Slot::operator Tuple *() const
{
    if (!this || !data_)
        return (Tuple *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Tuple))
        return dynamic_cast<Tuple*>(data_);
    }
    throw new Error("type", *this, "expected Tuple");
}

bool Slot::isList () const
{
  if (!data_ || isEncInt())
    return false;

  simplify();
  return (AKO(data_,List)
	  || AKO(data_,NList)
	  || AKO(data_,Tuple));
}


Slot::operator Builtin *(void) const
{
    if (!this || !data_)
        return (Builtin *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Builtin))
        return dynamic_cast<Builtin*>(data_);
    }
    throw new Error("type", *this, "expected Builtin");
}

Slot::operator Dict *(void) const
{
    if (!this || !data_)
        return (Dict *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Dict))
        return dynamic_cast<Dict*>(data_);
    }
    throw new Error("type", *this, "expected Dict");
}

Slot::operator DictIt *(void) const
{
    if (!this || !data_)
        return (DictIt *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,DictIt))
        return dynamic_cast<DictIt*>(data_);
    }
    throw new Error("type", *this, "expected DictIt");
}

Slot::operator Symbol *(void) const
{
    if (!this || !data_)
        return (Symbol *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Symbol))
        return dynamic_cast<Symbol*>(data_);
    }
    throw new Error("type", *this, "expected Symbol");
}

Slot::operator Name *(void) const
{
    if (!this || !data_)
        return (Name *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Name))
        return dynamic_cast<Name*>(data_);
    }
    throw new Error("type", *this, "expected Name");
}

Slot::operator Namespace *(void) const
{
    if (!this || !data_)
        return (Namespace *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Namespace))
        return dynamic_cast<Namespace*>(data_);
    }
    throw new Error("type", *this, "expected Namespace");
}

Slot::operator NamedNamespace *(void) const
{
    if (!this || !data_)
        return (NamedNamespace *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,NamedNamespace))
        return dynamic_cast<NamedNamespace*>(data_);
    }
    throw new Error("type", *this, "expected NamedNamespace");
}

Slot::operator Set *(void) const
{
    if (!this || !data_)
        return (Set *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Set))
        return dynamic_cast<Set*>(data_);
    }
    throw new Error("type", *this, "expected Set");
}

Slot::operator VectorSet *(void) const
{
    if (!this || !data_)
        return (VectorSet *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,VectorSet))
        return dynamic_cast<VectorSet*>(data_);
    }
    throw new Error("type", *this, "expected Set");
}

Slot::operator Directory *(void) const
{
    if (!this || !data_)
        return (Directory *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Directory))
        return dynamic_cast<Directory*>(data_);
    }
    throw new Error("type", *this, "expected Directory");
}

Slot::operator Connection *(void) const
{
    if (!this || !data_)
        return (Connection *)0;

    if (!isEncInt()) {
      if (AKO(data_,Connection))
        return dynamic_cast<Connection*>(data_);
    }
    throw new Error("type", *this, "expected Connection");
}

Slot::operator Server *(void) const
{
    if (!this || !data_)
        return (Server *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Server))
        return dynamic_cast<Server*>(data_);
    }
    throw new Error("type", *this, "expected Server");
}

Slot::operator Error *(void) const
{
    if (!this || !data_)
        return (Error *)0;

    if (!isEncInt()) {
      simplify();
      if (AKO(data_,Error))
        return dynamic_cast<Error*>(data_);
    }
    throw new Error("type", *this, "expected Error");
}

Slot::operator Integer *() const
{
    if (!this || !data_) {
        return (Integer *)0;
    } else if (isEncInt()) {
        return (Integer*)mkData();
    }

    simplify();
    if (AKO(data_,Integer)) {
        return dynamic_cast<Integer*>(data_);
    } else if (AKO(data_,BigInt)) {
        return dynamic_cast<BigInt*>(data_)->asInteger();
    } else if (AKO(data_,Real)) {
        return dynamic_cast<Real*>(data_)->asInteger();
    } else
        throw new Error("type", *this, "expected numeric type");
}

Slot::operator BigInt *() const
{
    if (!this || !data_) {
        return (BigInt *)0;
    } else if (isEncInt()) {
        return new BigInt(integer_ >> 1);
    }

    simplify();
    if (AKO(data_,Integer)) {
        return new BigInt(((Integer*)data_)->value());
    } else if (AKO(data_,Real)) {
        return new BigInt(((Real*)data_)->asGMP());
    } else if (AKO(data_,BigInt)) {
        return dynamic_cast<BigInt*>(data_);
    } else
        throw new Error("type", *this, "expected numeric type");
}

Slot::operator Real *() const
{
    if (!this || !data_) {
        return (Real *)0;
    } else if (isEncInt()) {
        return new Real(integer_ >> 1);
    }

    simplify();
    if (AKO(data_,Integer)) {
        return new Real(dynamic_cast<Integer*>(data_));
    } else if (AKO(data_,BigInt)) {
        return new Real(dynamic_cast<BigInt*>(data_)->asGMP());
    } else if (AKO(data_,Real)) {
        return dynamic_cast<Real*>(data_);
    } else
        throw new Error("type", *this, "expected numeric type");
}

bool Slot::isNumeric() const
{
  if (isEncInt())
    return true;

  simplify();
  return AKO(data_,Integer)
    || AKO(data_,BigInt)
    || AKO(data_,Real);
}

Slot::operator char *(void) const
{
    String *str = dynamic_cast<String*>(simplify());
    if (!str)
        throw new Error("type", *this, "expected string type");
    return (char*)*str;
    //char *result = (char*)(str->content());
    //DEBLOG(cerr << "Converted Slot " << *this
    //       << " to char* " << result << '\n');
    //return result;
}

Slot::operator int() const {
    DEBLOG(cerr << "Converting Slot " << this << " to int\n");
    if (isEncInt()) {
        DEBLOG(cerr << "Converted Slot " << this
               << " to int " << (integer_ >> 1) << '\n');
        return integer_>>1;
    } else if (!this || !data_) {
        throw new Error("type", (Data*)NULL, "expected numeric type");
    }

    simplify();
    if (AKO(data_,BigInt)) {
        return dynamic_cast<BigInt*>(data_)->asint();
    } else if (AKO(data_,Real)) {
        return dynamic_cast<Real*>(data_)->asint();
    } else if (AKO(data_,Integer)) {
        return dynamic_cast<Integer*>(data_)->value();
    } else {
        throw new Error("type", *this, "expected numeric type");
    }
}

Slot::operator long() const {
    DEBLOG(cerr << "Converting Slot " << this << " to int\n");
    if (isEncInt()) {
        DEBLOG(cerr << "Converted Slot " << this
               << " to int " << (integer_ >> 1) << '\n');
        return integer_>>1;
    } else if (!this || !data_) {
        throw new Error("type", (Data*)NULL, "expected integer");
    }

    simplify();
    if (AKO(data_,BigInt)) {
        return dynamic_cast<BigInt*>(data_)->asint();
    } else if (AKO(data_,Real)) {
        return dynamic_cast<Real*>(data_)->asint();
    } else if (AKO(data_,Integer)) {
        return dynamic_cast<Integer*>(data_)->value();
    } else {
        throw new Error("type", *this, "expected numeric type");
    }
}

const char *Slot::typeId() const {
    if (isEncInt())
        return "7Integer";
    else
        return typeid(*data_).name();
}

#if 0
// get the real pointer to data (accounting for any Forward'ing)
Data *Slot::forward()
{
    assert(_data);	// can't dereference a NULL Slot
    
    if ((Type)*_data == isForward) {
        // _data in fact points to a Forward
        // so we update our data_ to point to the forwardee
        *this = ((Forward *)_data)->moved();	// update our pointer
    }
    
    return _data;	// return the actual data, stripped of forwardings
}

// A Data method needs to change something in itself
// Thus it first checks if this is the only pointer to *data_
Data *Slot::mutate() const
{
    //forward();
    if (!isEncInt()) {
        // if the Data is immutable,
        // make a clone and point-at/return it
        data_ = data_->mutate();
    }

    // assert((integer & 1) || data_->refs() == 1);	// clearly, to be mutable, refs == 1
    return *this;	// return the (possibly) new Data to method:
}
#endif

// find offset of leftmost NULL element in range 
int Slot::findNull(const Slot *from)
{
    const Slot *p = from;
    while (*p++);
    return p - from;
}

// consistency checker for Slot
void Slot::check(const Slot *from, size_t len)
{
    for (size_t i = 0; i < len; i++) {
      if (from[i].integer_ && !(from[i].isEncInt())) 
        //((Data*)(dynamic_cast<const void*>(from[i].data_)))->check();
        from[i].data_->check();
    }
}

// duplicate the range
// perform necessary reference counting
Slot *Slot::dup(Slot *to, const Slot *from, size_t range)
{
    for (size_t i = 0; i < range; i++) {
        to[i] = from[i];
    }
    return to;
}

void Slot::destroy(Slot *from, size_t range)
{
    for (size_t i = 0; i < range; i++) {
        if (from[i] && !from[i].isEncInt()) {
            from[i].mkData()->dncount();
        }
        from[i].integer_ = 0;	// wipe it clean
    }
}

Slot *Slot::move(Slot *to, const Slot *from, size_t range)
{
    return (Slot *)memmove(to, from, range * sizeof(Slot));
}

void Slot::init(Slot *from, size_t range)
{
    memset(from, 0, sizeof(Slot) * range);
}

// find a single matching T
Slot *Slot::find(Slot *from, const Slot &_c, size_t range)
{
    for (size_t i = 0; i < range; i++) {
        if (from[i] == _c)
            return from + i;
    }
    return NULL;
}

bool DumpVerbose = false;
void DumpVerbosely(bool yes)
{
    DumpVerbose = yes;
}

ostream &Slot::Dump(ostream& out) const
{
    if (integer_ != 0) {
        mkData();
        if (DumpVerbose) {
            out << data_->typeId();
            out.form("[0x%08x]/", data_);
        }
        data_->dump(out);
    } else {
        out << "[NULL]";
    }
    return out;
}

// Data ordering over Slot
int Slot::order(const Slot& d) const
{
    if (integer_ - d.integer_) {
        // they're not numerically equal - point to different things
        if (isEncInt()) {
            // encoded Integer - temporarily decode it and recurse
            Slot temp(Integer::getInteger(integer_>>1));
            return temp.order(d);			// recurse
        } else {
            // this is not an encoded Integer - use the virtual comparison
            if (integer_) {
                // this is non-null
                if (d.integer_)
                    return simplify()->order(d.simplify());	// try comparison on its merits
                else
                    return 1;		// other is null, must be greater
            } else {
                // this is null
                if (d.integer_)
                    return -1;		// this is null, so d must be greater
                else
                    return 0;		// they're both null
            }
        }
    } else {
        // They're the same entity, ergo ==
        return 0;
    }
}

int Slot::order(const Slot& d1, const Slot& d2)
{
    return d1.order(d2);
}

// Data equality over Slot
bool Slot::equal(const Slot& d) const
{
    if (integer_ != d.integer_) {
        // they're not numerically equal - point to different things
        if (isEncInt()) {
            // this is an encoded Integer - temporarily decode it and recurse
            Slot temp(Integer::getInteger(integer_>>1));
            return temp.equal(d);			// recurse
        } else {
            // this is not an encoded Integer - use the virtual comparison
            if (integer_) {
                // this is non-null
                if (d.integer_) {
                    return simplify()->equal(d.simplify());	// try equality on its merits
                } else
                    return false;		// other is null, this is not
            } else {
                // this is null
                if (d.integer_)
                    return false;		// this is null, other isn't
                else
                    return true;		// both null
            }
        }
    } else {
        // They're the same entity, ergo ==
        return true;
    }
}

bool Slot::operator==(const Slot &d) const
{
    return (this == &d) || (integer_ == d.integer_) || equal(d);
}

bool Slot::operator!=(const Slot &d) const
{
    return (this != &d) && (integer_ != d.integer_) && !equal(d);
}

bool Slot::operator>=(const Slot &d) const
{
    return (this == &d) || (integer_ == d.integer_) || (order(d) >= 0);
}

bool Slot::operator>(const Slot &d) const
{
    return (this != &d) && (integer_ != d.integer_) && (order(d) > 0);
}

bool Slot::operator<=(const Slot &d) const
{
    return (this == &d) || (integer_ == d.integer_)
        || (this != &d) && (order(d) <= 0);
}

bool Slot::operator< (const Slot &d) const
{
    return (this != &d) && (integer_ != d.integer_) && (order(d) < 0);
}

void Slot::segFill(Slot *to, Slot const &filler, int count)
{
    Slot *sptr = to;
    for (int i=count;  i; i--)
        *(sptr++) = filler;
}

// search - find a needle in this haystack O(n^2)
Slot *Slot::search(Slot *haystack, const Slot *needle,
                   size_t len, size_t range)
{
    for (size_t i = 0; i < (range - len + 1); i++) {
        size_t j;
        for (j = 0; j < len; j++) {
            if (*(haystack + i + j) != *(needle + j))
                break;
        }
        if (j == len)
            return haystack + i;
    }
    return NULL;
}


ostream& operator<< (ostream& out, const Slot & slot)
{
    slot.Dump(out);
#if 0
    out << &slot << " [" << slot.data_ << "] ";
    if (slot.data_) {
        out << slot.typeId();
    } else {
        out << "NIL";
    }
    //  out << slot.typeId() << '(' << slot->toconstruct() << ')';
#endif
    return out;
}

Slot Slot::operator + (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)+((int)d);
  return simplify() -> add(d.simplify());
}

Slot Slot::operator - (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)-((int)d);
  return simplify() -> subtract(d.simplify());
}

Slot Slot::operator * (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)*((int)d);
  return simplify() -> multiply(d.simplify());
}

Slot Slot::operator / (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)/((int)d);
  return simplify() -> divide(d.simplify());
}

Slot Slot::operator % (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)%((int)d);
  return simplify() -> modulo(d.simplify());
}

Slot Slot::operator ~ () const
{
  if(isEncInt()) return ~((int)*this);
  return simplify() -> invert();
}

Slot Slot::operator & (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)&((int)d);
  return simplify() -> and(d.simplify());
}

Slot Slot::operator ^ (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)^((int)d);
  return simplify() -> xor(d.simplify());
}

Slot Slot::operator | (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)|((int)d);
  return simplify() -> or(d.simplify());
}

Slot Slot::operator << (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)<<((int)d);
  return simplify() -> lshift(d.simplify());
}

Slot Slot::operator >> (const Slot& d) const
{
  if(isEncInt()) return ((int)*this)>>((int)d);
  return simplify() -> rshift(d.simplify());
}

#if MAIN
#include <iostream.h>
#include <typeinfo>

main()
{
    cerr << typeid(Slot).name() << " size: " << sizeof(Slot) << '\n';
}
// reports 4 bytes
#endif 
