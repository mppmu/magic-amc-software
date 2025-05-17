#include <stdio.h>
#include <stdlib.h>
#include "st7.h"

short st7Error;
static unsigned short *myImage;
unsigned short *pImage;
unsigned short *thePicture(void) { return myImage; }
struct timeval  *tv, atv;
struct timezone *tz, atz;
long t1,t2,s1,s2;


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

short ShotAPicture(double exposureInSec, unsigned short *image)
{   unsigned short row;
    static StartExposureParams sePrm;
    static QueryCommandStatusParams qPrm;
    static QueryCommandStatusResults qRes;
    static EndExposureParams eePrm;
    static StartReadoutParams srPrm;
    static ReadoutLineParams rPrm;
    static EndReadoutParams erPrm;
    int i;

    /*  Setup parms  */
    sePrm.ccd = eePrm.ccd = srPrm.ccd = rPrm.ccd = erPrm.ccd = CCD_IMAGING;
    sePrm.exposureTime = (unsigned long)(100.*exposureInSec+.5);
    sePrm.abgState = ABG_NOT_PRESENT;
    sePrm.openShutter = SC_OPEN_SHUTTER;
    qPrm.command = CC_START_EXPOSURE;
    srPrm.readoutMode = rPrm.readoutMode = 0;
    srPrm.top = srPrm.left = rPrm.pixelStart = 0;
    srPrm.width = rPrm.pixelLength = st7width;
    srPrm.height = st7height;

    /*  Shoot the Picture  */

    if (!(st7Error=SBIGUnivDrvCommand(CC_START_EXPOSURE, &sePrm, NULL)))
    {   
        printf("exposing: st7Error %d \n", st7Error);
        gettimeofday( tv, tz);
        t1=atv.tv_usec;
        s1=atv.tv_sec-1100100000;
        i=exposureInSec;
        sleep(i);
        i=0;
        do if (!(st7Error
                   =SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS, &qPrm, &qRes)))
           {
//          printf("exposing: in if: st7Error %d qRes %x %d \n", st7Error, qRes, i++);
//          msleep(10);
//          return st7Error;
           }
           else 
           {
//          printf("exposing: st7Error %d qRes %x %d\n", st7Error, qRes, i++);
//          msleep(10);
           }
        while ((qRes.status & 0x0003) != 0x0003);
        gettimeofday( tv, tz);
        t2=atv.tv_usec;
        s2=atv.tv_sec-1100100000;
        s2=s2-s1;
        t2=t2-t1;
        if(t2 < 0 ) {s2=s2-1; t2=t2+1000000;}
        printf("exposing time: %d %d \n", s2,t2);
        st7Error=SBIGUnivDrvCommand(CC_END_EXPOSURE, &eePrm, NULL); 
    }
    if (st7Error) return st7Error;


    /*  Read data  */
    if (!image) image=myImage;  /*  set default memory array  */

    pImage=image;
    if (!(st7Error=SBIGUnivDrvCommand(CC_START_READOUT, &srPrm, NULL)))
    {   printf("read0 error %d %d %d \n",st7Error,st7height,st7width);
        for (row=0; row<st7height; row++, image+=st7width)
            if (st7Error=SBIGUnivDrvCommand(CC_READOUT_LINE, &rPrm, image))
            { //printf("read1 error %d \n",st7Error);
                return st7Error;             
            }
        printf("read2 error %d all done \n",st7Error);
    }
    else printf("read3 error %d no readout\n",st7Error);

    /*  End reading data  */
    return st7Error=SBIGUnivDrvCommand(CC_END_READOUT, &erPrm, NULL);
}   /*  end of ShotAPicture  */
