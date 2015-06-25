#include <elf.hh>
#include <Data.hh>

void initElf(Elf *elf);

class Skeleton : public Data
{
protected:
  /** copy constructor
   *
   * Used as the destination of @ref mutate when a copy on write mutable
   * object is desired.
   * @param where the locale into which to clone this object
   * @return mutable copy of object
   */
  virtual Data *clone(void *where = (void*)0) const;
  
  /** order two objects
   * @param arg object to be ordered relative to this
   * @return 1,0,-1 depending on object order
   */
  virtual int order(const Slot &arg) const;
};
