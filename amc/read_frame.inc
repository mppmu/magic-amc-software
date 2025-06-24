void sameboxversion(AMCpanel *pb );

int read_store_answer( int k, unsigned char *echo, AMCpanel *p)
{
 int i, it, i1, j, k1, icase, ihall, addr;

 unsigned char   code;

 AMCframe_rcv_0    *rcvframe      ;
 AMCframe_rcv_b    *rcvframe_b    ;
 AMCframe_rcv_bb   *rcvframe_bb   ;
 AMCframe_rcv_w    *rcvframe_w    ;
 AMCframe_rcv_uw   *rcvframe_uw   ;
 AMCframe_rcv_bw   *rcvframe_bw   ;
 AMCframe_rcv_wb   *rcvframe_wb   ;
 AMCframe_rcv_ww   *rcvframe_ww   ;
 AMCframe_rcv_wwww *rcvframe_wwww ;
 AMCframe_rcv_9w   *rcvframe_9w   ;
 AMCframe_rcv_wwwwww *rcvframe_wwwwww ;


// printf("read_store_answer:  \n");

 rcvframe   = (AMCframe_rcv_0 *) (echo+1);

  k1=(*rcvframe).src_addr ;

  if( (k1>0x500) || (k1<0) ) {
   return(-3);
  }
  i1=addr2indx[k][k1];
  j=chpoint[k][i1];
  it = itfromactaddr[ k1 ];

  it=0;

  if(it<0 || it>1) {printf("read_frame:wrong itfromactaddr[%d]=%d\n",k1,it); return(-4);}


 (*p).status[0][0] = (*rcvframe).status[0];    //status1
 (*p).status[0][1] = (*rcvframe).status[1];    //status2
 (*p).status[1][0] = (*rcvframe).status[0];    //status1
 (*p).status[1][1] = (*rcvframe).status[1];    //status2
 if(  ((*p).status[0][1] & 0x80) == 0 )
       (*p).laser =  ((*p).status[0][1] & 0x40) ;    //laser on bit
 
 icase=100;
 code= (char) (*(echo+6) );                 //command
 if ( code == cmdbf[cmdmvx ][6] ||
      code == cmdbf[cmdcntr][6] ||
      code == cmdbf[cmdresd][6] ||
      code == cmdbf[cmdkill][6] ||
      code == cmdbf[cmdwtfq][6] ||
      code == cmdbf[cmdwtwc][6] ||
      code == cmdbf[cmdwthc][6] ||
      code == cmdbf[cmdwtmt][6] ||
      code == cmdbf[cmdpwof][6] ||
      code == cmdbf[cmdpwon][6] ||
      code == cmdbf[cmdlasn][6] ||
      code == cmdbf[cmdlasf][6] ||
//    code == cmdbf[cmdwtemt][6]) ||
      code == cmdbf[cmdkill][6] ) icase= 1;
 if ( code == cmdbf[cmdsdvn][6])  icase= 3;
 if ( code == cmdbf[cmdrdfq][6])  icase= 4;
 if ( code == cmdbf[cmdrdwc][6])  icase= 5;
 if ( code == cmdbf[cmdrdhc][6])  icase= 6;
 if ( code == cmdbf[cmdrdmt][6])  icase= 8;
 if ( code == cmdbf[cmdrdth ][6]) icase=13;

// printf("read_store_answer: case=%d, code=%x \n",icase,code);
if(icase == 100) { 
 printf("read_store_answer: nothing to do, return\n");
 return(0);
}

 switch( icase) {
//only status
  case(1):
   if ( code == cmdbf[cmdmvx ][6] ) { (*p).move_by[0] = 0; (*p).move_by[1] = 0; }
// if ( code == cmdbf[cmdlasn][6] ) (*p).laser = 1;
// if ( code == cmdbf[cmdlasf][6] ) (*p).laser = 0;
   if ( code == cmdbf[cmdresd][6] ) (*p).pan_centred = 0; // x/y = 0/0 --> 17000/17000
   if ( code == cmdbf[cmdcntr][6] ) (*p).pan_centred = 1; // x/y = 0/0 is legal
   break;

//one byte (unsigned)
  case(2):
   rcvframe_b = (AMCframe_rcv_b *) rcvframe;
   break;

  case(3):
//   rcvframe_b = rcvframe;
   rcvframe_bb = (AMCframe_rcv_bb *) rcvframe;
   (*p).version[0][0] = (*rcvframe_bb).byte0;    //version0
   (*p).version[0][1] = (*rcvframe_bb).byte1;    //version1
   (*p).version[1][0] = (*rcvframe_bb).byte0;    //version0
   (*p).version[1][1] = (*rcvframe_bb).byte1;    //version1
   (*p).version[2][0] = (*rcvframe_bb).byte0;    //version0
   (*p).version[2][1] = (*rcvframe_bb).byte1;    //version1
   (*p).actstat[it] &= (! AMCwrngvers);
   if( (*p).version[it][0] < AMCminvers) (*p).actstat[it] |= AMCwrngvers;

   sameboxversion( p );

   break;

//one unsigned word
//AMCframe_rcv_uw   *rcvframe_freq ;
  case(4):
   rcvframe_uw = (AMCframe_rcv_uw *) rcvframe;
   (*p).freq[it] = (*rcvframe_uw).word0;    //frequency
   if( abs( (*p).freq[it] - AMCfreq ) > 5 ) {
    (*p).actstat[it] |= AMCwrngfrq;
   } else {
    (*p).actstat[it] &= (! AMCwrngfrq);
   }
   printf("freq  %d, %x\n", (*rcvframe_uw).word0, (*p).actstat[it]);    //frequency

//printf("(! AMCwrngfrq) %2.2x\n", (*p).actstat[it] );
   break;

  case (5):
   rcvframe_uw = (AMCframe_rcv_uw *) rcvframe;
   (*p).work[it] = (*rcvframe_uw).word0;    //working current
   if( abs( (*p).work[it] - 1000.*AMCwork ) > 1 ) {
    (*p).actstat[it] |= AMCwrngwrk;
   } else {
    (*p).actstat[it] &= (! AMCwrngwrk);
   }
   printf("workc %d, %x\n", (*rcvframe_uw).word0, (*p).actstat[it]);    //frequency
   break;

  case (6):                                 //read holding current
   rcvframe_uw = (AMCframe_rcv_uw *) rcvframe;
   (*p).hold[it] = (*rcvframe_uw).word0;    //holding current
   if( abs( (*p).hold[it] - 1000.*AMChold ) > 1 ) {
    (*p).actstat[it] |= AMCwrnghld;
   } else {
    (*p).actstat[it] &= (! AMCwrnghld);
   }
   printf("holdc %d, %x\n", (*rcvframe_uw).word0, (*p).actstat[it]);    //frequency
   break;

//one signed word
//AMCframe_rcv_w   *rcvframe_pos ;
  case (8):                                 //read motor position
   rcvframe_ww = (AMCframe_rcv_ww *) rcvframe;
   (*p).act_mot[0] = (*rcvframe_ww).xpos;   //actual motor position
   (*p).act_mot[1] = (*rcvframe_ww).ypos;   //actual motor position

   if( ((*p).status[0][1] & 3) == 0 ) {     //manipulate only at the end of the moves
    if( ((*p).act_mot[0] == 0)
     && ((*p).act_mot[1] == 0) ) {
     if( (*p).pan_centred == 0) {
      (*p).act_mot[0] = 17000;               // motor position lost
      (*p).act_mot[1] = 17000;               // motor position lost
     }
    } else (*p).pan_centred = 0;
   }


   addr=(*rcvframe_ww).src_addr;

   if( abs( (*p).act_mot[it] - (*p).pc_mot[it] ) > 2 ) {
    (*p).actstat[it] |= AMCwrngpos;
   } else {
    (*p).actstat[it] &= (! AMCwrngpos);
   }
   break;

  case (9)  :                                 //read motor position from EEprom
   rcvframe_w = (AMCframe_rcv_w *) rcvframe;
   (*p).act_mot[it] = (*rcvframe_w).word0;    //actual motor position
   break;


//six words
  case (13) :                   //read humidity and temperature, current voltage
   rcvframe_wwwwww = (AMCframe_rcv_wwwwww *) rcvframe;
   (*p).humi[0] = (*rcvframe_wwwwww).humid;
   (*p).temp[0] = (*rcvframe_wwwwww).temp;
   (*p).curr[0] = (*rcvframe_wwwwww).curr;
   (*p).Vsup[0] = (*rcvframe_wwwwww).volt1;
   (*p).V2  [0] = (*rcvframe_wwwwww).volt2;
   (*p).Vlog[0] = (*rcvframe_wwwwww).voltlog;

   (*p).humi[1] = (*rcvframe_wwwwww).humid;
   (*p).temp[1] = (*rcvframe_wwwwww).temp;
   (*p).curr[1] = (*rcvframe_wwwwww).curr;
   (*p).Vsup[1] = (*rcvframe_wwwwww).volt1;
   (*p).V2  [1] = (*rcvframe_wwwwww).volt2;
   (*p).Vlog[1] = (*rcvframe_wwwwww).voltlog;

//   printf("driv temp=%d, humid=%d %d %d %d %d\n", (*rcvframe_wwwwww).temp,
//         (*rcvframe_wwwwww).humid, (*rcvframe_wwwwww).curr, (*rcvframe_wwwwww).volt1,
//         (*rcvframe_wwwwww).volt2, (*rcvframe_wwwwww).voltlog);
//     sleep(10);
   break;


  default:
  //printf("read_frame.inc: default reached: code=%2.2x, icase %d\n",code, icase);
  //for(i=0;i<12;i++) printf("%2.2x ",(*(echo+i)) ); printf("\n");

  printf("error exit read_store_answer: case= %d \n", icase);
  return(-1);


 }

 gettimeofday( tv, tz);                 //time
 (*p).time[0]=atv.tv_usec;
 (*p).time[1]=atv.tv_sec - AMCtimeoff;
 (*p).pan_stat=STAT_OK2;
 (*p).ech2_cnt[it][0]++;

//printf("regular exit read_store_answer \n");
 return(0);
}

