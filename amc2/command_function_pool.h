// // // // AB?!  added tons of missing explicit return statements (else return is value is undefined !!!!!!)

int do_it( unsigned char *addr, int cmdxxxx ) {
    int retcod,retcomd;
    retcod=send_command( addr, cmdbf[cmdxxxx ] );
    if(retcod<0) printf("do_it retcod %d\n",retcod);
    usleep(n5000);
//  retcomd=get_response1alt( lbufalt, &res, resin[cmdxxxx]);
    return(get_response1( lbuf, &res, resin[cmdxxxx ]));
 }

int do_it_slow( unsigned char *addr, int cmdxxxx ) {
    int retcod,retcomd;
    retcod=send_command( addr, cmdbf[cmdxxxx ] );
    usleep(3000000);
 printf("do_it_slow retcod %d \n",retcod);
//  retcomd=get_response1alt( lbufalt, &res, resin[cmdxxxx]);
    return(get_response1( lbuf, &res, resin[cmdxxxx ]));
 }

int do_it_timed( unsigned char *addr, int cmdxxxx, long micsec ) {
    int retcod,retcomd;
    retcod=send_command( addr, cmdbf[cmdxxxx ] );
    usleep(micsec);
 printf("do_it_timed retcod %d micsec %d \n",retcod,micsec);
//  retcomd=get_response1alt( lbufalt, &res, resin[cmdxxxx]);
    return(get_response1( lbuf, &res, resin[cmdxxxx ]));
 }

int do_query( unsigned char *addr, int* motpos ) {
    int retcod;
    retcod=do_it( addr, cmdrdmt);
    if(retcod >= 0) get_int( 2, lbuf, motpos);
  printf("do_query ret %d\n",retcod);
    return(retcod);
 }

int do_move( unsigned char *addr, int steps ) {
    int pos;
    (*word)=  steps;         //move rel x
    cmdbf[cmdmvx ][8]= byte[0];
    cmdbf[cmdmvx ][9]= byte[1];
    nwd= get_int( 2, &cmdbf[cmdmvx ][8], wrdbuf);
    do_it( addr, cmdmvx);
    printf("move s= %d  \n",wrdbuf[0]);
    usleep(n5000);
    return(do_query( addr, &pos));
 }

int do_newcenter( unsigned char *addr ) {
    int pos, retcod;
    retcod=do_it( addr, cmdncnt);
//  do_query( addr, &pos);
//  printf("x= %d \n",pos);
    return(retcod);
 }

int do_center( unsigned char *addr ) {
    int pos;
    int retcod;
    retcod = do_it( addr, cmdcntr);
    do_query( addr, &pos);
    printf("x= %d \n",pos);
    return(retcod);
 }

int do_reset( unsigned char *addr ) {
    return( do_it_slow( addr, cmdresb) );
 }

int do_kill( unsigned char *addr ) {
    return( do_it( addr, cmdkill) );
 }

int do_lason( unsigned char *addr ) { //laser on (new command (0x60)
    return(do_it( addr, cmdnlasn));
 }

int do_lasoff( unsigned char *addr ) { //laser off (new command (0x61)
    return(do_it( addr, cmdnlasf) );
 }

int do_rdfreq( unsigned char *addr, int *freq ) {
    int retcod;
    retcod=do_it( addr, cmdrdfq);
    get_int( 2, lbuf, freq);
    return(retcod);
 }

int do_wtfreq( unsigned char *addr, int freq ) {
    int retcod;
    (*word)=  freq;        
    cmdbf[cmdwtfq][8]= byte[0];
    cmdbf[cmdwtfq][9]= byte[1];
    retcod=do_it( addr, cmdwtfq);
    return(retcod);
 }

int do_rdwork( unsigned char *addr, int *Iwork ) {
    int retcod;
    retcod=do_it( addr, cmdrdwc);
    get_int( 2, lbuf, Iwork);
    return(retcod);
 }

int do_wtwork( unsigned char *addr, int Iwork ) {
    int retcod;
    (*word)=  Iwork;
    cmdbf[cmdwtwc][8]= byte[0];
    cmdbf[cmdwtwc][9]= byte[1];
    retcod=do_it( addr, cmdwtwc);
    return(retcod);
 }

int do_rdhold( unsigned char *addr, int *Ihold ) {
    int retcod;
    retcod=do_it( addr, cmdrdhc);
    get_int( 2, lbuf, Ihold);
    return(retcod);
 }

int do_wthold( unsigned char *addr, int Ihold ) {
    int retcod;
    (*word)=  Ihold;
    cmdbf[cmdwthc][8]= byte[0];
    cmdbf[cmdwthc][9]= byte[1];
    retcod=do_it( addr, cmdwthc);
    return(retcod);
 }

int do_rdtemp( unsigned char *addr, int *temp, int *humid ) {
    int retcod;
    retcod=do_it_timed( addr, cmdrdth, 100000);
    get_int( 2, lbuf, humid);
    get_int( 2, &lbuf[2], temp);
    return(retcod);
 }

int do_rdepos( unsigned char *addr, int *mpos ) {  //read motor pos from EEPROM
    int retcod;
    retcod = do_it( addr, cmdrdemt);
    get_int( 2, lbuf, mpos);
    return(retcod);
 }

int do_wtspos( unsigned char *addr, int mpos ) {  //write motor pos to SDRAM
    int retcod;
    (*word)=  mpos;
    cmdbf[cmdwtemt][8]= byte[0];
    cmdbf[cmdwtemt][9]= byte[1];
    retcod = do_it( addr, cmdwtmt);
    return(retcod);
 }

int do_wtepos( unsigned char *addr ) {  //write motor pos to EEPROM
//  (*word)=  mpos;
//  cmdbf[cmdwtemt][8]= byte[0];
//  cmdbf[cmdwtemt][9]= byte[1];
    int retcod;
    retcod = do_it_timed( addr, cmdwtemt, 1000000);
    return(retcod);
 }

int do_rdlsw( unsigned char *addr, int *sens_num, int *level ) {  //read sens nmbr and level of lw endsw
    int retcod;
    retcod = do_it( addr, cmdrdlsw);
    *sens_num=lbuf[0];
    get_int( 2, &lbuf[1], level);
    return(retcod);
 }

int do_wtlsw( unsigned char *addr, int sens_num, int level ) {  //write sens nmbr and level of lw endsw
    int retcod;
    cmdbf[cmdwtlsw][8]= sens_num;
    (*word)=  level;
    cmdbf[cmdwtlsw][9]= byte[0];
    cmdbf[cmdwtlsw][10]= byte[1];
    retcod = do_it( addr, cmdwtlsw);
    return(retcod);
 }

int do_rdusw( unsigned char *addr, int *sens_num, int *level ) {  //read sens nmbr and level of up endsw
    int retcod;
    retcod = do_it( addr, cmdrdusw);
    *sens_num=lbuf[0];
    get_int( 2, &lbuf[1], level);
    return(retcod);
 }

int do_wtusw( unsigned char *addr, int sens_num, int level ) {  //write sens nmbr and level of up endsw
    cmdbf[cmdwtusw][8]= sens_num;
    int retcod;
    (*word)=  level;
    cmdbf[cmdwtusw][9]= byte[0];
    cmdbf[cmdwtusw][10]= byte[1];
    retcod = do_it( addr, cmdwtusw);
    return(retcod);
 }

int do_pwof( unsigned char *addr ) {  //motor power off
    return( do_it( addr, cmdpwof) );
 }

int do_pwon( unsigned char *addr ) {  //motor power on
    return( do_it( addr, cmdpwon) );
 }

int do_r4ms( unsigned char *addr, float *sens_addr ) {  //read 4 sensors
    int i,m;
    int retcod;
    for(i=0;i<4;i++) lwrdbuf[i]=0;
    for(i=0;i<4;i++) wrdbuf[i]=0;
    retcod = do_it_timed( addr, cmdr4ms, 5100);
    get_int( 8, lbuf, wrdbuf);
    for(i=0;i<4;i++) { if(wrdbuf[i]<0) lwrdbuf[i]=wrdbuf[i]+65536;
                       else            lwrdbuf[i]=wrdbuf[i];
                       (*(sens_addr+1*i))=lwrdbuf[i]/70.;
//                     m=lwrdbuf[i]/70;
//                     fwrdbuf[i]=10*m;
//                     lwrdbuf[i]=lwrdbuf[i]-m*70;
//                     fwrdbuf[i]=fwrdbuf[i]+lwrdbuf[i];
//                     (*(sens_addr+1*i))=fwrdbuf[i]/10.;
     }
//     printf("locsens= %f %f %f %f \n",fwrdbuf[0],fwrdbuf[1],fwrdbuf[2],fwrdbuf[3]);
    return(retcod);
 }

int do_rdfr( unsigned char *addr, int *full, float *sens_addr ) {  //read fullrange and 8 sensors
    int i,m;
    int retcod;
    retcod = do_it( addr, cmdrdfr );
    get_int( 18, lbuf, wrdbuf);
    *full = wrdbuf[0];
 //printf("full %d\n",*full);
    for(i=0;i<8;i++) { if(wrdbuf[i+1]<0) lwrdbuf[i]=wrdbuf[i+1]+65536;
                       else              lwrdbuf[i]=wrdbuf[i+1];
                       (*(sens_addr+1*i))=lwrdbuf[i]/70.;
//                     m=lwrdbuf[i]/70;
//                     fwrdbuf[i]=10*m;
//                     lwrdbuf[i]=lwrdbuf[i]-m*70;
//                     fwrdbuf[i]=fwrdbuf[i]+lwrdbuf[i];
//                     (*(sens_addr+1*i))=fwrdbuf[i]/10.;
     }
//   printf("magsens= %f %f %f %f \n",fwrdbuf[0],fwrdbuf[1],fwrdbuf[2],fwrdbuf[3]);
    return(retcod);
 }

int do_qwdc( unsigned char *addr, int *reset_cnt, int *source ) {  //query reset_count and source of latest
    int retcod;
    retcod = do_it( addr, cmdqwdc);
    get_int( 1, lbuf, reset_cnt);
    *source=lbuf[2];
    return(retcod);
 }

int do_suid( unsigned char *addr, int *uniq_id ) {  //query unique ID
    int retcod;
    retcod = do_it( addr, cmdsdui);
    get_int( 2, lbuf, uniq_id);
    return(retcod);
 }

int do_sver( unsigned char *addr, int *version ) {  //query version number
    int retcod;
    retcod=do_it( addr, cmdsdvn);
    if(retcod == 0) *version=lbuf[0] ;
    return(retcod);
 }
     
int  adj_freq( unsigned char *addr, int nfreq, int *freq ) {  //adjust frequency
     int retcod;
//read and set frequency
     retcod=do_rdfreq( addr, freq );
     if(retcod != 0) {printf("freq retcod= %d\n",retcod); return(retcod);}
     printf("frequency= %d \n",*freq);
     if(*freq != nfreq) {
      retcod=do_wtfreq( addr, nfreq );
      if(retcod != 0) return(retcod);
      retcod=do_rdfreq( addr, freq );
      if(retcod != 0) return(retcod);
      printf("frequency= %d \n",*freq);
      return(retcod);
     }
    return(retcod);
}

int  adj_wcurr( unsigned char *addr, int nwcurr, int *wcurr ) {  //adjust working current
     int retcod;
     retcod=do_rdwork( addr, wcurr);
     if(retcod != 0) {printf("wcur retcod= %d\n",retcod); return(retcod);}
     printf("working current= %d \n",*wcurr);
     if(abs(*wcurr - nwcurr) > 4) {
      retcod=do_wtwork( addr, nwcurr );
      if(retcod != 0) return(retcod);
      retcod=do_rdwork( addr, wcurr );
      if(retcod != 0) return(retcod);
      printf("working current= %d \n",*wcurr);
      return(retcod);
     }
    return(retcod);
}

int  adj_hcurr( unsigned char *addr, int nhcurr, int *hcurr ) {  //adjust working current
     int retcod;
     retcod=do_rdhold( addr, hcurr);
     if(retcod != 0) {printf("hcur retcod= %d\n",retcod); return(retcod);}
     printf("holding current= %d \n",*hcurr);
     if(abs(*hcurr - nhcurr) > 2) {
      retcod=do_wthold( addr, nhcurr );
      if(retcod != 0) return(retcod);
      retcod=do_rdhold( addr, hcurr );
      if(retcod != 0) return(retcod);
      printf("holding current= %d \n",*hcurr);
      return(retcod);
     }
    return(retcod);
}

