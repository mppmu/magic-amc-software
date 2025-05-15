/*RS232lib.inc*/

 int open_RS232( int port) {

  #define RS232_MODEMDEVICE0 "/dev/ttyS0"
  #define RS232_MODEMDEVICE1 "/dev/ttyS1"
  #define RS232_MODEMDEVICE2 "/dev/ttyS2"
  #define RS232_MODEMDEVICE3 "/dev/ttyS3"

  #define RBAUDRATE B115200
  #define RS232_BAUDRATE B19200

  struct termios oldtio0,newtio0;
  struct termios oldtio1,newtio1;
  struct termios oldtio2,newtio2;
  struct termios oldtio3,newtio3;


  if( (port < 0) || (port > 3)) {
   printf("open_RS323: illegal port %d\n",port);
   return(-5);
  }


//RS232:  distance sensor
  if( port == 0 ) {
   fd232[port] = open(RS232_MODEMDEVICE0, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
   if (fd232[port] <0) {perror(RS232_MODEMDEVICE0); exit(-1); }
    else {printf(RS232_MODEMDEVICE0); printf("   opened, port= %d \n",port);}
    tcgetattr(fd232[port],&oldtio1); /* save current port settings */
    /* specially for the DISTANCE SENSOR: CS7 | PARENB */
    bzero( &newtio0, sizeof( newtio0 ) );
    newtio0.c_cflag = RS232_BAUDRATE | CS7 | CLOCAL | CREAD | PARENB; //even parity is default
    newtio0.c_iflag = IGNPAR ;                                  //ignor parity errors
    newtio0.c_oflag = 0;
    newtio0.c_lflag = 0;
    newtio0.c_cc[VMIN]=0;
    newtio0.c_cc[VTIME]=05;     //for polling

    tcflush(fd232[port], TCIFLUSH);
    tcsetattr(fd232[port],TCSANOW,&newtio0);
    fcntl(fd232[port], F_SETFL, 0);
   }


//RS485:  unused
  if( port == 1 ) {
   fd232[port] = open(RS232_MODEMDEVICE1, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
   if (fd232[port] <0) {perror(RS232_MODEMDEVICE1); exit(-1); }
    else {printf(RS232_MODEMDEVICE1); printf("   opened, port= %d \n",port);}
    tcgetattr(fd232[port],&oldtio1); /* save current port settings */
    /* set new port settings for canonical input processing */
    newtio1.c_cflag = RBAUDRATE | CS8 | CLOCAL | CREAD ;
    newtio1.c_iflag = IGNPAR ;                                  //ignor parity errors
    newtio1.c_oflag = 0;
    newtio1.c_lflag = 0;
    newtio1.c_cc[VMIN]=0;
    newtio1.c_cc[VTIME]=1;     //for polling

    tcflush(fd232[port], TCIFLUSH);
    tcsetattr(fd232[port],TCSANOW,&newtio1);
    fcntl(fd232[port], F_SETFL, 0);
   }


//RS232:  unused
  if( port == 2 ) {
   fd232[port] = open(RS232_MODEMDEVICE2, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
   if (fd232[port] <0) {perror(RS232_MODEMDEVICE2); exit(-1); }
    else {printf(RS232_MODEMDEVICE2); printf("   opened, port= %d \n",port);}
    tcgetattr(fd232[port],&oldtio2); /* save current port settings */
    /* set new port settings for canonical input processing */
    newtio2.c_cflag = RS232_BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio2.c_iflag = IGNPAR ;                                  //ignor parity errors
    newtio2.c_oflag = 0;
    newtio2.c_lflag = 0;
    newtio2.c_cc[VMIN]=0;
    newtio2.c_cc[VTIME]=1;     //for polling

    tcflush(fd232[port], TCIFLUSH);
    tcsetattr(fd232[port],TCSANOW,&newtio2);
    fcntl(fd232[port], F_SETFL, 0);
   }


//RS485:  unused
  if( port == 3 ) {
   fd232[port] = open(RS232_MODEMDEVICE3, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
   if (fd232[port] <0) {perror(RS232_MODEMDEVICE3); exit(-1); }
    else {printf(RS232_MODEMDEVICE3); printf("   opened, port= %d \n",port);}
    tcgetattr(fd232[port],&oldtio3); /* save current port settings */
    /* set new port settings for canonical input processing */
    newtio3.c_cflag = RBAUDRATE | CS8 | CLOCAL | CREAD ; 
    newtio3.c_iflag = IGNPAR ;                                  //ignor parity errors
    newtio3.c_oflag = 0;
    newtio3.c_lflag = 0;
    newtio3.c_cc[VMIN]=0;
    newtio3.c_cc[VTIME]=1;     //for polling

    tcflush(fd232[port], TCIFLUSH);
    tcsetattr(fd232[port],TCSANOW,&newtio3);
    fcntl(fd232[port], F_SETFL, 0);
   }

   return 0 ;

 }


 int flush_RS232( int port ) {
  int           res2,res3;
  unsigned char buf[2050];

//flush everything
  res3 =0;
morestringflush:
  ioctl(fd232[port], FIONREAD, &res2);   //get $ of bytes available in RS232 
  if(res2 > 0 ) {
   if(res2 > 2048) res2 = 2048;
   res3 += read(fd232[port],buf ,res2);
   buf[res3]=0;
   goto morestringflush;
  }
  return(res3);
 }



 int write_RS232( int port, unsigned char* chbuf, int many ) {
  int           res;

//output the string
  res = write(fd232[port],chbuf,many);
  if (res != many)
  {                                               
     printf("transmission problems (write_RS232) port %d\n",port);
     return -111 ;                                    
  }                                                   
  return(res);
 }



 int read_RS232( int port, unsigned char* chbuf, int many ) {
  int           j, res0, res1, res2, res3;

//get a string
  res3=0;
  for(j=0;j<25;j++) {
   ioctl(fd232[port], FIONREAD, &res2);   //get $ of bytes available in RS232
   if(res2 > 0 ) {
    res1  = many - res3;
    if(res2 < res1) res1 = res2;
    res0 = read(fd232[port],chbuf,res1);
    if(res0 < 0 ) { printf("Receive problems (read_RS232) port %d\n",port); return(res0);}
    res3 += res0;
    if(res3 >= many) return (res3);
   }          
   usleep(100000);
  }                        
  return (res3);
 }
