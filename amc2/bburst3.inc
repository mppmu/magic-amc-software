//bburst.inc

//echo buffers
//unsigned char echo[8][4000], echoalt[8][4000]; //echo[port][frame#]
//int echpnt[8],echaltpnt[8], res1, res2;

//address management
//actuators
//int chpoint[n8][n150],npoint[n8];
//unsigned char xcntr[65*8][2]; //actuators: addresses
//int xport[65*8];
//boxes
//unsigned char xxcntr[65][2]; //boxes
//int           xxport[65];
//int bchpoint[n8][n150],bnpoint[n8];
//
//the address of an actuator in chain k is: xcntr[j][0/1]
//                                   where: j=chpoint[k][i];  0 <= i=ipoint[k] <npoint[k]
//                                  
//                                  
//                                  




int bburst(icmd, dtend, chpoint,gchpoint,npoint, nk )
 int icmd, nk;
 int chpoint[n8][n150], gchpoint[n8][n150], npoint[n8];
 long dtend;
{

 int ipoint[n8], nbyte, idx, idebug, i2, portoralt, ihelp;
 unsigned char sentframe[8][50];
 unsigned char sentframe8[50];
 unsigned char debug[2560];
 ushort   crcword;
 unsigned char value[8];


 //bursting
 for(k=0;k<nport;k++) {
  abcount[k]=ipoint[k]=0;
  for(i=0;i<npoint[k];i++) {
   j=chpoint[k][i];
   (*pa[j]).ech1_cnt[2][0]=0;            //clear 1a echo counter (box)
   (*pa[j]).ech1_cnt[2][1]=0;            //clear 1b echo counter (box)
  }
 }

 for(i=0;i<n200;i++)  {
  endtime[i]=-1;                          //preset slot_timers
  kendtime[i]=-1;                         //preset port pointers
}
 ilast=0;                                 //preset index of slot_timer modified latest
 iset=-1;                                 //set latest_channel_processed to 'none'
  nmaxcurr=410.;                          //max current available


 idx=2;    
 k=gettimeofday( tv, tz);                 //define start time
 tp0=atv.tv_usec;
 sp0=atv.tv_sec-AMCtimeoff;
 tp1=0;
 sp1=0;




//AMC_scroll= 1. ;
ii= 0 ;
nextround:

if( AMC_check_gui( ii, AMC_scroll) != 0) goto end;
                              
 iok=0;                                   //no work in any chain to be done
 for(k=0;k<nk;k++){

  i=gettimeofday( tv, tz);                //get real time of this action
  tp2=atv.tv_usec           -tp0;         // usec
  sp2=atv.tv_sec-AMCtimeoff -sp0;         // sec
  if(tp2<0) {
   sp2--; tp2+=1000000;
  }
  tanfa=1000*sp2+tp2/1000;                 //start_time in 'ms'

check_end_port:
  if(ipoint[k]>=npoint[k]) goto endloop;  //all done for this chain: poiter is at end
  if( gchpoint[k][ipoint[k]] > 0 ) { ipoint[k]++; goto check_end_port; }
  iok++;                                  //there is a chain with work_pending
  j=chpoint[k][ipoint[k]];


  portoralt=(*pb[j]).port[0][idx];
  if((*pb[j]).portflg[idx] != 0) portoralt=(*pb[j]).port[1][idx];
  if( portoralt>=n8 ) portoralt++;


  tend=tanfa+dtend;                        //end_time
  for(ii=ilast;ii<nmaxcurr+ilast;ii++) {  //go through 'slot_list' starting 1 after the last entry
   i=ii%nmaxcurr;
   if(tanfa > endtime[i]) {
    ilast=i;
    goto iscurrent;
   }
  }
  goto endloop;


iscurrent:


//send command
    if(abcount[k] <=0) {
     gchpoint[k][ipoint[k]]=-1;
     ipoint[k]++;
     endtime[ilast++]=tend;              //put end_time into slot_list; update pointer to it

//  prepare frame and sending of it



//nbyte must be cmdbf[icmd][6]-1
     fill_send_data( &nbyte, value, cmdbf[icmd][7], idx, pb[j]);
     for(i=0;i<nbyte;i++)  cmdbf[icmd ][8+i]= value[i];

     cmdbf[icmd][2]=xxcntr[j][0];          //fill in address into frame
     cmdbf[icmd][3]=xxcntr[j][1];

     crcword = 0;                         //calculate CRC
     for(i=2;i<cmdbf[icmd][6]+7;i++) crcword = CRC16( cmdbf[icmd][i], crcword);

     (*word)=crcword;                     //fill in CRC into frame
     cmdbf[icmd][cmdbf[icmd][6]+7]=byte[0];
     cmdbf[icmd][cmdbf[icmd][6]+8]=byte[1];

//send frame




//HA 16.11.09
     sentframe8[0]=0;
     for(i=0;i<cmdbf[icmd][6]+9;i++) sentframe[k][i]=cmdbf[icmd][i] ;
     for(i=0;i<cmdbf[icmd][6]+9;i++) sentframe8[i+1]=cmdbf[icmd][i] ;
     sentframe8[i+1]=0;
     res = write(fd[portoralt],&sentframe8[0],cmdbf[icmd][6]+9+2);
//HA 16.11.09       end


     abcount[k]= AMCnmintime;            //start distance timer for this chain
     iset=k;                             //keep in mind the number of the chain processed

     idebug=0;
     sprintf(&debug[idebug],"bWB :j=%3d, port=%d ; ",j,portoralt); idebug +=21;
     for(i=0;i<imin((2558-idebug)/3,cmdbf[icmd][6]+9);i++) {
      sprintf(&debug[idebug],"%2.2x ", cmdbf[icmd][i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
    }  //if(abcount[k] <=0)

//  else AMC_check_gui( 0);

endloop:                                 //if there has been no free slot, we jumped to here
    if(tp2 < tp1 ) {                     //prepare calculation of "elapsed time": (tp2-tp1)
     tp1=tp1-1000000;                    //(tp2-tp1) time of this action minus time of last one
     ++sp1;                              //in usec.
    }
    for(i=0;i<nport;i++)
      if(i!=iset) abcount[i]-=(tp2-tp1); //decrement distance timer, exept the one just processed
    iset=-1;                             //preset "last processed chain" to none

    tp1=tp2;                             //"actual tiime" is "previous time" for next action
    sp1=sp2;                             //


} //k-loop
    usleep(5000);                       //"each" chain got a frame; wait for acknowledge

 sprintf(&debug[0],"bburst3:begin 2nd k-loop %d\n\0",sp0 );
 log_write(fdisk, debug, &i2);


for(k=0;k<nk;k++){



//check if there are echos of the command and/or responses from the actuators/boxes
//read data and store it in "echo_array"

    ioctl(fd[k], FIONREAD, &res1);    //get # of bytes received by port
    if(res1>0) {
     if(res1>1024) res1=1023;
     res2=read(fd[k],&echo[k][echpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bEB :j=%3d, port=%d ; ",j,k); idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echo[k][echpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
     if(res2>0) echpnt[k]+=res2;
    }
    ioctl(fdalt[k], FIONREAD, &res1); //get # of bytes received by alternate port
    if(res1>0) {
     if(res1>1024) res1=1023;
     res2=read(fdalt[k],&echoalt[k][echaltpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bEBa:j=%3d, port=%d ; ",j,k); idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echoalt[k][echaltpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
     if(res2>0) echaltpnt[k]+=res2;
     ihelp=0;
     for(ii=5;ii<res2;ii++) {  
      for(i=ii;i>ii-6;i--) {
       if(echoalt[k][echaltpnt[k]+i-res2] != sentframe[k][i] )  goto failed;
       ihelp++;
      }
      abcount[k]=-1; 
      ihelp=0;


      if(ipoint[k]<npoint[k]) goto iscurrent;   //          new!


failed: ;
      ihelp=0;
     }
    }


} //k-loop

if(iok>0) {
do_sleep:
 i=gettimeofday( tv, tz);                //get real time of this action
 tp3=atv.tv_usec           -tp0;         // usec
 sp3=atv.tv_sec-AMCtimeoff -sp0;         // sec
 if(tp3<0) {
  sp3--; tp3+=1000000;
 }
 i=tp3-tp1;
 if(i<0) i=i+1000000;
 if(i < 500) goto do_sleep;
 goto nextround;
}


AMC_check_gui( 0, -1.);

end:
usleep(20000);                  
//get rest of echoes and answers
for(k=0;k<nk;k++){
 if(npoint[k] > 0) {
    ioctl(fd[k], FIONREAD, &res1);   //get $ of bytes to flush
    if(res1>0) {
     if(res1>1024) res1=1023;                          //HA 1.9.08
     res2=read(fd[k],&echo[k][echpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bRB : port=%2d, rd %2d ",k,res2);
     idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echo[k][echpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
     if(res2>0) echpnt[k]+=res2;
    }
    ioctl(fdalt[k], FIONREAD, &res1);   //get $ of bytes to flush
    if(res1>0) {
     if(res1>1024) res1=1023;                          //HA 1.9.08
     res2=read(fdalt[k],&echoalt[k][echaltpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bRBa: port=%2d, rd %2d ",k,res2);
     idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echoalt[k][echaltpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
     if(res2>0) echaltpnt[k]+=res2;
    }
  }
 }
 i=gettimeofday( tv, tz);                //get real time of this action
 tp3=atv.tv_usec           -tp0;         // usec
 sp3=atv.tv_sec-AMCtimeoff -sp0;         // sec
 if(tp3<0) {
  sp3--; tp3+=1000000;
 }

}
