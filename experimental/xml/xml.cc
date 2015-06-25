// xml.cc: cold loader for xml
// Copyright (C) 2001, Ryan Daum
// see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms

static char *id __attribute__((unused))="$Id: xml.cc,v 1.1 2001/06/13 01:56:22 opiate Exp $";

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
    {"../libs/libcold.so", Elf::COLDSTORE, NULL},
    {"../browser/browser.so", Elf::COLDSTORE, NULL},
    {"./xml.so", Elf::COLDSTORE, NULL},
    {NULL}
};

static int fd;

void freon_bkpt()
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

int main(int argc, char **argv)
{
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
    freon_bkpt();
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
