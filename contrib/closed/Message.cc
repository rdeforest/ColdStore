// Message - Message object
// Copyright (C) 2000,2001 Colin McCormack, Ryan Daum
//
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// $Id

//#define DEBUGLOG
#include "Data.hh"
#include "Message.hh"
#include "List.hh"
#include "Node.hh"
#include "Error.hh"

/** create blank msg
 */
Message::Message() : Tuple(5)
{
  _visited = new List();
}

/** copy frame
 */
Message::Message(const Message *msg)
  : Tuple(4),
    _caller(msg->_caller),
    _this(msg->_this),
    _selector(msg->_selector),
    _arguments(msg->_arguments),
    _metaobjects(msg->_metaobjects),
    _visited(msg->_visited)
{}

/** new Message construction with args (what do the args mean?)
 */
Message::Message(const Slot &source) : Tuple(5)
{
  _visited = new List();
  _metaobjects = new List();
}

Message::Message( const Slot &caller,
		  const Slot &self,
		  const Slot &selector,
		  const Slot &arguments,
		  const Slot &metaobjects ) 
  : Tuple(5),
    _caller( caller ),
    _this(self),
    _selector(selector),
    _arguments(arguments),
    _metaobjects(metaobjects),
    _visited(new List())
{}

/////////////////////////////////////
// argument handling

Slot Message::nextArgument()
{
    Slot node = _arguments[0];
    _arguments = _arguments->del(0);

    return node;
}

Slot Message::getArguments()
{
    return _arguments;
}

void Message::reduce( Slot &inexpr, Frame *context ) 
{
  Message *that = new Message(this);

  // reduce _this, first!
  Node::Reduce( that->_this, context );

  // now reduce the arguments
  int a_len = that->_arguments->length();
  Slot new_args = new Tuple( a_len );
  for (int i=0;i<that->_arguments->length();i++) {
    Slot x = that->_arguments->slice(i);
    Node::Reduce( x, context );
    new_args->replace( i, x );
  }

  that->_arguments = new_args;

  // now send the message
  Slot result;
  result = that->_this->receive( that, context );
  inexpr = result;
}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Message::clone(void *where) const
{
    //  return new (_TupleBase<Slot>::Length(), where) Message(*this);
    return new Message(this);
}

// returns a mutable copy of this
Data *Message::mutate(void *where) const
{
    return (Data*)this;
}
