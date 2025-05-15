//-----------------------------------------------------------------------------------


//open RS485 additional board in MPX computer **

int open_485_blueboard() {

//#define UMODEMDEVICE1 "/dev/ttyS0"   //for communication with USCM
  #define UBAUDRATE B19200


typedef enum {
 OSCI_IOCTL_RS232  = 1 << 0,
 OSCI_IOCTL_RS485  = 1 << 1,
 OSCI_IOCTL_FD     = 1 << 2,
} OSCI_IOCTL_MODE;

#define OIOC_SETMODE      _IOW('O', 0x00, unsigned int)
#define OIOC_GETMODE      _IOR('O', 0x01, unsigned int)
//#define OIOC_SETFIFO      _IOW('O', 0x02, unsigned int)
//#define OIOC_SETFLOW      _IOW('O', 0x03, unsigned int)
#define OIOC_SETRS485DLY  _IOW('O', 0x04, unsigned int)



  unsigned char rescmd[256];
//fduscm defined in HAdef_m1.h

  #include <stdlib.h>                                         
  #include <sys/socket.h>                                     
  #include <netinet/in.h>                                     
  #include <netdb.h>                                          
                                                         

//chains 1-8 ohne 3: 0-7 without 2
   #define CMODEMDEVICE0    "/dev/ttyOS0"         //1   ch0 port
   #define CMODEMDEVICE1    "/dev/ttyOS1"         //2   ch1
   #define CMODEMDEVICE2    "/dev/ttyOS2"         //3   ch2
   #define CMODEMDEVICE3    "/dev/ttyOS3"         //4   ch3
   #define CMODEMDEVICE4    "/dev/ttyOS4"         //5   ch4
   #define CMODEMDEVICE5    "/dev/ttyOS5"         //6   ch5
   #define CMODEMDEVICE6    "/dev/ttyOS6"         //7   ch6
   #define CMODEMDEVICE7    "/dev/ttyOS7"         //8   ch7
   #define CBAUDRATE B115200





  int chain, ierr, i, rs485delay = 0, lfd[18], lfdalt[18];
  int lfd0;
  unsigned int mode2=OSCI_IOCTL_RS232, mode1, mode = OSCI_IOCTL_RS485; //half duplex
//  unsigned int mode = OSCI_IOCTL_RS485 | OSCI_IOCTL_FD ;  //full duplex
  unsigned char chbuf[256];

  struct termios oldtio[16],newtio[16];



//================================================


for(chain=0;chain<18;chain++) {lfd[chain]=-1; lfdalt[chain]=-1;}

bzero( &newtio, sizeof( newtio0 ) );


chain=0;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE0, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE0); goto A1; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     printf(CMODEMDEVICE0); printf("   opened \n");
    }

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
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A1:;

//0-----------------------------------------------------------    //AB


chain=1;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE1, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE1); goto A2; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     printf(CMODEMDEVICE1); printf("   opened \n");
    }

    tcgetattr(lfd[chain],&oldtio[chain]); //save current port settings
    newtio[chain].c_cflag = CBAUDRATE | CS8 | CLOCAL | CREAD;
    newtio[chain].c_iflag = IGNPAR ;
    newtio[chain].c_oflag = 0;
    newtio[chain].c_lflag = 0;
    newtio[chain].c_cc[VMIN]=0;
    newtio[chain].c_cc[VTIME]=1;     //for polling
    tcflush(lfd[chain], TCIFLUSH);
    tcsetattr(lfd[chain],TCSANOW,&newtio[chain]);
    fcntl(lfd[chain], F_SETFL, 0);            //blocking mode
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A2:;

//0-----------------------------------------------------------    //AB
  

chain=2;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE2, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE2); goto A3; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     printf(CMODEMDEVICE2); printf("   opened \n");
    }

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
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A3:;
//0-----------------------------------------------------------    //AB
  
chain=3;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE3, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE3); goto A4; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     printf(CMODEMDEVICE3); printf("   opened \n");
    }

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
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A4:;
//0==================================================================

  
chain=4;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE4, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE4); goto A5; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     printf(CMODEMDEVICE4); printf("   opened \n");
    }

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
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A5:;
//0-----------------------------------------------------------    //AB
  
chain=5;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE5, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE5); goto A6; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     mode1=0;
     ioctl(lfd[chain], OIOC_GETMODE, &mode1);   //get mode:RS485 full duplex
     printf(CMODEMDEVICE5); printf("   opened: chain=%d, mode= %x \n",chain, mode1);
    }

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
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A6:;
//0-----------------------------------------------------------    
  
chain=6;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE6, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    close(lfd[chain]);
    lfd[chain] = open(CMODEMDEVICE6, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE6); goto A7; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     mode1=0;
     ioctl(lfd[chain], OIOC_GETMODE, &mode1);   //get mode:RS485 full duplex
     printf(CMODEMDEVICE6); printf("   opened: chain=%d, mode= %x \n",chain, mode1);
    }

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
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A7:;
//0-----------------------------------------------------------    
  
chain=7;
    printf("open chain %d\n",chain);
    // open the device to be non-blocking (read will return immediatly)
    lfd[chain] = open(CMODEMDEVICE7, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    close(lfd[chain]);
    lfd[chain] = open(CMODEMDEVICE7, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (lfd[chain] <0) {perror(CMODEMDEVICE7); goto A8; }
    else {
     ioctl(lfd[chain], OIOC_SETMODE, &mode);   //set mode RS485 full duplex
     ioctl(lfd[chain], OIOC_SETRS485DLY, &rs485delay);   //set RS485 delay
     mode1=0;
     ioctl(lfd[chain], OIOC_GETMODE, &mode1);   //get mode:RS485 full duplex
     printf(CMODEMDEVICE6); printf("   opened: chain=%d, mode= %x \n",chain, mode1);
    }

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
    if(send_break != 0 ) ioctl(lfd[chain], TCSBRK, 100);
A8:;

//0-----------------------------------------------------------    
                          
//    fduscm = lfd[7];

//  lfd0     = lfd[0];        // switch  0 <--> 7
//  lfd[0]   = lfd[7];
//  lfd[7]   = lfd0  ;

//  lfd0     = lfd[1];        // switch  1 <--> 5
//  lfd[1]   = lfd[5];
//  lfd[5]   = lfd0  ;

    fduscm = 0;
    fd[    0] = lfd[0];
    fdalt[ 9] =  fd[0];
    fd[    1] = lfd[1];
    fdalt[10] =  fd[1];
    fd[    2] = lfd[2];
    fdalt[11] =  fd[2];
    fd[    3] = lfd[3];
    fdalt[12] =  fd[3];
    fd[    4] = lfd[4];
    fdalt[13] =  fd[4];
    fd[    5] = lfd[5];
    fdalt[14] =  fd[5];
    fd[    6] = lfd[6];
    fdalt[15] =  fd[6];
    fd[    7] = lfd[7];
    fdalt[16] =  fd[7];


printf("blueboard485: exiting\n");


}
