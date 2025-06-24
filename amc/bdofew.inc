//bdofew.inc

int bdofew(icmd, dtend, chpoint, gchpoint, npoint, nk )
 int icmd, nk;
 int chpoint[n8][n150], gchpoint[n8][n150], npoint[n8];
 long dtend;
//nk  number of ports/chains
{
//extern long endtime[n200],tanfa,tend, abcount[8];
 int ipoint[n8], nbyte, idx, alt, ip, iip, retcod, iok, round, k, wtansw_cnt;
 unsigned char sentframe[8][50];
 int  idebug, i2;
 unsigned char debug[2560];
 ushort   crcword;
 unsigned char value[8];


 sprintf(&debug[0],"bdofew entered icmd=%d, %x\n\0",icmd,cmdbf[icmd][6] );
 log_write(fdisk, debug, &i2);
 printf("%s",debug);

 k=gettimeofday( tv, tz);                 //define start time
 tp0=atv.tv_usec;
 sp0=atv.tv_sec-AMCtimeoff;
 tp1=0;
 sp1=0;
 printf("start time %ldsec %ldusec\n" ,sp0,tp0);
                                                                                      

// buffer flushed at beginning of "send_command"


 for(k=0;k<n8;k++) ipoint[k]=0;

 round=0;
nextround:
 AMC_check_gui( 0, -1.);
 iok=0;
 for(k=0;k<nk;k++){
more:
  if(ipoint[k]>=npoint[k]) goto endloop;  //all done for this chain: poiter is at end
  if( gchpoint[k][ipoint[k]] > 0 ) { ipoint[k]++; goto more; }
  iok++;
  j=chpoint[k][ipoint[k]];


//  prepare frame and sending of it
//nbyte must be cmdbf[icmd][6]-1
  idx=2;  //idx  =it (elsewhere); 0/1 for motors, here: 2, for box
 sprintf(&debug[0],"bdofew:box addr %2.2x%2.2x  idx %d, ipoint=%d, j=%d\n"
      ,xxcntr[j][1],xxcntr[j][0],idx,ipoint[k],j);
 log_write(fdisk, debug, &i2);
  fill_send_data( &nbyte, value, cmdbf[icmd][6], idx, pb[j]);
  for(i=0;i<nbyte;i++)  cmdbf[icmd ][7+i]= value[i];

  cmdbf[icmd][2]=xxcntr[j][0];          //fill in address into frame
  cmdbf[icmd][3]=xxcntr[j][1];

  crcword = 0;                         //calculate CRC
  for(i=2;i<cmdbf[icmd][5]+6;i++) crcword = CRC16( cmdbf[icmd][i], crcword);

  (*word)=crcword;                     //fill in CRC into frame
  cmdbf[icmd][cmdbf[icmd][5]+6]=byte[0];
  cmdbf[icmd][cmdbf[icmd][5]+7]=byte[1];



  alt=(*pb[j]).portflg[idx];
  if(alt != 0) alt=1; 
  ip =(*pb[j]).port[alt][idx]; if(ip>=n8) ip++;

  globalnech1a=0;
  globalnech1b=0;
  gchpoint[k][ipoint[k]]=-1;
  send_command( ip, cmdbf[icmd], pb[j] );
  if( (globalnech1a > 2) && (globalnech1b == 0) ) {
   (*pb[j]).portflg[idx]=1;
   printf("broken chain %d %d, %2.2x\n",(*pb[j]).port[0][idx], (*pb[j]).port[1][idx], (*pb[j]).addr[idx]);
                                      //   put_logfile(2, 0, title );
  }
  wtansw_cnt=0;

wait_now_ans:
  if( icmd == cmdresb  ) usleep(800000);
  if( icmd == cmdrdth  ) usleep(100000);
  retcod=get_response1( ip, lbuf, &res, resin[icmd]);
  printf("response \n");
  if(  AMC_check_gui( ii, AMC_scroll) != 0) goto end;
  wtansw_cnt++;
  if(retcod >= 0) {
   if(bread_store_answer( k, globallbuf, pb[j]) >= 0) gchpoint[k][ipoint[k]] = 1;
  } else {
   retcod=get_response1alt( ip, lbuf, &res, resin[icmd]);
   if(retcod >= 0) { if(bread_store_answer( k, globallbuf, pb[j]) >= 0) gchpoint[k][ipoint[k]] = 1;
   if(  AMC_check_gui( ii, AMC_scroll) != 0) goto end;
   } else if(wtansw_cnt<3) {
    cmdbf[cmdrsnd][2]=xxcntr[j][0];          //fill in address into frame
    cmdbf[cmdrsnd][3]=xxcntr[j][1];
    crcword = 0;                         //calculate CRC
    for(i=2;i<cmdbf[cmdrsnd][5]+6;i++) crcword = CRC16( cmdbf[cmdrsnd][i], crcword);
    (*word)=crcword;                     //fill in CRC into frame
    cmdbf[cmdrsnd][cmdbf[cmdrsnd][5]+6]=byte[0];
    cmdbf[cmdrsnd][cmdbf[cmdrsnd][5]+7]=byte[1];
    usleep(10000);
    printf("resend  delta= %lds %6.6ldus\n" ,sp2,tp2 );
    send_command( ip, cmdbf[cmdrsnd], pb[j] );
    goto wait_now_ans;
   }
   else {
    (*pb[j]).pan_stat=STAT_ERR;
    sameboxstat(pb[j], STAT_ERR);
    if(gchpoint[k][ipoint[k]] < 0 ) (*pb[j]).box_stat++ ;             //AB 25.7.09
    printf("bdx/y %d/%d stat %d %d %d %d\n", (*pb[j]).ix,  (*pb[j]).iy,  (*pb[j]).pan_stat,
    k, ipoint[k], gchpoint[k][ipoint[k]] );
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
 if(tp2<0) {
  sp2--; tp2+=1000000;
 }
 i=tp2-tp1;
 if(i<0) i=i+1000000;
 if(i < 500) goto do_sleep;
 round++;
 if((iok>0) && (round<ntodomin+10)) goto nextround;
 

 printf("b: end time  delta= %lds %6.6ldus\n" ,sp2,tp2 );
 //sprintf(text,"end time  delta= %ds %6.6dus\n" ,sp2,tp2 );
 //log_write(fdisk, text, &ntext);

 AMC_check_gui( 0, -1.);

//usleep(300000);

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
 printf("b: got all echoes; exiting bdofew\n" );
end: ;
 return 0;
}
