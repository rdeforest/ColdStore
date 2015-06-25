#include <stdlib.h>

template <class T>
union Link
{
protected:
    T *link;

    // returns wether or not *mem == oldval and the operation was successful.
    static __inline__ int CaS(T* oldval, T *newval) {
        unsigned char c;
    
#define LOCK "lock ; "
        asm(LOCK "cmpxchg %5,%4\n\tsete %1":
            "=a" (oldval),"=qm" (c),"=m" (&link):
            "a" (oldval), "m" (&link),"r" (newval));
        
        return c;
    }

public:
    Link()
        : link(NULL)
        {}

    Link(T *t)
        : link(t)
        {}
        
    T *operator->() {
        return link;
    }

    operator =(Link<T> *next) {
        push(next);
    }

    void push(T *t) {
        t->link = next;
        while (!CaS(t->link, next)) {
            if (sched_yield()) {
            }
            t->link = next;
        }
    }

    // NB: this must be the designated head
    T *pop() {
        T *retval = next;
        while (!CaS(retval, retval->link)) {
            if (sched_yield()) {
            }
            retval = next;
        }
        return retval;
    }
};

