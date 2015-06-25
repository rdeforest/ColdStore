// Directory - automatic string folding tree, and the base of all naming trees
// Copyright 2000 Matthew Toseland
// GPL; See LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#ifndef _DIRECTORY_HH
#define _DIRECTORY_HH

#include "List.hh"
#include "Symbol.hh"
#include "NamedNamespace.hh"

class Directory : public NamedNamespace
{
public:
  Directory(); // fails
  Directory(const Slot& init);
  Directory(const Slot& name, const Data* nspace);
  virtual ~Directory();
  static Slot construct(const Slot& arg) { return (Namespace *)(new Directory(arg)); }
protected:
  Directory(Directory& d);
  virtual bool isPrefix(const Slot& prefix, const Slot& name) const;
  virtual Slot killPrefix(const Slot& prefix, const Slot& name) const;
  virtual Slot commonPrefix(const Slot& name1, const Slot& name2) const;
  virtual Slot insertPrefix(const Slot& prefix);
  virtual Slot createSubspace(const Slot& subspace);
public:
  virtual Slot prependPrefix(const Slot& suffix) const;
  virtual ostream& dump(ostream& o) const;
  // sequence
  virtual int length() const;
  virtual Slot concat(const Slot &arg, const Slot& prefix) const;	// combine two sequences
  virtual Slot concat(const Slot &arg) {return concat(arg,(Data*)NULL);}
  virtual Slot replace(const Slot& from);
  virtual Slot replace(const Slot &from, const Slot &val);
  virtual Slot insert(const Slot &from, const Slot &val); // monadic doesn't make sense generally
  virtual Slot insert(const Slot &from);
  virtual Slot del(const Slot &from);
  virtual Slot search(const Slot &search) const;
  // iteration
  virtual Slot lshift(const Slot& arg) const;
  virtual Slot rshift(const Slot& arg) const;
  virtual List *toSequence(const Slot& curprefix) const;
  virtual List *toSequence() const { return toSequence((Data*)NULL); }
  virtual Slot modulo(const Slot& arg) const;
};

#endif // _DIRECTORY_HH
