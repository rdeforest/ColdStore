void Latch::lock( void* target)
{
    size_t i, j, n;
    char r;

    /* look for a free spot to lock:
       start at (target%size)
       advance (mod size) until you find the spot.
       beware of infinite loops when table is full.
    */

    for (n=i=((size_t)target) % size; ; i = j) {
        // find two consecutive free entries
        if ((j=i+1) == size) j = 0;
        if (j == n) {
            qvmm_throw("Too many latches __FILE__ __LINE__.");
        }

        // look for a first one
        r = 1;
        asm volatile ("xchgb (%2),%0" : "=r"(r) : "0"(r),
                      "r"(latch+i));
        if (r) continue;

        // look for a second
        r = 1;
        asm volatile ("xchgb (%2),%0" : "=r"(r) : "0"(r),
                      "r"(latch+j));
        if (!r) break;

        // we don't have the second.  release the first and keep looking
        latch[i] = 0;
        if (++j == size) j = 0;
        if (j == n) {
            qvmm_throw("Too many latches __FILE__ __LINE__.");
        }
    }

    // got two consecutive entries.  release the second.  `i' is our spot
    latch[j] = 0;

    /* wait for the target lock:
       start from our spot.
       wait for previous (mod size) entry to be free.
       lock previous and unlock current.
       repeat starting from previous until we have the target lock
    */
    while (i != n) {
        j = i ? i - 1 : size - 1;
        
        // wait for previous lock
        for (r = 1;;) {
            asm volatile ("xchgb (%2),%0" : "=r"(r) : "0"(r),
                          "r"(latch+j));
            if (!r) break;
            usleep(timeout);
        }

        // release current lock
        latch[i] = 0;
        i = j;
    }
}

/** unlock a latch
 */
void Latch::unlock( void* target) {
    latch[((size_t)target)%size] = 0;
}
