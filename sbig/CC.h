#include <netinet/in.h>
#include <sys/poll.h>
#define PORT 7407
#define S_LIBRARY

#define S_DELAY 0
#define S_NDELAY 1

#ifdef S_LIBRARY
#define S_RESET 0
#define S_SET 1
#define S_NAMLEN 64
#endif

typedef struct
{
    struct sockaddr_in sin; 
    int sinlen;
    int bindflag; 
    int sd;
} SOCKET;


SOCKET *sopen(void);
int     sclose(SOCKET *);
int     sserver(SOCKET *, int, int);
int     sclient(SOCKET *, char *, int);
//int     sclient(SOCKET *, int);
int     readline(int fd, char *ptr, int maxlen);
//int   readbuffer(int fd, char *ptr, int maxlen);

int     CC_init(void) ;
//int     fatal(char *text);
//int     do_service(int sd);

