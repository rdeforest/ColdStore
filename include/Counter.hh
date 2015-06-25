union Counter
{
private:
    int counter;

public:
    Counter(int c = 0)
        : counter(c)
        {}

    operator int() {
        return counter;
    }

    void operator++ (int) {
        __asm__ __volatile__(
                             "lock; incl %0"
                             :"=m" (&counter)
                             :"m" (&counter)
                             );
    }

    // true iff counter fell to 0
    bool operator--(int) {
        int ret = 0;
        __asm__ __volatile__ ("decl (%2);jz 1f;incl %0;1:"
                              : "=r" (ret)
                              : "0" (ret), "r" (&counter)
                              : "cc", "memory");
        return ret == 0;
#if 0
        unsigned char retval;
        
        __asm__ __volatile__(
                             "lock ; decl %0; sete %1"
                             :"=m" (counter), "=qm" (retval)
                             :"m" (counter)
                             );
        return retval != 0;
#endif
    }
};

