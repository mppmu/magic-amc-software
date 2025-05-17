//#include "di16.h"
#include "/usr/include/sbigudrv.h"
//#include "/usr/local/sbig/sbigudrv.h"
//#include "/usr/local/sbig/csbigcam.h"
//#include "/usr/local/sbig/csbigimg.h"

const short CE_DI16_ERROR = -1;
const short CE_NOMEMORY = -2;
//const unsigned long di16 = USBOPTO16IO;  /* DI16 kind (leave it USBOPTO16IO) */
const unsigned short di16line = 0;   /* di16-line to sw/on st7. 0:line #1,.. */
const unsigned short st7 = DEV_USB;           /* ST7 kind (leave it DEV_USB) */
const double kTempError = 100.;          /* retval on cooling error: celsius */
const double kTempSetpoint = 0.;     /* setpoint for cooling system: celsius */
//const unsigned short st7width = 320;
//const unsigned short st7height = 200;
const unsigned short st7width  = 1024;
const unsigned short st7height = 1024;

extern short st7Error;

short SetCoolingOn(double celsius);
short SetCoolingOff(void);
double GetTemperature(void);

short InitST7(short turnItOn);   /*bool: 0:don't care; !0:turn the switch on */
short CloseST7(short turnItOff); /*bool: 0:don't care; !0:turn the switch off*/
//short GetST7Status(void);

/*  1) take a picture for "exposureInSec" seconds,
    2) wait until the picture is taken and downloaded
    3) load it into "image" is not null, else store in a private buffer
       that can be addressed via unsigned short *thePicture(void)  */
short ShotAPicture(double exposureInSec, unsigned short *image, int x0, int y0, int xwid, int ywid, int dark);
unsigned short *thePicture(void);
