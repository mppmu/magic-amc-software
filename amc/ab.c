// dummy interface to read panel definitions and execute commands


#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <asm/ioctls.h>
#include <string.h>

#include "AMCconst.h"
#include "AMCpanel.h"
#include "AMCpower.h"
#include "AMCgui.h"

int  PWR_read(AMCpower  p[32])
{
  int i,j,k;
     //default  p: 0=off, 1=on, -1=undefined


//AMC I version LPalma
  i = 0;
  p[i].deflt=-1; strncpy( p[i].str,"QUERY",16);         p[i].chan= 0; i++; // does not exist = query only
  p[i].deflt=-9; strncpy( p[i].str,"Laser Cam"    ,16); p[i].chan= 1; i++; // chain 1
  p[i].deflt= 0; strncpy( p[i].str,"SBIG",16);          p[i].chan= 2; i++; // chain 2
  p[i].deflt=-9; strncpy( p[i].str,"Pyromter",16);      p[i].chan= 3; i++; // chain 3
  p[i].deflt= 1; strncpy( p[i].str,"AMC Power",16);     p[i].chan= 4; i++; // chain 4
  p[i].deflt=-9; strncpy( p[i].str,"T-Pnt Cam"  ,16);   p[i].chan= 5; i++; // chain 5
  p[i].deflt=-9; strncpy( p[i].str,"free 12V",16);      p[i].chan= 6; i++; // chain 6
  p[i].deflt=-9; strncpy( p[i].str,"StarG.Cam"    ,16); p[i].chan= 7; i++; // chain 7
  p[i].deflt=-9; strncpy( p[i].str,"Calib Box",16);     p[i].chan= 8; i++; // chain 8

  for (i=0; i<9; i++) p[i].request = -9;

  return 0;
}


int  AMC_read(AMCpanel panel[17][17])
{
  int i,j,k,nchain,ip,ip2,ib,nb,nc,ic,i0,i1,i2,i3,j0,j1,j2,j3,nx;
  int px,py,po[3],ap[3],ps[3],ad[3],is,nmbr;
  float dLaserX, dLaserY, dAxisX, dSlopeX, dConvX, dAxisY, dSlopeY, dConvY;
  FILE* param;
  FILE* parampos;
  char line[1000], linepos[1000];

  int ii, ij, iX, iY, iIsReversed, iIsInstalled;
  float  dLaser[2], dAxis[2], dSlope[2], dConv[2];

//initialize to 'undef'
  nmbr = 0;
  for (j=0; j<17; j++)
    for (i=0; i<17; i++) {
       panel[i][j].loc_x = panel[i][j].loc_y = panel[i][j].loc_z = 0. ;
       panel[i][j].ix = panel[i][j].iy = panel[i][j].pnmbr = -9999;
       panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_DIS;
       panel[i][j].pan_inst =-1;
       panel[i][j].pan_material =-1;
       panel[i][j].act_stat = panel[i][j].box_stat = 0;
       if( abs(i-8)+abs(j-8) > 12 || (i==8 && j==8) || (i==7 && j==8) ) {
          panel[i][j].pan_inst = 0;
          panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_NOT;
       }
//     panel[i][j].laser    = LAS_OFF ;
       panel[i][j].laser    = LAS_UDF ;           //HA 6.6.2011
       panel[i][j].pan_grp  = (j*3 +i)%8+1;
       for (k=0; k<3; k++) {
          panel[i][j].port[0][k]=-1;
          panel[i][j].port[1][k]=-1;
          panel[i][j].portflg[k]=0;
          panel[i][j].addr[k]=0;
          panel[i][j].cpos[k]=-1;
          panel[i][j].bpos[k]= 9;
          panel[i][j].temp[k]=-1;
          panel[i][j].humi[k]=-1;
          panel[i][j].status[k][0]= 0;
          panel[i][j].status[k][1]=0;
          panel[i][j].version[k][0]= 0;
          panel[i][j].version[k][1]= 0;
          panel[i][j].err_cnt[k]= 0;
          panel[i][j].cmd_cnt[k]= 0;
       }
       for (k=0; k<2; k++) {
          panel[i][j].freq[k]=-1;
          panel[i][j].work[k]=-1;
          panel[i][j].hold[k]=-1;
          panel[i][j].act_mot[k]=-8888;
          panel[i][j].pc_mot[k] =-8888;
          panel[i][j].pc_moveto[k] =-8888;
//        panel[i][j].hal_pos[k]=-8888;
//        panel[i][j].hal_val[k][0]=-8888;
//        panel[i][j].hal_val[k][1]=-8888;
//        panel[i][j].hal_val[k][2]=-8888;
//        panel[i][j].hal_val[k][3]=-8888;
          panel[i][j].move_by[k] = 0;
          panel[i][j].actstat[k] = 0;
          panel[i][j].esw_centr[k] = 0;
     }
  }



  param = fopen(PANELSFILE,"r");
  if(param == NULL) { printf("can't open "); printf(PANELSFILE); printf(" \n");
      return -1;
  }
  printf("ab:did open "); printf(PANELSFILE); printf(" \n");
  while (   fgets(line,1000,param) ) {
     if (line[0] != '#') {

//# Panel   Type   RS-488  Box  Driver  Installed
        sscanf(line, "%d  %d  %d     %d     %d     %d     %d",
                     &px,&py,&ap[0],&po[0],&ps[0],&ad[0],&is);
        ap[0]=-1;    ap[1]=-1;    ap[2]=-1;         //no alternate ports
                     po[1]=po[0]; po[2]=po[0];      //all ports are the same

        if( ((activchain >> (po[0]-1)) & 0x01) == 0 ) is=0;


        ad[0]--;
        ad[0]=0x100*ad[0]+ps[0];                   //addr of motor 1: box number + 100* driver position
        ad[1]=ad[0];                               //        motor 2  if it would have an address
        ad[2]=0xff;                                //        laser
        ad[2]=0x100*ad[2]+ps[0];                   //addr of motor 1: box number + 100* driver position
        ad[2]=ad[0];                               //        laser
        ps[0]=-1;    ps[1]=-1;    ps[2]=-1;        //no positions

        if (is !=0) {
           i0 = px +8;
           j0 = py +8;
           panel[i0][j0].ix = px;
           panel[i0][j0].iy = py;
           panel[i0][j0].pnmbr = nmbr++;
           for (k=0; k<3; k++) {
              panel[i0][j0].port[0][k]=abs(po[k])-1;
              panel[i0][j0].port[1][k]=abs(ap[k])-1;
              panel[i0][j0].portflg[k]=0;
              panel[i0][j0].addr[k]=ad[k];
           }
           if ( is>0 )          panel[i0][j0].pan_stat = STAT_NIN ; // not initialized  [blue]
           else if ( is > -99)  panel[i0][j0].pan_stat = STAT_DIS ; // not installed    [gray]
           else                 panel[i0][j0].pan_stat = STAT_COM ; // no communication [brown]
           panel[i0][j0].pan_inst = is;
  }  }  }
  fclose(param);

  printf(" nmbr %d\n", nmbr);


//...................................................................................


    printf("opening "); printf( PANELPOSFILE ); printf(" \n");
    parampos = fopen(PANELPOSFILE,"r");                //for now take the old one
    if(parampos == NULL) { printf("can't open "); printf(PANELPOSFILE); printf("\n");
    } else {
     printf("did open "); printf(PANELPOSFILE); printf(" \n");
newinp:
//   if( fgets(linepos,109,parampos) != NULL ) {
     while (   fgets(linepos,1000,parampos) ) {
      if(linepos[0] != '#' ) {
       for(i=0;i<100;i++) if(linepos[i] == 9) linepos[i]=32; //replace tab by space
       sscanf(linepos, "%d %d %d %d %f %f %f %f %f %f %f %f %d %d ",
        &ii, &ij, &iX, &iY, &dLaserX, &dLaserY,
        &dAxisX, &dSlopeX, &dConvX, &dAxisY, &dSlopeY, &dConvY,
        &iIsReversed, &iIsInstalled );

//     printf("%2.1d %2.1d %4.3d %4.3d %7.2lf %7.2lf %8.2lf %8.3lf %8.3lf %8.2lf %8.3lf %8.3lf %d %d \n",
//      ii, ij, iX, iY, dLaserX, dLaserY,
//      dAxisX, dSlopeX, dConvX, dAxisY, dSlopeY, dConvY,
//      iIsReversed, iIsInstalled );


//     if(panel[ii+8][ij+8].pan_inst>0)
       if(iIsInstalled != 0 ) {

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
         panel[ii+8][ij+8].cali_rev   =0    ;
         if(iIsReversed != 0 ) {
//        printf("IsReversed is set for %d, %d \7\7\7\7\n",ii,ij);
          panel[ii+8][ij+8].cali_slp[0]=atan(1./dSlopeX);     //HA 21.112011
          panel[ii+8][ij+8].cali_slp[1]=atan(1./dSlopeY);     //
//?????   panel[ii+8][ij+8].cali_spp[0]=-dConvX;              //
//?????   panel[ii+8][ij+8].cali_spp[1]=-dConvY;              //HA 21.112011
          panel[ii+8][ij+8].cali_spp[0]= dConvX;              //AB 21.12.2012
          panel[ii+8][ij+8].cali_spp[1]= dConvY;              //AB 21.12.2012
          panel[ii+8][ij+8].cali_rev   =1    ;
         }

       }
      }    //not #
//      if((ii != 8) | (ij != 8) ) goto newinp;
     }     //line not NULL
     printf("closing \n");
     fclose(parampos);
    }      //file was open

//.........................................................................

  return 0;
}





int LUT_read(AMCpanel p[17][17], int nnew)
{
// read LUT; interpolate between not measured zd
// filenames:  AMC_Z<zd>_A<az>.lut1    ("AMC_Z%+3d_A%+2d.lut1")

   int i,iz,ia,j,jz,izmin,izmax,lut_zen[200],nmbr,ifst;
   int k,kz,lz,kkk,xza,xzb,yza,yzb,px,py,motA,motB,jj;
   float dla, dlb;
   char line[1000], directory[500], lstr[300], file[160];

   FILE* param;
   DIR *dp;
   struct dirent *ep;

// preset all to 0
   for (i=0; i<17; i++)
      for (j=0; j<17; j++) {
         p[i][j].corr[0] = p[i][j].corr[1] = 0;
         for (iz=0; iz<200; iz++)
            p[i][j].lut[0][iz][0] = p[i][j].lut[0][iz][1] = 0;
      }

   nmbr = 0;
   for (i=0; i<200; i++) lut_zen[i]=0;

// read directory:
   dp = opendir (".");

   while ((ep = readdir (dp))) {

      jj=sscanf(ep->d_name,"AMC_Z%d_A%d.lut1" ,&iz,&ia);

      if (jj==2 && abs(iz)<100 && strlen(ep->d_name)==19 &&
           ep->d_name[17]=='t' ) {         //found a file

//       read the file and store in the lut structure
//       format:  panel_address (i j) motA motB

         jz=iz+100;
         param = fopen(ep->d_name,"r");
         if(param == NULL) {
            sprintf(lstr,"can't open %s",ep->d_name);
            put_logfile(LOG_ERR, 0, ep->d_name);
            return(-1);
         }
         put_logfile(LOG_DB1, 0, ep->d_name);
         nmbr++;
         lut_zen[iz+100]++;
         ifst=0;
         while ( fgets(line,1000,param) ) {
            if (ifst++ == 0)  put_logfile(LOG_INF, 0, line);
            if (line[0] != '#') {
               sscanf(line, "%d %d %d %d", &px,&py,&motA,&motB);
               i = px + 8;
               j = py + 8;
               p[i][j].lut[0][jz][0] = motA;
               p[i][j].lut[0][jz][1] = motB;
            }
         }
         fclose(param);
   }  }

   (void) closedir (dp);

// interpolate the values
   for (iz=0;   iz<200 && lut_zen[iz]==0; iz++);
   izmin = iz;

   for (iz=199; iz>0   && lut_zen[iz]==0; iz--);
   izmax = iz;

   sprintf(lstr,"interpolate LUT: min, max, %d %d",izmin-100,izmax-100);
   put_logfile(LOG_DB9, 0, lstr);
   for (jz=0; jz<izmin; jz++) {   //expand fixed to lowest zd
      lut_zen[jz]=-1;
      for ( i=0; i<17; i++ )
         for ( j=0; j<17; j++) {
            p[i][j].lut[0][jz][0] = p[i][j].lut[0][izmin][0];
            p[i][j].lut[0][jz][1] = p[i][j].lut[0][izmin][1];
   }     }

   for (jz=199; jz>izmax; jz--) { //expand fixed to highest zd
      lut_zen[jz]=-1;
      for ( i=0; i<17; i++ )
         for ( j=0; j<17; j++) {
            p[i][j].lut[0][jz][0] = p[i][j].lut[0][izmax][0];
            p[i][j].lut[0][jz][1] = p[i][j].lut[0][izmax][1];
   }     }

//linear interpolation
   kz=izmin;
   while (kz<izmax) {
      for (lz=kz+1; lut_zen[lz]==0 && lz<=izmax; lz++);
//    printf("interpolate from %d to %d; %d %d\n", kz, lz, lut_zen[lz], lut_zen[lz]==0 && lz<=izmax );
      kkk = (lz - kz );
      if (kkk > 1)
         for ( i=0; i<17; i++ )
            for ( j=0; j<17; j++) {
               xza = p[i][j].lut[0][kz][0];
               yza = p[i][j].lut[0][lz][0];
               xzb = p[i][j].lut[0][kz][1];
               yzb = p[i][j].lut[0][lz][1];
               dla = (float) (yza - xza)/kkk;
               dlb = (float) (yzb - xzb)/kkk;

               for (k=1; k<kkk; k++ ) {
                  p[i][j].lut[0][k+kz][0] = xza+ k*dla +0.5;
                  p[i][j].lut[0][k+kz][1] = xzb+ k*dlb +0.5;

            }  }

      for (k=kz+1; k<lz; k++) lut_zen[k]=-2;
      kz=lz;
   }

   if (nnew <=0 )               //there are no NEW files to test ==> for safety use LUT
      for ( i=0; i<17; i++)
          for ( j=0; j<17; j++)
              for (k=0; k<200; k++) {
                  p[i][j].new[0][k][0] =  p[i][j].lut[0][k][0];
                  p[i][j].new[0][k][1] =  p[i][j].lut[0][k][1];
              }

   sprintf(lstr,"read %d lut-files",nmbr);
   put_logfile(LOG_OK_, 0, lstr);
   return(nmbr);
}

//--------------------------------------------------------------------

int NEW_read(AMCpanel p[17][17])
{
// read NEW; interpolate between not measured zd
// filenames:  AMC_Z<zd>_A<az>.new1   ("AMC_Z%+3d_A%+2d.lut")

   int i,iz,ia,j,jz,izmin,izmax,lut_zen[200],nmbr,ifst;
   int k,kz,lz,kkk,xza,xzb,yza,yzb,px,py,motA,motB,jj;
   float dla, dlb;
   char line[1000], directory[500], lstr[300];

   FILE* param;
   DIR *dp;
   struct dirent *ep;

// preset all to 0
   for (i=0; i<17; i++)
      for (j=0; j<17; j++) {
         p[i][j].corr[0] = p[i][j].corr[1] = 0;
         for (iz=0; iz<200; iz++)
            p[i][j].new[0][iz][0] = p[i][j].new[0][iz][1] = 0;
      }

   nmbr = 0;
   for (i=0; i<200; i++) lut_zen[i]=0;

// read directory:
   dp = opendir (".");

   while ((ep = readdir (dp))) {

      jj=sscanf(ep->d_name,"AMC_Z%d_A%d.new1" ,&iz,&ia);

      if (jj==2 && abs(iz)<100 && strlen(ep->d_name)==19 &&
         ep->d_name[17]=='w' ) {         //found a file

//       read the file and store in the lut structure
//       format:  panel_address (i j) motA motB

         jz=iz+100;
         param = fopen(ep->d_name,"r");
         if(param == NULL) {
            sprintf(lstr,"can't open %s",ep->d_name);
            put_logfile(LOG_ERR, 0, ep->d_name);
            return(-1);
         }
         put_logfile(LOG_DB1, 0, ep->d_name);
         nmbr++;
         lut_zen[iz+100]++;
         ifst=0;
         while ( fgets(line,1000,param) ) {
            if (ifst++ == 0 )  put_logfile(LOG_INF, 0, line);
            if (line[0] != '#') {
               sscanf(line, "%d %d %d %d", &px,&py,&motA,&motB);
               i = px + 8;
               j = py + 8;
               p[i][j].new[0][jz][0] = motA;
               p[i][j].new[0][jz][1] = motB;
            }
         }
         fclose(param);
   }  }

   (void) closedir (dp);

// interpolate the values
   for (iz=0;   iz<200 && lut_zen[iz]==0; iz++);
   izmin = iz;

   for (iz=199; iz>0   && lut_zen[iz]==0; iz--);
   izmax = iz;

   sprintf(lstr,"interpolate NEW: min, max, %d %d",izmin-100,izmax-100);
   put_logfile(LOG_DB9, 0, lstr);
   for (jz=0; jz<izmin; jz++) {   //expand fixed to lowest zd
      lut_zen[jz]=-1;
      for (i=0; i<17; i++)
         for (j=0; j<17; j++) {
            p[i][j].new[0][jz][0] = p[i][j].new[0][izmin][0];
            p[i][j].new[0][jz][1] = p[i][j].new[0][izmin][1];
   }     }

   for (jz=199; jz>izmax; jz--) { //expand fixed to highest zd
      lut_zen[jz]=-1;
      for (i=0; i<17; i++)
         for (j=0; j<17; j++) {
            p[i][j].new[0][jz][0] = p[i][j].new[0][izmax][0];
            p[i][j].new[0][jz][1] = p[i][j].new[0][izmax][1];
   }     }

//linear interpolation
   kz=izmin;
   while (kz<izmax) {
      for (lz=kz+1; lut_zen[lz]==0 && lz<=izmax; lz++);
//    printf("interpolate from %d to %d; %d %d\n", kz, lz, lut_zen[lz], lut_zen[lz]==0 && lz<=izmax );
      kkk = (lz - kz );
      if (kkk > 1)
         for ( i=0; i<17; i++ )
            for ( j=0; j<17; j++) {
               xza = p[i][j].new[0][kz][0];
               yza = p[i][j].new[0][lz][0];
               xzb = p[i][j].new[0][kz][1];
               yzb = p[i][j].new[0][lz][1];
               dla = (float) (yza - xza)/kkk;
               dlb = (float) (yzb - xzb)/kkk;

               for (k=1; k<kkk; k++ ) {
                  p[i][j].new[0][k+kz][0] = xza+ k*dla +0.5;
                  p[i][j].new[0][k+kz][1] = xzb+ k*dlb +0.5;

            }  }

      for (k=kz+1; k<lz; k++) lut_zen[k]=-2;
      kz=lz;
   }

   sprintf(lstr,"read %d new-files",nmbr);
   put_logfile(LOG_OK_, 0, lstr);
   return(nmbr);
}


//--------------------------------------------------------------------

int  IF_read(AMCpanel panel[17][17], int fileID)
{
  FILE* param;
  char line[1000], lstr[160] ;
  int  i,j,n, ii, jj ;
  float xx, yy, zz ;

  //set all values to zero; overwrite those panels mentioned in file fileID
  for (i=0; i<17; i++)
     for (j=0; j<17; j++)
        panel[i][j].loc_x = panel[i][j].loc_y = panel[i][j].loc_z = 0. ;   

  sprintf(lstr,"IFpanel_%05d.txt",fileID) ;

  param = fopen(lstr,"r");
  if(param == NULL) { 
      printf("can't open %s \n",lstr);
      return -1 ;
  } 
  printf("did open %s \n",lstr);
  while (   fgets(line,1000,param) ) {
     if (line[0] != '#') {
        n=sscanf(line, "%d %d %f %f %f", &ii, &jj, &xx, &yy, &zz);
        if (n>2 && ii>-9 && ii<9 && jj>-9 && jj<9) {
           ii = ii+8 ;
           jj = jj+8 ;
           if (n>2) panel[ii][jj].loc_x = xx ;
           if (n>3) panel[ii][jj].loc_y = yy ;
           if (n>4) panel[ii][jj].loc_z = zz ;
        }
     }
  }

  fclose(param) ;

  return fileID;
}

