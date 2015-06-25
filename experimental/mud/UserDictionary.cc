// UserDictionary.cc: builds the $user_dictionary prototype
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms


#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <Closure.hh>
#include <Dict.hh>

#include <crypt.h>

/** user_dictionary addUser: <user object> withName: <user name> 
 */
static Slot addUser_withName( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot u_obj = args[0];
  Slot u_name = args[1];

  Slot obj_dictionary = self.obj_dictionary;
  obj_dictionary = obj_dictionary->insert( u_name, u_obj );
  self.obj_dictionary = obj_dictionary;

  Slot name_dictionary = self.name_dictionary;
  name_dictionary->insert( u_obj, u_name );
  self.name_dictionary = name_dictionary;

  return true;
}

/** user_dictionary findUser: <user object>
 */
static Slot findUser( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot u_name = args[0];

  Slot obj_dictionary = self.obj_dictionary;

  return obj_dictionary->slice( u_name );
}


static Slot initialize( Frame *context )
{
  Slot self = context->_this;

  self.obj_dictionary = new Dict();
  self.name_dictionary = new Dict();

  return true;
}

Slot createUserDictionary( Frame *context )
{
  Slot self = context->_this;

  Slot root = self.root;

  Slot user_dictionary = root.create("user_dictionary");
  user_dictionary.initialize = &initialize;
  user_dictionary.findUser= &findUser;
  user_dictionary.addUser_withName = &addUser_withName;

  user_dictionary.initialize();

  return user_dictionary;
}

