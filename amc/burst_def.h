//burst_def.h

#define n200 520
// define n8 8
// define n8 4

#define n150 150
// define nactuators 500

long tp0,tp1,tp2,tp3,tp4,tp5,tp6, sp0,sp1,sp2,sp3,sp4,sp5,sp6;
char text[330], text1[330];

//echo buffers
unsigned char echo[n8][n4000], echoalt[n8][n4000]; //echo[port][byte#]
int           echpnt[n8],      echaltpnt[n8];      //echpnt, echaltpnt: first free locatioin
int res1, res2; 

//address management
//actuators
int ipoint[n8];   //pointer into chpoint for sending (Tx)
#define n700 1024
int chpoint[n8][n150], npoint[n8], totact, addr2indx[n8][n700];
int gchpoint[n8][n150] ;
unsigned char xcntr[75*8][2]; //actuators: addresses
int xport[75*8], xact[75*8];
//boxes
unsigned char xxcntr[75*4][2]; //boxes
int           xxport[75*4];
int bchpoint[n8][n150],bnpoint[n8], totbox, baddr2indx[n8][n700];
int bgchpoint[n8][n150];


int           savexport[75*8], savexxport[75*4];
unsigned char savexcntr[75*8][2], savexxcntr[75*4][2];
int           ssavexport[75*8], ssavexxport[75*4];
unsigned char ssavexcntr[75*8][2], ssavexxcntr[75*4][2];

int iok,iset,ilast,i,ii,j,k, nmaxcurr, nmintime, res;
long abcount[8];
long endtime[n200], tanfa, tend, dtend, tstamp;
int  kendtime[n200];

//# of correct echo1/2 - frames found got from alt port
int  nechpnt1[8],nechpnt2[8],   nechaltpnt1[8],nechaltpnt2[8];
//pointer into echo buffer for frame #i=0<250
int  echpnt1[8][250],echpnt2[8][250],  echaltpnt1[8][250],echaltpnt2[8][250];
int fncact, cntact;

int fdisk, fdisk2;

//
//the address of an actuator in chain k is: xcntr[j][0/1]
//                                   where: j=chpoint[k][i];  0 <= i=ipoint[k] <npoint[k]
//                                  
//                                  
//                                  
//void check_echo(echpnt1[8][100],    nechpnt1[8],
//                echpnt2[8][100],    nechpnt2[8],
//                echaltpnt1[8][100], nechaltpnt1[8],
//                echaltpnt2[8][100], nechaltpnt2[8],
//                int fncact, int cntact ) ;
//void check_echo(echpnt1, nechpnt1, echpnt2, nechpnt2,
//                echaltpnt1, nechaltpnt1, echaltpnt2, nechaltpnt2,
//                fncact, cntact ) ;
void check_echo(echpnt1, nechpnt1, echpnt2, nechpnt2, 
                echaltpnt1, nechaltpnt1, echaltpnt2, nechaltpnt2,
                fncact, cntact, gchpoint, cleared_bits, sumgch );

