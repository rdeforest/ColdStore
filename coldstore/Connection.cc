// Connection.cc - Network/FIFO/pty connection
// Copyright (C) 1998,1999 Colin McCormack,
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

static char *id __attribute__((unused))="$Id: Connection.cc,v 1.18 2002/05/09 13:31:16 coldstore Exp $";
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

#include "Connection.hh"

#include <Q.th>
template class Q<Connection>;
template class Qh<Connection>;
template class Qi<Connection>;

#include "tSlot.th"
template union tSlot<Connection>;

Connection::Connection(char *addr)
        :  Q<Connection>(Connection::pending)
{
    if (addr[0] == '|') {
        fd = pconnect(addr+1);
        pipe = true;
    } else {
        fd = uconnect(addr);
    }

    upcount();	// reflect the inclusion on Q
}

Connection::Connection(char *addr, int port)
    : Q<Connection>(Connection::pending)
{
    fd = iconnect(addr, port);
    upcount();	// reflect the inclusion on Q
}


Connection::Connection(int _fd)
        : Endpoint(_fd),
          Q<Connection>(Connection::all)
{
    // assert server here?
    upcount();	// reflect the inclusion on Q
}

Connection::Connection(const Slot &sequence)
        : Q<Connection>(Connection::all)
{
    int l = sequence->length();
    if (l == 1) {
	Slot arg1(sequence[0]);
	if (AKO(arg1, String)) {
	    char *addr = (char*)(toType(arg1, String)->nullterm());
	    if (addr[0] == '|') {
		fd = pconnect(addr+1);
		pipe = true;
	    } else {
		fd = uconnect(addr);
	    }
	} else {
	    int port(arg1);
	    fd = port;
	}
    } else {
	tSlot<String> addr = sequence[0];
	int port = sequence[1];
	fd = iconnect((char*)(addr->nullterm()), port);
    }
}

Connection::~Connection()
{
    // Free the data associated with the connection.
    if (pipe)
        wait(0);	// clean up zombie
}

char *Connection::peername()
{
    socklen_t addr_size = 100;	// Size of sockin.
    static char name[101];

    int result = getpeername(fd,
                             (struct sockaddr *) name,
                             &addr_size);
    if (result == 0) {
        name[addr_size] = '\0';
        return name;
    } else {
        int err = EOPNOTSUPP;  // in case this is a unix domain
        socklen_t dummy = sizeof(int);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *) &err,
                   &dummy);
        throw new Error("net", this, err);
    }
}

long Connection::iconnect(char *addr, int port)
{
    // Convert address to struct in_addr.
    struct in_addr inaddr;
    if (!inet_aton(addr, &inaddr))
        throw new Error("net", addr, "bad address");
    
    // Get a socket for the connection.
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        throw new Error("net", this, errno);
    }

    // Set the socket non-blocking.
    int flags = fcntl(s, F_GETFL);
#ifdef FNDELAY
    flags |= FNDELAY;
#else
#ifdef O_NDELAY
    flags |= O_NDELAY;
#endif
#endif
    fcntl(s, F_SETFL, flags);

    // Make the connection.
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((unsigned short) port);
    saddr.sin_addr = inaddr;
    int result;
    do {
        result = ::connect(s, (struct sockaddr *) &saddr, sizeof(saddr));
    } while (result == -1 && errno == EINTR);
    
    if (result == -1 && errno == EINPROGRESS) {
        ::close(s);
        throw new Error("net", this, errno);
    }
    return s;
}

long Connection::uconnect(char *p)
{
    // Get a socket for the connection.
    int s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s < 0) {
        throw new Error("net" , this, errno);
    }
    
    // Set the socket non-blocking.
    int flags = fcntl(s, F_GETFL);
#ifdef FNDELAY
    flags |= FNDELAY;
#else
#ifdef O_NDELAY
    flags |= O_NDELAY;
#endif
#endif
    fcntl(s, F_SETFL, flags);
    
    // Make the connection.
    struct sockaddr_un sock_un;
    sock_un.sun_family =  AF_UNIX; 
    strcpy(sock_un.sun_path,p);
    int result = ::connect(s,
                         (struct sockaddr *)&sock_un,
                         strlen(sock_un.sun_path) + 2);
    
    if (result) {
        ::close(s);
        throw new Error("net", this, errno);
    }
    return s;
}

void Connection::openpty(int &master,
                         int &slave,
                         char *name,
                         struct termios *termp,
                         struct winsize *winp)
{
    static char line[] = "/dev/ptyXX";
    struct group *gr;

    register int ttygid;
    if ((gr = getgrnam("tty")) != NULL)
        ttygid = gr->gr_gid;
    else
        ttygid = -1;
    
    register const char *cp1, *cp2;
    for (cp1 = "pqrs"; *cp1; cp1++) {
        line[8] = *cp1;
        for (cp2 = "0123456789abcdef"; *cp2; cp2++) {
            struct stat buf;
            line[9] = *cp2;
            
            if (stat(line, &buf) < 0) {
                if (errno == ENOENT)
                    throw new Error("net", this, "out of ptys");
            } 
            
            master = open(line, O_RDWR | O_EXCL, 0);
            if ((master >= 0) && (buf.st_mode & 4)) {
                line[5] = 't';
                if ((slave = open(line, O_RDWR, 0)) != -1) {
                    
                    (void) chown(line, getuid(), ttygid);
                    (void) chmod(line, S_IRUSR|S_IWUSR|S_IWGRP);
                    
                    if (name)
                        strcpy(name, line);
                    if (termp)
                        (void) tcsetattr(slave, TCSAFLUSH, termp);
                    if (winp)
                        (void) ioctl(slave, TIOCSWINSZ, (char *)winp);
                    
                    return;
                }
            }
            (void) ::close(master);
            line[5] = 'p';
        }
    }
}

// non-pty exec usage is like rsh, shell can handle compound command
long Connection::pconnect(char *prog)
{
    int soc[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, soc) < 0) {
        throw new Error("net", this, strerror(errno));
    }

    // soc[0] is coldmud's end.  Set it non-blocking.
    int flags = fcntl(soc[0], F_GETFL);
#ifdef FNDELAY
    flags |= FNDELAY;
#else
#ifdef O_NDELAY
    flags |= O_NDELAY;
#endif
#endif
    fcntl(soc[0], F_SETFL, flags);

    int pid;
    if ((pid = fork ()) < 0) {
        throw new Error("net", this, strerror(errno));
    }

    if (pid == 0) // child.
    { 
        char **argv;
        int i=0,j=0;

        // count args
        for (j = 0, i = 1; prog[j]; j++)
            if (prog[j] == ' ')
                i++;
        argv = (char **)calloc(sizeof(char *), i+1);

        // unpack args
        argv[0] = prog;
        for (j = 0, i = 0; prog[j]; j++) {
            if (prog[j] == ' ') {
                prog[j++] = '\0';
                while (prog[j] && prog[j] == ' ');
                if (prog[j])
                    argv[++i] = &prog[j];
            }
        }
        argv[++i]=(char *)0;

        ::close (0);
        ::close (1);
        ::close (2);

        // soc[1] is child's end
        dup2 (soc[1], 0);
        dup2 (soc[1], 1);
        dup2 (soc[1], 2);
        for (i = 3; i < 64;++i)
            ::close(i);
        
        setbuf(stdout, NULL);
    
        setuid(getuid());
        setgid(getgid());
    
        execvp(argv[0], argv);
        
        throw new Error("net", this, strerror(errno));
        exit(1);
    }

    // parent
    ::close(soc[1]);

    return soc[0];
}

Slot Connection::slice(const Slot &len) const
{
    int l = (int)len;
    if (l < 0)
        l = MaxReadLen;
    Slot result = new String(l);
    String *r = (String*)result;
    r->vconcat((Char*)"",l);	// populate the String
    l = read(fd, r->content(), l);	// read the string
    DEBLOG(cerr << "Connection::slice " << l << ' ' << errno << '\n');

    if (l < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            // We were interrupted; deal with this next time around.
            return (Data*)NULL;
        } else {
            // The connection closed.
            error->push(new Error("net", this, errno));
            check_error();
            dncount();
            return (Data*)NULL;
        }
    } else if (check_error()) {
        // The connection closed.
        const_cast<Connection* const>(this)->disconnect(this);
        return (Data*)NULL;
    } else if (l == 0) {
        // The connection had no data
        const_cast<Connection* const>(this)->disconnect(this);
        return (Data*)NULL;
    } else {
        result = result->del(l, -1);
        return result;
    }
}

Slot Connection::slice(const Slot &start, const Slot &len) const
{
  return slice(len);
}

Slot Connection::replace(const Slot &value)
{
    tSlot<String> data = value;    
    int r = write(fd, data->content(), data->length());	// try to write whole buffer
    
    if (r <= 0) {
        if (errno == EINTR || errno == EAGAIN) {
            // We were interrupted; deal with this next time around.
            return value;
        } else {
            // We lost the connection.
            error->push(new Error("net", this, errno));
            check_error();
	    disconnect(this);
            return value;
        }
    } else if (check_error()) {
	// We lost the connection.
        disconnect(this);
        return value;
    } else {
      // we've written part of the output.  Return the unsent part
      return value->del(0, r);
    }
}

bool Connection::tryRead(fd_set &read_fds)
{
  if (!dead && FD_ISSET(fd, &read_fds)) {
    DEBLOG(cerr << "Connection " << this
	   << " readable"
	   << '\n');
    try {
      incoming(this);
    } catch (Error *err) {
      DEBLOG(cerr << "Connection " << this
	     << " incoming error " << *err
	     << '\n');
      error->push(err);
      return false;
      try {
	disconnect(this);
      } catch (Error *err1) {
	DEBLOG(cerr << "Connection " << this
	       << " disconnection error " << *err1
	       << '\n');
	error->push(err1);
	return false;
      }
      return false;
    }
    return true;
  }
  return false;
}

bool Connection::tryWrite(fd_set &write_fds)
{
  if (!dead && FD_ISSET(fd, &write_fds)) {
    DEBLOG(cerr << "Connection " << this
	   << " writable"
	   << '\n');
    try {
      Slot result = outgoing(this);
      // turn off writable notifications if outgoing result == NULL
      // they can be turned on again with a NULL write
      if (!result) {
	want_write = false;
      }
    } catch (Error *err) {
      DEBLOG(cerr << "Connection " << this
	     << " outgoing error " << *err
	     << '\n');
      error->push(err);
      return false;
      try {
	disconnect(this);
      } catch (Error *err1) {
	DEBLOG(cerr << "Connection " << this
	       << " disconnection error " << *err1
	       << '\n');
	error->push(err1);
	return false;
      }
    }
    return true;
  }
  return false;
}

bool Connection::tryPend(fd_set &write_fds)
{
  if (FD_ISSET(fd, &write_fds)) {
    if (check_error()) {
      try {
	disconnect(this);
      } catch (Error *err) {
	DEBLOG(cerr << "Pending " << this
	       << " disconnection error " << *err
	       << '\n');
	error->push(err);
	dead = true;
	return false;
      }
    } else {
      // move Connection into active state
      Connection::all.add(this);
      try {
	DEBLOG(cerr << "Pending " << this
	       << " connected"
	       << '\n');
	connect(this);
      } catch (Error *err) {
	DEBLOG(cerr << "Pending " << this
	       << " connection error " << *err
	       << '\n');
	error->push(err);
	try {
	  disconnect(this);
	} catch (Error *err1) {
	  DEBLOG(cerr << "Pending " << this
		 << " disconnection error " << *err1
		 << '\n');
	  error->push(err1);
	  dead = true;
	}
	return false;
      }
      return true;
    }
  }
  return false;
}

FConnection::FConnection(char *addr, Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL)
        : Connection(addr),
          rx(_rx),
          tx(_tx)
{}
                
FConnection::FConnection(char *addr, int port, Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL)
        : Connection(addr, port),
          rx(_rx),
          tx(_tx)
{}

FConnection::FConnection(int _fd, Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL)
        : Connection(_fd),
          rx(_rx),
          tx(_tx)
{}

FConnection::FConnection(const Slot& sequence)
    : Connection(sequence[0])
{
    rx = sequence[1];
    tx = sequence[2];
}

FConnection::~FConnection()
{}


Slot FConnection::connect(const Slot to)
{
    try {
        if (rx) {
            DEBLOG(cerr << "FConnection " << this
                   << " forwarding to rx " << rx
                   << " connection " << to
                   << '\n');
	    want_read = true;
            rx->connect(to);
        }
    } catch (Error *err) {
        error->push(err);
        DEBLOG(cerr << "FConnection " << this
               << " error " << *err
               << " forwarding to rx " << rx
               << " connection " << to
               << '\n');
    }
    try {
        if (tx) {
            DEBLOG(cerr << "FConnection " << this
                   << " forwarding to tx " << tx
                   << " connection " << to
                   << '\n');
	    want_write = true;            
            tx->connect(to);
        }
    } catch (Error *err) {
        error->push(err);
        DEBLOG(cerr << "FConnection " << this
               << " error " << *err
               << " forwarding to tx " << tx
               << " connection " << to
               << '\n');
    }
    return this;
}

Slot FConnection::disconnect(const Slot from)
{
    try {
        if (rx) {
            DEBLOG(cerr << "FConnection " << this
                   << " forwarding to rx " << rx
                   << " disconnection " << from
                   << '\n');

            rx->disconnect(from);
        }
    } catch (Error *err) {
        error->push(err);
        DEBLOG(cerr << "FConnection " << this
               << " error " << *err
               << " forwarding to rx " << rx
               << " disconnection " << from
               << '\n');
    }
    try {
        if (tx) {
            DEBLOG(cerr << "FConnection " << this
                   << " forwarding to rx " << rx
                   << " disconnection " << from
                   << '\n');
            
            tx->disconnect(from);
        }
    } catch (Error *err) {
        error->push(err);
        DEBLOG(cerr << "FConnection " << this
               << " error " << *err
               << " forwarding to tx " << tx
               << " disconnection " << from
               << '\n');
    }
    close();
    return this;
}

Slot FConnection::incoming(const Slot from)
{
    try {
        if (rx) {
            DEBLOG(cerr << "FConnection " << this
                   << " forwarding to rx " << rx
                   << " incoming " << from
                   << '\n');

            return rx = rx->incoming(from);
        } else {
            want_read = false;
        }
    } catch (Error *err) {
        error->push(err);
        DEBLOG(cerr << "FConnection " << this
               << " error " << *err
               << " forwarding to rx " << rx
               << " incoming " << from
               << '\n');
    }
}

Slot FConnection::outgoing(const Slot to)
{
    try {
        if (tx) {
            DEBLOG(cerr << "FConnection " << this
                   << " forwarding to rx " << rx
                   << " outgoing " << to
                   << '\n');
            // send write indication
            return tx = tx->outgoing(to);
        } else {
	  // if there's no sender, we don't want to hear write indications
	  want_write = false;
        }
    } catch (Error *err) {
        error->push(err);
        DEBLOG(cerr << "FConnection " << this
               << " error " << *err
               << " forwarding to tx " << tx
               << " outgoing " << to
               << '\n');
    }
    return tx;
}

Slot FConnection::replace(const Slot &value)
{
    want_write = true;
    return Connection::replace(value);
}

Slot FConnection::replace(const Slot &element, const Slot &value)
{
    if (tx) {
        if (element != tx) {
            throw new Error("net", element, "Trying to steal write forwarding");
        }
    } else {
        tx = element;
    }
    want_write = true;
    return Connection::replace(value);
}

Slot FConnection::slice(const Slot &element, const Slot &len) const
{
    if (rx) {
        if (element != rx) {
            throw new Error("net", element, "Trying to steal read forwarding");
        }
    } else {
        rx = element;
    }
    want_read = true;
    Connection::slice(len);
    return rx;
}

Slot FConnection::slice(const Slot &len) const
{
    want_read = true;
    Connection::slice(len);
    return rx;
}

BConnection::BConnection(char *addr, Slot _rx, Slot _tx)
        : FConnection(addr, _rx, _tx)
{}

BConnection::BConnection(char *addr, int port, Slot _rx, Slot _tx)
        : FConnection(addr, port, _rx, _tx)
{}

BConnection::BConnection(int _fd, Slot _rx, Slot _tx)
        : FConnection(_fd, _rx, _tx)
{}

BConnection::BConnection(const Slot& sequence)
    : FConnection(sequence)
{}

BConnection::~BConnection()
{}

Slot BConnection::connect(const Slot to)
{
    rx_buf = new String();
    tx_buf = new String();
    return FConnection::connect(to);
}

Slot BConnection::incoming(const Slot from)
{
    Slot result;
    Slot len(MaxReadLen);

    // get input from the underlying connection
    while (result = Connection::slice(len))
        rx_buf = rx_buf->concat(result);

    // let FConnection handle notifications
    return FConnection::incoming(from);
}

Slot BConnection::outgoing(const Slot to)
{
  // while there's some output to go, and some space for it
  if (tx_buf->truth()) {
    // try to send any output we have buffered
    tx_buf = Connection::replace(tx_buf);
  }
  if (want_write = tx_buf->truth()) {	// do we still have something to write?
    return (Data*)NULL;	// gotta return something :)
  } else {
    // if we have exhausted our bufferred output, let forwarder know there's more space
    return FConnection::outgoing(to);
  }
}

Slot BConnection::slice(const Slot &len) const
{
  Slot result = rx_buf->slice(0, len);	// get the required string
  if (rx_buf->length())
    rx_buf = rx_buf->del(0, len);	// delete it from our buffer
  want_read = true;	// we want input indications
  return result;	// here's what was read
}

Slot BConnection::slice(const Slot &start, const Slot &len) const
{
    Slot result = slice(len);
    rx = start;
    return result;
}

Slot BConnection::replace(const Slot &value)
{
    tx_buf = tx_buf->concat(value);
    want_write = true;	// we have something to write - we want write indications
    //return tx_buf;
    return this;
}

LConnection::LConnection(char *addr, Slot _rx, Slot _tx)
  : BConnection(addr, _rx, _tx),
    lines(new List()),
    divisor("\n")
{}

LConnection::LConnection(char *addr, int port, Slot _rx, Slot _tx)
  : BConnection(addr, port, _rx, _tx),
    lines(new List()),
    divisor("\n")
{}

LConnection::LConnection(int _fd, Slot _rx, Slot _tx)
  : BConnection(_fd, _rx, _tx),
    lines(new List()),
    divisor("\n")
{}

LConnection::LConnection(const Slot& sequence)
    : BConnection(sequence)
{}

LConnection::~LConnection()
{}

Slot LConnection::connect(const Slot to)
{
  return BConnection::connect(to);
}

Slot LConnection::incoming(const Slot from)
{
  // get input from the underlying connection as part of incomplete
  Slot result;
  Slot len(MaxReadLen);

  // get input from the underlying connection
  while (result = Connection::slice(len)) {
    DEBLOG(cerr << "LConnection::incoming " << result << '\n');
    rx_buf = rx_buf->concat(result);
  }
  DEBLOG(cerr << "LConnection::incoming " << rx_buf->dump(cerr) << '\n');
  DEBLOG(cerr << "LConnection::incoming " << lines->dump(cerr) << '\n');
  // break incomplete input into lines
  lines = lines->concat(rx_buf->divide(divisor));
  rx_buf = lines->slice(-1);	// push back possibly incomplete last line
  lines = lines->del(-1);	// only provide complete lines
  return FConnection::incoming(from);
}

Slot LConnection::slice(const Slot &len) const
{
  // return up to the nominated number of lines
  if (lines->length()) {
    Slot result = lines->slice(0, len);	// get the required strings
    lines = lines->del(0, len);	// delete it from our buffer
    want_read = true;	// we want input indications
    return result;	// here's what was read
  } else {
    return new List();
  }
}

Slot LConnection::slice(const Slot &start, const Slot &len) const
{
    Slot result = slice(len);
    rx = start;
    return result;
}

