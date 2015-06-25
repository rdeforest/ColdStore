#ifndef CLOSURETYPE_HH
#define CLOSURETYPE_HH

#include <Closure.hh>

// forward declare frame operations
PyObject * cold_closure_call( PyColdObject *x, PyObject *y, PyObject *z);
PyObject * cold_closure_getattr( PyColdObject *self, char *name);
int        cold_closure_setattr( PyColdObject *self, char *name, PyObject *v);

PyTypeObject PyColdClosure_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "coldClosure",			/*tp_name*/
    sizeof(PyColdObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)cold_op_dealloc, /*tp_dealloc*/
    0,			/*tp_print*/
    (getattrfunc)cold_closure_getattr, /*tp_getattr*/
    (setattrfunc)cold_closure_setattr, /*tp_setattr*/
    (cmpfunc)cold_op_compare,			/*tp_compare*/
    (reprfunc)cold_op_getrepr,			/*tp_repr*/
    0,			/*tp_as_number*/
    0,           	/*tp_as_sequence*/
    0,  	/*tp_as_mapping*/
    0,			/*tp_hash*/
    (ternaryfunc)cold_closure_call,              /*tp_call*/
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
