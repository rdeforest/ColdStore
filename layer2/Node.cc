static char *id __attribute__((unused))="$Id: Node.cc,v 1.1 2002/01/26 05:56:45 coldstore Exp $"; 

#include <typeinfo> 

#include "Data.hh"
#include "Store.hh"
#include "String.hh"

#include "List.hh"
#include "Error.hh"
//#include "Frame.hh"

#include "Node.hh"

#include "tSlot.th"
template union tSlot<Node>;

Node::Node(const Node *node)
{
  _lineNumber = node->_lineNumber;
}

Node::Node(int lineNumber)
{
  _lineNumber = lineNumber;
}

Node::Node(int lineNumber, const Slot &A)
{
  _lineNumber = lineNumber;
  push(A);
}
Node::Node(int lineNumber, const Slot &A, const Slot &B)
{
  _lineNumber = lineNumber;
  push(A);
  push(B);
}
Node::Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C)
{
  _lineNumber = lineNumber;
  push(A);
  push(B);
  push(C);
}

Node::Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C, const Slot &D)
{
  _lineNumber = lineNumber;
  push(A);
  push(B);
  push(C);
  push(D);
}

Node::Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C, const Slot &D, const Slot &E)
{
  _lineNumber = lineNumber;
  push(A);
  push(B);
  push(C);
  push(D);
  push(E);
}

Node::Node(int lineNumber, const Slot &A, const Slot &B, const Slot &C, const Slot &D, const Slot &E, const Slot &F)
{
  _lineNumber = lineNumber;
  push(A);
  push(B);
  push(C);
  push(D);
  push(E);
  push(F);
}

Node::Node::~Node()
{}


Node::Node()
{}

ostream &Node::dump(ostream& out) const
{
    pprint(out);
    return out;
} 

Slot Node::toLisp() const
{
  Slot topNode = this;
  Slot str = new String();
  str = str->concat("( ")->concat(topNode->typeId())->concat(" ") ;

  if (topNode.isList()) {
    Slot i(topNode->iterator());
    while (i->More()) {
      str = str->concat( ((Node*)(Data*)i->Next() )->toLisp() );
    }	

  }
  str = str->concat(" )");
  return (char*)str;

}

ostream &Node::pprint(ostream& out, int indent) const
{
    Slot topNode = this;

    if (!this) {
        out << "<nullNode>\n";
        return out;
    }
    out << '\n';
    for (int i = indent; i > 0; i--)
        out << "   ";
    out << "<" << typeid(*this).name() << ", " << _lineNumber << ">: (";

    if (topNode.isList()) {
        Slot i(topNode->iterator());
        while (i->More()) {
            Slot v(i->Next());
            Node *n = dynamic_cast<Node*>((Data*)v);
            if (n) {
                n->pprint(out, indent+1);
                out << ' ';
            } else {
                out << v << ' ';
            }
        }
    }
    out << '\n';
    for (int i = indent; i > 0; i--)
        out << "   ";
    out << ')';
    if (indent == 0)
        out << '\n';
    return out;
}

Node *Node::prepare_edit(Slot &inexpr)
{
    assert((Node*)(Data*)inexpr == this);
    inexpr = (typeof(this))(dynamic_cast<const void*>((Data*)Mutate()));
    //DEBLOG(cerr << "prepare_edit: " << inexpr.typeId() << "\n");
    return  (typeof(this))((Data*)(inexpr));
}

Slot Node::inplace(Slot &context, int el)
{
    Slot &e = (*this)[el];	// get the next element
    Reduce(e, context);	// reduce the name to a constant
    return slice(el);
}

void Node::Reduce(Slot &el, Slot &context)
{
  Slot org(el);
  //Node *node;
  while ((Data*)el && (!el->isAtomic())) {
    
    DEBLOG(cerr << "Reduce: " << node << '\n');
    try {
      //       Reduce(el, context);        // reduce the element in-place
      el->reduce(el, context);
      DEBLOG(cerr << org << " ==> " << el << '\n');
    } catch (Error *err) {
      // the reduction errored.  construct traceback
      err->upcount();
      //context->_line =  ((Node*)(Data*)el)->_lineNumber ; //FUBAR - CMC
      //context->_error = err;
      err->dncount();
      //throw new Frame(context);
    }
  }
}

Data *Node::clone(void *store = (void*)0) const
{
    return new Node(*this);
}

void Node::reduce(Slot &inexpr, Slot &context)
{
    DEBLOG(cerr << "Node reduce: " << this << '\n');
    Reduce(inexpr, context);
    DEBLOG(cerr << "Node reduce result: " << inexpr << '\n');
}
