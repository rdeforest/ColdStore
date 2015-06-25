#ifndef MAPPINGTYPE_HH
#define MAPPINGTYPE_HH

// forward declare map operations
PyObject * cold_op_subscript( PyColdObject *seq, PyObject *key);
int cold_op_ass_sub(PyColdObject *mp, PyObject *v, PyObject *w);
int       cold_op_length( PyColdObject *self);

// mapping operations methods
static PyMappingMethods slot_as_mapping = {
    (inquiry)cold_op_length, /*mxp_length*/
    (binaryfunc)cold_op_subscript, /*mp_subscript*/
    (objobjargproc)cold_op_ass_sub, /*mp_ass_subscript*/
};

// basic coldType method definitions
PyTypeObject PyColdMapping_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "coldMapping",			/*tp_name*/
    sizeof(PyColdObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)cold_op_dealloc, /*tp_dealloc*/
    0,			/*tp_print*/
    (getattrfunc)cold_op_getattr, /*tp_getattr*/
    0,//(setattrfunc)cold_op_setattr, /*tp_setattr*/
    (cmpfunc)cold_op_compare,			/*tp_compare*/
    (reprfunc)cold_op_getrepr,			/*tp_repr*/
    0,			/*tp_as_number*/
    0,	                /*tp_as_sequence*/
    &slot_as_mapping,	/*tp_as_mapping*/
    0,			/*tp_hash*/
    // (BELOW IS Python2.0+ ONLY)

    0,//(reprfunc)class_str, /*tp_str*/
    0,//(getattrofunc)class_getattr, /*tp_getattro*/
    0,//(setattrofunc)class_setattr, /*tp_setattro*/
    0,//0,              /* tp_as_buffer */
    0,//Py_TPFLAGS_DEFAULT | Py_TPFLAGS_GC, /*tp_flags*/
    0,              /* tp_doc */
    0,//(traverseproc)class_traverse,   /* tp_traverse */
};

#endif
