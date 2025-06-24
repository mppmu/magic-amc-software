#include <strings.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/socket.h>      
#include <netinet/in.h> 
#include <netdb.h>      
#include <asm/ioctls.h>
#include <sys/stat.h>

#include "AMCgui.h"
#include "AMCframe.h"
#include "HAdef_m1.h"
#include "burst_def.h"
#include "command_vardef.h"
#include "pwrsockets_def.h"
#include "uscm.h"

 extern AMCpanel *pa[600];
 extern AMCpanel *pb[600];
 extern AMCpanel panel[17][17] ;

 int log_write(int fdisk, char text[],int *ntext);
 int absleep( );
#include "command_function_pool.h"             //subroutine for commands for single addresses
#include "magpos2.inc"
#include "amcmotor_m1.inc"    //most of it can be removed ?!!!!
#include "read_frame.inc"
#include "send_frame.inc"
#include "bread_frame.inc"
#include "burst5.inc"
#include "bburst3.inc"
#include "burst_and_get.inc"
#include "bburst_and_get.inc"
#include "haroutines_m1.inc"
#include "uscm.inc"
#include "check_echo.inc"


//-----------------------------------------------------------------------------------


long get_tstamp() {

 i=gettimeofday( tv, tz);                //get real time of this action
 tp6=atv.tv_usec           -tp0;         // usec
 sp6=atv.tv_sec-AMCtimeoff -sp0;         // sec
 if(tp6<0) { sp6--; tp6+=1000000; }
 tstamp=1000*sp6+tp6/1000;                 //stamp_time in 'ms'
 return tstamp ;
}


 int log_write(int fdisk, char text[],int *ntext) {
     char chstamp[20];

     sprintf(chstamp,"%10ld ", get_tstamp() );
     write(fdisk, chstamp, 11 );

     for(*ntext=0;(*ntext)<310;(*ntext)++) if(text[*ntext] == '\0') break;
     return( write(fdisk,text,*ntext) );
 }


 int absleep( ) {
  int ii =-1,  ibreak;
  long tstamp;

  tstamp= get_tstamp();
  ibreak=0;
next:
  if( tstamp > AMC_endtimemove) goto end;
  usleep(10000);
  if( (ibreak = AMC_check_gui( ii, AMC_scroll)) != 0) goto end;
  tstamp= get_tstamp();
  goto next;
end: 
  return(ibreak);

 }


int ausleep(int i) {         //i: time to wait in usec
  struct timeval  *tv, atv;
  struct timezone *tz, atz;
  long mainusec, mainsec;
  long mainusec0, mainsec0;
  int ip;
  float x, y;

  tv=&atv;
  tz=&atz;
  gettimeofday( tv, tz);
  mainusec0=atv.tv_usec;
  mainsec0=atv.tv_sec ;
  x=10.;
  ip=1;
loop:
   y=log(sqrt(x*x+ip*ip));
   x=exp(y);
   if(x > 1000000.) x=10;
   gettimeofday( tv, tz);
   mainusec=atv.tv_usec;
   mainsec=atv.tv_sec ;
  if( (mainsec-mainsec0)*1000000+(mainusec-mainusec0) < i) goto loop;
  return(0);
}





//==================================================================================

int  AMC_init(AMCpanel panel[17][17])
{

 char text[330], text1[330];
 int ierr, res, ntext;

 extern AMCpanel *pa[600] ;
 extern AMCpanel *pb[600] ;

#include "HAmaindef_m1.h"

#include "hamain_m1.inc"


    return 0 ;
}





//==================================================================================


int AMC_exec_pwr(int type, int soll[32], int ist[32] )
{
  int i,j,k,l,it,jt,istat, iist[32], retcd ;
  unsigned char so[4], is[4], aid, aidall, regloc;

  for (i=0; i<32; i++) {
     ist[i]=soll[i] ;
  }

  return 0 ;        //for time being claim power as requested





  printf("\nint AMC_exec_pwr: type %d\n",type);
  i=gettimeofday( tv, tz);                //get real time of this action
  tp1=atv.tv_usec;
  sp1=atv.tv_sec-AMCtimeoff;
  printf("%ld s, %ld us\n",sp1,tp1);
  for(i=0;i<32;i++) iist[i]=0;
  retcd=9;
  if( (type >= 0) || (USCMreg[2]==0) ) retcd=allstatus_USCM(  );
  if( retcd >= 0) {
   for(i=0;i<4;i++) {
    for(j=0;j<8;j++) {
     k=8*i+j;
     aid= ( 1 << j );
     iist[k]=0;
     if( (USCMreg[i] & aid) > 0 ) iist[k]=1;
    }
   }
  }
  if( type < 0 ) {
   printf("registers: %2.2x,%2.2x,%2.2x,%2.2x\n"
          ,USCMreg[0],USCMreg[1],USCMreg[2],USCMreg[3]);
   return(retcd);       //fast query: returns the present USCMreg values
  }

//get_prompt();

  printf("t:%d ",type); for(i=31;i>-1;i--) printf("%1d",soll[i]); printf("\n");
  printf("i:%d ",type); for(i=31;i>-1;i--) printf("%1d", ist[i]); printf("\n");
  printf("i:%d ",type); 
   for(i=31;i>23;i--) printf("%1d",iist[i]);
   printf(" ");
   for(i=23;i>15;i--) printf("%1d",iist[i]);
   printf(" ");
   for(i=15;i>7;i--) printf("%1d",iist[i]);
   printf(" ");
   for(i=7;i>-1;i--) printf("%1d",iist[i]);
  printf("\n");

  if(retcd < 0) { for(i=31;i>-1;i--) ist[i]=iist[i]; return(retcd);}

  aidall=0;
  aid=0;
  for(i=0;i<4;i++) {
//any difference between ist and soll in this byte?
   for(j=0;j<8;j++) {
    k=8*i+j;
    if(iist[k] != soll[k]) aid++;
   }
  }
  if(aid ==0) goto getsta;           // no

  for(i=0;i<4;i++) {
   aidall+=aid;
   aid=0;
//find bits, which should remain set, and set this pattern
   for(j=0;j<8;j++) {
    k=8*i+j;
    if(iist[k]  >  0) aid = aid | ( 1 << j );     //incl or    //to be kept, if not to
    if(soll[k] <= 0)  aid = aid & (~( 1 << j ));  //and        //   be cleared
   }
   so[i]=aid;
   if( USCMreg[i] != so[i])
    if(type == 1) retcd=set_USCM( so[i], i );   //clearing can be done in one go

//find bits, which should additionally become set, and set this pattern
   for(j=0;j<8;j++) {
    k=8*i+j;
    if((soll[k] > 0) && (iist[k] <= 0) ) {     //has to be set
printf("iist[%d]=%d, soll[%d]=%d, aid=%d\n",k,iist[k],k,soll[k],aid);
     aid = aid | ( 1 << j );
     if( (i < 2) && ((j%2) == 1) ) {           //set chain after chain (2 bits per call)
      so[i]=aid;
      if(type == 1) {
       it=gettimeofday( tv, tz);               //get real time of this action
       tp2=atv.tv_usec - tp1;
       sp2=atv.tv_sec-AMCtimeoff - sp1;
       if(tp2<0) {tp2+=1000000; sp2--;}
       printf("bset1 %ld s, %ld ms\n",sp2,tp2);

//       retcd=set_USCM( so[i], i );
       retcd=set_USCM0( so[i], i );

       it=gettimeofday( tv, tz);               //get real time of this action
       tp2=atv.tv_usec - tp1;
       sp2=atv.tv_sec-AMCtimeoff - sp1;
       if(tp2<0) {tp2+=1000000; sp2--;}
       printf("bset2 %ld s, %ld ms\n",sp2,tp2);
      }
     }
    }
   }
   so[i]=aid;
//   if(i > 1) if(type == 1) retcd=set_USCM( so[i], i );
   if(i > 0) if(type == 1) retcd=set_USCM( so[i], i );

  }
  if(aidall != 0) retcd=allstatus_USCM(  );
getsta: ;
  for(i=0;i<4;i++) is[i]=USCMreg[i];

  printf(
  "AMC_exec_pwr t:%d, s: %2.2x %2.2x %2.2x %2.2x, i: %2.2x %2.2x %2.2x %2.2x\n",
    type, so[0], so[1], so[2], so[3]
        , is[0], is[1], is[2], is[3]);

  for(i=0;i<4;i++) {
   aid=is[i];
   for(j=0;j<8;j++) {
    k=8*i+j;
    ist[k] = 0;
    if( (aid & ( 1 << j )) > 0) ist[k] =1;
   }
  }

  printf("t:%d, ist: ",type); for(i=31;i>-1;i--) printf("%1d",ist[i]); 
  printf("\n");

  for(i=0;i<32;i++) soll[i]=iist[i];

  get_prompt();

  i=gettimeofday( tv, tz);                //get real time of this action
  tp2=atv.tv_usec - tp1;
  sp2=atv.tv_sec-AMCtimeoff - sp1;
  if(tp2<0) {tp2+=1000000; sp2--;}
  printf("exec_pwr %ld s, %ld us\n",sp2,tp2);
  return(retcd);
}







//==================================================================================

int AMC_exec_cmd(AMCpanel panel[17][17], int cmd, int ncmd, int ix, int iy )
{
  int i,j, j0, j1, ja,k,k1,it,jt,istat, retcodm ;
  unsigned char lbuf[256], text[128];
  int actcmd;
  AMCpanel* ptr;

    int  l, nled, nact=0, flrd, ibreak;
    static long mainusec0, mainsec0;
    long mainusec, mainsec;
    int  anyaction, portaction[nport], ok, iport;
    int  ntext;



  for (i=0; i<17; i++)
     for (j=0; j<17; j++)
        if (panel[i][j].pan_stat == STAT_TDO ) panel[i][j].pan_sel = 1 ;
        else                                   panel[i][j].pan_sel = 0 ;


  //printf("========== start cmd  %d\n",cmd);
  ibreak=0;
  ptr=&panel[0][0];

  switch ( cmd ) {

//------------------------------------------------------------------------------------------------

//   case ( CMD_ACAD ):
//end:printf("   ending check addressess \n");
//    break;


//------------------------------------------------------------------------------------------------

   case ( CMD_NONE ):
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_INIT ):
    initall_inc(0);
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_INIF ):
    initall_inc(1);
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_INFF ):
    initall_inc(2);
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_CNTR ):
     for (i=0; i<17; i++)
        for (j=0; j<17; j++)
           if (panel[i][j].pan_sel !=0 ) panel[i][j].pc_mot[0]=panel[i][j].pc_mot[1]=0;
    initall_inc(-1);
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_MOVE ):
     for (i=0; i<17; i++)
        for (j=0; j<17; j++) 
           if (panel[i][j].pan_sel !=0 ) {
              panel[i][j].pc_mot[0]=panel[i][j].pc_moveto[0] ;
              panel[i][j].pc_mot[1]=panel[i][j].pc_moveto[1] ; }
    initall_inc(-2);
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_QUERY):
    initall_inc(-3);
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_LSON ):
//printf("laser on\n");
//printf("laser on: panstat=%2.2x\n", panel[2][13].pan_stat);
    initall_inc(-11);
//printf("laser on1: panstat=%2.2x\n", panel[2][13].pan_stat);
//printf("laser on done\n");
    break;

//------------------------------------------------------------------------------------------------

   case ( CMD_LOFF ):
//printf("laser off\n");
    initall_inc(-12);
//printf("laser off done\n");
    break;

//------------------------------------------------------------------------------------------------

   default:
    printf(" ---------  unknonw command ---------------- \n");
    break ;
  }       //switch

  printf("end of  cmd  %d\n",cmd);
  return ibreak ;
}

