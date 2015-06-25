// List.cc - Coldstore Lists
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: List.hh,v 1.26 2002/05/09 13:31:16 coldstore Exp $

#ifndef LIST_HH
#define LIST_HH

#include <Data.hh>
#include <Vector.hh>
#include <tSlot.hh>

/** dynamic ordinally indexed array of @ref Slot
 */
class List
    : public Data,
      public Vector<Slot>
{
    
public:
    
    // Constructors

    /** construct and init a List of given size (default 0)
     * -size means there are that many initialisation args to follow
     */
    List(int size = 0, ...);

    /** construct a List from a sequence Slot
     */
    List(const Slot &sequence);

    /** copy construct a subList
     */
    List(const List *contentT, int start=0, int l=-1);

    /** copy construct a subList
     */
    List(const List &contentT, int start = 0, int l = -1);

    /** copy from a Vector<Slot>
     */
    List(const Vector<Slot>* contentT, int start=0, int l=-1);

    /** copy from a Vector<Slot>
     */
    List(const Vector<Slot>& contentT, int start=0, int l=-1);

    /** construct a List from a subTuple
     */
    List(const Tuple *contentT, int start = 0, int l = -1);

    /** construct a List from a TupleBase<Slot>
     */
    List(TupleBase<Slot> *contentT);
    
    List(TupleBase<Slot> *contentT, int size, ...);

    virtual ~List();

    virtual void check(int=0) const;

    //Slot &List::operator [] (const Data &i) const;

    /** factor List into: [list1-only, intersection, list2-only]
     */
    Slot factor(const List *l) const;

public:
    ///////////////////////////////
    // Coldmud Interface

    // structural
    virtual Data *clone(void *store = (void*)0) const;
    virtual Data *mutate(void *store = (void*)0) const;

    // object
    virtual bool truth() const;
    virtual Slot toconstruct() const;
    virtual ostream &dump(ostream&) const;
    
    static Slot construct(const Slot &arg);
    virtual int order(const Slot &arg) const;
    virtual bool equal(const Slot &) const;

    // arithmetic
    virtual Slot positive() const{		// monadic `+', absolute value
        return qsort();    }
    virtual Slot negative()  const{		// monadic `-', negative absolute value
        return reverse();    }
    virtual Slot add(const Slot &arg) const;
#if 0
    virtual Slot subtract(const Slot &arg) const;
    virtual Slot multiply(const Slot &arg) const;
    virtual Slot divide(const Slot &arg) const;
    virtual Slot modulo(const Slot &arg) const;
    
    // bitwise
    virtual Slot invert() const;
    virtual Slot and(const Slot &arg) const;
    virtual Slot xor(const Slot &arg) const;
    virtual Slot or(const Slot &arg) const;
    virtual Slot lshift(const Slot &arg) const;
    virtual Slot rshift(const Slot &arg) const;
#endif
    
    // sequence
    virtual bool isSequence() const;
    virtual int length() const;
    virtual Slot concat(const Slot &arg) const;
    virtual Slot slice(const Slot &from, const Slot &len) const;
    virtual Slot slice(const Slot &from) const;
    virtual Slot replace(const Slot &from, const Slot &to, const Slot &value);
    virtual Slot replace(const Slot &from, const Slot &val);
    virtual Slot replace(const Slot &val);
    virtual Slot insert(const Slot &from, const Slot &val);
    virtual Slot insert(const Slot &val);
    virtual Slot del(const Slot &from, const Slot &len);
    virtual Slot del(const Slot &from);
    
    virtual Slot search(const Slot &search) const;
    virtual List *toSequence() const;
    
    // iterator
    virtual Slot iterator() const ;
    
    // List is its own iterator
    bool More() const;
    Slot Next();
    
    /** call object
     * 
     * treat object as functional and call it with arg
     * @param args the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot call(Slot &args);	// call object

    ////////////////////////////////////
    // Network primitives

    /** connection indication/reuest
     * @param to indication's source
     * @return 
     */
    virtual Slot connect(const Slot to);

    /** diconnection indication/request
     * @param from indication's source
     * @return 
     */
    virtual Slot disconnect(const Slot from);
    
    /** incoming available indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot incoming(const Slot from);

    /** ready-for-outgoing indication/request
     * @param from indication's source
     * @return where to send future indications - NULL to disable
     */
    virtual Slot outgoing(const Slot to);

    // additional ops
    virtual Slot qsort() const;
    virtual Slot reverse() const;
    virtual Slot sortuniq() const;
    virtual Slot join(const Slot sep = " ") const;

    // hoist some Vector methods to enable children to use them
    Vector<Slot>::vconcat;
    Vector<Slot>::Replace;

    // stack ops
    virtual List *push(const Slot &val);
    virtual List *pop(Slot &val);

    /** make a completely clean mutable copy with new Vector allocation
     */
    virtual Slot Mutate();
};

class Stack
    : public List
{
public:
    // Constructors

    /** construct a Stack of given size (default 0)
     */
    Stack(unsigned int size = 0)
            : List(size) {}

    /** construct a Stack from a sequence Slot
     */
    Stack(const Slot &sequence)
            : List(sequence) {}

    virtual ~Stack() {};

    /** call object
     * 
     * treat object as functional and call it with arg
     * @param args the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot call(Slot &args);	// call object

    virtual ostream &dump(ostream&) const;

    ////////////////////////////////////
    // Network primitives

    /** connection indication/reuest
     * @param to indication's source
     * @return 
     */
    virtual Slot connect(const Slot to);

    /** diconnection indication/request
     * @param from indication's source
     * @return 
     */
    virtual Slot disconnect(const Slot from);
    
    /** incoming available indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot incoming(const Slot from);

    /** ready-for-outgoing indication/request
     * @param from indication's source
     * @return where to send future indications - NULL to disable
     */
    virtual Slot outgoing(const Slot to);

    // stack ops
    virtual List *push(const Slot &val);
    virtual List *pop(Slot &val);    
    virtual Slot &top();
};

// Some utility List functions
void Push(Slot &list, const Slot &val);
Slot Pop(Slot &list);
Slot Append(Slot &list, const Slot &val);

#define MAKE_TEMP_LIST(target, sz, args...) \
size_t TEMP_LIST_size = sizeof(TupleBase<Slot>)+sizeof(Slot)*sz;  \
void* pos = alloca(TEMP_LIST_size); \
Slot TEMP_LIST_s((Data*)NULL); \
TupleBase<Slot>* TEMP_LIST_p = new (pos) ATupleBase<Slot> (&TEMP_LIST_s,sz); \
List ll(TEMP_LIST_p, -sz, ## args ); \
Slot target = &ll;
#endif
