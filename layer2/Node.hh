#ifndef NODE_HH
#define NODE_HH


#include "Data.hh"  
#include "Slot.hh"
#include "List.hh"

class Node
  : public List
{
  
public:
  /** create a Node
   */
  int _lineNumber;

  Node(int lineNumber);
  Node(int lineNumber, const Slot &A);
  Node(int lineNumber, const Slot &A, const Slot &B);
  Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C);
  Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C, const Slot &D);
  Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C, const Slot &D, const Slot &E);
  Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C, const Slot &D, const Slot &E, const Slot &F);
  Node(); 
  Node(const Node *node);
  virtual Data *clone(void *store = (void*)0) const;
  
  /** call the Node
   * @param environment arguments 
   */
  
  virtual ~Node();

  /** reduce the Slot in a given context
   * @param what Slot to be reduced
   * @param context context in which to reduce Slot
   */
  static void Reduce(Slot &what, Slot &context);

protected:
  /** reduce this Node
   * @param inexpr reference to Slot to reduce
   * @param context context in which to reduce Slot
   */
  virtual void reduce(Slot &inexpr, Slot &context);

  /** make this a mutable Node
   * @param inexpr location in which this Node appears
   */
  virtual Node *prepare_edit(Slot &inexpr);

public:
  virtual bool isAtomic() const { return false; };

  /** reduce the an element of this Node in-place in the Node, in the given context
   * @param context context within which to reduce the Node's element
   * @param el index of element to reduce
   * @return the reduced element
   */
  Slot inplace(Slot &context, int el);
  ostream& dump(ostream& out) const;

  // show me as a lispish string
  Slot toLisp() const;
  virtual ostream &pprint(ostream &out, int indent = 0) const;
};

#endif
