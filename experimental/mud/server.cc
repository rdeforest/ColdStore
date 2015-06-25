#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <Data.hh>
#include <Store.hh>
#include <Object.hh>
#include <Dict.hh>
#include <List.hh>
#include <Frame.hh>
#include <Error.hh>
#include <String.hh>
#include <Closure.hh>

#include "common.hh"

#include "server.hh"

#define TRUE             1
#define FALSE            0

void net::receive_line(int sockfd, char *buffer, int len, Frame *context )
{
  char *lf_end = rindex(buffer, '\n' );
  char *cr_end = rindex(buffer, '\r' );
  if (cr_end)
    *cr_end = 0;
  if (lf_end)
    *lf_end = 0;
  *(buffer + len) = 0;

  Slot str = new String( buffer );

  try {
    connections_dict[sockfd].receiveLine(str);
  } catch (Error *e) {
    cerr << "no entry in connections_dict for socket file descriptor" << endl;
    disconnect(sockfd, (Frame*)NULL);
  }
}

void net::disconnect(int sockfd, Frame *context )
{
  if (connections_dict->search(sockfd)) {
    Slot who = connections_dict[sockfd];
    connections_dict[sockfd].disconnect();
    connections_dict->del(sockfd);

    store->Names->insert("connections", connections_dict);
  }
}

void net::new_connection( int sockfd, Frame *context )
{
  // create user and add to dict
  char uname[20];
  sprintf(uname, "connection_%d", sockfd );
  
  Slot x = connection.create(uname);
  x.setNames(uname);
  x.setSocket( new_sd );

  connections_dict = connections_dict->insert( new_sd, x );
  store->Names->insert("connections", connections_dict);

  Slot args = new List();

  x.connect();
}

void net::startServer( int port )
{
   int    on = 1;
   struct sockaddr_in   addr;

   user_count = 0;

   if (store->Names->search("connections")) {
     connections_dict = store->Names->slice("connections")[2];
     Slot c_it = connections_dict->iterator();
     while (c_it->More()) {
       Slot entry = c_it->Next();
       cerr << entry << endl;
       Frame *context = (Frame*)NULL;
       disconnect( (int)entry[0], context );
       
     }
   }
   
   connections_dict = new Dict();

   connection = ((Object*)(Data*)store->Names->slice("connection")[2]);

   /*************************************************************/
   /* Create an AF_INET stream socket to receive incoming       */
   /* connections on                                            */
   /*************************************************************/
   listen_sd = socket(AF_INET, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(-1);
   }
 
   /*************************************************************/
   /* Allow socket descriptor to be reuseable                   */
   /*************************************************************/
   rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
   }
 
   /*************************************************************/
   /* Set socket to be non-blocking.  All of the sockets for    */
   /* the incoming connections will also be non-blocking since  */
   /* they will inherit that state from the listening socket.   */
   /*************************************************************/
   rc = ioctl(listen_sd, FIONBIO, (char *)&on);
   if (rc < 0)
   {
      perror("ioctl() failed");
      close(listen_sd);
      exit(-1);
   }
 
   /*************************************************************/
   /* Bind the socket                                           */
   /*************************************************************/
   memset(&addr, 0, sizeof(addr));
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port        = htons(port);
   rc = bind(listen_sd,
             (struct sockaddr *)&addr, sizeof(addr));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
   }
 
   /*************************************************************/
   /* Set the listen back log                                   */
   /*************************************************************/
   rc = listen(listen_sd, 32768);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
   }
 
   /*************************************************************/
   /* Initialize the master fd_set                              */
   /*************************************************************/
   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);
 
   /*************************************************************/
   /* Initialize the timeval struct to 3 minutes.  If no        */
   /* activity after 100 minutes this program will end.           */
   /*************************************************************/
   timeout.tv_sec  = 10000 * 60;
   timeout.tv_usec = 0;
}

void net::loop( Frame *context ) 
{
   char   buffer[1024];
   int i, len;
   int    close_conn;
   int    desc_ready, end_server = FALSE;
   
   /*************************************************************/
   /* Loop waiting for incoming connects or for incoming data   */
   /* on any of the connected sockets.                          */
   /*************************************************************/
   do
   {
      /**********************************************************/
      /* Copy the master fd_set over to the working fd_set.     */
      /**********************************************************/
      memcpy(&working_set, &master_set, sizeof(master_set));
 
      /**********************************************************/
      /* Call select() and wait 5 minutes for it to complete.   */
      /**********************************************************/
      printf("Waiting on select()...\n");
      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
 
      /**********************************************************/
      /* Check to see if the select call failed.                */
      /**********************************************************/
      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }
 
      /**********************************************************/
      /* Check to see if the 5 minute time out expired.         */
      /**********************************************************/
      if (rc == 0)
      {
         printf("  select() timed out.  End program.\n");
         break;
      }
 
      /**********************************************************/
      /* One or more descriptors are readable.  Need to         */
      /* determine which ones they are.                         */
      /**********************************************************/
      desc_ready = rc;
      for (i=0; i <= max_sd  &&  desc_ready > 0; ++i)
      {
         /*******************************************************/
         /* Check to see if this descriptor is ready            */
         /*******************************************************/
         if (FD_ISSET(i, &working_set))
         {
            /****************************************************/
            /* A descriptor was found that was readable - one   */
            /* less has to be looked for.  This is being done   */
            /* so that we can stop looking at the working set   */
            /* once we have found all of the descriptors that   */
            /* were ready.                                      */
            /****************************************************/
            desc_ready -= 1;
 
            /****************************************************/
            /* Check to see if this is the listening socket     */
            /****************************************************/
            if (i == listen_sd)
            {
               printf("  Listening socket is readable\n");
               /*************************************************/
               /* Accept all incoming connections that are      */
               /* queued up on the listening socket before we   */
               /* loop back and call select again.              */
               /*************************************************/
               do
               {
                  /**********************************************/
                  /* Accept each incoming connection.  If       */
                  /* accept fails with EWOULDBLOCK, then we     */
                  /* have accepted all of them.  Any other      */
                  /* failure on accept will cause us to end the */
                  /* server.                                    */
                  /**********************************************/
                  new_sd = accept(listen_sd, NULL, NULL);
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        end_server = TRUE;
                     }
                     break;
                  }
 
                  /**********************************************/
                  /* Add the new incoming connection to the     */
                  /* master read set                            */
                  /**********************************************/
                  printf("  New incoming connection - %d\n", new_sd);

		  new_connection( new_sd, context );
		  
                  FD_SET(new_sd, &master_set);
                  if (new_sd > max_sd)
                     max_sd = new_sd;
 
                  /**********************************************/
                  /* Loop back up and accept another incoming   */
                  /* connection                                 */
                  /**********************************************/
               } while (new_sd != -1);
            }
 
            /****************************************************/
            /* This is not the listening socket, therefore an   */
            /* existing connection must be readable             */
            /****************************************************/
            else
            {
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;
               /*************************************************/
               /* Receive all incoming data on this socket      */
               /* before we loop back and call select again.    */
               /*************************************************/
               do
               {
                  /**********************************************/
                  /* Receive data on this connection until the  */
                  /* recv fails with EWOULDBLOCK.  If any other */
                  /* failure occurs, we will close the          */
                  /* connection.                                */
                  /**********************************************/
                  rc = recv(i, buffer, sizeof(buffer), 0);
                  if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }
 
                  /**********************************************/
                  /* Check to see if the connection has been    */
                  /* closed by the client                       */
                  /**********************************************/
                  if (rc == 0)
                  {
                     printf("  Connection closed\n");
                     close_conn = TRUE;
		     disconnect(i, context );
                     break;
                  }
 
                  /**********************************************/
                  /* Data was recevied                          */
                  /**********************************************/
                  len = rc;
                  printf("  %d bytes received\n", len);
		  
		  receive_line( i, buffer, len, context );
		  break;
               } while (TRUE);
 
               /*************************************************/
               /* If the close_conn flag was turned on, we need */
               /* to clean up this active connection.  This     */
               /* clean up process includes removing the        */
               /* descriptor from the master set and            */
               /* determining the new maximum descriptor value  */
               /* based on the bits that are still turned on in */
               /* the master set.                               */
               /*************************************************/
               if (close_conn)
               {
                  close(i);
                  FD_CLR(i, &master_set);
                  if (i == max_sd)
                  {
                     while (FD_ISSET(max_sd, &master_set) == FALSE)
                        max_sd -= 1;
                  }
               }
            } /* End of existing connection is readable */
         } /* End of if (FD_ISSET(i, &working_set)) */
      } /* End of loop through selectable descriptors */
 
   } while (end_server == FALSE);
 
   /*************************************************************/
   /* Cleanup all of the sockets that are open                  */
   /*************************************************************/
   for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }
}

static Slot connections( Frame *context ) {
  Slot cons = new List();
  Slot it = n->connections_dict->iterator();
  while (it->More())
    cons->insert( it->Next()[1] );

  return cons;
}

static Slot startServer( Frame *context )
{
  n = new net();
  try { 
    n->startServer(4000);
    n->loop(context);
  } catch (Frame *fr) {
    fr->upcount();
    cerr << (char*)fr->traceback() ;
    fr->dncount();
  } 
  return true;
}  


void initElf(Elf *elf)
{

  try {
    // ladies and gentlemen, the top frame
    Frame *context = ((Frame*)0);

    // retrieve the world object defined by mud.so
    Slot world = store->Names->slice("world")[2];

    // if startServer is already on it, there's no need to do
    // anything else -- the world is already initialized.

    if (!world->search("startServer")) {
      // add our "start_server" method to it
      world.startServer = &startServer;
      world.connections = &connections;
      
      // and then use it to initialize the world
      world.initialize();
    }

    // then start the server
    world.startServer();

  } catch (Error *e) {
    e->upcount();
    cerr << "Error " <<  ((Symbol*)(Data*)e)->name() << "\n";
    e->dncount();
  } catch (Frame *e) {
    e->upcount();
    cerr << (char*)e->traceback() << endl;
    e->dncount();
  }
}



