// Builtin.hh - ColdStore interface to C++ functions
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef BUILTIN_HH
#define BUILTIN_HH

#include "String.hh"

class Elf;
class Csym;

// Builtin
class Builtin
    : public String
{

public:
  //    static const char * const COLDSTORE = "../libs/libcold.so";
    static const char * const COLDSTORE = "libcold.so";
    const Elf *elf;	// which Elf contains this symbol?
    const Csym *sym;	// what's the underlying symbol container?

protected:
    // Builtin mutators

    /** load the Builtin's underlying Elf
     */
    bool load() const;

public:
    // Builtin accessors

    /** predicate: is the builtin loaded?
     */
    bool isLoaded() const;

    /** Builtin's name
     * @return Builtin name
     */
    const char *name() const;

    /** Builtin's name
     * @return Builtin name
     */
    void *address() const;

    /** Builtin's file name
     * @return builtin's file name
     */
    const char *fname() const;

    /** Builtin's demangled symbol name
     * @return builtin's demangled name with parameter text
     */

    const char *rname() const;
    const char *rname_full() const;

public:
    /** create a Builtin implementing a type from a .so file
     * @param _sym a Csym structure describing the symbol
     */
    Builtin(Csym *_sym);

    /** create a Builtin implementing a type from a .so file
     * @param _sym the symbol's name
     * @param elf_name mangled name of the symbol
     */
    Builtin(const char *_sym, const char *elf_name = COLDSTORE);

    /** construct a Builtin from a coldstore description
     * @param arg Builtin description
     */    
    Builtin(const Slot &arg);

    /** destroy Builtin */
    virtual ~Builtin();

    ///////////////////////////////
    // Coldmud Interface

    /** copy constructor @ref Data::clone
     *
     * Builtins are inherently mutable
     */
    virtual Data *clone(void *store = (void*)0) const;

    /** create a mutable copy @ref Data::mutate
     *
     * Builtins are inherently mutable
     */
    virtual Data *mutate(void *where = (void*)0) const;

    /** dump the Builtin to a stream
     * @param ostream stream onto which to dump the object
     */
    ostream &dump(ostream& out) const;

    /** the Builtin's truth value
     *
     * We consider a Builtin to be true when it's loaded.
     * @return true iff the Builtin has been loaded
     */
    bool truth() const;

    /** constructor arguments sufficient to recreate object
     * @return a primitive type (pickled) representation of this object
     */
    Slot toconstruct() const;	// constructor args to recreate object

    /** Builtin order
     *
     * Builtins are ordered by .so file name, symbol name
     * @param arg object to be ordered relative to this
     * @return 1,0,-1 depending on object order
     */
    virtual int order(const Slot &arg) const;	// 1,0,-1 depending on order

    /** present Builtin as a sequence
     *
     * A Builtin presents its characteristics as three elements of a sequence.
     * @li 0 symbol name
     * @li 1 .so file name
     * @li 2 `is loaded' flag
     * @li 3 is symbol allocation length
     * @li 4 is symbol type
     */
    virtual Slot slice(const Slot &from);

    /** call Builtin new
     * 
     * call the constructor function represented by a function type Builtin
     * @param number of arguments
     * @param ... the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot new_call(int nargs, ...);	// call object

    /** call Builtin
     * 
     * call the function represented by a function type Builtin
     * @param args the argument to the function call
     * @return whatever the underlying function returns
     */
    virtual Slot call(Slot &args);	// call object

    /** call method on Builtin
     * 
     * call the method represented by a function type Builtin
     * @param obj object to invoke this method on
     * @param args argument to the method call
     * @return whatever the underlying method returns
     */
    Slot mcall(Slot that, Slot &args);	// call method on object

    /** call method on Builtin with void return
     * 
     * call the method represented by a function type Builtin
     * @param obj object to invoke this method on
     * @param args argument to the method call
     */
    void mvcall(Slot that, Slot &args);	// call method on object

    /** the virtual table pointer (if any) implemented by this builtin
     *
     * throws a ~builtin exception if the Builtin is not of VTBL type
     * @return virtual table pointer
     * @exception ~builtin
     */
    const void *vtbl() const;
};

#endif

