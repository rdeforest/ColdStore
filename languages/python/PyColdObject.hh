#ifndef PYCOLDOBJECT_HH
#define PYCOLDOBJECT_HH

#include <Object.hh>
#include <Frame.hh>
#include <strstream>

struct PyColdObject
{
    PyObject_HEAD

    /** actual data value
     */
    Slot data;

    /** basic constructor, creates a blank (null) PyColdObject
     *  handles refcounts
     *  @exceptions none
     */
    PyColdObject ();

    /** constructor with value, creates a PyColdObject with
     *  handles refcount
     *  the value added.
     */
    PyColdObject ( const Slot inSlot );

};

// forward declared basic operations
void      cold_op_dealloc( PyColdObject *self);
PyObject *cold_op_getattr( PyColdObject *self, char *name);
PyObject *cold_op_getrepr( PyColdObject *self);
int       cold_op_compare( PyColdObject *left, PyColdObject *right);


#endif
