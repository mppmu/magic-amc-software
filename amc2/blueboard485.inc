//-----------------------------------------------------------------------------------


//open RS485 additional board in MPX computer **

int open_485_blueboard() {

  #define UBAUDRATE B19200


typedef enum {
 OSCI_IOCTL_RS232  = 1 << 0,
 OSCI_IOCTL_RS485  = 1 << 1,
 OSCI_IOCTL_FD     = 1 << 2,
} OSCI_IOCTL_MODE;

#define OIOC_SETMODE      _IOW('O', 0x00, unsigned int)
#define OIOC_GETMODE      _IOR('O', 0x01, unsigned int)
#define OIOC_SETRS485DLY  _IOW('O', 0x04, unsigned int)



  unsigned char rescmd[256];

  #include <stdlib.h>                                         
  #include <sys/socket.h>                                     
  #include <netinet/in.h>                                     
  #include <netdb.h>                                          
                                                         

//chains 0-6 and USCM at 7
   #define CMODEMDEVICE0    "/dev/ttyOS0"         //1   ch0 port
   #define CMODEMDEVICE1    "/dev/ttyOS1"         //2   ch1
   #define CMODEMDEVICE2    "/dev/ttyOS2"         //3   ch2
   #define CMODEMDEVICE3    "/dev/ttyOS3"         //4   ch3
   #define CMODEMDEVICE4    "/dev/ttyOS4"         //5   ch4
   #define CMODEMDEVICE5    "/dev/ttyOS5"         //6   ch5
   #define CMODEMDEVICE6    "/dev/ttyOS6"         //7   ch6
   #define CMODEMDEVICE7    "/dev/ttyOS7"         //8   ch7
   #define CBAUDRATE B115200


  int chain, ierr, i, rs485delay = 0, lfd[16], lfdalt[16];
  unsigned int 
      mode2=OSCI_IOCTL_RS232,
      mode1,
      mode = OSCI_IOCTL_RS485; //half duplex
  unsigned char chbuf[256], osdevice[30];

  struct termios oldtio[16],newtio[16];

//================================================


for(chain=0;chain<16;chain++) {lfd[chain]=-1; lfdalt[chain]=-1;}

bzero( &newtio, sizeof( newtio0 ) );

 for (chain=0; chain<chainmax; chain++) {
    sprintf(osdevice,"/dev/ttyOS%d",chain) ;

    printf("open chain %d as %s\n",chain,osdevice);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(osdevice, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
       fdalt[chain+8]=fd[ chain] = lfd[chain];
    if (lfd[chain] <0) {perror(osdevice); }
    else {
       ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
       ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
       printf(osdevice); printf("   opened for chains %d result %d\n",chain,lfd[chain]);

       tcgetattr(lfd[chain],&oldtio[chain]); // save current port settings
       newtio[chain].c_cflag = CBAUDRATE | CS8 | CLOCAL | CREAD;
       newtio[chain].c_iflag = IGNPAR ;
       newtio[chain].c_oflag = 0;
       newtio[chain].c_lflag = 0;
       newtio[chain].c_cc[VMIN]=0;
       newtio[chain].c_cc[VTIME]=1;     //for polling
       tcflush(lfd[chain], TCIFLUSH);
       tcsetattr(lfd[chain],TCSANOW,&newtio[chain]);
       fcntl(lfd[chain], F_SETFL, 0);            //blocking mode
    }
 }
//0-----------------------------------------------------------    
    bzero( &newtio, sizeof( newtio0 ) );

//for uscm: RS232
 if (chainmax<8) {
    chain=7;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE7, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    close(lfd[chain]);
    lfd[chain] = open(CMODEMDEVICE7, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
       printf("================= set USCM to /dev/ttyOS7 =========== %d at port %d\n",lfd[chain],chain);
    if (lfd[chain] <0) {perror(CMODEMDEVICE7); }
    else {
       ioctl(lfd[chain], OIOC_SETMODE, &mode2);   //set mode RS232
       mode1=0;
       ioctl(lfd[chain], OIOC_GETMODE, &mode1);   //get mode RS485
       printf(CMODEMDEVICE7); printf("blueboard: opened: mode1= %x. set to %x \n"
       ,mode1, mode2);

       tcgetattr(lfd[chain],&oldtio[chain]); // save current port settings

       newtio[chain].c_cflag = RS232_BAUDRATE | CS8 | CLOCAL | CREAD;
       newtio[chain].c_iflag = IGNPAR ;
       newtio[chain].c_oflag = 0;
       newtio[chain].c_lflag = 0;
       newtio[chain].c_cc[VMIN]=0;
       newtio[chain].c_cc[VTIME]=5;     //for polling
       tcflush(lfd[chain], TCIFLUSH);
       tcsetattr(lfd[chain],TCSANOW,&newtio[chain]);
       fcntl(lfd[chain], F_SETFL, 0);            //blocking mode
       fduscm = lfd[7];

    }
// } else {
//   printf(" ---------------- switch chains 5 and 7 -------------\n");
//   fd[5] = lfd[7] ;
//   fd[7] = lfd[5] ;
 }

//0-----------------------------------------------------------    
                          

// printf("blueboard485: fduscm = %d\n",fduscm);

}
