#include "coldmodule.hh"
#include "FunctionType.hh"
#include "PyFunction.hh"

////////////////////////
// FUNCTION OPERATIONS
////////////////////////

PyObject *nativeFunction( PyColdObject *x )
{
  PyFunction *func = ((PyFunction*)(Data*)x->data);

  if (func->native)
    return func->native;
  else
    return func->native = ColdToPy( func );
  
}

PyObject * cold_func_call( PyColdObject *x, PyObject *y, PyObject *z)
{
  return PyEval_CallObject( nativeFunction(x), y );
}

PyObject * func_method_del( PyColdObject *self, PyObject *args ) 
{
  try {
    if (!PyArg_ParseTuple( args, "") )
	return NULL;
    cerr << "DELETING" << endl;
    return 0;
  } catch (Error *e) {
    handleError(e);
    return 0;
  }
}

// define the methods on PyColdObject
PyMethodDef func_methods[] = {
  { "__del__", (PyCFunction)func_method_del, METH_VARARGS }, 
  { NULL,                NULL}           /* sentinel */
};

PyObject *cold_func_getattr(PyColdObject *self, char *name)
{
  PyObject *result, *nresult;

  if (strcmp( name, "__del__") == 0) 
    return Py_FindMethod(func_methods, (PyObject *)self, name);

  nresult = PyObject_GetAttr( nativeFunction(self), PyString_FromString(name) );
  
  if (strcmp( name, "__members__") == 0) {
    PyList_Append( nresult, PyString_FromString("__del__") );
    return nresult;
  }

  return nresult;
}

int cold_func_setattr( PyColdObject *self, char *name, PyObject *v)
{
  return PyObject_SetAttr( nativeFunction(self), PyString_FromString(name), v );
}

long cold_func_hash( PyColdObject *self)
{
  return PyObject_Hash( nativeFunction(self) );
}

// PyObject *cold_func_getrepr( PyColdObject *self)
// {
//   return PyObject_Repr( nativeFunction(self) );
// }

int cold_func_compare( PyColdObject *self, PyObject *right)
{
  return PyObject_Compare( nativeFunction(self), right );
}

void cold_func_dealloc( PyColdObject *self) 
{
  return PyMem_DEL( nativeFunction(self) );
}
