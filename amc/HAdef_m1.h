  #include "AMCconst.h"
  #include "AMCpanel.h"

  #include <termios.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <math.h>
  #include <sys/time.h>
  #include <sys/signal.h>
  #include <sys/types.h>
  #include <asm/ioctls.h>

  #define n8 8         //for the difinition of the arrays
  #define n16 17       //must be twice n8 +1; for the difinition of the arrays
  #define n4000 15000  //length of echo, echoalt
  #define nport 8      //the actual number of active ports

  int fd[n16], fdalt[n16], fduscm;        // return code from open
  static unsigned char cmdframe[20];

//RS232
  int fd232[4];                 // return code from open
  struct termios oldtio0, newtio0;
  struct termios oldtio1, newtio1;
  struct termios oldtio2, newtio2;
  struct termios oldtio3, newtio3;

  struct timeval  *tv, atv;
  struct timezone *tz, atz;
  int  *word;
  unsigned char byte[4];

  #define green 0
  #define red   1
  #define npanx 17
  #define npany 17
  #define FALSE 0
  #define TRUE 1
  #define init_step 7777

//left right top bottom: index
  #define nix 200
  #define nax 650
  #define niy 180
  #define nay 490
  #define MY_WIDTH  800
  #define MY_HEIGHT 600

  #define CAMERA_X 344.0
  #define CAMERA_Y 239.2

  #define nboxmax 100
        //#boxes per port in virtual chain
  int   nbox[n8];
        //panel number in chain virtual positionwise
  int   abox[nboxmax][n8];    //abox[position][chain]
  int   actv[nboxmax][n8]; //instaled?
        //#lasers per port, #actuators per port
  int   nlas[n8], nactu[n8];
        //addresses according to cable topology
  int   adact[n8][nboxmax],panum[n8][nboxmax];
  int   adlas[n8][nboxmax];
        //panel x,y coordinates from panel number
  int   ipan[600], jpan[600];
        //panel x,y coordinates
  int   icpan[nboxmax][n8], jcpan[nboxmax][n8];
        //x position of panel
  int   ixpos[nboxmax][n8];
        //y position of panel
  int   iypos[nboxmax][n8];
        //reversed
  int   revers[nboxmax][n8];
        //installed >0
  int   instal[nboxmax][n8];
        //x, y reference position of Laser
  double Lsrefx[nboxmax][n8], Lsrefy[nboxmax][n8];
        //x, y motor: intersept of fitted line
  double axis[2][nboxmax][n8];
        //x, y motor: slope of fitted line
  double slope[2][nboxmax][n8];
        //x, y motor: conversion factor pixel <-> motor steps
  double cnvrs[2][nboxmax][n8];
        //consequtive panel number given by panel coordinates
  int   ij2npan[npanx][npany];
        //consequtive panel number given by address of actuator/box
  int   actaddr2npan[0x500], boxaddr2npan[0x500];
        //x or y given by address of actuator
  int   itfromactaddr[0x500];
        //laser positions (pixel) from calibration
  double pointarray[300][2][100];                      
        //motor positions (steps) from calibration
  double steparray[300][2][100];                       
        //motor positions (soll) for calibration
  int   calposx[100], calposy[100];

        //motor position by driver: x-, y-motor: steps
  int   motorx[2][nboxmax][n8];
        //motor position exspected: x-, y-motor: steps
  int   motexx[2][nboxmax][n8];

        //laser is on (red), off(green)
  int   lasersemaphor;
        //pixel picture with LED, not Laser
  float d0[MY_WIDTH][MY_HEIGHT];                       
        //pixel picture with LED+BG, not Laser
  float dd[MY_WIDTH][MY_HEIGHT];                       
        //pixel picture with LED+BG and Laser
  float d1[MY_WIDTH][MY_HEIGHT];                       
        //pixel picture only BG for Laser
  float db[MY_WIDTH][MY_HEIGHT];                       
//float  laswid[nboxmax][n8];
  static long sec0;
  static long porttimer[n8][2];
  static long portdtimer[n8][2];
  static unsigned char status1[n8], status2[n8];
  static int nconverge =0, n_sndcmd=0;
  static int n_lasn =0, n_lasf=0, n_movst=0, n_movnd=0;

//getspot section
//static int thl[306], ths[306]; //actually -50 to 256
         //actual position of LEDi ledpos[x/y][i]
  float  ledpos[2][4], ledpos0[2][4];                  
         //actual/last laser position of this panel (x/y,box,port)
  float  laspos[2][nboxmax][n8], laspos0[2][nboxmax][n8];

  double AMC_scroll;
  long   AMC_waitmove;        // in milllisecs
  long   AMC_waitmove0;       // in milllisecs: start of waiting time
  long   AMC_endtimemove;     // in milllisecs: end_time of move

//#include "amcmotor_m2.inc"
//#include "haroutines_m2.inc"


