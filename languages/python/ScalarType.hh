#ifndef SCALARTYPE_HH
#define SCALARTYPE_HH

#include "coldmodule.hh"

// forward declared scalar operations
PyObject * cold_op_add( PyColdObject *x, PyObject *y);
PyObject * cold_op_sub( PyColdObject *x, PyObject *y);
PyObject * cold_op_mul( PyColdObject *x, PyObject *y);
PyObject * cold_op_div( PyColdObject *x, PyObject *y);
PyObject * cold_op_mod( PyColdObject *x, PyObject *y);
int cold_op_coerce( PyColdObject **x, PyObject **y);
//PyObject * cold_op_divmod( PyColdObject *x, PyObject *y);
//PyObject * cold_op_pow( PyColdObject *x, PyObject *y, PyObject *z);
PyObject * cold_op_neg( PyColdObject *self);
PyObject * cold_op_pos( PyColdObject *self);
PyObject * cold_op_abs( PyColdObject *self);
int       cold_op_nonzero( PyColdObject *self);
PyObject * cold_op_invert( PyColdObject *self);
PyObject * cold_op_lshift( PyColdObject *x, PyObject *y);
PyObject * cold_op_rshift( PyColdObject *x, PyObject *y);
PyObject * cold_op_and( PyColdObject *x, PyObject *y);
PyObject * cold_op_xor( PyColdObject *x, PyObject *y);
PyObject * cold_op_or( PyColdObject *x, PyObject *y);
PyObject * cold_op_int( PyColdObject *self);
//PyObject * cold_op_long( PyObject *self);
//PyObject * cold_op_float( PyObject *self);
//PyObject * cold_op_oct( PyObject *self);
//PyObject * cold_op_hex( PyObject *self);

static PyNumberMethods slot_as_scalar = {
        (binaryfunc)cold_op_add, /*nb_add*/
        (binaryfunc)cold_op_sub, /*nb_subtract*/
        (binaryfunc)cold_op_mul, /*nb_multiply*/
        (binaryfunc)cold_op_div, /*nb_divide*/
        (binaryfunc)cold_op_mod, /*nb_remainder*/
	// UNIMPLEMENTED
        0,//(binaryfunc)cold_op_divmod, /*nb_divmod*/
        0,//(ternaryfunc)cold_op_pow, /*nb_power*/
        (unaryfunc)cold_op_neg, /*nb_negative*/
        (unaryfunc)cold_op_pos, /*nb_positive*/
        (unaryfunc)cold_op_abs, /*nb_absolute*/
        (inquiry)cold_op_nonzero, /*nb_nonzero*/
        (unaryfunc)cold_op_invert, /*nb_invert*/
        (binaryfunc)cold_op_lshift, /*nb_lshift*/
        (binaryfunc)cold_op_rshift, /*nb_rshift*/
        (binaryfunc)cold_op_and, /*nb_and*/
        (binaryfunc)cold_op_xor, /*nb_xor*/
        (binaryfunc)cold_op_or, /*nb_or*/
        (coercion)cold_op_coerce, /*nb_coerce*/
        (unaryfunc)cold_op_int, /*nb_int*/
        0,//(unaryfunc)cold_op_long, /*nb_long*/
        0,//(unaryfunc)cold_op_float, /*nb_float*/
        0,//(unaryfunc)cold_op_oct, /*nb_oct*/
        0,//(unaryfunc)cold_op_hex, /*nb_hex*/
};

PyTypeObject PyColdScalar_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "coldScalar",			/*tp_name*/
    sizeof(PyColdObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)cold_op_dealloc, /*tp_dealloc*/
    0,			/*tp_print*/
    (getattrfunc)cold_op_getattr, /*tp_getattr*/
    0,//(setattrfunc)cold_op_setattr, /*tp_setattr*/
    (cmpfunc)cold_op_compare,			/*tp_compare*/
    (reprfunc)cold_op_getrepr,			/*tp_repr*/
    &slot_as_scalar,			/*tp_as_number*/
    0,           	/*tp_as_sequence*/
    0,	                /*tp_as_mapping*/
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
