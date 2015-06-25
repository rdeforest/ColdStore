/* This program has been modified from a BTree implementation whose
 * copyright terms are contained immediately below.
 *
 * Colin McCormack modified the code, placing all modifications under 
 * the License terms here http://www.gnu.org/copyleft/gpl.html, and
 * reproduced herewith.
 *
 * The original author of this code is not responsible for any problems
 * caused by Colin McCormack's modifications.
 */

/*
 *
 *          Copyright (C) 1995, M. A. Sridhar
 *  
 *
 *     This software is Copyright M. A. Sridhar, 1995. You are free
 *     to copy, modify or distribute this software  as you see fit,
 *     and to use  it  for  any  purpose, provided   this copyright
 *     notice and the following   disclaimer are included  with all
 *     copies.
 *
 *                        DISCLAIMER
 *
 *     The author makes no warranties, either expressed or implied,
 *     with respect  to  this  software, its  quality, performance,
 *     merchantability, or fitness for any particular purpose. This
 *     software is distributed  AS IS.  The  user of this  software
 *     assumes all risks  as to its quality  and performance. In no
 *     event shall the author be liable for any direct, indirect or
 *     consequential damages, even if the  author has been  advised
 *     as to the possibility of such damages.
 *
 */
#include <qvmm.h>
#ifndef BTREE_HH
#define BTREE_HH
#include <assert.h>
#include <stdexcept>

template <class T, short _order> class BTree;
template <class T, short _order> class BTreeNode;
template <class T, short _order> class BTreeIterator;

/**
   The Generic templated B-tree
   
   There are three classes related to the generic B-tree:
   @li BTree, a class that encapsulates B-tree algorithms;
   @li BTreeNode, which defines the structure of a node in a B-tree;
   @li BTreeIterator, an object that  allows  inspection of  the items
   in a B-tree in ascending order;
   
   The  algorithms implemented  here assume that items are stored in the
   internal nodes  as  well as in  the leaves;  the definitions of the
   B-tree concepts are based on the ones in Cormen, Leiserson and Rivest's
   book Introduction to Algorithms.
   
   The BTreeNode class encapsulates a single node of the Generic
   B-tree. It is intended primarily for use by the NodeSpace, not by the
   user of the GenericBTree class.
*/
const short MAX_BTREE_HEIGHT = 50;

template <class T, short _order>
class BTreeNode
  : public Memory
{
protected:
    
    short               _keyCount;	// # keys in node
    bool                _isLeaf;		// Is this node a leaf?
    long                _subtreeSize;	// # keys in subtree rooted at this node
    
    T _item[2*_order-1];                // Vector of items of type T
    BTreeNode<T,_order> *_subtree[2*_order];   // Vector of subtrees
    
    friend BTree<T,_order>;
    friend BTreeIterator<T,_order>;
    static T _null;
    
public:

    // ------------------ Access and Manipulation -----------------
    
    // Return the number of items in this node.
    long Size() const {return _keyCount;};
    
    // Return the $i$-th item.  The value $i$ must be such that
    // $0 \le i < {\tt Size()}$.
    T Item (short i) const {return _item[i];};

    // Return the handle of the $i$-th subtree. The value $i$ must be such that
    // $0 \le i \le {\tt Size()}$.
    BTreeNode *Subtree (short i) const {return _subtree[i];}

    // Return the number of keys in the subtree rooted at this node. This
    // method consults an instance variable, and therefore takes constant
    // time; it does not need to  traverse the subtree.
    long SubtreeSize() const {return _subtreeSize;}
    
    // Search the node for the given key; return greatest $i$ such that
    // ${\tt key[i]} \le {\tt key}$. Return true if {\tt key[i] $=$ key}, false
    // otherwise.
    bool Search (const T itm, short& index) const;

    // --------------------- End public protocol -----------------------

    
    // ---------------- Construction and destruction ---------------
  
    // Constructor: create a node of the B-tree with given order.
    // The constructor is protected, because only BTrees may
    // create new nodes.
    BTreeNode ();

    virtual ~BTreeNode();

    static BTreeNode<T,_order> **subtreeAlloc(int size);

protected:
    // perform housekeeping when node is modified
    void modified() {}

    // Shift all the keys and subtrees, beginning at position {\tt pos}
    // right by the given amount. Note that the subtree to the left of
    // {\tt key[pos]} is {\it also\/} moved.
    void ShiftRightAt (short pos, short amount = 1);

    // Shift all the keys and subtrees, beginning at position {\tt pos},
    // left by the given amount. Note that the subtree to the left of
    // {\tt key[pos]} is {\it also\/} moved.
    void ShiftLeftAt (short pos, short amount = 1);


    // MoveSubNode: Move {\tt nkeys\/} keys, and their left and right
    // subtrees, beginning from position {\tt pos} in node $x$ into this node
    // beginning at position {\tt ourPos}.
    void MoveSubNode (BTreeNode& x, short pos, short ourPos, short nkeys);
};

template <class T, short _order> 
/*coldT*/ class BTree
     : public Memory
{
public:
    typedef BTreeIterator<T,_order> iterator;
    // --------------------- Construction and destruction ------------------
    
    // Create a new B-tree of given order. Duplicate items are not
    // allowed. The first parameter specifies the Comparator to be used when
    // comparing two cells. The {\tt order} parameter must be at least 2;
    // anything less than 2 is taken to be 2.
    //
    // The NodeSpace {\tt space} may by created by the derived
    // class and
    // passed to this constructor; if it is NULL, a default in-memory node
    // space is created. If the derived class passes a non-null NodeSpace,
    // it is the responsibility of the derived class to destroy the
    // NodeSpace object.
    BTree();
 
    // Destructor: tells the NodeSpace to destroy all the nodes.
    virtual ~BTree ();

    // ----------------------- Search and related methods ------------------

    // Search the tree for the given item.
    // Return a pointer to the found item in situ if the search was successful.
    // If the search fails, the return value is NULL.
    // The algorithm used is a standard B-tree search algorithm that takes
    // log_d n time in an n-item B-tree of order d.
    T *Find(const T &item) const;
    
    
    // Find and return the minimum item. If the tree is empty,
    // the null pointer is returned. The implementation simply returns the
    // value {\tt ItemWithRank (0)}.
    T *Smallest() const {return ItemWithRank (0);}

    // Find and return the maximum item. If the tree is empty,
    // the null pointer is returned. The implementation simply returns the
    // value {\tt ItemWithRank (Size()$-$1)}.
    T *Largest() const {return ItemWithRank (Size()-1);};


    // Given an index $i$ between 0 and {\tt Size()}$-1$, return the element
    // of rank
    // $i$, i.e., the element that has $i$ elements less than it in the tree.
    // If $i \le 0$, this returns the smallest element, and if $i \ge {\tt
    // Size()}$, this returns the largest element. If the tree is empty,
    // the null value of the base type is returned. The implementation
    // examines only the nodes on the path from the root to the one
    // containing the key sought, and therefore takes no more than $\log
    // n$ time steps with $n$ keys in the tree.
    //
    //   Note that it is possible to iterate through the elements of the tree
    // via calls to this method, varying the index from 0 to ${\tt Size()}-1$;
    // however, this is much less efficient than using the BTreeIterator.
    T *ItemWithRank (long rank) const;
    
    // Return the number of elements in the tree that are less than the
    // parameter.
    long RankOf (T item) const;
    
    // Return the size of the tree (number of items currently present).
    // The implementation needs constant time regardless of tree size.
    long Size () const {return _root?_root->_subtreeSize:0;}


    // ------------------------ Modification ------------------------------

    // Add the item to the tree. Return true if successfully added,
    // false if the item was already in the tree.
    bool Add(T item); 

    // Remove the specified item from the tree.
    // Return NULL if the item was not found in the tree, and the found item otherwise.
    // The implementation needs (in the worst case) two passes over the path
    // to the key, and so takes $2\log n$ time steps with $n$ keys in the
    // tree.
    // It immediately coalesces any underflowing nodes along the path
    // from the root to the deleted key.
    T Remove (T key);
  
    // Remove and return the smallest item in the tree. Return NULL if
    // if the tree is empty.
    T ExtractMin ();

    // --------------------- End public protocol -----------------------
 
        
protected:

    //------------------- Protected helper methods ---------------------

    // update subtree sizes along a search path
    void updSubtree(BTreeNode<T,_order>** stack, int sp);

    enum DeleteActionEnum {NoAction, RotateLeft, RotateRight, Merge};
    
    // Ensure that the node {\tt n1} is non-full, and recurse into it while
    // inserting.
    bool _InsertNonFull (BTreeNode<T,_order>* x, T item);
    
    void _SplitChild (BTreeNode<T,_order>* x, short i, BTreeNode<T,_order>*y);

    BTreeNode<T,_order>* _DescendInto (BTreeNode<T,_order>*node,
                                short subtreeIndex,
                                DeleteActionEnum& action);
  
    BTreeNode<T,_order>* _Adjust (BTreeNode<T,_order>* node, short index,
                           BTreeNode<T,_order>* c0,
                           BTreeNode<T,_order>* c1, DeleteActionEnum& action);
    
    //------------ Instance data -----------------------------
  
    BTreeNode<T,_order>	*_root;
    
    friend BTreeNode<T,_order>;
    friend BTreeIterator<T,_order>;
    
    //  void        NewRoot (BTreeNode<T,_order> *h);
    // Make the node the root of the tree.
    
    // BTreeNode<T,_order>   *CreateNode ();
    // Create a new node and return its address.
    
    BTreeNode<T,_order>   *Root () const {return _root;}
    // Create a new node and return its address.
  
    // void        DestroyNode (BTreeNode<T,_order>*);
    // Destroy the given node.
  
    // void        NodeModified (BTreeNode<T,_order>* n);
    // Tell this NodeSpace that the node {\tt n}'s contents
    // have been modified. Node {\tt n} must have been previously borrowed
    // via the {\tt BorrowNode} method.
};

// A search path is a sequence of pairs of the form <node#, subtree#>, with
// the  first pair <root,  subtree#> and the   last pair being  of the form
// <node#,  key#>. It completely   specifies the path   from the root  to a
// particular key in the tree.
//
template <class T, short _order = 2>
class PathStruct
    : public Memory
{
public:
    BTreeNode<T,_order> *_handle;
    short              _indexInNode;
};

// The BTreeIterator provides iteration over a BTree, with
// the {\tt Reset}, {\tt Next} amd {\tt More} methods. 

// The  BTreeIterator  remembers and   manipulates  the  search path  to  a
// particular key in the tree.
// 
// The Iterator maintains the invariant that the path specified by the
// current values in the array represents the path to the key that was
// returned by the most recent call to Next().

template <class T, short _order>
class BTreeIterator
    : public Memory
{
protected:
    BTreeIterator();
public:
    
    // Constructor: create a BTreeIterator for the given tree {\tt t}.
    BTreeIterator (const BTree<T,_order>& tree);
    
    
    // Copy constructor. The copy inspects the same B-tree as {\tt itr}, and
    // (unless reset) begins  its iteration at the item at which {\tt itr}
    // is currently positioned.
    BTreeIterator (const BTreeIterator<T,_order>& itr);

    
    // Destructor.
    virtual ~BTreeIterator();
    
    // Reset the iterator to the leftmost (smallest) item.
    void Reset();
    
    // Begin the iteration from the given item. The next call to {\tt Next}
    // will return the given item (or the one immediately
    // larger, if the given item isn't in the tree).
    void BeginFrom (T item);
    
    // Tell whether there are more items in the iteration.
    bool More() const;
    
    // Return the next item in the iteration sequence. Return the NULL
    // pointer if no more items.
    T *Next();

    // Return the rank of the element that was returned by the most recent
    // call to {\tt Next()}.
    long CurrentRank () const {return _index;}
    
    // --------------------- End public protocol -----------------------

 
protected:
    PathStruct<T,_order> *_path;		// Stack containing path to current element
    short         _length;	// Length of stack
    long          _index;		// Rank of  element most recently returned by Next
    const      BTree<T,_order>& 	_tree;  // The tree being inspected
};

#ifdef GUARD_ALLOC
#define BIGBTREESIZE(T) ((((((4096-(4+sizeof(Guard)+sizeof(BTreeNode<T,1>)-(3*sizeof(T))))))/Memory::quantum)*Memory::quantum+sizeof(T))/(4*sizeof(T)))
#else
#define BIGBTREESIZE(T) ((((((4096-(4+sizeof(BTreeNode<T,1>)-(3*sizeof(T))))))/Memory::quantum)*Memory::quantum+sizeof(T))/(4*sizeof(T)))
#endif

template <class T> class BigBTree : public BTree<T,BIGBTREESIZE(T)>
{
 public:
  BigBTree();
};

template <class T> class BigBTreeIterator : public BTreeIterator<T,BIGBTREESIZE(T)>
{
 public:
  BigBTreeIterator (const BTree<T,BIGBTREESIZE(T)>& tree);
  BigBTreeIterator (const BTreeIterator<T,BIGBTREESIZE(T)>& it);
};

template <class T> class BigBTreeNode : public BTreeNode<T,BIGBTREESIZE(T)>
{
  // no public constructors
};

#endif
