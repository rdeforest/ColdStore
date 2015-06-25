#ifdef GUARD_ALLOC
/// Guard/wall structure laminated onto each Memory allocation, protects against overrun
class Guard
{
    size_t magic;		/// an arbitrary value
    void *locale;		/// region's locale
    void *address;		/// region's internal start address
    size_t size;		/// region's external size
    size_t checksum;		/// checksum of Guard fields

public:
    static const size_t Magic = 0x87654321;

    // placement storage allocation is a NOOP
    void *operator new (size_t s, void *address) {
        return address;
    }
    void operator delete(void *allocation) {}

    // Constructor
    Guard(void *_locale, void *_address, size_t _size)
            : magic(Magic),
              locale(_locale),
              address(_address),
              size(_size),
              checksum(0)
        {
            // cerr << "sizeof Guard " << sizeof(Guard) << '\n';
            // calculate checksum
            for (unsigned int i = 0; i < sizeof(Guard)/sizeof(size_t) - 1; i++)
                checksum ^= ((int*)this)[i];
        }

    // check that a guard appears to be conformant.
    bool check() {
        // check magic
        if (magic != Magic) {
            cerr << "\n**bad Magic "
                 << magic << "!=" << Magic
                 << " at " << &magic
                 << '\n';

            // call a hook, for debugging
            extern void onError(const char *type = NULL);
            onError();
            
            return false;
        }

        // compare checksums
        size_t ck = 0;
        for (unsigned int i = 0; i < sizeof(Guard)/sizeof(size_t) - 1; i++)
            ck ^= ((int*)this)[i];

        if (ck != checksum)
            cerr << "bad checksum\n";

        return ck == checksum;
    }

    bool operator != (Guard &g) {  // inequality between Guards
        return magic != g.magic
            || locale != g.locale
            || address != g.address
            || size != g.size
            || checksum != g.checksum;
    }

    bool operator == (Guard &g) {	// equality between Guards
        return !(*this != g);
    }

    // damage - damage the magic so the next check will fail
    void damage() {
        magic = ~magic;
    }

    // endGuard - given a pointer to a start Guard, find the endGuard
    Guard *endGuard() {
        return ((Guard*)((char*)address + size)) - 1;
        // Guard abutting end region
    }

    void reason() {
        if (magic == ~Guard::Magic)
            throw runtime_error("Guard: Deallocating Twice");
        else
            throw runtime_error("Guard Underwritten");
    }
};
#endif

#ifdef GUARD_ALLOC
static void *checkGuards(void *allocation, bool damage)
{
    allocation = ((Guard*)allocation)-1;	// skip back over the Guard
    Guard *start = dynamic_cast<Guard*>((Guard*)allocation);

    if (!start) {
        throw runtime_error("Corrupt allocation - no Guard");
    }

    // Check the opening Guard
    if (!start->check()) {
        cerr << "Offending Start Guard address " << start
             << " in " << allocation
             << '\n';
        start->reason();
    }

    // Check the closing Guard
    Guard *end = start->endGuard();
    if (!end->check()) {
        cerr << "Offending End Guard address " << end
             << " in " << allocation
             << " starts at " << start
             << '\n';
        end->reason();
    }

    if (*end != *start)
        throw runtime_error("Guards don't match");

    if (damage) {
        // damage the guards so we know we've deallocated
        DEBLOG(cerr << "Damage Guard: " << allocation << '\n');
        start->damage();
        end->damage();
    }

    return (void*)start;	// return the start of allocation
}
#endif
