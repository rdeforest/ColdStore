// Elf - interning of Elf DSOs
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: elf.cc,v 1.18 2001/10/27 06:33:48 coldstore Exp $";

// This file contains modified code (where marked <LIBELF>)
// from ftp://sunsite.unc.edu/pub/Linux/libs/libelf-0.7.0.tar.gz
// Clearly, my copyright does not apply to that code, nor does
// the author of that code (Michael "Tired" Riepe 
// <michael@stud.uni-hannover.de>) bear responsibility for my mods.

//#define DEBUGLOG

#include <unistd.h>
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <exception>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <string>
//#include <elf.h>                // AKPM
//#include <sys/elf.h>            // AKPM
#include <libelf/libelf.h>
#include "demangle.h"
#include <link.h>
#include <qvmm.h>
#include "elf.hh"
#include "elfexe.hh"

#include <Q.th>
template class Q<Elf>;
template class Qh<Elf>;
template class Q<Csym>;
template class Qh<Csym>;
ElfExe *Elf::exe = NULL;
//Q<Elf> Elf::all;	// header of all Elf Q
extern void bkpt();

Elf::Elf(const char *name)
        : Q<Elf>(all)
{
    elf= name;
    DEBLOG(cerr << "new Elf(" << elf << "): "
           << this << " vptr=" << this->_vptr << '\n');
}

Elf::~Elf()
{
    throw runtime_error("Can't destroy Elfs");
}


void Elf::reset()
{
    _so = NULL;	// NULL persistent .so handle refs
    for (Qi<Csym> sp(symbols); sp.More(); sp.Next()) {
        ((Csym*)sp)->reset();
    }
}


void Elf::initialize(ElfExe *_exe) {
    exe = _exe;
    if (all) {
        // reset all the Elf instances
        for (Qi<Elf> ep(all); ep.More(); ep.Next()) {
            return ((Elf*)ep)->reset();
        }
    }
}

Csym *Elf::findSym(const char *sym) const
{
    for (Qi<Csym> sp(symbols); sp.More(); sp.Next()) {
        if (((Csym*)sp)->name==sym) {
            DEBLOG(cerr << "findSym(" << sym << "): "
                   << (Csym*)sp << '\n');
            return (Csym*)sp;
        }
    }
    //cerr << "findSym(" << sym << "): NULL\n";
    return NULL;
}

bool Elf::addSym(const char *sym)
{
    if (findSym(sym)) {
        DEBLOG(cerr << "addSym(" << sym << "): false\n");
        return false;
    }

    csym(sym, Csym::UNKNOWN, 0);	// add a placeholder
    return true;
}

Csym *Elf::csym(const char *mangled, Csym::Type type, int size)
{
    DEBLOG(cerr << "csym(" << mangled << ", " << size << ")\n");
    if (!size) {
        size = Csym::JMPSIZE * sizeof(int);
    }
    return new (size) Csym(this, mangled, type, size);
}

Elf *Elf::findElf(const char *elf)
{
    for (Qi<Elf> ep(all); ep.More(); ep.Next()) {
        if (((Elf*)ep)->elf== elf) {
            DEBLOG(cerr << "findElf(" << elf << "): "
                   << (Elf*)ep << '\n');
            return (Elf*)ep;
        }
    }
    return NULL;
}

void Elf::forallElf(void (fn)(Elf*))
{
    for (Qi<Elf> ep(all); ep.More(); ep.Next()) {
        (*fn)((Elf*)ep);
    }
}

void Elf::forallSym(void (fn)(Elf*,Csym*))
{
    for (Qi<Csym> sp(symbols); sp.More(); sp.Next()) {
        (*fn)(this, (Csym*)sp);
    }
}

void *Elf::sym_search(const char *sname)
{
    for (Qi<Elf> ep(all); ep.More(); ep.Next()) {
        void *result = dlsym(ep.Current()->_so, sname);
        if (result)
            return result;
    }
    return NULL;
}

// create new Csym
Csym *Elf::snarfSym(char *symname, Elf32_Sym *sym)
{
    Csym *fsym = NULL;
    switch (ELF32_ST_TYPE(sym->st_info)) {
        case STT_FUNC:
            fsym = csym(symname, Csym::FUNCTION);
            break;
        case STT_OBJECT:
            fsym = csym(symname, Csym::DATA,
                        sym->st_size);
            break;
        default:
            break;
    }
    return fsym;
}

void library_paths( char *paths ) {
  paths[0] = 0;
  char *path1 = getenv("LD_LIBRARY_PATH");
  char *path2 = getenv("LD_CONFIG_PATH");
  char *path3 = getenv("LD_PATH");
  
  if (path1 && *path1) {
    strcat( paths, path1 );
    strcat( paths, ":" );

  }
  if (path2 && *path2) {
    strcat( paths, path2 );
    strcat( paths, ":" );
  }
  if (path3 && *path3) {
    strcat( paths, path3 );
  }
  strcat( paths, "/lib:/usr/lib" );

  DEBLOG(cerr << "searching paths: " << paths << endl);
}

/** utility routine to find shared library in available paths.
 *  adapted (and heavily modified) from uClibc's ld.so code.
 */
int open_so( char *full_libname)
{
  char *pnt, *pnt1, *pnt2;
  int fd = -1;
  char mylibname[2050];
  char *libname;

  /* quick hack to ensure mylibname buffer doesn't overflow.  don't 
     allow full_libname or any directory to be longer than 1024. */
  if (strlen(full_libname) > 1024)
    return -1;

  pnt = libname = full_libname;
  while (*pnt) {
    if (*pnt == '/')
      libname = pnt + 1;
    pnt++;
  }

  /* If the filename has any '/', try it straight and leave it at that.
   */
  if (libname != full_libname)
    return open(full_libname, O_RDONLY);

  /* Get a list of paths
   */
  char paths[1024];
  library_paths( paths );
  pnt1 = paths;

  /* Search them
   */
  if (pnt1 && *pnt1) {
    while (*pnt1) {
      pnt2 = mylibname;
      while (*pnt1 && *pnt1 != ':' && *pnt1 != ';') {
	if (pnt2 - mylibname < 1024)
	  *pnt2++ = *pnt1++;
	else
	  pnt1++;
      }
      if (pnt2 - mylibname >= 1024)
	break;
      if (pnt2[-1] != '/')
	*pnt2++ = '/';
      pnt = libname;
      while (*pnt)
	*pnt2++ = *pnt++;
      *pnt2++ = 0;
      fd = open( mylibname, O_RDONLY );
      if (fd != -1) {
	cerr << "(from " << mylibname << ")" << endl;
	return fd;
      }
      if (*pnt1 == ':' || *pnt1 == ';')
	pnt1++;
    }
  }

  return fd;
}

// Code from Elk which snarfs symbol tables.
void Elf::snarf(Policy personality)
{
    Elf		*elf_ptr;
    Elf_Scn	*elf_scn_ptr = NULL, *symtab_scn_ptr = NULL;
    Elf_Data	*elf_data_ptr = NULL;
    Elf32_Ehdr	*elf_ehdr_ptr = NULL;
    Elf32_Shdr	*elf_shdr_ptr = NULL,
        *symtab_ptr = NULL;
    size_t	elf_str_index = 0;
    size_t	shstrndx;
    char	*section_name;

    DEBLOG(cerr << "Elf::snarf " << elf << endl);

    // open the .so file
    char name[ strlen(elf.c_str()) ];
    strcpy( name, elf.c_str() );
    int f = open_so( name );
    //    int	f = open(elf.c_str(), O_RDONLY);

    if (f == -1)
        throw runtime_error("Can't open Elf .so file");

    if (elf_version (EV_CURRENT) == EV_NONE) {
        ::close(f);
        throw runtime_error("Elf .so file Elf version out of date");
    }
    if ((elf_ptr = elf_begin (f, ELF_C_READ, (Elf *)NULL)) == NULL) {
        ::close(f);
        throw runtime_error("can't elf_begin() .so file");
    }

    // get the elf header, so we'll know where to look for the section 
    // names. <LIBELF>
    if ((elf_ehdr_ptr = elf32_getehdr (elf_ptr)) == NULL) {
        elf_end(elf_ptr);
        ::close(f);
        throw runtime_error("no elf header in .so file");
    }
    shstrndx = elf_ehdr_ptr->e_shstrndx;

    // look for the symbol and string tables
    while ((elf_scn_ptr = elf_nextscn (elf_ptr, elf_scn_ptr))) {
	if ((elf_shdr_ptr = elf32_getshdr (elf_scn_ptr)) == NULL) {
            ::close(f);
            throw runtime_error("can't get section header in .so file");
        }
	if (elf_shdr_ptr->sh_type == SHT_STRTAB) {
            // save the index to the string table for later use by 
            // elf_strptr().
	    section_name = elf_strptr (elf_ptr, shstrndx,
                                       (size_t)elf_shdr_ptr->sh_name);
	    if (strcmp (section_name, ".strtab") == 0 ||
                strcmp (section_name, ".dynstr") == 0) {
		elf_str_index = elf_ndxscn (elf_scn_ptr);
	    }
	}
	else if (elf_shdr_ptr->sh_type == SHT_SYMTAB ||
		 elf_shdr_ptr->sh_type == SHT_DYNSYM) {
	    symtab_ptr = elf_shdr_ptr;
	    symtab_scn_ptr = elf_scn_ptr;
	}
    }

    // check the .so
    if (!symtab_ptr) {
        elf_end(elf_ptr);
        ::close(f);
        throw runtime_error("no symbol table in .so file");
    }
    if (!elf_str_index) {
        elf_end(elf_ptr);
        ::close(f);
        throw runtime_error("no string table in .so file");
    }

    // we've located the symbol table -- go through it and save the names 
    // of the interesting symbols.
    while ((elf_data_ptr = elf_getdata(symtab_scn_ptr, elf_data_ptr))) {
	char *name = NULL;
	unsigned int symbol_count;
	Elf32_Sym *symbol_ptr = (Elf32_Sym*)elf_data_ptr->d_buf;
	Elf32_Sym *current_symbol;
        Csym *fsym;

	for (symbol_count = 1;
	     symbol_count < symtab_ptr->sh_size / symtab_ptr->sh_entsize;
	     symbol_count++) {
	    current_symbol = symbol_ptr + symbol_count;

	    if (ELF32_ST_BIND(current_symbol->st_info) != STB_GLOBAL
                && ELF32_ST_BIND(current_symbol->st_info) != STB_WEAK) {
                // discard non-globals
		continue;
	    }

            if (!current_symbol->st_shndx) {
                // discard symbols defined relative to the UNDEFINED section
                continue;
            }

            name = elf_strptr(elf_ptr, elf_str_index,
                              (size_t)current_symbol->st_name);
            //</LIBELF>
	    if (name == NULL) {
                elf_end(elf_ptr);
		::close(f);
		throw runtime_error(elf_errmsg(elf_errno()));
	    }

            // see if this symbol is already interned
            fsym = findSym(name);
            if (fsym) {
                // existing interned symbol
                switch (fsym->_type) {
                    case Csym::UNKNOWN: {
                        // We've located a symbol which was previously unknown
                        DEBLOG(cerr << "found UNKNOWN symol "
                               << name << " as " << fsym << '\n');
                        // previously unknown symbol
                        delete fsym;	// destroy placeholder

                        // create new Csym
                        if (!snarfSym(name, current_symbol)) {
                            elf_end(elf_ptr);
                            ::close(f);
                            throw runtime_error("Requested bad symbol type");
                        }
                        break;
                    }
                    case Csym::FUNCTION:
                        DEBLOG(cerr << "found requested FUNCTION symol "
                               << name << " as " << fsym << '\n');
                        // nothing to be done
                        break;

                    case Csym::DATA:
                    case Csym::VTBL:
                        DEBLOG(cerr << "found requested DATA/VTBL symol "
                               << name << " as " << fsym << '\n');
                        if (fsym->len() < current_symbol->st_size) {
                            elf_end(elf_ptr);
                            ::close(f);
                            throw runtime_error("symbol changed size catastrophically");
                        }
                        break;
                    default:
                        elf_end(elf_ptr);
                        ::close(f);
                        throw runtime_error("corrupt Csym found");
                }
            } else if (!strncmp("__vt_", name, 5)) {
                // vtbls are interned regardless
                DEBLOG(cerr << "found new VTBL symbol "
                       << name
                       << " st_info " << (void*)current_symbol->st_info
                       << " st_shndx " << current_symbol->st_shndx
                       << '\n');
                csym(name, Csym::VTBL, current_symbol->st_size);
            } else if (personality == COLDSTORE) {
                // coldstore constructors need to be interned too
                // A coldstore constructor is a symbol of the form
                // ending in 'RC4Slot' and beginning __nOBJNAME
                char *ending = "RC4Slot";
                if (!strncmp("__", name, 2)
                    && ((name[2] >= '0') && (name[2] <= '9'))
                    && !strcmp(ending, name + strlen(name) - strlen(ending))
                    && (ELF32_ST_TYPE(current_symbol->st_info) == STT_FUNC)) {
                    DEBLOG(cerr << "found new Coldstore Constructor "
                           << name
                           << " st_info " << (void*)current_symbol->st_info
                           << " st_shndx " << current_symbol->st_shndx
                           << " st_size " << current_symbol->st_size
                           << '\n');
                    csym(name, Csym::FUNCTION);
                }
            } else if (personality == STL) {
                // STL all globals need to be interned
                if ((ELF32_ST_TYPE(current_symbol->st_info) == STT_FUNC) 
                    && current_symbol->st_shndx
                    && current_symbol->st_size) {
                    DEBLOG(cerr << "found new Global function "
                           << name
                           << " st_info " << (void*)current_symbol->st_info
                           << " st_shndx " << current_symbol->st_shndx
                           << '\n');
                    if (!snarfSym(name, current_symbol)) {
                        elf_end(elf_ptr);
                        ::close(f);
                        throw runtime_error("Requested bad symbol type");
                    }
                }
            } else {
                // unwanted symbol - do nothing
            }
	}
    }
    elf_end(elf_ptr);
    ::close(f);
}

int Elf::shadow()
{
    int symcnt = 0;	// count of symbols defined

    // count up the symbols and check that they're all defined
    for (Qi<Csym> sp(symbols); sp.More(); sp.Next()) {
        switch (((Csym*)sp)->_type) {
            case Csym::FUNCTION:
            case Csym::DATA:
            case Csym::VTBL:
                symcnt++;
                continue;
            default:
                // let undefined symbols pass silently
                throw runtime_error("undefined symbol in Elf");
        }
    }

    DEBLOG(cerr << "Elf::shadow " << elf
           << ' ' << symcnt << " symbols \n");

    // Elf's Csyms into a bfd symbol table
    symcnt = 0;

    bkpt();
    for (Qi<Csym> sp(symbols); sp.More(); sp.Next()) {
        Csym *csym = (Csym*)sp;
        if (csym->type() != Csym::UNKNOWN) {
            DEBLOG(cerr << "Elf::shadow "
                   << elf << " making sym "
                   << csym->name << " @ " << csym->address() << '\n');
            exe->addSym(csym->symname(), csym->address(), csym->len(),
                        (csym->type() == Csym::FUNCTION)
                        ?STT_FUNC
                        :STT_OBJECT);
            symcnt++;
        }
    }

    return symcnt;
}

void Elf::load()
{
    DEBLOG(cerr << "Elf::load \n");

    // load the .so file
    //_so = (link_map*)dlopen(elf, RTLD_GLOBAL | RTLD_LAZY);
    _so = (link_map*)dlopen( elf.c_str(), RTLD_GLOBAL | RTLD_NOW );
    if (!_so) {
        throw runtime_error( dlerror() );
    }
}

void Elf::resolve()
{
    for (Qi<Csym> sp(symbols); sp.More(); sp.Next()) {
        Csym *csym = (Csym*)sp;
        if (csym->_type != Csym::UNKNOWN) {
            // don't reloc a symbol twice
            if (!csym->isLoaded()) {
                void *sym = dlsym(_so, csym->name.c_str());
                const char *error = dlerror();
                if (error) {
                    throw runtime_error(error);
                }
                
                csym->reloc(sym);
            }
        }
    }
}

void Elf::intern(Policy policy)
{
    snarf(policy);
    shadow();
    load();
    resolve();
}

Csym::Csym(Elf *elf, const char *mangled, Csym::Type type, int len)
        : Q<Csym>(elf->symbols),
    in(elf),
    _type(type),
    _len(len),
    loaded(false)
{
    name= mangled;
    DEBLOG(cerr << this
           << " new Csym " << symname()
           << " in: " << in
           << " type: " << type
           << " len " << len
           << '\n');
}

Csym::~Csym()
{
    throw runtime_error("Can't destroy Csyms");
}

const char *Csym::demangle() const {
    return (const char*)cplus_demangle(name.c_str(),DMGL_ANSI);
}

const char *Csym::demangle_full() const {
    return (const char*)cplus_demangle(name.c_str(),DMGL_ANSI|DMGL_PARAMS|DMGL_AUTO);
}

#if 0
// bug in __builtin_apply and co.  Fuckit.
void *Csym::vcall(...)
{
    if (type() == Csym::FUNCTION && isLoaded()) {
        void *result = __builtin_apply((void (*)())address(),
                                       __builtin_apply_args(),
                                       32);
        __builtin_return(result);
    } else {
        throw runtime_error("Csym is not a loaded FUNCTION");
    }
}

void *Csym::call(int nargs, ...) const
{
    if (_type == FUNCTION && loaded) {
        // GCC 2.96 has a parser bug, and doesn't like pointer types in va_args
        typedef int tmpdataptr;
        va_list args;
        tmpdataptr datum;
        void *(*fn)(void *) = (void *(*)(void*))address();
        
        va_start(args, nargs);
        for (int size = nargs; size > 0; size--) {
            datum = va_arg(args, tmpdataptr);
            __asm__ __volatile__ ("pushl (%0)"
                                  : : "r" (&datum) : "cc", "memory");
        }
        va_end(args);
        __asm__ __volatile__ ("call %0"
                              : : "r" (fn) : "cc", "memory");
    } else
        throw runtime_error("Csym is not a loaded FUNCTION");
}
#endif

void *Csym::caller(void *(*what)(...), void **args, int count)
{
  void **to[count];
  cerr << "to address " << &to << " size " << sizeof(to) << '\n';
  memcpy(to, args, count * sizeof(void*));
  cerr << "to value[0] " << (void*)(to[0]) << '\n';
  cerr << "to value[1] " << (void*)(to[1]) << '\n';
  __asm__ __volatile__ ("addl $20, %esp");
  return (what)();
}

void *Csym::callit(Csym *sym, int nargs, ...)
{
    if (sym->_type == FUNCTION && sym->loaded) {
        // GCC 2.96 has a parser bug, and doesn't like pointer types in va_args
        typedef void *tmpdataptr;
        va_list args;
        tmpdataptr datum;
        void *(*fn)(void *) = (void *(*)(void*))(sym->address());
        DEBLOG(cerr << "Csym::callit " << sym << " " << nargs << " ");

        va_start(args, nargs);
        for (int size = nargs; size > 0; size--) {
            datum = va_arg(args, tmpdataptr);
            __asm__ __volatile__ ("pushl (%0)"
                                  : : "r" (&datum) : "cc", "memory");
#if 0
	    cerr << "arg #" << nargs - size
		 << " == " << (void*)datum
		 << '\n';
#endif
        }
        va_end(args);
        __asm__ __volatile__ ("call %0"
                              : : "r" (fn) : "cc", "memory");
        __asm__ __volatile__ ("movl %%eax,%0"
                              : "=r" (datum) :
                              : "eax", "cc", "memory");
        return datum;
    } else
        throw runtime_error("Csym is not a loaded FUNCTION");
}

#if 0
void *Csym::call(void *obj, void *arg) const
{
    if (_type == FUNCTION && loaded) {
        void *(*fn)(void*, void*) = (void *(*)(void*, void*))address();
        return (*fn)(obj, arg);
    } else
        throw runtime_error("Csym is not a loaded FUNCTION");
}
#endif

void *Csym::functional() const
{
    if (_type == FUNCTION && loaded) {
        return address();
    } else
        throw runtime_error("Csym is not a loaded FUNCTION");
}

void Csym::reset()
{
    loaded = false;
}

bool Csym::isLoaded() const
{
    return loaded;
}

void Csym::reloc(void *so)
{
    switch (_type) {
        case FUNCTION:
            // form up a JMP instruction
            DEBLOG(cerr << "Csym::reloc FUNCTION " << name
                   << ' ' << so << '\n');
            content[0] = JMP;
            //*((int*)(content + 1)) = (int)so;
            *((int*)(content + 1)) = ((int)so) - (int)content - 5; // feh, Intel.
            DEBLOG(cerr << "set " << (void*)content
                   << " to " << JMP << ' ' << so << '\n');
            break;

        case DATA:
            // copy the content
            DEBLOG(cerr << "Csym::reloc DATA "
                   << name << ' ' << so << ' ' << _len
                   << " from " << so << " to " << (void*)content << '\n');
            memcpy((void *)content, so, _len);
            break;

        case VTBL:
            // copy the content
            DEBLOG(cerr << "Csym::reloc VTBL "
                   << name << ' ' << so << ' ' << _len
                   << " from " << so << " to " << (void*)content << '\n');
            memcpy((void *)content, so, _len);
            break;

        default:
            throw runtime_error("Unknown type in Csym::reloc");
    }
    loaded = true;	// flag Csym as loaded
}

void *Elf::getSym(const char *name) const
{
    void *sym = dlsym(_so, name);
    const char *error = dlerror();
    if (error)
        throw runtime_error(error);
    return sym;
}

void Elf::close()
{
    const char *error;

    if (_so) {
        dlclose(_so);
        if ((error = dlerror()))
            throw runtime_error(error);
    }
}

/** default AutoLoad (ie: none)
 */
AutoLoad autoload_elfs[] = {
    {NULL}
};

extern void bkpt();

/** function to unload Elfs
 */
static void elf_shutdown(Elf *elf)
{
    elf->close();
}

void *InitElf::load(const char *name, int flags /*= RTLD_LAZY*/)
{
    void *so = dlopen(name, flags);
    if (!so) {
        throw runtime_error(dlerror());
    }
    DEBLOG(cerr << "InitElf::load " << name << " -> "
           << so << '\n');
    return so;
}

void *InitElf::getSym(void *so, char *name, bool chuck)
{
    void *sym = dlsym(so, name);
    const char *err = dlerror();
    if (err && chuck) {
        throw runtime_error(err);
    } else {
        DEBLOG(cerr << "InitElf::getSym " << name
               << " -> " << sym << " from " << so << '\n');
    }
    return sym;
}

/** copy symbol contents from .so into the store.
 */
char *InitElf::copy_reloc(void *from, char *what, size_t len)
{
    void *symt = getSym(main_so, what);
    void *symf = getSym(from, what);
    
    DEBLOG(cerr << "InitElf::copy_reloc " << symt
           << ", " << symf
           << " " << len
           << '\n');
    memcpy(symt, symf, len);
    return NULL;
}

Elf *this_elf;

void dumpObj(const link_map *);

static void elf_done() {} // breakpoint on completion

InitElf::InitElf(const char *exename, const char *elflib)
        : exe(new ElfExe(exename)), 
    main_so((link_map *)load(NULL, RTLD_GLOBAL | RTLD_NOW)),
    elf((link_map *)load(elflib, RTLD_GLOBAL | RTLD_NOW /*| RTLD_LAZY */))
{
    //link_map *qvmm = (link_map *)load("../libs/libqvmm.so", RTLD_GLOBAL | RTLD_NOW /*| RTLD_LAZY */);

#ifdef notdef
    dumpObj(main_so);
    dumpObj(elf);
    //dumpObj(qvmm);
#endif
    //dlclose(elf);	// remove extra reference
    
    // copy relocate the global.so Elf structures
    copy_reloc(elf, "__vt_3Elf", 12);
    copy_reloc(elf, "__vt_4Csym", 12);
    copy_reloc(elf, "__vt_t1Q1Z3Elf", 12);
    copy_reloc(elf, "__vt_t1Q1Z4Csym", 12);
    //copy_reloc(qvmm, "__vt_8RefCount", 20);

    // initialize Elf module
    Elf::initialize(exe);
    
    // autoload requested 
    bkpt();	// call to bkpt
    for (AutoLoad *to_load = autoload_elfs; to_load->name; to_load++) {
        cerr << "Autoload " << to_load->name << endl;
        Elf *elf = Elf::findElf(to_load->name);
        if (!elf)
            elf = new Elf(to_load->name);

        this_elf = elf;

        try {
            elf->intern(to_load->type);
        } catch (char *err) {
            cerr << "Error: " << err << '\n';
        }

        // now call the initElf routine
        {
            void (*initElf)(Elf*);

            if (!to_load->initFn)
                to_load->initFn = "initElf__FP3Elf";
            initElf = (void(*)(Elf*))getSym(elf->_so, to_load->initFn, false);
            if (!initElf) {
                // the DSO may be a C program
                to_load->initFn = "initElf";
                initElf = (void(*)(Elf*))getSym(elf->_so, to_load->initFn, false);
            }

            if (initElf) {
                try {
                    bkpt();
                    (*initElf)(elf);
                } catch (char *message) {
                    cerr << "Autoload threw: "
                         << message
                         << '\n';
                } catch(std::exception& e) {
                    std::cerr << "Autoload exception: "
                              << e.what()
                              << std::endl;
                } catch (...) {
                    cerr << "Autoload: "
                         << to_load->initFn
                         << " threw\n";
                }
            } else {
                cerr << "No initElf function for " << to_load->name << '\n';
            }
        }
    }
    cerr << "DONE\n";
    elf_done();	// call to bkpt
}

InitElf::~InitElf()
{
    Elf::forallElf(elf_shutdown);
    delete exe;

    DEBLOG(cerr << "InitElf destroyed!!\n");
}

// Local Variables:
// mode:C++
// End:
