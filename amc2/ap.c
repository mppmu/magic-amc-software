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
  int i,j,k ;
     //default  p: 0=off, 1=on, -1=always on

  i = 0 ;
  p[i].deflt=1; strncpy( p[i].str," 1 ",6); p[i].chan= 0; i++; // chain 0       0
  p[i].deflt=1; strncpy( p[i].str," 2 ",6); p[i].chan= 2; i++; // chain 1       1
  p[i].deflt=1; strncpy( p[i].str," 3 ",6); p[i].chan= 4; i++; // chain 2       2
  p[i].deflt=1; strncpy( p[i].str," 4 ",6); p[i].chan= 6; i++; // chain 3       3
  p[i].deflt=1; strncpy( p[i].str," 5 ",6); p[i].chan= 8; i++; // chain 4       4
  p[i].deflt=1; strncpy( p[i].str," 6 ",6); p[i].chan=10; i++; // chain 5       5
  p[i].deflt=1; strncpy( p[i].str," 7 ",6); p[i].chan=12; i++; // chain 6       6
  p[i].deflt=0; strncpy( p[i].str," SBIG ",6); p[i].chan=14; i++; // SBIG          7
  p[i].deflt=1; strncpy( p[i].str," 1b",6); p[i].chan= 1; i++; // chain 8       8
  p[i].deflt=1; strncpy( p[i].str," 2b",6); p[i].chan= 3; i++; // chain 9       9
  p[i].deflt=1; strncpy( p[i].str," 3b",6); p[i].chan= 5; i++; // chain 10     10
  p[i].deflt=1; strncpy( p[i].str," 4b",6); p[i].chan= 7; i++; // chain 11     11
  p[i].deflt=1; strncpy( p[i].str," 5b",6); p[i].chan= 9; i++; // chain 12     12
  p[i].deflt=1; strncpy( p[i].str," 6b",6); p[i].chan=11; i++; // chain 13     13
  p[i].deflt=1; strncpy( p[i].str," 7b",6); p[i].chan=13; i++; // chain 14     14
  p[i].deflt=0; strncpy( p[i].str," 15",6); p[i].chan=15; i++; // free         15

  p[i].deflt=0; strncpy( p[i].str,"Disto", 6); p[i].chan=22; i++; // DistoGuide   16
  p[i].deflt=0; strncpy( p[i].str,"  24  ",6); p[i].chan=23; i++; // free         17
  p[i].deflt=0; strncpy( p[i].str,"  25  ",6); p[i].chan=24; i++; // free         18
  p[i].deflt=0; strncpy( p[i].str,"  26  ",6); p[i].chan=25; i++; // free         19
  p[i].deflt=0; strncpy( p[i].str,"  27  ",6); p[i].chan=26; i++; // free         20
  p[i].deflt=0; strncpy( p[i].str,"  28  ",6); p[i].chan=27; i++; // free         21
  p[i].deflt=0; strncpy( p[i].str,"  29  ",6); p[i].chan=28; i++; // free         22
  p[i].deflt=0; strncpy( p[i].str,"  30  ",6); p[i].chan=29; i++; // free         23
  p[i].deflt=0; strncpy( p[i].str,"  31  ",6); p[i].chan=30; i++; // free         24
  p[i].deflt=0; strncpy( p[i].str,"  32  ",6); p[i].chan=31; i++; // free         25

  p[i].deflt=-1;strncpy( p[i].str," USCM ",6); p[i].chan=16; i++; // internal     26
  p[i].deflt=-1;strncpy( p[i].str,"485-A", 6); p[i].chan=20; i++; // internal     27
  p[i].deflt=-1;strncpy( p[i].str,"485-B", 6); p[i].chan=21; i++; // internal     28
  p[i].deflt=-1;strncpy( p[i].str,"Ether", 6); p[i].chan=19; i++; // internal     29
  p[i].deflt=-1;strncpy( p[i].str," PC  ", 6); p[i].chan=17; i++; // internal     30
  p[i].deflt=0 ;strncpy( p[i].str,"empty", 6); p[i].chan=18; i++; // internal     31

  for (i=0; i<32; i++) p[i].request = p[i].nominal = abs( p[i].deflt ) ;
  for (i=0; i<32; i++) p[i].actual  = p[i].errtim  = 0 ;


}


int  AMC_read(AMCpanel panel[17][17])
{
  int i,j,k,nchain,ip,ip2,ib,nb,nc,ic,i0,i1,i2,i3,j0,j1,j2,j3,nx ;
  int px,py,po[3],ap[3],ps[3],ad[3],is,nmbr ;
  FILE* param;
  char line[1000], linepos[1000];

  int ii, ij, iX, iY, iIsReversed, iIsInstalled;
  float  dLaser[2], dAxis[2], dSlope[2], dConv[2];

//initialize to 'undef'
  nmbr = 0 ;
  for (j=0; j<17; j++)
    for (i=0; i<17; i++) {
       panel[i][j].ix = panel[i][j].iy = panel[i][j].pnmbr = -9999 ;
       panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_DIS ;
       panel[i][j].pan_inst =-1 ;
       panel[i][j].pan_material =-1 ;
       panel[i][j].act_stat = panel[i][j].box_stat = 0 ;
       if( abs(i-8)+abs(j-8) > 12 || (i==8 && j==8) || (i==7 && j==8) ) {
          panel[i][j].pan_inst = 0 ;
          panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_NOT ;
       }
//     panel[i][j].laser    = LAS_OFF  ;
       panel[i][j].laser    = LAS_UDF  ;
       panel[i][j].pan_grp  = (j*3 +i)%8+1;
       for (k=0; k<3; k++) {
          panel[i][j].port[0][k]=-1 ;
          panel[i][j].port[1][k]=-1 ;
          panel[i][j].portflg[k]=0 ;
          panel[i][j].addr[k]=0 ;
          panel[i][j].cpos[k]=-1 ;
          panel[i][j].bpos[k]= 9 ;
          panel[i][j].temp[k]=-1 ;
          panel[i][j].humi[k]=-1 ;
          panel[i][j].status[k][0]= 0 ;
          panel[i][j].status[k][1]=0 ;
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
          panel[i][j].hal_pos[k]=-8888;
          panel[i][j].hal_val[k][0]=-8888;
          panel[i][j].hal_val[k][1]=-8888;
          panel[i][j].hal_val[k][2]=-8888;
          panel[i][j].hal_val[k][3]=-8888;
          panel[i][j].move_by[k] = 0;
          panel[i][j].actstat[k] = 0;
          panel[i][j].esw_centr[k] = 0;
     }
  }

//read file "test_Panels_m2_general.txt"
//param = fopen("test_Panels_m2_general.txt","r");
  param = fopen("test_Panels_m2_general2.txt","r");
  if(param == NULL) { printf("can't open  test_Panels_m2_general2.txt \n");
      return -1 ;
  } 
  printf("did open  test_Panels_m2_general2.txt \n");
//  printf("did open  test_Panels_m2_general.txt \n");
  while (   fgets(line,1000,param) ) {
     if (line[0] != '#') {
        sscanf(line, "%d  %d  %d     %d     %d     %x     %d     %d     %d     %x     %d     %d     %d     %x     %d",
                     &px,&py,&po[0],&ap[0],&ps[0],&ad[0],&po[1],&ap[1],&ps[1],&ad[1],&po[2],&ap[2],&ps[2],&ad[2],&is);
        if (is !=0) {
           i0 = px +8 ;
           j0 = py +8 ;
           panel[i0][j0].ix = px ;
           panel[i0][j0].iy = py ;
           panel[i0][j0].pnmbr = nmbr++ ;
           for (k=0; k<3; k++) {
              panel[i0][j0].port[0][k]=abs(po[k])-1 ;
//              panel[i0][j0].port[1][k]=abs(ap[k])-1 ;
              panel[i0][j0].port[1][k]=abs(po[k])-1 ;     // set original port also into port[1]
              panel[i0][j0].portflg[k]=0 ;
              if (po[k] <0 ) {                   //std. port disabled
                 panel[i0][j0].portflg[k]=1 ;    //try alternate port
                 if (ap[k]<0 )
                    if ( k==2 ) panel[i][j].laser = LAS_NOT ; //laser does not exist; panel ok
                    else is=-999 ;                            //disable panel
              }
              panel[i0][j0].addr[k]=ad[k] ;
//      printf("%x\n",ad[k]) ;
              panel[i0][j0].cpos[k]=ps[k] ;
              panel[i0][j0].bpos[k]=ad[2]%4 ;
           }
           if ( is>0 )          panel[i0][j0].pan_stat = STAT_NIN  ; // not initialized  [blue]
           else if ( is > -99)  panel[i0][j0].pan_stat = STAT_DIS  ; // not installed    [gray]
           else                 panel[i0][j0].pan_stat = STAT_COM  ; // no communication [brown]
           panel[i0][j0].pan_inst = is ;

           if (px*px + py*py >=49 ) panel[i0][j0].pan_material= 0 ;
                             else   panel[i0][j0].pan_material= 1 ;


           panel[i0][j0].pan_inst = is ;
  }  }  }
  fclose(param);

  printf(" nmbr %d\n", nmbr);


  nmbr=0;
//read file "Mag2PanelPos6.txt"
  param = fopen("Mag2PanelPos6.txt","r");
  if(param == NULL) { printf("can't open  Mag2PanelPos6.txt \n");
      return -1 ;
  } 
  while (fgets(linepos,1000,param) ) 
    if(linepos[0] != '#' ) {
       for(i=0;i<100;i++) if(linepos[i] == 9) linepos[i]=32;
       i=sscanf(linepos, "%d %d %d %d %f %f %f %f %f %f %f %f %d %d",
           &ii, &ij, &iX, &iY, &dLaser[0], &dLaser[1],
           &dAxis[0], &dSlope[0], &dConv[0], &dAxis[1], &dSlope[1], &dConv[1],
           &iIsReversed, &iIsInstalled );
       ii=ii+8;
       ij=ij+8;
       if (panel[ii][ij].pan_inst>0) {
          nmbr++ ;
          panel[ii][ij].ladj_xlas=dLaser[0];
          panel[ii][ij].ladj_ylas=dLaser[1];
          panel[ii][ij].cali_slp[0]=PI * dSlope[0]/180.;
          panel[ii][ij].cali_slp[1]=PI * dSlope[1]/180.;
          panel[ii][ij].cali_dca[0]=dAxis[0];
          panel[ii][ij].cali_dca[1]=dAxis[1];
          panel[ii][ij].cali_spp[0]=dConv[0];
          panel[ii][ij].cali_spp[1]=dConv[1];

       }
    }
  fclose(param);
  printf(" nmbr %d\n", nmbr);

}





int  LUT_read(AMCpanel p[17][17],int nnew)
{
// read LUT; interpolate between not measured zd
// filenames:  AMC_Z<zd>_A<az>.lut    ("AMC_Z%+3d_A%+2d.lut")

   int i,iz,ia,j,jz,izmin,izmax,lut_zen[200],nmbr,ifst ;
   int k,kz,lz,kkk,xza,xzb,yza,yzb,px,py,motA,motB,jj ;
   float dla, dlb ;
   char line[1000], directory[500], lstr[160] ;

   FILE* param;
   DIR *dp;
   struct dirent *ep;
   

// preset all to 0
   for (i=0; i<17; i++)
      for (j=0; j<17; j++) {
         p[i][j].corr[0] = p[i][j].corr[1] = 0 ;
         for (iz=0; iz<200 ; iz++) ;
            p[i][j].lut[0][iz][0] = p[i][j].lut[0][iz][1] = 0 ;
      }


   nmbr = 0;
   for (i=0; i<200; i++) lut_zen[i]=0 ;

// read directory:
     
   dp = opendir (".");
/* dp = opendir ("/home/biland/AMC2/");
   if (dp == NULL)  return -1 ;

        getcwd(directory,500);
        chdir("/home/biland/AMC2/");
 */


   while (ep = readdir (dp)) {

      jj=sscanf(ep->d_name,"AMC_Z%d_A%d.lut" ,&iz,&ia) ;

      if (jj==2 && abs(iz)<100 && strlen(ep->d_name)==18 &&
           ep->d_name[17]=='t' ) {         //found a file
         
//       read the file and store in the lut structure
//       format:  panel_address (i j) motA motB

         jz=iz+100 ;
         param = fopen(ep->d_name,"r");
         if(param == NULL) { 
            sprintf(lstr,"can't open %s",ep->d_name); 
            put_logfile(LOG_ERR, 0, ep->d_name) ;
            return(-1);
         }
         put_logfile(LOG_DB1, 0, ep->d_name) ;
         nmbr++ ;
         lut_zen[iz+100]++ ;
         ifst=0;
         while ( fgets(line,1000,param) ) {
            if (ifst++ == 0 )  put_logfile(LOG_INF, 0, line) ;
            if (line[0] != '#') {
               sscanf(line, "%d %d %d %d", &px,&py,&motA,&motB);
               i = px + 8;
               j = py + 8;
               p[i][j].lut[0][jz][0] = motA ;
               p[i][j].lut[0][jz][1] = motB ;
            }
   }  }  }
          
   (void) closedir (dp);


// interpolate the values
   for (iz=0;   iz<200 && lut_zen[iz]==0; iz++) ;
   izmin = iz ;

   for (iz=199; iz>0 && lut_zen[iz]==0; iz--) ;
   izmax = iz ;

   sprintf(lstr,"interpolate LUT: min, max, %d %d",izmin-100,izmax-100);
   put_logfile(LOG_DB9, 0, lstr);
   for (jz=0; jz<izmin; jz++) {   //expand fixed to lowest zd
      lut_zen[jz]=-1;
      for ( i=0; i<17; i++ )
         for ( j=0; j<17; j++) {
            p[i][j].lut[0][jz][0] = p[i][j].lut[0][izmin][0] ;
            p[i][j].lut[0][jz][1] = p[i][j].lut[0][izmin][1] ;
   }     }

   for (jz=199; jz>izmax; jz--) { //expand fixed to highest zd
      lut_zen[jz]=-1;
      for ( i=0; i<17; i++ ) 
         for ( j=0; j<17; j++) {
            p[i][j].lut[0][jz][0] = p[i][j].lut[0][izmax][0] ;
            p[i][j].lut[0][jz][1] = p[i][j].lut[0][izmax][1] ;
   }     }


   //linear interpolation

   kz=izmin;
   while (kz<izmax) {
      for (lz=kz+1; lut_zen[lz]==0 && lz<=izmax; lz++) ;
//    printf("interpolate from %d to %d; %d %d\n", kz, lz, lut_zen[lz], lut_zen[lz]==0 && lz<=izmax ) ;
      kkk = (lz - kz ) ;
      if (kkk > 1)
         for ( i=0; i<17; i++ )
            for ( j=0; j<17; j++) {
               xza = p[i][j].lut[0][kz][0] ;
               yza = p[i][j].lut[0][lz][0] ;
               xzb = p[i][j].lut[0][kz][1] ;
               yzb = p[i][j].lut[0][lz][1] ;
               dla = (float) (yza - xza)/kkk ;
               dlb = (float) (yzb - xzb)/kkk ;

               for (k=1; k<kkk; k++ ) {
                  p[i][j].lut[0][k+kz][0] = xza+ k*dla +0.5;
                  p[i][j].lut[0][k+kz][1] = xzb+ k*dlb +0.5;

            }  }
      
      for (k=kz+1; k<lz; k++) lut_zen[k]=-2;
      kz=lz ;
   }


   if (nnew <=0 )               //there are no NEW files to test ==> for safety use LUT
      for ( i=0; i<17; i++)
          for ( j=0; j<17; j++)
              for (k=0; k<200; k++) {
                  p[i][j].new[0][k][0] =  p[i][j].lut[0][k][0] ;
                  p[i][j].new[0][k][1] =  p[i][j].lut[0][k][1] ;
              }




   sprintf(lstr,"read %d lut-files",nmbr);
   put_logfile(LOG_OK_, 0, lstr);
   return(nmbr);
}

//--------------------------------------------------------------------

int  NEW_read(AMCpanel p[17][17])
{
// read NEW; interpolate between not measured zd
// filenames:  AMC_Z<zd>_A<az>.lut    ("AMC_Z%+3d_A%+2d.lut")

   int i,iz,ia,j,jz,izmin,izmax,lut_zen[200],nmbr,ifst ;
   int k,kz,lz,kkk,xza,xzb,yza,yzb,px,py,motA,motB,jj ;
   float dla, dlb ;
   char line[1000], directory[500], lstr[160] ;

   FILE* param;
   DIR *dp;
   struct dirent *ep;
   

// preset all to 0
   for (i=0; i<17; i++)
      for (j=0; j<17; j++) {
         p[i][j].corr[0] = p[i][j].corr[1] = 0 ;
         for (iz=0; iz<200 ; iz++) ;
            p[i][j].new[0][iz][0] = p[i][j].new[0][iz][1] = 0 ;
      }


   nmbr = 0;
   for (i=0; i<200; i++) lut_zen[i]=0 ;

// read directory:
     
   dp = opendir (".");


   while (ep = readdir (dp)) {

      jj=sscanf(ep->d_name,"AMC_Z%d_A%d.new" ,&iz,&ia) ;

      if (jj==2 && abs(iz)<100 && strlen(ep->d_name)==18 &&
         ep->d_name[17]=='w' ) {         //found a file

         jz=iz+100 ;
         param = fopen(ep->d_name,"r");
         if(param == NULL) { 
            sprintf(lstr,"can't open %s",ep->d_name); 
            put_logfile(LOG_ERR, 0, ep->d_name) ;
            return(-1);
         }
         put_logfile(LOG_DB1, 0, ep->d_name) ;
         nmbr++ ;
         lut_zen[iz+100]++ ;
         ifst=0;
         while ( fgets(line,1000,param) ) {
            if (ifst++ == 0 )  put_logfile(LOG_INF, 0, line) ;
            if (line[0] != '#') {
               sscanf(line, "%d %d %d %d", &px,&py,&motA,&motB);
               i = px + 8;
               j = py + 8;
               p[i][j].new[0][jz][0] = motA ;
               p[i][j].new[0][jz][1] = motB ;
            }
   }  }  }
          
   (void) closedir (dp);


// interpolate the values
   for (iz=0;   iz<200 && lut_zen[iz]==0; iz++) ;
   izmin = iz ;

   for (iz=199; iz>0 && lut_zen[iz]==0; iz--) ;
   izmax = iz ;

   sprintf(lstr,"interpolate NEW: min, max, %d %d",izmin,izmax);
   put_logfile(LOG_DB9, 0, lstr);
   for (jz=0; jz<izmin; jz++) {   //expand fixed to lowest zd
      lut_zen[jz]=-1;
      for ( i=0; i<17; i++ )
         for ( j=0; j<17; j++) {
            p[i][j].new[0][jz][0] = p[i][j].new[0][izmin][0] ;
            p[i][j].new[0][jz][1] = p[i][j].new[0][izmin][1] ;
   }     }

   for (jz=199; jz>izmax; jz--) { //expand fixed to highest zd
      lut_zen[jz]=-1;
      for ( i=0; i<17; i++ ) 
         for ( j=0; j<17; j++) {
            p[i][j].new[0][jz][0] = p[i][j].new[0][izmax][0] ;
            p[i][j].new[0][jz][1] = p[i][j].new[0][izmax][1] ;
   }     }


   //linear interpolation

   kz=izmin;
   while (kz<izmax) {
      for (lz=kz+1; lut_zen[lz]==0 && lz<=izmax; lz++) ;
//    printf("interpolate from %d to %d; %d %d\n", kz, lz, lut_zen[lz], lut_zen[lz]==0 && lz<=izmax ) ;
      kkk = (lz - kz ) ;
      if (kkk > 1)
         for ( i=0; i<17; i++ )
            for ( j=0; j<17; j++) {
               xza = p[i][j].new[0][kz][0] ;
               yza = p[i][j].new[0][lz][0] ;
               xzb = p[i][j].new[0][kz][1] ;
               yzb = p[i][j].new[0][lz][1] ;
               dla = (float) (yza - xza)/kkk ;
               dlb = (float) (yzb - xzb)/kkk ;

               for (k=1; k<kkk; k++ ) {
                  p[i][j].new[0][k+kz][0] = xza+ k*dla +0.5;
                  p[i][j].new[0][k+kz][1] = xzb+ k*dlb +0.5;

            }  }
      
      for (k=kz+1; k<lz; k++) lut_zen[k]=-2;
      kz=lz ;
   }

   sprintf(lstr,"read %d new-files",nmbr);
   put_logfile(LOG_OK_, 0, lstr);
   return(nmbr);
}

//--------------------------------------------------------------------

