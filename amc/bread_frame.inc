void sameboxhumtemp(AMCpanel *pb, int hum, int temp, int curr, 
                    int v1, int v2, int v3) {

 int i,i2,k,j;
 unsigned debug[2560];
 AMCpanel *p;

 for(k=0;k<3;k++) {
  i= (*pb).samebox[k];
  if(i >= 0) {
   p=&panel[ipan[i]+8][jpan[i]+8];
   for(j=0;j<3;j++) {
    (*p).humi[j]=hum;
    (*p).temp[j]=temp;
    (*p).curr[j] = curr;
    (*p).Vsup[j] = v1;
    (*p).V2  [j] = v2;
    (*p).Vlog[j] = v3; 
   }
  }
 }
}

void sameboxcent(AMCpanel *pb, int cntr_flg ) {

 int i,i2,k,j;
 unsigned debug[2560];
 AMCpanel *p;

 for(k=0;k<3;k++) {
  i= (*pb).samebox[k];
  if(i >= 0) {
   printf("panel x %d y %d, center flag %d \n",ipan[i],jpan[i], cntr_flg);
   p=&panel[ipan[i]+8][jpan[i]+8];
   (*p).pan_centred = cntr_flg;
  }
 }
}

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
 AMCframe_rcv_wwwwww *rcvframe_wwwwww ;
 AMCframe_rcv_9w   *rcvframe_9w   ;
  
 for(i=0;i<12;i++) printf("%2.2x ",*(i+echo)); printf("\n");



 rcvframe   = echo+1;
 printf("bread_store_answer: %2.2x %4.4x, %4.4x, %d, %2.2x\n"
  ,(*rcvframe).header
  ,(*rcvframe).src_addr  ,(*rcvframe).dst_addr
  ,(*rcvframe).counter   ,(*rcvframe).function);

 it=2;


 (*p).status[it][0] = (*rcvframe).status[0];    //status1 
 (*p).status[it][1] = (*rcvframe).status[1];    //status2
 
 icase=100;
 code= (char) (*(echo+6) );                 //command

/*  box commands:
fb   01   bn    ff    f0   01  09       Box reset
fb   01   bn    ff    f0   01  1C       Update the absolute x and y position for all motors from EEPROM
fb   01   bn    ff    f0   01  1D       Store the absolute x and y position for all motors from EEPROM
fb   01   bn    ff    f0   01  1F       Request for different parameters: t, h, v, ...
fb   01   bn    ff    f0   01  FD       Jump to Boot Section
fb   01   bn    ff    f0   01  FF       Resend latest (previous) answer frame
*/


 if ( code == cmdbf[cmdresb][6] )          // reset box (alll cards
                                    {icase= 1; goto casefound;}
 if ( code == cmdbf[cmdrdth ][6])   {icase=13; goto casefound;}     //only this command allowed



  printf("bread_frame.inc: code=%2.2x, cmdbf %d\n",code, cmdbf[cmdrdth ][6]);

casefound:
  printf("bread_store_answer: case=%d \n",icase);

 switch( icase) {
//only status
  case(1):
   if ( code == cmdbf[cmdresb][6] ) {
    (*p).pan_centred = 0; // x/y = 0/0 --> 17000/17000
    sameboxcent(p, 0 );
   }
   printf("case=1, %d\n", code ) ;
   break;

//one byte (unsigned)
  case(2):
   rcvframe_b = rcvframe;
   (*p).laser = (*rcvframe_b).byte0 ;
   break;
  case(3):
   rcvframe_b = rcvframe;
   (*p).version[it][0] = (*rcvframe_b).byte0;    //version
   break;



//six words
//AMCframe_rcv_ww   *rcvframe_tmp ;
  case (13) :                   //read humidity and temperature

   rcvframe_wwwwww = rcvframe;
   for(j=0;j<3;j++) {
    (*p).humi[0] = (*rcvframe_wwwwww).humid;
    (*p).temp[0] = (*rcvframe_wwwwww).temp;
    (*p).curr[0] = (*rcvframe_wwwwww).curr;
    (*p).Vsup[0] = (*rcvframe_wwwwww).volt1;
    (*p).V2  [0] = (*rcvframe_wwwwww).volt2;
    (*p).Vlog[0] = (*rcvframe_wwwwww).voltlog;
   }

   sameboxhumtemp( p, (*p).humi[0], (*p).temp[0]
                    , (*p).curr[0], (*p).Vsup[0], (*p).V2[0], (*p).Vlog[0] );

   printf("it=%d, humid=%d, temp=%d\n",it, (*rcvframe_wwwwww).humid,
                                           (*rcvframe_wwwwww).temp);
   printf("curr, v1, v2, vlog =%d, %d, %d, %d\n", (*rcvframe_wwwwww).curr
                                                , (*rcvframe_wwwwww).volt1
                                                , (*rcvframe_wwwwww).volt2
                                                , (*rcvframe_wwwwww).voltlog);
   break;

  default:
  printf("bread_frame.inc: default reached: code=%2.2x, icase %d\n",code, icase);
  for(i=0;i<12;i++) printf("%2.2x ",(*(echo+i)) ); printf("\n");

  printf("error exit bread_store_answer \n");
  return(-1);


 }

 gettimeofday( tv, tz);                 //time
 (*p).time[0]=atv.tv_usec;
 (*p).time[1]=atv.tv_sec - 1221000000;
 (*p).pan_stat=STAT_OK1;
 i= STAT_OK1;
 sameboxstat(p, i);
 (*p).ech2_cnt[it][0]++;

 printf("regular exit bread_store_answer \n");
 return(0);
}


