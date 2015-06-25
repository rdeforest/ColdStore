// Tape - a funky random access iterator
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
//      $Id

#include "Data.hh"
#include "Tape.hh"
#include "List.hh"
#include "String.hh"


template class Tape<String>;

template <class T>
Tape<T>::Tape( const Slot &from, const signed int startPosition = 0 )
  : T( ((T*)(Data*)from) )
{
  mPosition = startPosition;
  mMarkers = new List();
}

template <class T>
bool Tape<T>::More() const {
  return mPosition < length() ? true : false;
}

template <class T>
Slot Tape<T>::Next() {
  Slot res = Peek();
  mPosition++;
  return res;
}


template <class T>
Slot Tape<T>::Peek() {
  return slice(mPosition);
}

template <class T>
void Tape<T>::Mark() {
  mMarkers = ((List*)(Data*)mMarkers)->push(mPosition);
}

template <class T>
void Tape<T>::Pop() {
  Slot position;

  mMarkers = ((List*)(Data*)mMarkers)->pop(position);
}

template <class T>
void Tape<T>::Reset() {
  Slot position;
  mMarkers = ((List*)(Data*)mMarkers)->pop(position);
  mPosition = position;
}

template <class T>
void Tape<T>::Skip( signed int amount ) {
  mPosition = mPosition + amount;
}

template <class T>
void Tape<T>::Rewind() {
  mPosition = 0;
}

template <class T>
bool Tape<T>::Follows( const Slot &match ) const {
  return slice( 0, match->length() ) == match ? true : false;
}

template <class T>
int Tape<T>::Tell() const {
  return mPosition;
}

