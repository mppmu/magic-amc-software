
#include <string.h> 
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>


#include "CC.h"

//=========================================================


/************************************/
SOCKET *sopen(void)
{
  int optval=1;
  socklen_t optlen;

    SOCKET *sp;

    if ((sp = (SOCKET *)malloc(sizeof(SOCKET)))==0) 
        return 0;

    if ((sp->sd = socket(AF_INET, SOCK_STREAM & SO_KEEPALIVE, 0))==-1)
    {
        free(sp); 
        return 0;
    }

    sp->sinlen = sizeof(sp->sin); 
    sp->bindflag = S_RESET;

    return sp;
}

/******************************************/
int sclose(SOCKET *sp)
{
    int sd;

    sd = sp->sd; 
    free(sp);
    return close(sd);
}

/******************************************/
int sserver(SOCKET *sp, int port, int sync)
{
    int flags; 
    struct hostent *hostent; 
    char localhost[S_NAMLEN+1];

    if (sp->bindflag==S_RESET)
    {
        if (gethostname(localhost, S_NAMLEN)==-1
            || (hostent = gethostbyname(localhost))==0) 
            return -1;

        sp->sin.sin_family = (short)hostent->h_addrtype; 
        sp->sin.sin_port = htons((unsigned short)port); 
        sp->sin.sin_addr.s_addr = *(unsigned long *)hostent->h_addr;
        
        if (bind(sp->sd, (struct sockaddr *)&sp->sin, sp->sinlen)==-1
            || listen(sp->sd, 5)==-1) 
            return -1;

        sp->bindflag = S_SET;
        switch (sync)
        {
            case S_DELAY:
                if ((flags = fcntl(sp->sd, F_GETFL))==-1
                    || fcntl(sp->sd, F_SETFL, flags&~O_NDELAY)==-1) 
                    return -1;
                break;

            case S_NDELAY:
                if ((flags = fcntl(sp->sd, F_GETFL))==-1
                    || fcntl(sp->sd, F_SETFL, flags|O_NDELAY)==-1) 
                    return -1;
                break;
            default:
                return -1;
        }               
    }
    return accept(sp->sd, (struct sockaddr *)&sp->sin, &sp->sinlen);
}

/**********************************************/
int sclient(SOCKET *sp, char *name, int port)
{
    struct hostent *hostent;

//    if ((hostent = gethostbyname(name))==0)
//        return -1;

    sp->sin.sin_family = (short)hostent->h_addrtype; 
    sp->sin.sin_port = htons((unsigned short)port); 
//  sp->sin.sin_addr.s_addr = *(unsigned long *)hostent->h_addr;
    sp->sin.sin_addr.s_addr = 0                                ;
    if (connect(sp->sd, (struct sockaddr *)&sp->sin, sp->sinlen)==-1) 
        return -1;

    return sp->sd;
}





/***************************************************************************
*
* readline - read a line from socket 
*
* Read a line from a descriptor.  Read the line one byte at a time,
* looking for the newline.  We store the newline in the buffer,
* then follow it with a null (the same as fgets(3)).
* We return the number of characters up to, but not including,
* the null (the same as strlen(3)).
*
* RETURNS
* number of bytes read, -1 if error,  0 if connection closed
*/

int readline(
        int fd,                 /* the sockets file descriptor */
        char *ptr,              /* pointer where data are put in */
        int maxlen              /* maximum number of bytes to read */
        )
{
int n, rc;
char c;

  for (n = 1; n < maxlen; n++) {
    if ( (rc = recv(fd, &c, 1, 0 ) ) == 1) {
      /* read exactly one char, like it should do */
//    printf("readline %c %d\n",c,n);
      *ptr++ = c;
      if (c == '\n')
        break;
    } else if (rc == 0) {
           if (n == 1)   return(0);  /* EOF, no data read */
           else          break;              /* EOF, some data was read */
    } else {    /* rc == -1 ?? */
      int errno_tmp;
      errno_tmp = errno;
      perror("readline(read())");
      if(errno_tmp ==  EINTR) { continue;
      } else { return(-1); }  /* error */
  } }   
  *ptr = 0;
  return(n);
}


