// Regexp.cc - Coldstore Regexps
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Regexp.cc,v 1.8 2002/03/15 10:31:08 coldstore Exp $";

//#define DEBUGLOG
union Char;
#include "Data.hh"
#include "Store.hh"

#include "Vector.hh"
#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"

#include "Regexp.hh"
#include <stdlib.h>

#include "tSlot.th"
template union tSlot<Regexp>;

bool Regexp::inited = false;
int Regexp::default_options = PCRE_EXTRA;
//| PCRE_CASELESS
//| PCRE_MULTILINE
//| PCRE_DOTALL
//| PCRE_EXTENDED
//| PCRE_ANCHORED
//| PCRE_DOLLAR
//| PCRE_EXTRA
//| PCRE_NOTBOL
//| PCRE_NOTEOL
//| PCRE_UNGREEDY
//| PCRE_NOTEMPTY

Data *Regexp::clone(void *where) const
{
    Regexp *result = new (where) Regexp((char*)this, options);
    //DEBLOG(cerr << "Regexp::clone " << *this << " -> " << *result << '\n');
    return result;
}

// virtual constructor
Data *Regexp::mutate(void *where) const
{
    if (refcount() <= 1) {
        //DEBLOG(cerr << "Regexp::mutate " << *this << " -> NO CHANGE\n");
        return (Data*)this;
    }
    
    return clone(where);
}

void Regexp::getinfo() {
    nr_captures = pcre_info(compiled, NULL, NULL);
    if (nr_captures < 0) {
        // failed
        Slot errmsg;
        switch (nr_captures) {
            case PCRE_ERROR_NOMATCH:
                return;
            case PCRE_ERROR_NULL:
                errmsg = "Null argument"; break;
            case PCRE_ERROR_BADOPTION:
                errmsg = "bad option"; break;
            case PCRE_ERROR_BADMAGIC:
                errmsg = "bad magic"; break;
            case PCRE_ERROR_UNKNOWN_NODE:
                errmsg = "unknown node"; break;
            case PCRE_ERROR_NOMEMORY:
                
            default:
                errmsg = "unknown error"; break;
        }
        throw new Error("regexp", this, errmsg);
    }
    nr_captures = (nr_captures+1)*3;
}

void Regexp::init()
{
    pcre_malloc = malloc;
    pcre_free = free;
    inited = true;
}

Regexp::Regexp(const Slot &arg)
        : String((char*)arg),
          options(default_options),
          studied(false)
{
    char const *errptr;
    int err;
    if (!inited)
        init();
    compiled = pcre_compile((char*)*this, options, &errptr, &err, NULL);
    if (!compiled) {
        // got an error
        throw new Error("regexp", err, errptr);
    }
    getinfo();
}

Regexp::Regexp(char const *arg, int _options)
        : String(arg),
          options(_options),
          studied(false)
{
    char const *errptr;
    int err;
    if (!inited)
        init();

    compiled = pcre_compile((char*)*this, options, &errptr, &err, NULL);
    if (!compiled) {
        // got an error
        throw new Error("regexp", err, errptr);
    }
    getinfo();
}

Regexp::~Regexp()
{
    if (compiled)
        free(compiled);

    if (extra)
        free(extra);
}

Slot Regexp::positive() const
{
    if (!studied) {
        char const *errmsg;
        extra = pcre_study(compiled, 0, &errmsg);
        studied = true;
        if (errmsg) {
            throw new Error("regexp", this, errmsg);
        }
    }
    return this;
}

Slot Regexp::divide(const Slot &arg) const
{
    const int perm = (PCRE_ANCHORED | PCRE_NOTBOL | PCRE_NOTEOL | PCRE_NOTEMPTY);
    int ovector[nr_captures];
    char *subject = (char*)arg;
    int result =  pcre_exec(compiled, extra,
                            subject, arg->length(), 0,
                            options & perm,
                            ovector, nr_captures);
    DEBLOG(cerr << "regexp: " << (char*)(*(Regexp*)(mutate()))
           << '/' << (const char*)arg
           << "->" << result << '\n');

    if (result < 0) {
        // failed
        char const *errmsg;
        switch (result) {
            case PCRE_ERROR_NOMATCH:
                return new List();	// not strictly an error
                
            case PCRE_ERROR_NULL:
                errmsg = "Null argument"; break;
            case PCRE_ERROR_BADOPTION:
                errmsg = "bad option"; break;
            case PCRE_ERROR_BADMAGIC:
                errmsg = "bad magic"; break;
            case PCRE_ERROR_UNKNOWN_NODE:
                errmsg = "unknown node"; break;
            case PCRE_ERROR_NOMEMORY:
                
            default:
                errmsg = "unknown error"; break;
        }
        throw new Error("regexp", this, errmsg);
    } else {
        if (result == 0) {
            throw new Error("regexp", this, "Ran out of matchspace");
        }

        Slot matches = new List(result + 1);
        String *str = dynamic_cast<String*>((Data*)arg);

        for (int i = 0; i < result; i++) {
            int start = ovector[i*2];
            int len = ovector[i*2 + 1] - start;

            DEBLOG(cerr << "(" << start
                   << "," << len
                   << ")/(" << ovector[i*2]
                   << "," << ovector[i*2 + 1]
                   << ") ");

            if (start == -1) {
                matches[i] = Slot();
            } else {
                if (str)
                    matches = matches->insert(new String(str, start, len));
                else
                    matches = matches->insert(new String(subject + start, len));
            }
        }
        DEBLOG(cerr << " => " << matches << '\n');
        return matches;
    }
}
