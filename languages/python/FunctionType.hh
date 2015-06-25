#ifndef FUNCTIONTYPE_HH
#define FUNCTIONTYPE_HH

// forward declare function operations
PyObject     *cold_func_call( PyColdObject *x, PyObject *y, PyObject *z);
long          cold_func_hash( PyColdObject *f );
PyObject  *cold_func_getattr( PyColdObject *self, char *name);
int        cold_func_setattr( PyColdObject *self, char *name, PyObject *v);   
int        cold_func_compare( PyColdObject *self, PyObject *x );
void       cold_func_dealloc( PyColdObject *self);

PyTypeObject PyColdFunction_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "coldFunction",			/*tp_name*/
    sizeof(PyColdObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)cold_func_dealloc, /*tp_dealloc*/
    0,			/*tp_print*/
    (getattrfunc)cold_func_getattr, /*tp_getattr*/
    (setattrfunc)cold_func_setattr, /*tp_setattr*/
    (cmpfunc)cold_func_compare,			/*tp_compare*/
    (reprfunc)cold_op_getrepr,			/*tp_repr*/
    0,			/*tp_as_number*/
    0,           	/*tp_as_sequence*/
    0,	                /*tp_as_mapping*/
    0,//(hashfunc)cold_func_hash,			/*tp_hash*/
    (ternaryfunc)cold_func_call,              /*tp_call*/
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
