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

/** return the end of the current word
 *  accounting for commas and quoted sections
 */
static int find_word_end(Slot pStr, int pStart) {
  if (pStr[pStart].equal("\"")) {
    Slot sub = pStr->slice( pStart + 1, -1 );
    Slot x = sub->search("\"");
    if (x)
      return x  + Slot(1);
  } else {
    Slot sub = pStr->slice( pStart, -1 );
    Slot space = sub->search(" ");
    Slot comma = sub->search(",");
    if ( (!comma) || ((space) && (space < comma)))
      if (space)
	return space;
    if (comma)
      return comma;
  }
  return pStr->length() - pStart;

}

/** break the string into a list of words
 *  - filter garbage and reduce non-quoted stuff to lower case
 *  - convert commas to conjunctions
 */
static Slot string_to_words( Slot pStr, Slot garbage = new List() ) {
  Slot words;
  
  int wordstart, wordend;
  int len;

  wordstart = wordend = 0;
  len = pStr->length();
  words = new List();
  
  while (wordstart < len) {
    wordend = find_word_end( pStr, wordstart  );
    if (pStr[wordstart].equal("\"")) {
      Slot word = pStr->slice ( wordstart + 1, wordend - 1 );
      words->insert( word );
    } else {
      Slot word = pStr->slice ( wordstart, wordend);

      if (word->length() && (!garbage->search(word))) {
	if ((wordend < len ) && pStr[wordend].equal(",")) {
	  words->insert("and");
	  wordend--;
	} else
	  words->insert( word );

      }
    }
    wordstart = wordstart + wordend + 1;

  }
  return words;
}
 

static Slot explodeWords( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  if (args->length())
    return string_to_words( self, args[0] );
  else
    return string_to_words( self );
}


static Slot explode( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot sep = " ";
  if (args->length())
    sep = args[0];
  
  return ((String*)(Data*)self)->explode( sep );
}

Slot defineExtraStringMethods(Frame *context)
{
  if (!MetaData::instance)
    MetaData::instance = new MetaData();

  MetaData::instance.explode = &explode;
  MetaData::instance.explodeWords = &explodeWords;

  return true;
}
