#ifndef SEQUENCETYPE_HH
#define SEQUENCETYPE_HH

#include "coldmodule.hh"

// forward declared sequence operations
int       cold_op_length( PyColdObject *self);
int       cold_op_contains( PyColdObject *a, PyObject *el);
PyObject *cold_op_concat( PyColdObject *left, PyObject *right);
PyObject *cold_op_item( PyColdObject *seq, int index);
PyObject *cold_op_slice( PyColdObject *seq, int index, int end);
PyObject *cold_op_setitem( PyColdObject *seq, int index, PyObject *item);

// sequence operations methods
static PySequenceMethods slot_as_sequence = {
    (inquiry)    cold_op_length,             /* __len__ */
    (binaryfunc) cold_op_concat,          /* __add__ */
    // UNIMPLEMENTED
    0,//(intargfunc)sq_repeat;          /* __mul__ */
    (intargfunc)cold_op_item,            /* __getitem__ */
    (intintargfunc)cold_op_slice,        /* __getslice__ */
    (intobjargproc)cold_op_setitem,     /* __setitem__ */
    // UNIMPLEMENTED
    0,//(intintobjargproc)cold_ass_slice; /* __setslice__ */
    (objobjproc)cold_op_contains, /*sq_contains*/
    //(binaryfunc)cold_inplace_concat, /*sq_inplace_concat*/
    //(intargfunc)cold_inplace_repeat, /*sq_inplace_repeat*/
};

PyTypeObject PyColdSequence_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "coldSequence",			/*tp_name*/
    sizeof(PyColdObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)cold_op_dealloc, /*tp_dealloc*/
    0,			/*tp_print*/
    (getattrfunc)cold_op_getattr, /*tp_getattr*/
    0,//(setattrfunc)cold_op_setattr, /*tp_setattr*/
    (cmpfunc)cold_op_compare,			/*tp_compare*/
    (reprfunc)cold_op_getrepr,			/*tp_repr*/
    0,//&slot_as_scalar,			/*tp_as_number*/
    &slot_as_sequence,           	/*tp_as_sequence*/
    0,          	/*tp_as_mapping*/
    0,			/*tp_hash*/
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
