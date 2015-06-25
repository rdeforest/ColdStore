// Null - ColdStore interface to nothing
// Copyright (C) 2001 Robert de Forest
// 
//      $Id: Null.hh,v 1.2 2001/10/03 04:29:34 maelstorm Exp $

#ifndef NULL_HH
#define NULL_HH

cold class Null
    : public Data
{
public:
    Null();
    virtual ~Null();

    virtual Data *clone(void * = 0) const;
    virtual void check(int num = 0) const;

    virtual bool truth() const;
    virtual Slot toconstruct() const;
    virtual ostream &dump(ostream& out) const;

    static Slot construct(const Slot &arg);
    virtual int order(const Slot &arg) const;
    virtual bool equal(const Slot &arg) const;
};

#endif

