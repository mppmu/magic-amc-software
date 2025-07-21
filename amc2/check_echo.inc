void check_echo(echpnt1, nechpnt1, echpnt2, nechpnt2, 
                echaltpnt1, nechaltpnt1, echaltpnt2, nechaltpnt2,
                fncact, cntact, gchpoint, cleared_bits, sumgch )
     int nechpnt1[8],nechaltpnt1[8], nechpnt2[8],nechaltpnt2[8];
     int echpnt1[8][250],echaltpnt1[8][250], echpnt2[8][250],echaltpnt2[8][250];
     int fncact, cntact, gchpoint[n8][n150], *sumgch;
     unsigned char cleared_bits;
{
 #include "AMCframe.h"
 int fralen, i, j, k, l, i1, j1, k1, l1, it;
 ushort crcword;
// unsigned char cetext[200];
 AMCframe_rcv_0 *rcvframe;
 AMCframe_snd_0 *sndframe;


//-------------------------------------------------

//find echo 1b frames
for(k=0;k<nport;k++) {
 nechaltpnt1[k]=0;
 j=0;
 for(l=j;l<echaltpnt[k]-1;l++) {
  if((echoalt[k][l] == 0xfb) & (echoalt[k][l+1] == 0x01) &
     (echoalt[k][l+4] == 0x8f) & (echoalt[k][l+5] == 0x8f)) {
   fralen=imin(16,echoalt[k][l+6]+9)+15;
   j=l+8;
   echaltpnt1[k][nechaltpnt1[k]++]=l;
   sndframe   = &echoalt[k][l];
   k1=((*sndframe).dst_addr)-0x8000 ;
   if(k1<0) continue;
   if(k1>1000) {                            //Box
    k1=((*sndframe).dst_addr)-0x8800 ;
    if(k1<0 || k1 >700) continue;
    i1=baddr2indx[k][k1];
    if(i1<0) goto illegaddr;
    it=2;
    j1=bchpoint[k][i1];
    (*pb[j1]).ech1_cnt[it][1]++;            //count 1b echo box
   } else {                                 //Actuator
    i1=addr2indx[k][k1];
    if(i1<0) goto illegaddr;
    it=itfromactaddr[k1];
    if(it<0 || it >1 ) goto illegaddr;
    j1=chpoint[k][i1];         
    (*pa[j1]).ech1_cnt[it][1]++;            //count 1b echo actuator
   }
  }     //send frame found
illegaddr:;
 }      //l=j to end
//-}       //k=0 to 6


//-------------------------------------------------

//find echo 2b frames (responses/acknowledges)
//-for(k=0;k<nport;k++) {
 nechaltpnt2[k]=0;
 j=0;
  for(l=j;l<echaltpnt[k]-1;l++) {
   if((echoalt[k][l] == 0xfb)   & (echoalt[k][l+1] == 0x85) &
     (echoalt[k][l+2] == 0x8f) & (echoalt[k][l+3] == 0x8f) &
     (echoalt[k][l+6] == cntact) & (echoalt[k][l+7] == fncact)) {
   crcword = 0;
   for(l1=l+2;l1<l+echoalt[k][l+6]+7;l1++) crcword = CRC16( echoalt[k][l1], crcword);
   (*word)=crcword;
   if( (echoalt[k][l+echoalt[k][l+6]+7] == byte[0]) &
       (echoalt[k][l+echoalt[k][l+6]+8] == byte[1]) &        //CRC ok?
       (echoalt[k][l+8] == 0)                       )        //st1: 0?
//check also status2 here
   if( (echoalt[k][l+9] & cleared_bits) == 0     ) {      //st2^bits = 0?

    rcvframe   = &echoalt[k][l];
    k1=((*rcvframe).src_addr)-0x8000 ;
    if(k1<0) continue;
    if(k1>1000) {
     k1=((*rcvframe).src_addr)-0x8800 ;
     if(k1<0 || k1>700) continue;
     i1=baddr2indx[k][k1];
     if(i1<0) goto lab1;
     it=2;
     j1=bchpoint[k][i1];
     (*pb[j1]).ech2_cnt[it][1]++;              //box
    } else {
     i1=addr2indx[k][k1];
     if(i1<0) goto lab1;
     it=itfromactaddr[k1];
     if( (it<0) || (it>1) ) goto lab1;

     j1=chpoint[k][i1];
     (*pa[j1]).ech2_cnt[it][1]++;              //actuator
    }
    if(gchpoint[k][i1]==0) (*sumgch)++;
    gchpoint[k][i1]=1;
    j=l+8;
    echaltpnt2[k][nechaltpnt2[k]++]=l;


   }     //recv_frame found
  } 
lab1:   ;
 }      //l=j to end

}       //k=0 to 6



//find echo 1 frames     ECHO OF COMMAND SENT
for(k=0;k<nport;k++) {
 nechpnt1[k]=0;
 j=0;
 for(l=j;l<echpnt[k]-1;l++) {
  if((echo[k][l] == 0xfb) & (echo[k][l+1] == 0x01) &
     (echo[k][l+4] == 0x8f) & (echo[k][l+5] == 0x8f)) {
   fralen=imin(16,echo[k][l+6]+9)+14;
   j=l+2;
   echpnt1[k][nechpnt1[k]++]=l;

   sndframe   = &echo[k][l];
   k1=((*sndframe).dst_addr)-0x8000 ;
   if(k1<0) continue;
   if(k1>1000) {
    k1=((*sndframe).dst_addr)-0x8800 ;
    if(k1<0 || k1>700) continue;
    i1=baddr2indx[k][k1];
    if(i1<0) goto illegaddr0;
    it=2;
    j1=bchpoint[k][i1];
    (*pb[j1]).ech1_cnt[it][0]++;            //count1a echo box
   } else {
    i1=addr2indx[k][k1];
    if(i1<0) goto illegaddr0;
    it=itfromactaddr[k1];
    if(it<0 || it>1) goto illegaddr0;
    j1=chpoint[k][i1];         //$$
    (*pa[j1]).ech1_cnt[it][0]++;            //count1a echo act
   }
  }
illegaddr0: ;
 }
//-}

//find echo 2 frames (responses/acknowledges)  RESPONSE OF RECEIVER
//-for(k=0;k<nport;k++) {
 nechpnt2[k]=0;
 j=0;
 for(l=j;l<echpnt[k]-1;l++) {
  if((echo[k][l] == 0xfb)   & (echo[k][l+1] == 0x85) &
     (echo[k][l+2] == 0x8f) & (echo[k][l+3] == 0x8f) &
     (echo[k][l+6] == cntact) & (echo[k][l+7] == fncact)) {
      //             length                     function
   crcword = 0;
   for(l1=l+2;l1<l+echo[k][l+6]+7;l1++) crcword = CRC16( echo[k][l1], crcword);
   (*word)=crcword;
   if( (echo[k][l+echo[k][l+6]+7] == byte[0]) &
       (echo[k][l+echo[k][l+6]+8] == byte[1]) &        //CRC ok?
       (echo[k][l+8] == 0)                    )        //st1: 0?
//check also status2 here
   if( (echo[k][l+9] & cleared_bits) == 0     ) {      //st2^bits = 0?

    rcvframe   = &echo[k][l];
    k1=((*rcvframe).src_addr)-0x8000 ;
    if(k1>1000) {
     k1=((*rcvframe).src_addr)-0x8800 ;
     if(k1<0 || k1>700) continue;
     i1=baddr2indx[k][k1];
     if(i1<0) goto lab2;
     it=2;
     j1=bchpoint[k][i1];
     (*pb[j1]).ech2_cnt[it][0]++;
    } else {
     if(k1<0 || k1>700) continue;
     i1=addr2indx[k][k1];
     if( i1<0 ) goto lab2;
     j1=chpoint[k][i1];
     it=itfromactaddr[k1];              //HA 10.12.09 added
     if( (it<0) || (it>1) ) goto lab2;
     (*pa[j1]).ech2_cnt[it][0]++;
    }
    if(gchpoint[k][i1]==0) (*sumgch)++;
    gchpoint[k][i1]=2;
    j=l+8;
    echpnt2[k][nechpnt2[k]++]=l;



   }    //status cleared
  }     // if header, source addr, counter, function  ok

lab2: ; //found this one
 }
}
}
