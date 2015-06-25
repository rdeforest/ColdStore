// ElfExe - adding symbols to running Elf processes
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

/** ElfExe - direct meddling in elf executable symbol table to add new symbols
 *
 * this class modifies the symbol table of the currently running
 * process' executable by directly modifying the internal data
 * of the ld-linux and libdl.so libraries.
 *
 * This is regrettable, but the alternative is to beg the glibc
 * `maintainers' to fix bugs they hardly even seem to recognise.
 *
 * The result is a glibc2 version-specific meddling object which will
 * completely die as soon as the internal representation used in libdl.so
 * and ld.so changes.  It will need to be ported to new architectures, and
 * may not even be portable to some.
 */
class ElfExe
{
    // Elf direct meddling
    Elf *elf;		// libelf elf ptr for executable
    Elf32_Shdr *dynsym;	// dynamic symbol table section header
    Elf32_Sym *symtab;	// executable's symbol table
    Elf32_Word *bucket;	// hash table buckets
    Elf32_Word nbuckets;// number of buckets
    Elf32_Word *chain;	// hash table chains
    Elf32_Word nchains;	// number of chains

    // free symbol list
    Elf32_Word free_sym;	// free symbol list header
    int nr_free;		// count of free symbols

    static const char *disposable;	// prefix for disposable strings
    static const size_t disp_len;	// size of disposable prefix

    //unsigned long elf_hash (const unsigned char *name);

    /**
     * Make a section writable in the process image
     *
     * @param shdr Segment Header to become writable
     */
    void make_writable(Elf32_Shdr *shdr);

public:
    /** create an ElfExe for the current process
     *
     * Permits new symbols to be defined in the current process
     */
    ElfExe(const char *exename);

    /** restore the process' symbol table to a pristine state
     */
    ~ElfExe();

    /** add a global symbol to the executable
     * @param name symbol's mangled name
     * @param value symbol's value
     * @param size symbol's allocation size
     * @param type symbol's STT_ type
     * @param type symbol's STB_ binding (def: STB_GLOBAL)
     * @param section symbol's program .section (def: SHN_ABS)
     */
    void addSym(const char *name, void *value, int size,
                int type, int bind = STB_GLOBAL, int section = SHN_ABS);

    /** delete a symbol from the executable
     * @param sname symbol's mangled name
     */
    void delSym(char *sname);

    /** vacuum the symbol table
     * 
     * remove all disposable symbols from the symbol table and
     * recover the space occupied.
     * (useful if .so files are loaded with RTLD_NOW)
     */
    void vacuum();

};
