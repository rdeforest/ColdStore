// Message - Messsage object 
// Copyright (C) 2000,2001 Colin McCormack, Ryan Daum 
// 
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// $Id 

#ifndef MESSAGE_HH
#define MESSAGE_HH

#include "List.hh"
#include "Tuple.hh"

/** Message: represents all context about a message invocation
 *
 *  contains: caller, this, selector, arguments, and a visited list 
 */

cold class Message
  : public Tuple
{
  
public:
  Slot _caller;         // frame of caller
  Slot _this;	        // self in evaluation
  Slot _selector;       // message selector
  Slot _arguments;	// argument list
  Slot _metaobjects;    // meta objects for lookup
  Slot _visited;        // objects that have received this
			// message and ignored it

public:
  /////////////////
  // Construction

  /** construct a Message (for Data::New)
   *  @param source closure
   */
  Message(const Slot &source);

  /** construct a Message from an existing Message
   *  @param frame an existing Message to copy
   */
  Message(const Message *msg );

  /** construct a blank Message
   */
  Message();

  /** construct a Message with values filled
   */
  Message( const Slot &caller,
	   const Slot &self,
	   const Slot &selector,
	   const Slot &arguments,
	   const Slot &metaobjects = new List() );

public: 
  /** reduce this message to result of call
   *  i.e. send it
   */
  virtual void reduce( Slot &inexpr, Frame *context ) ; 

  virtual bool isAtomic() const { return false; };

public:
  /** set the arguments list for this Message
   *  @param args the Message arguments
   */
  virtual void setArguments( Slot args ) { _arguments = args; };
  
  /** get the current arguments list for this Message
   *  @return arguments
   */
  virtual Slot getArguments();
  
  /** pop off the next argument in the list
   *  @return the next argument
   */
  virtual Slot nextArgument();
  
public:
  ///////////////////////////////
  // Coldmud Interface

  // structural
  virtual Data *clone(void *store = (void*)0) const;
  virtual Data *mutate(void *where) const;

};

#endif

