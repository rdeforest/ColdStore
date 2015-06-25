// Tokenizer - create tokens from a string
// Copyright (C) 2000,2001 Ryan Daum 
// 
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// $Id 

#include "Data.hh"
#include "List.hh"
#include "Dict.hh"
#include "Frame.hh"
#include "Closure.hh"
#include "Error.hh"
#include "Frob.hh"

#include "Tokenizer.hh"


static Slot word_match( Frame *context ) {
  Slot args = context->_arguments;
  Slot self = context->_this;

  if (args->length() < 3)
    throw new Error("invarg", self, "invalid number of arguments to token matcher");

  // arguments: string, position in string, frob parent [, separator]
  Slot total_string = args[0];
  int position = args[1];
  Slot fclass = args[2];

  Slot separator;
  if (args->length() >= 4 && args[3])
    separator = args[3];
  else
    separator = " ";

  // if we're at a space, skip it.
  if (total_string->slice(position).equal( separator ))
    return 1;

  Slot word;

  // find the next space
  Slot remaining = total_string->slice( position, -1 );
  Slot next_space = remaining->search( separator );
  if (!next_space)
    word = remaining;
  else
    word = remaining->slice( 0, next_space );

  // return is tuple of ( scan increment, token )
  Slot ret = new Tuple(2);
  ret->replace(0, word->length() );  // add one to eat the space
  ret->replace(1, new Frob( fclass, word) );

  return ret;

}

static Slot quoted_match( Frame *context ) {
  Slot args = context->_arguments;
  Slot self = context->_this;

  if (args->length() < 3)
    throw new Error("invarg", self, "invalid number of arguments to token matcher");

  // arguments: string, position in string, frob parent [, separator]
  Slot total_string = args[0];
  int position = args[1];
  Slot fclass = args[2];

  Slot separator;
  if (args->length() >= 4 && args[3])
    separator = args[3];
  else
    separator = " ";

  Slot ret = (Data*)NULL;

  // we're at the start of quotations, find the end
  if ( total_string->slice( position ).equal(separator) ) {
    int after = position + 1;
    Slot remaining = total_string->slice( after, -1 );
    Slot quotation = remaining->search( separator );
    if (quotation) {
      ret = new Tuple(2);
      Slot section = remaining->slice(0, quotation);
      ret->replace( 0, section->length() + 2); //  +1 to eat the separator
      ret->replace( 1, new Frob( fclass, section ) );
    }
  }
  return ret;
}

const Slot Tokenizer::QuotedTokenMatcher = new Closure( &quoted_match );
const Slot Tokenizer::WordTokenMatcher = new Closure( &word_match );

Tokenizer::Tokenizer( const Slot &string, const Slot &matchers, Frame *inContext = (Frame*)0 )
  : Tuple(4),
    _string(string),
    _matchers(matchers),
    context(inContext),
    string_pos(0)
{}

Tokenizer::Tokenizer( const Tokenizer *src )
  : Tuple(4),
    _string(src->_string),
_matchers(src->_matchers),
context(src->context),
string_pos(src->string_pos)
{}

bool Tokenizer::isIterator() const {
  return true;
}

bool Tokenizer::More() const {
  if (string_pos < _string->length() - 1)
    return true;
  else
    return false;
}

Slot Tokenizer::Next()  {

  while ( string_pos < _string->length() ) {

    Slot match_it = _matchers->iterator();
    while (match_it->More()) {
      // retrieve the next closure/argument pair
      Slot func_t = match_it->Next();

      // make the call frame
      Slot closure = func_t[0];

      Frame *frame = new Frame( closure );

      // prepare the arguments
      Slot argument = func_t[1];
      Slot arguments = new List();

      // first, the total string
      arguments->insert( _string );

      // second, the position pointer
      arguments->insert( string_pos );

      // third, the optional argument
      arguments->insert(argument);
      ((Frame*)(Data*)frame)->_arguments = arguments ;
      ((Frame*)(Data*)frame)->_caller = context;
      if (context)
      	((Frame*)(Data*)frame)->_top = context->_top;

      closure->reduce( closure, frame );
      Slot res = closure;

      // if not null, set the last match pointer, increment
      // by amount, and return result
      // otherwise, move along
      if (res) {
	if (res.isNumeric()) {
	  int skip_amount = res;
	  string_pos += skip_amount - 1 ;
	} else {
	  int length = res[0];
	  Slot token = res[1];
	  string_pos += length ;
	  
	  return token;
	}
      }

    }
      string_pos++;
  }

  // if we've gotten here, it means nothing was matched
  // and we passed the end of the string
  throw new Error("range", this, "end of string reached and no token found");
    
  return (Data*)NULL; // to satisfy gcc
}

/////////////////////////////////////
// structural

// Uses the copy constructor
Data *Tokenizer::clone(void *where) const
{
  //  return new (_TupleBase<Slot>::Length(), where) Tokenizer(*this);
  return new Tokenizer(this);
}

// returns a mutable copy of this
Data *Tokenizer::mutate(void *where) const
{
  return (Data*)this;
}
