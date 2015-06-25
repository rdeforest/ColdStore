#include "coldmodule.hh"

#include "ObjectType.hh"


///////////////////////
// ROOT OBJECT METHODS
///////////////////////

PO_ONE_ARG( add_parent, addParent );
PO_ONE_ARG( add_child, addChild );
PO_ONE_ARG( del_child, delChild );
PO_ONE_ARG( del_parent, delParent );
PO_NO_ARG( create_child, createChild );
PO_NO_ARG( parents, parents );
PO_NO_ARG( children, children );

// define the methods on PyColdObject
PyMethodDef object_methods[] = {
  PO_METHOD( add_parent ),
  PO_METHOD( add_child ),
  PO_METHOD( del_child ),
  PO_METHOD( del_parent ),
  PO_METHOD( parents ),
  PO_METHOD( children ),
  PO_METHOD( create_child ),
  {NULL,		NULL}		/* sentinel */
};

PyObject *cold_obj_getattr(PyColdObject *self, char *name)
{
  PyObject *result;
  // first try the object's namespace for the key
  try {
    Slot res = self->data->slice(name)[2];
    result = (PyObject*) new PyColdObject( res );
  } catch (Error *e) {
    result = Py_FindMethod(object_methods, (PyObject *)self, name);
    if (!strcmp( name, "__methods__")) {
      Slot keys = self->data->iterator();
      while (keys->More()) {
	Slot n = keys->Next();
	PyList_Append( result, PyString_FromString( n[1]) );
      }
    }
  }

  return result;

}

// set the attributes of a PyColdObject
// most things have readonly attributes -- only classes and instances
// can be changed.
int cold_obj_setattr(PyColdObject *self, char *name, PyObject *v)
{
  try {
    Slot what = PyToCold( v );
    self->data->insert( name, what );
    return 0;
  } catch (Error *e) {  
    handleError(e);  
    return 0;
  }
}
 
PyObject * cold_obj_call( PyColdObject *x, PyObject *y, PyObject *z)
{
  // we ignore arguments

  Slot parent = x->data;
  return (PyObject*) new PyColdObject( ((Object*)(Data*)parent)->createChild() );
}
