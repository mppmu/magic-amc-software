void check_echo(echpnt1, nechpnt1, echpnt2, nechpnt2, 
                echaltpnt1, nechaltpnt1, echaltpnt2, nechaltpnt2,
                fncact, cntact, gchpoint, cleared_bits, sumgch )
     int nechpnt1[8],nechaltpnt1[8], nechpnt2[8],nechaltpnt2[8];
     int echpnt1[8][250],echaltpnt1[8][250], echpnt2[8][250],echaltpnt2[8][250];
     int fncact, cntact, gchpoint[n8][n150], *sumgch;
     unsigned char cleared_bits;
{
 #include "AMCframe.h"
 int fralen, i, j, k, l, i1, j1, k1, l1, it, nnn;
 ushort crcword;
// unsigned char cetext[200];
 AMCframe_rcv_0 *rcvframe;
 AMCframe_rcv_1 *rcvframe1;
 AMCframe_snd_0 *sndframe;


//-------------------------------------------------

nnn=0;
//find echo 1b frames
for(k=0;k<nport;k++) {
 nechaltpnt1[k]=0;
 j=0;
 for(l=j;l<echaltpnt[k]-1;l++) {
// fb 01 act box f0 cnt fnc
  if((echoalt[k][l] == 0xfb) && (echoalt[k][l+1] == 0x01) &&
     (echoalt[k][l+4] == 0xf0) ) {
   fralen=imin(16,echoalt[k][l+5]+8)+15;
    for(i=l;i<fralen+l;i++) printf("%2.2x ", echoalt[k][i] );
    printf("\n");
   j=l+8;
   echaltpnt1[k][nechaltpnt1[k]++]=l;
   sndframe   = (AMCframe_snd_0 *) &echoalt[k][l];
   k1=((*sndframe).dst_addr) ;
   if(k1<0) continue;
   if(k1>0x500) continue;
   i1=addr2indx[k][k1];
   if(i1<0) goto illegaddr;
   it=itfromactaddr[k1];

   it=0;

   if(it<0) goto illegaddr;
   if(it>1) goto illegaddr;
//printf("$check_echo i1 %d,it %d\n",i1,it);
   j1=chpoint[k][i1];         
//printf("$check_echo j1 %d\n",j1);
   (*pa[j1]).ech1_cnt[it][1]++;            //count 1b echo actuator
  }     //send frame found
illegaddr:;
 }      //l=j to end
  printf("nechaltpnt1 %d\n",nechaltpnt1[k]);
//-}       //k=0 to 6


//-------------------------------------------------

//find echo 2b frames (responses/acknowledges)
//-for(k=0;k<nport;k++) {
 nechaltpnt2[k]=0;
 j=0;
  printf("2b: k %d, echaltpnt %d\n",k,echaltpnt[k]);
  for(l=j;l<echaltpnt[k]-1;l++) {
// fb 85 f0 act box cnt fnc s1 s2
   if((echoalt[k][l] == 0xfb)  && (echoalt[k][l+1] == 0x85) &&
     (echoalt[k][l+2] == 0xf0)  &&
     (echoalt[k][l+5] == cntact) && (echoalt[k][l+6] == fncact)) {
   crcword = 0;
   for(l1=l+2;l1<l+echoalt[k][l+5]+6;l1++) crcword = CRC16( echoalt[k][l1], crcword);
   (*word)=crcword;
   if( (echoalt[k][l+echoalt[k][l+5]+6] == byte[0]) &&
       (echoalt[k][l+echoalt[k][l+5]+7] == byte[1]) &&        //CRC ok?
       (echoalt[k][l+7] == 0)                       &&        //st1: 0?
       ((echoalt[k][l+8] & 0x80) == 0)               )        //st2: no response from driver?
//check also status2 here
   if( (echoalt[k][l+8] & cleared_bits) == 0     ) {      //st2^bits = 0?

    rcvframe   = (AMCframe_rcv_0 *) ((&echoalt[k][l])+1);
    k1=((*rcvframe).src_addr);
    i1=addr2indx[k][k1];
    if(i1<0) goto lab1;
    it=itfromactaddr[k1];

   it=0;

    if( (it<0) || (it>1) ) goto lab1;

printf("k %d, k1 %d, i1 %d, it %d, j1 %d\n",k,k1,i1,it,j1);

    j1=chpoint[k][i1];
    (*pa[j1]).ech2_cnt[it][1]++;              //actuator
    if(gchpoint[k][i1]==0) (*sumgch)++;
     printf("k %d i1 %d gch %d, sumgch %d\n",k,i1,gchpoint[k][i1],*sumgch);
    gchpoint[k][i1]=1;
    j=l+8;
// printf("check_e nechaltpnt2[k]=%d, k=%d\n",nechaltpnt2[k],k);                                          
    echaltpnt2[k][nechaltpnt2[k]++]=l;


   }     //recv_frame found
  } 
lab1:   ;
 }      //l=j to end


  printf("nechaltpnt2 %d\n",nechaltpnt2[k]);
  printf("\n");
}       //k=0 to 6



//find echo 1a frames
 printf("\n");
for(k=0;k<nport;k++) {
 nechpnt1[k]=0;
 printf("port/chain %d\n",k);
 j=0;
 for(l=j;l<echpnt[k]-1;l++) {
  if((echo[k][l] == 0xfb) && (echo[k][l+1] == 0x01) &&
     (echo[k][l+4] == 0xf0) ) {
   fralen=imin(16,echo[k][l+5]+8)+14;
//  for(i=l;i<fralen+l;i++) printf("%2.2x ", echo[k][i] );
    printf("\n");
   j=l+2;
   echpnt1[k][nechpnt1[k]++]=l;

   sndframe   = (AMCframe_snd_0 *) &echo[k][l];
   k1=((*sndframe).dst_addr) ;
   if(k1<0) continue;
   if(k1>0x500) continue;
   i1=addr2indx[k][k1];
   if(i1<0) goto illegaddr0;
   it=itfromactaddr[k1];

   it=0;

   if(it<0) goto illegaddr0;
   if(it>1) goto illegaddr0;
   j1=chpoint[k][i1];         //$$
   (*pa[j1]).ech1_cnt[it][0]++;            //count1a echo act
  }
illegaddr0: ;
 }
  printf("nechpnt1 %d\n",nechpnt1[k]);
//-}


//===============================================================

//find echo 2a frames (responses/acknowledges)
//-for(k=0;k<nport;k++) {
 nechpnt2[k]=0;
 j=0;
 printf("2a: k %d, echpnt %d\n",k,echpnt[k]);
 for(l=j;l<echpnt[k]-1;l++) {
// for(l1=l;l1<l+25;l1++) printf("%2.2x ",echo[k][l1]);
// printf("  l= %d, c=%d, f=%2.2x\n",l,cntact,fncact);

  if((echo[k][l] == 0xfb)   && (echo[k][l+1] == 0x85) &&
     (echo[k][l+2] == 0xf0)   &&
     (echo[k][l+5] == cntact) && (echo[k][l+6] == fncact)) {
   crcword = 0;
   nnn++;
   for(l1=l+2;l1<l+echo[k][l+5]+6;l1++) crcword = CRC16( echo[k][l1], crcword);
   (*word)=crcword;
    for(i=l;i<15    +l;i++) printf("%2.2x ", echo[k][i] );
// printf("CRC %2.2x %2.2x, %d, cleared %2.2x\n",byte[0], byte[1], nnn, cleared_bits );
   if( (echo[k][l+echo[k][l+5]+6] == byte[0]) &&
       (echo[k][l+echo[k][l+5]+7] == byte[1]) &&        //CRC ok?
       (echo[k][l+7] == 0)                    ) {      //st1: 0?
//check also status2 here
   if( (echo[k][l+8] & cleared_bits) == 0     ) {      //st2^bits = 0?

    rcvframe   = (AMCframe_rcv_0 *) ((&echo[k][l])+1);
    rcvframe1  = (AMCframe_rcv_1 *) ((&echo[k][l])+1);
    if((*rcvframe1).drv_addr < 4) {
     k1=(*rcvframe).src_addr;
     if(k1<0) continue;
     if(k1>=0x500) continue;
     i1=addr2indx[k][k1];
     if( i1<0 ) goto lab2;
     j1=chpoint[k][i1];
//   it=itfromactaddr[k1];
printf("k,k1,i1,j1 %d %d %d, %d\n",k,k1,i1,j1);
    } else {
     k1=(*rcvframe1).box_addr;
     if(k1<0) continue;
     if(k1>=0x500) continue;
     i1=baddr2indx[k][k1];
     if( i1<0 ) goto lab2;
     j1=bchpoint[k][i1];
//   it=itfromactaddr[k1];
     printf("box ");
printf("k,k1,i1,j1 %d %d %d, %d\n",k,k1,i1,j1);
     if(j1 < 0) {
      printf("bchpoint[k][l1]=");
      for(l1=0;l1<bnpoint[k];l1++)
       printf(" %d,",bchpoint[k][l1]);
      printf("\n");
     }
    }
    it=0;



printf("gchpoint[k][i1]= %d \n",gchpoint[k][i1]);

// why j1=-1
    (*pa[j1]).ech2_cnt[it][0]++;

    if(gchpoint[k][i1]==0) (*sumgch)++;
    printf("k %d i1 %d gch %d, sumgch %d\n",k,i1,gchpoint[k][i1],*sumgch);
    gchpoint[k][i1]=2;
//  j=l+8;
    j=l+7;
    echpnt2[k][nechpnt2[k]++]=l;


   }
   }    //status cleared
// l+=4;//the next 4 guys can't match
  }     // if header, source addr, counter, function  ok

lab2: ; //found this one
 }

}
}
