// coldmodule.hh: cold/python interface (header)
// Copyright (C) 2001, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

#ifndef COLDMODULE_HH
#define COLDMODULE_HH

#include "Python.h"

#include "Data.hh"
#include "Error.hh"

#include "PyColdObject.hh"

// forward declared utility routines
Slot PyToCold( PyObject *x );
PyObject *ColdToPy( Slot x );
void handleError( Error *e );
PyObject *get_cold_names();

static PyObject *NamesObject;

extern PyTypeObject PyColdScalar_Type;
extern PyTypeObject PyColdSequence_Type;
extern PyTypeObject PyColdMapping_Type;
extern PyTypeObject PyColdClosure_Type;
extern PyTypeObject PyColdFreon_Type;
extern PyTypeObject PyColdObject_Type;

// Macro to see if what we're dealing with is a PyColdObject
#define PyColdObject_Check(v) ((v)->ob_type == &PyColdScalar_Type || (v)->ob_type == &PyColdSequence_Type || (v)->ob_type == &PyColdMapping_Type || (v)->ob_type == &PyColdObject_Type || (v)->ob_type == &PyColdClosure_Type  )



////////////////////////////////////////////////////////////////////////
// Macros for defining arguments in the PyColdObject method dictionary

// For Scalars, Sequences, and Mappings

#define PC_NO_ARG( METHODNAME ) PyObject * data_method_##METHODNAME( PyColdObject *self, PyObject *args ) { if (!PyArg_ParseTuple( args, "" )) return NULL; try { Slot returnValue = self->data->METHODNAME(); return (PyObject *)new PyColdObject( returnValue ); } catch (Error *e) {  handleError(e);  return 0;  } }

#define PC_ONE_ARG( METHODNAME ) PyObject * data_method_##METHODNAME( PyColdObject *self, PyObject *args ) { try { PyObject *arg; if (!PyArg_ParseTuple( args, "O", &arg ))  return NULL; Slot argument = PyToCold( arg ); Slot returnValue = self->data->METHODNAME( argument ); return (PyObject *)new PyColdObject( returnValue ); } catch (Error *e) {  handleError(e);  return 0; } }

#define PC_ONE_OR_TWO_ARG( METHODNAME ) PyObject * data_method_##METHODNAME( PyColdObject *self, PyObject *args ) { try { PyObject *arg1, *arg2; arg2 = 0; if (!PyArg_ParseTuple( args, "O|O", &arg1, &arg2 ))  return NULL; Slot argument1 = PyToCold( arg1 ); if (arg2) { Slot argument2 = PyToCold( arg2 ); return (PyObject*) new PyColdObject( self->data->METHODNAME( argument1, argument2 )); } else { return (PyObject*) new PyColdObject(self->data->METHODNAME( argument1 )); } } catch (Error *e) {  handleError(e);  return 0; } }

#define PC_TWO_ARG( METHODNAME ) PyObject * data_method_##METHODNAME( PyColdObject *self, PyObject *args ) { try { PyObject *arg1, *arg2; if (!PyArg_ParseTuple( args, "OO", &arg1, &arg2 ))  return NULL; Slot argument1 = PyToCold( arg1 ); Slot argument2 = PyToCold( arg2 ); Slot returnValue = self->data->METHODNAME( argument1, argument2 ); return (PyObject *)new PyColdObject( returnValue ); } catch (Error *e) {  handleError(e);  return 0; } }

#define PC_METHOD( METHODNAME ) { "" #METHODNAME "", (PyCFunction)data_method_##METHODNAME, METH_VARARGS }

// for objects

#define PO_NO_ARG( PYMETHODNAME, METHODNAME ) PyObject * object_method_##PYMETHODNAME( PyColdObject *self, PyObject *args ) { if (!PyArg_ParseTuple( args, "" )) return NULL; try { Slot returnValue = ((Object*)(Data*)self->data)->METHODNAME(); return (PyObject *)new PyColdObject( returnValue ); } catch (Error *e) {  handleError(e);  return 0;  } }

#define PO_ONE_ARG( PYMETHODNAME, METHODNAME ) PyObject * object_method_##PYMETHODNAME( PyColdObject *self, PyObject *args ) { try { PyObject *arg; if (!PyArg_ParseTuple( args, "O", &arg ))  return NULL; Slot argument = PyToCold( arg ); Slot returnValue = ((Object*)(Data*)self->data)->METHODNAME( argument ); return (PyObject *)new PyColdObject( returnValue ); } catch (Error *e) {  handleError(e);  return 0; } }

#define PO_METHOD( METHODNAME ) { "" #METHODNAME "", (PyCFunction)object_method_##METHODNAME, METH_VARARGS }

// for frames

#define PF_NO_ARG( PYMETHODNAME, METHODNAME ) PyObject * frame_method_##PYMETHODNAME( PyColdObject *self, PyObject *args ) { if (!PyArg_ParseTuple( args, "" )) return NULL; try { Slot returnValue = ((Closure*)(Data*)self->data)->METHODNAME(); return (PyObject *)new PyColdObject( returnValue ); } catch (Error *e) {  handleError(e);  return 0;  } }

#define PF_ONE_ARG( PYMETHODNAME, METHODNAME ) PyObject * frame_method_##PYMETHODNAME( PyColdObject *self, PyObject *args ) { try { PyObject *arg; if (!PyArg_ParseTuple( args, "O", &arg ))  return NULL; Slot argument = PyToCold( arg ); Slot returnValue = ((Closure*)(Data*)self->data)->METHODNAME( argument ); return (PyObject *)new PyColdObject( returnValue ); } catch (Error *e) {  handleError(e);  return 0; } }

#define PF_METHOD( METHODNAME ) { "" #METHODNAME "", (PyCFunction)frame_method_##METHODNAME, METH_VARARGS }


#endif
 
