// Frame - Frame object 
// Copyright (C) 2000,2001 Colin McCormack, Ryan Daum 
// 
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// $Id 

#ifndef FRAME_HH
#define FRAME_HH

#include "Data.hh"
#include "Message.hh"

/** an entry in the stack frame
 *
 * Frame stores: execution history, arguments, "this", selector etc.
 */

cold class Frame
  : public Message
{

public:
  Slot _top;	        // top of the frame chain
  Slot _closure;        // resolve closure of method
  Slot _error;          // error in frame
  Slot _line;           // line of last evaluation

public:
  /////////////////
  // Construction

  /** construct a Frame from an existing Frame
   *  @param frame an existing Frame to copy
   */
  Frame(const Frame *frame);

  /** construct a new Frame from a message
   *  and chain to old frame
   */
  Frame( const Message *msg, const Frame *frm, const Slot &closure = (Data*)NULL );

  /** construct a blank Frame
   */
  Frame();

  /** construct a Frame from a Closure
   *  @param source closure
   */
  Frame(const Slot &source);

public:
  ///////////////////
  // Call interface

  /** resume reduction on a Frame after suspension
   *  @param a return value for the executed frame
   */
  virtual Slot resume();
  
  /** traceback report
   *  @return a string of a traceback for this frame
   */
  virtual Slot traceback();
     
public:
  ///////////////////////////////
  // Coldmud Interface

  // structural
  virtual Data *clone(void *store = (void*)0) const;
  virtual Data *mutate(void *where) const;

};

#endif

