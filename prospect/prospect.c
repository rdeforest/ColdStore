/* Prospect.
   Copyright 1993, 1994, 1995 Tristan Gingold
		  Written August 1993 by Tristan Gingold

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License 
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

The author may be reached by US/French mail:
		Tristan Gingold 
		8 rue Parmentier
		F-91120 PALAISEAU
		FRANCE
*/

// #define START_DEEPCHECK_AT 0

// coldstore is curently starting at 0x
// #define START_DEEPCHECK_AT 48


#define HAVE_UNISTD_H
#define HAVE_SYS_TIME_H
#define HAVE_SYS_RESOURCE_H
#define HAVE_SYS_MMAN_H

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

/* How to stringify...  */
#define STR(c) #c
#define STR1(c) STR(c)

extern char etext, edata, end;

/* Verbose.  */
int flag_verbose = 0;

#define SPAM0(level,fmt) if (flag_verbose >= level) { fprintf(stderr, fmt); }
#define SPAM1(level,fmt,expr) if (flag_verbose >= level) { fprintf(stderr, fmt, expr); }
#define SPAM2(level,fmt,e1,e2) if (flag_verbose >= level) { fprintf(stderr, fmt, e1, e2); }
#define SPAM3(level,fmt,e1,e2,e3) if (flag_verbose >= level) { fprintf(stderr, fmt, e1, e2,e3); }

#define MB 1024*1024

#define DEFAULT_STACK_LIMIT 8*MB
#define DEFAULT_PAGESIZE 4*1024
int mm_heap_size = 64*MB;
int mm_stack_size = 64*MB;
int mm_mem_size = 64*MB;
int mm_sym_size = 64*MB;
#define CHECKER_NEEDED_MEMORY (mm_heap_size + mm_stack_size + mm_mem_size + mm_sym_size)
#define MAX_NEEDED_MEMORY (4 * 64 * MB)

#ifndef HAVE_GETPAGESIZE

#ifdef VMS
#define getpagesize() 512
#endif

#ifdef HAVE_SC_PAGESIZE
#define getpagesize() sysconf(_SC_PAGESIZE)
#else

#ifdef HAVE_SC_PAGE_SIZE
#define getpagesize() sysconf(_SC_PAGE_SIZE)
#else

#include <sys/param.h>

#ifdef EXEC_PAGESIZE
#define getpagesize() EXEC_PAGESIZE
#else
#ifdef NBPG
#define getpagesize() NBPG * CLSIZE
#ifndef CLSIZE
#define CLSIZE 1
#endif /* no CLSIZE */
#else /* no NBPG */
#ifdef NBPC
#define getpagesize() NBPC
#else
#define getpagesize() DEFAULT_PAGESIZE
#define DEFAULT_PAGESIZE_USED
#endif /* NBPC */
#endif /* no NBPG */
#endif /* no EXEC_PAGESIZE */
#endif /* no _SC_PAGE_SIZE */
#endif /* no _SC_PAGESIZE */

#endif /* not HAVE_GETPAGESIZE */


caddr_t
anon_mmap (caddr_t addr, size_t len, int prot, int flags)
{

#ifndef MAP_ANONYMOUS
  static int fd = -1;
  if (fd == -1)
    {
      fd = open ("/dev/zero", O_RDWR);
      if (fd == -1)
	{
	  perror ("Can't open /dev/zero");
	  exit (2);
	}
    }
#endif
#ifdef MAP_ANONYMOUS
  return mmap (addr, len, prot, flags | MAP_ANONYMOUS, -1, 0);
#else
  return mmap (addr, len, prot, flags, fd, 0);
#endif
}

int
get_max_stack_size (void)
{
#ifdef RLIMIT_STACK
  struct rlimit stack_limit;
  if (getrlimit (RLIMIT_STACK, &stack_limit) == 0)
    if (stack_limit.rlim_max != RLIM_INFINITY)
      return stack_limit.rlim_max;
#endif
  return DEFAULT_STACK_LIMIT;
}

int *
call_for_sp()
{
  int d;
  int *res = &d;
  
  return res;
}

void
buggy_os (void)
{
  fprintf (stderr, "This OS is too bad.  Find another one!");
  exit (1);
}

#undef MY_MIN
#undef MY_MAX
#undef BASE 
#define MY_MIN(a,b) ((a) < (b) ? (a) : (b))
#define MY_MAX(a,b) ((a) > (b) ? (a) : (b))
#define BASE(a,b) (stack_grows_way == -1 ? MY_MAX((a),(b)) : MY_MIN((a),(b)))

unsigned int stack_top_int;
unsigned int stack_base_int;
unsigned int checker_base_int;
int stack_grows_way;
int pagesize;

jmp_buf env;

void
sig_handler (int sig)
{
  longjmp (env, sig);
}

void
set_sig_handler (void)
{
    signal (SIGBUS, sig_handler);
    signal (SIGSEGV, sig_handler);
}

/* 0: bad, 1: Ok */
int
check_for_ptr (char *ptr, int fd_file)
{
    int res;
    caddr_t addr;
  
    /* An available area must not be busy.  */
    res = write (fd_file, ptr, 8);
    if (res == -1) {
        if (errno != EFAULT)
            buggy_os ();
    } else {
        /* Success: this mean that the memory is already used. */
        return 0;	/* Don't touch.  */
    }

    /* An available area must be usuable.  */
    addr = anon_mmap (ptr, pagesize, PROT_READ | PROT_WRITE, 
                      MAP_PRIVATE | MAP_FIXED);
    if (addr == (caddr_t) -1)
        return 0;
    
    res = setjmp (env);
    if (res == 0) {
        /* Now, test it.  */
        *(volatile char *)addr += 1;
        munmap (addr, pagesize);
        return 1;
    } else {
	  SPAM2(1, "Received signal %d at addr = 0x%08x\n", res, (unsigned)addr);
	  munmap (addr, pagesize);
	  set_sig_handler ();
	  return 0;
    }
    return 1;
}

#define TMP_FILE "tmp-prosp"

void
find_available_memory (void)
{
  char map[256];		/* 0: available, 1: busy.  */
#define AVAILABLE	0
#define BUSY		1
#define NULLTRAP	2
#define STACK		3
#define OSAVAILABLE	4
#define ACCURATEBUSY	5
  //int fd;
  int fd_file;
  caddr_t addr;
  unsigned int i, j;
  unsigned int i1, j1;
  int begin, len;
  int deepsearch;

  set_sig_handler ();
  fd_file = open (TMP_FILE, O_RDWR | O_CREAT, 0666);
  if (fd_file == -1)
    buggy_os ();
  if (write (fd_file, "hello", 4) != 4)
    buggy_os ();
 
  /* Search for available area.  */
  for (i = 0; i < 256; i++)
    {
      if (check_for_ptr ((char *) (i << 24), fd_file) == 0) {
        map[i] = BUSY;
		SPAM2(2,"Quick Check 0x%08x-0x%08x\t===> Busy\n",
			  i << 24, ((i + 1) << 24) - 1)
	  } else {
		map[i] = AVAILABLE;
		SPAM2(2, "Quick check 0x%08x-0x%08x\t===> Available\n", 
			  i << 24, ((i + 1) << 24) - 1)
	  }
    }

  /* Test the NULL zone... */
  if (map[0] == AVAILABLE)
    {
      SPAM0(1, "NULL pointer are not trapped on this machine ???\n");
    }
  map[0] = NULLTRAP;
      
  /* The stack is not available.  */
  if (stack_grows_way > 0)
    for (i = stack_base_int >> 24; i <= (stack_top_int >> 24); i++)
      map[i] = STACK;
  else
    for (i = stack_top_int >> 24; i <= (stack_base_int >> 24); i++)
      map[i] = STACK;
  
  /* Find the most available area from the os...  It is not available for
     Checker.  */
  addr = anon_mmap ((caddr_t)0, pagesize, PROT_READ | PROT_WRITE, MAP_PRIVATE);
  if (addr == (caddr_t) -1)
    buggy_os ();
  munmap (addr, pagesize);
  i = ((unsigned long int) addr) >> 24;
  map[i] = OSAVAILABLE;
  SPAM1(1,"Available area from the OS: 0x%08x\n",(unsigned int) addr)
    
  /* If all the memory is busy, search deeply...  This feature is useful
     when the verbose flag is set.  */
  deepsearch = 1;
  for (i = 1; i < 256; i++) {
    if (map[i] == AVAILABLE) {
	  deepsearch = 0;
	  break;
	}
  }

  SPAM0(1,"Performing deep search... This will take a while\n");

  /* Be more accurate.  */
  for (i = 0; i < 256; i++) {
    if (map[i] == AVAILABLE || deepsearch) {
        SPAM0(2, " Deep Check           ");
        for (j = 0; j < 4096; j++)
          {
			SPAM1(2,"\b\b\b\b\b\b\b\b\b\b0x%08x",((i << 12) + j) << 12)
            if (check_for_ptr ((char *)(((i << 12) + j) << 12), fd_file) == 0)
              {
                map[i] = ACCURATEBUSY;
                SPAM3(1,"\tFail at 0x%08x (i:%2x, j:%2x)",
					  ((i << 12) + j) << 12, i, j);
                if (!deepsearch)
                  break;
              } else {
				//
			  }
          }
		  SPAM0(1,"\n");
      }
  }  
  /*close (fd);*/
  close (fd_file);
  unlink (TMP_FILE);
  
  /* Speak again.  */
  if (flag_verbose)
    for (i = 0; i < 256; i++)
      if (map[i] == AVAILABLE)
        {
          j = i + 1;
          while (map[j] == AVAILABLE && j < 256)
            j++;
          fprintf (stderr, "Available memory in: 0x%08x-0x%08x (0x%08x)\n",
                   i << 24,
				   (j << 24) - 1,
                   (j << 24) - (i << 24)
                   );
          i = j;
        }

  /* Find a big enough available zone.  */
  begin = 0;
  len = 0;
  for (i = 1; i < 256; i++)
    if (map[i] == AVAILABLE)
      {
	j = i + 1;
	while (map[j] == AVAILABLE && j < 256)
	  j++;
	/* If this zone is too near from the available space given by the
	   os, don't accept it.  */
	i1 = i;
	j1 = j;
	if (map[i1 - 1] == OSAVAILABLE)
	  i1++;
	if (j1 > i1 && map[j1] == OSAVAILABLE)
	  j1--;
	/* If the size is greather than the previous one, use this zone.  */
	if (j1 - i1 > len)
	  {
	    len = j1 - i1;
	    begin = i1;
	  }
	i = j;
      }
  
  if (begin == 0)
    return;
  
  /* Convert len into bytes.  */
  if (len >= (MAX_NEEDED_MEMORY >> 24))
    {
      /* Reduce the size according to the stack.  */
      if (stack_grows_way == 1)
	begin = begin + len - (MAX_NEEDED_MEMORY >> 24);
      len = MAX_NEEDED_MEMORY;
    }
  else
    len <<= 24;
  
  checker_base_int = begin << 24;
  /* Equipartition.  */
  mm_heap_size = mm_stack_size = mm_mem_size = mm_sym_size = len / 4;
}

int
main (int argc, char *argv[], char *envp[])
{
  int dummy;
  int exit_status;
  int i;
  char *stack_base;
  char *p;
  
  if (argc == 2 && (strcmp (argv[1], "-v") == 0 || strcmp (argv[1], "--verbose") == 0))
    flag_verbose = 1;
  else
    flag_verbose = 0;

  if (argc == 3 && (strcmp (argv[2], "-v") == 0 || strcmp (argv[2], "--verbose") == 0))
    flag_verbose = 2;
 
  exit_status = 0;
  
  /* Page size.  */
  pagesize = getpagesize ();
  printf (
	"/* The page size.  Note: CHKR_PAGESIZE = 1 << LOG_PAGESIZE. */\n"
	"#undef CHKR_PAGESIZE\t/* this is defined in sys/param.h */\n"
	"#define CHKR_PAGESIZE pagesize\n"
	"#define LOG_PAGESIZE log_pagesize\n"
	"#define INIT_PAGESIZE " STR1(getpagesize())
#ifdef DEFAULT_PAGESIZE_USED
	"\t/* default value.  */"
#endif
	"\n\n");

  /* Find the behavior of the stack.  */
  printf ("/* Define STACK_GROWS_DOWNWARD is the stack grows downward.  */\n");
  if (call_for_sp() < &dummy)
    {
      if (flag_verbose)
        printf("Stack seems to grow down.\n");
      printf("#define STACK_GROWS_DOWNWARD\n");
      stack_grows_way = -1;
    }
  else
    {
      if (flag_verbose)
        printf("Stack seems to grow up.\n");
      printf("#undef STACK_GROWS_DOWNWARD\n");
      stack_grows_way = 1;
    }
  printf ("\n");
  
  /* Find the base of the stack.  */
  stack_base = (char*)&argc;
  for (i = 0; i < argc; i++)
    {
      p = argv[i];
      stack_base = BASE(stack_base, p);
      p = argv[i] + strlen(argv[i]) + 1;
      stack_base = BASE(stack_base, p);
    }
  for (i = 0; envp[i]; i++)
    {
      p = envp[i];
      stack_base = BASE(stack_base, p);
      p = envp[i] + strlen(envp[i]) + 1;
      stack_base = BASE(stack_base, p);
    }
  stack_base_int = ((int)stack_base + pagesize - 1) & ~(pagesize-1);
  if (flag_verbose)
    {
      printf("The raw stack base: %p\n", stack_base);
      printf("The rounded stack base: 0x%08x\n", stack_base_int);
    }
  printf ("/* The stack base, ie where the stack begins.  */\n");
  printf("#define STACK_BASE 0x%08x\n\n", stack_base_int);
  
  stack_top_int = stack_base_int + stack_grows_way * get_max_stack_size ();
  printf ("/* The stack top.  */\n");
  printf ("#define STACK_TOP 0x%08x\n\n", stack_top_int);
  
  /* Try to find enough available memory for Checker.  */
  find_available_memory ();
  
  /* Default available memory.  */
  if (!checker_base_int)
    {
      if (stack_grows_way == 1)
        checker_base_int = stack_top_int;
      else
        checker_base_int = stack_top_int - CHECKER_NEEDED_MEMORY;
      printf ("/* Aie Aie Aie... \n"
      	      "   `prospect' fails to find available memory.\n"
      	      "   Try to analyse the output of `./prospect -v'.  \n");
      exit_status = 1;
    }
    
  printf (
#ifdef MAP_ANONYMOUS
	"/* MAP_ANONYMOUS is available.  */\n"
	"#define HAVE_ANONYMOUS\n"
#else
	"/* ANONYMOUS is not available.  Use devzero_fd. */\n"
	"#undef HAVE_ANONYMOUS\n"
#endif
	"\n"
	"#ifdef NEED_MM\n"
	"#include <sys/mman.h>\n"
	"#ifndef MAP_FILE\n"
	"#define MAP_FILE 0\n"
	"#endif\n"
	"#define MM_PROT 	PROT_READ | PROT_WRITE\n"
#ifdef MAP_ANONYMOUS
	"#define MM_FLAGS	MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS\n"
	"#define MM_FILE	(-1)\n"
#else
	"#define MM_FLAGS	MAP_FIXED | MAP_PRIVATE\n"
	"#define MM_FILE	devzero_fd\n"
#endif
	);
  
  printf ("/* Between MM_LOW and MM_HIGH, the user can't access.  */\n");
  printf ("#define MM_LOW 0x%08x\n", checker_base_int);
  printf ("#define MM_HIGH 0x%08x\n\n", checker_base_int + CHECKER_NEEDED_MEMORY);
  
  printf ("/* Memory above MM_HEAP is used by sys_malloc (ie the internal heap, used only"
	  "   Checker). */\n");
  printf ("#define MM_HEAP 0x%08x\n\n", checker_base_int);
  
  printf ("/* Where the stack bitmap begins.  */\n");
  printf ("#define MM_STACK 0x%08x\n\n", checker_base_int + mm_heap_size);
  
  printf ("/* Where the bitmap for heaps begins.  */\n");
  printf ("#define MM_MEM 0x%08x\n\n", checker_base_int + mm_heap_size + mm_stack_size);
  
  printf ("/* Where the symbol table is loaded.  */\n");
  printf ("#define MM_SYM 0x%08x\n\n", checker_base_int + mm_heap_size + mm_stack_size + mm_mem_size);
  
  printf ("#endif /* NEED_MM */\n");
  
  if (flag_verbose)
    {
      fprintf(stderr, "&etext = 0x%08x; &edata = 0x%08x; &end = 0x%08x\n", (int)&etext, (int)&edata, (int)&end);
    }
    
  return exit_status;
}
