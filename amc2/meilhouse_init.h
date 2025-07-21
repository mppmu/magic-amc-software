
//#define MODEMDEVICE    "/dev/ttyS4"

// meilhouse device
    /* open the device to be non-blocking (read will return immediatly) */
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (fd <0) {perror(MODEMDEVICE); exit(-1); }

    /* install the signal handler before making the device asynchronous */

    tcgetattr(fd,&oldtio); /* save current port settings */
    /* set new port settings for canonical input processing */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR ;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VMIN]=0;
    newtio.c_cc[VTIME]=1;     //for polling
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
    fcntl(fd, F_SETFL, 0);            //blocking mode
//  fcntl(fd, F_SETFL, FNDELAY);      //non-blocking mode
//  ioctl(fd, FIONREAD, &j);   //get $ of bytes available

