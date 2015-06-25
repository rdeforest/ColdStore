#include "coldmodule.hh"

#include <Integer.hh>
#include <String.hh>
#include <Object.hh>
#include <Real.hh>
#include <Dict.hh>
#include <Store.hh>
#include <Freon.hh>

#include "compile.h"

//////////////////////
// UTILITY ROUTINES
//////////////////////

void handleError( Error *e )
 {
  e->upcount();

  if (e->slice(0)->equal("type"))
    PyErr_SetString( PyExc_TypeError, e->slice(2) );
  else if (e->slice(0)->equal("dictnf"))
    PyErr_SetString( PyExc_AttributeError, e->slice(2) );
  else if (e->slice(0)->equal("range"))
    PyErr_SetString( PyExc_IndexError, e->slice(2) );
  else
    PyErr_SetString( PyExc_Exception, e->slice(2) );

  e->dncount();
}

PyObject *ColdToPy( Slot data )
{

  if (!data)
    return Py_None;

  // it's an integer
  if (  AKO((Data*)data, Integer ) )
    return PyInt_FromLong( data );

  if ( AKO((Data*)data, String )) 
    return PyString_FromString( data );

  if ( AKO((Data*)data, Tuple )) 
    {
      PyObject *rettuple = PyTuple_New( data->length() );
      for (int x=0;x++;x<=data->length())
	PyTuple_SetItem( rettuple, x, ColdToPy( data[x] ) );
      return rettuple;      
    }
  
  if ( data->isSequence() )
    {
      PyObject *retlist = PyList_New(0);
      Slot it = data->iterator();
      while (it->More())
	PyList_Append( retlist, ColdToPy( it->Next() ) );
      return retlist;
    }
  
  if (data->isMap() )
    {
      PyObject *retdict = PyDict_New();
      Slot it = data->iterator();
      while (it->More()) {
	Slot t = it->Next();
	PyDict_SetItem( retdict, ColdToPy( t[0] ), ColdToPy( t[1] ) );
      }
      return retdict;
    }



  throw new Error( "type", 0, "unable to unconvert type" );

}  

Slot PyToCold( PyObject *right )
{

  if (right == Py_None || !right ) {
    return 0;
  }

  // if it's already a PyColdObject, just return its data
  if (PyColdObject_Check( right))
    return ((PyColdObject*)right)->data;
  
  // ints, first.
  if (PyInt_Check(right))
    return PyInt_AS_LONG(right);
  
  // floats => real (hm?)
  if (PyFloat_Check(right))
    return new Real(PyFloat_AsDouble(right));
  
  // Tuples
  if (PyTuple_Check(right))
    {
      int length = PyTuple_Size(right);
      Slot myT = new Tuple( length );
      for (int count = 0; count < length; count++ )
        {
	  Slot element = PyToCold(PyTuple_GET_ITEM( right, count ));
	  myT = myT->replace( count, element );
        }
      return myT;
    }
  
  // strings are easy
  if (PyString_Check(right))
    return new String( (PyString_AS_STRING(right)) );
  
  // lists are harder
  if (PyList_Check(right))
    {
      Slot myL = new List();
      int length = PyList_Size( right );
      for (int count = 0; count < length; count++)
        {
	  //            PyObject *element = PyList_GetItem( right, count );
	  // nah, let's use the unsafe, faster method.. the MACRO
	  PyObject *element = PyList_GET_ITEM( right, count ) ;
	  myL = myL->concat( PyToCold( element) );
		  
        }
      return myL;
    }
  
  // dicts are harder still
  if (PyDict_Check(right))
    {
      Slot myD = new Dict();
      
      // get a list of the keys, then iterate through them
      PyObject *keys = PyDict_Keys( right );
      int length = PyList_Size( keys );
      for (int count = 0; count < length; count++)
        {
	  PyObject *key = PyList_GET_ITEM( keys, count );
	  PyObject *element = PyDict_GetItem( right, key );
	  Slot cKey = PyToCold( key );
	  Slot cElement = PyToCold( element );
	  myD->insert( cKey, cElement );
        }
      return myD;
    }



  throw new Error( "type", 0, "unable to convert type" );

}

// Retrieves the store->Names.
PyObject *
get_cold_names()
{
  try
    {
      PyColdObject *obj = new PyColdObject( (Data*)store->Names ) ;
      return (PyObject *)obj;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}
