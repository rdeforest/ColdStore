// Server.cc - N: provider of network services
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Server.cc,v 1.9 2002/05/09 13:31:16 coldstore Exp $";

#define DEBUGLOG
// net.c: Network routines.
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#if 0
#include <sys/param.h>
#endif
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <grp.h>

#include "Data.hh"
#include "Vector.hh"
#include "Tuple.hh"
#include "Error.hh"
#include "List.hh"
#include "String.hh"

#include "Endpoint.hh"
#include "Connection.hh"
#include "Server.hh"

#include <Q.th>
template class Q<Server>;
template class Qh<Server>;
template class Qi<Server>;

#include "tSlot.th"
template union tSlot<Server>;

Server::Server(int port)
    : Endpoint(socket(AF_INET, SOCK_STREAM, 0)),
      Q<Server>(Server::all)
{
    // Create a socket.
    // Set SO_REUSEADDR option to avoid restart problems.
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(int));

    // Bind the socket to port.
    struct sockaddr_in sockin;		// An internet address.
    memset(&sockin, 0, sizeof(sockin));
    sockin.sin_family = AF_INET;
    sockin.sin_port = htons((unsigned short) port);
    if (bind(fd, (struct sockaddr *) &sockin, sizeof(sockin)) < 0) {
        close();
        throw new Error("net", this, errno);
    }

    // Start listening on port.  This shouldn't return an error under any circumstances.
    listen(fd, 8);
}

Server::Server(const Slot &sequence)
    : Endpoint(socket(AF_INET, SOCK_STREAM, 0)),
      Q<Server>(Server::all)
{
    int port = sequence[0];
    // Create a socket.
    // Set SO_REUSEADDR option to avoid restart problems.
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(int));

    // Bind the socket to port.
    struct sockaddr_in sockin;		// An internet address.
    memset(&sockin, 0, sizeof(sockin));
    sockin.sin_family = AF_INET;
    sockin.sin_port = htons((unsigned short)(int)port);
    if (bind(fd, (struct sockaddr *) &sockin, sizeof(sockin)) < 0) {
        close();
        throw new Error("net", this, errno);
    }

    // Start listening on port.  This shouldn't return an error under any circumstances.
    listen(fd, 8);
}

Server::~Server()
{}

Slot Server::incoming(const Slot from)
{
    return new BConnection(from);
}

Slot Server::disconnect(const Slot from)
{
    if ((Data*)from == (Data*)this) {
        // disconnect request - close down listener
    } else {
        return from;	// do nothing
    }
}

Slot Server::connect(const Slot to)
{
    struct sockaddr_in sockin;		// An internet address.
    socklen_t addr_size = sizeof(sockin);	// Size of sockin.
    int client_socket = accept(fd,
                               (struct sockaddr *) &sockin,
                               &addr_size);
    if (client_socket < 0) {
        check_error();
        return -1;
    }
    
    // Set the CLOEXEC flag on socket so that it will be
    // closed for a run_script() operation.
#ifdef FD_CLOEXEC
    int flags = fcntl(client_socket, F_GETFD);
    flags |= FD_CLOEXEC;
    fcntl(client_socket, F_SETFD, flags);
#endif

    return client_socket;
}

bool Server::tryConnect(fd_set &read_fds)
{
  if (FD_ISSET(fd, &read_fds)) {
    // A new connection attempt
    DEBLOG(cerr << "Server " << this
	   << " connection accepted"
	   << '\n');

    // spawn a new client socket
    try {
	Slot c = connect(this);	// indicate connection
	DEBLOG(cerr << "Server " << this
	       << " spawned connection " << c
	       << '\n');
    } catch (Error *err) {
	err->upcount();
	DEBLOG(cerr << "Server " << this
	       << " Connection error " << *err
	       << '\n');
	error->push(err);
	err->dncount();
	return false;
    }
    return true;
  }
  return false;
}

