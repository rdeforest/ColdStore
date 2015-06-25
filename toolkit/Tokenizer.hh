// Tokenizer - create tokens from a string
// Copyright (C) 2000,2001 Ryan Daum 
// 
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms 
// $Id 

#ifndef TOKENIZER_HH
#define TOKENIZER_HH

#include "Data.hh"
#include "Tuple.hh"
#include "Frame.hh"

static Slot word_match( Frame *context );

/** tokenizer class - given a string and a dictionary of closures 
 *                      => closure arguments,  evaluates each closure
 *                    for a section of text, and appends results of 
 *                    evaluation for those closures matching
 *                    true.
 *
 * example:
 *   Slot tDict = new Dict();
 *   tDict->insert( Tokenizer::WordTokenMatcher );
 *   Slot t = new Tokenizer("my cat is 50 \"years old\"", tDict);
 *   while (t->More())
 *       cerr << t->Next() << endl;
 */
cold class Tokenizer
  : public Tuple 
{
protected:
  Slot _string;  // the string to tokenize
  Slot _matchers;  // the list of token matcher closures
  Frame *context;  // a context with which to evaluate the closures
  int string_pos ; // our position in the string

 
public:
  /** token matching functions:
   *           WordTokenMatcher - match words separated by separator.
   *                              default separator is ' '
   *           QuotedTokenMatcher - match quoted section - quote
   *                 type argument optional, default '\"'
   *           IntegerTokenMatcher - match and convert integers
   *           NameTokenMatcher - match $names
   *           KeywordTokenMatcher - matches keyword from 1st argument
   */

  static const Slot WordTokenMatcher;
  static const Slot QuotedTokenMatcher ;

public:

  /** construct a tokenizer
   *  @param string the string to tokenize
   *  @param matchers a list of Tokens to match with
   */
  Tokenizer( const Slot &string, const Slot &matchers, Frame *context = (Frame*)0 );

  /** copy a tokenizer
   */
  Tokenizer( const Tokenizer *source );

public:
  /////////////////////
  // Iterator interface

  /** answer yes to isIterator question
   *  @return true
   */
  virtual bool isIterator() const;

  /** is there more to tokenize?
   *  @return true if more available
   */
  virtual bool More() const;

  /** advance token iterator over string
   *  @return next token
   */
  virtual Slot Next();

public:
  //////////////
  // Structural

  // structural
  virtual Data *clone(void *store = (void*)0) const;
  virtual Data *mutate(void *where) const;

};


#endif
