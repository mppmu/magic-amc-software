int fill_send_data( int *nbyte, unsigned char value[8], unsigned char code, int idx, AMCpanel *p)
{
 int i, it, i1, j, k1, icase, deltime, AMCfreq_ncnt, AMCfreq_act;
 int AMCwidth_act, width, AMCupmax;

 *nbyte=0;
 deltime=20;
 icase=100;
                                               //send no data
 if ( code == cmdbf[cmdcntr][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdncnt][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdpwof][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdpwon][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdnlasn][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdnlasf][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrlas][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdsdvn][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdfq][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdwc][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdhc][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdsdui][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdmt][7]  )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdemt][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdlsw][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdusw][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdqwdc ][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdth ][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdr4ms ][7] )  { icase=1; goto doswitch; }
 if ( code == cmdbf[cmdrdfr ][7] )  { icase=1; goto doswitch; }

                                               //send 1 word data
 if ( code == cmdbf[cmdmvx ][7]  )  { icase=2; goto doswitch; }
 if ( code == cmdbf[cmdwtfq][7]  )  { icase=2; goto doswitch; }
 if ( code == cmdbf[cmdwtwc][7]  )  { icase=2; goto doswitch; }
 if ( code == cmdbf[cmdwthc][7]  )  { icase=2; goto doswitch; }
 if ( code == cmdbf[cmdwtemt][7] )  { icase=2; goto doswitch; }
 if ( code == cmdbf[cmdwtmt][7]  )  { icase=2; goto doswitch; }

                                               //send 1 byte and 1 word of data
 if ( code == cmdbf[cmdwtlsw][7] )  { icase=3; goto doswitch; }
 if ( code == cmdbf[cmdwtusw][7] )  { icase=3; goto doswitch; }


doswitch:
 switch( icase) {
//no data to be sent
  case(1):
   if(idx < 2 ) {
    AMCfreq_act=(*p).freq[idx];
    AMCfreq_ncnt=AMCfreq_act*AMCfreq_reduct;
    AMCwidth_act=(*p).lup_mot[idx] - (*p).llw_mot[idx];

    if( code == cmdbf[cmdcntr ][7]  ) {                         //centre
//   width = (*p).pc_mot[idx] - (*p).endsw_lwp[idx] + (*p).esw_centr[idx]
     width = (*p).pc_mot[idx];
     if(width < -3300) width = 3500;     //if pc_mot is flag (not set to position) assume upper end
     if(width >  3500) width = 3500;     //if pc_mot is flag (not set to position) assume upper end
     width = width            - (*p).endsw_lwp[idx] + (*p).esw_centr[idx]
                              + 1.5 * ( (*p).esw_range[idx] );
     deltime=10.+(1000./AMCfreq)*width;

     break;
    }

    if( code == cmdbf[cmdncnt ][7]  ) {                         //new centre
     width = 2. * ( (*p).u600_pos[idx] - (*p).l600_pos[idx] );
     AMCupmax=imin( (*p).pc_mot[idx],  (*p).lup_mot[idx] );
     if( AMCupmax >  (*p).u600_pos[idx] )
       width = AMCupmax - (*p).l600_pos[idx] +
             ( (*p).u600_pos[idx] - (*p).l600_pos[idx] ) /2.;
     else if( AMCupmax < (*p).l600_pos[idx] )
       width = (*p).u600_pos[idx] -AMCupmax +
             ( (*p).u600_pos[idx] - (*p).l600_pos[idx] ) /2.;
     deltime=10.+(1100./AMCfreq_ncnt)*width;
     break;
    }



    if( code == cmdbf[cmdrdmt ][7]  ) {                         //query
     (*p).act_mot[idx] = -9000;
    }


    if( (code == cmdbf[cmdcntr][7])  ||
        (code == cmdbf[cmdncnt][7]) ) {
     (*p).act_mot[idx] = -20000;
    }
   }
   *nbyte=0;
   break;

//one word  (un)signed
  case(2):
   *nbyte=2;
   if( code == cmdbf[cmdmvx ][7]  ) {*word=(*p).move_by[idx]; k1=*word/2; *word=2*k1; //make even
                                     if(abs(*word)>0) deltime=10.+(1000./AMCfreq)*abs(*word);
                                     value[0]=byte[0];value[1]=byte[1]; 
                                     break;}
// if( code == cmdbf[cmdwtfq][7]  ) {*word=AMCfreq; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdwtfq][7]  ) {*word=(*p).freq[idx]; value[0]=byte[0];value[1]=byte[1]; break;}
// if( code == cmdbf[cmdwtwc][7]  ) {*word=AMCwork*1000; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdwtwc][7]  ) {*word=(*p).work[idx]; value[0]=byte[0];value[1]=byte[1]; break;}
// if( code == cmdbf[cmdwthc][7]  ) {*word=AMChold*1000; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdwthc][7]  ) {*word=(*p).hold[idx]; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdwtemt][7] ) {*word=(*p).pc_mot[idx]; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdwtmt][7]  ) {*word=(*p).pc_mot[idx]; value[0]=byte[0];value[1]=byte[1]; break;}
   if( code == cmdbf[cmdmvx ][7]  )  (*p).act_mot[idx] = -20000;

//one byte and one word  (un)signed
  case(3):
   *nbyte=3;
   if( code == cmdbf[cmdwtlsw][7] ) {value[0]=(*p).endsw_lwn[idx]; *word=(*p).endsw_lwl[idx];
                                     value[1]=byte[0];value[2]=byte[1]; break;}
   if( code == cmdbf[cmdwtusw][7] ) {value[0]=(*p).endsw_upn[idx]; *word=(*p).endsw_upl[idx];
                                     value[1]=byte[0];value[2]=byte[1]; break;}


  default:

  return(-1);


 }


 gettimeofday( tv, tz);                 //time
 (*p).time[0]=atv.tv_usec;
 (*p).time[1]=atv.tv_sec - AMCtimeoff;

 return(deltime);
}
