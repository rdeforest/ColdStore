// ElfExe - adding symbols to running Elf processes
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: elfexe.cc,v 1.4 2002/06/09 23:49:18 coldstore Exp $";
//#define DEBUGLOG
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdexcept>
#include <errno.h>
#include <dlfcn.h>
#include <libelf/libelf.h>

#include <qvmm.h>
#include "elfexe.hh"

#ifdef DEBUGLOG
#include "elfdump.hh"
#endif
const char *ElfExe::disposable = "Disposable_Symbol_Used_By_ElfExe_to_Reserve_Space_in_the_Executable_Elf_Dynamic_Symbol_Table_Should_be_loooooooong";
const size_t ElfExe::disp_len = strlen(ElfExe::disposable);
void ElfExe::addSym(const char *name,
                    void *value,
                    int size,
                    int type,
                    int bind,
                    int section)
{
    // grab a free symbol
    if (--nr_free < 0) {
        throw runtime_error("No free symbols.  Statically link executable with ../libs/slop.o");
    }
    if (strlen(name) > disp_len) {
        throw runtime_error("Excessively long symbol.  Can't intern it.");
    }

    Elf32_Word index = free_sym;
    free_sym = symtab[free_sym].st_value;	// unlink the free symbol
    
    // construct the symbol's value
    Elf32_Sym *sym = symtab + index;
    strcpy(elf_strptr(elf, dynsym->sh_link, sym->st_name), name);
    sym->st_size = size;
    sym->st_value = (Elf32_Addr)value;
    sym->st_info = ELF32_ST_INFO(bind, type);
    sym->st_shndx = section;

    // splice the symbol into the hash
    for (Elf32_Word *hp = bucket + (elf_hash((unsigned char*)name) % nbuckets);
         *hp;
         hp = chain + *hp)
    {
        if (!*hp) {
            // found the chain's end
            *hp = index;
            break;
        }
    }
}

void ElfExe::vacuum()
{
    // walk the symtab (by bucket) to find the disposable symbols by st_other flag
    for (size_t bucket_nr = 0;
         bucket_nr < nbuckets;
         bucket_nr++) {
        
        Elf32_Word *sym_ptr = bucket + bucket_nr;
        while (*sym_ptr != 0) {
            if (symtab[*sym_ptr].st_other) {
                // found a disposable symbol

                // add symbol to free list
                symtab[*sym_ptr].st_value = free_sym;
                symtab[*sym_ptr].st_other = 1;
                free_sym = *sym_ptr;
                nr_free++;

                // delete it from chain
                *sym_ptr = chain[*sym_ptr];
            } else {
                // skip to next symbol in chain
                sym_ptr = chain + *sym_ptr;
            }
        }
    }
}

void ElfExe::delSym(char *sname)
{
    Elf32_Word *sym_ptr = bucket + (elf_hash((unsigned char*)sname) % nbuckets);
    while (*sym_ptr != 0) {
        const char *name = elf_strptr(elf, dynsym->sh_link,
                                      symtab[*sym_ptr].st_name);
        if (!name) {
            throw runtime_error(string("elf_strptr: ") + elf_errmsg(elf_errno()));
        }
        if (strcmp(name, sname) == 0) {
            // found a symbol with the disposable prefix
            
            // add symbol to free list
            symtab[*sym_ptr].st_value = free_sym;
            free_sym = *sym_ptr;
            
            // delete it from chain
            *sym_ptr = chain[*sym_ptr];
            break;
        } else {
            // skip to next symbol in chain
            sym_ptr = chain + *sym_ptr;
        }
    }
}

ElfExe::ElfExe(const char *fname)
        : symtab(NULL), bucket(NULL)
{
    int fd;

    fd = open(fname, O_RDONLY);
    if (fd == -1) {
        throw runtime_error(string("opening executable") + strerror(errno));
    }
    if (elf_version(EV_CURRENT) == EV_NONE) {
        throw runtime_error(string("elf_version: ") + elf_errmsg(elf_errno()));
    }

    Elf32_Ehdr *ehdr = elf32_getehdr(elf = elf_begin(fd, ELF_C_READ, NULL));
    if (!ehdr) {
        throw runtime_error(string("elf_begin: ") + elf_errmsg(elf_errno()));
    }

    // Find .hash section
    Elf_Scn *shash = NULL;
    Elf32_Shdr *shdr = NULL;
    while ((shdr = elf32_getshdr(shash = elf_nextscn(elf, shash)))) {
        const char *name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
        if (!name) {
            throw runtime_error((string("elf_strptr: ") + elf_errmsg(elf_errno())));
        }
        if (strcmp(name, ".hash") == 0) {
            if (!shdr->sh_addr) {
                //error
            }

            // Get pointers to symbol hash table
            bucket = (Elf32_Word*)shdr->sh_addr;
            nbuckets = *bucket++;
            nchains = *bucket++;
            chain = bucket + nbuckets;

            make_writable(shdr);
            break;
        }
    }
    
    // Get .dynsym section
    dynsym = elf32_getshdr(elf_getscn(elf, shdr->sh_link));
    if (dynsym && dynsym->sh_addr) {
        //nr_syms = dynsym->sh_size / dynsym->sh_entsize;
        symtab = (Elf32_Sym*)dynsym->sh_addr;

        make_writable(dynsym);
    }

    if (!symtab) {
        throw runtime_error("no symbol table");
    }
    
    // walk the symtab (by bucket) to find the disposable symbols by prefix
    // these symbols must have been placed there at link time.
    free_sym = 0;
    nr_free = 0;
    for (size_t bucket_nr = 0;
         bucket_nr < nbuckets;
         bucket_nr++) {

        Elf32_Word *sym_ptr = bucket + bucket_nr;
        while (*sym_ptr != 0) {
            // currently unused, so we use it to distinguish disposable and other symbols
            assert(symtab[*sym_ptr].st_other == 0);

            const char *name = elf_strptr(elf, dynsym->sh_link,
                                          symtab[*sym_ptr].st_name);
            if (!name) {
                throw runtime_error(string("elf_strptr: ") + elf_errmsg(elf_errno()));
            }

            if (strncmp(name, disposable, disp_len) == 0) {
                // found a symbol with the disposable prefix

                // add symbol to free list
                symtab[*sym_ptr].st_value = free_sym;
                symtab[*sym_ptr].st_other = 1;
                free_sym = *sym_ptr;
                nr_free++;

                // delete it from chain
                *sym_ptr = chain[*sym_ptr];
            } else {
                // skip to next symbol in chain
                sym_ptr = chain + *sym_ptr;
                symtab[*sym_ptr].st_other = 0;
            }
        }
    }
}

ElfExe::~ElfExe()
{
}

void ElfExe::make_writable(Elf32_Shdr *shdr)
{
    Elf32_Addr addr = shdr->sh_addr - shdr->sh_offset;
    Elf32_Word size = shdr->sh_size + shdr->sh_offset;
    int err;

    DEBLOG(cerr <<  addr << ".." <<addr + size - 1 << "writable\n");
    err = mprotect((void*)addr, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    if (err == -1) {
        throw runtime_error(string("mprotect ") + strerror(errno));
    }
}

#if 0
// Standard ELF hash function.  Do not change this function; you will
// cause invalid hash tables to be generated.
unsigned long ElfExe::elf_hash (const unsigned char *name)
{
    unsigned long h = 0;
    unsigned long g;
    int ch;

    while ((ch = *name++) != '\0') {
        h = (h << 4) + ch;
        if ((g = (h & 0xf0000000)) != 0) {
            h ^= g >> 24;
            h &= ~g;
	}
    }
    return h;
}
#endif

