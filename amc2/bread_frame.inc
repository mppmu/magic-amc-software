int bread_store_answer( int k, unsigned char *echo, AMCpanel *p)
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
  if( (k1<0) || (k1>0x0200) ) {
   return(-2);
  }
  i1=addr2indx[k][k1];
  j=bchpoint[k][i1];
  it = itfromactaddr[ k1 ];
 } else {                           
  it=2;
 }


 (*p).status[it][0] = (*rcvframe).status[0];    //status1 
 (*p).status[it][1] = (*rcvframe).status[1];    //status2
 (*p).laser =  ((*p).status[it][1] & 0x40) ;    //laser on bit
 
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
      code == cmdbf[cmdwtemt][7] ||
      code == cmdbf[cmdwtmt][7] )   {icase= 1; goto casefound;}


 if ( code == cmdbf[cmdrlas][7])    {icase= 2; goto casefound;}
 if ( code == cmdbf[cmdsdvn][7])    {icase= 3; goto casefound;}
 if ( code == cmdbf[cmdrdfq][7])    {icase= 4; goto casefound;}
 if ( code == cmdbf[cmdrdwc][7])    {icase= 5; goto casefound;}
 if ( code == cmdbf[cmdrdhc][7])    {icase= 6; goto casefound;}
 if ( code == cmdbf[cmdsdui][7])    {icase= 7; goto casefound;}
 if ( code == cmdbf[cmdrdmt][7])    {icase= 8; goto casefound;}
 if ( code == cmdbf[cmdrdemt][7])   {icase= 9; goto casefound;}
// if ( code == cmdbf[cmdrdlsw][7])   {icase=10; goto casefound;}
// if ( code == cmdbf[cmdrdusw][7])   {icase=11; goto casefound;}
 if ( code == cmdbf[cmdqwdc ][7])   {icase=12; goto casefound;}
 if ( code == cmdbf[cmdrdth ][7])   {icase=13; goto casefound;}
 if ( code == cmdbf[cmdr4ms ][7])   {icase=14; goto casefound;}
 if ( code == cmdbf[cmdrdfr ][7])   {icase=15; goto casefound;}

casefound:

 switch( icase) {
//only status
  case(1):
   break;

//one byte (unsigned)
  case(2):
   rcvframe_b = rcvframe;
   (*p).laser = (*rcvframe_b).byte0 ;
   break;
  case(3):
   rcvframe_b = rcvframe;
   (*p).version[it][0] = (*rcvframe_b).byte0;    //version
//   (*p).version[it][1] = 0;
   break;

//one unsigned word
//AMCframe_rcv_uw   *rcvframe_freq ;

  case(4):
   rcvframe_uw = rcvframe;
//no frequency for boxes
   (*p).actstat[it] &= (! AMCwrngfrq);
   break;

  case (5):
   rcvframe_uw = rcvframe;
//no current for boxes
   (*p).actstat[it] &= (! AMCwrngwrk);
   break;

  case (6):                                 //read holding current
   rcvframe_uw = rcvframe;
//no current for boxes
    (*p).actstat[it] &= (! AMCwrnghld);
   break;

  case (7):                                 //read unique ID
   rcvframe_uw = rcvframe;
// if( (*p).addr[it] != (*rcvframe_uw).word0) {    //unique ID
   if( (*p).addr[it]/4 != (*rcvframe_uw).word0/4 ) {    //unique ID: all 88x0,1,2,3 respond 88x0
    (*p).actstat[it] |= AMCwrngID;
   } else {
    (*p).actstat[it] &= (! AMCwrngID);
   }
   break;

//one signed word
//AMCframe_rcv_w   *rcvframe_pos ;
  case (8):                                 //read motor position
   rcvframe_w = rcvframe;
//no motor business for boxes
   (*p).actstat[it] |= AMCwrngpos;
   break;

  case (9)  :                                 //read motor position from EEprom
//no motor business for boxes
   rcvframe_w = rcvframe;
//   (*p).act_mot[it] = (*rcvframe_w).word0;    //actual motor position
   break;


// no endswitches for boxes


//one unsigned word, one byte
//AMCframe_rcv_bw   *rcvframe_watch ;
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


//no Hall sensors for boxes


  default:

  return(-1);


 }

 gettimeofday( tv, tz);                 //time
 (*p).time[0]=atv.tv_usec;
 (*p).time[1]=atv.tv_sec - AMCtimeoff;
 (*p).ech2_cnt[it][0]++;

 return(0);
}


