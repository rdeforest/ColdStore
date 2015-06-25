// Endpoint.cc - Coldstore Endpoint
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Endpoint.cc,v 1.14 2002/05/09 13:31:17 coldstore Exp $";

//#define DEBUGLOG
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

#include "tSlot.th"
template union tSlot<Endpoint>;

Endpoint::Endpoint(int _fd)
        : fd(_fd),
          error(new Stack)
{
    if (fd < 0) {
        throw new Error("net", "socket failed");
    }
}

Endpoint::Endpoint()
        :  fd(-1),
          error(new Stack)
{
}

Endpoint::Endpoint(Connection *p)
        : fd(p->fd),
          error(new Stack)
{
    if (fd < 0) {
        throw new Error("net", "socket failed");
    } else {
        p->fd = -1;
        delete p;
    }
}

// Endpoints are inherently mutable
Data *Endpoint::clone(void *where) const
{
    return (Data*)this;
}

int Endpoint::order(const Slot &arg) const
{
    return -(arg->order(fd));
}

void Endpoint::close() const
{ 
   DEBLOG(cerr << "Endpoint::close " << this << '\n');
   if (fd >=0) {
        ::close(fd);
        fd = -1;
    }
    dead = true;
}

Endpoint::~Endpoint()
{
    close();
}

bool Endpoint::check_error() const
{
    socklen_t dummy = sizeof(int);
    int err = 0;  // in case this is a unix domain
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *) &err,
               &dummy);
    if (err) {
        error->push(new Error("net", this, err));
        return true;
    } else {
        return false;
    }
}

// Wait for and Handle any I/O events.  sec is the number of seconds we get to wait for
// something to happen before the timer wants the thread back, or -1 if we can
// wait forever.  Returns count of I/O events.
int Endpoint::handle_io_events(long sec)
{
    // Begin with blank file descriptor masks and an nfds of 0.
    int nfds = 0;
    fd_set read_fds, write_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    // Listen for new data on connections, and also check for ability to write
    // to them if we have data to write.
    for (Qi<Connection> cp(Connection::all); cp.More();) {
        Connection *conn = (Connection*)cp;
        cp.Next();

        int fd = conn->fd;
        if (!conn->dead) {
	    DEBLOG(cerr << "live: " << conn << ' ');
            if (conn->want_read) {
                // connections can ignore readability
	        DEBLOG(cerr << "rx ");
                FD_SET(fd, &read_fds);
            }

            if (conn->want_write) {
                // connections can ignore writability
	        DEBLOG(cerr << "tx ");
                FD_SET(fd, &write_fds);
            }

            if (fd >= nfds)
                nfds = fd + 1;
	    DEBLOG(cerr << nfds << "\n");

        } else {
            // clean up defunct Connections
            conn->Q<Connection>::del();
            try {
                conn->dncount();
            } catch (Error *err) {
                DEBLOG(cerr << "Connection " << conn
                       << " dncount error " << *err
                       << '\n');
            }            
        }
    }

    // Listen for connections on the server sockets.
    for (Qi<Server> sp(Server::all); sp.More();) {
        Server *serv = (Server*)sp;
        sp.Next();

        if (!serv->dead) {
            // Servers always listen
            FD_SET(serv->fd, &read_fds);
            if (serv->fd >= nfds)
                nfds = serv->fd + 1;
        } else {
            // clean up defunct Servers
            serv->Q<Server>::del();
            try {
                serv->dncount();
            } catch (Error *err) {
                DEBLOG(cerr << "Server " << serv
                       << " dncount error " << *err
                       << '\n');
            }
        }
    }

    // Check pending connections for connectedness
    for (Qi<Connection> cp(Connection::pending); cp.More();) {
        Connection *pend = (Connection*)cp;
        cp.Next();

        if (!pend->dead) {
            // Pending always wait for connection
            FD_SET(pend->fd, &write_fds);
            if (pend->fd >= nfds)
                nfds = pend->fd + 1;
        } else {
            // clean up defunct Pending connections
            pend->Q<Connection>::del();
            try {
                pend->dncount();
            } catch (Error *err) {
                DEBLOG(cerr << "Pending Connection " << pend
                       << " dncount error " << *err
                       << '\n');
            }
        }
    }

    // Set time structure according to sec.
    struct timeval tv, *tvp;
    if (sec == -1) {
	tvp = NULL;
        // this is a rather odd thing to happen for me
        //throw new Error("net", "select:  forever wait");
    } else {
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	tvp = &tv;
    }

    // Call select().
    int count = rand();
    count = select(nfds, &read_fds, &write_fds, NULL, tvp);

    // Lose horribly if select() fails on anything but an interrupted system call.
    // On EINTR, we'll return 0.
    if (count == -1 && errno != EINTR)
	throw new Error("net", new String(strerror(errno)));

    // Stop and return zero if no I/O events occurred.
    if (count <= 0)
	return 0;

    int events = 0;
    
    // Check if any connections are readable or writable.
    for (Qi<Connection> cp(Connection::all); cp.More(); cp.Next()) {
      Connection *conn = (Connection*)cp;
      if (conn->tryRead(read_fds))
	events++;
      if (conn->tryWrite(write_fds))
	events++;
    }

    // Check if any server sockets have new connections.
    for (Qi<Server> sp(Server::all); sp.More(); sp.Next()) {
        Server *serv = (Server*)sp;
        if (serv->tryConnect(read_fds))
	  events++;
    }

    // Check if any pending connections have succeeded or failed.
    for (Qi<Connection> cp(Connection::pending); cp.More(); cp.Next()) {
      Connection *pend = (Connection*)cp;
      if (pend->tryPend(write_fds)) {
	events++;
      }
    }

    // Return nonzero, indicating that at least one I/O event occurred.
    return events;
}

Slot Endpoint::disconnect(const Slot from)
{
  cerr << "Endpoint::disconnect\n";
    close();
}

Slot Endpoint::connect(const Slot from)
{
    throw new Error("~unimpl", this, "Raw Endpoint doesn't implement connect");
}

Slot Endpoint::incoming(const Slot from)
{
    throw new Error("~unimpl", this, "Raw Endpoint doesn't implement incoming");
}

Slot Endpoint::outgoing(const Slot from)
{
    throw new Error("~unimpl", this, "Raw Endpoint doesn't implement outgoing");
}

ostream &Endpoint::dump(ostream& out) const
{
  out << "Endpoint " << fd;
  return out;
}

Slot Endpoint::negative() const
{
    return error;
}
