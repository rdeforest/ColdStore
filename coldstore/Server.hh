/* server.h: Declarations for network routines. */

// $Id: Server.hh,v 1.6 2002/05/09 13:31:16 coldstore Exp $

#ifndef SERVER_H
#define SERVER_H
#include <Endpoint.hh>

class Server
    : public Endpoint,
    public Q<Server>
{
    unsigned short port;	// what port are we listening to?

protected:
    friend class Endpoint;
    static void flush_defunct();
    //int client_socket;

    /** try Connection
     * @param read_fds FD_SET indicating whether socket is readable
     */
    bool tryConnect(fd_set &read_fds);

    /** connection indication
     * @param to indication's source
     * @return Connection to handle incoming request
     */
    virtual Slot connect(const Slot to);
    
    /** incoming available indication/request
     * @param always this, with a valid client_socket
     * @return Connection to handle request
     */
    virtual Slot incoming(const Slot from);

    /** disconnection indication/request
     * @param from indication's source
     * @return 
     */
    virtual Slot disconnect(const Slot from);
    
public:
    Server(int port);
    Server(const Slot &);
    ~Server();

    static Qh<Server> all;
};

#endif
