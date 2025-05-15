#include "AMCconst.h"

#include "dofew.inc"

//------------------------------------------------------------------------

int reset_status_new(unsigned char cleared_bits, double dscrl) {
int i,j,k,l,jj,ll,stat,ibreak,ierr;

gettimeofday( tv, tz);                   //time


if (cleared_bits ==0 ) {
   for (k=0; k<nport; k++)
      for (i=0; i<npoint[k]; i++) {
         j= chpoint[k][i];
         if ( j >= 0 ) {
            l=xact[j];
            if ( gchpoint[k][i] <=0 ) {    //communication error

               (*pa[j]).err_cnt[l]++;
               (*pa[j]).tmp_cnt++ ;
printf("ACT: comm error %d %d %d %d %d\n",j,l,(*pa[j]).err_cnt[0],(*pa[j]).err_cnt[1],(*pa[j]).err_cnt[2]);
            }
            (*pa[j]).cmd_cnt[l]++ ;
            if ( (*pa[j]).tmp_cnt > 0) (*pa[j]).pan_stat   = STAT_ER2 ;
            else                       (*pa[j]).pan_stat   = STAT_OK2 ;
      }  }
} else {     // we might have to repeat query until no longer moving; not error
   for (k=0; k<nport; k++)
      for (i=0; i<npoint[k]; i++) {
         j= chpoint[k][i];
         if ( j >= 0 ) {
            l=xact[j];
            if ( gchpoint[k][i] >0 ) {    //communication ok
//             if ( ((*pa[j]).status[0][1] & 1) == 0 &&
//                  ((*pa[j]).status[1][1] & 1) == 0 ) {  //done !?
               if ( ((*pa[j]).status[0][1] & 3) == 0 ) {  //done !?  //HA 7.12.11, x&y motor!
                  (*pa[j]).tmp_sel  = -1 ;      //deselect for next round
                  (*pa[j]).pan_stat = STAT_OK2 ;
                  (*pa[j]).cmd_cnt[l]++ ;
               } else (*pa[j]).pan_stat = STAT_TDO ;
            } else (*pa[j]).pan_stat = STAT_ER2 ;
}     }  }

if (dscrl>=0) AMC_scroll+= dscrl ;
else          AMC_scroll =-dscrl ;
ibreak=AMC_check_gui( ii, AMC_scroll) ;  //display current status

// setup chain for next subcommand
for (k=0; k<nport; k++) {
   npoint[k]=0;
   for(i=0;i<n700;i++) addr2indx[k][i]=-1;
   for(i=0;i<n150;i++) chpoint[k][i]=-1;
}
for (j=0;j<totact;j++){
   k=xport[j]%8 - xport[j]/8;
   l=xact[j] ;
   chpoint[k][npoint[k]]=-1;
   (*pa[j]).tmp_cnt = 0 ;
   if ( ( (*pa[j]).tmp_sel > 0 ) &&
        ( (*pa[j]).err_cnt[0]+(*pa[j]).err_cnt[1] < AMC_errmax*1.5 ) ) {
//    (*pa[j]).version[l][1]=0;
      byte[0]=xcntr[j][0];
      byte[1]=xcntr[j][1];
      i=(*word);
      if (i>1023 || i<0 ) continue;
      addr2indx[k][i] = npoint[k];
      chpoint[k][npoint[k]++]=j;
   }
}
return(ibreak) ;











}
//------------------------------------------------------------------------

int burst_and_get( int icmd, unsigned char cleared_bits,
                    int steps, int nroundmax, double dscrl )
{
int i, j, k, l, ll, it, sumgch, nech1a, nech1b, ibreak;
int i1, j1, k1, nallact, nallech, nround, ntodo;
int pact[17][17];  
double  dscr2, dscr3 ;
unsigned char regloc, title[80] ;
char qstr[1000];
 AMCframe_rcv_0    *rcvframe      ;

/*loops on the following calls:
 burst(icmd,  )           sends commands, collects echos (1&2)
 check_echo(  )           finds pointers to frames in echo buffer
 read_store_answer( )     analyzes the frames

*/






burst_clear();
AMC_waitmove=0;
AMC_endtimemove =0;

for(k=0;k<nport;k++) {
 abcount[k]=0;
 echpnt[k]=echaltpnt[k]=0;
}


ntodo=0;
for(i=0;i<nport;i++) ntodo+=npoint[i]; 
for(i=0;i<nport;i++) for(j=0;j<n150;j++) gchpoint[i][j]=0;
ilast=0;                                 //preset index of slot_timer modified latest
iset=-1;                                 //set latest_channel_processed to 'none'

//snprintf(qstr,120,"ACT: %d %s ; #=%d",icmd,cmdtxt[ cmdbf[icmd][7] ],ntodo);
  printf(         "ACT: %d %s ; #=%d bit=%d\n",icmd,cmdtxt[ cmdbf[icmd][6] ],ntodo,cleared_bits);
snprintf(qstr,120,"ACT: %d %s ; #=%d",icmd,cmdtxt[ cmdbf[icmd][6] ],ntodo);
put_logfile(LOG_DB6, 0,qstr );
//put_logfile(LOG_DB6, 0, cmdtxt[ cmdbf[icmd][7] ] );

k=gettimeofday( tv, tz);                 //define start time
tp0=atv.tv_usec;
sp0=atv.tv_sec-AMCtimeoff;
tp1=0;
sp1=0;

dscr3 = 0. ;      //scrollbar update during indiv. actuator moves
dscr2 = dscrl ;   //scrollbar update at end
if (icmd == cmdncnt || icmd == cmdcntr || icmd == cmdmvx ) {
   dscr3 = dscr2/(ntodo+2) ;
   dscr2 = 2.*dscr3 ;
}

if(ntodo <= 2*ntodomin) { 
 globalnech1a=0;  
 globalnech1b=0;
 dofew(icmd, dtend, chpoint,gchpoint,npoint,nport,dscr3 );
 goto endsub ;
}


//actuators

(*word)=steps;

nround=0;
nextround:


dtend=abs(steps)*1.3+3.+5*nround;          //delta_itme: |steps|/freq*1000 = 'ms' || freq=750
burst(icmd, dtend, chpoint,gchpoint,npoint,nport,dscr3 );       //$$


//========================================================================

//check the frames received (stored in "echo.echoalt")

fncact=cmdbf[icmd][6];                         //actual function
cntact=resin[icmd] - 8 ;                       //actual count


sumgch=0;
check_echo(echpnt1, nechpnt1, echpnt2, nechpnt2, 
           echaltpnt1, nechaltpnt1, echaltpnt2, nechaltpnt2,
           fncact, cntact, gchpoint, cleared_bits, &sumgch );

for(i=0;i<17;i++) for(j=0;j<17;j++) pact[i][j]=0;

//========================================================================

// not working, since no altport possible for the moment
//check broken line
for(k=0;k<nport;k++) for(i=0;i<npoint[k];i++) {
 j=chpoint[k][i];
 for(it=0;it<3;it++) {
  nech1a= nech1b= 0;
  nech1a+=(*pa[j]).ech1_cnt[it][0];
  nech1b+=(*pa[j]).ech1_cnt[it][1];
  if( (nech1a > 2) && (nech1b == 0) ) {
   (*pa[j]).portflg[it]=1;
  }
 }
}


//========================================================================


for(k=0;k<nport;k++) for(i=0;i<npoint[k];i++) {
 if(gchpoint[k][i]>9) {                    //gchpoint > 9 dont need echoes; not executed commands, but ok
  j=chpoint[k][i];
  pact[(*pa[j]).ix+8][(*pa[j]).iy+8]++;
 }
}


for(k=0;k<nport;k++) {
 for(i=0;i<nechpnt2[k];i++) {
  j1=echpnt2[k][i];
//rcvframe   = &echo[k][j1];
  rcvframe   = &echo[k][j1+1];
  k1=((*rcvframe).src_addr) ;
//  k1=((*rcvframe).src_addr)-0x8000 ;

//printf("%2.2x %2.2x %2.2x %2.2x %2.2x \n"
//,echo[k][j1],echo[k][j1+1],echo[k][j1+2],echo[k][j1+3],echo[k][j1+4]);
//printf("burst_and_get: k1=%2.2x\n",k1);

  if(k1<0 || k1> n700) continue;
  i1=addr2indx[k][k1];
  if(i1<0) goto illegaddr;
  j=chpoint[k][i1];

//  printf("burst_and_get: x=%d, y=%d\n",(*pa[j]).ix+8,(*pa[j]).iy+8);
  pact[(*pa[j]).ix+8][(*pa[j]).iy+8]++;
  read_store_answer( k, &echo[k][j1], pa[j]);

illegaddr: ;
 }

 for(i=0;i<nechaltpnt2[k];i++) {
  j1=echaltpnt2[k][i];
  rcvframe   = &echoalt[k][j1+1];
//  k1=((*rcvframe).src_addr)-0x8000 ;
  k1=((*rcvframe).src_addr) ;
  if(k1<0 || k1> n700) continue;
  i1=addr2indx[k][k1];
  if(i1<0) goto illegaddr1;

  j=chpoint[k][i1];
  pact[(*pa[j]).ix+8][(*pa[j]).iy+8]++;

  read_store_answer( k, &echoalt[k][j1], pa[j]);
illegaddr1: ;
 }
}

nallech=0;
for(i=0;i<17;i++) for(j=0;j<17;j++) {
 if(pact[i][j]>0) nallech++;
 if(pact[i][j]>1) nallech++;
}

j=0;
j1=0;
nallact=0;
for(k=0;k<nport;k++) {
 nallact+=npoint[k];
 ipoint[k]=0;
}


ilast=0;                                 //preset index of slot_timer modified latest
iset=-1;                                 //set latest_channel_processed to 'none'
dscr3=0 ;

if( (nallact > nallech) & (nround < imin(nroundmaxmax,nroundmax)) ) { 
 nround++;
 if( (icmd ==cmdncnt) || (icmd ==cmdcntr) ) sleep(2);

 goto nextround;
}



endsub:
//information about success is stored in   gchpoint[k][i]    ( >0 is ok ; <=0 is communic. error )
// call breset_status() to setup error-counters and prepare list for next sub-command

ibreak=reset_status_new(cleared_bits, dscr2) ;

return(ibreak);

}


