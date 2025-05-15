int fill_send_data( int *nbyte, unsigned char value[8], unsigned char code, int idx, AMCpanel *p)
{
 int i, it, i1, j, k1, icase, deltime, AMCfreq_ncnt, AMCfreq_act, dlt0;
 int AMCwidth_act, width, AMCupmax;

// printf("fill_send_data in send_frame entered\n");
 *nbyte=0;
 deltime=20;
 icase=100;

                                               //send no data
 if ( code == cmdbf[cmdcntr][6]  )  { icase=1; goto doswitch; }  //  centre
 if ( code == cmdbf[cmdpwof][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdpwon][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdlasn][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdlasf][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdsdvn][6]  )  { icase=1; goto doswitch; }   //
 if ( code == cmdbf[cmdrdfq][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdrdwc][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdrdhc][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdrdmt][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdrdth][6]  )  { icase=1; goto doswitch; }   //
 if ( code == cmdbf[cmdresb][6]  )  { icase=1; goto doswitch; }  //
 if ( code == cmdbf[cmdkill][6]  )  { icase=1; goto doswitch; }  //

                                               //send 1 word data
 if ( code == cmdbf[cmdwtfq][6]  )  { icase=2; goto doswitch; }  //
 if ( code == cmdbf[cmdwtwc][6]  )  { icase=2; goto doswitch; }  //
 if ( code == cmdbf[cmdwthc][6]  )  { icase=2; goto doswitch; }   //
 if ( code == cmdbf[cmdwtmt][6]  )  { icase=2; goto doswitch; }

                                               //send 2 word data
 if ( code == cmdbf[cmdmvx ][6]  )  { icase=3; goto doswitch; }     //

doswitch:

 switch( icase) {
//no data to be sent
  case(1):
   if(idx < 2 ) {
    AMCfreq_act=(*p).freq[idx];
    AMCfreq_ncnt=AMCfreq_act*AMCfreq_reduct;
    AMCwidth_act=(*p).lup_mot[idx] - (*p).llw_mot[idx];

    if( code == cmdbf[cmdcntr ][6]  ) {                         //centre
     width = (*p).pc_mot[idx];
     if(width < -3300) width = 3500;     //if pc_mot is flag (not set to position) assume upper end
     if(width >  3500) width = 3500;     //if pc_mot is flag (not set to position) assume upper end
     width = width       + 2 * 7000 ;
     deltime=10.+(1000./AMCfreq)*width;
//   deltime=10.+(1200./AMCfreq)*width;  //wait 20% longer
     break;
    }




    if( code == cmdbf[cmdrdmt ][6]  ) {                         //query
printf(" ----->  send_frame: send query command; preset to -9000 ...\n");
     (*p).act_mot[0] = -9000;
     (*p).act_mot[1] = -9000;
    }


    if( (code == cmdbf[cmdcntr][6]) ) {
     (*p).act_mot[idx] = -20000;
    }
   }
   *nbyte=0;
   break;

//one word  (un)signed
  case(2):
   *nbyte=2;
   if( code == cmdbf[cmdwtfq][6]  ) {*word=(*p).freq[0]; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdwtwc][6]  ) {*word=(*p).work[0]; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdwthc][6]  ) {*word=(*p).hold[0]; value[0]=byte[0];value[1]=byte[1]; break;}

//two words signed
  case(3):
   *nbyte=4;
   if( code == cmdbf[cmdmvx ][6]  ) {*word=(*p).move_by[0]; k1=*word/2; *word=2*k1; //make even
                                     value[0]=byte[0];value[1]=byte[1]; 
                                     if(abs(*word)>0) dlt0=10.+(1000./AMCfreq)*abs(*word);

                                     *word=(*p).move_by[1]; k1=*word/2; *word=2*k1; //make even
                                     value[2]=byte[0];value[3]=byte[1];
                                     if(abs(*word)>0) deltime=10.+(1000./AMCfreq)*abs(*word);
                                     if(dlt0 > deltime) deltime=dlt0;
                                     printf(" =====> move %d steps, wait %d\n",2*k1,deltime) ; //AB 20.03.09

printf("send_frame: value %x %x %x %x\n",value[0],value[1],value[2],value[3]);

if( ( (*p).act_mot[0] + (*p).move_by[0] == 0 ) &&
    ( (*p).act_mot[1] + (*p).move_by[1] == 0 ) ) (*p).pan_centred = -1 ;
printf("act_mod %d, move_by %d; [1] %d %d, pan_cent %d \n",
      (*p).act_mot[0],(*p).move_by[0], (*p).act_mot[1],(*p).move_by[1], (*p).pan_centred);

                                     break;}


  default:
  printf("send_frame.inc: default reached: code=%2.2x, icase %d\n",code, icase);
  for(i=0;i<12;i++) printf("%2.2x ",(*(echo+i)) ); printf("\n");

  return(-1);


 }


 gettimeofday( tv, tz);                 //time
 (*p).time[0]=atv.tv_usec;
 (*p).time[1]=atv.tv_sec - AMCtimeoff;

 return(deltime);
}
