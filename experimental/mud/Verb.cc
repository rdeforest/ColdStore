// Verb.cc: builds the $named prototype object
// Copyright (C) 2001 Ryan Daum

// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

#include <Data.hh>
#include <Tuple.hh>
#include <Object.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <Closure.hh>
#include <Message.hh>
#include <VectorSet.hh>
#include <Error.hh>

#include "common.hh"

/** do - receive a command object and execute it
 */
static Slot invoke_on( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot on = args[1];
  Slot cmd = args[0];

  Slot dobj = cmd.dobj();
  Slot iobj = cmd.iobj();


  return 1;
}

static Slot findPrepPosition( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot mt = self.argTemplate;
  Slot prepM = mt[1];
  Slot preps = self.prepositions;

  Slot words = self.words;
  words->iterator();
  int count = 0;
  while (words->More()) {
    Slot w = words->Next();
    count++;
    if (preps->search(w))
      if (prepM->equal("any"))
	return count;
      else if (prepM->equal(w))
	return count;
  }
  
  return (Data*)NULL;
}

static Slot canHandle_on( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot command = args[0];
  Slot on = args[1];

  self.command = command;

  Slot words = command.words();
  self.words = words;
  Slot environment = command.environment();
  self.environment = environment;

  Slot mt = self.argTemplate;

  Slot dobjM = mt[0];
  Slot prepM = mt[1];
  Slot iobjM = mt[2];

  Slot dobj = command.dobj();
  Slot dobjstr = command.dobjstr();
  Slot prep = command.prep();
  Slot iobj = command.iobj();
  Slot iobjstr = command.iobjstr();
  Slot dobjlength = command.dobjlength();
  Slot iobjlength = command.iobjlength();
  Slot actor = command.actor();

  Slot preps = self.prepositions;


  if (on != actor && on != actor.location() && !dobjM.equal("this") && !dobjM.equal("this"))
    return (Data*)NULL;

  if (dobjM.equal("none")) {
    if (prepM.equal("none") && !words->length() )
      return true;
  } else if (dobjM.equal("string")) {
    if (prepM.equal("none") && words->length())
      return true;
    if (!dobjstr) {
      Slot prepPos = self.findPrepPosition();
      Slot dobjwords = words->slice(0, prepPos);
      command.setDobjstr( dobjwords.join() );
    }
  } else if (dobjM.equal("object") || dobjM.equal("this") ) {
    if (!dobj) {
      Slot result = self.parseNoun();
      if (result) {
	dobj = result[0];
	dobjlength = result[1];
	command.setDobj( dobj );
	command.setDobjLength(dobjlength);
      }
    } else {
      words = words->del(0, dobjlength);
      self.words = words;
    }

    if (dobjM.equal("this") && dobj != on) {
      return (Data*)NULL;
    }
  }   

  words = self.words;

  if (prepM.equal("none"))
    if (!words->length()) {
      return true;
    } else {
      return (Data*)NULL;
    }
  else if (prepM.equal("any") && (preps->search(words[0])))
    return true;
  else {
    if (!prep) {
      if (!words->length())
	return (Data*)NULL;
      prep = self.parsePreposition();
      command.setPrep( prep );
    }
    if (!prep.equal(prepM))
      return (Data*)NULL;
  }

  words = self.words;

  if (!words->length())
    return (Data*)NULL;

  if (iobjM.equal("string")) {
    if (!iobjstr) {
      iobjstr = words.join();
      command.setIobjstr( iobjstr );
    }
    return true;
  } else if (iobjM.equal("object") || iobjM.equal("this")) {
    if (!iobj) {
      Slot result = self.parseNoun();
      if (result) {
	iobj = result[0];
	iobjlength = result[1];
	command.setIobj( iobj );
	command.setIobjLength( iobjlength );
      } else {
	return (Data*)NULL;
      }
    } else {
      words = words->del(0, iobjlength);
    }

    if (iobjM.equal("this") && !iobj.equal(on))
      return (Data*)NULL;
    else
      return true;
    
  }

  return (Data*)NULL;
}

static Slot parseNoun( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot words = self.words;
  int words_length = words->length();

  int name_length = 0;
  int marked_length = 0;
  Slot marked = (Data*)NULL;

  Slot env = self.environment;
  Slot c_it = env->iterator();
  while (c_it->More()) {
    Slot obj = c_it->Next();
    
    Slot names = obj.names();
    Slot n_it = names->iterator();
    while (n_it->More()) {
      Slot name = n_it->Next();
      Slot name_explode = name.explode();
      name_length = name_explode->length();

      if ( (words_length >= name_length) &&
	   (words->slice(0, name_length).equal(name_explode)) &&
	   (name_length > marked_length) ) {
	marked_length = name_length;
	marked = obj;
      }
    }
  }
  
  if (marked) {
    words = words->del(0, marked_length);
    self.words = words;
    Slot command = self.command;
    Slot res = new Tuple(2);
    return res->replace(0, marked)->replace(1, marked_length);
  }

  return (Data*)NULL;
}

static Slot parseAdverb( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot words = self.words;

  Slot adverb = words[0];
  Slot adverbs = self.adverbs;

  if (adverbs->search(adverb)) {
    words->Next();
    self.words = words;
    return adverb;
  }

  return (Data*)NULL;
}

static Slot parseArticle( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot words = self.words;

  Slot article = words[0];
  Slot articles = self.articles;

  if (articles->search(article)) {
    words->Next();
    self.words;
    return article;
  }

  return (Data*)NULL;
}

static Slot parsePreposition( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot words = self.words;

  Slot prep = words[0];
  Slot preps = self.prepositions;
  
  if (preps->search(prep)) {
    words->Next();
    self.words;
    return prep;
  }
  return (Data*)NULL;
}

static Slot argTemplate( Frame *context )
{
  Slot self = context->_this;

  return self.argTemplate;
}

static Slot setDobjMatch( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot mt = self.argTemplate;

  Slot iD = args[0];

  if ( !iD.equal("object") && !iD.equal("none") && !iD.equal("string") && !iD.equal("this") )
    throw new Error("invarg", iD, "invalid direct object match template");

  mt = mt->replace(0, iD);

  self.argTemplate = mt;

  return true;
}

static Slot setIobjMatch( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot mt = self.argTemplate;

  Slot iI = args[0];

  if ( !iI.equal("object") && !iI.equal("none") && !iI.equal("string") && !iI.equal("this") )
    throw new Error("invarg", iI, "invalid indirect object match template");

  mt = mt->replace(2, iI);

  self.argTemplate = mt;

  return true;
}

static Slot setPrepMatch( Frame *context )
{
  Slot self = context->_this;
  Slot args = context->_arguments;

  Slot mt = self.argTemplate;

  Slot iP = args[0];

  Slot preps = self.prepositions;
  preps = preps->insert("none");
  preps = preps->insert("any");

  if (!preps->search(iP))
    throw new Error("invarg", iP, "invalid preposition for verb match template");

  mt = mt->replace(1, iP);

  self.argTemplate = mt;

  return true;
}


static Slot initialize( Frame *context )
{
  Slot self = context->_this;

  Slot prepositions = new VectorSet();
  prepositions->insert("with");
  prepositions->insert("to");
  prepositions->insert("from");
  prepositions->insert("at");
  prepositions->insert("for");
  prepositions->insert("in");
  prepositions->insert("into");
  prepositions->insert("of");
  prepositions->insert("over");
  prepositions->insert("under");
  self.prepositions = prepositions;

  Slot articles = new VectorSet();
  articles->insert("a");
  articles->insert("the");
  articles->insert("an");
  articles->insert("that");
  articles->insert("these");
  articles->insert("those");
  self.articles = articles;

  self.adverbs = new VectorSet();

  self.argTemplate = new Tuple(3);

  return true;
}

Slot createVerb(Frame *context)
{
  Slot self = context->_this;

  Slot named = self.named;
  Slot verb = named.create("verb");

  verb.setNames("generic verb");
  verb.invoke_on = &invoke_on;
  verb.initialize = &initialize;
  verb.canHandle_on = &canHandle_on;
  verb.parseNoun = &parseNoun;
  verb.parsePreposition = &parsePreposition;
  verb.parseAdverb = &parseAdverb;
  verb.parseArticle = &parseArticle;
  verb.argTemplate = &argTemplate;
  verb.setDobjMatch = &setDobjMatch;
  verb.setIobjMatch = &setIobjMatch;
  verb.setPrepMatch = &setPrepMatch;
  verb.findPrepPosition = &findPrepPosition;

  verb.initialize();

  return verb;
}
