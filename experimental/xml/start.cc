// start.cc: start the xml
// Copyright (C) 200!, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <elf.hh>
#include <Data.hh>
#include <Slot.hh>
#include <Tuple.hh>
#include <Symbol.hh>
#include <Error.hh>
#include <Store.hh>
#include <List.hh>
#include <Frame.hh>
#include <Object.hh>

#include <expat.h>

#include <gnome--/main.h>
#include "../browser/coldBrowser.hh"

#define BUFFSIZE 8192
char Buff[BUFFSIZE];

void pre_eval_bkpt() {}

void yac_break()
{}

Slot total;
Slot names;
int tagcount;

void elementDeclare(void *userData,
		    const XML_Char *name,
		    XML_Content *model) {
  Slot element = new Object();
  element.id = name;
  store->Names->insert( name, element );
  total->insert(element);
  free(model);
 
}

void attlistDeclare( void           *userData,
		     const XML_Char *elname,
		     const XML_Char *attname,
		     const XML_Char *att_type,
		     const XML_Char *dflt,
		     int            isrequired )
{
  Slot element = store->Names->slice(elname)[2];

  const char *attribute = attname;
  const char *value;
  if (dflt)
    value = dflt;
  else
    value = "";
  if (strcmp( "id", attribute ) > 0)
    element->insert( attribute, value );
}


void start(void *data, const char *el, const char **attr) {
  char name[80];

  Slot parent = names->slice( el )[2];
  Slot us = ((Object*)(Data*)parent)->createChild();

  for (int i = 0; attr[i]; i+=2) {
    us->insert( attr[i], attr[i+1] );
  }

  // if the element gave itself an ID, use that as its name.
  // otherwise, make one up
  cerr << us->search("id");

  if (us->search("id")) {
    sprintf( name,  (char*)us.id );
  } else {
    int num_children = ((Object*)(Data*)parent)->children()->length();
    sprintf( name, "%s%d", el, num_children );
    us->insert( "id", name );
    cerr << name << endl;
  }

  total->insert(us);
  store->Names->insert( name, us );
  tagcount++;
}

void end(void *data, const char *el) {
}

void parseDTD(FILE *in) {
  XML_Parser c = XML_ParserCreate(NULL);
  XML_Parser p = XML_ExternalEntityParserCreate(c,NULL, NULL );
  XML_SetParamEntityParsing(p, XML_PARAM_ENTITY_PARSING_ALWAYS );
  XML_SetElementDeclHandler(p, elementDeclare );
  XML_SetAttlistDeclHandler(p, attlistDeclare );
  if (! p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }
  for (;;) {
    int done;
    int len;
    
    len = fread(Buff, 1, BUFFSIZE, in);
    if (ferror(in)) {
      fprintf(stderr, "Read error\n");
      exit(-1);
    }
    done = feof(in);
    
    if (! XML_Parse(p, Buff, len, done)) {
      fprintf(stderr, "Parse error at line %d:\n%s\n",
	      XML_GetCurrentLineNumber(p),
	      XML_ErrorString(XML_GetErrorCode(p)));
      exit(-1);
    }
    
    if (done)
      break;
  }
  
}

void parseXML(FILE *in) {
  XML_Parser p = XML_ParserCreate(NULL);
  XML_SetElementHandler(p, start, end);
  if (! p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }
  for (;;) {
    int done;
    int len;
    
    len = fread(Buff, 1, BUFFSIZE, in);
    if (ferror(in)) {
      fprintf(stderr, "Read error\n");
      exit(-1);
    }
    done = feof(in);
    
    if (! XML_Parse(p, Buff, len, done)) {
      fprintf(stderr, "Parse error at line %d:\n%s\n",
	      XML_GetCurrentLineNumber(p),
	      XML_ErrorString(XML_GetErrorCode(p)));
      exit(-1);
    }
    
    if (done)
      break;
  }
  
}

void initElf(Elf *elf)
{

  try {
    names = store->Names;
    
    total = new List();

    FILE *f = fopen("svg.dtd", "r");
    parseDTD(f);
    fclose(f);

    f = fopen("svg-example.svg", "r");
    parseXML(f);
    fclose(f);

    int argc = 1;
    char *argv[1];
    argv[0] = "test";
    char **cargv = (char**)argv;
    
    Gnome::Main m("coldBrowser", "0.1", argc, cargv);  
    
    manage( new class coldBrowser( total ));
    m.run();
    } catch (Error *e) {
        e->upcount();
        cerr << "Error " << e << "\n";
        e->dncount();
    } catch (Frame *e) {
    }
}
