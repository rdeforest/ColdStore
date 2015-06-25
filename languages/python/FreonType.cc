#include "coldmodule.hh"
#include "FreonType.hh"

/////////////////////
// FREON OPERATIONS
/////////////////////

PyObject * freon_method_compile( PyColdObject *self, PyObject *args )
{
  try { 
    PyObject *arg1;
    if (!PyArg_ParseTuple( args, "O", &arg1 ))  
      return NULL; 
    const Slot argument1 = PyToCold( arg1 );
    Slot ret = ((Freon*)(Data*)self->data)->compile( argument1 );
    return (PyObject*) new PyColdObject( ret );
  } catch (Error *e) {  
    handleError(e);  
    return 0;
  }
}

PyMethodDef freon_methods[] = {
  { "compile", (PyCFunction)freon_method_compile, METH_VARARGS },
  {NULL,		NULL}		/* sentinel */
};

PyObject *cold_freon_getattr( PyColdObject *self, char *name)
{
  return Py_FindMethod(freon_methods, (PyObject *)self, name);
}
