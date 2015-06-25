// File - class for local file access
// Copyright (C) 2000,2001 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id
//#define DEBUGLOG
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Data.hh"
#include "String.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"
#include "File.hh"

#include <Q.th>
template class Q<File>;
template class Qh<File>;
template class Qi<File>;

#include "tSlot.th"
template union tSlot<File>;

File::File(const Slot &file)
        : Q<File>(File::all),
	  name(file[0]),
          mode(file[1]),
          prot((file->length()>2)?(const int)file[2]:0664),
          stream(new fstream(name, mode, prot))
{
  if (!stream)
    throw new Error("file", this, "open failed");
}

File::File(const char *_name, int _mode, int _prot)
        : Q<File>(File::all),
	  name(_name), mode(_mode), prot(_prot),
          stream(new fstream(_name, _mode, _prot))
{
  if (!stream)
    throw new Error("file", this, "open failed");

  DEBLOG(cerr << "new File: " << name << stream << '\n');
}

File::File(fstream &f)
  : Q<File>(File::all),
    name(""),
    mode(0),
    prot(0),
    stream(&f)
{
  if (!stream)
    throw new Error("file", this, "open failed");
  DEBLOG(cerr << "new File: " << name << stream << '\n');
}
	   

File::~File()
{}

File *File::clone(void *where) const
{
    return new (where) File(name[0], name[1],
                            (name->length()>2)?(int)name[2]:0644);
}

File *File::mutate(void *where) const
{
    return (File*)this;
}

bool File::truth() const
{
    return !stream->fail();
}

Slot File::toconstruct() const
{
    return name;
}

ostream &File::dump(ostream& output) const
{
    output << name;
    return output;
}

Slot File::construct(const Slot &arg)
{
    return new File(arg);
}

int File::order(const Slot &arg) const
{
    int cmp = typeOrder(arg);
    if (cmp)
        return cmp;
    
    File *other = dynamic_cast<File*>((Data*)arg);

    return name->order(other->name);
}

bool File::equal(const Slot &arg) const
{
    return order(arg) == 0;
}

int File::length() const
{
    return 0; // return file length
}

Slot File::slice(const Slot &from, Slot &len) const
{
    int pos = (int)from;

    if (pos < 0) {
        stream->seekg(pos, ios::end);
    } else {
        stream->seekg(pos);
    }
    if (stream->fail()) {
        throw new Error("file", this, "seek failed");
    }
    return slice(len);
}

Slot File::slice(const Slot &len) const
{
    int l = (int)len;
    if (l < 0)
        throw new Error("domain", len, "negative read length");
    String *result = new String(l);
    result->vconcat((Char*)"",l);	// populate the String
    stream->read(result->content(), l);	// read the string
    if (stream->fail()) {
        // read error
        throw new Error("file", this, "read failed");
    } else {
        // trim result
        return lastread = result;
    }
}

Slot File::replace(const Slot &value, const Slot &from)
{
    int pos = (int)from;
    if (pos < 0) {
        stream->seekp(pos, ios::end);
    } else {
        stream->seekp(pos);
    }
    if (stream->fail()) {
        throw new Error("file", this, "seek failed");
    }
    return replace(value);
}

Slot File::replace(const Slot &value)
{
    String *data = (String*)value;
    stream->write((char*)data, data->length());	// write the string
    if (stream->fail()) {
        // read error
        throw new Error("file", this, "write failed");
    } else {
        // trim result
        return this;
    }
}

Slot File::iterator() const
{
    return mutate();
}

bool File::isIterator() const
{
    return true;
}

Slot File::Next()
{
    char *line;
    stream->gets(&line);
    if (line) {
        DEBLOG(cerr << "got: " << line << " ");
        Slot l = new String(line);
        DEBLOG(l->dump(cerr) << "\n");
        lastread = l;
        free(line);
        return lastread;
    } else {
        throw new Error("range", this, "file iterator exhausted");
    }
}

bool File::More() const
{
    return !stream->eof();
}

Slot File::read_file()
{
  tSlot<List> whole;
  while (More()) {
    whole = whole->insert(-1, Next());
  }
  return whole;
}
