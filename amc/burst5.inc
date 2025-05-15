//burst5.inc

//echo buffers
//see burst_def.h   and   HAdef_m1.h

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


void flush_buffers() {
 int k, res1, res2, idebug, i2;
 unsigned char debug[2560];

 for(k=0;k<nport;k++){
flushagain:
   if(nosim == 0)
    aioctl(fd[k], FIONREAD, &res1);   //get # of bytes to flush
   else
    ioctl(fd[k], FIONREAD, &res1);   //get # of bytes to flush

    echpnt[k]=0;
    if(res1>0) {
     if(res1>1024) res1=1023;
     res2= read(fd[k],&echo[k][echpnt[k]],res1);   //all read were "aread"
//----
     idebug=0; j=-2;
     sprintf(&debug[idebug],"bF  :j=%3d, port=%2d; ",j, k ); idebug +=21;
     for(i=0;i<imin(100,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echo[k][echpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
//---
     if(res2>1000) {
      sprintf(debug,"\n\0" );
      log_write(fdisk, debug, &i2);
      goto flushagain;
     }
    }

 }
}

void burst_clear(){
 int k, i;

 for(k=0;k<n8;k++) {
  echpnt[k]=echaltpnt[k]=0;
  for(i=0;i<n4000;i++) echo[k][i]=echoalt[k][i]=0;
 }

}


int burst(icmd, dtend, chpoint, gchpoint,  npoint, nk )
 int icmd, nk;
 int chpoint[n8][n150], gchpoint[n8][n150], npoint[n8];
 long dtend;
{
 int ipoint[n8], ii, nbyte, idebug, i2, it, portoralt, nactcnt;
 unsigned char sentframe[8][50], *regloc;
 unsigned char sentframe8[50];

 unsigned char value[8], debug[2560], lstr[200];
 ushort   crcword;
 double scroll;
 long ddtend;
 int kactactmax, kactact[n8], jumpflag, kk, klast, klast1; //number of active (moving) actuators per chain
 int execution_pending, towait;

 log_write(fdisk, "burst5\n", &i2);
 printf("burst5: icmd=%2.2x\n",cmdbf[icmd][6]);

 //bursting
 for(k=0;k<nport;k++) {
  abcount[k]=ipoint[k]=kactact[k]=0;     //kactact active actuators in chain k
  for(i=0;i<npoint[k];i++) {
   j=chpoint[k][i];
   (*pa[j]).ech1_cnt[0][0]=0;            //clear 1a echo counter (actuator1)
   (*pa[j]).ech1_cnt[1][0]=0;            //clear 1a echo counter (actuator2)
   (*pa[j]).ech1_cnt[0][1]=0;            //clear 1b echo counter (actuator1)
   (*pa[j]).ech1_cnt[1][1]=0;            //clear 1b echo counter (actuator2)
  }
 printf("npoint[%d]=%d\n",k,npoint[k]);
 }

 kactactmax=100;
 if( (icmd==cmdcntr) || 
     (icmd==cmdncnt) ) kactactmax=kactactmax0;



 for(i=0;i<n200;i++)  {
  kendtime[i]=-1;
  endtime[i]=-1;      //preset slot_timers
 }
 ilast=0;                                 //preset index of slot_timer modified latest
 iset=-1;                                 //set latest_chain_processed to 'none'
 if(nmaxcurr<=1) {
  printf("max number of movable actuators too small %d, %f\n", nmaxcurr, macu);
  exit(0);
 }

// nmintime=20000; //usec                  //min time interval between sending for same port


 k=gettimeofday( tv, tz);                 //define start time
 tp0=atv.tv_usec;
 sp0=atv.tv_sec-AMCtimeoff;
 tp1=0;
 sp1=0;
 printf("start time %dsec %dusec\n" ,sp0,tp0);


//AMC_scroll= 1. ;
ii= 0 ;
klast=nk-1;
nextround:
iok=0;                                   //no work in any chain to be done (preset value of iok)

if( AMC_check_gui( ii, AMC_scroll) != 0) goto end;
for(kk=klast+1;kk<=nk+klast;kk++) {
 k=kk%nk;
 jumpflag=0;                             //work possible

next_in_chain:
// printf("ipoint[%d]=%d   npoint[%d]=%d\n" ,k,ipoint[k] ,k,npoint[k]);

 i=gettimeofday( tv, tz);                //get real time of this action
 tp2=atv.tv_usec           -tp0;         // usec
 sp2=atv.tv_sec-AMCtimeoff -sp0;         // sec
 if(tp2<0) {
  sp2--; tp2+=1000000;
 }
 tanfa=1000*sp2+tp2/1000;                 //start_time in 'ms'

//check if there is an item to be done?
 if(ipoint[k]>=npoint[k]) {
  jumpflag++;                             //all done for this chain: pointer is at end
  goto update;                            //just update the actual  queues
 }

 if( gchpoint[k][ipoint[k]] > 0 ) { ipoint[k]++; jumpflag++; } //already done

 j=chpoint[k][ipoint[k]];
 if( j < 0 ) { ipoint[k]++; jumpflag++; goto next_in_chain;} //illegal pointer! why?

 byte[0]=xcntr[j][0];                     //get "it" index from address
 byte[1]=xcntr[j][1];
 byte[2]=0;
 byte[3]=0;
 i2=*word;
 it=0;
 if(i2<0) goto update;                    //illegal address
 if(i2>=0x500) goto update;               //illegal address


//some command_data_combinations are not executed
 if( icmd==cmdmvx) {
  if((abs( (*pa[j]).move_by[0]) < AMCstepmin)
  && (abs( (*pa[j]).move_by[1]) < AMCstepmin) ) {
   gchpoint[k][ipoint[k]]=10;
         sprintf(lstr,"b5x/y %d/%d stat %d (%d,%d)", k,ipoint[k],10,(*pa[j]).ix,(*pa[j]).iy);
         put_logfile(LOG_SVR, 0, lstr );
   (*pa[j]).pan_stat=STAT_OK_;   //--
   ipoint[k]++; 
   if(ipoint[k]<npoint[k]) goto next_in_chain;
   jumpflag++; 
  } else  kactactmax=kactactmax0;
 }

/*
 if( icmd==cmdwtlsw )  {
  printf("do lower endswitch %x, k=%d, j=%d, ipoint=%d\n",(*pa[j]).addr[it],k,j,ipoint[k]);
  if( ((*pa[j]).endsw_lwl[0] == (*pa[j]).rndsw_lwl[0]) 
   && ((*pa[j]).endsw_lwl[1] == (*pa[j]).rndsw_lwl[1]) ) {
   gchpoint[k][ipoint[k]]=11;
         sprintf(lstr,"LSW ok  %d/%d stat %d (%d,%d)", k,ipoint[k],11,(*pa[j]).ix,(*pa[j]).iy);
         put_logfile(LOG_SVR, 0, lstr );
   (*pa[j]).pan_stat=STAT_OK_;   //--
   ipoint[k]++; 
   printf("lower endswitch NOT to be set, alrady ok; k=%d ipoint=%d<%d\n",k,ipoint[k],npoint[k]);
   if(ipoint[k]<npoint[k]) goto next_in_chain;
   jumpflag++; 
  } else {
         sprintf(lstr,"LSW BAD %d/%d stat %d (%d,%d)", k,ipoint[k],11,(*pa[j]).ix,(*pa[j]).iy);
         put_logfile(LOG_SVR, 0, lstr );
   printf("lower endswitch %d, %d\n",(*pa[j]).endsw_lwl[it], (*pa[j]).rndsw_lwl[it]);
  }
 }
*/
/*
 if( (icmd==cmdwtusw) && (it >=0) )  {
  printf("do upper endswitch %x, k=%d, j=%d\n",(*pa[j]).addr[it],k,j);
  if( ((*pa[j]).endsw_upl[0] == (*pa[j]).rndsw_upl[0])
   && ((*pa[j]).endsw_upl[1] == (*pa[j]).rndsw_upl[1]) ) {
   gchpoint[k][ipoint[k]]=12;
         sprintf(lstr,"USW ok  %d/%d stat %d (%d,%d)", k,ipoint[k],12,(*pa[j]).ix,(*pa[j]).iy);
         put_logfile(LOG_SVR, 0, lstr );
   (*pa[j]).pan_stat=STAT_OK_;   //--
   ipoint[k]++; 
   printf("upper endswitch %d, %d\n",(*pa[j]).endsw_upl[it], (*pa[j]).rndsw_upl[it]);
   if(ipoint[k]<npoint[k]) goto next_in_chain;
   jumpflag++; 
   printf("upper endswitch NOT to be set, alrady ok\n");
  } else {
         sprintf(lstr,"USW BAD %d/%d stat %d (%d,%d)", k,ipoint[k],12,(*pa[j]).ix,(*pa[j]).iy);
         put_logfile(LOG_SVR, 0, lstr );
  }
 }
*/

 if(jumpflag <= 0 ) iok++;                //there is a chain with work_pending
 if(kactact[k]>=kactactmax) jumpflag++;   //current limit (10A) for chain k reached
                                          //iok set before, because this one 
                                          //        should be done later

portoralt=(*pa[j]).port[0][0];
if((*pa[j]).portflg[0] != 0) portoralt=(*pa[j]).port[1][0];
if( portoralt >=n8 ) portoralt++;
//printf("portflg=%d, actport=%d\n",(*pa[j]).portflg[0],portoralt);


update:
 tend=tanfa+dtend;                        //end_time
 execution_pending=0;
 towait=0;

//go through 'slot_list' starting 1 after the last entry
 for(ii=ilast;ii<nmaxcurr+ilast;ii++) {  
  i=ii%nmaxcurr;

//if(endtime[i]>0)
//printf("burst5: i=%d, tanfa=%d, endtime[i]=%d, jumpflag=%d, towait=%d\n",
//        i, tanfa, endtime[i], jumpflag, towait);

  if(tanfa > endtime[i]) {                //this command is finished
   if(kendtime[i]<0 || kendtime[i]>nport-1) {     //got flagged/illegal chain_pointer?
    ;
//    printf("burst5: kendtime[%d] wrong!!  %d; k=%d\n",i,kendtime[i],k);
   } else {                               //chain_pointer is legal
//  printf("kactact[ kendtime[i=%d]=%d ]=%d\n",i, kendtime[i], kactact[ kendtime[i] ]);
    kactact[ kendtime[i] ]--;         //chain kendtime[i] has current for one more command
    kendtime[i] = -1;                 //this queue item inactive now
   }
//   printf("is current, cell %d, times %d %d %d, k=%d tp2 %d, abc%d\n"
//      ,i,tanfa,tend,endtime[i],k,tp2,abcount[k]);

   if( jumpflag <= 0 ) {                  //can execute one more command 
    ilast=i;                              //keep pointer to latest entry into slot_list
    towait =  5 ;
    goto iscurrent;                       //go send command
   }
  } else {
   execution_pending++;
   towait =  endtime[i] - tanfa ;

   if(towait > 90000 ) {
    towait = 90000; // 3.11.2011  work around
    printf("endtime[%d]=%d, tanfa=%d ",i, endtime[i], tanfa);
    printf("new towait = %d\n",towait);
   }


  }
 }
 //dont printf("current limit %x/%d t:%d\n",0x100*xcntr[j][1]+xcntr[j][0],xport[j],tanfa );

//checked whole slot_list; did not find a command to be executed
 goto endloop;

iscurrent:                                //try executyng next command


//send command
    if(abcount[k] <=0) {                  //time for acknowledge elapsed?
     gchpoint[k][ipoint[k]]=-1;
     ipoint[k]++;                         //set pointer to next command (of this chain)

//  prepare frame and sending of it


//nbyte must be cmdbf[icmd][5]-1
     byte[0]=xcntr[j][0];          
     byte[1]=xcntr[j][1];
     byte[2]=0;
     byte[3]=0;
     i=*word;         


    
     ddtend=fill_send_data( &nbyte, value, cmdbf[icmd][6],itfromactaddr[i], pa[j] );
//   printf("burst5: AMC_waitmove=%d, ddtend=%d\n",AMC_waitmove,ddtend);
//   if(ddtend > AMC_waitmove) AMC_waitmove = ddtend;
     AMC_waitmove = ddtend;
     i=gettimeofday( tv, tz);                //get real time of this action
     tp2=atv.tv_usec           -tp0;         // usec
     sp2=atv.tv_sec-AMCtimeoff -sp0;         // sec
     if(tp2<0) {
      sp2--; tp2+=1000000;
     }
     AMC_waitmove0=1000*sp2+tp2/1000;                 //start_time in 'ms'
     if(AMC_endtimemove <= AMC_waitmove0 + AMC_waitmove) AMC_endtimemove=AMC_waitmove0 + AMC_waitmove;
                 //AMC_endtimemove: "absleep" waits until time >= AMC_endtimemove


     //dont printf("-----ddtend %d\n",ddtend);
     if( ddtend > 0 ) tend=tanfa+ddtend;                      //end_time
     kendtime[ilast]=k;
     endtime[ilast++]=tend;              //put end_time into slot_list; update pointer to it
     for(i=0;i<nbyte;i++)  cmdbf[icmd ][7+i]= value[i];

     cmdbf[icmd][2]=xcntr[j][0];          //fill in address into frame
     cmdbf[icmd][3]=xcntr[j][1];

     crcword = 0;                         //calculate CRC
     for(i=2;i<cmdbf[icmd][5]+6;i++) crcword = CRC16( cmdbf[icmd][i], crcword);

     (*word)=crcword;                     //fill in CRC into frame
     cmdbf[icmd][cmdbf[icmd][5]+6]=byte[0];
     cmdbf[icmd][cmdbf[icmd][5]+7]=byte[1];

//send frame

printf("burst:crucial write to fd[%d]=%d, fnc= %2.2x\n"
,portoralt,fd[portoralt], cmdbf[icmd][6]);


     kactact[k]++;                       //one more active actuator for this chain
     nactcnt=0;
     for(i=0;i<nk;i++) nactcnt+=kactact[i];
     printf("active actuators in chain k=%d are %d, maxact=%d %d\n",k,kactact[k], nactcnt,nmaxcurr);
     klast1=k;
//1     res = write(fd[portoralt],&cmdbf[icmd][0],cmdbf[icmd][5]+8);
/****
     for(i=0;i<cmdbf[icmd][5]+8;i++) sentframe[k][i]=cmdbf[icmd][i] ;

//next line for test reason; when removed, activate //1 (2 lines before)
     res = write(fd[portoralt],&sentframe[k][0],cmdbf[icmd][5]+8+2);  //added 28.10.09 HA
**/
//HA 16.11.09
     sentframe8[0]=0;
     for(i=0;i<cmdbf[icmd][5]+8;i++) sentframe[k][i]=cmdbf[icmd][i] ;
     for(i=0;i<cmdbf[icmd][5]+8;i++) sentframe8[i+1]=cmdbf[icmd][i] ;
     sentframe8[i+1]=0;
     res = write(fd[portoralt],&sentframe8[0],cmdbf[icmd][5]+8+2);
//   res =awrite(fd[portoralt],&sentframe8[0],cmdbf[icmd][5]+8+2);
//HA 16.11.09       end


     abcount[k]= AMCnmintime;            //start distance timer for this chain
                                         //line free! for response of actuator
     iset=k;                             //keep in mind the number of the chain processed
//   printf("%dk%6.6d$",k,tp2);
     //dont printf("%dk%6.6d$",xport[j],tp2);
     //dont for(i=0;i<cmdbf[icmd][5]+8;i++) printf("%2.2x ", cmdbf[icmd][i] );
     //dont printf("\n");


//debug output to logfile
     idebug=0;
     sprintf(&debug[idebug],"bW  :j=%3d, port=%2d, ",j,portoralt); idebug +=21;
     for(i=0;i<imin((2558-idebug)/3,cmdbf[icmd][5]+8);i++) {
      sprintf(&debug[idebug],"%2.2x ", cmdbf[icmd][i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
    }                                    //end send_command_code-if
    else if( AMC_check_gui( ii, AMC_scroll) != 0) goto end;
//  usleep(AMC_burst25000);

endloop:                                 //if there has been no free slot, we jumped to here
    if(tp2 < tp1 ) {                     //prepare calculation of "elapsed time": (tp2-tp1)
     tp1=tp1-1000000;                    //(tp2-tp1) time of this action minus time of last one
     ++sp1;                              //in usec.
    }
    for(i=0;i<nport;i++)
      if(i!=iset) abcount[i]-=(tp2-tp1); //decrement distance timer, exept the one just processed
    iset=-1;                             //preset "last processed chain" to none

    tp1=tp2;                             //"actual time" becomes "previous time" 
    sp1=sp2;                             //                       for next action


next_k0:;
} //k-loop
klast=klast1;

    usleep(AMC_burst25000);
//    printf("2nd k-loop\n");


//for(k=0;k<nk;k++){
for(k=nk-1;k>=0;k--){

//check if there are echos of the command and/or responses from the actuators/boxes
//read data and store it in "echo_array"
//  usleep(AMC_burst25000);
   if(nosim == 0)
    aioctl(fd[k], FIONREAD, &res1);   //get # of bytes to flush
   else
    ioctl(fd[k], FIONREAD, &res1);   //get # of bytes to flush

    if(res1>0) {
     //dont printf("%d%6.6d$",sp2,tp2);
//     printf(" k %d, IOCTL res1 %d\n",k,res1);
     if(res1>1024) res1=1023;                          //HA 1.9.08
     res2= read(fd[k],&echo[k][echpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bR  :j=%3d, port=%2d, ",j, k); idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echo[k][echpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
     if(res2>0) echpnt[k]+=res2;

     if(echpnt[k]>n4000) {printf("echpnt[%d]=%d/n",k,echpnt[k]); echpnt[k]=n4000-100; }

    }
   if(nosim == 0)
    aioctl(fdalt[k], FIONREAD, &res1); //get # of bytes received by alternate port
   else
    ioctl(fdalt[k], FIONREAD, &res1); //get # of bytes received by alternate port

   //dont printf(" k %d, IOCTLalt res1 %d\n",k,res1);
    if(res1>0) {
     //dont printf("%d%6.6d$",sp2,tp2);
     //dont printf(" k %d, IOCTLalt res1 %d, echaltpnt=%d\n",k,res1,echaltpnt[k]);
     if(res1>1024) res1=1023;                          //HA 1.9.08
     res2= read(fdalt[k],&echoalt[k][echaltpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bRa :j=%3d, port=%2d, ",j, k); idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echoalt[k][echaltpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
//   log_write(fdisk, debug, &i2);
     if(res2>0) echaltpnt[k]+=res2;

     if(echaltpnt[k]>n4000) {printf("echaltpnt[%d]=%d/n",k,echaltpnt[k]); echaltpnt[k]=n4000-100; }

     for(ii=5;ii<res2;ii++) {  
      for(i=ii;i>ii-6;i--) {
       if(echoalt[k][echaltpnt[k]+i-res2] != sentframe[k][i] ) goto failed;
       //dont printf("i %d, e %2.2x, s %2.2x ",i,echoalt[k][echaltpnt[k]+i-res2],sentframe[k][i]);
      }
      abcount[k]=-1; 
      //dont printf("f\n");
      goto next_k;
failed: ;
     }
     //dont printf("\nechoalt, k %d, iset %d  ",k,iset);
     //dont for(i=0;i<res2;i++) printf("%2.2x ", echoalt[k][echaltpnt[k]+i-res2] );
     //dont printf("\n");
    }

next_k:;
} //k-loop

if( (iok>0)  ) {
 goto nextround;
}

if( (iok>0) || ( execution_pending>0) ) {
 towait = imax(towait, 5) ;
// towait =  5 ;    //13.6.09
 if(towait > 5) printf("burst5:waiting %d msec\n",towait);

        printf("burst: begin do_sleep\n");
do_sleep:
 i=gettimeofday( tv, tz);                //get real time of this action
 tp2=atv.tv_usec           -tp0;         // usec
 sp2=atv.tv_sec-AMCtimeoff -sp0;         // sec
 if(tp2<0) {
  sp2--; tp2+=1000000;
 }
 i=(tp2-tp1)/1000 + (sp2-sp1)*1000;

 if(i < towait) {
  if(towait-i > 10) usleep(10000);       //5.11.09 HA.
  goto do_sleep;
 }
 towait =  5 ;
        printf("burst: end do_sleep\n");
 goto nextround;
}

//dont printf("end time  delta= %ds %6.6dus\n" ,sp2,tp2 );
//sprintf(text,"end time  delta= %ds %6.6dus\n" ,sp2,tp2 );
//log_write(fdisk, text, &ntext);

end:
printf("AMC_scroll=%f\n",AMC_scroll);
AMC_check_gui( ii, AMC_scroll);
//get rest of echoes and answers
for(k=0;k<nk;k++){
 if(npoint[k] > 0) {
   if(nosim == 0)
    aioctl(fd[k], FIONREAD, &res1);   //get # of bytes to flush
   else
    ioctl(fd[k], FIONREAD, &res1);   //get # of bytes to flush

    if(res1>0) {
     if(res1>1024) res1=1023;                          //HA 1.9.08
     res2= read(fd[k],&echo[k][echpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bR .:j=%3d, port=%2d, ",j, k); idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echo[k][echpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
     log_write(fdisk, debug, &i2);
     if(res2>0) echpnt[k]+=res2;
    }
   if(nosim == 0)
    aioctl(fdalt[k], FIONREAD, &res1);   //get # of bytes to flush
   else
    ioctl(fdalt[k], FIONREAD, &res1);   //get # of bytes to flush

    if(res1>0) {
     if(res1>1024) res1=1023;                          //HA 1.9.08
     res2= read(fdalt[k],&echoalt[k][echaltpnt[k]],res1);
     idebug=0;
     sprintf(&debug[idebug],"bRa.:j=%3d, port=%2d, ",j, k); idebug +=21;
     for(i=0;i<imin(50,res2);i++) {
      sprintf(&debug[idebug],"%2.2x ", echoalt[k][echaltpnt[k]+i] );
      idebug += 3;
     }
     sprintf(&debug[idebug],"\n\0" );
//   log_write(fdisk, debug, &i2);
     if(res2>0) echaltpnt[k]+=res2;
    }
 }
}

}
