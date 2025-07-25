#include "AMCconst.h"

void sameboxstat(AMCpanel *pb, int stat) {

 int i,i2,k;
 unsigned debug[2560];
 AMCpanel *p;

 for(k=0;k<3;k++) {
  i= (*pb).samebox[k];
  if(i >= 0) {
   p=&panel[ipan[i]+8][jpan[i]+8];
   (*p).pan_stat=stat;
  }
 }
}

void sameboxversion(AMCpanel *pb ) {

 int i,i2,j,k;
 unsigned debug[2560];
 AMCpanel *p;

 for(k=0;k<3;k++) {
  i= (*pb).samebox[k];
  if(i >= 0) {
   p=&panel[ipan[i]+8][jpan[i]+8];
   for(j=0;j<3;j++) {
    (*p).version[j][0]=(*pb).version[j][0];
    (*p).version[j][1]=(*pb).version[j][1];
   }
  }
 }
}


#include "bdofew.inc"

//--------------------------------------------------

int breset_status_new(double dscrl) {
int i,j,k,l,jj,ll,stat, ibreak;

gettimeofday( tv, tz);                   //time
jj=0;
for (k=0; k<nport; k++)
   for (i=0; i<bnpoint[k]; i++) {
      j= bchpoint[k][i];
      if ( j >= 0 ) {
         l=2 ;  // always a box ....
         if ( bgchpoint[k][i] <=0 ) {    //communication error
printf("BOX: comm error %d %d\n",j,l);
            (*pb[j]).err_cnt[l]++;
            (*pb[j]).pan_stat   = STAT_ER1 ;
         } else {
            (*pb[j]).pan_stat   = STAT_OK1 ;
         }
         (*pb[j]).cmd_cnt[l]++ ;
         (*pb[j]).time[0]=atv.tv_usec;
         (*pb[j]).time[1]=atv.tv_sec  - AMCtimeoff;
      }
   }


if (dscrl>=0) AMC_scroll+= dscrl/2. ;
else          AMC_scroll =-dscrl    ;
ibreak=AMC_check_gui( ii, AMC_scroll) ;  //display current status

// setup chain for next subcommand
for (k=0; k<nport; k++) {
   bnpoint[k]=0;
   for(i=0;i<n700;i++) baddr2indx[k][i]=-1;
   for(i=0;i<n150;i++) bchpoint[k][i]=-1;
}
for (j=0;j<totbox;j++){
   k=xxport[j]%8 - xxport[j]/8;
   l=2 ;
   bchpoint[k][bnpoint[k]]=-1;
   if ( ( (*pb[j]).tmp_sel > 0 ) &&
        ( (*pb[j]).err_cnt[l] < AMC_errmax   ) ) {
      (*pb[j]).version[l][1]=0;
//    byte[0]=xxcntr[j][0];
//    byte[1]=xxcntr[j][1];
//    i=(*word)-0x8800;
      i=xxcntr[j][0];
      if (i>1023 || i<0 ) continue;
      baddr2indx[k][i] = bnpoint[k];
      bchpoint[k][bnpoint[k]++]=j;
   }
}

return(ibreak) ;
}
//--------------------------------------------------

int bburst_and_get( int icmd, unsigned char cleared_bits,
     int steps, int nroundmax, double dscrl   ) {

long btp0,btp1,bsp0,bsp1;
int i, j, k, l, it, sumgch, ibreak;
int i1, j1, k1, nallact, nallech, nround, ntodo;
int pact[17][17];  
char qstr[1000];
 AMCframe_rcv_0    *rcvframe ;
 AMCframe_rcv_1    *rcvframe1; //
/* unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
   unsigned char dst_addr ;       //destination address
   unsigned char box_addr ;       //source: box address
   unsigned char drv_addr ;       //source: driver address
*/


burst_clear();

for(k=0;k<nport;k++) {
   abcount[k]=0;
   echpnt[k]=echaltpnt[k]=0;
}


nroundmax=2;  // was 6, but do not try too often (usually, bad is bad...)

ntodo=0;
for(i=0;i<nport;i++) ntodo+=bnpoint[i];  
for(i=0;i<nport;i++) for(j=0;j<n150;j++) bgchpoint[i][j]=0;
ilast=0;                                 //preset index of slot_timer modified latest
iset=-1;                                 //set latest_channel_processed to 'none'

//snprintf(qstr,120,"BOX: %s ; #=%d",cmdtxt[ cmdbf[icmd][7] ],ntodo);
  printf(         "BOX: %s ; #=%d",cmdtxt[ cmdbf[icmd][6] ],ntodo);
snprintf(qstr,120,"BOX: %s ; #=%d",cmdtxt[ cmdbf[icmd][6] ],ntodo);
put_logfile(LOG_DB6, 0,qstr );
//put_logfile(LOG_DB6, 0, cmdtxt[ cmdbf[icmd][7] ] );

k=gettimeofday( tv, tz);                 //define start time
tp0=atv.tv_usec;
sp0=atv.tv_sec-AMCtimeoff;
btp0=atv.tv_usec;
bsp0=atv.tv_sec-AMCtimeoff;
tp1=0;
sp1=0;

if (ntodo <= ntodomin) {
   bdofew(icmd, dtend, bchpoint,bgchpoint,bnpoint,nport );
   goto endsub ;
}


//boxes

(*word)=steps;

nround=0;              
nallact = ntodo;
nallech = 0 ;

nextround:

dtend=abs(steps)*1.3+3.+5*nround;   //delta_itme: |steps|/freq*1000 = 'ms' || freq=750
bburst(icmd, dtend, bchpoint,bgchpoint,bnpoint,nport );


//========================================================================

//check the frames received (stored in "echo.echoalt")

fncact=cmdbf[icmd][6];                         //actual function
cntact=resin[icmd] - 8 ;                       //actual count



i=gettimeofday( tv, tz);                //get real time of this action
btp1=atv.tv_usec           -btp0;         // usec
bsp1=atv.tv_sec-AMCtimeoff -bsp0;         // sec
if (btp1<0) { bsp1--; btp1+=1000000; }

sumgch=0;
check_echo(echpnt1, nechpnt1, echpnt2, nechpnt2, 
           echaltpnt1, nechaltpnt1, echaltpnt2, nechaltpnt2,
           fncact, cntact, bgchpoint, cleared_bits, &sumgch );

for(i=0;i<17;i++) for(j=0;j<17;j++) pact[i][j]=0;



 

sumgch=0;
for (k=0;k<nport;k++) {
   for (i=0;i<nechpnt2[k];i++) {
      j1=echpnt2[k][i];
      rcvframe   = (AMCframe_rcv_0 *) &echo[k][j1];
      rcvframe1  = (AMCframe_rcv_1 *) &echo[k][j1];
      k1= (*rcvframe1).drv_addr;
      if (k1<0 || k1>n700) continue;
      i1=baddr2indx[k][k1];
      if (i1<0) goto illegaddr;
      j=bchpoint[k][i1];

      pact[(*pb[j]).ix+8][(*pb[j]).iy+8]++;
      sumgch++;
      bread_store_answer( k, &echo[k][j1], pb[j]); //interpret answer


illegaddr: ;
   }
   for (i=0;i<nechaltpnt2[k];i++) {
      j1=echaltpnt2[k][i];
      rcvframe   = (AMCframe_rcv_0 *) &echoalt[k][j1];
      rcvframe1  = (AMCframe_rcv_1 *) &echo[k][j1];
      k1= (*rcvframe1).drv_addr;
      if (k1<0 || k1>n700) continue;
      i1=baddr2indx[k][k1];
      if (i1<0) goto illegaddr1;
      j=bchpoint[k][i1];

      pact[(*pb[j]).ix+8][(*pb[j]).iy+8]++;
      sumgch++;
      bread_store_answer( k, &echo[k][j1], pb[j]); //interpret answer

illegaddr1: ;
   }
}

nallech=0;
for(i=0;i<17;i++) for(j=0;j<17;j++) if(pact[i][j]>0) nallech++;


nallact=0;
for (k=0;k<nport;k++) {
   nallact+=bnpoint[k];
   ipoint[k]=0;
}

ilast=0;                                 //preset index of slot_timer modified latest
iset=-1;                                 //set latest_channel_processed to 'none'

if( (nallact > nallech) & (nround < nroundmax) ) { nround++; goto nextround; }                             




endsub:
//information about success is stored in   bgchpoint[k][i]    ( >0 is ok ; <=0 is communic. error )
// call breset_status() to setup error-counters and prepare list for next sub-command

printf("exiting bburst_and_get; do breset\n");

ibreak = breset_status_new(dscrl) ;

printf("return from bburst_and_get\n");

return(ibreak) ;
}
