//called from AMC_init
//included in AMC_init

//initialize everything

//open log files


if (flag ==0 ) {
     AMCtime(  utime ) ;
     sprintf(logfilename,"AMClog_%06d_%06d.log",utime[1],utime[0]);
     fdisk = open(logfilename,1);
     fdisk = creat(logfilename,0xFFFF);


     AMCtime(  utime ) ;
     sprintf(logfilename,"AMClog2_%06d_%06d.log",utime[1],utime[0]);
     if(AMCwritelog2 != 0) fdisk2= open(logfilename,1);
     if(AMCwritelog2 != 0) fdisk2= creat(logfilename,0xFFFF);
}

    #include "command_code.h"

    cmdncnt = cmdcntr;    // no new centre anymore; execute center if new-center asked



//initialize to -1  |  -10
    for(j=0;j<npany;++j) for(i=0;i<npanx;++i) {ij2npan[i][j]=-1;}
    for(j=0;j<n8;++j) {nbox[j]=-1;}
    for(j=0;j<300;++j)    {
     ipan[j]=-10;
     jpan[j]=-10;
    }

    for(i=0;i<1024;i++) {actaddr2npan[i]=-1; boxaddr2npan[i]=-1;
                         itfromactaddr[i]=-1;}

    for(j=0;j<n8;++j) { nbox[j]=0;
     for(i=0;i<nboxmax;++i) { abox[i][j]=-1; }
    }
    npanel=0;


//***********************************************************
//*****************************************************************

     totact=0; 
     totbox=0;
     for(i=0;i<nport;i++)  nactu[i]=0;
     for(px=-8;px<9;px++)
      for(py=-8;py<9;py++) {
       panel[px+8][py+8].ix=px;
       panel[px+8][py+8].iy=py;

       for(k=0;k<3;++k) {
//        if(panel[px+8][py+8].port[0][k] > panel[px+8][py+8].port[1][k] ) {
//         i=panel[px+8][py+8].port[0][k];
//         panel[px+8][py+8].port[0][k] = panel[px+8][py+8].port[1][k] ;
//         panel[px+8][py+8].port[1][k]=i;
//        }
        panel[px+8][py+8].ech2_cnt[k][0]=0;
        panel[px+8][py+8].ech2_cnt[k][1]=0;
       }

       for(k=0;k<2;++k) {
        panel[px+8][py+8].act_mot[k]= 12345;
        panel[px+8][py+8].pc_mot[k] = 54321;
        panel[px+8][py+8].endsw_lwn[0] = panel[px+8][py+8].endsw_lwn[1] = 0;
        panel[px+8][py+8].endsw_lwl[0] = panel[px+8][py+8].endsw_lwl[1] = 570;
        panel[px+8][py+8].endsw_upn[0] = panel[px+8][py+8].endsw_upn[1] = 3;
        panel[px+8][py+8].endsw_upl[0] = panel[px+8][py+8].endsw_upl[1] = 430;
       }
       if(panel[px+8][py+8].pan_inst>0) {
        panel[px+8][py+8].pnmbr=npanel;
        for(i=0;i<3;i++) {
         k =panel[px+8][py+8].portflg[i];      //portflag
         if(k!=0) k=1;
         po[i]=panel[px+8][py+8].port[k][i];   //port
         ap[i]=panel[px+8][py+8].port[1][i];   //alternate port
         ad[i]=panel[px+8][py+8].addr[i];      //address

         if(ad[i] < 0x8000) { printf("wrong address \b %4.4x\n",ad[i]); sleep(2);
                              ad[i]=panel[px+8][py+8].addr[i]=0x8000;
                              if(i == 2) ad[i]=panel[px+8][py+8].addr[i]=0x8800;}
         if(i < 2) if(ad[i] > AMCactmaxadd) {printf("wrong address \b %4.4x\n",ad[i]); sleep(2);
                                       ad[i]=panel[px+8][py+8].addr[i]=0x8000; }

         panel[px+8][py+8].watsrc[i]=63;
         panel[px+8][py+8].watch0[i]=0;
         panel[px+8][py+8].watch1[i]=0;
        }
        for(k=0;k<2;++k) {
         panel[px+8][py+8].freq[k]=-9;
         panel[px+8][py+8].work[k]=-9;
         panel[px+8][py+8].hold[k]=-9;
         actaddr2npan[ad[k]-0x8000]=npanel;
         itfromactaddr[ad[k]-0x8000]=k;
         pa[totact]=&panel[px+8][py+8];
         *word=ad[k];
         xcntr[totact][0]=byte[0];
         xcntr[totact][1]=byte[1];
         xact[totact]=k;
         xport[totact++]=po[k];
         sprintf(Rascii,"List: act have %4.4x %d\n",*word,po[k]);
         log_write(fdisk, Rascii,  &nascii);
        }

        pb[totbox]=&panel[px+8][py+8];
        boxaddr2npan[ad[2]-0x8800]=npanel;
        *word=ad[2];
        sprintf(Rascii,"List: box have %4.4x %d\n",*word,po[2]);
        log_write(fdisk, Rascii,  &nascii);
        xxcntr[totbox][0]=byte[0];
        xxcntr[totbox][1]=byte[1];
        xxport[totbox++]=po[2];

        ij2npan[px+8][py+8]=npanel;            //panel number given by x,y coordinates
        ipan[npanel]=px;                       //panel coordinates from panel number
        jpan[npanel]=py;
        npanel=npanel+1;                       //number of panels read in so far

//reflecting the chainwise cabling topology
        for(i=0;i<2;i++) {
         if(po[i] >= 0) {
          po0=po[i]; if(po0>=n8) po0-=8;
          adact[po0][nactu[po0]-1]=ad[i];     //address of last one
          panum[po0][nactu[po0]-1]=npanel-1;  //panel number of last one
          nactu[po0]++;                         //number of actuators in this chain
         }
        }

        if(po[0] >= 0) {
         po0=po[0];  if(po0>=n8) po0-=8;
         i=nbox[po0];                           //virtual chains
         nbox[po0]++;
         abox[i][po0]=npanel-1;                 //panel index (number-1) by box_pos,port
         actv[i][po0]=is;                       //installed
        }


        for(k=0;k<2;++k) {
         panel[px+8][py+8].freq[k]=AMCfreq;
         panel[px+8][py+8].work[k]=AMCwork*1000;
         panel[px+8][py+8].hold[k]=AMChold*1000;
         panel[px+8][py+8].act_mot[k]= init_step;
         panel[px+8][py+8].pc_mot[k]= -init_step;
        }
        panel[px+8][py+8].pan_stat=STAT_NIN;
        panel[px+8][py+8].laser=LAS_OFF;
        j=gettimeofday( tv, tz);
        mainusec0=atv.tv_usec;
        mainsec0=atv.tv_sec ;
        panel[px+8][py+8].time[0]=mainusec0;
        panel[px+8][py+8].time[1]=mainsec0;


       } //is>0
     } //   end py, px,

printf("totact %d, totbox %d\n",totact,totbox);

//***********************************************************


if (flag == 0 ) {       //xzxzxzxzxzxzxzxzxzxzxzxzxz

//read files calib*.hal
   for(px=-8;px<9;px++)
    for(py=-8;py<9;py++) {
     panel[px+8][py+8].numpos[0]=0 ;                  //number of positions measured
     panel[px+8][py+8].numpos[1]=0 ;                  //number of positions measured
     if(panel[px+8][py+8].pan_inst>0) {
      for(it=0;it<2;it++) {
       k=0;
       if( panel[px+8][py+8].addr[it] < 0x8000) continue;
       sprintf(calibfile,"/home/amc/calib/calibf%4.4x.hal\0",panel[px+8][py+8].addr[it]);
//D    printf("open     %s \n",calibfile);
       param = fopen(calibfile,"r");       
       if(param == NULL) { printf("can't open  %s\n",calibfile);
       } else {
newincal:
        if( fgets(line,88,param) != NULL) {
         if(line[0] != '#' ) {
          for(i=0;i<90;i++) if(line[i] == 9) line[i]=32;
          if(line[0] < ' ') goto noscan;

//        uhr            messuhr
//        step           steps
//        bval[4]        hall probe value
//        dbds[4]        derivative of bval

          sscanf(line,
            "%f  %f  %f    %f    %f    %f    %f    %f    %f    %f",
             &u, &s, &b[0],&b[1],&b[2],&b[3],&d[0],&d[1],&d[2],&d[3]);
          panel[px+8][py+8].uhr[it][k]=u ;            //Messuhr position (micron)
          panel[px+8][py+8].step[it][k]=s ;           //stepping motor position
          for(l=0;l<4;l++) {
           panel[px+8][py+8].bval[it][k][l]=b[l] ;    //hall value
           panel[px+8][py+8].dbds[it][k][l]=d[l] ;    //derivative: d bval/ d step
          }
          k++;
         }    //not #
         goto newincal;
        }     //line not EOF
noscan: ;
        panel[px+8][py+8].numpos[it]=k ;                 //number of positions measured
        panel[px+8][py+8].llw_mot[it]=panel[px+8][py+8].step[it][k-1];// lower limit of motor position
        panel[px+8][py+8].lup_mot[it]=panel[px+8][py+8].step[it][0];  // upper limit of motor position
//D     printf("did read  %s: %d \n",calibfile,panel[px+8][py+8].numpos[it]);
        fclose(param);
       }  //opened
      }   //it
     }    //inst>0
    }     //py
          //px
 printf("end reading calib* \n");

//***********************************************************

//read file gallendsw.doc
       param = fopen("gallendsw.doc","r"); 
       if(param == NULL) { printf("can't open gallendsw.doc\n");
       } else {
        printf("did open gallendsw.doc\n");
newinendsw:
        if((l=*fgets(line,49,param)) != EOF) {
         if(line[0] != '#' ) {
          for(i=0;i<90;i++) if(line[i] == 9) line[i]=32;
          if(line[0] < ' ') goto noscansw;

          sscanf(line,"%x  %d    %f    %f    %d    %f    %f"
                     , &k, &iel, &rel, &rel1,&ieu, &reu, &reu1);
          px = ipan[ actaddr2npan[k-0x8000] ];       //get panel coordinates
          py = jpan[ actaddr2npan[k-0x8000] ];
          it = itfromactaddr[k-0x8000];              //which motor?
          panel[px+8][py+8].endsw_lwl[it] = rel;     //sensor level for lower endswitch
          panel[px+8][py+8].endsw_upl[it] = reu;     //sensor level for upper endswitch
         }
         goto newinendsw ;
        }
noscansw: ;
        fclose(param);
        printf("gallendsw.doc read\n");
       }


//$
     for(px=-8;px<9;px++)
      for(py=-8;py<9;py++) {
       if( panel[px+8][py+8].pan_inst <= 0) goto notinst ;
       sprintf(Rascii,"px= %d,  py=%d\n",px,py);
       log_write(fdisk, Rascii,  &nascii);
       for(it=0;it<2;++it) {
        panel[px+8][py+8].l600_pos[it] = -10000;  //initialize
        panel[px+8][py+8].u600_pos[it] =  10000;
        panel[px+8][py+8].endsw_lwp[it]= -10000.; //position in steps at lower endsw level
        panel[px+8][py+8].endsw_upp[it]=  10000.; //position in steps at upper endsw level
        i = panel[px+8][py+8].endsw_lwn[it];      //which sensor low?
        j = panel[px+8][py+8].endsw_upn[it];      //which sensor up ?
//step is coming downwards!!
        calflg=0;
        for(k=0;k<panel[px+8][py+8].numpos[it];k++) {
         if(k>0) {
          if( abs(panel[px+8][py+8].step[it][k]-panel[px+8][py+8].step[it][k-1]) > 110) {
           calflg=1;
           sprintf(Rascii,"bad calibf %d:%f %f\n", k, panel[px+8][py+8].step[it][k-1], panel[px+8][py+8].step[it][k]);
           log_write(fdisk, Rascii,  &nascii);
          }
         }

         if( (panel[px+8][py+8].bval[it][k][i] >= panel[px+8][py+8].endsw_lwl[it] ) &&
             (panel[px+8][py+8].step[it][k]    >  panel[px+8][py+8].endsw_lwp[it] ) )
          panel[px+8][py+8].endsw_lwp[it] = panel[px+8][py+8].step[it][k-1];

         if( (panel[px+8][py+8].bval[it][k][j] >= panel[px+8][py+8].endsw_upl[it] ) &&
             (10000                            == panel[px+8][py+8].endsw_upp[it] ) )
          panel[px+8][py+8].endsw_upp[it] = panel[px+8][py+8].step[it][k];

         if( panel[px+8][py+8].bval[it][k][3] > 600 ) {
          if( panel[px+8][py+8].u600_pos[it] ==  10000)
              panel[px+8][py+8].u600_pos[it] = panel[px+8][py+8].step[it][k];
          if( panel[px+8][py+8].u600_pos[it] <   10000)
              panel[px+8][py+8].l600_pos[it] = panel[px+8][py+8].step[it][k];
         }
         panel[px+8][py+8].esw_range[it]
           = panel[px+8][py+8].endsw_upp[it] - panel[px+8][py+8].endsw_lwp[it];
         panel[px+8][py+8].esw_centr[it]
           = ( panel[px+8][py+8].endsw_upp[it] + panel[px+8][py+8].endsw_lwp[it] ) /4;
         panel[px+8][py+8].esw_centr[it]  *= 2;
        }     // for k
        if(calflg > 0 ) {
         sprintf(Rascii,"pos:ad= %2.2x bad calib file\n" ,panel[px+8][py+8].addr[it]);
         log_write(fdisk, Rascii,  &nascii);
        }
        k = panel[px+8][py+8].endsw_upp[it] /2;  //make even
        panel[px+8][py+8].endsw_upp[it] = 2*k;
        k = panel[px+8][py+8].endsw_lwp[it]/2;
        panel[px+8][py+8].endsw_lwp[it] = 2*k;
        panel[px+8][py+8].llw_mot[it]=panel[px+8][py+8].endsw_lwp[it] +10;
        panel[px+8][py+8].lup_mot[it]=panel[px+8][py+8].endsw_upp[it] -10;
        sprintf(Rascii,"pos:ad= %2.2x, 4x pos: %d %d %d %d; range=%d, offset=%d\n"
          , panel[px+8][py+8].addr[it]
          , panel[px+8][py+8].endsw_lwp[it]
          , panel[px+8][py+8].l600_pos[it]
          , panel[px+8][py+8].u600_pos[it]
          , panel[px+8][py+8].endsw_upp[it]
          , panel[px+8][py+8].esw_range[it] 
          , panel[px+8][py+8].esw_centr[it] );
        log_write(fdisk, Rascii,  &nascii);
        sprintf(Rascii,"endsw:ad= %2.2x, low/up: %d %d %d %d soll\n"
          , panel[px+8][py+8].addr[it]
          , panel[px+8][py+8].endsw_lwn[it]
          , panel[px+8][py+8].endsw_lwl[it]
          , panel[px+8][py+8].endsw_upn[it]
          , panel[px+8][py+8].endsw_upl[it] );
        log_write(fdisk, Rascii,  &nascii);

       }      // for it
notinst:; 
      }       // for py
              // for px


//.........................................................................

//.........................................................................

  open_485();        //do all OPEN of communication ports

for(i=0;i<10;i++) {
 res=init_USCM( Rascii );
 printf("prompted: %1d,   registers: %2.2x,%2.2x,%2.2x,%2.2x\n"
        ,prompted,USCMreg[0],USCMreg[1],USCMreg[2],USCMreg[3]);
 if(res==0) break;
 sleep(2);
}
if(res<0) exit(0);

/* for now: dont switch on
if(USCMreg[0] != 0xff) {
 if( USCMreg[0] == 0 ) {
  set_USCM( 3, 0 );
  USCMreg[0] = 3;
 }
 if(USCMreg[0] <= 3 ) {
  set_USCM( 0x0f, 0 );
  USCMreg[0] = 0x0f;
 }
 if(USCMreg[0] <= 0x0f ) {
  set_USCM( 0x3f, 0 );
  USCMreg[0] = 0x3f;
 }
 set_USCM( 0xff, 0 );
}



i=1;
if(USCMreg[i] != 0x3f) {
 set_USCM( 0x3f,i );
 printf("prompted: %1d,   registers: %2.2x,%2.2x,%2.2x,%2.2x\n"
        ,prompted,USCMreg[0],USCMreg[1],USCMreg[2],USCMreg[3]);
 printf("==============================\n");
}
 end: dont switch on */



i=3;
off_USCM( 31 );
printf("prompted: %1d,   registers: %2x,%2x,%2x,%2x\n"
       ,prompted,USCMreg[0],USCMreg[1],USCMreg[2],USCMreg[3]);
printf("==============================\n");


//for(i=0;i<4;i++) status_USCM( &USCMreg[i], i );
allstatus_USCM(  );
printf("prompted: %1d,   registers: %2.2x,%2.2x,%2.2x,%2.2x\n\n"
       ,prompted,USCMreg[0],USCMreg[1],USCMreg[2],USCMreg[3]);
printf("==============================\n");
 
printf("exiting startup in hamain\n");

}  // if (flag == 0 ) {       //xzxzxzxzxzxzxzxzxzxzxzxzxz
