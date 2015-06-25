// Tree.cc - Coldstore Trees
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: Tree.hh,v 1.5 2000/12/23 01:51:11 coldstore Exp $

#ifndef TREE_HH
#define TREE_HH

#include "List.hh"
/** A Tree is a coloured List.
    One can specify, per Tree instance, the number of colours which will be associated
    with the tree.
 */
cold class Tree
    : public List
{
    
protected:
    /** number of Slots of Colour prefix in the List
     */
    int prefix;

public:
    
    /** construct a Tree of with clen colours, of a given size (default 0)
     */
    Tree(int clen, int size = 0, ...);

    /** construct a Tree from a sequence Slot
     */
    Tree(const Slot &sequence);

    /** copy construct a Tree
     */
    Tree(const Tree *tree);

    /** construct a Tree from a pair of Lists
     */
    Tree(const Slot &colours, const Slot &children);

    virtual ~Tree();

public:
    ///////////////////////////////
    // Coldmud Interface

    // structural
    virtual Data *clone(void *store = (void*)0) const;
    virtual Data *Tree::mutate(void *where) const;
    
    // object
    virtual Slot toconstruct() const;
    virtual ostream &dump(ostream&) const;
    
    virtual int length() const;

    virtual Slot slice(const Slot &from, const Slot &len) const;
    virtual Slot slice(const Slot &from) const;

    virtual Slot replace(const Slot &from, const Slot &to, const Slot &value);
    virtual Slot replace(const Slot &from, const Slot &val);

    virtual Slot insert(const Slot &from, const Slot &val);
    virtual Slot del(const Slot &from, const Slot &len);
    virtual Slot del(const Slot &from);

    virtual Slot search(const Slot &search) const;

    virtual List *toSequence() const;

    Slot iterator() const;
};

#endif

