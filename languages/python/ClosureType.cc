#include <Frame.hh>
#include <Closure.hh>

#include "coldmodule.hh"
#include "ClosureType.hh"

//////////////////////
// CLOSURE OPERATIONS
//////////////////////

PyObject * cold_closure_call( PyColdObject *x, PyObject *y, PyObject *z)
{
  Slot arguments = new List(PyToCold(y));
  
  // first create a copy of the closure
  Slot method = x->data;
    
  // now create a frame
  Slot context = new Frame( method ) ;

  // set the arguments
  ((Frame*)(Data*)context)->setArguments( arguments );
  ((Frame*)(Data*)context)->setThis( new Object() );

  // now call it, with a call history of nothing
  try { 
    Slot result = ((Frame*)(Data*)context)->call( (Frame*)0 );
    PyObject* ret = (PyObject*) new PyColdObject(result);
    return ret;
  } catch (Frame *fr) {
    fr->upcount();
    cerr << (char*)fr->traceback() << endl;
    fr->dncount();
    return 0;
  }  
}

int  cold_closure_setattr( PyColdObject *self, char *name, PyObject *v)
{
  try {
    Closure * fr = ((Closure*)(Data*)self->data);

    if ( strcmp( name, "definer" ) == 0 ) 
      fr->setDefiner( PyToCold(v) );
    else if ( strcmp( name, "name" ) == 0 ) {
      // This has to be a Closure
      fr->setName( PyToCold(v) );
    } else if ( strcmp( name, "source" ) == 0 ) {
      fr->setSource( PyToCold(v) );
    } else if ( strcmp( name, "tree" ) == 0 ) 
      fr->setTree( PyToCold(v) );
    else {
      // none of the above
      PyErr_SetString(PyExc_AttributeError, name);
      return -1;
    }
    
    // everything passed through o.k.
    return 0;
    
  } catch (Error *e) {  
    handleError(e);  
    return -1;
  }
  
}

PyObject *cold_closure_getattr( PyColdObject *self, char *name)
{
  try {
    Closure * fr = ((Closure*)(Data*)self->data);

    if ( strcmp( name, "__methods__" ) == 0 )
      {
	PyObject *l = PyList_New(0);
	PyList_Append( l, PyString_FromString("definer") );
	PyList_Append( l, PyString_FromString("name") );
	PyList_Append( l, PyString_FromString("tree") );
	PyList_Append( l, PyString_FromString("source") );
	return l;
      }
    if ( strcmp( name, "definer" ) == 0 )
      return (PyObject*) new PyColdObject(fr->getDefiner());
    if ( strcmp( name, "name" ) == 0 )
      return (PyObject*) new PyColdObject(fr->getName());
    if ( strcmp( name, "tree" ) == 0 )
      return (PyObject*) new PyColdObject(fr->getTree());
    if ( strcmp( name, "source" ) == 0 )
      return (PyObject*) new PyColdObject(fr->getSource());
 
    // none of the above 
    PyErr_SetString(PyExc_AttributeError, name);
    return NULL;

  } catch (Error *e) {  
    handleError(e);  
    return NULL;
  }

}
