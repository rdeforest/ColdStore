// Q - Simple doubly-linked list class
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
//	$Id: Q.hh,v 1.3 2000/11/04 13:27:54 coldstore Exp $

#ifndef Q_HH
#define Q_HH
#include <assert.h>
template <class T> class Q;

template <class T>
class Qh
{
  Q<T> *start;

public:
  Qh();

  // add q to start of queue
  Q<T> *add(Q<T> *q);

  Q<T> *operator =(Q<T> *element);

  bool operator ==(Q<T> *element);

  Q<T> *queue();
    
  Q<T> *set(Q<T> *here);
    
  operator T*();

  // Predicates:
  bool empty() const;
  operator bool();
  
  void own(Q<T> *el);
};

/** class Q<T> - doubly linked list class
 *
 * a Q<T> is a triple [Q<T> *prev, Q<T> *next, Q<T> *parent]
 * such that prev & next != NULL
 * an empty Q<T> is a Q<T> in which prev == next == this.
 * Invariant:  next.prev = this, prev.next = this
 *
 * each Q is (optionally) a member of a group of Qs belonging to a parent
 * 
 */ 
template <class T>
class Q
{
protected:
  Q<T> *_next;
  Q<T> *_prev;

  friend class Qh<T>;
  Qh<T> *_parent;	// pointer somewhere into this ring

public:
  // Constructors

  // Default Constructor
  Q();

  /** construct an empty Q<T>
   */
  Q(Qh<T> &parent);
    
  virtual ~Q();
        
  // Constructor
  Q(T *q);
    
  // Copy Constructor
  // it doesn't actually copy, as that would be fatal
  Q(T &q);
    
  // Accessors
  T *next() const;
    
  T *prev() const;
    
  // Predicates:
  bool empty() const;
    
  // Invariant:  next.prev = this, prev.next = this
  void check() const;
    
  // Mutators
  
  // delete this from whatever Q<T> it's on
  //	returns an empty list
  Q<T> *del();
    
  // add q after this
  Q<T> *add(Q<T> *q);
  
  // insert q before this
  void ins(Q<T> *q);
    
#if 0
  operator T *();
#endif
};

// Q iterator
template <class T>
class Qi
{
    T *_head;
    T *_curr;

public:
#if 0
  Qi(Q<T> *q);
#endif

  Qi(T *q);

  bool check();
  Q<T> *Next();
  Q<T> *Prev();
  bool More() const;
  operator bool() const;
  Q<T> *operator ++(int);
  Q<T> *operator --(int);
  Q<T> *del();
  void add(T *q);
  void ins(T *q);
  operator T *() const;
  T *Current() const;
};

#endif
