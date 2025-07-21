//dofew.inc

int dofew(icmd, dtend, chpoint,gchpoint,npoint, nk )
 int icmd, nk;
 int chpoint[n8][n150], gchpoint[n8][n150], npoint[n8];
 long dtend;
//nk  number of ports/chains
{
 int ipoint[n8], nbyte, idx, alt, ip, iip, retcod, iok, round, k, wtansw_cnt;
 int  idebug, i2, iflag;
 unsigned char debug[2560], lstr[200];
 unsigned char sentframe[8][50];
 ushort   crcword;
 unsigned char value[8];
 long tend;


 log_write(fdisk, "dofew\n", &i2);

 i2=0;
 for(k=0;k<nk;k++) i2+=npoint[k];
 dtend = 0;


 k=gettimeofday( tv, tz);                 //define start time
 tp0=atv.tv_usec;
 sp0=atv.tv_sec-AMCtimeoff;
 tp1=0;
 sp1=0;




 for(k=0;k<n8;k++) ipoint[k]=0;  //set start value for all chains

round=0;
nextround:
 AMC_check_gui( 0, -1.);
 iok=0;
 for(k=0;k<nk;k++){
more:
  if(ipoint[k]>=npoint[k]) goto endloop;  //all done for this chain: pointer is at end
  if( gchpoint[k][ipoint[k]] > 0 ) { ipoint[k]++; goto more; }
  iok++;
  j=chpoint[k][ipoint[k]];

//  prepare frame and sending of it
//nbyte must be cmdbf[icmd][6]-1
  byte[0]=xcntr[j][0];          
  byte[1]=xcntr[j][1];
  byte[2]=0;
  byte[3]=0;
  i=*word-0x8000;
  idx=itfromactaddr[i];

//------
//some command_data_combinations are not executed
 if( icmd==cmdmvx) {
  if(abs( (*pa[j]).move_by[idx]) < AMCstepmin ) {
   gchpoint[k][ipoint[k]]=10;
   ipoint[k]++; 
   goto more;
  }
 }

 if( icmd==cmdwtlsw )  {
  if( (*pa[j]).endsw_lwl[idx] == (*pa[j]).rndsw_lwl[idx] ) {
   gchpoint[k][ipoint[k]]=11;
   ipoint[k]++; 
   goto more;
  } else {
         sprintf(lstr,"lsw bad %d/%d stat %d (%d,%d)", k,ipoint[k],11,(*pa[j]).ix,(*pa[j]).iy);
         put_logfile(LOG_SVR, 0, lstr );
  }
 }

 if( (icmd==cmdwtusw) && (idx=0) )  {
  if( (*pa[j]).endsw_upl[idx] == (*pa[j]).rndsw_upl[idx] ) {
   gchpoint[k][ipoint[k]]=12;
   ipoint[k]++; 
   goto more;
  } else {
         sprintf(lstr,"usw bad %d/%d stat %d (%d,%d)", k,ipoint[k],11,(*pa[j]).ix,(*pa[j]).iy);
         put_logfile(LOG_SVR, 0, lstr );
  }
 }
//-----

  tend=fill_send_data( &nbyte, value, cmdbf[icmd][7], idx, pa[j] );
  if(tend > dtend) dtend=tend;
  AMC_waitmove=dtend;
  i=gettimeofday( tv, tz);                //get real time of this action
  tp2=atv.tv_usec           -tp0;         // usec
  sp2=atv.tv_sec-AMCtimeoff -sp0;         // sec
  if(tp2<0) { sp2--; tp2+=1000000; }
  AMC_waitmove0=1000*sp2+tp2/1000;                 //start_time in 'ms'
  if(AMC_endtimemove <= AMC_waitmove0 + AMC_waitmove) AMC_endtimemove=AMC_waitmove0 + AMC_waitmove;

  for(i=0;i<nbyte;i++)  cmdbf[icmd ][8+i]= value[i];

  cmdbf[icmd][2]=xcntr[j][0];          //fill in address into frame
  cmdbf[icmd][3]=xcntr[j][1];

  crcword = 0;                         //calculate CRC
  for(i=2;i<cmdbf[icmd][6]+7;i++) crcword = CRC16( cmdbf[icmd][i], crcword);

  (*word)=crcword;                     //fill in CRC into frame
  cmdbf[icmd][cmdbf[icmd][6]+7]=byte[0];
  cmdbf[icmd][cmdbf[icmd][6]+8]=byte[1];

  alt=(*pa[j]).portflg[idx];
  if(alt != 0) alt=1; 
  ip =(*pa[j]).port[alt][idx]; if(ip>=n8) ip++;

  globalnech1a=0;
  globalnech1b=0;


  gchpoint[k][ipoint[k]] = -1;
  send_command( ip, cmdbf[icmd], pa[j] );


if(PORTFLAGSETTING > 0) {
  if( (globalnech1a > 2) && (globalnech1b == 0) ) {
   (*pa[j]).portflg[idx]=1;
   sprintf(lstr,"broken chain %d %d, %2.2x ",(*pa[j]).port[0][idx], (*pa[j]).port[1][idx], (*pa[j]).addr[idx]);
   put_logfile(LOG_SVR, 0, lstr );
  }
}
  wtansw_cnt=0;

wait_now_ans:

  if( icmd == cmdr4ms  ) usleep(AMC_dofew10000 +100);
  if( icmd == cmdrdth  ) usleep(AMC_dofew100000);       //$$



  retcod=get_response1( ip, lbuf, &res, resin[icmd]);
  if(  AMC_check_gui( ii, AMC_scroll) != 0) goto end;
  wtansw_cnt++;
  iflag=-9;
  if( (retcod >= 0) && (res == resin[icmd]-12) )  {

   iflag=read_store_answer( k, globallbuf, pa[j]);

   if( (cmdbf[icmd][7] == cmdbf[cmdrdmt][7]) && ( ( (*pa[j]).status[idx][1]&1) != 0)
     && (wtansw_cnt<12) ) {
    usleep(AMC_dofew200000);
    send_command( ip, cmdbf[icmd], pa[j] );
    goto wait_now_ans;
   }
  }

  sprintf(&debug,"iflag %d, ipoint[k]=%d, gchpoint= %d\n",iflag, ipoint[k], gchpoint[k][ipoint[k]] );
  log_write(fdisk, debug, &i2);

  if( iflag == 0 ) {
   gchpoint[k][ipoint[k]] = 1;
  } else {
//try alt channel
   retcod=get_response1alt( ip, lbuf, &res, resin[icmd]);
   if(  AMC_check_gui( ii, AMC_scroll) != 0) goto end;
   if( (retcod >= 0) && (res == resin[icmd]-12) ) {
    iflag=read_store_answer( k, globallbuf, pa[j]); }
   if( iflag == 0 ) {
    gchpoint[k][ipoint[k]] = 1;
   } else if( ( (*pa[j]).status[idx][0]&4) != 0     ) {    //command rejected
    if( (cmdbf[icmd][7] == cmdbf[cmdrdmt][7]) && (wtansw_cnt<12) ) {
     usleep(AMC_dofew200000);
     send_command( ip, cmdbf[icmd], pa[j] );
     goto wait_now_ans;
    }
   } else if(wtansw_cnt<3) {
    cmdbf[cmdrsnd][2]=xcntr[j][0];          //fill in address into frame
    cmdbf[cmdrsnd][3]=xcntr[j][1];
    crcword = 0;                         //calculate CRC
    for(i=2;i<cmdbf[cmdrsnd][6]+7;i++) crcword = CRC16( cmdbf[cmdrsnd][i], crcword);
    (*word)=crcword;                     //fill in CRC into frame
    cmdbf[cmdrsnd][cmdbf[cmdrsnd][6]+7]=byte[0];
    cmdbf[cmdrsnd][cmdbf[cmdrsnd][6]+8]=byte[1];
    usleep(AMC_dofew10000);
    send_command( ip, cmdbf[cmdrsnd], pa[j] );
    goto wait_now_ans;
   }
   else {
// here gchpoint is always <0
      sprintf(lstr,"FW COM (%d,%d) astat %d gch %d", (*pa[j]).ix,(*pa[j]).iy, (*pa[j]).act_stat, gchpoint[k][ipoint[k]] );
      put_logfile(LOG_ERR, 0, lstr );
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
 if(tp2<0) {  sp2--; tp2+=1000000; }
 i=tp2-tp1;
 if(i<0) i=i+1000000;
 if(i < 500) goto do_sleep;
 round++;
// if((iok>0) && (round<2*(ntodomin+10) )) goto nextround;
 if( (iok>0) && (round<4) ) goto nextround;
 


 AMC_check_gui( 0, -1.);


 //get rest of echoes and answers
 for(k=0;k<nk;k++){
  if(npoint[k]>0) {
   ioctl(fd[k], FIONREAD, &res1);   //get $ of bytes to flush
   if(res1>0) {
    if(res1>4095) res1=4095;
    res2=read(fd[k], globallbuf, res1);
    idebug=0;
    sprintf(&debug[idebug],"c   : port=%2d, rd %2d ",k,res2);
    idebug +=21;
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
    sprintf(&debug[idebug],"ca  :rd %2d, port=%2d;  ",res2, k);
    idebug +=21;
    for(i=0;i<imin(30,res2);i++) {
     sprintf(&debug[idebug],"%2.2x ", globallbuf[i] );
     idebug += 3;
    }
    sprintf(&debug[idebug],"\n\0" );
    log_write(fdisk, debug, &i2);
   }
  }
 }
end:;
}
