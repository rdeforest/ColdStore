// Dictionary.cc: builds the $dictionary prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Object.hh>
#include <Store.hh>
#include <Frame.hh>

#include "Event.hh"

#include "common.hh"

const Slot defineDictionary( Frame *context )
{
  

  // our parent is root
  Slot Root = names.root;
  Slot Dictionary = Root.create_child("dictionary");

  Slot nouns = new List();
  Dictionary.nouns = nouns;

  Slot verbs = new List();
  Dictionary.verbs = verbs;

  return Dictionary;
}
