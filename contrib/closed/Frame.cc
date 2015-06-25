// Frame - Frame object
// Copyright (C) 2000,2001 Colin McCormack, Ryan Daum
//
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// $Id

//#define DEBUGLOG
#include "Data.hh"
#include "Store.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"
#include "Hash.hh"
#include "Symbol.hh"
#include "Frame.hh"
#include "Node.hh"
#include "InterpreterNodes.hh"
#include "Closure.hh"
#include "Object.hh"

#include <stdio.h>

#include <strstream>

/** create blank frame
 */
Frame::Frame() : 
  Message(4)
{}

Frame::Frame( const Message *msg, const Frame *frm, const Slot &closure ) 
  : Message( msg )
{
  if (frm)
    _top = frm->_top;
  _caller = frm;
  _closure = closure;
}

/** copy frame
 */
Frame::Frame(const Frame *frame)
{
  _top = frame->_top;
  _closure = frame->_closure;
  _error = frame->_error;
  _line = frame->_line;
  _this = frame->_this;
  _selector = frame->_selector;
  _arguments = frame->_arguments;
  _caller = frame->_caller;
}

/** new frame from closure
 */
Frame::Frame(const Slot &source)
  : Message()
{
  _closure = source;
}


/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Frame::clone(void *where) const
{
    //  return new (_TupleBase<Slot>::Length(), where) Frame(*this);
    return new Frame(this);
}

// returns a mutable copy of this
Data *Frame::mutate(void *where) const
{
    return (Data*)this;
}

///////////////////
// Call interface


Slot Frame::resume()
{
  // continue reduction where we left off
  _closure->reduce( _closure, this );
  return _closure;
}


Slot Frame::traceback()
{
  ostrstream out;
  Slot e = _error;

  out << e << " raised at: " << endl;

  // make copy of frame
  Frame *f_c = new Frame(this);

  while (f_c) {

    // output the object id, or its address if it doesn't have one
    Slot origin = f_c->_this;
    if (origin) {
      try {
	Slot id = origin->slice( new IVKey( "id", origin) );
	out << "$" << (char*)id[2];
      } catch (Error *e) {
	out.form( "{0x%08x}", (Data*)origin );
      }

      out << ".";
      // output the frame/method name
      Closure *closure = ((Closure*)(Data*)f_c->_closure);
      if (closure) {
	out << (char*)f_c->_selector;
	if (closure->_definer) {
	  out << " (defined on ";

	  Slot d_id = closure->_definer->search("id");
	  
	  if (d_id)
	    out << "$" << (char*)d_id[2];
	  else
	    out.form( "{0x%08x}", (Data*)closure->_definer );
	
	  out << ")" ;
	}
      } else 
	out << "none";  // shouldn't happen
      
      
      // output the frame line number
      out << " @ line #" << f_c->_line;
    }
    f_c = ((Frame*)(Data*)f_c->_caller);
    if (f_c && origin)
      out << " called from ... ";
    if (f_c)
      out << endl;
  }
  out << ends;
  return out.str();
}

