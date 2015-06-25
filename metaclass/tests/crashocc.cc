// Define NOCRASH to crash occ OpenC++ version 2.5.7
// LD_LIBRARY_PATH=. occ -DNOCRASH -SColdClass -- -c crashocc.cc
// LD_LIBRARY_PATH=. occ -SColdClass -- -c crashocc.cc


#ifndef NOCRASH
class exception {
public:
  exception () { }
  virtual ~exception () { }
  virtual const char* what () const;
};

class bad_exception : public exception {
public:
  bad_exception () { }
  virtual ~bad_exception () { }
};

class type_info
{
public:
    bool operator== (const type_info& arg) const;
};

#else
class type_info
{
public:
    bool operator== (const type_info& arg) const;
};

class exception {
public:
  exception () { }
  virtual ~exception () { }
  virtual const char* what () const;
};

class bad_exception : public exception {
public:
  bad_exception () { }
  virtual ~bad_exception () { }
};

#endif

