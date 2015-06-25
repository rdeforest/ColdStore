#include "coldmodule.hh"


#include "PyColdObject.hh"

//////////////
// COLDOBJECT 
//////////////

PyColdObject::PyColdObject ()
{
  PyObject_Init((PyObject *)this, &PyColdScalar_Type);
  Py_INCREF(this);
}

PyColdObject::PyColdObject ( const Slot inSlot )
{
  if (inSlot ) {  
  
    if ( AKO((Data*)inSlot, Object )) 
      PyObject_Init((PyObject *)this, &PyColdObject_Type);
    else if ( AKO((Data*)inSlot, Symbol )) 
      PyObject_Init((PyObject *)this, &PyColdSequence_Type);
    else if ( AKO((Data*)inSlot, Closure )) 
      PyObject_Init((PyObject *)this, &PyColdClosure_Type);
    else if ( inSlot->isSequence() ) 
      PyObject_Init((PyObject *)this, &PyColdSequence_Type);
    else if (inSlot->isMap() )
      PyObject_Init((PyObject *)this, &PyColdMapping_Type);
    else
      PyObject_Init((PyObject *)this, &PyColdScalar_Type);
  } else {
    // not sure what to do with NULL object... for now treat us as scalar?
    PyObject_Init((PyObject *)this, &PyColdScalar_Type);
  }
  this->data = inSlot;
  Py_INCREF(this);
}

//////////////////////
// COMMON OPERATIONS
//////////////////////

// convert a PyColdObject into string representation
PyObject *cold_op_getrepr( PyColdObject *self)
{
  ostrstream str;

  try{
    str << self->data << ends;
    PyObject *repr = PyString_FromString( str.str() );

    if (repr != NULL)
      {
	Py_INCREF(repr);
	return repr;
      }
    return NULL;
  }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}

// compare two PyColdObjects
int cold_op_compare( PyColdObject *left, PyColdObject *right)
{
  try
    {
      return left->data == right->data;
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }

}

// deallocate the PyColdObject
void
cold_op_dealloc(PyColdObject *self)
{
  PyMem_DEL(self);
}

///////////////////////
// COLDOBJECT METHODS
///////////////////////

PC_NO_ARG( truth );
PC_NO_ARG( toconstruct );
PC_NO_ARG( toSequence );
PC_NO_ARG( isSequence );
PC_NO_ARG( isMap );
PC_NO_ARG( length );
PC_NO_ARG( iterator );
PC_NO_ARG( isIterator );
PC_NO_ARG( More );
PC_NO_ARG( Next );

PC_ONE_ARG( equal );
PC_ONE_ARG( order );
PC_ONE_ARG( concat );
PC_ONE_ARG( search );
PC_ONE_ARG( connect );
PC_ONE_ARG( disconnect );
PC_ONE_ARG( incoming );
PC_ONE_ARG( outgoing );

PC_ONE_OR_TWO_ARG( insert );
PC_ONE_OR_TWO_ARG( slice );
PC_ONE_OR_TWO_ARG( del );

PyObject * data_method_replace( PyColdObject *self, PyObject *args )
{
  try { 
    PyObject *arg1, *arg2, *arg3; 
    arg2 = arg3 = 0;
    if (!PyArg_ParseTuple( args, "O|O|O", &arg1, &arg2, &arg3 ))  
      return NULL; 
    Slot argument1 = PyToCold( arg1 );
    if (arg2) {
      Slot argument2 = PyToCold( arg2 );
      if (arg3) {
	Slot argument3 = PyToCold( arg3 );
	return (PyObject*) new PyColdObject( self->data->replace( argument1, argument2, argument3 ) ); 
      }
      return (PyObject*) new PyColdObject( self->data->replace( argument1, argument2 ) ); 
    }
    return (PyObject*) new PyColdObject( self->data->replace( argument1 ) );
  } catch (Error *e) {  
    handleError(e);  
    return 0;
  }
}

PyObject * data_method_typeId( PyColdObject *self, PyObject *args )
{
  try { 
    if (!PyArg_ParseTuple( args, "" ))  
      return NULL; 
    return (PyObject*) new PyColdObject( self->data.typeId() );
  } catch (Error *e) {  
    handleError(e);  
    return 0;
  }
}

// define the methods on PyColdObject
PyMethodDef data_methods[] = {
  PC_METHOD( truth ),
  PC_METHOD( toconstruct ),
  PC_METHOD( toSequence ),
  PC_METHOD( isSequence ),
  PC_METHOD( isMap ),
  PC_METHOD( isIterator ),
  PC_METHOD( length ),
  PC_METHOD( equal ),
  PC_METHOD( order ),
  PC_METHOD( concat ),
  PC_METHOD( slice ),
  PC_METHOD( search ),
  PC_METHOD( insert ),
  PC_METHOD( replace ),
  PC_METHOD( del ),
  PC_METHOD( iterator ),
  PC_METHOD( More ),
  PC_METHOD( Next ),
  PC_METHOD( connect ),
  PC_METHOD( disconnect ),
  PC_METHOD( incoming ),
  PC_METHOD( outgoing ),
  PC_METHOD( typeId ),
  {NULL,		NULL}		/* sentinel */
};

PyObject *cold_op_getattr(PyColdObject *self, char *name)
{
  try {
    // nothing for now
  } catch (Error *e) {  
    handleError(e);  
    return 0;
  }
  
  return Py_FindMethod(data_methods, (PyObject *)self, name);

}
