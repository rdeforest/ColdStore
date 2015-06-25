#include <typeinfo>
#include <iostream>

metaclass ColdClass;

class Unknown;

template <class T>
class Vector
{
 public:
    static unsigned int extra_allocation;
};

cold class Base {
    public:
    //virtual bool isAKO(const type_info&);
    void junk() {
        typeid(Base);
    }
};

cold struct Derived
: public Base
{
};

int main(int argc, char **argv)
{
    Derived d;
    Base b;

    cerr << d.isAKO(typeid(Derived));
    cerr << d.isAKO(typeid(Base));

    cerr << b.isAKO(typeid(typeid(Derived)));
    cerr << b.isAKO(typeid(Base));
}

