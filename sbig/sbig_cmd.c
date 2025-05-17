#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
     
#define PORT            7407
#define MESSAGE_GBJ     "GBADJ\n"
//## ##                  expos*100 zen azi pangrp
#define MESSAGE         " -1  -7 234 5016 \n"
#define MESSAGE_TST     "1\n2\n3\n4\n5\n6\n"
#define MESSAGE_REP     "CC-REPORT 00 2005 01 12 07 03 02 133 8 9 6 5 5 2 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 4 9 9 9 25.87 -10.07 00.00 00.00 04.53 729.23 00.00 05.87 107.60 01.77 OVER\n"
#define SERVERHOST      "amc-pc0"
//#define SERVERHOST      "ihp-pc12"


void
init_sockaddr (struct sockaddr_in *name,
               const char *hostname,
               uint16_t port)
{
  struct hostent *hostinfo;
     
  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL)
    {
      fprintf (stderr, "Unknown host %s.\n", hostname);
      exit (EXIT_FAILURE);
    }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}

     
void
write_to_server (int filedes)
{
  int nbytes;

  nbytes = write (filedes, MESSAGE, strlen (MESSAGE)    );
  if (nbytes < 0)
    {
      perror ("write");
      fprintf (stderr, "write error\n");
      exit (EXIT_FAILURE);
    }
}
     
void
write_to_server2(int filedes)
{
  int nbytes;
     
  nbytes = write (filedes, MESSAGE_REP, strlen (MESSAGE_REP)    );
  if (nbytes < 0)
    {
      perror ("write");
      fprintf (stderr, "write error\n");
      exit (EXIT_FAILURE);
    }
}
     
     
     
int
main (void)
{
  extern void init_sockaddr (struct sockaddr_in *name,
                             const char *hostname,
                             uint16_t port);
  int sock;
  struct sockaddr_in servername;
     
     
  printf("%s %d\n", SERVERHOST, PORT);

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket (client)");
      fprintf (stderr, "create error\n");
      exit (EXIT_FAILURE);
    }
     
  /* Connect to the server. */
  init_sockaddr (&servername, SERVERHOST, PORT);
  if (0 > connect (sock,
                   (struct sockaddr *) &servername,
                   sizeof (servername)))
    {
      perror ("connect (client)");
      fprintf (stderr, "connect error\n");
      exit (EXIT_FAILURE);
    }
     
  /* Send data to the server. */
      fprintf (stderr, "send 1\n");
  write_to_server (sock);


  close (sock);
  exit (EXIT_SUCCESS);
}
