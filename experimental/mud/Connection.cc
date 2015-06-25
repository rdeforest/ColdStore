// Connection.cc: builds the $connection prototype
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <Closure.hh>
#include <String.hh>

#include <unistd.h>

#include <sys/socket.h>
#include <stdio.h>
#include "Event.hh"
#include "common.hh"

static Slot socket( Frame *context )
{
  Slot self = context->_this;

  return self.socket;
}

static Slot setSocket( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  self.socket = args[0];

  return true;
}

static Slot tell( Frame *context)
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Slot line = args[0];
  
  try {
    int sock = self.socket();
    line=line->concat("\n\0");
    char *msg = line;
    send(sock, msg, line->length(), 0);
  } catch (Error *e) {
    cerr << (char*)line << endl;
  }
  return true;
}

static Slot setWelcomeMessage( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  self.welcome_message = args[0];

  return true;
}

static Slot welcomeMessage( Frame *context )
{
  Slot self = context->_this;

  return self.welcome_message;
}

static Slot connect( Frame *context )
{
  Slot self = context->_this;
  
  self.tell( self.welcomeMessage() );
  
  return true;
}

static Slot user( Frame *context )
{
  Slot self = context->_this;

  return self.user;
}

static Slot setUser( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  self.user = args[0];

  return true;
}

static Slot disconnect( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot user = self.user();
  if (user)
    user.disconnect();
    
  self.setUser( (Data*)NULL ); 

  int sock = self.socket();
  if (sock)
    shutdown( sock, 2 );

  return true;
}

static Slot connectCommand( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Slot user_name = args[1];
  Slot user_password = args[2];       

  Slot user ;

  Slot user_dictionary = store->Names->slice("user_dictionary")[2];

  try {
    user = user_dictionary.findUser( user_name );
  } catch (Frame *e) {
    self.tell("Name or password incorrect.", context );
    return true;
  }

  
  self.setUser( user );

  cerr << user.connection() << endl;

  if (user.connection()) {
    user.tell("** Reconnecting **");
    Slot old_conn = user.connection();
    old_conn.disconnect();
  }

  Slot sock = self.socket; 
  user.setConnection( self );

  Slot event = new Event( user, self, "connect" );
  Slot location = user.location();
  location.broadcastEvent( event );

  return true;
}

static Slot createCommand( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  Slot user_name = args[1];
  Slot user_password = args[2];       

  Slot actor = store->Names->slice("actor")[2];
  Slot starting_room = store->Names->slice("starting_room")[2];

  Slot user = actor.create( user_name );
  user.setNames( user_name );

  Slot userDict = store->Names->slice("user_dictionary")[2];
  userDict.addUser_withName( user, user_name );

  // tie together
  self.setUser( user );
  user.setConnection( self );

  user.moveTo( starting_room);

  Slot event = new Event( user, self, "connect" );

  Slot location = user.location();
  location.broadcastEvent( event );

  return true;
}

static Slot receiveLine( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot text = args[0];
  
  Slot user = self.user();
  if (user)
    return user.receiveLine( text );

  Slot words = ((String*)(Data*)text)->explode( new String(" "));
  
  if (words->length() != 3) {
    self.connect();
    return false;
  }
  
  if (words[0]->equal("create")) {
    self.createCommand( words[0], words[1], words[2] );
    return true;
  }

  if (words[0]->equal("connect")) {
    self.connectCommand( words[0], words[1], words[2] );
    return true;
  }
  
  self.connect();
  return false;
}

// one child created for each new connection
Slot createConnection(Frame *context)
{
  Slot self = context->_this;
  
  Slot named = self.named;

  Slot connection = named.create("connection");

  connection.tell = &tell;
  connection.receiveLine = &receiveLine;
  connection.connect = &connect;
  connection.disconnect = &disconnect;
  connection.createCommand = &createCommand;
  connection.connectCommand = &connectCommand;
  connection.setSocket = &setSocket;
  connection.socket = &socket;
  connection.user= &user;
  connection.setUser = &setUser;
  connection.welcomeMessage = &welcomeMessage;
  connection.setWelcomeMessage = &setWelcomeMessage;

  connection.setNames("generic connection");
  connection.setUser( (Data*)NULL );
  connection.setWelcomeMessage( "Welcome to abs(0)\n\nconnect <name> <password> or\ncreate <name> <password>" );

  return connection;
}
