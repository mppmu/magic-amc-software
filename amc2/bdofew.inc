//bdofew.inc

int bdofew(icmd, dtend, chpoint, gchpoint, npoint, nk )
 int icmd, nk;
 int chpoint[n8][n150], gchpoint[n8][n150], npoint[n8];
 long dtend;
{
 int ipoint[n8], nbyte, idx, alt, ip, iip, retcod, iok, round, k, wtansw_cnt;
 unsigned char sentframe[8][50];
 int  idebug, i2;
 unsigned char debug[2560];
 ushort   crcword;
 unsigned char value[8];


 log_write(fdisk, "bdofew\n", &i2);

 k=gettimeofday( tv, tz);                 //define start time
 tp0=atv.tv_usec;
 sp0=atv.tv_sec-AMCtimeoff;
 tp1=0;
 sp1=0;




 for(k=0;k<n8;k++) ipoint[k]=0;

 round=0;
nextround:
 AMC_check_gui( 0, -1.);
 iok=0;

 for (k=0;k<nk;k++) {
more:
    if (ipoint[k]>=npoint[k]) goto endloop;  //all done for this chain: poiter is at end
    if ( gchpoint[k][ipoint[k]] > 0 ) { ipoint[k]++; goto more; }
    iok++;
    j=chpoint[k][ipoint[k]];

//  prepare frame and sending of it
    idx=2;  //idx  =it (elsewhere); 0/1 for motors, here: 2, for box
    fill_send_data( &nbyte, value, cmdbf[icmd][7], idx, pb[j]);
    for (i=0;i<nbyte;i++)  cmdbf[icmd ][8+i]= value[i];

    cmdbf[icmd][2]=xxcntr[j][0];          //fill in address into frame
    cmdbf[icmd][3]=xxcntr[j][1];

    crcword = 0;                         //calculate CRC
    for (i=2;i<cmdbf[icmd][6]+7;i++) crcword = CRC16( cmdbf[icmd][i], crcword);

    (*word)=crcword;                     //fill in CRC into frame
    cmdbf[icmd][cmdbf[icmd][6]+7]=byte[0];
    cmdbf[icmd][cmdbf[icmd][6]+8]=byte[1];



    alt=(*pb[j]).portflg[idx];
    if(alt != 0) alt=1;
    ip =(*pb[j]).port[alt][idx]; if(ip>=n8) ip++;

    globalnech1a=0;
    globalnech1b=0;

    gchpoint[k][ipoint[k]]=-1;
    send_command( ip, cmdbf[icmd], pb[j] );
    if ( (globalnech1a > 2) && (globalnech1b == 0) ) {
       (*pb[j]).portflg[idx]=1;
    }
    wtansw_cnt=0;

wait_now_ans:
    if ( icmd == cmdrdth  ) usleep(100000);
    retcod=get_response1( ip, lbuf, &res, resin[icmd]);
    if ( AMC_check_gui( ii, AMC_scroll) != 0) goto end;
    wtansw_cnt++;
    if (retcod >= 0) {
       if (bread_store_answer( k, globallbuf, pb[j]) >= 0) gchpoint[k][ipoint[k]] = 1;
    } else {
       retcod=get_response1alt( ip, lbuf, &res, resin[icmd]);
       if (retcod >= 0) {
          if (bread_store_answer( k, globallbuf, pb[j]) >= 0) gchpoint[k][ipoint[k]] = 1;
       } else if(wtansw_cnt<3) {
          cmdbf[cmdrsnd][2]=xxcntr[j][0];          //fill in address into frame
          cmdbf[cmdrsnd][3]=xxcntr[j][1];
          crcword = 0;                         //calculate CRC
          for (i=2;i<cmdbf[cmdrsnd][6]+7;i++) crcword = CRC16( cmdbf[cmdrsnd][i], crcword);
          (*word)=crcword;                     //fill in CRC into frame
          cmdbf[cmdrsnd][cmdbf[cmdrsnd][6]+7]=byte[0];
          cmdbf[cmdrsnd][cmdbf[cmdrsnd][6]+8]=byte[1];
          usleep(10000);
          send_command( ip, cmdbf[cmdrsnd], pb[j] );
          goto wait_now_ans;
       } else {
//       here we have gchpoint still set to '-1'
       }
    }
    ipoint[k]++;

endloop: ;
 } //k-loop

//usleep( 500);
do_sleep:
 i=gettimeofday( tv, tz);                //get real time of this action
 tp2=atv.tv_usec           -tp0;         // usec
 sp2=atv.tv_sec-AMCtimeoff -sp0;         // sec
 if(tp2<0) { sp2--; tp2+=1000000; }
 i=tp2-tp1;
 if(i<0) i=i+1000000;
 if(i < 500) goto do_sleep;
 round++;
 // if((iok>0) && (round<ntodomin+10)) goto nextround;
 if((iok>0) && (round<4)) goto nextround;      // not too many iterations.....
 


 AMC_check_gui( 0, -1.);

 //get rest of echoes and answers
 for(k=0;k<nk;k++){
  if(npoint[k]>0) {
   ioctl(fd[k], FIONREAD, &res1);   //get $ of bytes to flush
   if(res1>0) {
    if(res1>4095) res1=4095;
    res2=read(fd[k], globallbuf, res1);
    idebug=0;
    sprintf(&debug[idebug],"Bc  : port=%2d, read %7d ",k,res2);
    idebug +=28;
    for(i=0;i<imin(30,res2);i++) {
     sprintf(&debug[idebug],"%2.2x ", globallbuf[i] );
     idebug += 3;
    }
    sprintf(&debug[idebug],"\n\0" );
    log_write(fdisk, debug, &i2);
   }
   ioctl(fdalt[k], FIONREAD, &res1);   //get $ of bytes to flush
   if(res1>0) {
    if(res1>4095) res1=4095;
    res2=read(fdalt[k], globallbuf, res1);
    idebug=0;
    sprintf(&debug[idebug],"Bca : port=%2d, read %7d ",k,res2);
    idebug +=28;
    for(i=0;i<imin(30,res2);i++) {
     sprintf(&debug[idebug],"%2.2x ", globallbuf[i] );
     idebug += 3;
    }
    sprintf(&debug[idebug],"\n\0" );
    log_write(fdisk, debug, &i2);
   }
  }
 }
end: ;
}
