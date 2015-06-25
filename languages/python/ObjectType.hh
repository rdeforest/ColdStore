#ifndef OBJECTTYPE_HH
#define OBJECTTYPE_HH

#include <Object.hh>

// forward declared objecttype  operations
PyObject *cold_obj_getattr( PyColdObject *self, char *name);
int       cold_obj_setattr( PyColdObject *self, char *name, PyObject *v);
PyObject *cold_obj_call( PyColdObject *x, PyObject *y, PyObject *z); 

PyTypeObject PyColdObject_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "coldObject",			/*tp_name*/
    sizeof(PyColdObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)cold_op_dealloc, /*tp_dealloc*/
    0,			/*tp_print*/
    (getattrfunc)cold_obj_getattr, /*tp_getattr*/
    (setattrfunc)cold_obj_setattr, /*tp_setattr*/
    (cmpfunc)cold_op_compare,			/*tp_compare*/
    (reprfunc)cold_op_getrepr,			/*tp_repr*/
    0,			/*tp_as_number*/
    0,           	/*tp_as_sequence*/
    0,	                /*tp_as_mapping*/
    0,			/*tp_hash*/
    (ternaryfunc)cold_obj_call,              /*tp_call*/
    0,              /*tp_str*/
    0,              /*tp_getattro*/
    0,              /*tp_setattro*/
    0,              /*tp_as_buffer*/
    0,//Py_TPFLAGS_DEFAULT | Py_TPFLAGS_GC,     /*tp_flags*/
    0,              /* tp_doc */
    0,//(traverseproc)cold_op_traverse,    /* tp_traverse */
    0,//(inquiry)list_clear,    /* tp_clear */
};

#endif
