#include "coldmodule.hh"

#include "MappingType.hh"

///////////////////////
// MAPPING OPERATIONS
///////////////////////

// slice by non-int
PyObject *cold_op_subscript( PyColdObject *self, PyObject *key)
{
  try
    {
      Slot cKey = PyToCold(key);
      Slot value;
      value = self->data->slice(cKey);
      
      PyColdObject *ret = new PyColdObject( value );
      
      return (PyObject *)ret;
    }
  catch (Error *e)
    {
      handleError(e);
      return NULL;
    }

}

// assign value by key
int cold_op_ass_sub(PyColdObject *self, PyObject *k, PyObject *v)
{
  try
    {

      Slot key = PyToCold(k);
      Slot value = PyToCold(v);
      self->data->insert( key, value );
      return 0;
    }
  catch (Error *e)
    {
      handleError(e);
      return 0;
    }

}
