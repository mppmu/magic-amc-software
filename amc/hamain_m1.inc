//called from AMC_init
//included in AMC_init

//initialize everything

//open log files

     AMCtime(  utime ) ;
     sprintf(logfilename,"AMC1log_%06ld_%06ld.log",utime[1],utime[0]);
     fdisk = open(logfilename,1);
     fdisk = creat(logfilename,0xFFFF);


     AMCtime(  utime ) ;
     sprintf(logfilename,"AMC1log2_%06ld_%06ld.log",utime[1],utime[0]);
     if(AMCwritelog2 != 0) fdisk2= open(logfilename,1);
     if(AMCwritelog2 != 0) fdisk2= creat(logfilename,0xFFFF);



//simulate something
// default LED position
    ledpos0[0][0]=170;
    ledpos0[1][0]=140;
    ledpos0[0][1]=675;
    ledpos0[1][1]=150;
    ledpos0[0][2]=180;
    ledpos0[1][2]=500;
    ledpos0[0][3]=680;
    ledpos0[1][3]=520;
    printf("Default LED positions defined \n");
//
// default LASER position
//    for(i=0;i<nport;i++) for(j=0;j<nboxmax;j++)  {
//    printf("Defining [j][i] %d %d \n",j,i);
//     laspos0[0][j][i]=(0.5-randflat())*(nax-nix)*0.3+(nix+nax)/2;
//     laspos0[1][j][i]=(0.5-randflat())*(nay-niy)*0.3+(niy+nay)/2;
//    }

    printf("Default positions defined \n");

    #include "command_code.h"

    cmdncnt = cmdcntr;    // no new centre anymore; do cneter



//initialize to -1  |  -10
    for(j=0;j<npany;++j) for(i=0;i<npanx;++i) {ij2npan[i][j]=-1;}
    for(j=0;j<n8;++j) {nbox[j]=-1;}
    for(j=0;j<300;++j)    {
     ipan[j]=-10;
     jpan[j]=-10;
    }

    for(i=0;i<0x500;i++) {actaddr2npan[i]=-1; boxaddr2npan[i]=-1;
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
         if(k!=0) panel[px+8][py+8].portflg[i]=0;
         k=0;
         po[i]=panel[px+8][py+8].port[k][i];   //port

         ap[i]=panel[px+8][py+8].port[1][i];   //alternate port
//       ad[i]=panel[px+8][py+8].addr[i];      //address
         ad[i]=panel[px+8][py+8].addr[0];      //use driver address also for controller

         if(ad[i] >0x900) { printf("wrong address \b %d\n",ad[i]); sleep(2);
                                       ad[i]=panel[px+8][py+8].addr[i]=000; }

        }
        for(k=0;k<1;++k) {      //only one address for both motors
         panel[px+8][py+8].freq[k]=-9;
         panel[px+8][py+8].work[k]=-9;
         panel[px+8][py+8].hold[k]=-9;
         actaddr2npan[ad[k]]=npanel;
         itfromactaddr[ad[k]]=k;
         pa[totact]=&panel[px+8][py+8];
         *word=ad[k];
         xcntr[totact][0]=byte[0];             //AMC 1 box address
         xcntr[totact][1]=byte[1];             //1 - 4 sub address of motors
         sprintf(Rascii,"List: act have %2x %x %d\n",xcntr[totact][0],xcntr[totact][1],po[k]);
         xport[totact++]=po[k];
         log_write(fdisk, Rascii,  &nascii);
        }

        pb[totbox]=&panel[px+8][py+8];
        boxaddr2npan[ad[2]]=npanel;
        *word=ad[2];
        xxcntr[totbox][0]=byte[0];
        xxcntr[totbox][1]=0xff;           //always this
        sprintf(Rascii,"List: box have %2x %x %d\n",xxcntr[totbox][0],xxcntr[totbox][1],po[2]);
        log_write(fdisk, Rascii,  &nascii);
        sprintf(Rascii,"List: box totbox=%d, pnmbr=%d\n", totbox, (*pb[totbox]).pnmbr );
        log_write(fdisk, Rascii,  &nascii);
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
        panel[px+8][py+8].laser=LAS_UDF;
        j=gettimeofday( tv, tz);
        mainusec0=atv.tv_usec;
        mainsec0=atv.tv_sec ;
        panel[px+8][py+8].time[0]=mainusec0;
        panel[px+8][py+8].time[1]=mainsec0;


       } //is>0
     } //   end py, px,

printf("totact %d, totbox %d\n",totact,totbox);


//find panels controlled by the same box/controller
     for(px=-8;px<9;px++)
      for(py=-8;py<9;py++) {
       panel[px+8][py+8].samebox[0]=-1;
       panel[px+8][py+8].samebox[1]=-1;
       panel[px+8][py+8].samebox[2]=-1;
       if(panel[px+8][py+8].pan_inst > 0 ) {
        for(qx=-8;qx<9;qx++)
         for(qy=-8;qy<9;qy++) {
          if( (px != qx) | (py != qy) )
          if(panel[qx+8][qy+8].pan_inst > 0 ) {
           if(panel[px+8][py+8].port[0][0] ==  panel[qx+8][qy+8].port[0][0] )
           if(panel[px+8][py+8].addr[0]%0x100 ==  panel[qx+8][qy+8].addr[0]%0x100 ) {
            k=0;
            if(panel[px+8][py+8].samebox[k]< 0) {
             panel[px+8][py+8].samebox[k]=panel[qx+8][qy+8].pnmbr;
            } else {
             k=1;
             if(panel[px+8][py+8].samebox[k]< 0) {
              panel[px+8][py+8].samebox[k]=panel[qx+8][qy+8].pnmbr;
             } else {
              panel[px+8][py+8].samebox[2]=panel[qx+8][qy+8].pnmbr;
             }
            }
           }
          }
         }
       }
//     if(panel[px+8][py+8].samebox[0] >= 0 )
//      printf("x %d, y %d, pnmbr %d ,same %d %d %d\n\n", px,py,panel[px+8][py+8].pnmbr
//        ,panel[px+8][py+8].samebox[0]
//        ,panel[px+8][py+8].samebox[1]
//        ,panel[px+8][py+8].samebox[2] );
     }

                                                                                           

// verification output
//  printf("verification output \n");
//  for(k=0; k<npanel; ++k) {
//   printf("ipan=%d, jpan=%d, npan=%d; %d %d %d\n",ipan[k], jpan[k], k
//   ,panel[ipan[k]+8][jpan[k]+8].samebox[0]
//   ,panel[ipan[k]+8][jpan[k]+8].samebox[1]
//   ,panel[ipan[k]+8][jpan[k]+8].samebox[2]
//   );
//  }

//    for(iport=0; iport<nport; iport++) {
//     printf("iport %d   \n",iport);
//     for(k=0; k<3; ++k) {
//      for(i=0; i<nbox[iport]; i++) {
//       printf("port %d ",apo[k][i][iport]);
//       printf("box%d ",abox[k][i][iport] );
//       for(j=0; j<ndriv[i][iport]; j++)
//        printf(";  pan i %d j %d ", ipan[i][iport], jpan[i][iport]);
//       printf("\n");
//      }
//     }
//    }

//    for(j=0;j<npany;++j) for(i=0;i<npanx;++i)
//     if(ij2npan[i][j] > -1) {
//      k=ij2npan[i][j];
//      po1=popan[k];
//      bo1=bopan[k];
//      printf("panel i %d %d  j %d %d \n",i-8,ipan[bo1][po1],j-8,jpan[bo1][po1]);
//    }


//...................................................................................

/*
    printf("opening "); printf( PANELPOSFILE ); printf(" \n");
    parampos = fopen(PANELPOSFILE,"r");                //for now take the old one
    if(parampos == NULL) { printf("can't open "); printf(PANELPOSFILE); printf("\n");
    } else {
     printf("did open "); printf(PANELPOSFILE); printf(" \n");
newinp:
     if( fgets(linepos,109,parampos) != NULL ) {
      if(linepos[0] != '#' ) {
       for(i=0;i<100;i++) if(linepos[i] == 9) linepos[i]=32;
       sscanf(linepos, "%d %d %d %d %f %f %f %f %f %f %f %f %d %d ",
        &ii, &ij, &iX, &iY, &dLaserX, &dLaserY,
        &dAxisX, &dSlopeX, &dConvX, &dAxisY, &dSlopeY, &dConvY,
        &iIsReversed, &iIsInstalled );

//     printf("%2.1d %2.1d %4.3d %4.3d %7.2lf %7.2lf %8.2lf %8.3lf %8.3lf %8.2lf %8.3lf %8.3lf %d %d \n",
//      ii, ij, iX, iY, dLaserX, dLaserY,
//      dAxisX, dSlopeX, dConvX, dAxisY, dSlopeY, dConvY,
//      iIsReversed, iIsInstalled );

       if(panel[ii+8][ij+8].pan_inst>0)
       if(iIsInstalled != 0 ) {
        k=ij2npan[ii+8][ij+8];
        if(k < 0 ) { printf("Consequtive panel number < 0; i %d, j %d, k %d \n",ii,ij,k);
        } else {

         panel[ii+8][ij+8].ladj_xlas=dLaserX;
         panel[ii+8][ij+8].ladj_ylas=dLaserY;
//       panel[ii+8][ij+8].cali_slp[0]=PI * dSlopeX/180.;     //HA 21.112011
//       panel[ii+8][ij+8].cali_slp[1]=PI * dSlopeY/180.;     //
         panel[ii+8][ij+8].cali_slp[0]=atan(dSlopeX);         //
         panel[ii+8][ij+8].cali_slp[1]=atan(dSlopeY);         //HA 21.11.2011
         panel[ii+8][ij+8].cali_dca[0]=dAxisX;
         panel[ii+8][ij+8].cali_dca[1]=dAxisY;
         panel[ii+8][ij+8].cali_spp[0]=dConvX;
         panel[ii+8][ij+8].cali_spp[1]=dConvY;
         if(iIsReversed != 0 ) {
//        printf("IsReversed is set for %d, %d \7\7\7\7\n",ii,ij);
          panel[ii+8][ij+8].cali_slp[0]=atan(1./dSlopeX);     //HA 21.112011
          panel[ii+8][ij+8].cali_slp[1]=atan(1./dSlopeY);     //
          panel[ii+8][ij+8].cali_spp[0]=-dConvX;              //
          panel[ii+8][ij+8].cali_spp[1]=-dConvY;              //HA 21.112011
         }
        }
       }
      }    //not #
      if((ii != 8) | (ij != 8) ) goto newinp;
     }     //line not NULL
     printf("closing \n");
     fclose(parampos);
    }      //file was open
 */
//.........................................................................

//***********************************************************

//read files calib*.hal        does not exist

//***********************************************************

//read file gallendsw.doc      does not exist


//.........................................................................


  open_485();        //do all OPEN of communication ports


for(i=0;i<10;i++) {
 res=init_USCM( Rascii );
 printf("prompted: %1d,   registers: %2.2x,%2.2x,%2.2x,%2.2x\n"
        ,prompted,USCMreg[0],USCMreg[1],USCMreg[2],USCMreg[3]);
 if(res==0) break;
 if(res==-999) break;    //no uscm
 sleep(2);
}
//if(res<0) exit(0);
printf("@%s\n",ascii);



printf("exiting startup in hamain\n");

