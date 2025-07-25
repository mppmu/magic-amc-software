/* TODO

   - include moveby

   - preset output to -8888 for all queries (like Q=-9000) so it can be detected afterwards

   - include retry of comm-error in amcab.c

   - cleanup amcmotor

*/


//========================================================================
//========================================================================



void initall_inc(int iflag) {
// iflag>=0: init of some kind   ;  iflag<-10: LaserOn/Off    ;  iflag<0: special actuator cmds
// iflag=0: init all   | iflag=1: init minimum, auto center if needed | iflag=2: not even autocenter
//      -1: center all |      -2: moveto       |   -3: query
//     -11: laserOn    |     -12: laserOff

 int i, j, k, l, it;
 int i1, j1, k1, ii;
 int steps, icmd, nroundmax, boxtot, acttot, totpan, ibreak;
 unsigned char cleared_bits;
 unsigned char tascii[256];
 double dscroll,dscrol2, dscrol3, dscrol4, scroll, scroll0;
 int locchpoint[n8][n150], locnpoint[n8], ipo;
 int px, py ;
 int numquery, iquery, uslptim, las_ask ;

//========================================================================


 printf("---------------------- initall------- totbox %d\n",totbox);


 //bursting
 ii=-1;
 AMC_scroll= 0.0;
 flush_buffers();
 burst_clear();


 //setup list of boxes and actuators to work with
 //boxes
 for (k=0; k<nport; k++) {
   bnpoint[k]=0;
   for(i=0;i<700;i++) baddr2indx[k][i]=-1;
   for(i=0;i<n150;i++) bchpoint[k][i]=-1;
 }
 for (j=0;j<totbox;j++){
   k=xxport[j]%8 - xxport[j]/8;
   l=2 ;
   bchpoint[k][bnpoint[k]]=-1;
   if (iflag >=0 ) (*pb[j]).err_cnt[l]=0;   //if command is init, reset err-counters
   if ( ( (*pb[j]).pan_sel > 0 ) &&
        ( (*pb[j]).err_cnt[l] < AMC_errmax   ) ) {
      (*pb[j]).version[l][1]=0;
      if (iflag >=0) {                      //if command is init
         (*pb[j]).version[l][0]=0;
         (*pb[j]).actstat[l] =0;
      }
      byte[0]=xxcntr[j][0];
      byte[1]=xxcntr[j][1];
      i=(*word)-0x8800;
      if (i>1023 || i<0 ) continue;
      baddr2indx[k][i] = bnpoint[k];
      bchpoint[k][bnpoint[k]++]=j;
   }
 }

 //actuators
 for (k=0; k<nport; k++) {
   npoint[k]=0;
   for(i=0;i<700;i++) addr2indx[k][i]=-1;
   for(i=0;i<n150;i++) chpoint[k][i]=-1;
 }
 for (j=0;j<totact;j++){
   k=xport[j]%8 - xport[j]/8;
   l=xact[j] ;
   chpoint[k][npoint[k]]=-1;
   (*pa[j]).tmp_cnt = 0 ;     
   (*pa[j]).tmp_sl2 =(*pa[j]).tmp_sel =(*pa[j]).pan_sel ;    //copy of original value
   if (iflag >=-1 ) (*pa[j]).err_cnt[l]=0;  //if command is init or center
   if ( ( (*pa[j]).tmp_sel > 0 ) &&
        ( (*pa[j]).err_cnt[0]+(*pa[j]).err_cnt[1] < AMC_errmax*1.5 ) ) {
      (*pa[j]).version[l][1]=0;
      if (iflag >=0) {                      //if command is init
         (*pa[j]).version[l][0]=0;
         (*pa[j]).actstat[l] =0;
      }
      byte[0]=xcntr[j][0];
      byte[1]=xcntr[j][1];
      i=(*word)-0x8000;
      if (i>1023 || i<0 ) continue;
      addr2indx[k][i] = npoint[k];
      chpoint[k][npoint[k]++]=j;
   }
 }


 //anything to do?
 acttot=0;
 boxtot=0;
 for(k=0;k<nport;k++) { boxtot += bnpoint[k]; acttot +=npoint[k]; }


 if (iflag < -10  && boxtot == 0 )  return ;   //box commands
 if (iflag <   0  && acttot == 0 )  return ;   //act commands
 if ( boxtot + acttot == 0 )        return ;   //combined


 cleared_bits=0;
 steps= 20;
 nmaxcurr=410;                                  //no current needed
 nroundmax=3;


 //dscroll = for normal commands ; dscrol2 for move and center
 if      (iflag ==  0 ) { dscrol2=50.; dscroll=(100.-dscrol2)/30.;}   //assume 50% for centering if needed
 else if (iflag ==  1 ) { dscrol2=30.; dscroll=(100.-dscrol2)/6. ;}
 else if (iflag ==  2 ) { dscrol2= 0.; dscroll=(100.-dscrol2)/4. ;}

 else if (iflag == -1 ) { dscrol2=95.; dscroll=(100.-dscrol2)/2. ;
                          numquery=4 ;  uslptim=1000000; goto docenter; }
 else if (iflag == -2 ) { dscrol2=80.; dscroll=(100.-dscrol2)/2. ;
                          numquery=2 ;   uslptim=500000; goto domove  ; }
 else if (iflag == -3 ) { dscrol2= 0.; dscroll=(100.-dscrol2)/3. ;
                          numquery=2 ;   uslptim=500000; goto doquery ; }
 else if (iflag == -4 ) { dscrol2= 0.; dscroll=(100.-dscrol2)/2. ;
                          numquery=2 ;   uslptim=500000; goto dohall  ; }

 else if (iflag == -11) { dscrol2=100.; dscroll=0.; las_ask=LAS_ON  ; goto dolason ; }
 else if (iflag == -12) { dscrol2=100.; dscroll=0.; las_ask=LAS_OFF ; goto dolasoff; }

 //end header, begin subroutine

 //-------------------------start execute box initialization


 icmd=cmdrdth;                      //temp humid
 if ( bburst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 if (iflag > 1) goto endbox ;  // do not execute for inifastfast

 icmd=cmdqwdc;                      //watch dog
 if ( bburst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 if (iflag > 0) goto endbox ;  // do not execute for inifast

 icmd=cmdsdvn;                      //version
 if ( bburst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

dolasoff:
 icmd=cmdnlasf;                      //laser off
 if ( bburst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto endlas ;   // break pressed ?
 if ( iflag > 0) goto endlas ;       //not part of init
 else            goto endbox ;       //init does never execute lason

dolason:
 icmd=cmdnlasn;                      //laser on
 if ( bburst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto endlas ;   // break pressed ?
 if ( iflag > 0) goto endlas ;      //not part of init

endbox:

//-------------------------start execute act initialization

 nmaxcurr=410;                                  //no current needed
 steps=20 ;

 icmd=cmdrdth;                      //temp humid
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 icmd=cmdr4ms;                      //read hall
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 icmd=cmdrdmt;                      //query
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?
 if (iflag > 1 || iflag <0 ) goto endact ;  // do not execute for inifastfast or actuator commands



 icmd=cmdqwdc;                      //watch dog
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?
 if (iflag > 0) goto inicenter ; // maybe have to do center even for inifast



 sprintf(tascii,"init: power on\n");
 log_write(fdisk, tascii,  &k);
 icmd=cmdpwon;                      //power on
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 icmd=cmdsdvn;                      //version
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 icmd=cmdkill;                      //stop motor movement
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 icmd=cmdrdfq;                      //frequency
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 if(AMC_doadjall != 0) {
  icmd=cmdwtfq;                      //frequency
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

  icmd=cmdrdfq;                      //frequency
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?
 }

 icmd=cmdrdwc;                      //working current
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 if(AMC_doadjall != 0) {
  icmd=cmdwtwc;                      //working
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

  icmd=cmdrdwc;                      //working
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?
 }

 icmd=cmdrdhc;                      //holding current
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 if(AMC_doadjall != 0) {
  icmd=cmdwthc;                      //holding
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

  icmd=cmdrdhc;                      //holding
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?
 }

 icmd=cmdrdusw;                     //read upper endswitch
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 icmd=cmdrdlsw;                     //read lower endswitch
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

 if(AMC_doadjall != 0) {
   //debug output HA
   sprintf(tascii,"endswitches read back\n");
   log_write(fdisk, tascii,  &k);
   for(px= -8;px< 9;px++)
    for(py= -8;py< 9;py++) {
     if( panel[px+8][py+8].pan_inst <= 0) goto notinst ;
     sprintf(tascii,"px= %d,  py=%d\n",px,py);
     log_write(fdisk, tascii,  &k);
     for(it=0;it<2;++it) {
      sprintf(tascii,"rndsw: ad= %2.2x, low/up: %d %d %d %d\n"
      , panel[px+8][py+8].addr[it]
      , panel[px+8][py+8].rndsw_lwn[it]
      , panel[px+8][py+8].rndsw_lwl[it]
      , panel[px+8][py+8].rndsw_upn[it]
      , panel[px+8][py+8].rndsw_upl[it] );
      log_write(fdisk, tascii,  &k);
      sprintf(tascii,"endsw: ad= %2.2x, low/up: %d %d %d %d;  %d %d\n"
      , panel[px+8][py+8].addr[it]
      , panel[px+8][py+8].endsw_lwn[it]
      , panel[px+8][py+8].endsw_lwl[it]
      , panel[px+8][py+8].endsw_upn[it]
      , panel[px+8][py+8].endsw_upl[it]
      , panel[px+8][py+8].rndsw_lwl[it] - panel[px+8][py+8].endsw_lwl[it]
      , panel[px+8][py+8].rndsw_upl[it] - panel[px+8][py+8].endsw_upl[it] );
      log_write(fdisk, tascii,  &k);
     }
     notinst: ;
    }

  sprintf(tascii,"init: write low esw\n");
  log_write(fdisk, tascii,  &k);
  icmd=cmdwtlsw;                     //write lower endswitch
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

  sprintf(tascii,"init: write upp esw\n");
  log_write(fdisk, tascii,  &k);
  icmd=cmdwtusw;                     //write upper endswitch
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

  sprintf(tascii,"init: read upp esw\n");
  log_write(fdisk, tascii,  &k);
  icmd=cmdrdusw;                     //read upper endswitch
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

  sprintf(tascii,"init: read low esw\n");
  log_write(fdisk, tascii,  &k);
  icmd=cmdrdlsw;                     //read lower endswitch
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

  sprintf(tascii,"init: read range esw\n");
  log_write(fdisk, tascii,  &k);
  icmd=cmdrdfr;                   //endswitches and range
  if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?

   //debug output HA
   sprintf(tascii,"endswitches read back2\n");
   log_write(fdisk, tascii,  &k);
   for(px= -8;px< 9;px++)
    for(py= -8;py< 9;py++) {
     if( panel[px+8][py+8].pan_inst <= 0) goto notinst1 ;
     sprintf(tascii,"px= %d,  py=%d\n",px,py);
     log_write(fdisk, tascii,  &k);
     for(it=0;it<2;++it) {
      sprintf(tascii,"rndsw: ad= %2.2x, low/up: %d %d %d %d\n"
      , panel[px+8][py+8].addr[it]
      , panel[px+8][py+8].rndsw_lwn[it]
      , panel[px+8][py+8].rndsw_lwl[it]
      , panel[px+8][py+8].rndsw_upn[it]
      , panel[px+8][py+8].rndsw_upl[it] );
      log_write(fdisk, tascii,  &k);
      sprintf(tascii,"endsw: ad= %2.2x, low/up: %d %d %d %d;  %d %d\n"
      , panel[px+8][py+8].addr[it]
      , panel[px+8][py+8].endsw_lwn[it]
      , panel[px+8][py+8].endsw_lwl[it]
      , panel[px+8][py+8].endsw_upn[it]
      , panel[px+8][py+8].endsw_upl[it]
      , panel[px+8][py+8].rndsw_lwl[it] - panel[px+8][py+8].endsw_lwl[it]
      , panel[px+8][py+8].rndsw_upl[it] - panel[px+8][py+8].endsw_upl[it] );
      log_write(fdisk, tascii,  &k);
     }
     notinst1: ;
    }
 }



inicenter:

 put_logfile(LOG_DB8, 0, "ACT init: centre if needed\0");

 //INIT:. only center if needed  ==> check if actuator was never centered
 for (k=0; k<nport; k++) {
   npoint[k]=0;
   for(i=0;i<700;i++) addr2indx[k][i]=-1;
   for(i=0;i<n150;i++) chpoint[k][i]=-1;
 }
 for (j=0;j<totact;j++){
   k=xport[j]%8 - xport[j]/8;
   l=xact[j] ;
   chpoint[k][npoint[k]]=-1;
   if ( ( (*pa[j]).tmp_sel > 0 ) &&
        ( (*pa[j]).err_cnt[0]+(*pa[j]).err_cnt[1] < AMC_errmax*1.5 ) &&
        ( abs( (*pa[j]).act_mot[0]) >5000  ||
          abs( (*pa[j]).act_mot[1]) >5000  ||
          (*pa[j]).status[0][2] & 0x08 ==0 ||
          (*pa[j]).status[1][2] & 0x08 ==0 ) ) {
      (*pa[j]).tmp_sl2 = 1 ;
      byte[0]=xcntr[j][0];
      byte[1]=xcntr[j][1];
      i=(*word)-0x8000;
      if (i>1023 || i<0 ) continue;
      addr2indx[k][i] = npoint[k];
      chpoint[k][npoint[k]++]=j;
   } else {
     (*pa[j]).tmp_sel = -1 ;   //'deselect' panels not to be centered
     (*pa[j]).tmp_sl2 = -1 ;   //'deselect' panels not to be centered
   }
 }


 //anything to do?
 acttot=0;
 for(k=0;k<nport;k++) acttot += npoint[k];
 if( acttot == 0 ){ AMC_scroll+= dscrol2+2*dscroll; goto endact ;}




docenter:
 //in all cases, set pc_mot=0 for all selected panels that ask for center
 for (j=0; j<totact;j++)
   if ( (*pa[j]).tmp_sel > 0 )
      (*pa[j]).pc_mot[0] = (*pa[j]).pc_mot[1] = 0 ;


 sprintf(tascii," centre\n");
 log_write(fdisk, tascii,  &k);
 steps=8000 ;
 nmaxcurr=(2.*macu-500.*AMChold)/(AMCwork-AMChold)-2.; //max current available
 icmd=cmdcntr;                       //centre
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscrol2 ) !=0 ) goto end ;   // break pressed ?
 steps= 20;
 nmaxcurr=410;
 numquery=4 ;
 uslptim=1000000;
 goto doquery ;




 domove:
 sprintf(tascii," moveby\n");
 log_write(fdisk, tascii,  &k);
 steps=20 ;
 nmaxcurr=(2.*macu-500.*AMChold)/(AMCwork-AMChold)-2.; //max current available
 icmd=cmdmvx ;                      //move rel
 if ( burst_and_get( icmd, cleared_bits, steps, 0 , dscrol2 ) !=0 ) goto end; ; //not allowed to re-send !!!!
 steps= 20;
 nmaxcurr=410;
 numquery=2 ;
 uslptim= 250000;
 goto doquery ;




doquery:
 iquery=0 ;

more_query:
 for (iquery=0; iquery < numquery; iquery++) {
   if ( iquery == 0) {
      dscrol4=dscroll ;
   } else {
      usleep(uslptim * iquery) ;
      dscrol4=0 ;
   }
   if ( iquery == numquery ) {
      cleared_bits=0;            //in last possible iteration we MUST accept
   } else {
      cleared_bits=1;
   }

   sprintf(tascii,": centre-->query\n");
   log_write(fdisk, tascii,  &k);
   icmd=cmdrdmt;                      //
   if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscrol4 ) !=0 ) goto end ;   // break pressed ?

   for(k=0;k<nport;k++) { acttot +=npoint[k];}
   if (acttot ==0 ) goto end_query ;
 }

 end_query:
 cleared_bits=0 ;
 for (k=0; k<nport; k++) {
   npoint[k]=0;
   for(i=0;i<700;i++) addr2indx[k][i]=-1;
   for(i=0;i<n150;i++) chpoint[k][i]=-1;
 }
 for (j=0;j<totact;j++){
   k=xport[j]%8 - xport[j]/8;
   l=xact[j] ;
   chpoint[k][npoint[k]]=-1;
   (*pa[j]).tmp_sel = (*pa[j]).tmp_sl2 ;  //reset before query loop
   if ( ( (*pa[j]).tmp_sel > 0 ) &&
        ( (*pa[j]).err_cnt[0]+(*pa[j]).err_cnt[1] < AMC_errmax*1.5 ) ) {
      byte[0]=xcntr[j][0];
      byte[1]=xcntr[j][1];
      i=(*word)-0x8000;
      if (i>1023 || i<0 ) continue;
      addr2indx[k][i] = npoint[k];
      chpoint[k][npoint[k]++]=j;
   } 
 }


dohall:
 cleared_bits=0 ;
 sprintf(tascii,"init: hall\n");
 log_write(fdisk, tascii,  &k);
 icmd=cmdr4ms;                      //read 4 magnetic sensors (hall)
 if ( burst_and_get( icmd, cleared_bits, steps, nroundmax , dscroll ) !=0 ) goto end ;   // break pressed ?



endact:


//----------------------------------


end:          //have to check if pc_mot == act_mot , else something fichy ...
 for (j=0;j<totact;j++)
   if ( (*pa[j]).pan_sel > 0 ) 
      if ( (*pa[j]).err_cnt[0]+(*pa[j]).err_cnt[1] > AMC_errmax*1.5 )
         (*pa[j]).pan_stat = STAT_COM ;
                //communication problem

      else if ( abs((*pa[j]).pc_mot[0] - (*pa[j]).act_mot[0] ) <  AMCstepmin  &&
                abs((*pa[j]).pc_mot[1] - (*pa[j]).act_mot[1] ) <  AMCstepmin  ) {
                //correctly moved
         (*pa[j]).pan_stat  = STAT_OK_ ;
         (*pa[j]).pc_mot[0] = (*pa[j]).act_mot[0] ;
         (*pa[j]).pc_mot[1] = (*pa[j]).act_mot[1] ; }

      else if ( abs((*pa[j]).act_mot[0]) > 6000  ||
                abs((*pa[j]).act_mot[1]) > 6000  )
         (*pa[j]).pan_stat = STAT_NIN ;
                //not initialized

      else if ( iflag>0 &&
                abs((*pa[j]).act_mot[0]) < 6000 &&
                abs((*pa[j]).act_mot[1]) < 6000 ) {
                //successfully initialized
         (*pa[j]).pan_stat  = STAT_OK_ ;
         (*pa[j]).pc_mot[0] = (*pa[j]).act_mot[0] ;
         (*pa[j]).pc_mot[1] = (*pa[j]).act_mot[1] ; }

      else
         (*pa[j]).pan_stat = STAT_ERR ;
                //error

 return(0) ;


endlas:
 for (j=0;j<totbox;j++)
   if ( (*pb[j]).pan_sel > 0 )
      if ( (*pb[j]).err_cnt[2] > AMC_errmax ) {
         (*pb[j]).pan_stat = STAT_COM ; (*pb[j]).laser = LAS_UDF ; }

      else if ( (*pb[j]).laser == las_ask ) 
         (*pb[j]).pan_stat = STAT_OK_ ;

      else
         (*pb[j]).pan_stat = STAT_ERR ;

 return(0) ;
}




