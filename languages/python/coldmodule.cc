// coldmodule.cc: cold/python interface module
// Copyright (C) 2001, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

static char *id __attribute__((unused))="$Id: coldmodule.cc,v 1.39 2001/06/22 02:57:33 opiate Exp $";

#include "Python.h"

//#define DEBUGLOG
#include <Slot.hh>
#include <List.hh>
#include <Tuple.hh>
#include <Data.hh>
#include <Store.hh>
#include <Object.hh>
#include <Freon.hh>
#include <String.hh>

#include "compile.h"
#include "coldmodule.hh"

///////////////////
// MODULE METHODS
///////////////////

// Function of 1 argument creating a new PyColdObject
// conversion of the object given
PyObject *
cold_convert( PyObject *self, PyObject *args )
{
  PyColdObject *rv;
  PyObject *obj;

  if (!PyArg_ParseTuple(args, "O", &obj))
    return NULL;

  try
    {
      Slot converted = PyToCold(obj);
      rv = new PyColdObject( converted );
      return (PyObject *)rv;
    }
  catch (Error *e)
    {
      handleError(e);

      return NULL;
    }

}

// Function of 1 argument creating a new PyColdObject
// conversion of the object given
PyObject *
cold_unconvert( PyObject *self, PyObject *args )
{
  PyObject *rv;
  PyObject *obj;

  if (!PyArg_ParseTuple(args, "O", &obj))
    return NULL;

  try
    {
      if (!PyColdObject_Check(obj))
	throw new Error( "type", 0, "type is not a cold type" );
      rv = ColdToPy( ((PyColdObject*)obj)->data );
      Py_INCREF(rv);
      return rv;
    }
  catch (Error *e)
    {
      handleError(e);

      return NULL;
    }

}

// Function of 1 argument creating a new PyColdObject
// conversion of the object given

PyObject *
cold_object( PyObject *self, PyObject *args )
{
  PyColdObject *rv;

  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  try
    {
      Slot obj = new Object();
      rv = new PyColdObject( obj );
      return (PyObject *)rv;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }
}

PyObject *
cold_freon( PyObject *self, PyObject *args )
{
  PyColdObject *rv;

  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  try
    {
      Slot names = store->Names;
      rv = new PyColdObject(names.freon_compiler);
      return (PyObject *)rv;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }
}

// Function of 1 argument creating a new PyColdObject
// of the type given
PyObject *
cold_new( PyObject *self, PyObject *args )
{
  PyColdObject *rv;
  PyObject *argu;
  char *lib;
  char *type;

  argu = 0;
  lib = 0;
  if (!PyArg_ParseTuple(args, "s|sO", &type, &lib, &argu))
    return NULL;

  try
    {
      Slot arguments;
      if (argu)
	arguments = PyToCold(argu);
      else
	arguments =  new List();

      Slot typestring = new String(type);

      Slot obj;


      obj = Data::New( typestring, arguments, lib ? lib : NULL );

      rv = new PyColdObject( obj );
      return (PyObject *)rv;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }
}

// Function taking one argument, a key by which a variable was stored.
// return a new List
PyObject *
cold_retrieve( PyObject *self, PyObject *args )
{
  char *objname;

  if (!PyArg_ParseTuple(args, "s", &objname))
    return NULL;

  try
    {
      Slot sym = store->Names->slice(objname);
      PyColdObject *obj = new PyColdObject( sym[2] );

      return (PyObject *)obj;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}

// Function taking two argument, a key, and a value, to hold in a
// common namespace
PyObject *
cold_store( PyObject *self, PyObject *args )
{
  char *objname;
  PyObject *obj;

  if (!PyArg_ParseTuple(args, "sO", &objname, &obj))
    return NULL;

  try
    {
      Slot value = PyToCold(obj);
      store->Names->insert(objname, value);
      return obj;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}

// called to cleanup objects in the queue on exit
void
_cleanup()
{
}

// List of functions defined in the module
PyMethodDef cold_methods[] = {
  {"new",             cold_new,  METH_VARARGS},
  {"convert",         cold_convert,    METH_VARARGS},
  {"unconvert",       cold_unconvert,    METH_VARARGS},
  {"store",           cold_store,  METH_VARARGS},
  {"retrieve",        cold_retrieve, METH_VARARGS},
  {"object",          cold_object, METH_VARARGS},
  {"freon",           cold_freon, METH_VARARGS},
  {NULL,		NULL}		/* sentinel */
};

//////////////////////////
// MODULE INITIALIZATION
//////////////////////////

// Initialization function for the module
extern "C"
{
  DL_EXPORT(void)
    initcold()
  {
    PyObject *m, *d;
    PyObject *pdict = PyDict_New();
    PyDict_SetItemString(pdict, "__builtins__", PyEval_GetBuiltins()); 

    /* Create the module and add the functions */
    m = Py_InitModule("cold", cold_methods);

    /* Add some symbolic constants to the module */
    d = PyModule_GetDict(m);
    NamesObject = get_cold_names();
    PyDict_SetItemString(d, "names", NamesObject);
   
    //    Py_AtExit( &_cleanup );
  }
}
