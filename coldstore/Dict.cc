// Dict - ordered mapping of key to value
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Dict.cc,v 1.19 2002/02/09 05:09:51 coldstore Exp $";

#include "Data.hh"
#include "Store.hh"

#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"

#include "Dict.hh"

#include "tSlot.th"
template union tSlot<Dict>;

bool SlotPair::operator == (const SlotPair &p) const {
    bool result = _key == p._key;
    return result;
}

bool SlotPair::operator != (const SlotPair &p) const {
    bool result = _key != p._key;
    return result;
}

bool SlotPair::operator < (const SlotPair &p) const {
    bool result = _key < p._key;
    return result;
}

bool SlotPair::operator <= (const SlotPair &p) const {
    bool result = _key <= p._key;
    return result;
}

bool SlotPair::operator >= (const SlotPair &p) const {
    bool result = _key >= p._key;
    return result;
}

bool SlotPair::operator > (const SlotPair &p) const {
    bool result = _key > p._key;
    return result;
}

const SlotPair &SlotPair::operator =(const SlotPair &p)
{
    _key = p._key;
    _value = p._value;
    return p;
}

const int SlotPair::operator =(const int i)
{
    _key = (Data*)0;
    _value = (Data*)0;
    return i;
}

int SlotPair::order(const SlotPair *arg) const
{
    return _key->order(arg->_key);
}

Dict::Dict(const Slot &init)
    : tree() //BigBTree<SlotPair>()
{
    int l = init->length();
    for (int i = 0; i < l; i+=2) {
        Slot key = init[i];
        Slot val = init[i+1];
        tree.BigBTree<SlotPair>::Add(SlotPair(key, val));
    }
}

Dict::Dict()
    : tree() //BigBTree<SlotPair>(order)
{
}

Dict::Dict(const Dict &o)
    : tree() // BigBTree<SlotPair>(order)
{
    //BigBTreeIterator<SlotPair> it(o);
    //cerr << "Dict::Dict " << o.dump(cerr) << '\n';
    BigBTreeIterator<SlotPair> it(o.tree);
    while (it.More()) {
        SlotPair *item = it.Next();
        
        // add the next item to the list
        Add(item->key(), item->value());
        // Note: we can get away with not assigning to l
        // because its refcount is 1
    }

#if 0
    if (o.length()) {
        for (Slot it = new DictIt(o); it->More();) {
            Slot element = it->Next();
            const Slot &key = element[0];
            const Slot &val = element[1];
            //cerr << "Dict::Dict element " << key.Dump(cerr) << ", " << val.Dump(cerr) << '\n';
            Add(key, val);
        }
    }
#endif
}

/*Dict::Dict(const Slot &pairs)
    : tree() //BigBTree<SlotPair>(order)
{
    Slot p = pairs->toSequence();
    for (int i = 0; i < p->length(); i+=2) {
        tree.BigBTree<SlotPair>::Add(SlotPair(p[i], p[i+1]));
    }
    }*/

Dict::~Dict()
{}

// structural

// Uses the copy constructor
Data *Dict::clone(void *where) const
{
    return new Dict(*this);
}

// returns a mutable copy of this
Data *Dict::mutate(void *where) const
{
    if (refcount() > 1)
        return clone(where);
    else
        return (Data*)this;
}

/////////////////////////////////////
// object

// the object's truth value
bool Dict::truth() const
{
    return tree.Size() != 0;
}

// constructor args to recreate object
Slot Dict::toconstruct() const
{
    int size = 2 * tree.Size();
    if (size) {
        Slot t = new (size) Tuple(size);
        //Slot::init(t->content(), size);
        
        BigBTreeIterator<SlotPair> it(tree);
        for (int i = 0; it.More(); i+=2) {
            SlotPair *item = it.Next();
            (*(Tuple*)t)[i] = item->key();
            (*(Tuple*)t)[i+1] = item->value();
        }
        return t;
    } else {
        return new List();
    }
}

// 1,0,-1 depending on order
int Dict::order(const Slot &arg) const
{
    BigBTreeIterator<SlotPair> it1(tree);
    BigBTreeIterator<SlotPair> it2(((Dict*)arg)->tree);
    while (it1.More() && it2.More()) {
        SlotPair *item1 = it1.Next();
        SlotPair *item2 = it2.Next();
        int result = item1->order(item2);
        if (result)
            return result;
    }
    return 0;
}

// 1,0,-1 depending on order
bool Dict::equal(const Slot &arg) const
{
    if ((void*)&arg == (void*)this)
        return true;
    
    BigBTreeIterator<SlotPair> it1(tree);
    BigBTreeIterator<SlotPair> it2(((Dict*)arg)->tree);
    while (it1.More() && it2.More()) {
        SlotPair *item1 = it1.Next();
        SlotPair *item2 = it2.Next();
        if (*item1 != *item2)
            return false;
    }
  return true;
}


/////////////////////////////////////
// sequence

// length as sequence
int Dict::length() const
{
    return tree.Size();
}

// concatenate two sequences - Merge two Dicts
Slot Dict::concat(const Slot &arg) const
{
    if (AKO(arg,Dict)) {
        // we're concatenating two Dicts
        Dict *m = (Dict*)mutate();
        Dict *other = arg;
        
        BigBTreeIterator<SlotPair> it(other->tree);
        while (it.More()) {
            SlotPair *item = it.Next();
            m->tree.BigBTree<SlotPair>::Add(SlotPair(item->key(), item->value()));
        }
        return m;
    } else {
        // construct an order, then merge
        Slot tmp = new Dict(arg);
        return concat(tmp);
    }
}

// dyadic `[]', range
Slot Dict::slice(const Slot &from, const Slot &len) const
{
    // a range of items
    Slot l = new List;
    
    BigBTreeIterator<SlotPair> it(tree);
    it.BeginFrom(SlotPair(from));
    while (it.More()) {
        SlotPair *item = it.Next();
        
        // break at end of subrange
        if (item->key() > len)
            break;
        
        // add the next item to the list
        l->add(item->key());
        l->add(item->value());
    }
    return l;
}

// dyadic `[]', item
Slot Dict::slice(const Slot &from) const
{
    // a single item
    SlotPair *found = FindEQ(from);
    return found->value();
}

// dyadic `search', matching subrange
Slot Dict::search(const Slot &what) const
{
    SlotPair *found = Find(what);
    if (!found || (*found != what))
        return (Data*)0;
    else {
#if 0
        Slot l = new List;
        l->add(found->key());
        l->add(found->value());
        return l;
#endif
        return found->value();
    }
}

// delete item from Dict
Slot Dict::del(const Slot &from)
{
    Dict *m = (Dict*)mutate();
    SlotPair *found = m->FindEQ(from);
    m->tree.Remove(*found);
    return m;
}

// delete subrange
Slot Dict::del(const Slot &from, const Slot &len)
{
    // Delete item(s) from Dict
    Dict *m = (Dict*)mutate();
    BigBTreeIterator<SlotPair> it(m->tree);
    it.BeginFrom(SlotPair(from));
    while (it.More()) {
        SlotPair *item = it.Next();
        
        // break at end of subrange
        if (item->key() > len)
            break;
        
        m = m->del(item->key());
    }
    
    return m;
}

// replace subrange
Slot Dict::replace(const Slot &from, const Slot &to, const Slot &value)
{
    Dict *m = (Dict*)mutate();

    // Assign a range of items
    BigBTreeIterator<SlotPair> it(tree);
    Slot values = value->iterator();
    it.BeginFrom(SlotPair(from));
    while (it.More()) {
        SlotPair *item = it.Next();
        
        // break at end of subrange
        if (item->key() > to)
            break;
        
        m = m->replace(item->key(), values->Next());		// replace the current value
    }
    // move to the next assignment value
    if (values->More()) {
            throw new Error("range", values, "Keys exhausted");
    }

    return m;
}

// replace Item
Slot Dict::replace(const Slot &key, const Slot &value)
{
    Dict *m = (Dict*)mutate();
    
    // Assign a single item
    SlotPair *found = m->FindEQ(key);
    // bind new value
    found->value() = value;
    return m;
}

// insert Item
Slot Dict::insert(const Slot &key, const Slot &value)
{
    Dict *m = (Dict*)mutate();
    
    // Assign a single item
    SlotPair *found = m->tree.Find(SlotPair(key));
    if (!found || (*found != key)){
        // unknown symbol - create and add it
        m->tree.Add(SlotPair(key, value));
    } else {
        // bind new value
        found->value() = value;
    }
    return m;
}

// insert Item
Slot Dict::insert(const Slot &value)
{
    Dict *m = (Dict*)mutate();
    
    // Assign a single item
    SlotPair *found = m->Find(value);
    if (!found || (*found != value)){
        // unknown symbol - create and add it
        m->tree.Add(SlotPair(value));
    } else {
        // bind new value
        found->value() = value;
    }
    return m;
}

Slot Dict::replace(const Slot &value)
{
    return Data::replace(value);	// in fact, it throws
}

// return an iterator
Slot Dict::iterator() const
{
    return new DictIt(this);
}

/////////////////////////////////////
// map

// return map as sequence
List *Dict::toSequence() const
{
    // a range of items
    List *l = new List;
    
    BigBTreeIterator<SlotPair> it(tree);
    while (it.More()) {
        SlotPair *item = it.Next();
        
        // add the next item to the list
        l->check();
        l = l->add(item->key());
        l = l->add(item->value());
        // Note: we can get away with not assigning to l
        // because its refcount is 1
    }
    l->upcount();
    return l;
}

// Local
bool Dict::Add(const Slot &key, const Slot &value)
{
    SlotPair p(key, value);
    return tree.Add(p);
}

SlotPair *Dict::Find(const Slot &key) const
{
    SlotPair p(key);
    return tree.Find(p);
}

SlotPair *Dict::FindEQ(const Slot &key) const
{
    SlotPair *found = Find(key);
    if (!found || (*found != key))
        throw new Error("dictnf", key, "key not found");
    return found;
}

bool Dict::FindV(const Slot &key, Slot &value) const
{
    SlotPair p(key, value);
    SlotPair *found = tree.Find(p);
    if (found) {
        value = found->value();
    }
    return found != NULL;
}

void Dict::Replace(SlotPair *entry, const Slot &value)
{
    entry->value() = value;
}


//////////////////////////////////
// DictIt - Iterator over Dicts

DictIt::DictIt(const Dict *o)
    : BigBTreeIterator<SlotPair>(o->tree),
      dict(o),
      min((Data*)0),
      max((Data*)0)
{}

DictIt::DictIt(const Dict &o)
    : BigBTreeIterator<SlotPair>(o.tree),
      dict(&o),
      min((Data*)0),
      max((Data*)0)
{}

DictIt::DictIt(const DictIt &oit)
    : BigBTreeIterator<SlotPair>(oit._tree),
      dict(oit.dict),
      min((Data*)0),
      max((Data*)0)
{}

DictIt::~DictIt() {}

// structural

// Uses the copy constructor
Data *DictIt::clone(void *where) const
{
    return new DictIt(*this);
}

// returns a mutable copy of this
Data *DictIt::mutate(void *where) const
{
    return clone(where);
}

// object

// the object's truth value
bool DictIt::truth() const
{
    return More();
}

// constructor args to recreate object
Slot DictIt::toconstruct() const
{
    return dict;
}

// construct from constructor args
Slot DictIt::construct(const Slot &arg)
{
    if (!AKO(arg,Dict))
        throw new Error("type", arg, "DictIt may only be constructed from an Dict");
    return new DictIt((Dict*)arg);
}

// 1,0,-1 depending on order
int DictIt::order(const Slot &arg) const
{
    if (typeOrder(arg))
        return typeOrder(arg);
    if ((((DictIt*)arg)->dict != dict))
        return dict->order(((DictIt*)arg)->dict);
    else 
        return CurrentRank() - ((DictIt*)arg)->CurrentRank();
}

// depending on equality
bool DictIt::equal(const Slot &arg) const
{
    return AKO(arg,DictIt)
        && (((DictIt*)arg)->dict == dict)
        && (((DictIt*)arg)->CurrentRank() == CurrentRank());
}

// iterator

// predicate - is object an iterator
bool DictIt::isIterator() const
{
    return true;
}

bool DictIt::More() const
{
    return BigBTreeIterator<SlotPair>::More();
}

Slot DictIt::Next()
{
    if (BigBTreeIterator<SlotPair>::More()) {
        SlotPair *n =  BigBTreeIterator<SlotPair>::Next();
        if (n) {
            curr = new (2) Tuple(-2, (Data*)(n->key()), (Data*)(n->value()));
            return curr;
        }
    }
    throw new Error("break", this, "Iterator exhaustion");        
}

#include "BTree.th"
template class BigBTree<SlotPair>;
template class PathStruct<SlotPair>;
template class BigBTreeNode<SlotPair>;
template class BigBTreeIterator<SlotPair>;
