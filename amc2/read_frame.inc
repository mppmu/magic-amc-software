int read_store_answer( int k, unsigned char *echo, AMCpanel *p)
{
 int i, it, i1, j, k1, icase, ihall;

 unsigned char   code;

 AMCframe_rcv_0    *rcvframe      ;
 AMCframe_rcv_b    *rcvframe_b    ;
 AMCframe_rcv_w    *rcvframe_w    ;
 AMCframe_rcv_uw   *rcvframe_uw   ;
 AMCframe_rcv_bw   *rcvframe_bw   ;
 AMCframe_rcv_wb   *rcvframe_wb   ;
 AMCframe_rcv_ww   *rcvframe_ww   ;
 AMCframe_rcv_wwww *rcvframe_wwww ;
 AMCframe_rcv_9w   *rcvframe_9w   ;
  



 rcvframe   = echo;
 if((*rcvframe).src_addr < 0x8800) { //actuator
  k1=((*rcvframe).src_addr)-0x8000 ;
  if( (k1>0x300) || (k1<0) ) {
   return(-3);
  }
  i1=addr2indx[k][k1];
  j=chpoint[k][i1];
  it = itfromactaddr[ k1 ];
  if(it<0 || it>1) return(-4);
 } else {                            //box
  it=2;
 }

 (*p).status[it][0] = (*rcvframe).status[0];    //status1 
 (*p).status[it][1] = (*rcvframe).status[1];    //status2
 
 icase=100;
 code= (char) (*(echo+7) );                 //command
 if ( code == cmdbf[cmdmvx ][7] ||
      code == cmdbf[cmdcntr][7] ||
      code == cmdbf[cmdncnt][7] ||
      code == cmdbf[cmdwtfq][7] ||
      code == cmdbf[cmdwtwc][7] ||
      code == cmdbf[cmdwthc][7] ||
      code == cmdbf[cmdwtmt][7] ||
      code == cmdbf[cmdwtemt][7] ||
      code == cmdbf[cmdpwof][7] ||
      code == cmdbf[cmdpwon][7] ||
      code == cmdbf[cmdwtlsw][7] ||
      code == cmdbf[cmdwtusw][7] ||
      code == cmdbf[cmdnlasn][7] ||
      code == cmdbf[cmdnlasf][7] ||
      code == cmdbf[cmdkill][7] ||
      code == cmdbf[cmdwtemt][7]) icase= 1;
 if ( code == cmdbf[cmdrlas][7])  icase= 2;
 if ( code == cmdbf[cmdsdvn][7])  icase= 3;
 if ( code == cmdbf[cmdrdfq][7])  icase= 4;
 if ( code == cmdbf[cmdrdwc][7])  icase= 5;
 if ( code == cmdbf[cmdrdhc][7])  icase= 6;
 if ( code == cmdbf[cmdsdui][7])  icase= 7;
 if ( code == cmdbf[cmdrdmt][7])  icase= 8;
 if ( code == cmdbf[cmdrdemt][7]) icase= 9;
 if ( code == cmdbf[cmdrdlsw][7]) icase=10;
 if ( code == cmdbf[cmdrdusw][7]) icase=11;
 if ( code == cmdbf[cmdqwdc ][7]) icase=12;
 if ( code == cmdbf[cmdrdth ][7]) icase=13;
 if ( code == cmdbf[cmdr4ms ][7]) icase=14;
 if ( code == cmdbf[cmdrdfr ][7]) icase=15;


 switch( icase) {
//only status
  case(1):
   if ( code == cmdbf[cmdmvx ][7] ) (*p).move_by[it] = 0;
   break;

//one byte (unsigned)
  case(2):
   rcvframe_b = rcvframe;
   break;
  case(3):
   rcvframe_b = rcvframe;
   (*p).version[it][0] = (*rcvframe_b).byte0;    //version
   (*p).actstat[it] &= (! AMCwrngvers);
   if( (*p).version[it][0] < AMCminvers) (*p).actstat[it] |= AMCwrngvers;
   break;

//one unsigned word
//AMCframe_rcv_uw   *rcvframe_freq ;
  case(4):
   rcvframe_uw = rcvframe;
   (*p).freq[it] = (*rcvframe_uw).word0;    //frequency
   if( abs( (*p).freq[it] - AMCfreq ) > 5 ) {
    (*p).actstat[it] |= AMCwrngfrq;
   } else {
    (*p).actstat[it] &= (! AMCwrngfrq);
   }

   break;
  case (5):
   rcvframe_uw = rcvframe;
   (*p).work[it] = (*rcvframe_uw).word0;    //working current
   if( abs( (*p).work[it] - AMCwork*1000 ) > 1 ) {
    (*p).actstat[it] |= AMCwrngwrk;
   } else {
    (*p).actstat[it] &= (! AMCwrngwrk);
   }
   break;
  case (6):                                 //read holding current
   rcvframe_uw = rcvframe;
   (*p).hold[it] = (*rcvframe_uw).word0;    //holding current
   if( abs( (*p).hold[it] - AMChold*1000 ) > 1 ) {
    (*p).actstat[it] |= AMCwrnghld;
   } else {
    (*p).actstat[it] &= (! AMCwrnghld);
   }
   break;
  case (7):                                 //read unique ID
   rcvframe_uw = rcvframe;
   if( (*p).addr[it] != (*rcvframe_uw).word0) {    //unique ID
    (*p).actstat[it] |= AMCwrngID;
   } else {
    (*p).actstat[it] &= (! AMCwrngID);
   }
   break;

//one signed word
//AMCframe_rcv_w   *rcvframe_pos ;
  case (8):                                 //read motor position
   rcvframe_w = rcvframe;
   (*p).act_mot[it] = (*rcvframe_w).word0;    //actual motor position

   if( abs( (*p).act_mot[it] - (*p).pc_mot[it] ) > 2 ) {
    (*p).actstat[it] |= AMCwrngpos;
   } else {
    (*p).actstat[it] &= (! AMCwrngpos);
   }
   break;

  case (9)  :                                 //read motor position from EEprom
   rcvframe_w = rcvframe;
   (*p).act_mot[it] = (*rcvframe_w).word0;    //actual motor position
   break;

//one byte, one unsigned word
//AMCframe_rcv_bw   *rcvframe_ends ;
  case (10) :                                 //read lower endswitch: number and value
   rcvframe_bw = rcvframe;
   (*p).rndsw_lwn[it] = (*rcvframe_bw).num;      //sensor number
   byte[0]=(*rcvframe_bw).val[0];    //sensor level
   byte[1]=(*rcvframe_bw).val[1];    //sensor level
   byte[2]=0;
   byte[3]=0;
   (*p).rndsw_lwl[it] = *word;
   if( (*p).rndsw_lwn[it] != (*p).endsw_lwn[it] ) {
    (*p).actstat[it] |= AMCwrnglwn;
   } else {
    (*p).actstat[it] &= (! AMCwrnglwn);
   }
   if( (*p).rndsw_lwl[it] != (*p).endsw_lwl[it] ) {
    (*p).actstat[it] |= AMCwrnglwl;
   } else {
    (*p).actstat[it] &= (! AMCwrnglwl);
   }
   break;

  case (11) :                                    //read upper endswitch: number and value
   rcvframe_bw = rcvframe;
   (*p).rndsw_upn[it] = (*rcvframe_bw).num;      //sensor number
   byte[0]=(*rcvframe_bw).val[0];    //sensor level
   byte[1]=(*rcvframe_bw).val[1];    //sensor level
   byte[2]=0;
   byte[3]=0;
   (*p).rndsw_upl[it] = *word ;
   if( (*p).rndsw_upn[it] != (*p).endsw_upn[it] ) {
    (*p).actstat[it] |= AMCwrngupn;
   } else {
    (*p).actstat[it] &= (! AMCwrngupn);
   }
   if( (*p).rndsw_upl[it] != (*p).endsw_upl[it] ) {
    (*p).actstat[it] |= AMCwrngupl;
   } else {
    (*p).actstat[it] &= (! AMCwrngupl);
   }
   break;

//one unsigned word, one byte
//AMCframe_rcv_wb   *rcvframe_watch ;
  case (12) :                   //read watchdog counter and source indicator
   rcvframe_wb = rcvframe;
   if( (*p).watsrc[it] > 8 ) (*p).watch0[it] = (*rcvframe_wb).word0;
   (*p).watch1[it] = (*rcvframe_wb).word0 - (*p).watch0[it];
   (*p).watsrc[it] = (*rcvframe_wb).byte0;
   (*p).version[it][1] = (*rcvframe_wb).byte0;  //watch dog source

   break;

//two words
//AMCframe_rcv_ww   *rcvframe_tmp ;
  case (13) :                   //read humidity and temperature
   rcvframe_ww = rcvframe;
   (*p).humi[it] = (*rcvframe_ww).humid;
   (*p).temp[it] = (*rcvframe_ww).temp;
   break;

//fore words
//AMCframe_rcv_wwww   *rcvframe_hall ;
  case (14) :                   //read 4 sensors
   rcvframe_wwww = rcvframe;
   for(i=0;i<4;i++) {
    ihall = (*rcvframe_wwww).hallval[i]/70.;
    ihall=imax(-11111, imin(11111,ihall));
    (*p).hal_val[it][i] = ihall;
   }
   (*p).hal_pos[it] =
       imax( -11111, imin (11111,
        magpos( (*p).hal_val[it], (*p).numpos[it],
                (*p).step[it], (*p).bval[it], (*p).dbds[it] ) )
        - (*p).esw_centr[it]
       );
   break;

//nine words
//AMCframe_rcv_9w     *rcvframe_9    ;
  case (15) :                   //read range and endvalues
   rcvframe_9w = rcvframe;
   (*p).range[it] = (*rcvframe_9w).length;
   for(i=0;i<4;i++) {
    (*p).hal_endl[it][i] = (*rcvframe_9w).lmag[i]/70. ;
    (*p).hal_endu[it][i] = (*rcvframe_9w).umag[i]/70. ;
   }
   break;

  default:

  return(-1);


 }

 gettimeofday( tv, tz);                 //time
 (*p).time[0]=atv.tv_usec;
 (*p).time[1]=atv.tv_sec - AMCtimeoff;
 (*p).ech2_cnt[it][0]++;

 return(0);
}

