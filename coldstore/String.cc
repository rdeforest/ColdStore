// String - Coldstore dynamic strings
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused)) = "$Id: String.cc,v 1.36 2002/03/15 15:31:45 coldstore Exp $";
//#define DEBUGLOG

#include <ctype.h>
#include <string.h>

#include "Data.hh"
#include "Store.hh"

#include "Vector.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"

#include "String.hh"

void *
memmem (
     const void *haystack,
     size_t haystack_len,
     const void *needle,
     size_t needle_len
	)
{
  const char *begin;
  const char *const last_possible
    = (const char *) haystack + haystack_len - needle_len;

  if (needle_len == 0)
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string.  */
    return (void *) haystack;

  for (begin = (const char *) haystack; begin <= last_possible; ++begin)
    if (begin[0] == ((const char *) needle)[0] &&
	!memcmp ((const void *) &begin[1],
		 (const void *) ((const char *) needle + 1),
		 needle_len - 1))
      return (void *) begin;

  return NULL;
}

union uChar;	// forward decl

union Char {
public:
    friend ostream& operator<< (ostream&, const Char&);
    char c;

    Char(): c('\0') {}
    Char(char _c): c(_c) {}
    Char(Char *_c): c((char)*_c) {}
    Char(int _c): c((char) _c) {}

    operator const char* () const {return &c;}
    operator const char &() const {return c;}
    static int order(const Char &_c1, const Char &_c2) {return _c1.c - _c2.c;}
    bool operator==(const Char &_c) const {return c == _c.c;}
    bool operator==(char _c) const {return c == _c;}
    bool operator!=(const Char &_c) const {return c != _c.c;}
    bool operator!=(char _c) const {return c != _c;}
    
    // move into this range
    // (the original is going to be deallocated)
    static Char *move(Char *to, const Char *from, size_t range) {
        return (Char *)memmove(to, from, range);
    }
    
    // move into this range
    // (the original is going to be deallocated)
    static Char *dup(Char *to, const Char *from, size_t range) {
        DEBLOG(cerr << "Char::dup ");
        DEBLOG(cerr.form("[0x%08x]", to));
        DEBLOG(cerr.form("[0x%08x]", from));
        DEBLOG(cerr << ", " << range << '\n');
        Char *result = (Char *)memmove(to, from, range);
        return result;
    }
    
    // fill a segment with a repetitive pattern
    static Char *segFill(Char *from, const char &content, int repetition) {
        return (Char *)memset(from, content, repetition);
    }
    
    // destroy the elements of the range
    static void destroy(Char *, size_t) {}
    
    // initialise default elements in range
    static void init(Char *from, size_t range = 1) {
        memset(from, 0, range);
    }
    
    // find offset of leftmost NULL element in range 
    static int findNull(const Char *from) {
        return strlen((char *)from);
    }
    
    // consistency checker for char* (none, as it happens)
    static void check(const Char *, size_t=0) {}
    
    // find a single matching character
    static Char *find(Char *from, Char _c, size_t range) {
        DEBLOG(cerr << "Char::find('" << (char)_c
               << "', " << (char*)from << ")\n");
        return (Char *)memchr(from, _c.c, range);
    }
    
    // find a needle in this haystack
    static Char *search(const Char *haystack,
                        const Char *needle,
                        size_t haystack_len, size_t needle_len) {
        DEBLOG(cerr << (char*)haystack << "->Char::search('" 
               << (char*)needle << "', " << needle_len << ", " << haystack_len << ")");
        Char *result;
        if (needle_len > 1)
            result = (Char *)memmem(haystack, haystack_len, needle, needle_len);
        else
            result = (Char *)memchr(haystack, *needle, haystack_len);
        DEBLOG(cerr << " -> " << result << '\n');
        return result;
    }
    
    // compare two Char strings
    int compare(const Char *_str, size_t len) const {
        // reduction in strength - strong equality
        if (_str == this)
            return 0;
        return strncmp(&c, &(_str->c), len);
    }
    
    // compare a Char string
    int compare(const uChar *_str, size_t len) const;
};

// as Char above, but all comparisons are non case-sensitive
union uChar
{
public:
    friend ostream& operator<< (ostream&, const Char&);
    char c;
    
    uChar(): c('\0') {}
    uChar(char _c): c(_c) {}
    uChar(Char *_c): c((char)*_c) {}
    uChar(int _c): c((char) _c) {}
    
    static int order(const uChar &_c1, const uChar &_c2) {
        return tolower(_c1.c) - tolower(_c2.c);
    }
    bool operator==(const uChar &_c) const {
        return tolower(c) == tolower(_c.c);
    }
    bool operator!=(const uChar &_c) const {
        return tolower(c) != tolower(_c.c);
    }
    bool operator==(const char &_c) const {return c == _c;}
    bool operator!=(const char &_c) const {return c != _c;}
    
    // compare a Char string
    inline int uChar::compare(const Char *_str, size_t len) const {
        // reduction in strength - strong equality
        if ((uChar*)_str == this)
            return 0;
        return strncasecmp(&c, &(_str->c), len);
    }
    
    
    // compare two uChar strings
    int compare(const uChar *_str, size_t len) const {
        // reduction in strength - strong equality
        if (_str == this)
            return 0;
        return strncasecmp(&c, &(_str->c), len);
    }
    
    // find a single matching character, ignoring case
    static const uChar *find(const uChar *from, uChar _c, size_t range) {
        DEBLOG(cerr << (char*)from
               << "->uChar::find('" << (char)_c.c
               << "', " << range
               << ")\n");
        for (const uChar *s = from; (size_t)(s - from) < range; s++) {
            if (*s == _c)
                return s;
        }
        return NULL;
    }
    
    // find a needle in this haystack, ignoring case
    static const uChar *search(const uChar *from,
                               const uChar *_str,
                               size_t len, size_t range) {
        DEBLOG(cerr << (char*)&from->c << "->uChar::search("
               << (char*)_str
               << ", " << len
               << ", " << range
               << ")\n");
        for (const uChar *p = find(from, *_str, range);
             p;
             p = find(p+1, *_str, range - (p - from)))
            {
                if (p->compare(_str, len) == 0)
                    return p;
            }
        
        return NULL;
    }
    
    // move into this range
    // (the original is going to be deallocated)
    static uChar *move(uChar *to, const uChar *from, size_t range) {
        return (uChar *)memmove(to, from, range);
    }
    
    // move into this range
    // (the original is going to be deallocated)
    static uChar *dup(uChar *to, const uChar *from, size_t range) {
        return (uChar *)memmove(to, from, range);
    }
    
    // fill a segment with a repetitive pattern
    static uChar *segFill(uChar *from, const uChar &content, int repetition) {
        return (uChar *)memset(from, content.c, repetition);
    }
    
    // destroy the elements of the range
    static void destroy(uChar *from, size_t) {}
    
    // initialise default elements in range
    static void init(uChar *from, size_t range = 1) {
        memset(from, 0, range);
    }
    
    // find offset of leftmost NULL element in range 
    static int findNull(const uChar *from) {
        return strlen((char *)from);
    }
    
    // consistency checker for char* (none, as it happens)
    static void check(const uChar*,size_t) {}
    
    // find a single matching character
    static uChar *find(uChar *from, Char _c, size_t range) {
        DEBLOG(cerr << (char*)from
               << "->uChar::find('" << (char)_c
               << "', " << range
               << ")\n");
        return (uChar *)memchr(from, _c.c, range);
    }
};

#include "tSlot.th"
template union tSlot<String>;
template union tSlot<UString>;


#include "Segment.th"
template class Segment<Char>;
#include "TupleBase.th"
template class TupleBase<Char>;
template class ATupleBase<Char>;
#include "Vector.th"
template class Vector<Char>;

unsigned int Vector<Char>::minimum_size = 4;
unsigned int Vector<Char>::extra_allocation = 4;
unsigned int Vector<Char>::minimum_occupancy = 4;
//ostream& operator<< (ostream&, const Vector<Char> &);

template class Segment<uChar>;
template class TupleBase<uChar>;
template class ATupleBase<uChar>;
template class Vector<uChar>;

unsigned int Vector<uChar>::minimum_size = 4;
unsigned int Vector<uChar>::extra_allocation = 4;
unsigned int Vector<uChar>::minimum_occupancy = 4;
//ostream& operator<< (ostream&, const Vector<uChar> &);

// String constructors
String::String(const char *str, int len)
    : Vector<Char>((Char*)str, (len >= 0)?len:strlen(str)) {}

String::String(size_t size)
    : Vector<Char>(size) {}

String::String(const String &str, ssize_t start, int l)
    : Vector<Char>(str, start, l) {}

String::String(const String *str, ssize_t start, int l)
    : Vector<Char>(str, start, l) {}

String::String(const Slot& s)
    : Vector<Char>((String*)s[0],(int)s[1],(int)s[2]) {}

#if 0
String::String(TupleBase<Char> *ts)
    : Vector<Char>(ts) {}
#endif

String::~String() {}

void String::check(int) const
{
    //validGuard();
    Data::check();	// check memory integrity
    Vector<Char>::check();
}

// the object's truth value
bool String::truth() const
{
    return !empty();
}

// constructor args to recreate object
Slot String::toconstruct() const
{
    const int len = Length();
    Slot result = new List(len);	// make a guess as to length
    char *accum = (char*)0;
    int acclen = 0;
    for (int i=0; i < len; i++) {
        const char c = element(i);
        if (isprint(c)) {
            if (accum) {
                // add to accumulator
                acclen++;
            } else {
                // create an accumulator
                accum = (char*)&element(i);
                acclen = 1;
            }
        } else {
            if (accum) {
                // we've accumulated printables
                result = result->add(new String(accum, acclen));
                accum = (char*)0;
                acclen = 0;
            }
            result = result->add((int)c);	// add the unprintable as an integer
        }
    }
    return result;
}

#ifndef strnlen
size_t
strnlen (const char *string, size_t maxlen)
{
  const char *end = (char*)memchr (string, '\0', maxlen);
  return end ? end - string : maxlen;
}
#endif

//#ifdef 0
#undef strndupa
// Return an alloca'd copy of at most N bytes of string.
// duplicated here to fix implicit cast from void*
#define strndupa(s, n)						      \
  (__extension__						      \
    ({								      \
      __const char *__old = (s);				      \
      size_t __len = strnlen (__old, (n));			      \
      char *__new = (char*)__builtin_alloca (__len + 1);	      \
      __new[__len] = '\0';					      \
      (char*)memcpy (__new, __old, __len);			      \
    }))
//#endif

ostream &String::dump(ostream& out) const
{
    if (this) {
        const int len = Length();
        char *accum = (char*)0;
        int acclen = 0;
        
        out << "'";
        
        for (int i=0; i < len; i++) {
            const char c = element(i);
            if (isprint(c) && (c != '\'')) {
                if (accum) {
                    // add to accumulator
                    acclen++;
                } else {
                    // create an accumulator
                    accum = (char*)&element(i);
                    acclen = 1;
                }
            } else {
                if (accum) {
                    // we've accumulated printables
                    char *tmp = (char*)strndupa(accum, acclen);
                    out << tmp;
                    accum = (char*)0;
                    acclen = 0;
                }
                if (c == '\'') 
                    out << "\\'";
                else
                    out.form("\\x%02x", c);
            }
        }
        if (accum) {
            // we've accumulated printables
            char *tmp = (char*)strndupa(accum, acclen);
            out << tmp;
            accum = (char*)0;
            acclen = 0;
        }
        out << "'";
    } else {
        out << "[NULL]";
    }
    return out;
}

int String::order(const Slot &d) const
{
  if(!AKO(d,String))
    {
      int cmp = typeOrder(d);
      if (cmp)
        return cmp;
    }
    DEBLOG(cerr << "comparing " << *this << " with " << d);
    int cmplen = MIN(Length(), (int)(d->length()));
    int result = content()->compare(((String *)d)->content(), cmplen);
    if (result == 0) {
        // strings eq as far as they compared, whichever is longer is greater
        result = Length() - (int)(d->length());
        DEBLOG(cerr << " using length ");
    }
    DEBLOG(cerr << "-> " << result << '\n');
    return result;
}

bool String::equal(const Slot &d) const
{
    if(!AKO(d,String))
      {
        if (typeOrder(d))
          return false;
      }
    Char *str = content();
    Char *other = ((String *)d)->content();
    int len = ((String*)d)->length();
    return (len == length()) && !str->compare(other, len);
}

// virtual constructor
Data *String::clone(void *where) const
{
    String *result = new (where) String(*this);
    DEBLOG(cerr << "String::clone " << *this << " -> " << *result << '\n');
    return result;
}

// virtual constructor
Data *String::mutate(void *where) const
{
    if (refcount() <= 1) {
        DEBLOG(cerr << "String::mutate " << *this << " -> NO CHANGE\n");
        return (Data*)this;
    }
    
    return clone(where);
}

// predicate - is object a sequence
bool String::isSequence() const
{
    return true;
}

// length as sequence
int String::length() const
{
    return Length();
}

// concatenate two sequences
Slot String::concat(const Slot &arg) const
{
    if (!arg)
        return this;
    
    Char *val = (Char*)(char*)arg;
    int l = arg->length();
    
    DEBLOG(cerr << "String::concat " << *this
           << " with " << arg
           << " val: " << (char *)arg
           << '\n');
    if (l) {
        String *m = (String *)mutate();
        m->Vector<Char>::vconcat(val, l);
	DEBLOG(cerr << "String::concat result:" << *m << '\n');
        return m;
    } else
        return this;
}


// dyadic `+', add
Slot String::add(const Slot &arg) const
{
    return concat(arg);
}

// return subrange of String
Slot String::slice(const Slot &from, const Slot &len) const
{
    if (!len)
        return new String(this, (int)from, 0);
    else
        return new String(this, (int)from, (int)len);
}

// return subrange of String
Slot String::slice(const Slot &from) const
{
    return new String(this, (int)from, 1);
}

// replace subrange with String
Slot String::replace(const Slot &val)
{
    return unimpl("monadic replace on String");
}

// replace subrange with String
Slot String::replace(const Slot &from, const Slot &len, const Slot &value)
{
    String *v = (String *)value;
    String *m = (String *)mutate();
    m->Vector<Char>::Replace(from, len, v->content(), v->Length());
    return m;
}

// replace item with String
Slot String::replace(const Slot &from, const Slot &val)
{
    return replace(from, 1, val);
}

// insert String
Slot String::insert(const Slot &from, const Slot &val)
{
    String *v = (String *)val;
    String *m = (String *)mutate();
    m->Vector<Char>::vinsert(from, v->content(), v->Length());
    return m;
}

// insert String
Slot String::insert(const Slot &val)
{
    String *v = (String *)val;
    String *m = (String *)mutate();
    m->Vector<Char>::vinsert(Length(), v->content(), v->Length());
    return m;
}

// delete subrange
Slot String::del(const Slot &from, const Slot &len)
{
    String *m = (String *)mutate();
    m->Vector<Char>::del(from, len);
    return m;
}

// delete element
Slot String::del(const Slot &from)
{
    return del(from, 1);
}

// dyadic `search', subrange
Slot String::search(const Slot &search) const
{
    if (search) {
        String *srch = (String*)search;
        Char *found = Char::search(content(), srch->content(),
                                   Length(), srch->Length());
        if (found) {
            DEBLOG(cerr << *this
                   << " String::search(" << *srch
                   << ")= " << found - content()
                   << '\n');
            return found - content();
        }
    }
    DEBLOG(cerr << *this
           << " String::search(" << *(String*)search 
           << ") Not Found \n");
    return (Data*)0;	// NULL result
}

// return an iterable Sequence as a List
List *String::toSequence() const
{
    // need a circumloqution to disambiguate :)
  //return new List((Slot&)*this, 1);
  List *l = new List(-1, (Data*)this);
  //(*l)[0] = this;
  l->upcount();
  return l;
}

// return an iterator
Slot String::iterator() const
{
    return new String(this);
}

bool String::More() const
{
    return truth()?1:0;
}

Slot String::Next()
{
    if (truth()) {
        Slot retval = slice(0);
        (*this)++;	// advance lower edge of Segment
        return retval;
    } else {
        throw new Error("break", this, "Iterator exhaustion");
    }
}

String::operator char *() const
{
    DEBLOG(cerr << "converting to char *:" << (char*)content() << '\n');
    return (char*)content();
}

String *String::crypt(Slot &key)
{
    return unimpl("crypt not yet implemented");
}

String *String::uppercase()
{
    return unimpl("uppercase not yet implemented");
}

String *String::lowercase()
{
    return unimpl("lowercase not yet implemented");
}

bool String::isPrefix(const Slot& s)
{
  if(!AKO(s,String)) return false;
  if((s->length()) < length()) return false;
  return memcmp((char*)*this,(char*)s,length()) == 0;
}

Slot String::killPrefix(const Slot& s)
{
  Slot x(s->length()-length());
  return s->slice(length(),x);
}

Slot String::commonPrefix(const Slot& s)
{
  int x = lenCommonPrefix(s);
  Slot n(x);
  return slice(0,n);
}

int String::lenCommonPrefix(const Slot& s)
{
  Data* d = (Data*)s;
  if(!AKO(d,String)) return 0;
  String& ss = *(String*)s;
  int x = MAX(ss.length(),length());
  int i;
  for(i=0;i<x;i++)
    {
      if(((char*)*this)[i]!=((char*)ss)[i]) break;
    }
  return i;
}

String *String::nullterm(char term)
{
  String *result = (String*)mutate();	// ensure we're able to mutate
  result->Vector<Char>::vconcat((Char)term);
  return result;
}

Slot String::positive()
{
  String *result = nullterm();
  result->upcount();
  return result;
}

Slot String::explode(const String *by) const
{
  Char *start = content();
  int len = length();
  const Char *srch = by->content();
  const int slen = by->length();
  Slot exploded = new List();
  Char *probe;
  for (probe = start; len && (probe = Char::search(start, srch, len, slen));) {
    exploded = exploded->insert(new String((char*)start,probe-start));
    DEBLOG(cerr << "explode: " << exploded << ' ' << len << '\n');
    len -= probe-start + 1;
    start = probe+1;
  }
  if (len) {
    exploded = exploded->insert(new String((char*)start, len));
    DEBLOG(cerr << "explode: " << exploded << ' ' << len << '\n');
  } else {
    exploded = exploded->insert(new String());
    DEBLOG(cerr << "explode: " << exploded << ' ' << len << '\n');
  }
  return exploded;
}

Slot String::divide(const Slot &arg) const
{
  return explode((String*)(Data*)arg);
}

UString::UString(const char *str, int len)
    : String(str, static_cast<int>(len?len:strlen(str))) {}

UString::UString(const UString &str, ssize_t start, ssize_t l)
    : String(str, start, l) {}

UString::~UString() {}

int UString::order(const Slot &d) const
{
    if (AKO(d,String)) {
        return
            ((uChar*)this)->compare((uChar*)(((String*)d)->content()),
                                    ((String*)d)->Length());
    } else {
        return typeOrder(d)
            || ((uChar*)this)->compare((uChar*)(((UString*)d)->content()),
                                       ((UString*)d)->Length());
    }
}

bool UString::equal(const Slot &d) const
{
    if (typeOrder(d)) {
        return false;
    }
    uChar *str = (uChar*)content();
    uChar *other = (uChar*)((UString *)d)->content();
    int len = ((UString*)d)->length();
    return (len == length()) && !str->compare(other, len);

#ifdef notdef
    return !((uChar*)this)->compare(((UString *)d)->content(),
                                    ((UString*)d)->length());
#endif
}

// dyadic `search', subrange
Slot UString::search(const Slot &search) const
{
    if (search) {
        UString *srch = (UString *)search;
        const uChar *found = uChar::search((uChar*)content(),
                                           (uChar*)(srch->content()),
                                           Length(), srch->Length());
        if (found) {
            DEBLOG(cerr << *this
                   << " UString::search(" << *srch
                   << ")= " << found - (uChar*)(content())
                   << '\n');
            return found - (uChar*)content();
        }
    }
    DEBLOG(cerr << *this
           << " UString::search(" << *(String*)search
           << ") Not Found \n");
    return (Data*)0;	// NULL result
}

// constructor
Data *UString::clone(void *where) const
{
    UString *result = new (where) UString(*this);
    DEBLOG(cerr << "UString::clone " << *this
           << " -> " << *result << '\n');
    return result;
}

// dump Char to file
ostream& operator<< (ostream& out, const Char& _c)
{
    out << (char)(_c.c);
    return out;
}

// dump Char to file
ostream& operator<< (ostream& out, const uChar& _c)
{
    out << (char)(_c.c);
    return out;
}

extern bool DumpVerbose;
// dump String to file
ostream& operator<< (ostream& out,  const String &str)
{
    if (&str && (const char *)str) {
        if (DumpVerbose) {
            out.form("[0x%08x,%d] ", (const char *)str, str.Length());
        }
        out << '{';
        const Char *contents = str.content();
        for (int i = 0; i < str.Length(); i++)
            out << contents[i];
        out << '}';
    } else {
        out << "[NULL]\n";
    }
    return out;
}
