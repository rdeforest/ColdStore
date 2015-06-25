# 1 "coldtest.cc"
# 1 "/usr/lib/gcc-lib/i386-linux/2.95.2/include/typeinfo" 1 3
 
 




#pragma interface "typeinfo"

# 1 "/usr/lib/gcc-lib/i386-linux/2.95.2/include/exception" 1 3
 
 




#pragma interface "exception"
extern "C++" {

namespace std {

class exception {
public:
  exception () { }
  virtual ~exception () { }
  virtual const char* what () const;
};

#if 0
class bad_exception : public exception {
public:
  bad_exception () { }
  virtual ~bad_exception () { }
};
#endif

typedef void (*terminate_handler) ();
typedef void (*unexpected_handler) ();

terminate_handler set_terminate (terminate_handler);
void terminate () __attribute__ ((__noreturn__));
unexpected_handler set_unexpected (unexpected_handler);
void unexpected () __attribute__ ((__noreturn__));
bool uncaught_exception ();
}  

}  

# 9 "/usr/lib/gcc-lib/i386-linux/2.95.2/include/typeinfo" 2 3


extern "C++" {

namespace std {

class type_info {
private:
   
  type_info& operator= (const type_info&);
  type_info (const type_info&);

protected:
  explicit type_info (const char *n): _name (n) { }

  const char *_name;

public:
   
  virtual ~type_info ();
    
  bool before (const type_info& arg) const;
  const char* name () const
    { return _name; }
  bool operator== (const type_info& arg) const;
  bool operator!= (const type_info& arg) const;
};

inline bool type_info::
operator!= (const type_info& arg) const
{
  return !operator== (arg);
}

#if 0
class bad_cast : public exception {
public:
  bad_cast() { }
  virtual ~bad_cast() { }
};

class bad_typeid : public exception {
 public:
  bad_typeid () { }
  virtual ~bad_typeid () { }
};
#endif

}  

}  

# 1 "coldtest.cc" 2


//metaclass ColdClass Base;
//metaclass ColdClass Derived;

class Unknown;
# 15 "coldtest.cc"


class Base {
    public:
    virtual bool isAKO(Unknown *arg);
};

struct Derived
: public Base
{
};
