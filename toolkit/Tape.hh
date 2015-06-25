// Tape - a funky iterator thing
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
//      $Id

#ifndef __BUFFER_HH
#define __BUFFER_HH 

#include "Data.hh"
#include "Tuple.hh"

// Tchinek says, "More wasn't built in a day."

/** Tape
 * 
 *  An iteration object that is templated around any sequence
 *  class to provide random access iteration ability.
 */

template <class T>
class Tape
  : public T
{
private:
  signed int mPosition;
  Slot mMarkers;

public:
  Tape( const Slot &from, const signed int startPosition = 0 );

public:

  /** true if more elements are available
   *  @return true if more elements available
   */
  virtual bool More() const;

  /** return current element and increment
   *  @return next element
   */
  virtual Slot Next();

  /** peek at the current element without incrementing
   *  @return current element
   */
  virtual Slot Peek() ;

  /** push a location into stack for return on later Reset()
   */
  virtual void Mark();

  /** reset to previously marked location on stack
   */
  virtual void Reset();

  /** pop a marker off stack without reset
   */
  virtual void Pop();

  /** skip ahead/back by amount
   *  @param amount amount to skip by (positive or negative)
   */
  virtual void Skip( signed int amount );

  /** rewind to beginning of iteration
   */
  virtual void Rewind();

  /** return true if match is subsequently available
   *  @return true if match found
   */
  virtual bool Follows( const Slot &match ) const;

  /** current position in the iteration
   *  @return position in the iteration
   */
  virtual int Tell() const;
};

#endif /* __BUFFER_HH */
