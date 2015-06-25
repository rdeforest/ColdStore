// MetaData - MetaObject for exposing layer1 facilities via messages.  singleton
// Copyright (C) 2001 Ryan Daum
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#include "Data.hh"
#include "Frame.hh"
#include "Closure.hh"
#include "Message.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "Object.hh"
#include "MetaData.hh"

#define MONADIC_MTHD( SELECTOR ) if ( msg->_selector.equal("" #SELECTOR "") ) return this->SELECTOR();
#define UNARY_MTHD( SELECTOR ) if ( msg->_selector.equal("" #SELECTOR "") ) return this->SELECTOR( msg->_arguments[0] );
#define BINARY_MTHD( SELECTOR ) if ( msg->_selector.equal("" #SELECTOR "") ) return this->SELECTOR( msg->_arguments[0], msg->_arguments[1] );
#define TRIADIC_MTHD( SELECTOR ) if ( msg->_selector.equal("" #SELECTOR "") ) return this->SELECTOR( msg->_arguments[0], msg->_arguments[1], msg->_arguments[2] );

Slot MetaData::instance = (Data*)NULL;

// receive message -- one big honkin' chunk of if statements. eww
Slot MetaData::receive( Message *msg, Frame *context )
{

  try {
    return this->Object::receive( msg, context );
  } catch (Error *e) {}

  // expose Layer1 VP to execution via selectors
  if ( msg->_selector.equal("+") )
    return ((Slot)msg->_this) + msg->_arguments[0];
  
  if ( msg->_selector.equal("-") )
    return ((Slot)msg->_this) - msg->_arguments[0];
  
  if ( msg->_selector.equal("*") )
    return ((Slot)msg->_this) * msg->_arguments[0];
  
  if ( msg->_selector.equal("/") )
    return ((Slot)msg->_this) / msg->_arguments[0];
  
  if ( msg->_selector.equal("%") )
    return ((Slot)msg->_this) % msg->_arguments[0];
  
  if ( msg->_selector.equal("!") )
    return (!(Slot)msg->_this);
  
  if ( msg->_selector.equal("==") )
    return (msg->_this->equal( msg->_arguments[0] ) );
  
  if ( msg->_selector.equal("!=") )
    return (!msg->_this->equal( msg->_arguments[0] ) );
  
  if ( msg->_selector.equal(">"))
    return msg->_this > msg->_arguments[0] ;
  
  if ( msg->_selector.equal("<"))
    return msg->_this < msg->_arguments[0] ;
  
  if ( msg->_selector.equal("<=") || msg->_selector.equal("=<") )
    return msg->_this <= msg->_arguments[0] ;
  
  if ( msg->_selector.equal(">=") || msg->_selector.equal("=>") )
    return msg->_this >= msg->_arguments[0];

  if ( msg->_selector.equal("length") )
    return msg->_this->length();

  if ( msg->_selector.equal("slice_range") )
    return msg->_this->slice( msg->_arguments[0], msg->_arguments[1] );

  if ( msg->_selector.equal("replace_with") )
    return msg->_this->replace( msg->_arguments[0], msg->_arguments[1] );

  if ( msg->_selector.equal("ifTrue")) {
    if (msg->_this->truth()) {
      Slot doBranch = msg->_arguments[0];
      Message *invoke = new Message( context, doBranch, "reduce", new List(), new List() );
      return doBranch->receive( invoke, context );
    } else 
      return (Data*)NULL; 
  }

  if ( msg->_selector.equal("ifFalse")) {
    if (!msg->_this->truth()) {
      Slot doBranch = msg->_arguments[0];
      Message *invoke = new Message( context, doBranch, "reduce", new List(), new List() );
      return doBranch->receive( invoke, context );
    } else 
      return (Data*)NULL; 
  }

  if ( msg->_selector.equal("ifTrue_else") || msg->_selector.equal("ifTrue_ifFalse") ) {
    Slot doBranch;
    if (msg->_this->truth())
      doBranch = msg->_arguments[0];
    else 
      doBranch = msg->_arguments[1];

    Message *invoke = new Message( context, doBranch, "reduce", new List(), new List() );
    return doBranch->receive( invoke, context );
  }

  if ( msg->_selector.equal("ifFalse_else") || msg->_selector.equal("ifFalse_ifTrue:") ) {
    Slot doBranch;
    if (!msg->_this->truth())
      doBranch = msg->_arguments[0];
    else 
      doBranch = msg->_arguments[1];

    Message *invoke = new Message( context, doBranch, "reduce", new List(), new List() );
    return doBranch->receive( invoke, context );
  }

  // sequence map: { closure } -- first argument to closure is element in sequence,
  // returns sequence of results
  if ( msg->_selector.equal("map") ) {
    Slot results = new List();

    Slot it = msg->_this->iterator();
    while (it->More()) {
      Slot doBranch( new Closure( ((Closure*)(Data*)msg->_arguments[0]) ) );
      Slot args = new List();
      args->insert( it->Next() );

      // unlike if statements we need to create a new frame for msg->_this.
      Frame *call_frame = new Frame( msg, context, doBranch );
      call_frame->_arguments = args;

      // we invoke because we want to be able to return values from blocks without
      // returning from method
      Message *invoke = new Message( call_frame, doBranch, "invoke", args );

      results->insert( doBranch->receive( invoke, call_frame ) );
    }
    return results;
  }
  
  // instead of calling a block closure for each element in sequence, call
  // a method by name
  if (msg->_selector.equal("mapMethod")) {
    Slot m_name = msg->_arguments[0];
    Slot results = new List();

    Slot it = msg->_this->iterator();
    while (it->More()) {
      Slot o = it->Next();
      results->insert(o->receive( new Message( context, o, m_name, new List() ), context ));
    }
    return results;
  }

  UNARY_MTHD( create );
  MONADIC_MTHD( positive );
  MONADIC_MTHD( negative );
  UNARY_MTHD( add );
  UNARY_MTHD( subtract );
  UNARY_MTHD( multiply );
  UNARY_MTHD( divide );
  UNARY_MTHD( modulo );
  MONADIC_MTHD( invert );
  UNARY_MTHD( and );
  UNARY_MTHD( xor );
  UNARY_MTHD( or );
  UNARY_MTHD( lshift );
  UNARY_MTHD( rshift );
  UNARY_MTHD( concat );
  UNARY_MTHD( slice );
  UNARY_MTHD( search );
  UNARY_MTHD( replace );
  UNARY_MTHD( insert );
  UNARY_MTHD( del );
  MONADIC_MTHD( iterator );
  MONADIC_MTHD( Next );
  MONADIC_MTHD( More );
  UNARY_MTHD( connect );
  UNARY_MTHD( disconnect );
  UNARY_MTHD( incoming );
  UNARY_MTHD( outgoing );

  throw new Error("selector", msg->_selector, "selector not found.");
}
