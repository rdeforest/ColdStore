/* connection.h: Declarations for network routines. */

// $Id: Connection.hh,v 1.13 2002/05/09 13:31:16 coldstore Exp $

#ifndef CONNECTION_H
#define CONNECTION_H
#include <Endpoint.hh>

class Connection
    : public Endpoint,
      public Q<Connection>
{
    long iconnect(char *addr, int port);
    long uconnect(char *p);
    void openpty(int &master,
                 int &slave,
                 char *name,
                 struct termios *termp,
                 struct winsize *winp);
    long pconnect(char *prog);

protected:
    bool pipe;		// Endpoint is a pipe
    friend class Endpoint;
    mutable bool want_write;	// Connection wants write notification
    mutable bool want_read;	// Connection wants read notification

    /** try to write to this connection, if the FD_SET permits
     * @param write_fds FD_SET indicating whether write is possible
     */
    bool tryWrite(fd_set &write_fds);

    /** try to read to this connection, if the FD_SET permits
     * @param read_fds FD_SET indicating whether read is possible
     */
    bool tryRead(fd_set &read_fds);

    /** try to move this connection from Pending to Active, if the FD_SET permits
     * @param write_fds FD_SET indicating whether write is possible
     */
    bool tryPend(fd_set &write_fds);

public:

    /** create a pipe or unix connection
     * @param addr |program or unix named fifo
     */
    Connection(char *addr);

    /** create an outgoing inet connection
     * @param addr remote host
     * @param port remote port
     */
    Connection(char *addr, int port);

    /** connect an open connection
     * @param _fd connected endpoint file descriptor
     */
    Connection(int _fd);

    Connection(const Slot &sequence);

    virtual ~Connection();

    /** read from connection
     * @param len number of bytes to attempt to read
     * @return String of bytes actually read
     */
    virtual Slot slice(const Slot &len) const;
    virtual Slot slice(const Slot &start, const Slot &len) const;
    
    /** write to connection
     * @param value String to write
     * @return String of bytes as yet unwritten
     */
    virtual Slot replace(const Slot &value);

    static Qh<Connection> pending;	// set of all pending connections
    static Qh<Connection> all;		// set of all connected connections
    static const unsigned int MaxReadLen = 4096;
    //////////////////////////////////////////////
    // local methods
    char *peername();
};

/** Forwarding Connection
 * A Connection which forwards its events to transmitter and receiver objects
 */
class FConnection
    : public Connection
{
protected:
    mutable Slot rx;
    mutable Slot tx;

public:
    /** create a pipe or unix connection with a receiver and transmitter
     * @param addr |program or unix named fifo
     * @param _rx object to receive incoming indications
     * @param _tx object to receive outgoing indications
     */
    FConnection(char *addr, Slot _rx, Slot _tx);

    /** create an outgoing inet connection with a receiver and transmitter
     * @param addr remote host
     * @param port remote port
     * @param _rx object to receive incoming indications
     * @param _tx object to receive outgoing indications
     */
    FConnection(char *addr, int port, Slot _rx, Slot _tx);

    /** connect an open connection to a receiver and transmitter
     * @param _fd connected endpoint file descriptor
     * @param _rx object to receive incoming indications
     * @param _tx object to receive outgoing indications
     */
    FConnection(int _fd, Slot _rx, Slot _tx);

    FConnection(const Slot &sequence);

    virtual ~FConnection();

    ////////////////////////////////////
    // Network primitives

    /** connection indication/reuest
     * @param to indication's source
     * @return 
     */
    virtual Slot connect(const Slot to);

    /** diconnection indication/request
     * @param from indication's source
     * @return 
     */
    virtual Slot disconnect(const Slot from);
    
    /** incoming available indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot incoming(const Slot from);

    /** ready-for-outgoing indication/request
     * @param from indication's source
     * @return where to send future indications, disable if NULL
     */
    virtual Slot outgoing(const Slot to);

    /** write some data to Connection
     * @param value write this string
     * @return data unwritten
     */
    virtual Slot replace(const Slot &value);

    /** write some data to Connection and change indication
     * @param element receiver of future outgoing indications
     * @param value String to be written
     * @return data unwritten
     */
    virtual Slot replace(const Slot &element, const Slot &value);

    /** read from connection
     * @param len number of bytes to attempt to read
     * @return String of bytes actually read
     */
    virtual Slot slice(const Slot &len) const;

    /** read some data from Connection and change indication
     * @param element receiver of future incoming indications
     * @param len max length of data to read
     * @return data read
     */
    virtual Slot slice(const Slot &element, const Slot &len) const;
};

/** Buffering Connection
 * A Connection which buffers incoming and outgoing characters
 */
class BConnection
    : public FConnection
{
protected:
    mutable Slot rx_buf;
    mutable Slot tx_buf;

public:
    /** create a pipe or unix connection with a receiver and transmitter
     * @param addr |program or unix named fifo
     * @param _rx object to receive incoming indications
     * @param _tx object to receive outgoing indications
     */
    BConnection(char *addr, Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL);

    /** create an outgoing inet connection with a receiver and transmitter
     * @param addr remote host
     * @param port remote port
     * @param _rx object to receive incoming indications
     * @param _tx object to receive outgoing indications
     */
    BConnection(char *addr, int port,  Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL);

    /** connect an open connection to a receiver and transmitter
     * @param _fd connected endpoint file descriptor
     * @param _rx object to receive incoming indications
     * @param _tx object to receive outgoing indications
     */
    BConnection(int _fd,   Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL);

    BConnection(const Slot &sequence);
    virtual ~BConnection();
    
    ////////////////////////////////////
    // Network primitives

    /** connection indication/request
     * @param to indication's source
     * @return 
     */
    virtual Slot connect(const Slot to);

    /** incoming available indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot incoming(const Slot from);

    /** ready-for-outgoing indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot outgoing(const Slot to);

    /** read from connection's buffer
     * @param len number of bytes to attempt to read
     * @return String of bytes actually read
     */
    virtual Slot slice(const Slot &len) const;
    virtual Slot slice(const Slot &start, const Slot &len) const;
    
    /** write to connection's buffer
     * @param value String to write
     * @return String of bytes as yet unwritten
     */
    virtual Slot replace(const Slot &value);
};

/** Line Buffering Connection
 * A Connection which buffers incoming and outgoing lines
 */
class LConnection
    : public BConnection
{
protected:
  mutable Slot lines;		// List of complete lines
  Slot divisor;		// line divisor

public:
    LConnection(char *addr, Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL);
    LConnection(char *addr, int port,  Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL);
    LConnection(int _fd,   Slot _rx = (Data*)NULL, Slot _tx = (Data*)NULL);
    LConnection(const Slot &sequence);
    virtual ~LConnection();
    
    ////////////////////////////////////
    // Network primitives

    /** connection indication/reuest
     * @param to indication's source
     * @return 
     */
    virtual Slot connect(const Slot to);

    /** incoming available indication/request
     * @param from indication's source
     * @return where to send future indications
     */
    virtual Slot incoming(const Slot from);

    /** read from connection's buffer
     * @param len number of bytes to attempt to read
     * @return String of bytes actually read
     */
    virtual Slot slice(const Slot &len) const;
    virtual Slot slice(const Slot &start, const Slot &len) const;
};
#endif
