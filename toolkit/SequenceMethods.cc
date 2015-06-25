// SequenceMethods.cc: builds the $list data object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <Frob.hh>
#include <Error.hh>
#include <String.hh>
#include <Closure.hh>
#include <MetaData.hh>

#include "common.hh"
 
static Slot join( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  if ( ! (AKO((Data*)(self), List )) )
    throw new Error("type", self, "join must be called on a list");

  Slot sep = " ";
  if (args->length() == 1) {
    sep = args[0];
    if ( ! (AKO((Data*)(sep), String )) )
      throw new Error("type", sep, "1st argument to words must be a string");
  }

  return ((List*)(Data*)self)->join(sep);
}

static Slot englishJoin( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;
  
  if ( ! (AKO((Data*)(self), List )) )
    throw new Error("type", self, "englishJoin must be called on a list");
 
  Slot res = new String(); 
  Slot l = self->iterator();
  while (l->More()) {
    Slot el = l->Next();
    if (!l->More())
      res = res->concat("and ");
    
    res = res->concat( el );
    if (l->More())
      res = res->concat(", ");
  }

  return res;
}

Slot defineExtraSequenceMethods(Frame *context)
{
  if (!MetaData::instance)
    MetaData::instance = new MetaData();

  MetaData::instance.join = &join;
  MetaData::instance.englishJoin = &englishJoin;

  return true;
}
