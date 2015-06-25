// Elf - interning of Elf DSOs
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef ELF_HH
#define ELF_HH

#include <stdlib.h>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <elf.h>

#include <qvmm.h>
#include <Q.hh>
#include <string>

#include <config.h>

// Have to take .so files,
// dig out the symbols needed,
// find their length,
// allocate that space in the store,
// spit out a new companion .s file defining those symbols,
// compile it up,
// and dlopen() the resultant .so before the target .so.

class Elf;	// forward reference
//struct Elf32_Sym; // external reference to libelf's Symbols

/** Interned Symbols from @ref Elf objects
 * @author Colin McCormack colin@field.medicine.adelaide.edu.au
 *
 * Each symbol interned from an @ref Elf object has a corresponding Csym
 * which is mapped into the store and contains storage for the symbol.
 *
 * In the case of a DATA or VTBL type Csym, the storage contains
 * the content of the symbol from the original Elf.  In the case of a
 * FUNCTION type, the storage contains an absolute JMP into the Elf
 * object's function.
 */
class Csym
    : public Q<Csym>,	// Q of symbols in a given Elf
    Memory
{
public:
    /** symbol type
     * FUNCTION	a function symbol
     * DATA	a data symbol
     * VTBL	a vtable
     * UNKNOWN	not found
     */
    enum Type {
        FUNCTION,
        DATA,
        VTBL,
        UNKNOWN
    };		// symbol type (F,V or O)

    /** Elf this Csym is in
     */
    const Elf *in;

protected:
    friend class Elf;	// only Elf can create Csyms

    /** symbol type (F,V or O)
     */
    Type _type;

    /** allocated length of this Csym
     */
    unsigned _len;

    /** true if this Csym's been loaded
     * that is, its Elf has been loaded,
     * and the Csym was found and has been relocated.
     */
    bool loaded;

    /** mangled name of Csym
     */
    string name;

    /** space for metadata pointer - initialized by DopeV
     */
    void *metadata;

    /** content of Csym (longword aligned)
     * ***** HAS TO BE LAST DATA MEMBER IN CLASS! *****
     */
    char content[0];
    static const int JMP = 0xE9;	// absolute JMP opcode
    static const int JMPSIZE = 3;	// number of longwords

    /** construct a Csym - can only be called by Elf
     * @param elf the @ref Elf with which this is associated
     * @param mangled the symbol's mangled name
     * @param the Type of the symbol
     * @param len the length of allocation for the symbol
     */
    Csym(Elf *elf, const char *mangled, enum Type type, int len);

    /** can never destroy a Csym, some Builtin may reference it.
     */
    ~Csym();

    /** reset this Csym to an unloaded state
     */
    void reset();

    /** copy-relocate contents from original to this Csym symbol
     * @param so dlopen()-returned value for the source Elf
     */
    void reloc(void *so);

public:
    /** Csym's mangled name
     * @returns mangled name of Csym
     */
    const char *symname() const {
        return name.c_str();
    }

    /** Csym's demangled name
     * @returns demangled name of Csym
     */
    const char *demangle() const;
    /** Csym's fully expanded demangling
     * @returns demanged name of Csym, and its parameters.
     */
    const char *demangle_full() const;

    /** address of the Csym's coldstored content
     * @returns address of the symbol's content
     * @exception undefined
     */
    void *address() const {
        if (_type != UNKNOWN) 
            return (void *)content;
        else
            throw runtime_error("Csym is undefined");
    }

    /** virtual type implemented by this vtbl
     * @returns type implemented by this vtbl
     * @exception not-loaded not-VTBL
     */
    void *vtype() const {
        if (_type == VTBL && loaded) {
            //cerr << "vtype " << (void*)content << '\n';
            return (void *)content;
        } else
            throw runtime_error("Csym is not a loaded VTBL");
    }

    /** call the function represented by this symbol
     * @param narg length of rest of args
     * @param ... args, IN REVERSE ORDER
     * @returns whatever's returned by the call
     * @exception not-loaded not-FUNCTION
     */
    static void *callit(Csym *sym, int nargs, ...);
    static void *caller(void *(*what)(...), void **args, int count);
#if 0
    /** call the function represented by this symbol
     * @param narg length of rest of args
     * @param ... args, IN REVERSE ORDER
     * @returns whatever's returned by the call
     * @exception not-loaded not-FUNCTION
     */
    void *call(int nargs, ...) const;

    /** call the function represented by this symbol
     * __builtin_apply is hopelessly bugged in C++
     * @returns whatever's returned by the call
     * @exception not-loaded not-FUNCTION
     */
    void *vcall(...) const;
#endif

    /** return the function address pointed to by this Csym
     * @returns address as function
     * @exception not-loaded not-FUNCTION
     */
    void *functional() const;

    /** allocation length
     * @returns symbol's allocation length
     * @exception undefined
     */
    unsigned len() const {
        if (_type != UNKNOWN) 
            return _len;
        else
            throw runtime_error("Csym is undefined");
    }

    /** Csym type
     * @returns type of the Csym
     */
    enum Type type() const {
        return _type;
    }

    /** predicate: is Csym loaded?
     * @returns true iff Csym has been loaded
     */
    bool isLoaded() const;
};

extern "C" {
    struct link_map;
}
class ElfExe;

/** Elf class - permits the dynamic interning of Elf DSOs
 * @author Colin McCormack colin@field.medicine.adelaide.edu.au
 *
 * Elf instances have Csyms Q'd off them, one for each interned symbol.
 *
 * Symbols may be requested to be interned, by using addSym to associate 
 * the Csym with the Elf, or may be added automatically (if they represent
 * vtables, for example.)
 */
class Elf
    : public Q<Elf>,	// Q of all the Elf objects
      public Memory
{
public:
    /** persistent header of Elf Q */
    static Qh<Elf> all;

    /** modifies the Elf symbol snarfing process
     *
     * COLDSTORE	snarf ColdStore constructors
     *			in addition to DEFAULT_P
     *
     * STL		snarf all global functional symbols
     *			in addition to DEFAULT_P
     *
     * DEFAULT_P	snarf only requested symbols
     *			and vtables
     */
    enum Policy {
        COLDSTORE,
        STL,
        DEFAULT_P
    };

    /** Q of all the symbols defined
     */
    mutable Qh<Csym> symbols;

protected:
    /** .so name implementing the Elf
     */
    string elf;

    /** size set aside trampolining functions
     */
    static const int functSize = 20;

    /** Elf target for BFD
     */
    static const char *const elf_target = "elf32-i386";

    /** Elf architecture for BFD
     */
    static const char *const elf_arch = "i386";

    /** run through all Elfs and reset them to a clean state
     * persistent references to .so handles must be NULLed
     */
    void reset();

private:    
    /** create a Csym given an Elf32 symbol 
     * Called from snarf
     */
    Csym *snarfSym(char *symname, Elf32_Sym *sym);

    /** can never destroy a Elf, some Builtin may reference it.
     */
    ~Elf();

    static ElfExe *exe;
    friend class ElfExe;// dummy to stop errors about private destructor

    /** snarf the interesting symbols from the .so file
     * the output of this phase is a Q of Csym objects
     * @param personality @ref Elf::Policy determining which symbols to intern
     */
    void snarf(Policy personality = DEFAULT_P);

    /** create shadow symbol definitions for Elf's Csyms
     */
    int shadow();

    /** load the shadowed Elf .so file
     */
    void load();

    /** resolve Csyms in shadowed Elf .so files
     */
    void resolve();

public:
    /** dlopen handle for main Elf
     */
    link_map *_so;

    /** create an Elf
     * @param name filename of elf DSO
     */
    Elf(const char *name);


    /** intern an Elf and all its Csyms
     * @param policy @ref Elf::Policy for interning
     */
    void intern(Policy policy);

    /** iterate over all Elf,
     * calling a function on each instance
     * @param fn function to call on each @ref Elf instance
     */
    static void forallElf(void (fn)(Elf*));

    /** utility method to search for a symbol in all Elfs
     * @param sname name of the symbol for which to search
     * @returns the value of the symbol (if NULL, check dlerror())
     */
    static void *sym_search(const char *sname);

    /** iterate over all Csym in this Elf,
     * calling a function on each instance
     * @param fn function to call on each (@ref Elf, @ref Csym) pair
     */
    void forallSym(void (fn)(Elf*,Csym*));

    /** Q of all Csyms interned by this Elf
     * @returns the @ref Q of all Csyms for this
     */
    const Csym *symQ() const {
        return symbols;
    }

    /** called once on startup to reset Elfs
     * @param _exe an @ref ElfExe instance to enable new symbol definition
     */
    static void initialize(ElfExe *_exe);

    /** close this Elf
     */
    void close();

    /** find an Elf by name
     * @param elf file name of elf
     * @returns the @ref Elf with this name, or NULL
     */
    static Elf *findElf(const char *elf);

    /** find a Csym by name
     * @param symname symbol name
     * @returns the @ref Csym with this name, or NULL
     */
    Csym *findSym(const char *symname) const;

    /** add an undefined Csym if it doesn't already exist
     * used to request the interning of symbols
     * @param sym symbol name
     * @returns a @ref Csym with this name
     */
    bool addSym(const char *sym);

    /** predicate: has this Elf been loaded?
     * @returns true iff this Elf has been loaded
     */
    bool isLoaded() const {
        return _so != NULL;
    }

    /** create a Csym interned by this Elf
     * @param mangled symbol name
     * @param type symbol type
     * @param size size of symbol's allocation
     * @returns a @ref Csym with this name
     */
    Csym *csym(const char *mangled, Csym::Type type, int size=0);

    /** Elf's .so name
     * @returns elf's DSO file name
     */
    const char *name() const {
        return elf.c_str();
    }

    /** find a simple C symbol in Elf
     * @param name C symbol name
     * @returns symbol's value
     */
    void *getSym(const char *name) const;
};

/** load an Elf
 */
void *loadElf(char *name,
              Elf::Policy type = Elf::DEFAULT_P);

/** elf autoloading and interning
 * @author Colin McCormack colin@field.medicine.adelaide.edu.au
 *
 * The InitElf instance, when created:
 * @li looks for a NULL terminated variable named: AutoLoad autoload_elfs[]
 * @li interns each named library with the given policy
 * @li calls the named function with a pointer to its @ref Elf
 */
struct AutoLoad {
    /** file name of the elf DSO to autoload
     */
    char *name;

    /** what kind of interning @ref Elf::Policy to apply?
     */
    Elf::Policy type;

    /** mangled name of a function to call after the library's been
     * loaded, interned and initialised.
     *
     * Underlying function should be of type void fn(Elf *)
     */
    char *initFn;
};

/** class to initialize Elf
 * @author Colin McCormack colin@field.medicine.adelaide.edu.au
 *
 * InitElf is instantiated once and only once, by libintern.so.
 *
 * InitElf arranges for the
 * initialization of the libintern.so module and the autoloading of modules
 * @see AutoLoad
 */

class InitElf
{
private:
    ElfExe *exe;
    link_map *main_so;
    link_map *elf;

    // loadElf()
    void *load(const char *name, int flags = RTLD_LAZY);
    void *getSym(void *so, char *name, bool chuck = true);

    /** copy symbol contents from .so into the store.
     */
    char *copy_reloc(void *from, char *what, size_t len);

public:
    /** initialize the Elf interning library 
     * @param exename the name of the executable
     * @param elflib the name of the libintern.so lib
     * @returns THE InitElf instance
     */
    InitElf(const char *exename, const char *elflib="libintern.so");

    /** shut down the Elf interning library
     */
    ~InitElf();
};

extern "C" {
    void *__vcall(void (*fn)(), ...);
}

// Local Variables:
// mode:C++
// End:
#endif // ELF_HH
