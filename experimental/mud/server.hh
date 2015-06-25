

struct net
{
public:
  Slot connections_dict;

  fd_set master_set, working_set;
  int user_count;
  int listen_sd, max_sd, new_sd;
  Slot connection;
  int rc;
  struct timeval       timeout;

  void startServer( int port );
  void loop( Frame *context );
  void receive_line( int sockfd, char *buffer, int len, Frame *context );
  void new_connection(int sockfd, Frame *context );
  void disconnect( int sockfd, Frame *context );
};

net *n;
