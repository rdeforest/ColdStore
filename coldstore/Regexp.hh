// RegExp.hh - ColdStore interface to pcre regexp package
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

//     Regular expression support is provided by the PCRE library package,
//     which is open source software, written by Philip Hazel, and copyright
//     by the University of Cambridge, England.
//	ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/

#ifndef REGEXP_HH
#define REGEXP_HH

#include <pcre.h>
#include "String.hh"

// Regexp
class Regexp
    : public String
{

protected:
    pcre *compiled;
    mutable pcre_extra *extra;

    static bool inited;
    static void init();

    int options;
    mutable bool studied;
    int nr_captures;

    void getinfo();
public:
    static int default_options;

    /** construct a Regexp from a String
     * @param arg Regexp description
     */    
    Regexp(const Slot &arg);

    /** construct a Regexp from a String
     * @param arg Regexp description
     */    
    Regexp(const char *arg, int options = Regexp::default_options);

    /** destroy Regexp */
    virtual ~Regexp();

    ///////////////////////////////
    // Coldmud Interface

    /** copy constructor @ref Data::clone
     *
     * Regexps are inherently mutable
     */
    virtual Data *clone(void *store = (void*)0) const;

    /** create a mutable copy @ref Data::mutate
     *
     * Regexps are inherently mutable
     */
    virtual Data *mutate(void *where = (void*)0) const;

    virtual Slot positive() const;		// monadic `+', absolute value
    virtual Slot divide(const Slot &arg) const;		// dyadic '/', divide
};

#endif

