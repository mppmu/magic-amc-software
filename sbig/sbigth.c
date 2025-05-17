/*
  xforms coord.system:                img_buffer[y][x]

       +-------------> x
       |
       |
       |
       |
    y  V

*/


#include <termios.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <pthread.h>

#include "st7.h"
#include "st7temp.c"
#include "/usr/local/include/fitsio.h"

#include "forms.h"
#include "flimage.h"

#define V_INV  -1
#define V_LIN   0
#define V_LOG   1
#define V_SQRT  2
#define V_HIST  3
long    g_viewmode = V_LOG ;
long    g_viewold  = V_LOG ;
long    g_viewinv  = 1     ;

#define C_WID 1024
#define C_HIG 1024

#define S_BREAK    -3
#define S_ERROR    -2
#define S_NOTREADY -1
#define S_READY     0
#define S_WAIT      1
#define S_READOUT   2
#define S_EXPOSING  3
#define S_FILTER    4
#define S_WRITING   5
#define S_PLOTTING  6
long    g_status = S_NOTREADY ;
long    g_stat0  = S_NOTREADY ;
long    g_statpc =    0 ;
long    g_statpc0=    0 ;


#define A_DARK  -1
#define A_PICT   0
#define A_LOOP   1
#define A_AUTO   2
long    g_pictloop = 0 ;


#define ACTboxcol FL_YELLOW,FL_YELLOW
#define STDV_COL   FL_COL1,FL_YELLOW
FLIMAGE_SETUP mysetup ;
  
FL_FORM *form;
FL_OBJECT *FLtemp, *FLexpos, *FLexit,  *FL_Setting_grp, *FL_Action_grp ;
FL_OBJECT *FLtemp0,*FLexpos0,*FLexit0, *FL_Active, *FLstatus, *FLauto ;
FL_OBJECT *FLval0 ;
FL_OBJECT *canvas, *cnv1, *cnv2, *FLlab[6] ;
FL_IMAGE  *image,  *img1, *img2 ;
FL_Coord  xpos, ypos ;
unsigned  keymask ;

Window     WINccd;
int  TMexp=0, TMtemp=0, TMpict ;

long g_xmin =    0 ;
long g_xmax = 1024 ;
long g_ymin =   60 ;
long g_ymax =  960 ;


double g_expos =  2.0 ;
double g_exptot=    0 ;
long   g_expmsc=    0 ;
long   g_exptmp=    0 ;
double g_temp  =  0.0 ;
double g_temp0 =  0.0 ;
double g_azim  = -999 ;   // azi
double g_zenit = -999 ;   // zen  received from AMC or CC
long   g_newpic=    0 ;
long   g_active=    0 ;
long   g_break =    0 ;
long   g_busy  =    0 ;
long   g_auto  =    0 ;
long   g_filter=    5 ;  //default filter (at start) = 5

double pct_expos ;
double pct_temp  ;
double pct_azim  ;
double pct_zenit ;
char   pct_filter[80], pct_imgtyp[80], pct_fulldat[80], pct_pangrp[80];

char   xtmp[100], filter[80], imgtyp[80], fulldat[80], pangrp[80];
char   str[1000] ;
char  fname[1000] ;
int   fdisk1 ;


long   k0=1 ;
unsigned short img_buffer[C_WID][C_HIG] ;


unsigned long idlsec0,idlsec,idlusec0,idlusec,iexpusec,itmpusec ;



short st7Error;
static unsigned short *myImage;
unsigned short *pImage;
unsigned short *thePicture(void) { return myImage; }
struct timeval  *tv, atv;
struct timezone *tz, atz;
long t1,t2,t3,s1,s2,s3;


//--------------------------------------------------------------------
int  canvas_motion()
{
   unsigned ix,iy,iv;

   fl_get_form_mouse( form,  &xpos, &ypos, &keymask);
   ix = xpos-250 ;
   iy = ypos+60  ;
   iv = img_buffer[iy][ix] ;
 //sprintf(xtmp,"X:%4d Y:%3d  V:%5d G:%3d\n",ix,iy,iv,image->gray[iy][ix]) ;
   sprintf(xtmp,"X:%4d Y:%3d  V:%5d\n",ix,iy,iv) ;
   fl_set_object_label(FLval0, xtmp ) ;

}
//--------------------------------------------------------------------
int  canvas_expose()
{
 //printf("canvas exposed....\n");
   draw_pixmap();   //expose
}
//--------------------------------------------------------------------
int  canvas_leave()
{
 //printf("canvas leave....\n");
   sprintf(xtmp,"\n") ;
   fl_set_object_label(FLval0, xtmp ) ;
}
//--------------------------------------------------------------------
int  canvas_enter()
{
 //printf("canvas entered....\n");
}
//--------------------------------------------------------------------
int  canvas_button()
{
 //printf("canvas button on ....\n");
}
//--------------------------------------------------------------------
int  canvas_buttof()
{
 //printf("canvas button off ....\n");
 /*fl_get_form_mouse( form,  &xpos, &ypos, &keymask);
   printf("button %d %d\n", xpos, ypos);

   flimage_add_marker(image,"oval",xpos-250.,ypos-0.,10.,10.,0,0,0,FL_PACK(255,0,0),0) ;
   draw_pixmap() ;
 */
}
//--------------------------------------------------------------------
void on_off(FL_OBJECT *ob, long n)
{
  if (n ==0 )
  {
     fl_deactivate_object( ob );
     fl_set_object_lcol( ob, FL_GRAY75 );
  }
  else
  {
     fl_activate_object( ob );
     fl_set_object_lcol( ob, FL_BLACK );
  }

}
//--------------------------------------------------------------------
void set_status(long n, long pc)
{
   g_status = n ;
   g_statpc = pc ;
   printf("SBig: %d %d\n",n,pc);

   if ( n == S_EXPOSING )
     { sprintf(xtmp, "SBig:  EXPOSING %3d%%",pc); g_active = 1; }
   else if ( n == S_READOUT )
     { sprintf(xtmp, "SBig:  READOUT %3d%%", pc); g_active = 1; }
   else if ( n == S_FILTER )
     { sprintf(xtmp, "SBig:  FILTER WHEEL %3d%%",pc); g_active = 1; }
   else if ( n == S_WAIT )
     { sprintf(xtmp, "SBig:  WAITING") ; g_active = 1; }
   else if ( n == S_NOTREADY )
     { sprintf(xtmp, "SBig:  NOT READY");g_active = 0; }
   else if ( n == S_WRITING )
     { sprintf(xtmp, "SBig:  WRITING") ; g_active = 0; }
   else if ( n == S_PLOTTING )
     { sprintf(xtmp, "SBig:  WRITING") ; g_active = 0; }
   else if ( n == S_READY )
     { sprintf(xtmp, "SBig:  READY") ;   g_active = 0; }
   else if ( n == S_ERROR )
     { sprintf(xtmp, "SBig:  ERROR") ;   g_active = 0; }
   else if ( n == S_BREAK )
     { sprintf(xtmp, "SBig:  BREAK") ;   g_active = 0; }

   fl_set_object_label(FLstatus, xtmp ) ;
// fl_check_forms() ;

}
//--------------------------------------------------------------------
short CloseST7(short turnItOff)
{
    if (!(st7Error=SetCoolingOff())
     && !(st7Error=SBIGUnivDrvCommand(CC_CLOSE_DEVICE, NULL, NULL))
     && !(st7Error=SBIGUnivDrvCommand(CC_CLOSE_DRIVER, NULL, NULL))
     && turnItOff)
//      if (!ResetDI16Line(di16line) || !CloseDI16())
//          return st7Error = CE_DI16_ERROR;

    if (myImage) free(myImage);

    return st7Error;
}   /*  end of CloseST7  */
//--------------------------------------------------------------------
short InitST7(short turnItOn)
{   static GetDriverInfoParams   infoRqs; /* GetInfo structs */
    static GetDriverInfoResults0 infoAns;

    static OpenDeviceParams      openPrm; /* link structs */
    static EstablishLinkParams   linkPrm;
    static EstablishLinkResults  linkAns;

    myImage = malloc(st7width*st7height*sizeof(unsigned short));
    if (!myImage) return CE_NOMEMORY;

//  if (turnItOn)
//      if (!InitDI16(di16) || !SetDI16Line(di16line))
//          return st7Error = CE_DI16_ERROR;

    /*  init structs  */
    infoRqs.request = 0;
    openPrm.deviceType = st7;
    /* xmccd doesn't init "linkPrm": let's xfingers.. */

    if (!(st7Error=SBIGUnivDrvCommand(CC_OPEN_DRIVER, NULL, NULL))
     && !(st7Error=SBIGUnivDrvCommand(CC_GET_DRIVER_INFO, &infoRqs, &infoAns))
     && !(st7Error=SBIGUnivDrvCommand(CC_OPEN_DEVICE, &openPrm, NULL))
     && !(st7Error=SBIGUnivDrvCommand(CC_ESTABLISH_LINK, &linkPrm, &linkAns)))
        st7Error=SetCoolingOn(kTempSetpoint);

    return st7Error;
}   /*  end of InitST7  */

//--------------------------------------------------------------------
int  draw_pixmap()      
{ 
  img2->modified = 1 ;
  img2->type = FL_IMAGE_GRAY;
  flimage_display(img2, FL_ObjWin(cnv2)) ;
  img1->modified = 1 ;
  img1->type = FL_IMAGE_GRAY;
  flimage_display(img1, FL_ObjWin(cnv1)) ;
  image->modified = 1 ;
  image->type = FL_IMAGE_GRAY;
  flimage_display(image, FL_ObjWin(canvas)) ;
}
//--------------------------------------------------------------------
int  upd_pixmap()      //scale picture in img_buffer, put into image->gray, display image->gray
{ 
  int i,j,k;
  int kmin = 100000 ;
  int kmax =-100000 ;
  int kskip=     33 ; //skip few very bright/dim pixels



  double label3[7]     = {7*0.} ;
  double label2[7]     = {0.,51.,102.,153.,204.,254.,66000} ;
  int    level1[256]   = {256*0} ;
  int    level4[256]   = {256*0} ;
  int    level0[66001] = {66001*0} ;          //contains translated gray
  int    levelA[66001] = {66001*0} ;          //contains #pixels per gray
  double scale, sc0, sc1 ;

   printf("upd_pixmap()\n");

   if ( g_newpic ==0 && g_viewmode == g_viewold ) return 0 ; //nothing to do
   g_viewold = g_viewmode ;


//find min and max value and density
   for (i=g_xmin; i<g_xmax; i++)
      for (j=g_ymin; j<g_ymax; j++)
         levelA[ img_buffer[j][i] ]++ ;
   level0[0]=levelA[0] ;
   for (i=1; i<66000; i++)
      level0[i]=level0[i-1]+levelA[i] ;


   kmin= 0;
   while (level0[kmin] <= kskip && kmin<66000) kmin++ ;

   kmax= 65998;
   while (level0[kmax] >= level0[65999]-kskip && kmax>kmin) kmax-- ;
   kmax++ ;

   scale = kmax-kmin ;
   printf("kmin,kmax, %d %d\n",kmin,kmax);

// scale it:
   if ( g_viewmode == V_HIST )
   {
     scale = level0[65999] / 256.;
     k  = 0 ;
     sc0= scale +0.5 ;

     for (i=kmin; i<kmax; i++)
        if (level0[i] < sc0 )            level0[i]=k ;
        else { k++; sc0=(k+1)*scale+0.5; level0[i]=k; }
     k=1 ;
     for (i=kmin; i<kmax; i++)
        if (level0[i] >= label2[k]) { label3[k] = i ;  k++ ; }
     label3[0]=kmin;
     label3[5]=kmax;
  }
  else if ( g_viewmode == V_LOG )
  {
     if ( scale < 2 ) scale =2 ;
     scale = 255./log10(scale) ;
     for (i=kmin; i<kmax; i++) level0[i] = log10(i-kmin)*scale ;
     for (k=0; k<6; k++)       label3[k] = pow(10.,label2[k]/scale)+kmin;
  }
  else if ( g_viewmode == V_SQRT )
  {
     if ( scale < 1 ) scale =1 ;
     scale = 255./sqrt(scale) ;
     for (i=kmin; i<kmax; i++) level0[i] = sqrt(i-kmin)*scale ;
     for (k=0; k<6; k++)       label3[k] = pow(label2[k]/scale,2.)+kmin;
  }
//else if ( g_viewmode == V_LIN )   //use lin whenever not known
  else
  {
     if ( scale < 1. ) scale =1. ;
     scale = 255./scale ;
     for (i=kmin; i<kmax; i++) level0[i] = (i-kmin)*scale ;
     for (k=0; k<6; k++)       label3[k] = label2[k]/scale + kmin ;
  }

  for (i=0;    i<=kmin; i++) level0[i]=  0;
  for (i=kmax; i<66000; i++) level0[i]=255;


//fill the picture accordingly
  for (i=g_xmin; i<g_xmax; i++)
     for (j=g_ymin; j<g_ymax; j++)
        image->gray[j][i] = level0[ img_buffer[j][i] ] ;

//color-histogram:  
  for (k=kmin; k<kmax; k++)
     level4[ level0[k] ] += levelA[k] ;
  i=0;    
  for (k=0; k<256; k++)
    if ( level4[k] > i) i=level4[k] ;
  scale = 12.5 / i ;
  for (i=0; i<128; i++)
  {
     k=(level4[2*i]+level4[2*i+1])*scale ;
     for (j=0; j< k; j++) img2->gray[i][j] =255 ;
     for (j=k; j<25; j++) img2->gray[i][j] =128 ;
  }


//do inversion if requested
  if ( g_viewinv == V_INV )
  {
     for (i=0; i<C_WID; i++)
        for (j=0; j<C_HIG; j++)
           image->gray[j][i] = 255 - image->gray[j][i] ;
     for (i=0; i<128; i++)
        for (j=0; j<25; j++)
           img1->gray[i][j]=255-(i*2);
  }
  else
     for (i=0; i<128; i++)
        for (j=0; j<25; j++)
           img1->gray[i][j]=i*2;



//and show the pictures
  img2->sx =   0;
  img2->sy =   0;
  img2->sw =  25;
  img2->sh = 128;
  img2->wx =   0;
  img2->wy =   0;
//  img2->modified = 1 ;
//  img2->type = FL_IMAGE_GRAY;
//  flimage_display(img2, FL_ObjWin(cnv2)) ;


  img1->sx =   0;
  img1->sy =   0;
  img1->sw =  25;
  img1->sh = 128;
  img1->wx =   0;
  img1->wy =   0;
//  img1->modified = 1 ;
//  img1->type = FL_IMAGE_GRAY;
//  flimage_display(img1, FL_ObjWin(cnv1)) ;

  for (k=0; k<6; k++)
  {
     sprintf(xtmp,"%5.0f\n",label3[k]);
     fl_set_object_label( FLlab[k], xtmp) ;
  }

  image->sx = g_xmin ;
  image->sy = g_ymin ;
  image->sw = g_xmax-g_xmin ;
  image->sh = g_ymax-g_ymin ;

  image->wx = g_xmin ;
  image->wy = g_ymin-60 ;
//  image->modified = 1 ;
//  image->type = FL_IMAGE_GRAY;
//  flimage_display(image, FL_ObjWin(canvas)) ;

  draw_pixmap();
  g_newpic = 0 ;

}                                       
//--------------------------------------------------------------------
short ShotAPicture(double exposureInSec, unsigned short *pict, int x0, int y0, int xwid, int ywid, int dark)
{   unsigned short row;
    static StartExposureParams sePrm;
    static QueryCommandStatusParams qPrm;
    static QueryCommandStatusResults qRes;
    static EndExposureParams eePrm;
    static StartReadoutParams srPrm;
    static ReadoutLineParams rPrm;
    static EndReadoutParams erPrm;
    struct timeval  *tv, atv;
    struct timezone *tz, atz;
    int i,j,k;
    struct tm  *mytime ;


    /*  Setup parms  */
    sePrm.ccd = eePrm.ccd = srPrm.ccd = rPrm.ccd = erPrm.ccd = CCD_IMAGING;
    sePrm.exposureTime = (unsigned long)(100.*exposureInSec+.5);
    sePrm.abgState = ABG_NOT_PRESENT;

    if ( dark > 0 ) { sePrm.openShutter = SC_CLOSE_SHUTTER; strncpy(pct_imgtyp,"DarkFrame", 80); }
    else            { sePrm.openShutter = SC_OPEN_SHUTTER;  strncpy(pct_imgtyp,"Picture"  , 80); }

    pct_expos = exposureInSec ;
    pct_temp  = g_temp0;
    pct_zenit = g_zenit;
    pct_azim  = g_azim ;

    strncpy(pct_filter,filter, 80) ;
    strncpy(pct_pangrp,pangrp, 80) ;

    tv=&atv ;
    tz=&atz ;


    gettimeofday( tv, tz);
    printf("--- %d\n",atv.tv_sec) ;
    t1=atv.tv_usec;
    s1=atv.tv_sec-1100100000;

    mytime = gmtime( &atv.tv_sec );
    sprintf(pct_fulldat,"%4d-%02d-%02d %02d:%02d:%02d",
       mytime->tm_year+1900, mytime->tm_mon, mytime->tm_mday,
       mytime->tm_hour,      mytime->tm_min, mytime->tm_sec );



    qPrm.command = CC_START_EXPOSURE;
    srPrm.readoutMode = rPrm.readoutMode = 0;
    srPrm.top = srPrm.left = rPrm.pixelStart = 0;
    srPrm.width = rPrm.pixelLength = st7width;
    srPrm.height = st7height;

    /*  Shoot the Picture  */

    set_status( S_EXPOSING, 0 );
        printf("start exposing: st7Error %d %f \n", st7Error,exposureInSec);
    if (!(st7Error=SBIGUnivDrvCommand(CC_START_EXPOSURE, &sePrm, NULL)))
    {   
        printf("start exposing: st7Error %d %f \n", st7Error,exposureInSec);



        j=exposureInSec *5. ;
        printf(".... %d\n",j);

        for (i=0; i<j; i++)
        {
           sprintf(xtmp,"%3.1f\n",(i*0.2) );
           fl_set_object_label( FLexpos0, xtmp) ;
           k = (i*20)/exposureInSec ;
           set_status( S_EXPOSING, k );
           if (g_pictloop < 0 )     //break
           {
              set_status( S_BREAK, 0 );
              goto shutit;
           }
           else usleep(200000) ;
        }
               printf(     "expose  : %3.1f %3.1f %d\n",(i*0.2),exposureInSec,i );


        do if ( st7Error =SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS, &qPrm, &qRes))
           {
            printf("exposing: st7Error %d qRes %x %d\n", st7Error, qRes, i++);
            goto shutit;
           }
           else
           {
            usleep(1000);
           }

        while ((qRes.status & 0x0003) != 0x0003);

        gettimeofday( tv,tz);
        t2=tv->tv_usec;
        s2=tv->tv_sec-1100100000;
        s2=s2-s1;
        t2=t2-t1;
        if(t2 < 0 ) {s2=s2-1; t2=t2+1000000;}
        printf("exposition time: %d %d \n", s2,t2);
        sprintf(xtmp,"%3.1f\n",exposureInSec  );
        fl_set_object_label( FLexpos0, xtmp) ;
        set_status( S_EXPOSING, 100 );
shutit:
        SBIGUnivDrvCommand(CC_END_EXPOSURE, &eePrm, NULL); 
    }
    if (st7Error) 
    {
       set_status( S_ERROR, 0 );
       return st7Error;
    }

    /*  Read data  */
    set_status( S_READOUT, 0 );
    if (!pict) pict=myImage;  /*  set default memory array  */

    pImage=pict;
    if (!(st7Error=SBIGUnivDrvCommand(CC_START_READOUT, &srPrm, NULL)))
    {   
        for (row=0; row<st7height; row++, pict+=st7width)
        {
            if (st7Error=SBIGUnivDrvCommand(CC_READOUT_LINE, &rPrm, pict))
            { 
                set_status( S_ERROR, 0 );
                return st7Error;             
            }
            if ( (row%100) == 0 )
            {
               set_status( S_READOUT, (row/10) );
               if (g_pictloop < 0 )     //break
               {
                  set_status( S_BREAK, 0 );
                  goto endit;
               }
            }
        }
        printf("read2 error %d all done \n",st7Error);
    }
    else printf("read3 error %d no readout\n",st7Error);

    /*  End reading data  */
    g_newpic++ ;
    set_status( S_READY, 0 );

endit:
    return st7Error=SBIGUnivDrvCommand(CC_END_READOUT, &erPrm, NULL);
}   /*  end of ShotAPicture  */
//--------------------------------------------------------------------


void *filter_th(void *ifilternum)
{
  PulseOutParams p;
  static QueryCommandStatusParams qPrm;
  static QueryCommandStatusResults qRes;
  int i,j,k,ret ;

  i = (int)ifilternum ;
  sprintf(filter,"%d\n",i);

  p.pulsePeriod = 18270;
  p.numberPulses = 60;
  p.pulseWidth = 500 + 300*(i-1);
  qPrm.command = CC_PULSE_OUT;

  ret = SBIGUnivDrvCommand(CC_PULSE_OUT, &p, NULL);
  if (ret != CE_NO_ERROR) printf("CC_PULSE_OUT %d\n", ret);

  j = (i- g_filter);
  g_filter = i ;
  if (j < 0) j+=5;

  k = 0;
  do
  {
      usleep(200000);
      k+= 95;
      ret =SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS, &qPrm, &qRes) ;
      g_stat0 = S_FILTER ;
      g_stat1 = k/(j*4) ;
  }
  while (qRes.status != 0) ;

  printf("...end of thread\n");
  g_busy  = 0 ;                     // finished, so unblock the GUI
  g_stat0 = S_READY ;
  g_stat1 = 0 ;

//pthread_exit(NULL);
}


//--------------------------------------------------------------------
void filter_cb(FL_OBJECT *ob, long i)
{
  pthread_t thread;
  int       jf,rc;

  printf(" use filter %d\n",i);
  if (i < 1 || i > 5) 
  {
    printf("Invalid CFW-8 filter value \n",i);
    return;
  }

  if (i == g_filter) return ;

  g_busy  = 1 ;
  on_off( FL_Setting_grp, 0);
  on_off( FL_Action_grp,  0);
//  on_off( FLauto,         0);
  set_status( S_FILTER, 0 );       //make sure GUI is 'blocked' in case
                                   //thread does not start immediately
                                   //thread must 'unblock' when finished!

  jf = i ;
  rc = pthread_create(&thread, NULL, filter_th, (void *)jf );
  if (rc) printf("error creating thread....%d\n",rc);
}

//--------------------------------------------------------------------
void temp_tm(int id, void *jtmp)
{
  char   xtmp[100];

  g_temp0 = GetTemperature();
 //printf(     "Temp: %3.1f\n",g_temp0);
  sprintf(xtmp,"%3.1f\n",g_temp0);
  fl_set_object_label( FLtemp0, xtmp) ;
  TMtemp=fl_add_timeout(1000, temp_tm, NULL);

}
//--------------------------------------------------------------------
void temp_cb(FL_OBJECT *ob, long n)
{
  double temp ;
  int    ret ;

  if (TMtemp !=0 ) fl_remove_timeout(TMtemp);
  if (ob != NULL )
  {
     sprintf(str,"%s\n", fl_get_input( FLtemp ));
     g_temp = atof(str) ;
  }
  else
  {
     g_temp = n ;
     g_temp = g_temp / 10. ;
  }
  ret = SetCoolingOn(g_temp) ;
  if (ret == 0) set_status(S_READY, 0);
  else          set_status(S_NOTREADY, 0);

  TMtemp=fl_add_timeout(1000, temp_tm, NULL);
}

//--------------------------------------------------------------------
void exit_cb(FL_OBJECT *ob, long n)
{
  if (g_busy != 0 )
  {                    //break an operation   ??
        g_break = -999 ;
        g_pictloop = -1;
        printf(" break \n");
  }
  else                //exit program
  {
     if (fl_show_question("Do you really want to Quit?",0)) 
     {
        CloseST7( 0);
        fl_finish();
        exit(0);
     }
  }
}
//--------------------------------------------------------------------
void expos_cb(FL_OBJECT *ob, long n)
{
  if (ob != NULL )    //called from GUI
  {
     sprintf(str,"%s\n", fl_get_input( FLexpos ));
     g_expos = atof(str) ;
  }
  else                //called from SW
  {
     g_expos = n ;
     g_expos = g_expos / 100. ;
  }

  if (g_expos < 0.1 ) g_expos=0.1 ;

  printf("Exposure %f\n",g_expos);

  if      (g_expos< 10) sprintf(xtmp,"%.2f\n",g_expos);
  else if (g_expos<100) sprintf(xtmp,"%.1f\n",g_expos);
  else                  sprintf(xtmp,"%.0f\n",g_expos);
  fl_set_object_label( FLexpos0, xtmp) ;
  fl_set_input( FLexpos ,xtmp) ;
}

//--------------------------------------------------------------------
void auto_cb(FL_OBJECT *ob, long n)
{
  int state ;

  if ( ob!= NULL) state=fl_get_button(ob) ;
  else            state=n ;

  if ( state != 0 )
  {
     g_auto = 1 ;
     printf("activate remote\n");
     on_off( FL_Setting_grp, 0);
     on_off( FL_Action_grp,  0);
  }
  else
  {
     g_auto = 0 ;
     printf("deactivate remote\n");
     on_off( FL_Setting_grp, 1);
     on_off( FL_Action_grp,  1);
  }
}
//--------------------------------------------------------------------
void pict_cb(FL_OBJECT *ob, long n)
{
  int ret, x0, y0, xwid, ywid, dark;

  g_busy  = 1 ;
  g_break = 0 ;
  FL_Active = ob ;
  on_off( FL_Setting_grp, 0);
  on_off( FL_Action_grp,  0);
//  on_off( FLauto,         0);
  fl_set_object_label( FLexit, "BREAK") ;
  fl_set_object_color( FLexit, FL_MAGENTA , FL_YELLOW);
  if      ( n == A_DARK ) {g_pictloop = 1 ;      dark = 1;}
  else if ( n == A_PICT ) {g_pictloop = 1 ;      dark =-1;}
  else if ( n == A_LOOP ) {g_pictloop = 1000000; dark =-1;}
  else if ( n == A_AUTO )  g_pictloop = -1 ;

  if (dark > 0) sprintf(imgtyp,"DarkFrame\n");
  else          sprintf(imgtyp,"Picture\n");

  while ( g_pictloop > 0 && g_break ==0)
  {
     ret=ShotAPicture(g_expos,&img_buffer, x0, y0, xwid, ywid, dark) ;
     if (ret != 0) { g_pictloop = -1; }
     else          { upd_pixmap(); g_pictloop-- ;   }
  }

  on_off( FL_Setting_grp, 1);
  on_off( FL_Action_grp,  1);
//  on_off( FLauto,         1);

  if (FL_Active != NULL ) fl_set_button( FL_Active , 0) ;
  fl_set_object_label( FLexit, "EXIT") ;
  fl_set_object_color( FLexit, FL_COL1 , FL_YELLOW);
  g_busy  = 0 ;

}




//--------------------------------------------------------------------
void view_cb(FL_OBJECT *ob, long n)
{
   g_viewold = g_viewmode ;
   if ( n > V_INV ) g_viewmode = n ;                   //lin,log,sqrt,hist
   else { g_viewold = -1 ; g_viewinv=g_viewinv*V_INV;} //invert


   if (g_active==0) upd_pixmap() ;                     //show (if not busy)
}

//--------------------------------------------------------------------
void save_cb(FL_OBJECT *ob, long n)
{
  const char *s;
  fitsfile *fptr;
  int   status=0 ;
  long  exposure,nelements,naxes[2]={C_WID,C_HIG};

  printf("save:\n");

  fname[0]='!';
  strncpy(&fname[1],(s=fl_show_input("Give a filename:",&fname[1])) ? s:"", 1000);
  if (s == NULL) printf("NoName ==> cancel\n");
  else 
  {
     printf("Start   Name: %s|||\n",fname);

     fits_create_file(&fptr, fname, &status);
     printf("2 create_file: %d\n",status);
     fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);
     printf("3 create_img %d\n",status);
     exposure=g_expos*100;
     nelements=C_WID*C_HIG; 
     printf("4 test %d \n",nelements*2);
     fits_write_img(fptr, TUSHORT, 1, nelements, img_buffer, &status);
     printf("5 write_img %d \n",status);

     fits_update_key_str(fptr, "INSTRUME", "SBig", "MAGIC-telescope", &status)  ;
     fits_update_key_dbl(fptr, "EXPTIME" , pct_expos,-5, "[s] exposure time ", &status)  ;
     fits_update_key_dbl(fptr, "TEMPERAT", pct_temp, -5, "[deg] temperature ", &status)  ;
     fits_update_key_dbl(fptr, "ZENITH"  , pct_azim, -5, "[deg] zenith from drive", &status)  ;
     fits_update_key_dbl(fptr, "AZIMUTH" , pct_zenit,-5, "[deg] azimuth from drive", &status)  ;
     fits_update_key_str(fptr, "PANGROUP", pct_pangrp,"Panel (j,i) or Group 0 ... 9", &status)  ;
     fits_update_key_str(fptr, "FILTER"  , pct_filter,"Filter used", &status)  ;
     fits_update_key_str(fptr, "IMAGETYP", pct_imgtyp,"Dark or Pict", &status)  ;
     fits_update_key_str(fptr, "DATE"    , pct_fulldat,"YYYY-MM-SS hh:mm:ss [UTC]", &status)  ;
     printf("6 update_key %d\n",status);

     fits_close_file(fptr,  &status);
     printf("7 close_file %d\n",status);
     fits_report_error(stderr, status);
     printf("8 report_error %d \n", status);
     printf("End   Name: %s|||\n",fname);
  }
  if (ob != NULL ) fl_set_button( ob, 0) ;
}

//--------------------------------------------------------------------
void mode_cb(FL_OBJECT *ob, long n)
{
   printf("Mode %d\n",n);
}

//--------------------------------------------------------------------
void init_pixmap()
{ 
  unsigned short k;
  int           i,j ;


  image = flimage_alloc();
  image->type = FL_IMAGE_GRAY;         
  image->w = C_WID ;
  image->h = C_HIG ;
  flimage_getmem(image);

  img1 = flimage_alloc();
  img1->type = FL_IMAGE_GRAY;
  img1->w = 25 ;
  img1->h = 128 ;
  flimage_getmem(img1);

  img2 = flimage_alloc();
  img2->type = FL_IMAGE_GRAY;
  img2->w = 25 ;
  img2->h = 128 ;
  flimage_getmem(img2);

  upd_pixmap();
}
//--------------------------------------------------------------------
void gen_pixmap(int ix, int iy,int dx,int dy)
{ 
  int k,i,j,jy0 ;
  unsigned char c1,c2 ;

  memset(&mysetup , 0, sizeof(mysetup) );
  mysetup.delay = 1;
  mysetup.do_not_clear = 1;
  flimage_setup(&mysetup) ;

  fl_set_border_width(-1);
  canvas   = fl_add_canvas(FL_NORMAL_CANVAS,ix,iy,dx,dy," ");
             fl_add_canvas_handler(canvas, 12, canvas_expose, 0); //expose
             fl_add_canvas_handler(canvas,  8, canvas_leave , 0); //leave
             fl_add_canvas_handler(canvas,  7, canvas_enter , 0); //enter
             fl_add_canvas_handler(canvas,  6, canvas_motion, 0); //motion
             fl_add_canvas_handler(canvas,  5, canvas_buttof, 0); //button press
             fl_add_canvas_handler(canvas,  4, canvas_button, 0); //button release
  WINccd   = FL_ObjWin( canvas) ;
  fl_set_object_boxtype(canvas,FL_FLAT_BOX);

  jy0 = 632 ;

  cnv1     = fl_add_canvas(FL_NORMAL_CANVAS,55,jy0,25,128," ");
  fl_set_object_boxtype(cnv1  ,FL_FLAT_BOX);

  cnv2     = fl_add_canvas(FL_NORMAL_CANVAS,82,jy0,25,128," ");
  fl_set_object_boxtype(cnv2  ,FL_FLAT_BOX);


  for (k=0; k<6; k++)
  {
     FLlab[k] = fl_add_box(FL_FLAT_BOX, 55, jy0+k*25-4,1,25," ");
     fl_set_object_lalign(FLlab[k] ,FL_ALIGN_LEFT);
  }

  FLval0 =
        fl_add_box(FL_FLAT_BOX,115,jy0+145,1,12," ");
        fl_set_object_lalign(FLval0 ,FL_ALIGN_LEFT);

//printf("canvas depth: %d \n",fl_get_canvas_depth( canvas   ) );
  fl_set_border_width(-3);
}                                       
//--------------------------------------------------------------------
int sbig_idle(XEvent *i, void *j)
{
  if ( g_status != g_stat0)        set_status( g_stat0; g_statpc0) ;
  else if ( g_statpc != g_statpc0) set_status( g_stat0; g_statpc0) ;

  on_off( FL_Setting_grp, 1);       //
  on_off( FL_Action_grp,  1);       //
//  on_off( FLauto,         1);     //
  set_status( S_READY, 0 );         //

  printf("end of thread...\n");
}                                       


//--------------------------------------------------------------------

void sbig_form(void)
{
  FL_OBJECT *obj , *obj1 ;
  int jx,jy,dx,dy,jx0,jy0,dx0,dy0,jy1 ;

  form = fl_bgn_form(FL_FLAT_BOX,1275, 900);
  fl_set_goodies_font(FL_NORMAL_STYLE,FL_LARGE_SIZE);

  gen_pixmap(250, 0, 1024, 900) ;     //picture area ...
  init_pixmap() ;

  jx0 = 10 ;
  jy0 = 10 ;

  jx  = jx0 ;
  jy  = jy0 ;
  dy0 = 40  ;
  dy  = dy0 ;

  jx = jx0 ;
  jy = jy0 + 20 ;
  FLstatus =
  obj = fl_add_box(FL_FLAT_BOX,jx,jy,1,25,"SBig:  NOT READY");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_lalign(obj ,FL_ALIGN_RIGHT);

  dy  = dy0 *1.5   ;
  jy  = jy +dy     ;
  FLauto =
  obj = fl_add_button(FL_PUSH_BUTTON,jx,jy,225,dy,"Remote (AMC)");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_callback(obj,auto_cb, A_AUTO);
        fl_set_object_color( obj , FL_COL1 , FL_YELLOW);
    on_off( FLauto,         0);

  jy1= jy0 + 80 +1.5*dy;
  jx = jx0 ;
  jy = jy1 ;
  obj = fl_add_box(FL_FLAT_BOX,jx,jy,1,25,"Temperat.[deg]");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_lalign(obj ,FL_ALIGN_RIGHT);
  jy  =jy+dy0 ;
  obj = fl_add_box(FL_FLAT_BOX,jx,jy,1,25,"Exposure [sec]");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_lalign(obj ,FL_ALIGN_RIGHT);
  jy  =jy+dy0 ;
  obj = fl_add_box(FL_FLAT_BOX,jx,jy,1,25,"Filter");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_lalign(obj ,FL_ALIGN_RIGHT);

  jx = jx0+140;
  jy = jy1 ;
  dx =  50;
  dy =  30;
  FL_Setting_grp=fl_bgn_group();
  FLtemp =
  obj = fl_add_input(FL_FLOAT_INPUT,jx,jy,dx,dy," ");  
        fl_set_input_maxchars(obj,7 );
        fl_set_input_return(obj, FL_RETURN_END_CHANGED );
        fl_set_object_lsize(obj ,FL_LARGE_SIZE );
        fl_set_input(obj,"0.0") ;
        g_temp =  0.0 ;
        fl_set_object_callback(obj,temp_cb, 0);

  jy = jy+dy0;
  FLexpos =
  obj = fl_add_input(FL_FLOAT_INPUT,jx,jy,dx,dy," ");  
        fl_set_input_maxchars(obj,7 );
        fl_set_input_return(obj, FL_RETURN_END_CHANGED );
        fl_set_object_lsize(obj ,FL_LARGE_SIZE );
        fl_set_input(obj,"2.0") ;
        fl_set_object_callback(obj,expos_cb,32);


  jx = jx0+50;
  jy = jy1+2*dy0 ;
  dx = 25 ;
  dy = 25 ;
      obj = fl_add_button(FL_RADIO_BUTTON, jx,jy,dx,dy,"1");
      fl_set_object_callback(obj,filter_cb, 1  ) ;
      jx=jx+dx+1 ;
      obj = fl_add_button(FL_RADIO_BUTTON, jx,jy,dx,dy,"2");
      fl_set_object_callback(obj,filter_cb, 2  ) ;
      jx=jx+dx+1 ;
      obj = fl_add_button(FL_RADIO_BUTTON, jx,jy,dx,dy,"3");
      fl_set_object_callback(obj,filter_cb, 3  ) ;
      jx=jx+dx+1 ;
      obj = fl_add_button(FL_RADIO_BUTTON, jx,jy,dx,dy,"4");
      fl_set_object_callback(obj,filter_cb, 4  ) ;
      jx=jx+dx+1 ;
      obj = fl_add_button(FL_RADIO_BUTTON, jx,jy,dx,dy,"5");
      fl_set_object_callback(obj,filter_cb, 5  ) ;
      fl_set_button( obj, 1) ;     
      jx=jx+dx+1 ;
  fl_end_group();

  jx  = jx0 +190 ;
  jy  = jy1 + 12 ;
  FLtemp0 =
  obj = fl_add_box(FL_FLAT_BOX,jx,jy,1,25," 0.0");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_lalign(obj ,FL_ALIGN_RIGHT);
  jy  =jy+dy0 ;
  FLexpos0 =
  obj = fl_add_box(FL_FLAT_BOX,jx,jy,1,25," 2.3");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_lalign(obj ,FL_ALIGN_RIGHT);

//------------------------------------------------------------------

  dy0 = 30 ;
  jx  = jx0 ;
  jy  = jy0 +320 ;
  dy  = dy0 *2 ;
  FL_Action_grp = fl_bgn_group();
  obj = fl_add_button(FL_PUSH_BUTTON,jx,jy,225,dy,"Take DarkFrame");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_callback(obj,pict_cb, A_DARK );
        fl_set_object_color( obj , FL_COL1 , FL_YELLOW);
  jy  = jy +dy ;
  obj = fl_add_button(FL_PUSH_BUTTON,jx,jy,225,dy,"Take Picture");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_callback(obj,pict_cb, A_PICT);
        fl_set_object_color( obj , FL_COL1 , FL_YELLOW);
  jy  = jy +dy ;
  obj = fl_add_button(FL_PUSH_BUTTON,jx,jy,225,dy,"Loop Picture");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_callback(obj,pict_cb, A_LOOP);
        fl_set_object_color( obj , FL_COL1 , FL_YELLOW);
  jy  = jy +dy ;
  obj = fl_add_button(FL_PUSH_BUTTON,jx,jy,225,dy,"Save Picture");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_callback(obj,save_cb, 0);
        fl_set_object_color( obj , FL_COL1 , FL_YELLOW);
  fl_end_group();
//------------------------------------------------------------------

  jx  = jx0 ;
  jy  = jy0 +585 ;
  obj = fl_add_box(FL_FLAT_BOX,jx,jy,1,25,"ViewMode");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_lalign(obj ,FL_ALIGN_RIGHT);
  jy  = jy  - 10;
  jx  = jx0 +112;
  dy  = dy0*1.5 ;
  fl_bgn_group();
     obj = fl_add_button(FL_RADIO_BUTTON,jx,jy,112,dy,"linear");
           fl_set_object_lsize( obj ,FL_LARGE_SIZE );
           fl_set_object_callback(obj,view_cb, V_LIN);
     jy  = jy +dy ;
     obj = fl_add_button(FL_RADIO_BUTTON,jx,jy,112,dy,"Sqrt");
           fl_set_object_lsize( obj ,FL_LARGE_SIZE );
           fl_set_object_callback(obj,view_cb, V_SQRT);
     jy  = jy +dy ;
     obj = fl_add_button(FL_RADIO_BUTTON,jx,jy,112,dy,"log");
           fl_set_object_lsize( obj ,FL_LARGE_SIZE );
           fl_set_object_callback(obj,view_cb, V_LOG);
           fl_set_button( obj, 1) ;     
           g_viewmode = V_LOG ;
     jy  = jy +dy ;
     obj = fl_add_button(FL_RADIO_BUTTON,jx,jy,112,dy,"HistEq");
           fl_set_object_lsize( obj ,FL_LARGE_SIZE );
           fl_set_object_callback(obj,view_cb, V_HIST);
     jy  = jy +1.5*dy ;
     obj = fl_add_button(FL_PUSH_BUTTON,jx,jy,112,dy,"Invert");
           fl_set_object_lsize( obj ,FL_LARGE_SIZE );
           fl_set_object_callback(obj,view_cb, V_INV);
     jy  = jy +dy ;
  fl_end_group();

//------------------------------------------------------------------

  jx  = jx0 ;
  jy  = jy0 + 815 ;
  dy  = 2*dy0 ;
  FLexit =
  obj = fl_add_button(FL_NORMAL_BUTTON,jx,jy,100,dy,"EXIT");
        fl_set_object_lsize( obj ,FL_LARGE_SIZE );
        fl_set_object_callback( obj,exit_cb, 0);

  fl_end_form();
}

//--------------------------------------------------------------------

int
main(int argc, char *argv[])
{
  FL_OBJECT *obj;
  int ret ;

  sprintf(filter,"1");
  sprintf(pangrp,"(99,99)");

  strncpy(fname,"!SBig_Test0.fits\n",1000) ;
  fl_initialize(&argc, argv, "MAGIC - SBig", 0, 0);
  sbig_form();

  set_status(S_NOTREADY, 0);
  ret = InitST7( 0);
  if (ret =0) set_status(S_READY, 0);
  temp_cb(NULL,0);
  expos_cb(NULL,200);

//fl_set_idle_callback( sbig_idle, 0);
  fl_show_form(form,FL_PLACE_CENTER,FL_TRANSIENT,"Sbig") ;

  fl_do_forms(); 

  ret = CloseST7( 0);
  return 0;
}

