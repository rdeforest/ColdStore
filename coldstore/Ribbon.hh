// Ribbon.cc - Coldstore Ribbons
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id: Ribbon.hh,v 1.3 2000/10/22 21:28:08 maelstorm Exp $

#ifndef RIBBON_HH
#define RIBBON_HH

#include "List.hh"
/** A Ribbon is a coloured List.
    One can specify, per Ribbon instance, the number of colours which will be associated
    with the ribbon.
 */
cold class Ribbon
    : public List
{
    
protected:
    /** number of Slots of Colour prefix in the List
     */
    int prefix;

public:
    
    /** construct a Ribbon of with clen colours, of a given size (default 0)
     */
    Ribbon(int clen, int size = 0, ...);

    /** construct a Ribbon from a sequence Slot
     */
    Ribbon(const Slot &sequence);

    /** copy construct a Ribbon
     */
    Ribbon(const Ribbon *ribbon);

    /** construct a Ribbon from a pair of Lists
     */
    Ribbon(const Slot &colours, const Slot &children);

    virtual ~Ribbon();

public:
    ///////////////////////////////
    // Coldmud Interface

    // structural
    virtual Data *clone(void *store = (void*)0) const;
    virtual Data *Ribbon::mutate(void *where) const;
    
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

