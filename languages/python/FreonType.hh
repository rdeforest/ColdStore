#ifndef FREONTYPE_HH
#define FREONTYPE_HH

#include <Freon.hh>
#include "coldmodule.hh"

// forward declare freon operations
PyObject * cold_freon_getattr( PyColdObject *self, char *name);

PyTypeObject PyColdFreon_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "freonInterpreter",			/*tp_name*/
    sizeof(PyColdObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)cold_op_dealloc, /*tp_dealloc*/
    0,			/*tp_print*/
    (getattrfunc)cold_freon_getattr, /*tp_getattr*/
    0,                               /*tp_setattr*/
    (cmpfunc)cold_op_compare,			/*tp_compare*/
    (reprfunc)cold_op_getrepr,			/*tp_repr*/
    0,		    /*tp_as_number*/
    0,              /*tp_as_sequence*/
    0,              /*tp_as_mapping*/
    0,		    /*tp_hash*/
    0,              /*tp_call*/
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
