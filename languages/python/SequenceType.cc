#include "coldmodule.hh"
#include "SequenceType.hh"

////////////////////////
// SEQUENCE OPERATIONS
////////////////////////

// return the length of the object
int cold_op_length(PyColdObject *self)
{
  try {
    return (int)self->data->length();
  }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }

}

// slice by int
PyObject *cold_op_item( PyColdObject *self, int index)
{
  try
    {
      Slot value = self->data->slice(index);
      PyColdObject *ret = new PyColdObject( value );
      return (PyObject *)ret;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }
 
}

// *el in *self
int cold_op_contains( PyColdObject *self, PyObject *el)
{
  try
    {
      // THIS SUCKS
      // List::search appears broken, so we use our own
      
      Slot it = self->data->iterator();
      Slot comp = PyToCold(el);
      while (it->More()) {
	if ( (it->Next().equal(comp)) )
	  return 1;
      }
      return 0;
    }
  catch (Error *e)
    {
      handleError(e);
      return -1;
    }

}

// slice by int .. int
PyObject *cold_op_slice( PyColdObject *self, int index, int rindex)
{
  try
    {
      Slot value = self->data->slice(index, rindex);
      PyColdObject *ret = new PyColdObject( value );
      return (PyObject *)ret;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}

// assign value by index
PyObject *cold_op_setitem( PyColdObject *self, int index, PyObject *item)
{
  try
    {
      Slot value = PyToCold(item);
      self->data = self->data->replace( index , value );
      return 0;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}

// concatenate
PyObject *cold_op_concat( PyColdObject *left, PyObject *right)
{
  try
    {
      PyColdObject *ret = new PyColdObject;
      DEBLOG(cerr << "cold_op_concat ret: " << *(int*)&(ret->data) << '\n');
      if (ret == NULL)
	return NULL;
      if (PyColdObject_Check( right))
        {
	  DEBLOG(cerr << ((PyColdObject*)right)->data << '\n');
	  ret->data = left->data->concat( ((PyColdObject*)right)->data );
        }
      else
        {
	  // either raise an error, or convert it to a cold type!!
	  // (i.e. we need a general method for converting python->layer1
	  Slot x = PyToCold( right );
	  DEBLOG(cerr << "cold_op_concat: " << x << '\n');
	  ret->data = left->data->concat( x );
        }

      return (PyObject *)ret;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}

