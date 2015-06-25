/* endpoint.h: Declarations for network routines. */

// $Id: Endpoint.hh,v 1.7 2002/05/09 13:31:17 coldstore Exp $

#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <Data.hh>
#include <List.hh>
#include <Q.hh>

class Connection;
class Server;

class Endpoint
    : public Data
{
protected:
    mutable int fd;	// file descriptor

    /** check underlying socket for errors, recording them in error List
     * @return true iff an error was found
     */
    bool check_error() const;

public:
    Endpoint();
    /** Create an Endpoint from a connected file descriptor
     * @param _fd the connected file descriptor
     */
    Endpoint(int _fd);

    /** Create an Endpoint from a Connection
     * @param p the Connection
     */
    Endpoint(Connection *p);
    ~Endpoint();
    
    /** Wait for and Handle any I/O events.  
     * @param sec the number of seconds to wait before timeout or -1 to wait forever.
     * @return count of I/O events.
     */
    static int handle_io_events(long sec = -1);

    class Data *clone(void * = 0) const;
    int order(const Slot &) const;
    virtual Slot disconnect(const Slot from);
    virtual Slot connect(const Slot to);
    virtual Slot incoming(const Slot from);
    virtual Slot outgoing(const Slot to);

    //////////////////////
    // local methods
    void close() const;

    mutable bool dead;		// Endpoint is defunct.
    mutable tSlot<Stack> error;		// List of error conditions on this endpoint
    virtual Slot negative() const;	// dump the error conditions

    virtual ostream &dump(ostream&) const;
};

#endif
