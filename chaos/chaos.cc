// Data - class from which all ColdStore entities must derive
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

static char *id __attribute__((unused))="$Id: chaos.cc,v 1.12 2002/04/22 13:47:14 coldstore Exp $";

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream.h>
#include <exception>
#include <string.h>
#include <dlfcn.h>
#include <qvmm.h>
#include <Q.hh>
#include <elf.hh>
#include <assert.h>
template class __default_alloc_template<false, 0>;
template class basic_string<char, string_char_traits<char>, __default_alloc_template<false, 0> >;

/* InitElf looks for a NULL terminated array called autoload_elfs[]
 */
AutoLoad autoload_elfs[] = {
    {"libcold.so", Elf::STL, NULL},
    {"chaos.so", Elf::STL, NULL},
    {NULL}
};

static int fd;

void chaos_bkpt()
{
}
 
void breakpoint()
{
}

void onError(const char *type = NULL)
{
}
 
void ColdTerminate()
{
}

void bkpt()
{
}

InitElf *initElf;

char **argv_glob;
int argc_glob;

int main(int argc, char **argv)
{
    // stash the args in a global
    argc_glob = argc;
    argv_glob = argv;

    set_terminate(ColdTerminate);
    // This check should probably go into Mmap itself
    //assert(static_cast<void *>(sbrk(0)) < Mmap::base || !"Program runs into the store!");
    void *so = dlopen(NULL, RTLD_GLOBAL | RTLD_NOW);
    if (!so) {
        throw dlerror();
    }
#ifdef DEBUGLOG
        cerr << "dlopen() -> " << so << '\n';
#endif

    try {
        fd = Mmap::open("coldstore");	// construct the Mmap'd store
        initElf = new InitElf(argv[0]);	// initialize Elf module
    } catch(std::exception& e) {
        std::cerr << "exception: "
                  << e.what()
                  << std::endl;
    } catch (char *err) {
        cerr << "ERROR: " << err << " (?" << strerror(int errno) << ")\n";
        onError();
    }

    chaos_bkpt();
    delete initElf;
    Mmap::close(fd);
    //close(fd);
    exit(0);
}

void shutdown()
{
    cerr << "Shutdown";
    //    dlclose(elf_so);
    delete initElf;
    Mmap::close(fd);
    //close(fd);
    exit(0);
}
