// Prime - non-functioning test of qvmm.  From Texas pStore
// Modifications Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: prime.cc,v 1.2 2000/04/21 02:14:25 skeptopotamus Exp $";

////////////////////////////////////////////////////////////////
// File: prime.cc
//
// This program generates prime numbers and stores them in a linked
// list which is persistent.
//
#include <assert.h>
#include <stream.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include "qvmm.h"

class prime
    : public Memory
{
public:
    prime (unsigned long i, prime *_next = NULL)
            : number(i), next(_next)
        {}
    
    unsigned long number;
    prime *next;
};

prime *head __attribute__ ((section ("absolute"))) = NULL;
prime *head1 __attribute__ ((section ("absolute"))) = NULL;
prime *head2 __attribute__ ((section ("absolute"))) = NULL;

int store_fd;

// Catch signals and dump the store
void sighandle(int sig)
{
    Mmap::close( store_fd);
    exit(0);
}

main (int argc, const char **argv)
{
    unsigned int delay;

    if (argc < 2)
        delay = 0;
    else
        delay = atoi (argv[1]);

    signal(SIGHUP, sighandle);
    signal(SIGINT, sighandle);

    // Open the persistent store
    store_fd = open( "prime.store", O_RDWR|O_CREAT, 0777);
    assert( store_fd != -1);
    assert( Mmap::open( store_fd));

    cerr << head << '\n';
    int i = 0;
    
    // Allocate the head of the list as a named persistent object
    prime *ptr;
    unsigned long n;
    
    // Print and skip over and the prime numbers that we have already
    // generated. Nothing special is necessary to read the pstore. The
    // data will be faulted in automatically when the pointer is
    // referenced.
    if (head != NULL) {
        cout << "Primes from the persistent store:\n";
    } else {
        // seed the primes
        head = new prime(3);
        head = new prime(2, head);
    }
    
    for (ptr = head; ptr; ptr = ptr->next)
    {
        cout << ptr->number << ' ';
        i++;
        if (!(i % 8))
            cout << endl;
        n = ptr->number;
    }
    cout << endl;
    
    i = 0;
    cout << "New primes:\n";

    // Do a forever loop, calculating new primes
    for (n += 2; 1; n += 2) {
        for (ptr = head; ptr->next; ptr = ptr->next)
            if ((n % ptr->number) == 0)
                goto bottom_of_loop;
            
        // If we reached here, it means that the number is prime. Create a
        // new persistent node and store the prime number in the node and
        // link it on the existing list.
        ptr->next = new prime(n);
            
        // Increment the count of primes generated in this run. This value
        // is used to periodically checkpoint the pstore.
        i++;
        
        cout << n << ' ';
        cout.flush ();
        
        //sleep (delay);
        
      bottom_of_loop:;
    }

    cout << '\n';

    Mmap::close( store_fd);
}

// Original copyright of work from which this was derived:
////////////////////////////////////////////////////////////////
// Copyright (C) 1992, 1993, 1994 Sheetal Kakkad, Vivek Singhal and
// Paul Wilson
//
// This file is part of the Texas Persistent Storage system.
//
// The Texas Persistent Storage system is free software; you can
// redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// The Texas Persistent Storage system is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the Texas Persistent Storage system; if not, write to
// the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
// 02139, USA.
//
// You can contact the authors by email at <oops@cs.utexas.edu>.
//
