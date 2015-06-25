
#include "coldmodule.hh"
#include "ScalarType.hh"

//////////////////////
// SCALAR OPERATIONS
//////////////////////

PyObject *cold_op_add( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data + PyToCold( y) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_sub( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data - PyToCold( y) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_mul( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data * PyToCold( y) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_div( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data / PyToCold( y) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_mod( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data % PyToCold( y) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

// PyObject *cold_op_pow(  PyObject *x, PyObject *y, PyObject *z);

PyObject *cold_op_neg( PyColdObject *self)
{
  try
    {
      return (PyObject*) new PyColdObject( self->data->negative() );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_pos( PyColdObject *self)
{
  try
    {
      return (PyObject*) new PyColdObject( self->data->positive()  );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}
PyObject *cold_op_abs( PyColdObject *self)
{
  try
    {
      Slot x = self->data;
      Slot y = 0;
      return (PyObject*) new PyColdObject( (x > y) ? x : x->negative() ) ;
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

int cold_op_nonzero( PyColdObject *self)
{
  try
    {
      return self->data->truth();
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_invert( PyColdObject *self)
{
  try
    {
      return (PyObject*) new PyColdObject( self->data->invert() );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}
PyObject *cold_op_lshift( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data->lshift( PyToCold(y) ) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}
PyObject *cold_op_rshift( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data->rshift( PyToCold( y) ) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_and( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data && PyToCold( y) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_or( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data || PyToCold( y) );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

int cold_op_coerce( PyColdObject **x, PyObject **y)
{
  try
    {
      if (!(PyColdObject_Check( *y ))) {
	Py_INCREF(*x);
	*y = (PyObject*) new PyColdObject( PyToCold(*y) );
	return 0;
      }
      return 1;
    }
  catch (Error *e)
    {
      handleError(e);
      return 1;
    }  
}

PyObject *cold_op_xor( PyColdObject *x, PyObject *y)
{
  try
    {
      return (PyObject*) new PyColdObject( x->data );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}

PyObject *cold_op_int( PyColdObject *self)
{
  try
    {
      return (PyObject*) new PyColdObject( (int)self->data );
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }  
}
