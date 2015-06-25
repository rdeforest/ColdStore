#ifndef TNODE_HH
#define TNODE_HH

#include "Node.hh"

template <class T>
coldT class TNode: public Node {
public:
	TNode(int lineNumber) : Node(lineNumber) {
	}
	TNode(int lineNumber, const Slot &A) : Node(lineNumber,A) {
	}
	TNode(int lineNumber, const Slot &A, const Slot &B) : Node(lineNumber,A,B) {
	}
	TNode(int lineNumber, const Slot &A, const Slot &B, const Slot &C) : Node(lineNumber,A,B,C) {
	}
	TNode(int lineNumber, const Slot &A, const Slot &B, const Slot &C, const Slot &D) : Node(lineNumber,A,B,C,D) {
	}
	TNode(const Node* anode) : Node(anode) {
	}
	TNode() : Node() {
	}

	TNode(const T* node) : Node(node) {
	}

	virtual Data *clone(void *store = (void*)0) const {
		return new T(this);
	}
};


#endif //TNODE_HH

