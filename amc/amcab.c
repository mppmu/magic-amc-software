//size of SBIG CCD pixel:  1 Pixel  = 2.28mm on the Camera-Lid ==> 1mm=0.44mm
#define pixPerMM 0.44

/*   0---------------> x
 *   |
 *   |
 *   V  y
 */

// g_AMCstat: 0 = error               4 = adjusted
//            1 = parked              5 = laser on
//            2 = initialized         6 = moving
//            3 = need adjust

#include "forms.h"
#include "flimage.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include "AMCconst.h"
#include "AMCpanel.h"
#include "AMCpower.h"
#include "AMCgui.h"
#include "CC.h"
#include "st7.h"
#include "st7temp.c"
#include "fitsio.h"
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>

#include "cursor1.xbm"
#include "curmsk1.xbm"
int myXC_cursor;
char xtmp[100];

short st7Error;

FL_FORM *form;

//AMC gui
FL_OBJECT *FLpanel[17][17], *FLhidden, *FLexit, *FLselect1_grp, *FLselect2_grp, *FLinp_dummy,
          *FLautoFoc, *FLautoAdj, *FLinfo, *FLmova, *FLmovb, *FLmov_grp, *FLlog_long, *FLlog_short,
          *FLset_grp, *FL_lasoff, *FLtemp, *FLfilt[6],
          *FLpwr_grp, *FLpwr[32], *FLset_X, *FLset_Y, *FLset_Foc, *FLset_Az, *FLset_Zd, *FL_scroll, *FL_durat,
          *FL_CC_grp, *FLin_time, *FLin_Zd, *FLin_Az, *FLin_Lid, *FLin_ALED, *FLin_DRIV, *FLsbig_on,
          *FLin_X, *FLin_Y, *FLin_Foc, *FLin_AMC, *FLstarFoc, *FLlab[6], *FLval0, *FLsbig_grp, *FLsbig_cmd, *FLsbig_view,
          *FLcmd_time, *FLcmd_cmd, *FLcmd_rslt, *FLout_info, *FLout_AMC, *FLout_PWR, *FLout_SBIG,
          *FLsbig_temp, *FLsbig_expo, *FLsbig_filt, *FLsbig_cycimg, *FLsbig_file, *FLfilt_cmd, *FLdisto,
          *FLsbig_scrol, *FLfilt1, *FLpwrqu, *FLpwrdf, *FLdistoff, *FLdistlas, *FLdistance,
          *FLcounter[6][2], *FLcounter_grp, *FLccdpix, *FLbreak, *FLtest,
          *FL_actIni, *FL_actInif, *FL_actAdj, *FL_actCen, *FL_actZer, *FL_actDef, *FLmod_grp, *FL_pict, *FL_dark, *FL_cycle,
          *FLdist_on, *FLsbig_ymin, *FLsbig_ymax, *FLread_lut, *FLmodM, *FLmodX, *FLmodO, *FLmodA,
          *canvas, *cnv1, *cnv2, *cnvx;
FL_OBJECT *FLsbig_th = NULL;
FL_OBJECT *FLsbig_sl = NULL;

FL_IMAGE  *image,  *img1, *img2;
FLIMAGE_SETUP mysetup;
FL_Coord  xpos, ypos;
unsigned  keymask;

Window     WINccd;
Window     WINpwr;

// SBIG
long g_viewmode = V_LOG;
long g_viewold  = V_LOG;
long g_viewinv  = 1;
long g_newpic   = 0;
long g_xmin     = 0;
long g_xmax     = 1024;
long g_ymin     = 0;
long g_ymax     = 1024;
long g_x0, g_y0; // borders of the SBIG image
long g_filter   = 1;
long g_oldfilt  = 1;
long g_sbigon   = 0; //is SBIG switched on ???
long g_sbig_err = 0; //does SBIG report an error ???
long g_sbig_brk = 0; //shall we interrupt SBIG ???
long g_autoadjt = 0; //at what time shall we do next autoadjust ?  // not used ?
long g_updtime  = 999999; //at what time shall we do next autoadjust ?

long gpict_x0   = 5;
long gpict_y0   = 259;

float g_cc_cFoc, g_cc_cX, g_cc_cY, g_cc_cAzi, g_cc_cZen, g_cc_az, g_cc_zd;
int g_cc_tosock = 0, g_cc_start = 0, g_cc_errcnt = -10;
long g_cc_tim0, g_cc_time, g_cc_timc, g_cc_sec, g_cc_tosec, g_cc_tostat; //containing time for report and command...
long g_cc_utim;
long g_cc_cmd = 0;  // CC-command to execute ...
long g_ccsnd = 0;   // 0=manual mode (no cc), >0 cc mode, <0 exit
long g_ccrec = 0;   // 0=manual mode (no cc), >0 cc mode, <0 exit
long g_ccpos = 0;   // pointer into local buffer 'from CC'
long g_ccstat = -1; // no status ...
char g_ccbuf[MAXMSG * 2];
char g_cc_name[MAXMSG];
char g_cc_source[MAXMSG];

int g_AMCstat, g_AMCstat0, g_AMCstat1;
int g_COMstat, g_yy, g_mon, g_dd, g_hh, g_mm, g_ss, g_ms;
int            g_all, g_err, g_mov, g_nin, g_com, g_ok;
int            g_zd0, g_az0, g_fc0, g_tim0, g_tim0x;
int            g_zd1, g_az1, g_fc1, g_tim1, g_tim1x;
int g_i, g_j, g_nsele = 0;

int g_lidstat = -9;   //lid-state: allow lasers only when 'closed' = 4
int g_drvstat = -9;   //drv-state: allow autoad only when 'tracking= 4

char a_sb_name[80], g_datstr[80];
long g_sb_shutt = SC_OPEN_SHUTTER;
long a_sb_shutt = SC_OPEN_SHUTTER;
long g_sb_mode = 0;
long a_sb_mode = 0;
long g_sb_first = 1;     //is it first picture in a series ???
double g_expos = 0.5;
int g_cycimg = 0;
double g_sbig_scrol = 0.0;
double g_sbig_dscr  = 0.0;
double g_sbig_temp  = -10.0;
int TMexp = 0, TMtemp = 0, TMpict;

double g_scrol_max = 0.;

//char sbig_path[256] = "/home/operator/sbig1_local";
char sbig_path[256] = "/home/operator/sbig1";

unsigned short img_buffer[C_WID][C_HIG];
unsigned short img_buff2[C_WID / 2][C_HIG / 2];
static unsigned short *myImage;

int g_initgui = 0;             //first initialization

int g_dist_th  = S_NULL_TH;
int g_sbig_th  = S_NULL_TH;
int g_sbiga_th = S_NULL_TH;

int g_amc_th = 0;
int g_exit = -1;

int g_pwr_th = PWR_0_TH;
int g_pwr_req = 1;             //do we have to set power ???
int g_pwr_qry = 0;             //do we have to query power ???
int g_pwr_cnt = 0;
int g_pwr_ret = 0;             //power return code
int g_pwr_stat = 0;            //did power ever run ?
int g_pwr_err = 0;             //numer of power-errors in sequence
int g_ret = 0;                 //some dummy return flag for debug

int g_pictcnt    = 0;
int g_pict_expos = 0;

unsigned long g_pwr_slp = 0;   //actual sleep until next power check
unsigned long isec, iusec, isec0, iusec0, isec1, iusec1;
unsigned long idlsec;
unsigned long g_ccisec, g_cciusec;

#define LOGLEN 1000
long g_numstr = 0;
char g_logstr[LOGLEN][160];    //buffer to store log-strings

int g_log_col[20];
char g_log_chr[20];

int g_exiton = -1;

int g_ifID = 0;
int g_ifIDold =0;
int g_ifIDuse =0;

char lstr[LOGLEN];
char tstr[LOGLEN];

FILE *f_log, *f_param, *f_err;

pthread_t dist_threads[2], sbig_threads[2], pwr_threads[2], amc_threads[2], sbig_autoth[2], cc_threads[2];

//picture headers (SBIG): take picture-->'tmp'; readout-->'pct'
double tmp_expos, tmp_temp, tmp_azim, tmp_zenit, tmp_foclen, tmp_ymin, tmp_ymax;
double pct_expos, pct_temp, pct_azim, pct_zenit, pct_foclen, pct_ymin, pct_ymax;
char   tmp_pangrp[80], tmp_filter[80], tmp_imgtyp[80], tmp_fulldat[80];
char   pct_pangrp[80], pct_filter[80], pct_imgtyp[80], pct_fulldat[80];
char   act_pangrp[80];

//----------------------------------------

void *TakePict_th(void *threadid);
void *Filter_th(void *threadid);
void *CloseST7_th(void *threadid);
void *StandbyST7_th(void *threadid);
void *InitST7_th(void *threadid);
void *InitDST_th(void *threadid);
void *CloseDST_th(void *threadid);
void *From_CC_th(void *threadid);

void gen_sbig(int ix, int iy, int dx, int dy);
void gen_disto(int ix, int iy, int dx, int dy);
void action_cb(FL_OBJECT *ob, long cmd);
void mode_cb(FL_OBJECT *ob, long m);
void push_pan(FL_OBJECT *ob, long n);
void sbigonoff_cb(FL_OBJECT *ob, long m);
void upd_pixmap();
void save_cb(FL_OBJECT *ob, long n);
void sbig_cb(FL_OBJECT *ob, long n);
short PictureST7(double exposureInSec, unsigned short *pict, int x0, int y0, int xwid, int ywid, int shutter, int mode);

//----------------------------------------

int global_action = -9;
int global_break;
int global_mode;
int global_autoAdj;       //autoadjust-mode ?
int global_autoFoc;       //autofocus-mode ?
int global_starFoc;       //starfocus-mode ?
int global_zenith = 20;   //zenith angle
int global_azimut = 0;    //azimut angle
int global_foclen = 0;    //foclen correction
int global_view;          //sbig view-mode
double global_xoff = 0;
double global_yoff = 0;

char glob_info[LOGLEN] = "this\n is\n a\n text\n";

AMCpower power[32];
AMCpanel panel[17][17];
AMCpanel *pa[600];
AMCpanel *pb[600];

//----------------------------------------
//steering SBIG from CC

double g_cc_expos = 50;  //exposure time (sec*100)
int  g_sbig_stat = 0;    //status of SBIG   0=off,1=switching off, 2=switching on, 3=ready, 4=busy, 9=error
int  g_cc_temp = -10;    //temperature to set
int  g_cc_filt = 1;      //filter to set
int  g_sbig_cc = 0;      //sbig command from cc ?
int  g_pict_filt = 9;    //0 = darkpict, else filter ?

//----------------------------------------

AMCbutton p_but[17][17];

struct AMCcommand {
  int grp;      //-1 = not used;  MODE_AUTO, MODE_OPER, MODE_MANU, MODE_EXPRT
  int cmd;
  int stat;      //>0 = active; 0=not active
  FL_OBJECT *obj;
  char txt[10];
} command[50];

struct p_s {                     //information about the currently active panel
  int i, j, io, jo;            //     panel-id
  double xmov, ymov;
  int   ixmov, iymov;
} p_sele;

#define  MODE_AUTO  100
#define  MODE_OPER  200
#define  MODE_EXPT  300
#define  MODE_MANU  400
#define  MODE_UDEF  900

#define  DEF_AUTOFOC  0
#define  DEF_AUTOADJ  0
#define  DEF_STARFOC 0

//===========================================================================
void CC_connect(int, int);
//===========================================================================

void col_define(void)
{
  int i;

  fl_mapcolor(100            , COL_GRAY);

  fl_mapcolor(110 +  STAT_DIS, COL_DIS_1);
  fl_mapcolor(110 +  STAT_COM, COL_COM_1);
  fl_mapcolor(110 +  STAT_BAD, COL_BAD_1);
  fl_mapcolor(110 +  STAT_BCM, COL_BCM_1);
  fl_mapcolor(110 +  STAT_NIN, COL_NIN_1);
  fl_mapcolor(110 +  STAT_OK_, COL_OK__1);
  fl_mapcolor(110 +  STAT_ERR, COL_ERR_1);
  fl_mapcolor(110 +  STAT_BRK, COL_BRK_1);
  fl_mapcolor(110 +  STAT_TDO, COL_TDO_1);
  fl_mapcolor(110 +  STAT_ACT, COL_ACT_1);
  fl_mapcolor(110 +  STAT_OK1, COL_OK1_1);
  fl_mapcolor(110 +  STAT_ER1, COL_ER1_1);
  fl_mapcolor(110 +  STAT_OK2, COL_OK2_1);
  fl_mapcolor(110 +  STAT_ER2, COL_ER2_1);
  fl_mapcolor(110 +  STAT_SBG, COL_SBG_1);

  fl_mapcolor(210 +  STAT_DIS, COL_DIS_2);
  fl_mapcolor(210 +  STAT_COM, COL_COM_2);
  fl_mapcolor(210 +  STAT_BAD, COL_BAD_2);
  fl_mapcolor(210 +  STAT_BCM, COL_BCM_2);
  fl_mapcolor(210 +  STAT_NIN, COL_NIN_2);
  fl_mapcolor(210 +  STAT_OK_, COL_OK__2);
  fl_mapcolor(210 +  STAT_ERR, COL_ERR_2);
  fl_mapcolor(210 +  STAT_BRK, COL_BRK_2);
  fl_mapcolor(210 +  STAT_TDO, COL_TDO_2);
  fl_mapcolor(210 +  STAT_ACT, COL_ACT_2);
  fl_mapcolor(210 +  STAT_OK1, COL_OK1_2);
  fl_mapcolor(210 +  STAT_ER1, COL_ER1_2);
  fl_mapcolor(210 +  STAT_OK2, COL_OK2_2);
  fl_mapcolor(210 +  STAT_ER2, COL_ER2_2);
  fl_mapcolor(210 +  STAT_SBG, COL_SBG_2);

  fl_mapcolor(310 +  STAT_DIS, COL_DIS_3);
  fl_mapcolor(310 +  STAT_COM, COL_COM_3);
  fl_mapcolor(310 +  STAT_BAD, COL_BAD_3);
  fl_mapcolor(310 +  STAT_BCM, COL_BCM_3);
  fl_mapcolor(310 +  STAT_NIN, COL_NIN_3);
  fl_mapcolor(310 +  STAT_OK_, COL_OK__3);
  fl_mapcolor(310 +  STAT_ERR, COL_ERR_3);
  fl_mapcolor(310 +  STAT_BRK, COL_BRK_3);
  fl_mapcolor(310 +  STAT_TDO, COL_TDO_3);
  fl_mapcolor(310 +  STAT_ACT, COL_ACT_3);
  fl_mapcolor(310 +  STAT_OK1, COL_OK1_3);
  fl_mapcolor(310 +  STAT_ER1, COL_ER1_3);
  fl_mapcolor(310 +  STAT_OK2, COL_OK2_3);
  fl_mapcolor(310 +  STAT_ER2, COL_ER2_3);
  fl_mapcolor(310 +  STAT_SBG, COL_SBG_3);

  fl_mapcolor(400, PCL_OK_1);   //    green
  fl_mapcolor(401, PCL_NOK_1);  //    yellow
  fl_mapcolor(402, PCL_WRN_1);  //    orange
  fl_mapcolor(403, PCL_ERR_1);  //    red
  fl_mapcolor(404, PCL_NDEF);  //    blue
  fl_mapcolor(405, PCL_BUSY);  //    cyan

  fl_mapcolor(410, PCL_OK_2);
  fl_mapcolor(411, PCL_NOK_1);
  fl_mapcolor(412, PCL_WRN_2);
  fl_mapcolor(413, PCL_ERR_2);
  fl_mapcolor(414, PCL_NDEF);
  fl_mapcolor(415, PCL_BUSY);

  fl_mapcolor(800, 64, 200, 64);
  fl_mapcolor(801, 255, 255,  0);
  fl_mapcolor(802, 255, 64, 64);
  fl_mapcolor(803, 255,  0, 255);
  fl_mapcolor(804, 128, 128, 255);
  fl_mapcolor(805,  0, 255, 255);
  fl_mapcolor(806, 255, 255, 255);
  fl_mapcolor(807,  0,  0,  0);
  fl_mapcolor(808, 128, 128, 128);

  fl_mapcolor(900           , LAB_ACT);
  fl_mapcolor(901           , LAB_DCT);
}

//--------------------------------------------------------------------
void upd_logout(int n)             //should be made threadsave ....
{
  int locnum = 0;

  while (locnum < g_numstr) {
    g_logstr[locnum][120] = '\0';
    fl_addto_browser(FLlog_short, &g_logstr[locnum][0]);
    fl_addto_browser(FLlog_long, &g_logstr[locnum][0]);
    locnum++;
  }
  g_numstr = 0;

}

//--------------------------------------------------------------------
void put_logfile(int type, int flag, char* str)
{
  long utime[5];
  char wc;
  int  col = 6;
  int  k;

  if (type < 0 || type > 19) k = 19;
  else k = type;

  AMCtime(utime);

  if (flag >= 0) {        //output to log display
    snprintf(&g_logstr[g_numstr][0], LOGLEN, "@C%d %06d %s", g_log_col[k], utime[0], str);
    if (g_numstr < 100) g_numstr++;
  }

  if (flag <= 0) {       //output to log-file
    fprintf(f_log, "%06d %c %s\n", utime[0], g_log_chr[k], str);
  }

  if (k == LOG_ERR || k == LOG_SVR) fprintf(f_err, "%06d %s\n", utime[0], str);

  /*
    add a line to the logfile (no \n!!!)

    flag: -1: only to logfile
           0: logfile and display
           1: display only

       fl_addto_browser(FLlog_short, str);   */

}

//--------------------------------------------------------------------

void shiftfoc(int dFoc, double dXx, double dYy, int flag)
{
  int i, j;
  double dF0 = 17000.0;
  double dd, di, dj, dh, df, dcor, dx, dy, dx0, dy0, motA, motB, dx1, dy1, dd1;

  if (flag > -99) {
    sprintf(lstr, "shiftFoc F=%d X=%f Y=%f | ifID_new=%d  ifID_old=%d", dFoc, dXx, dYy, g_ifID, g_ifIDold);
    put_logfile(LOG_DB1, flag, lstr);
  }

  dx1 = dXx;
  dy1 = dYy;
  dd1 = dFoc;

  if (g_ifID != g_ifIDold) {
     g_ifIDuse = IF_read(panel, g_ifID); 
     if (g_ifIDuse != g_ifID ) {
       sprintf(lstr, "ifID file %d not found",g_ifID) ;
       put_logfile(LOG_ERR, flag, lstr);
     }
     g_ifIDold = g_ifIDuse ;
  }

  for (i = 0; i < 17; i++) {
    di = (i - 8) * 1000.;
    for (j = 0; j < 17; j++) {
      dj = (j - 8) * 1000.;
      if (panel[i][j].pan_stat > STAT_NIN) {

        dx0 = dx1 + panel[i][j].loc_x ;
        dy0 = dy1 + panel[i][j].loc_y ;
        dd  = dd1 + panel[i][j].loc_z ;

        dh = di * di + dj * dj;
        df = dF0 - dh / (4.*dF0); // Parabola sqrt(df^2 + dh^2) + df = 2*df0
        dcor = dd / (dd + df); // Strahlensatz
        panel[i][j].mov_dxy[0] = dx = di * pixPerMM * dcor + dx0;
        panel[i][j].mov_dxy[1] = dy = dj * pixPerMM * dcor + dy0;

        motA = panel[i][j].cali_spp[0] *
               (dx * sin(panel[i][j].cali_slp[1]) - dy * cos(panel[i][j].cali_slp[1]))
               / sin(panel[i][j].cali_slp[1] - panel[i][j].cali_slp[0]);
        motB = panel[i][j].cali_spp[1] *
               (dx * sin(panel[i][j].cali_slp[0]) - dy * cos(panel[i][j].cali_slp[0]))
               / sin(panel[i][j].cali_slp[0] - panel[i][j].cali_slp[1]);
        panel[i][j].mov_dab[0] = rint(motA / 2.); // 0.5 because calib with laser, and make sure even numbers ...
        panel[i][j].mov_dab[1] = rint(motB / 2.);
      }
      else {
        panel[i][j].mov_dab[0] = 0;
        panel[i][j].mov_dab[1] = 0;
      }
    }
  }
}

//--------------------------------------------------------------------

int AMCtime(long utime[5])
{
  struct tm       *ttm, atm;
  struct timeval  *tv, atv;
  tv = &atv;
  time_t t;

  gettimeofday(tv, NULL);
  utime[2] = atv.tv_sec;
  utime[3] = atv.tv_usec / 1000;

  t   = atv.tv_sec;
  ttm = localtime(&t);
  atm = *ttm;

  utime[0] =  atm.tm_hour     * 10000 + atm.tm_min * 100 + atm.tm_sec;
  utime[1] = (atm.tm_year - 100) * 10000 + (atm.tm_mon + 1) * 100 + atm.tm_mday;

  sprintf(g_datstr, "%04d %02d %02d %02d %02d %02d %03d",
          atm.tm_year + 1900, atm.tm_mon + 1, atm.tm_mday,
          atm.tm_hour, atm.tm_min, atm.tm_sec, utime[3]);

  utime[5] = atm.tm_hour     * 3600 +  atm.tm_min * 60 + atm.tm_sec;
  if (atm.tm_hour < 12)
    utime[5] = utime[5] + 12 * 3600;

  return(0);
}

//--------------------------------------------------------------------

int gen_logfile()
{
  long utime[5];
  char fname[LOGLEN];
  int k;

  AMCtime(utime);
  sprintf(fname, "AMC1_%06d_%06d.log", utime[1], utime[0]);

  f_log = fopen(fname, "w");
  setlinebuf(f_log); //flush buffer at each end of line

  f_err = fopen("AMC1_err.logall", "a");  //append to standard file
  setlinebuf(f_err); //flush buffer at each end of line

  fprintf(f_log, "%06d X V4.50 AMC_%06d_%06d.log\n", utime[0], utime[1], utime[0]);
  fprintf(f_err, "%06d --V4.50 AMC_%06d_%06d.log --- \n", utime[1], utime[1], utime[0]);

  for (k = 0; k < 20; k++) {
    g_log_col[ k] = 6;
    g_log_chr[ k] = ' ';
  }

  k = LOG_INF;
  g_log_col[k] = 7;
  g_log_chr[k] = 'I'; // 0=info
  k = LOG_OK_;
  g_log_col[k] = 2;
  g_log_chr[k] = 'O'; // 1=ok
  k = LOG_WRN;
  g_log_col[k] = 8;
  g_log_chr[k] = 'W'; // 2=warning
  k = LOG_ERR;
  g_log_col[k] = 3;
  g_log_chr[k] = 'E'; // 3=error
  k = LOG_SVR;
  g_log_col[k] = 1;
  g_log_chr[k] = 'S'; // 4=severe error
  k = LOG_DB1;
  g_log_col[k] = 5;
  g_log_chr[k] = '1'; // 5=debug 1
  k = LOG_DB2;
  g_log_col[k] = 5;
  g_log_chr[k] = '2'; // 6=debug 2
  k = LOG_DB3;
  g_log_col[k] = 5;
  g_log_chr[k] = '3'; // 7=debug 3
  k = LOG_DB4;
  g_log_col[k] = 5;
  g_log_chr[k] = '4'; // 8=debug 4
  k = LOG_DB5;
  g_log_col[k] = 5;
  g_log_chr[k] = '5'; // 9=debug 5
  k = LOG_DB6;
  g_log_col[k] = 5;
  g_log_chr[k] = '6'; //10=debug 6
  k = LOG_DB7;
  g_log_col[k] = 5;
  g_log_chr[k] = '7'; //11=debug 7
  k = LOG_DB8;
  g_log_col[k] = 5;
  g_log_chr[k] = '8'; //12=debug 8
  k = LOG_DB9;
  g_log_col[k] = 5;
  g_log_chr[k] = '9'; //13=debug 9
  k = LOG_HRT;
  g_log_col[k] = 5;
  g_log_chr[k] = 'H'; //14=heart beat
  k = LOG_CC_;
  g_log_col[k] = 5;
  g_log_chr[k] = 'C'; //15=cmd from cc
  k = LOG_ACT;
  g_log_col[k] = 5;
  g_log_chr[k] = 'A'; //16=info actuators
  k = LOG_PWR;
  g_log_col[k] = 5;
  g_log_chr[k] = 'P'; //17=info power
  k = LOG_SBG;
  g_log_col[k] = 5;
  g_log_chr[k] = 'G'; //18=info sbig
  k = LOG_LAS;
  g_log_col[k] = 5;
  g_log_chr[k] = 'L'; //19=info laser

  return(0);
  /*
   for color, start str  @C<i>
     i=0 black
     i=1 red      severe error
     i=2 green    ok
     i=3 yellow   warning
     i=4 blue
     i=5 magenta  debug
     i=6 cyan     unknown
     i=7 white    comment
     i=8 orange   error
     i=9 wheat
     i=10 violet
     i=11 gray
       18 light green
       19 dark yellow
   */
}

//--------------------------------------------------------------------

void *Power_th(void *threadid)
{
  int i, j, ireq, jreq, nominal[32], status[32], ret;
  int jloc = 0;

  while (g_exit >= 0) {
    sleep(1);  //just do nothing
  }
  return NULL;

  sprintf(lstr, "run power thread");
  put_logfile(LOG_DB1, 0, lstr);

  while (g_exit >= 0) {
    jreq = 0;

    for (j = 0; j < 32; j++) power[j].old_nom = power[j].old_act;

    if (g_pwr_req == 0) {
      g_pwr_qry = 0;
      sprintf(lstr, "qry power");
      put_logfile(LOG_PWR, 0, lstr);
      for (j = 0; j < 32; j++) {
        i = power[j].chan;
        nominal[i] = power[j].nominal = power[j].request;
        status[i] = power[j].actual;
      }
      g_pwr_ret = AMC_exec_pwr(0, &nominal, &status); //query the status
      if (g_pwr_ret == 0)
        if (g_pwr_stat == 0) g_pwr_stat = 1; //flag we know about power ..

      for (j = 0; j < 32; j++) {
        i = power[j].chan;
        power[j].actual = status[i];
        power[j].old_act = nominal[i];
      }
    }
    else {                                      //make sure to set the power as requested
      sprintf(lstr, "set power %d", g_pwr_req);
      put_logfile(LOG_PWR, 0, lstr);
      g_pwr_req = 0;
      do {
        for (j = 0; j < 32; j++) {
          i = power[j].chan;
          nominal[i] = power[j].nominal = power[j].request;
          status[i] = power[j].actual;
        }
        g_pwr_ret = AMC_exec_pwr(1, &nominal, &status);

        if (g_pwr_ret == 0) {
          if (g_pwr_stat == 0) g_pwr_stat = 1; //flag we know about power ..
          ireq = 0;
          for (j = 0; j < 32; j++) {
            i = power[j].chan;
            power[j].actual = status[i];
            if (jreq == 0) power[j].old_act = nominal[i]; //do it only in the first trial ...
            if (power[j].nominal != power[j].request) ireq++;
          }
          jreq++;
        }
        else {
          g_pwr_req = 1;    //something went wrong; try to set again
          ireq = 0;         //not now, but next iteration
        }

      }
      while (ireq != 0);
    }
    g_pwr_th = -1;

    //will wake up if somebody is setting g_pwr_th > 0
    while (g_pwr_th <= 0 && g_exit >= 0) usleep(250000);
  }

  for (j = 0; j < 32; j++) {
    i = power[j].chan;
    nominal[i] = power[j].nominal = power[j].request;
    status[i] = power[j].actual;
  }
  g_pwr_ret = AMC_exec_pwr(1, &nominal, &status); //make sure everything as requested ...

  sprintf(lstr, "end power thread");
  put_logfile(LOG_PWR, 0, lstr);

  return NULL;
}

//--------------------------------------------------------------------

void upd_act_pan(int ii, int jj, int nsele, int mode)
{
  int k, i, j, ix, iy, l0, l1, l2;
  int ls0, ls1, ls2, ls3, ls4, ls;
  double x, y;
  char  str[LOGLEN];
  char  lstr[LOGLEN];
#define  n200 200
  char glob_inf0[n200];
  char glob_inf1[n200] = "this\n is\n a\n text\n";
  char glob_inf2[n200] = "this\n is\n a\n text\n";
  char glob_inf3[n200] = "this\n is\n a\n text\n";
  char glob_inf4[n200] = "this\n is\n a\n text\n";

  if (mode >= 0) {
    if (ii < -50) {            //use old status
      ii = p_sele.i;
      jj = p_sele.j;
    }
    else {
      p_sele.i = ii;
      p_sele.j = jj;
    }
  }

  i = abs(ii);
  j = abs(jj);

  if (i < 17 && j < 17) {                //single panel active ///IB
    // ==> show all info
    if (mode >= 0) {  //display
      x = panel[i][j].act_mot[0];
      y = panel[i][j].act_mot[1];
      fl_set_counter_value(FLmova, x);
      fl_set_counter_value(FLmovb, y);
    }
    /*
    sprintf(glob_info," (%+1d%+1d)  motA   motB   lasR |         motA   motB  lasR |         motA  motB |         motA  motB\n
                         . Chain                       | Posit                     | hal-0              | ActPos            \n
                         . Addr                        | Versi                     | hal-1              | PC-Pos            \n
                         . Stat-1                      | Temp                      | hal-2              | HalPos            \n
                         . Stat-2                      | Humi                      | hal-3              | Status            ",
    */
//                                   1         2         3         4         5         6         7         8         9         0
//                      123 45  67 8901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901

    sprintf(glob_inf0, " (%+1d%+1d)       Driver    |          Driver    ||             Box   |            Box    ",  i - 8, j - 8);
    sprintf(glob_inf1, " Chain         %5d    |  Version %3d,%2d    ||   Temp    %5d   |  Vsup     %4d    ",
            panel[i][j].port[ panel[i][j].portflg[0]][0] + 1,
            panel[i][j].version[0][0], panel[i][j].version[0][1],
            panel[i][j].temp[0], panel[i][j].Vsup[0]);
    sprintf(glob_inf2, " Addr            %3x    |  Status1   0x%2.2x    ||   Humid   %5d   |  V2       %4d    ",
            panel[i][j].addr[2], panel[i][j].status[0][0], panel[i][j].humi[0], panel[i][j].V2[0]);
    sprintf(glob_inf3, " ActPos %5d  %5d    |  Status2   0x%2.2x    ||   Current %5d   |  Vlog     %4d    ",
            panel[i][j].act_mot[0], panel[i][j].act_mot[1],
            panel[i][j].status[0][1], panel[i][j].curr[0], panel[i][j].Vlog[0]);
    sprintf(glob_inf4, " PC-Pos %5d  %5d    |  Status    0x%2.2x    ||   pan_cent   %2d   |  pan_stat %4.4x    ",
            panel[i][j].pc_mot[0], panel[i][j].pc_mot[1], panel[i][j].actstat[0]
           , panel[i][j].pan_centred, panel[i][j].pan_stat);

    sprintf(glob_info, "%s\n%s\n%s\n%s\n%s", glob_inf0, glob_inf1, glob_inf2, glob_inf3, glob_inf4);

    if (mode >= 0) fl_set_object_label(FLinfo, glob_info);
    if (mode <= 0) {

      sprintf(lstr, "-------I-N-I------------------");
      put_logfile(LOG_DB1, -2, lstr);
      l0 = 0;

      put_logfile(LOG_DB1, -2, glob_inf0);
      put_logfile(LOG_DB1, -2, glob_inf1);
      put_logfile(LOG_DB1, -2, glob_inf2);
      put_logfile(LOG_DB1, -2, glob_inf3);
      put_logfile(LOG_DB1, -2, glob_inf4);

      sprintf(lstr,
              " freq  %6d %6d    | work %6d %6d  | hold %5d  %5d | endswl%5d %5d",
              panel[i][j].freq[0], panel[i][j].freq[1],
              panel[i][j].work[0], panel[i][j].work[1],
              panel[i][j].hold[0], panel[i][j].hold[1],
              panel[i][j].rndsw_lwl[0], panel[i][j].rndsw_lwl[1]);
      put_logfile(LOG_DB1, -2, lstr);

      //check for humidity problems
      /*      if (panel[i][j].humi[0] > 70) {
                 sprintf(lstr," Humi: %3d,  Actuator 0x%4x, (%2d,%2d):%1d  Zen=%3d",
                         panel[i][j].humi[0],
                         panel[i][j].addr[0],
                         i-8,j-8,
                         panel[i][j].port[ panel[i][j].portflg[0]] [0] +1,
                         global_zenith);
                 put_logfile(LOG_ERR, -2, lstr);
              }
              if (panel[i][j].humi[1] > 70) {
                 sprintf(lstr," Humi: %3d,  Actuator 0x%4x, (%2d,%2d):%1d  Zen=%3d",
                         panel[i][j].humi[1],
                         panel[i][j].addr[1],
                         i-8,j-8,
                         panel[i][j].port[ panel[i][j].portflg[1]] [1] +1,
                         global_zenith);
                 put_logfile(LOG_ERR, -2, lstr);
              }
      //I dont understand this; to be removed, I think, AB?
              if (panel[i][j].humi[2] > 70 && panel[i][j].addr[2] %4 ==0) {
                 sprintf(lstr," Humi: %3d,    BOX    0x%4x, (%2d,%2d):%1d  Zen=%3d",
                         panel[i][j].humi[2],
                         panel[i][j].addr[2],
                         i-8,j-8,
                         panel[i][j].port[ panel[i][j].portflg[2]] [2] +1,
                         global_zenith);
                 put_logfile(LOG_ERR, -2, lstr);
              }
      */

    }

  }

  if (mode >= 0) {
    k = global_mode;
    if (g_nsele == 1 && p_but[i][j].select > 0 && global_mode > MODE_OPER) {
      fl_activate_object(FLmov_grp);
      fl_set_object_lcol(FLmov_grp, 900);
    }
    else {
      fl_deactivate_object(FLmov_grp);
      fl_set_object_lcol(FLmov_grp, 901);
    }

    if (k == MODE_AUTO || k == MODE_OPER || ii >= 0 && jj >= 0)
      fl_set_object_lcol(FLinfo, 900);
    else
      fl_set_object_lcol(FLinfo, 901);
  }
}

//--------------------------------------------------------------------

void upd_all_pan(int mode)
{
  int i, j, ps;
  int npan = 0, nlas = 0, nsele = 0;

  fl_freeze_form(form);

  for (i = 0; i < 17; i++) {
    for (j = 0; j < 17; j++) {
      if (p_but[i][j].select > 0) nsele++;
      ps = panel[i][j].pan_stat;
      if (ps != STAT_NOT) {
        if (p_but[i][j].status !=  ps) {
          p_but[i][j].status  =  ps;
          fl_set_object_color(FLpanel[i][j], ps + 110, ps + 310);
          npan++;
        }
        if (panel[i][j].laser > LAS_NOT &&  p_but[i][j].laser  !=  panel[i][j].laser) {
          p_but[i][j].laser   =  panel[i][j].laser;
          if (p_but[i][j].laser != LAS_ON) p_but[i][j].str[6] = ' ';
          else                               p_but[i][j].str[6] = 'X';
          fl_set_object_label(FLpanel[i][j], p_but[i][j].str);
          nlas++;
        }
      }
    }
  }

  g_nsele = nsele;
  upd_act_pan(-999, -999, nsele, 1);

  fl_unfreeze_form(form);
}

//--------------------------------------------------------------------

void log_act(int i, int j, int logtype, int logmode,  int cmd)
{
  int ia;
  char lstr[LOGLEN];

  sprintf(lstr, "(%+02d,%+02d):%1d stat:%2d %2d %2d; act: %5d,%5d; pc: %5d %5d; soll: %5d,%5d; las %1d; cmd: %2d",
          i - 8, j - 8,
          panel[i][j].port[0][0],
          panel[i][j].pan_stat,
          panel[i][j].act_stat,
          panel[i][j].box_stat,
          panel[i][j].act_mot[0],
          panel[i][j].act_mot[1],
          panel[i][j].pc_mot[0],
          panel[i][j].pc_mot[1],
          panel[i][j].pc_moveto[0],
          panel[i][j].pc_moveto[1],
          panel[i][j].laser,
          cmd);
  put_logfile(logtype, logmode, lstr);
}

//--------------------------------------------------------------------

int AMC_exec_cmd2(AMCpanel panel[17][17], int cmd, int numpan, int i, int j)
{
  int iret, istat;

  if (numpan != 1) return -100;     //this should never happen ....
  if (i < 0 || j < 0 || i > 17 || j > 17) return -200;

  panel[i][j].pan_stat = STAT_TDO;
  AMC_check_gui(0, -2.);

  iret = AMC_exec_cmd(panel,  cmd,  numpan, i, j);

  if (panel[i][j].pan_stat == STAT_OK_ ||
      panel[i][j].pan_stat == STAT_OK1 ||
      panel[i][j].pan_stat == STAT_OK2) {
    iret = 0;
    log_act(i, j, LOG_ACT, -2, cmd);
  }
  else if (cmd == CMD_MOVE) {
    if (abs(panel[i][j].act_mot[0]) > 6000 ||
        abs(panel[i][j].act_mot[1]) > 6000) {  //query failed ==> redo it
      log_act(i, j, LOG_WRN, 0, CMD_MOVE);
      panel[i][j].pan_stat = STAT_TDO;
      AMC_exec_cmd(panel, CMD_QUERY, numpan, i, j);
      if (abs(panel[i][j].act_mot[0] - panel[i][j].pc_moveto[0]) < 10 &&
          abs(panel[i][j].act_mot[1] - panel[i][j].pc_moveto[1]) < 10) {
        panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_OK_;
        log_act(i, j, LOG_ACT, -2, CMD_MOVE);
        iret = 0;
      }
      else {
        panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_ERR;
        log_act(i, j, LOG_ERR, 0, CMD_MOVE);
        iret = -234;
      }
    }
    else {
      iret = -123;
      log_act(i, j, LOG_ERR, 0, cmd);
    }
  }

  AMC_check_gui(0, -2.);

  return iret;
}

//--------------------------------------------------------------------

void sbig_auto(int n)
{
  if (n < 0) {
    g_sbig_th = 999;                          //flag sbig as used
    fl_deactivate_object(FLsbig_on);
    fl_deactivate_object(FLsbig_cmd);
    fl_deactivate_object(FLfilt_cmd);
    fl_set_object_lcol(FLsbig_on, 901);
    fl_set_object_lcol(FLsbig_cmd, 901);
    fl_set_object_lcol(FLfilt_cmd, 901);
  }
  else {
    fl_activate_object(FLsbig_on);
    fl_activate_object(FLsbig_cmd);
    fl_activate_object(FLfilt_cmd);
    fl_set_object_lcol(FLsbig_on, 900);
    fl_set_object_lcol(FLsbig_cmd, 900);
    fl_set_object_lcol(FLfilt_cmd, 900);
    g_sbig_th = 0;                          //flag sbig as free
  }
}

//--------------------------------------------------------------------

void *AutoPict_th(void *threadid)
{
  int x, y, ret, x0, y0, xwid, ywid, shutt, mode, expt;
  double time = 0.;

  x0 = g_xmin;
  xwid = g_xmax - g_xmin;

  if (g_ymin < g_ymax) {
    y0 = g_ymin;
    ywid = g_ymax - g_ymin;
  }
  else {
    y0 = g_ymax;
    ywid = g_ymin - g_ymax;
  }

  shutt = a_sb_shutt;
  mode = a_sb_mode;
  g_sbig_scrol = 0.0;
  if (a_sb_mode == S_READ_TH) time = 3500. / ywid;
  if (a_sb_mode == S_EXPO_TH) time = g_expos + 1.;
  if (a_sb_mode == S_PICT_TH) time = g_expos + 1. + 3500. / ywid;
  g_sbig_dscr  = 1. / time;

  sprintf(lstr, "SBIGA Pict %d %f", a_sb_mode, time);
  put_logfile(LOG_SBG, 0, lstr);
  g_sbiga_th = g_sb_mode;      //flag thread as active
  ret = PictureST7(g_expos, &img_buffer, x0, y0, xwid, ywid, shutt, mode);

  g_sbiga_th = g_sb_mode * -1; //flag thread as finished
}

//--------------------------------------------------------------------


void pict_anal(int mode)
{
  int i, j, k;
  /*
    for (i=g_xmin; i<g_xmax; i++)
       for (j=g_ymin; j<g_ymax; j++)
          if (img_buffer[i][j]<20000) img_buffer[i][j]=0;
   */

}

//--------------------------------------------------------------------

void action_pola(int imod)
{
  //imod =1: Polaris;  imod =2: Roque

  int k, i, j, it, jt, istat, ix, iy, jz, foca, focb, ncmd, idur, ret;
  int ipan = -1, jpan = -1, numpan = -1, numbad = 0, ii, jj;
  int old_stat, status, mpan, mpan1, hh, mm, ss, YY, MM;
  long utime[5], utim1[5], utim2[5], mov0, mov1;

//polaris adjust: defocus all / focus individual / take pict / defocus and focus next

  jz = global_zenith + 100;         //in case we need the zenith angle  [+100 for offset in array]

  a_sb_shutt = SC_OPEN_SHUTTER;  //standard picture using shutter

  if (g_sbigon != 1) {
    sprintf(lstr, "pola: sbig camera not switched on ??? %d", g_sbigon);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }
  if (g_sbig_th != 0) {
    sprintf(lstr, "pola: sbig camera busy ??? %d", g_sbig_th);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }

  //deactivate interactive SBIG
  sbig_auto(-1);
  g_sb_first = 1;
  g_sbig_err = 0;

  AMCtime(utime);
  YY = utime[1] / 10000 + 2000;
  MM = (utime[1] / 100) % 100;
  hh =  utime[0] / 10000;
  mm = (utime[0] - 10000 * hh) / 100;
  ss =  utime[0] % 100;
  sprintf(lstr, "<%02d:%02d:%02d>", hh, mm, ss);

  fl_set_object_label(FL_durat, lstr);  //put start-time
  mpan1 = 0;

  //loop over all panels; mark them all 'TDO' for total defocus
  sprintf(lstr, "pola: defocus all");
  put_logfile(LOG_WRN, 0, lstr);
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (panel[i][j].pan_stat > STAT_NIN) {
        panel[i][j].pan_stat = STAT_TDO;
        mpan1++;
      }
  fl_set_button(FL_actDef, 1);   //set corresponding action-button
  action_cb(NULL, GUIcmd_DFOC);
  fl_set_button(FL_actDef, 0);   //deselect action-button

  //loop over all selected panels; mark them all 'ACT', loop over them
  mpan = 0;
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (p_but[i][j].select != 0) {
        panel[i][j].pan_stat = STAT_ACT;
        mpan++;
      }

  if (imod == 2) sprintf(lstr, "roqa: mpan= %d mpan1= %d ==============", mpan, mpan1);
  else         sprintf(lstr, "pola: mpan= %d mpan1= %d ==============", mpan, mpan1);
  put_logfile(LOG_WRN, 0, lstr);

  //now loop over each, flag as 'TDO' and execute
  //for (i = 0; i < 17; i++)
  //  for (j = 0; j < 17; j++)
  for (i = 16; i >= 0; i--)
    for (j = 16; j >= 0; j--)
      if (panel[i][j].pan_stat == STAT_ACT) {
        if (g_sbig_err > 0)
          goto badpan;
        numpan = 1;
        p_sele.i = ipan = i;
        p_sele.j = jpan = j;

        ii = i - 8;
        jj = j - 8;

        //first: focus
        panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0];
        panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1];

        mov0 = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
        mov1 = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
        panel[i][j].move_by[0] = mov0;
        panel[i][j].move_by[1] = mov1;
        if (AMC_exec_cmd2(panel, CMD_MOVE, numpan, ipan, jpan) != 0) goto badpan;
        sprintf(lstr, "pola: (%+2d,%+2d) posi_stat %d %d;    ask: %6d %6d | pc: %6d %6d | act: %6d %6d",
                ii, jj, panel[i][j].pan_stat, g_ret,
                panel[i][j].lut[0][jz][0], panel[i][j].lut[0][jz][1],
                panel[i][j].pc_mot[0], panel[i][j].pc_mot[1],
                panel[i][j].act_mot[0], panel[i][j].act_mot[1]);
        put_logfile(LOG_WRN, 0, lstr);

        //take picture;
        sprintf(act_pangrp, "(%+2d,%+2d)", ii, jj);

        a_sb_mode = S_PICT_TH; //expose and read picture
        AMCtime(utim1);
        utim1[2] = utim1[2] + (int)g_expos + 2;
        ret = pthread_create(&sbig_autoth[0], NULL, AutoPict_th, NULL);        //start expose and read
        //wait until picture exposed ....
        do {
          usleep(100000);
          AMC_check_gui(0, -2.);
          AMCtime(utim2);
        }
        while (utim2[2] < utim1[2]);

        ret = pthread_join(sbig_autoth[0], (void **)&status);                  //join read
        g_sb_first = 0;

        AMCtime(utime);
        YY = utime[1] / 10000 + 2000;
        MM = (utime[1] / 100) % 100;

        if (imod == 2) {
          sprintf(a_sb_name, "%s/%04d/%02d/M1_ROQ_%06d_%06d_Z%+03d_P%+02d%+02d_A%+05d_B%+05d",
                  sbig_path, YY, MM, utime[1], utime[0], jz - 100, ii, jj,
                  panel[i][j].pc_mot[0], panel[i][j].pc_mot[1]);
        }
        else {
          sprintf(a_sb_name, "%s/%04d/%02d/M1_POL_%06d_%06d_Z%+03d_P%+02d%+02d_A%+05d_B%+05d",
                  sbig_path, YY, MM, utime[1], utime[0], jz - 100, ii, jj,
                  panel[i][j].pc_mot[0], panel[i][j].pc_mot[1]);
        }

        if (g_sbig_err > 0) {
          panel[i][j].pan_stat = STAT_ERR;
          goto badpan;
        }

        pict_anal(imod);  //does nothing so far                          //analyse picture
        save_cb(NULL, -1);                                               //save picture
        g_newpic = 1;
        upd_pixmap();                                                    //show picture
        numbad = 0;
        old_stat = panel[i][j].pan_stat;
        goto goodpan;

badpan:
        old_stat = panel[i][j].pan_stat = STAT_ERR;
        sprintf(lstr, "pola(%2d,%2d): BADPAN %d", ii, jj, g_sbig_err);
        put_logfile(LOG_ERR, 0, lstr);
        if (g_sbig_err > 0)
          numbad++;  //check if there is a severe SBIG problem
        g_sbig_err = 0;


goodpan:   //move to defocused position in any case
        if (panel[i][j].lut[0][jz][0] > 0)
          panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0] - 1900;
        else
          panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0] + 1900;
        if (panel[i][j].lut[0][jz][1] > 0)
          panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1] - 1900;
        else
          panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1] + 1900;

        mov0 = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
        mov1 = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
        panel[i][j].move_by[0] = mov0;
        panel[i][j].move_by[1] = mov1;

        if (AMC_exec_cmd2(panel, CMD_MOVE, numpan, ipan, jpan) == 0)
          panel[i][j].pan_stat = old_stat;
        else {   //try again to defocus ...
          mov0 = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
          mov1 = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
          panel[i][j].move_by[0] = mov0;
          panel[i][j].move_by[1] = mov1;
          if (AMC_exec_cmd2(panel, CMD_MOVE, numpan, ipan, jpan) == 0)
            panel[i][j].pan_stat = old_stat;
        }

        panel[i][j].pan_tmpstat = panel[i][j].pan_stat;
        if (numbad > 1 || global_break > 0) {
          sprintf(lstr, "goto ENDPOLA 2 (too many bad)");
          put_logfile(LOG_ERR, 0, lstr);
          goto endpola;   //2 consecutive SBIG problems==>stop
        }
      }

endpola:

  /*
  //we are done with all selected panels .....
  //loop over all panels; mark them all 'TDO' for total focus/adjust
  sprintf(lstr, "pola: focus all back");
  put_logfile(LOG_WRN, 0, lstr);
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (panel[i][j].pan_stat > STAT_NIN)
        panel[i][j].pan_stat = STAT_TDO;
  fl_set_button(FL_actAdj, 1);   //set corresponding action-button
  action_cb(NULL, GUIcmd_ADJS);
  fl_set_button(FL_actAdj, 0);   //deselect action-button
  */

  //loop over all selected panels and set status to POLA-status
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (p_but[i][j].select != 0)
        panel[i][j].pan_stat = panel[i][j].pan_tmpstat;

  //activate interactive SBIG
  sprintf(lstr, "...... reactivate manual SBIG ---- ");
  put_logfile(LOG_DB9, 0, lstr);
  sprintf(act_pangrp, "UNDEF");
  sbig_auto(+1);
}

//--------------------------------------------------------------------

void action_star(int imod)
{
  // group pictures

  int k, i, j, it, jt, istat, ix, iy, jz, foca, focb, ncmd, idur, ret, p;
  int ipan = -1, jpan = -1, numpan = -1, numbad = 0, ii, jj, idefoc, idefoc0;
  int old_stat, status, mpan, mpan1, hh, mm, ss, YY, MM, jmod, jmax;
  long utime[5], utim1[5], utim2[5], mov0, mov1;

  //star adjust: defocus all but group / take pict / next group

  jz  = global_zenith + 100;         //in case we need the zenith angle  [+100 for offset in array]

  a_sb_shutt = SC_OPEN_SHUTTER;  //standard picture using shutter

  if (g_sbigon != 1) {
    sprintf(lstr, "star: sbig camera not switched on ??? %d", g_sbigon);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }
  if (g_sbig_th != 0) {
    sprintf(lstr, "star: sbig camera busy ??? %d", g_sbig_th);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }

  //deactivate interactive SBIG
  sbig_auto(-1);
  g_sb_first = 1;
  g_sbig_err = 0;

  AMCtime(utime);
  YY = utime[1] / 10000 + 2000;
  MM = (utime[1] / 100) % 100;
  hh =  utime[0] / 10000;
  mm = (utime[0] - 10000 * hh) / 100;
  ss =  utime[0] % 100;
  sprintf(lstr, "<%02d:%02d:%02d>", hh, mm, ss);

  fl_set_object_label(FL_durat, lstr);  //put start-time
  mpan1 = 0;

  idefoc0 = 330;

  for (p = 0; p < 9; p++) { //loop over all panel-groups;
    // group '0' does not exist ==> all defocus

    jmax = 0;

    for (jmod = 0; jmod <= jmax; jmod++) {

      for (idefoc = idefoc0; idefoc < 360; idefoc += 330) {

        jz = global_zenith + 100;

        //now loop over each, flag as 'TDO' and execute
        for (i = 0; i < 17; i++)
          for (j = 0; j < 17; j++)
            if (p_but[i][j].select != 0)
              panel[i][j].pan_stat = STAT_TDO;

        shiftfoc(idefoc, global_xoff, global_yoff, 0);

        mpan = 0;
        for (i = 0; i < 17; i++) {
          for (j = 0; j < 17; j++) {
            if (p_but[i][j].select != 0) {
              if (jmod == 1) { // rotate by 90deg
                panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0] + panel[i][j].mov_dab[1]; //+delta_Y
                panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1] - panel[i][j].mov_dab[0]; //-delta_X
              }
              else { // default
                panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0] + panel[i][j].mov_dab[0]; //+delta_X
                panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1] + panel[i][j].mov_dab[1]; //+delta_Y
              }

              if (panel[i][j].pan_grp != p) {
                //for those not in the correct panel group, move mot[0] far away
                //but mot[1] to correct position ==> need only to move mot[0] further on
                if (panel[i][j].lut[0][jz][0] > 0) {
                  panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0] - 1900;
                  panel[i][j].mov_dab[0]   = -1900;
                  panel[i][j].mov_dxy[0]   = -999.;
                }
                else {
                  panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0] + 1900;
                  panel[i][j].mov_dab[0]   = +1900;
                  panel[i][j].mov_dxy[0]   = +999.;
                }
                if (panel[i][j].lut[0][jz][1] > 0) {
                  panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1] - 1900;
                  panel[i][j].mov_dab[1]   = -1900;
                  panel[i][j].mov_dxy[1]   = -999.;
                }
                else {
                  panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1] + 1900;
                  panel[i][j].mov_dab[1]   = +1900;
                  panel[i][j].mov_dxy[1]   = +999.;
                }
              }

              mov0 = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
              mov1 = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
              panel[i][j].move_by[0] = mov0;
              panel[i][j].move_by[1] = mov1;
              if (abs(mov0) > 4 || abs(mov1) > 4) {
                panel[i][j].pan_stat = STAT_TDO;
                mpan++;
              }
              else
                panel[i][j].pan_stat = STAT_ACT;
            }
          }
        }
        if (mpan > 0)
          action_cb(NULL, GUIcmd_MVTO);

        if (global_break > 0)
          goto end_star;

        mpan = 0; //do second round in case some panels not yet correct
        for (i = 0; i < 17; i++) {
          for (j = 0; j < 17; j++) {
            if (p_but[i][j].select != 0) {
              mov0 = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
              mov1 = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
              panel[i][j].move_by[0] = mov0;
              panel[i][j].move_by[1] = mov1;
              if (abs(mov0) > 4 || abs(mov1) > 4) {
                panel[i][j].pan_stat = STAT_TDO;
                mpan++;
              }
              else
                panel[i][j].pan_stat = STAT_ACT;
            }
          }
        }
        if (mpan > 0)
          action_cb(NULL, GUIcmd_MVTO);

        if (global_break > 0)
          goto end_star;

        mpan = 0; //do 3rd round in case some panels not yet correct
        for (i = 0; i < 17; i++) {
          for (j = 0; j < 17; j++) {
            if (p_but[i][j].select != 0) {
              mov0 = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
              mov1 = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
              panel[i][j].move_by[0] = mov0;
              panel[i][j].move_by[1] = mov1;
              if (abs(mov0) > 4 || abs(mov1) > 4) {
                panel[i][j].pan_stat = STAT_TDO;
                mpan++;
              }
              else
                panel[i][j].pan_stat = STAT_ACT;
            }
          }
        }
        if (mpan > 0)
          action_cb(NULL, GUIcmd_MVTO);

        if (global_break > 0)
          goto end_star;

        AMCtime(utime);
        YY = utime[1] / 10000 + 2000;
        MM = (utime[1] / 100) % 100;

        sprintf(a_sb_name, "%s/%04d/%02d/M1_STR_%06d_%06d_Z%+03d_PGR%02d_DF%04d_M%d.txt",
                sbig_path, YY, MM, utime[1], utime[0], jz - 100, p, idefoc, jmod);
        f_param = fopen(a_sb_name, "w");
        for (i = 0; i < 17; i++) {
          for (j = 0; j < 17; j++) {
            if (p_but[i][j].select != 0) {
              sprintf(lstr, "(%2d,%2d) act: %5d %5d; req: %5d %5d; X/Y: %6.1f %6.1f %5d %5d; lut: %5d %5d",
                      i - 8, j - 8,
                      panel[i][j].act_mot[0],    panel[i][j].act_mot[1],
                      panel[i][j].pc_moveto[0],  panel[i][j].pc_moveto[1],
                      panel[i][j].mov_dxy[0],    panel[i][j].mov_dxy[1],
                      panel[i][j].mov_dab[0],    panel[i][j].mov_dab[1],
                      panel[i][j].lut[0][jz][0], panel[i][j].lut[0][jz][1]);
              fprintf(f_param, "%s\n", lstr);
              if (panel[i][j].pan_grp == p)
                panel[i][j].pan_stat = STAT_OK_;
            }
          }
        }
        fprintf(f_param, "# shiftfoc: F=%5d, X=%8.1f, Y=%8.1f\n", idefoc, global_xoff, global_yoff);
        fclose(f_param);

        sprintf(a_sb_name, "%s/%04d/%02d/M1_STR_%06d_%06d_Z%+03d_PGR%02d_DF%04d_M%d",
                sbig_path, YY, MM, utime[1], utime[0], jz - 100, p, idefoc, jmod);

        a_sb_mode = S_PICT_TH; //expose and read picture
        AMCtime(utim1);
        utim1[2] = utim1[2] + (int)g_expos + 2;
        ret = pthread_create(&sbig_autoth[0], NULL, AutoPict_th, NULL);        //start expose and read
        //wait until picture exposed ....
        do {
          usleep(200000);
          AMC_check_gui(0, -2.);
          AMCtime(utim2);
        }
        while (utim2[2] < utim1[2]);

        ret = pthread_join(sbig_autoth[0], (void **)&status);                  //join read

        if (global_break > 0)
          goto end_star;

        if (g_sbig_err != 0) {
          //something went wrong with SBIG ==> ERROR !!!!
          sprintf(lstr, "SBIG ERROR: power-cycle SBIG and redo action ");
          put_logfile(LOG_WRN, 0, lstr);
          for (i = 0; i < 17; i++)
            for (j = 0; j < 17; j++)
              if (p_but[i][j].select != 0)
                panel[i][j].pan_stat = STAT_ERR;
          goto end_star;
        }

        g_sb_first = 0;

        pict_anal(imod);  //does nothing so far                          //analyse picture
        save_cb(NULL, -1);                                               //save picture
        g_newpic = 1;
        upd_pixmap();                                                    //show picture
        numbad = 0;

      } //end of defocus loop
    } //end of jmod loop
  } //end of this panel_group

  //adjust all panels
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (p_but[i][j].select != 0)
        panel[i][j].pan_stat = STAT_TDO;
  fl_set_button(FL_actAdj, 1); //set corresponding action-button
  action_cb(NULL, GUIcmd_ADJS);
  fl_set_button(FL_actAdj, 0); //deselect action-button

end_star:
  //activate interactive SBIG
  sprintf(lstr, "...... reactivate manual SBIG ---- ");
  put_logfile(LOG_DB9, 0, lstr);
  sprintf(act_pangrp, "UNDEF");
  sbig_auto(+1);
}


//--------------------------------------------------------------------
void action_cali(int imod)
{
  int k, i, j, ii, jj, ix, iy, jz, ret;
  int knull = 0, kmax = 11;
  int ipan = -1, jpan = -1, mpan = 0, numpan = -1, numbad = 0, ierr = 0, jerr = 0;
  int old_stat, status, hh, mm, ss, YY, MM;
  int calstep[2][12];
  long utime[5];

  jz = global_zenith + 100;         //in case we need the zenith angle

  int loc_foc = global_foclen;
  shiftfoc(loc_foc, global_xoff, global_yoff, 0);

  a_sb_shutt = SC_OPEN_SHUTTER;  //standard picture using shutter

  AMCtime(utime);
  YY = utime[1] / 10000 + 2000;
  MM = (utime[1] / 100) % 100;
  hh =  utime[0] / 10000;
  mm = (utime[0] - 10000 * hh) / 100;
  ss =  utime[0] % 100;
  sprintf(lstr, "<%02d:%02d:%02d>", hh, mm, ss);

  fl_set_object_label(FL_durat, lstr);  //put start-time

  if (g_sbigon != 1) {
    sprintf(lstr, "cali: sbig camera not switched on ??? %d", g_sbigon);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }
  if (g_sbig_th != 0) {
    sprintf(lstr, "cali: sbig camera busy ??? %d", g_sbig_th);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }
  //deactivate interactive SBIG
  sbig_auto(-1);
  g_sb_first = 1;
  g_sbig_err = 0;

  //loop over all selected panels; mark them as 'ACT'
  mpan = 0;
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (p_but[i][j].select != 0) {
        panel[i][j].pan_stat = STAT_ACT;
        mpan++;
      }

  sprintf(lstr, "cali: mpan= %d ===============", mpan);
  put_logfile(LOG_WRN, 0, lstr);

  //now loop over each, flag as 'TDO' and execute
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (panel[i][j].pan_stat == STAT_ACT) {

        ix = panel[i][j].lut[0][jz][0];
        iy = panel[i][j].lut[0][jz][1];

        // focus
        calstep[0][0]  =    ix;  calstep[1][0]  =    iy;
        calstep[0][1]  =     0;  calstep[1][1]  =     0;

        // calibrate
        calstep[0][2]  =     0;  calstep[1][2]  =  1000;
        calstep[0][3]  =     0;  calstep[1][3]  =   500;
        calstep[0][4]  =     0;  calstep[1][4]  =  -500;
        calstep[0][5]  =     0;  calstep[1][5]  = -1000;

        calstep[0][6]  =  1000;  calstep[1][6]  =     0;
        calstep[0][7]  =   500;  calstep[1][7]  =     0;
        calstep[0][8]  =  -500;  calstep[1][8]  =     0;
        calstep[0][9]  = -1000;  calstep[1][9]  =     0;

        // defocus
        if (ix < 0)
          calstep[0][10] = ix + 1900;
        else
          calstep[0][10] = ix - 1900;
        if (iy < 0)
          calstep[1][10] = iy + 1900;
        else
          calstep[1][10] = iy - 1900;

        /*
        calstep[0][12] =   500;   calstep[1][12] =  1000;
        calstep[0][13] =   500;   calstep[1][13] =   500;
        calstep[0][14] =   500;   calstep[1][14] =  -500;
        calstep[0][15] =   500;   calstep[1][15] = -1000;

        calstep[0][16] =  1000;   calstep[1][16] =   500;
        calstep[0][17] =   500;   calstep[1][17] =   500;
        calstep[0][18] =  -500;   calstep[1][18] =   500;
        calstep[0][19] = -1000;   calstep[1][19] =   500;

        if (ix < 0)
          calstep[0][20] = ix + 1900;
        else
          calstep[0][20] = ix - 1900;
        if (iy < 0)
          calstep[1][20] = iy + 1900;
        else
          calstep[1][20] = iy - 1900;
        */

        numpan = 1;
        ipan = i;
        jpan = j;
        ii = i - 8;
        jj = j - 8;

        if (g_sbig_err != 0)
          goto badpan;  //do nothing if SBIG has problem

        //first: laser on and center
        p_sele.i = ipan;
        p_sele.j = jpan;

        // do center only if needed (i.e. panel position looks fichy...)
        if (panel[i][j].pan_stat == STAT_NIN ||
            abs (panel[i][j].pc_mot[0] - panel[i][j].act_mot[0]) > 2 ||
            abs (panel[i][j].pc_mot[1] - panel[i][j].act_mot[1]) > 2) {
          if (AMC_exec_cmd2(panel, CMD_CNTR, numpan, ipan, jpan) != 0)
            goto badpan;
          sprintf(lstr, "cali(%2d,%2d): cntr_stat %d %d", ii, jj, panel[i][j].pan_stat, g_ret);
          put_logfile(LOG_WRN, 0, lstr);
        }

        for (k = knull; k < kmax; k++) {

          ix = panel[i][j].lut[0][jz][0];
          iy = panel[i][j].lut[0][jz][1];

          // focus and zero
          calstep[0][0]  =    ix;  calstep[1][0]  =    iy;
          calstep[0][1]  =     0;  calstep[1][1]  =     0;

          // calibrate
          calstep[0][2]  =     0;  calstep[1][2]  =  1000;
          calstep[0][3]  =     0;  calstep[1][3]  =   500;
          calstep[0][4]  =     0;  calstep[1][4]  =  -500;
          calstep[0][5]  =     0;  calstep[1][5]  = -1000;

          calstep[0][6]  =  1000;  calstep[1][6]  =     0;
          calstep[0][7]  =   500;  calstep[1][7]  =     0;
          calstep[0][8]  =  -500;  calstep[1][8]  =     0;
          calstep[0][9]  = -1000;  calstep[1][9]  =     0;

          // defocus
          if (ix < 0)
            calstep[0][10] = ix + 1900;
          else
            calstep[0][10] = ix - 1900;
          if (iy < 0)
            calstep[1][10] = iy + 1900;
          else
            calstep[1][10] = iy - 1900;

          //move panel to new position
          panel[i][j].pc_moveto[0] = calstep[0][k];
          panel[i][j].pc_moveto[1] = calstep[1][k];
          panel[i][j].move_by[0] = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
          panel[i][j].move_by[1] = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
          sprintf(lstr, "cali(%+2d,%+2d): %d. postition: %6d %6d", ii, jj, k + 1, calstep[0][k], calstep[1][k]);
          put_logfile(LOG_WRN, 0, lstr);

          if (AMC_exec_cmd2(panel, CMD_MOVE, numpan, ipan, jpan) != 0)
            if (AMC_exec_cmd2(panel, CMD_MOVE, numpan, ipan, jpan) != 0)
              goto badpan;

          sprintf(lstr, "cali(%+2d,%+2d): posi_stat %d %d;    ask: %6d %6d | pc: %6d %6d | act: %6d %6d",
                  ii, jj, panel[i][j].pan_stat, g_ret,
                  calstep[0][k], calstep[1][k],
                  panel[i][j].pc_mot[0], panel[i][j].pc_mot[1],
                  panel[i][j].act_mot[0], panel[i][j].act_mot[1]);
          put_logfile(LOG_WRN, 0, lstr);
          old_stat = panel[i][j].pan_stat;

          if (k > knull) { //finish read out of previous picture (should be breakable)
            panel[i][j].pan_stat = STAT_SBG;
            AMC_check_gui(0, -2.);      //check GUI and CC
            ret = pthread_join(sbig_autoth[0], (void **)&status);            //join read
            if (g_sbig_err != 0) {
              old_stat = panel[i][j].pan_stat = STAT_ERR;
              sprintf(lstr, "SBIG error %d (%+2d,%+2d) %d", g_sbig_err, i - 8, j - 8, panel[i][j].pan_stat);
              put_logfile(LOG_WRN, 0, lstr);
              goto badpan;
            }
            pict_anal(0);                                                    //analyse picture
            save_cb(NULL, -1);                                               //save picture
            g_newpic = 1;
            upd_pixmap();                                                    //show picture
          }

          sprintf(act_pangrp, "(%+2d,%+2d)", ii, jj);
          a_sb_mode = S_EXPO_TH;
          ret = pthread_create(&sbig_autoth[0], NULL, AutoPict_th, NULL);    //start expose
          AMCtime(utime);
          YY = utime[1] / 10000 + 2000;
          MM = (utime[1] / 100) % 100;

          ix = panel[i][j].lut[0][jz][0];
          iy = panel[i][j].lut[0][jz][1];

          // focus and zero
          calstep[0][0]  =    ix;  calstep[1][0]  =    iy;
          calstep[0][1]  =     0;  calstep[1][1]  =     0;

          // calibrate
          calstep[0][2]  =     0;  calstep[1][2]  =  1000;
          calstep[0][3]  =     0;  calstep[1][3]  =   500;
          calstep[0][4]  =     0;  calstep[1][4]  =  -500;
          calstep[0][5]  =     0;  calstep[1][5]  = -1000;

          calstep[0][6]  =  1000;  calstep[1][6]  =     0;
          calstep[0][7]  =   500;  calstep[1][7]  =     0;
          calstep[0][8]  =  -500;  calstep[1][8]  =     0;
          calstep[0][9]  = -1000;  calstep[1][9]  =     0;

          // defocus
          if (ix < 0)
            calstep[0][10] = ix + 1900;
          else
            calstep[0][10] = ix - 1900;
          if (iy < 0)
            calstep[1][10] = iy + 1900;
          else
            calstep[1][10] = iy - 1900;

          if (k == 0)
            sprintf(a_sb_name, "%s/%04d/%02d/M1_CAL_%06d_%06d_Z%+03d_P%+02d%+02d_A%+05d_B%+05d_focus",
                    sbig_path, YY, MM, utime[1], utime[0], jz - 100, i - 8, j - 8, calstep[0][k], calstep[1][k]);
          else if (k == 10)
            sprintf(a_sb_name, "%s/%04d/%02d/M1_CAL_%06d_%06d_Z%+03d_P%+02d%+02d_A%+05d_B%+05d_defocus",
                    sbig_path, YY, MM, utime[1], utime[0], jz - 100, i - 8, j - 8, calstep[0][k], calstep[1][k]);
          else
            sprintf(a_sb_name, "%s/%04d/%02d/M1_CAL_%06d_%06d_Z%+03d_P%+02d%+02d_A%+05d_B%+05d",
                    sbig_path, YY, MM, utime[1], utime[0], jz - 100, i - 8, j - 8, calstep[0][k], calstep[1][k]);
          ret = pthread_join(sbig_autoth[0], (void **)&status);               //join expose

          // start readout of picture
          a_sb_mode = S_READ_TH; //read already exposed picture
          ret = pthread_create(&sbig_autoth[0], NULL, AutoPict_th, NULL);     //start read
          g_sb_first = 0;  //we are no longer first picture in thread
        }

        if (g_sbig_err == 0) {
          //read out last picture
          panel[i][j].pan_stat = STAT_SBG;
          AMC_check_gui(0, -2.);      //check GUI and CC
          ret = pthread_join(sbig_autoth[0], (void **)&status);               //join expose
          if (g_sbig_err == 0) {
            pict_anal(0);                                                     //analyse picture
            save_cb(NULL, -1);                                                //save picture
            g_newpic = 1;
            upd_pixmap();                                                     //show picture
            panel[i][j].pan_stat = old_stat;
          }
          else {
            panel[i][j].pan_stat = STAT_ERR;
            goto badpan;
          }
        }

        sprintf(lstr, "cali(%2d,%2d): ok", ii, jj);
        put_logfile(LOG_WRN, 0, lstr);
        numbad = 0;    //not consecutive problems ==> try to continue
        goto goodpan;

badpan:
        sprintf(lstr, "cali(%2d,%2d): BADPAN %d", ii, jj, g_sbig_err);
        put_logfile(LOG_ERR, 0, lstr);
        if (g_sbig_err > 0)
          numbad++;  //check if there is a severe SBIG problem
        g_sbig_err = 0;

goodpan:
        old_stat = panel[i][j].pan_stat;
        if (numbad > 1 || global_break > 0)
          goto endcali; //2 consecutive SBIG problems==>stop
      }

  //we are done with all selected panels .....

endcali:
  //activate interactive SBIG
  sprintf(lstr, "...... reactivate manual SBIG ---- ");
  put_logfile(LOG_DB9, 0, lstr);
  sprintf(act_pangrp, "UNDEF");
  sbig_auto(+1);

}

//--------------------------------------------------------------------

void action_lasi(int imod)   //take laser pictures indep. of actuators
{
  int k, i, j, it, jt, istat, ix, iy, jz, foca, focb, ncmd, idur, ret;
  int ipan = -1, jpan = -1, numpan = -1, ierr = 0, jerr = 0, ii, jj;
  int old_stat, status, mpan, hh, mm, ss, YY, MM, numbad = 0;
  int knull, kmax, rdact, oldstat;
  long utime[5];

  jz  = global_zenith + 100;         //in case we need the zenith angle
  mpan = 0;

  a_sb_shutt = SC_OPEN_SHUTTER;  //standard picture using shutter

  AMCtime(utime);
  YY = utime[1] / 10000 + 2000;
  MM = (utime[1] / 100) % 100;
  hh =  utime[0] / 10000;
  mm = (utime[0] - 10000 * hh) / 100;
  ss =  utime[0] % 100;
  sprintf(lstr, "<%02d:%02d:%02d>", hh, mm, ss);

  fl_set_object_label(FL_durat, lstr);  //put start-time

  if (g_sbigon != 1) {
    sprintf(lstr, "lasi: sbig camera not switched on ??? %d", g_sbigon);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }
  if (g_sbig_th != 0) {
    sprintf(lstr, "cali: sbig camera busy ??? %d", g_sbig_th);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }
  //deactivate interactive SBIG
  sbig_auto(-1);
  g_sb_first = 1;
  g_sbig_err = 0;

  //loop over all selected panels; mark them as 'ACT'
  mpan = 0;
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (p_but[i][j].select != 0) {
        panel[i][j].pan_stat = STAT_ACT;
        mpan++;
      }

  sprintf(lstr, "lasi: mpan= %d ===============", mpan);
  put_logfile(LOG_WRN, 0, lstr);

  //now loop over each, flag as 'TDO' and execute
  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (panel[i][j].pan_stat == STAT_ACT) {

        numpan = 1;
        p_sele.i = ipan = i;
        p_sele.j = jpan = j;
        ii = i - 8;
        jj = j - 8;

        //laser on
        if (AMC_exec_cmd2(panel, CMD_LSON, numpan, ipan, jpan) != 0) 
          goto badpan;
        sprintf(lstr, "lasi: %+2d %+2d lson_stat %d %d %d", ii, jj, panel[i][j].pan_stat, panel[i][j].laser, g_ret);
        put_logfile(LOG_WRN, 0, lstr);

        //take picture
        a_sb_mode = S_EXPO_TH;
        sprintf(act_pangrp, "(%+2d,%+2d)", ii, jj);
        ret = pthread_create(&sbig_autoth[0], NULL, AutoPict_th, NULL);     //start expose
        //wait until done
        ret = pthread_join(sbig_autoth[0], (void **)&status);               //join expose
        //start readout
        rdact = 1;
        a_sb_mode = S_READ_TH; //read already exposed picture
        ret = pthread_create(&sbig_autoth[0], NULL, AutoPict_th, NULL);     //start read
        g_sb_first = 0;  //we are no longer first picture in thread

        AMCtime(utime);
        YY = utime[1] / 10000 + 2000;
        MM = (utime[1] / 100) % 100;
        sprintf(a_sb_name, "%s/%04d/%02d/M1_LAS_%06d_%06d_Z%+03d_P%+02d%+02d_A%+05d_B%+05d_Z%+03d",
                sbig_path, YY, MM, utime[1], utime[0], jz - 100, ii, jj,
                panel[i][j].pc_mot[0], panel[i][j].pc_mot[1], jz - 100);

        //laser off
        AMC_exec_cmd2(panel, CMD_LOFF, numpan, ipan, jpan);
        sprintf(lstr, "lasi: %+2d %+2d loff_stat %d %d %d", ii, jj, panel[i][j].pan_stat, panel[i][j].laser, g_ret);
        put_logfile(LOG_WRN, 0, lstr);

        //finish read out
        rdact = 0;
        ret = pthread_join(sbig_autoth[0], (void **)&status);            //join read
        if (g_sbig_err != 0) {
          old_stat = panel[i][j].pan_stat = STAT_ERR;
          sprintf(lstr, "SBIG error %d (%+2d,%+2d) %d", g_sbig_err, i - 8, j - 8, panel[i][j].pan_stat);
          put_logfile(LOG_WRN, 0, lstr);
          goto badpan;
        }
        pict_anal(0);                                                   //analyse picture
        save_cb(NULL, -1);                                               //save picture
        g_newpic = 1;
        upd_pixmap();                                                    //show picture
        numbad = 0;
        goto goodpan;

badpan:
        if (g_sbig_err > 0)
          numbad++;  //check if there is a severe SBIG problem
        sprintf(lstr, "lasi: %2d %2d BADPAN%d %d %d", ii, jj, panel[i][j].pan_stat, g_sbig_err, numbad);
        put_logfile(LOG_ERR, 0, lstr);

goodpan:   //make sure laser is off !!!!  and readout thread terminated
        oldstat = panel[i][j].pan_stat;
        AMC_exec_cmd2(panel, CMD_LOFF, numpan, ipan, jpan);
        if (rdact != 0) {
          rdact = 0;
          ret = pthread_join(sbig_autoth[0], (void **)&status);
        }
        panel[i][j].pan_stat = oldstat;
        g_sbig_err = 0;
        if (numbad > 1 || global_break > 0) goto endlasi; //2 consecutive SBIG problems==>stop
      }


endlasi:
  //activate interactive SBIG
  sprintf(lstr, "...... reactivate manual SBIG ---- ");
  put_logfile(LOG_DB9, 0, lstr);
  sprintf(act_pangrp, "UNDEF");
  sbig_auto(+1);

}

//--------------------------------------------------------------------

void action_none(int imod)   //
{
  char lstr[LOGLEN];

  sprintf(lstr, "command %d not yet implemented", imod);
  put_logfile(LOG_WRN, 0, lstr);
}

//--------------------------------------------------------------------

void timeout_yesno(int id, void *data)
{
  fl_hide_question();
}

//--------------------------------------------------------------------

void action_cb(FL_OBJECT *ob, long cmd)
{

  if (cmd == GUIcmd_CNTR) {
    fl_add_timeout(5000, timeout_yesno, 0);
    int ret = fl_show_question("Do you really want to center?\nThe procedure can take up to 2 minutes.\n(you have 5 seconds to press NO)", 1);
    if (ret != 1) {
      fl_set_button(ob, 0);      //reset action-button
      return;
    }
  }

  int k, i, j, it, jt, istat, ix, iy, jz, foca, focb, ncmd, idur, npan;
  int ipan = -1, jpan = -1, numpan = -1, exc_cmd, idum;
  int tmp_pan[17][17] = {289 * 0};
  int tmp_mov[17][17] = {289 * 0};
  int moveflg;
  unsigned long isec, isec0, iusec, iusec0;
  char str[10], xstr[10];
  char lstr[LOGLEN];
  long jcmd;
  double x, y, dd, xcmd, xdur;
  int nok0 = 0, nerr0 = 0, nbrk0 = 0, nlas0 = 0, nudf0 = 0, nmov0 = 0, ncom0 = 0;
  int nokx = 0, nerr1 = 0, nbrk1 = 0, nlas1 = 0, nudf1 = 0, nerrx = 0, ncom1 = 0;
  long utime[5];

  it = 0;
  moveflg = -1;
  if (ob != NULL && global_action > 0) return;  //this should never happen, but ????

  if (cmd == GUIcmd_LSAD || cmd == GUIcmd_CALI || cmd == GUIcmd_LSON)  //flag laser on if needed...
    if (g_AMCstat != 5) {
      g_AMCstat0 = g_AMCstat;
      g_AMCstat = 5;
    }

  if (ob != NULL) {
    global_action = 1;

    if (global_mode != MODE_MANU) {            //shall we get info from CC-rep ?
      fl_set_button(ob, 1);                   //set corresponding action-button

      if (cmd == GUIcmd_ADJS || cmd == GUIcmd_LSAD || cmd == GUIcmd_CALI || cmd == GUIcmd_NEW1 ||
          cmd == GUIcmd_RQAD || cmd == GUIcmd_PLAD || cmd == GUIcmd_STAD) {
        sprintf(xstr, "%d", global_zenith);
        fl_set_input(FLset_Zd, xstr);
        sprintf(xstr, "%d", global_azimut);
        fl_set_input(FLset_Az, xstr);
        sprintf(xstr, "%d", global_foclen);
        fl_set_input(FLset_Foc, xstr);
        sprintf(xstr, "%d", global_xoff);
        fl_set_input(FLset_X, xstr);
        sprintf(xstr, "%d", global_yoff);
        fl_set_input(FLset_Y, xstr);
      }
    }
    fl_gettime(&isec0, &iusec0);
    if (cmd == GUIcmd_ADJS || cmd == GUIcmd_LSAD || GUIcmd_NEW1) {
      g_zd0 = global_zenith;
      g_az0 = global_azimut;
      g_fc0 = global_foclen;
      g_tim0x = isec0;
    }

    fl_show_object(FLbreak);
    fl_deactivate_object(FLselect1_grp);
    fl_deactivate_object(FLselect2_grp);
    fl_deactivate_object(FLexit);
    fl_set_object_lcol(FLselect1_grp, 901);
    fl_set_object_lcol(FLselect2_grp, 901);
    fl_set_object_lcol(FLexit, 901);
    fl_set_cursor(FL_ObjWin(FLexit), FL_BUSY_CURSOR);
    mode_cb(NULL, -9);  //deactivate all action-buttons

    if      (cmd == GUIcmd_NONE) sprintf(lstr, "------> action %2d=NONE started", cmd);
    else if (cmd == GUIcmd_TEST) sprintf(lstr, "------> action %2d=TEST started", cmd);
    else if (cmd == GUIcmd_INIT) sprintf(lstr, "------> action %2d=INIT started", cmd);
    else if (cmd == GUIcmd_ADJS) sprintf(lstr, "------> action %2d=ADJS started", cmd);
    else if (cmd == GUIcmd_TPNT) sprintf(lstr, "------> action %2d=TsAd started", cmd);
    else if (cmd == GUIcmd_LSON) sprintf(lstr, "------> action %2d=LSON started", cmd);
    else if (cmd == GUIcmd_LOFF) sprintf(lstr, "------> action %2d=LOFF started", cmd);
    else if (cmd == GUIcmd_LSAD) sprintf(lstr, "------> action %2d=LSAD started", cmd);
    else if (cmd == GUIcmd_CNTR) sprintf(lstr, "------> action %2d=CNTR started", cmd);
    else if (cmd == GUIcmd_CNT2) sprintf(lstr, "------> action %2d=CNT2 started", cmd);
    else if (cmd == GUIcmd_DFOC) sprintf(lstr, "------> action %2d=DFOC started", cmd);
    else if (cmd == GUIcmd_RAND) sprintf(lstr, "------> action %2d=RAND started", cmd);
    else if (cmd == GUIcmd_MVTO) sprintf(lstr, "------> action %2d=MVTO started", cmd);
    else if (cmd == GUIcmd_MOVE) sprintf(lstr, "------> action %2d=MOVE started", cmd);
    else if (cmd == GUIcmd_MOVA) sprintf(lstr, "------> action %2d=MOVA started", cmd);
    else if (cmd == GUIcmd_MOVB) sprintf(lstr, "------> action %2d=MOVB started", cmd);
    else if (cmd == GUIcmd_MVMI) sprintf(lstr, "------> action %2d=MVMI started", cmd);
    else if (cmd == GUIcmd_MVMA) sprintf(lstr, "------> action %2d=MVMA started", cmd);
    else if (cmd == GUIcmd_CALI) sprintf(lstr, "------> action %2d=CALI started", cmd);
    else if (cmd == GUIcmd_RQAD) sprintf(lstr, "------> action %2d=RQAD started", cmd);
    else if (cmd == GUIcmd_PLAD) sprintf(lstr, "------> action %2d=PLAD started", cmd);
    else if (cmd == GUIcmd_STAD) sprintf(lstr, "------> action %2d=STAD started", cmd);
    else if (cmd == GUIcmd_ACAD) sprintf(lstr, "------> action %2d=ACAD started", cmd);
    else if (cmd == GUIcmd_INIF) sprintf(lstr, "------> action %2d=INIF started", cmd);
    else if (cmd == GUIcmd_INFF) sprintf(lstr, "------> action %2d=INFF started", cmd);
    else if (cmd == GUIcmd_ZERO) sprintf(lstr, "------> action %2d=ZERO started", cmd);
    else if (cmd == GUIcmd_NEW1) sprintf(lstr, "------> action %2d=TADJ started", cmd);
    else                           sprintf(lstr, "------> action %2d=???? started", cmd);

    put_logfile(LOG_OK_, 0, lstr);  //log only if not called from macro
  }

  npan = 0;
  jcmd = cmd;
  numpan = 0;
  jz  = global_zenith + 100;         //in case we need the zenith angle  [+100 for offset in array]

  if      (cmd == GUIcmd_CALI) {
    action_cali(0);  //these are macro commands
  }
  else if (cmd == GUIcmd_PLAD) {
    action_pola(1);
  }
  else if (cmd == GUIcmd_RQAD) {
    action_pola(2);
  }
  else if (cmd == GUIcmd_LSAD) {
    action_lasi(3);
  }
  else if (cmd == GUIcmd_STAD) {
    action_star(4);
  }
  else {
    //start of direct actions

    if      (cmd == GUIcmd_NONE) {
      ncmd = 0;
      exc_cmd = CMD_NONE;
    }
    else if (cmd == GUIcmd_MOVA || cmd == GUIcmd_MOVB) {       //move only one panel
      moveflg = 1;
      ncmd   = 1;
      numpan = 1;
      ipan = i = p_sele.i;
      jpan = j = p_sele.j;

      x = fl_get_counter_value(FLmova);
      y = fl_get_counter_value(FLmovb);
      panel[i][j].pc_moveto[0] = x;
      panel[i][j].pc_moveto[1] = y;
      panel[i][j].pan_stat = STAT_TDO;

      tmp_pan[i][j] = 1;

    }
    else if (cmd == GUIcmd_TEST ||    //these commands can all be executed
             cmd == GUIcmd_INIT ||    //  independent of current state of
             cmd == GUIcmd_INIF ||    //  the panel
             cmd == GUIcmd_INFF ||    //  the panel
             cmd == GUIcmd_LSON ||    //
             cmd == GUIcmd_LOFF ||    //  result of the command will be
             cmd == GUIcmd_CNTR ||    //  final status of the panel
             cmd == GUIcmd_CNT2 ||    //
             cmd == GUIcmd_ACAD) {

      if (cmd == GUIcmd_INIT || cmd == GUIcmd_INIF || cmd == GUIcmd_CNTR || cmd == GUIcmd_CNT2)
        moveflg = 0;
      else
        moveflg = -1;

      ncmd   = 0;
      for (i = 0; i < 17; i++)         //flag all 'selected' panels as 'todo' and prepare action if needed
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0) panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set

          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            tmp_pan[i][j] = 1;
            tmp_mov[i][j] = 2;
            if (cmd == GUIcmd_CNTR)
              panel[i][j].pc_moveto[0] = panel[i][j].pc_moveto[1] = panel[i][j].pc_mot[0] = panel[i][j].pc_mot[1] = 0;

          }
        }

      if      (cmd == GUIcmd_TEST) exc_cmd = CMD_QUERY;
      else if (cmd == GUIcmd_INIT) exc_cmd = CMD_INIT;
      else if (cmd == GUIcmd_INIF) exc_cmd = CMD_INIF;
      else if (cmd == GUIcmd_INFF) exc_cmd = CMD_INFF;
      else if (cmd == GUIcmd_LSON) exc_cmd = CMD_LSON;
      else if (cmd == GUIcmd_LOFF) exc_cmd = CMD_LOFF;
      else if (cmd == GUIcmd_CNTR) exc_cmd = CMD_CNTR;
      else if (cmd == GUIcmd_CNT2) exc_cmd = CMD_CNT2;
      else if (cmd == GUIcmd_ACAD) exc_cmd = CMD_ACAD;
    }
    else if (cmd == GUIcmd_MVMI) {
      moveflg = 1;
      ncmd   = 0;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0) panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            panel[i][j].pc_moveto[0] = -3000; //these numbers have to be adjusted
            panel[i][j].pc_moveto[1] = -3000;
          }
        }
    }
    else if (cmd == GUIcmd_MVMA) {
      moveflg = 1;
      ncmd   = 0;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0) panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            panel[i][j].pc_moveto[0] =  3000; //these numbers have to be adjusted
            panel[i][j].pc_moveto[1] =  3000;
          }
        }
    }
    else if (cmd == GUIcmd_RAND) {
      moveflg = 1;
      ncmd   = 0;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0) panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            panel[i][j].pc_moveto[0] = rand() % 3000 - 1500;
            panel[i][j].pc_moveto[1] = rand() % 3000 - 1500;
          }
        }
    }
    else if (cmd == GUIcmd_DFOC) {
      moveflg = 1;
      ncmd   = 0;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0) panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            ix = panel[i][j].lut[0][jz][0];
            iy = panel[i][j].lut[0][jz][1];
            if (ix < 0) panel[i][j].pc_moveto[0] = ix + 1900;
            else panel[i][j].pc_moveto[0] = ix - 1900;
            if (iy < 0) panel[i][j].pc_moveto[1] = iy + 1900;
            else panel[i][j].pc_moveto[1] = iy - 1900;
          }
        }
    }
    else if (cmd == GUIcmd_ZERO) {
      moveflg = 1;
      ncmd   = 0;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0) panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            panel[i][j].pc_moveto[0] = 0;
            panel[i][j].pc_moveto[1] = 0;
          }
        }
    }
    else if (cmd == GUIcmd_MVTO) {
      moveflg = 1;
      ncmd   = 0;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0) panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
          }
        }
    }
    else if (cmd == GUIcmd_ADJS ||  cmd == GUIcmd_TPNT) {
      sprintf(lstr, " ADJS, Zen=%d", jz - 100);
      put_logfile(LOG_INF, 0, lstr);

      moveflg = 1;
      ncmd   = 0;
      int loc_foc = global_foclen;
      shiftfoc(loc_foc, global_xoff, global_yoff, 0);

      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0)
            panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            panel[i][j].pc_moveto[0] = panel[i][j].lut[0][jz][0] + panel[i][j].mov_dab[0];
            panel[i][j].pc_moveto[1] = panel[i][j].lut[0][jz][1] + panel[i][j].mov_dab[1];
          }
        }
    }
    else if (cmd == GUIcmd_NEW1) {
      sprintf(lstr, " NEW1, Zen=%d", jz - 100);
      put_logfile(LOG_INF, 0, lstr);

      moveflg = 1;
      ncmd   = 0;
      int loc_foc = global_foclen;
      shiftfoc(loc_foc, global_xoff, global_yoff, 0);

      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          if (ob != NULL && p_but[i][j].select != 0)
            panel[i][j].pan_stat = STAT_TDO; //if we are called with NULL (i.e. from macro) ==> TDO already set
          if (panel[i][j].pan_stat == STAT_TDO) {
            ncmd++;
            panel[i][j].pc_moveto[0] = panel[i][j].new[0][jz][0] + panel[i][j].mov_dab[0];
            panel[i][j].pc_moveto[1] = panel[i][j].new[0][jz][1] + panel[i][j].mov_dab[1];
          }
        }
    }
    else {
      ncmd = 0;
      exc_cmd = CMD_NONE;
      sprintf(lstr, "command %d not yet implemented", cmd);
      put_logfile(LOG_WRN, 0, lstr);
    }

    nok0 = nerr0 = nudf0 = nmov0 = nlas0 = 0;
//  in case of move-command, calculate the needed move_by (and set pc_mot = act_mot)
    if (moveflg > 0 && ncmd > 0) {
      ncmd = 0;
      if (g_AMCstat != 5) {
        g_AMCstat0 = g_AMCstat;  //set to MOVING (if not laser)
        g_AMCstat = 6;
      }
      exc_cmd = CMD_MOVE;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++) {
          tmp_mov[i][j] = -1;
          if (panel[i][j].pan_stat == STAT_TDO) {
            tmp_pan[i][j] = 1;
            if (abs(panel[i][j].act_mot[0]) > 6000 ||
                abs(panel[i][j].act_mot[1]) > 6000) {
              panel[i][j].pan_stat = STAT_NIN;
              nudf0++;
            }
            else {
//                 and make sure pc_moveto is even ....
              nmov0++;
              idum = panel[i][j].pc_moveto[0] / 2;
              panel[i][j].pc_moveto[0] = idum * 2;
              idum = panel[i][j].pc_moveto[1] / 2;
              panel[i][j].pc_moveto[1] = idum * 2;
              panel[i][j].move_by[0] = panel[i][j].pc_moveto[0] - panel[i][j].act_mot[0];
              panel[i][j].move_by[1] = panel[i][j].pc_moveto[1] - panel[i][j].act_mot[1];
              if (abs(panel[i][j].move_by[0]) > 3  || abs(panel[i][j].move_by[1]) > 3
                  || panel[i][j].mot_stat != STAT_OK_) {
                ncmd++;
                panel[i][j].pc_mot[0]  = panel[i][j].act_mot[0];
                panel[i][j].pc_mot[1]  = panel[i][j].act_mot[1];
                tmp_mov[i][j] = 1;                           //flag actuators really moving
              }
              else
                panel[i][j].pan_stat = panel[i][j].mot_stat; //not worth to move; keep status ...
            }
          }
          else if (panel[i][j].pan_stat == STAT_NIN) nudf0++;
          else if (panel[i][j].pan_stat == STAT_ERR) nerr0++;
          else if (panel[i][j].pan_stat == STAT_ER1) nerr0++;
          else if (panel[i][j].pan_stat == STAT_ER2) nerr0++;
          else if (panel[i][j].pan_stat == STAT_BRK) nerr0++;
          else if (panel[i][j].pan_stat == STAT_COM) ncom0++;
          else if (panel[i][j].pan_stat == STAT_BCM) ncom0++;
          else if (panel[i][j].pan_stat == STAT_OK_) nok0++;
          else if (panel[i][j].pan_stat == STAT_OK1) nok0++;
          else if (panel[i][j].pan_stat == STAT_OK2) nok0++;

          if (ob != NULL) {  //set gobal values only if direct action
            g_ok  = nok0;
            g_err = nerr0;
            g_nin = nudf0;
            g_com = ncom0;
            g_mov = nmov0;
          }
        }
    }
    else if (cmd == GUIcmd_INIT || cmd == GUIcmd_INIF || cmd == GUIcmd_INFF || cmd == GUIcmd_CNTR) {
      g_AMCstat = 6;
      nmov0 = 0;
      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++)
          if (panel[i][j].pan_stat == STAT_TDO) {
            nmov0++;
            panel[i][j].pc_moveto[0] = panel[i][j].pc_moveto[1] = -8888;
          }
          else if (panel[i][j].pan_stat == STAT_NIN) nudf0++;
          else if (panel[i][j].pan_stat == STAT_ERR) nerr0++;
          else if (panel[i][j].pan_stat == STAT_ER1) nerr0++;
          else if (panel[i][j].pan_stat == STAT_ER2) nerr0++;
          else if (panel[i][j].pan_stat == STAT_BRK) nerr0++;
          else if (panel[i][j].pan_stat == STAT_COM) ncom0++;
          else if (panel[i][j].pan_stat == STAT_BCM) ncom0++;
          else if (panel[i][j].pan_stat == STAT_OK_) nok0++;
          else if (panel[i][j].pan_stat == STAT_OK1) nok0++;
          else if (panel[i][j].pan_stat == STAT_OK2) nok0++;

      if (ob != NULL) {  //set gobal values only if direct action
        g_ok  = nok0;
        g_err = nerr0;
        g_nin = nudf0;
        g_com = ncom0;
        g_mov = nmov0;
      }
    }


    AMC_check_gui(0, -2.);
//  set up execution scroll-bar (if not called from macro)
    if (ob != NULL && ncmd > 0) {
      g_scrol_max = xcmd = 100;
      fl_set_slider_bounds(FL_scroll, 0., xcmd);
      fl_set_slider_value(FL_scroll, 0.);
      fl_show_object(FL_scroll);
    }

    npan = ncmd;
//  execute the command
    if (ncmd <= 0) {
      sprintf(lstr, "nothing to execute for %d (ncmd %d)...", jcmd, ncmd);
      put_logfile(LOG_DB1, 0, lstr);
    }
    else {
      AMC_check_gui(0, -2.);
      if (numpan == 0) numpan = -ncmd;
      AMC_exec_cmd(panel, exc_cmd, numpan, ipan, jpan);  //<===================== execute it
      AMC_check_gui(0, -2.);

//     in case it was a actuator-command, redo a query if something is fichy....
      if (exc_cmd != CMD_LSON && exc_cmd != CMD_LOFF && exc_cmd != CMD_ACAD) {
        ncmd = 0;
        int q1 = 0;
        for (i = 0; i < 17; i++)
          for (j = 0; j < 17; j++)
            if (tmp_pan[i][j] > 0) {
              if (panel[i][j].pan_stat == STAT_COM ||  panel[i][j].act_stat > COM_LIMIT) {
                push_pan(NULL, -1 * (i * 100 + j)); //deselect panel .. and reset counter
                log_act(i, j, LOG_SVR, 0, jcmd);
                panel[i][j].act_stat = 0;
                panel[i][j].pan_stat = STAT_COM;
                tmp_pan[i][j] = -1; //do not retry in this round
              }
              else if (panel[i][j].pan_stat == STAT_TDO ||
                       abs(panel[i][j].act_mot[0]) > 6000 ||
                       abs(panel[i][j].act_mot[1]) > 6000 ||
                       abs(panel[i][j].act_mot[0] - panel[i][j].pc_mot[0]) > 4 ||
                       abs(panel[i][j].act_mot[1] - panel[i][j].pc_mot[1]) > 4) {
                ncmd++;
                log_act(i, j, LOG_WRN, 0, jcmd);
                panel[i][j].pan_stat = STAT_TDO;
              }
              /* AMC1 must be correctly initialized .......  */
              else {
//                    do we have a reasonable query even if not initialized ???
                if (panel[i][j].pc_moveto[0] == -8888 && abs(panel[i][j].act_mot[0] - panel[i][j].pc_mot[0]) <= 4) {
                  panel[i][j].pc_moveto[0] = panel[i][j].act_mot[0];
                  q1++;
                }
                if (panel[i][j].pc_moveto[1] == -8888 && abs(panel[i][j].act_mot[1] - panel[i][j].pc_mot[1]) <= 4) {
                  panel[i][j].pc_moveto[1] = panel[i][j].act_mot[1];
                  q1++;
                }
              }
            }
        if (q1 > 0) {
          sprintf(lstr, " got %d actuators as correct even if not calibr.", q1);
          put_logfile(LOG_WRN, 0, lstr);
        }

        if (ncmd > 0) {
          AMC_check_gui(0, -2.);
          numpan = -ncmd;
          sprintf(lstr, "redo a query on %d panels", ncmd);
          put_logfile(LOG_WRN, 0, lstr);
          AMC_exec_cmd(panel, CMD_QUERY, numpan, ipan, jpan); // <========== additional query in case
        }
        AMCtime(utime);
        if (g_updtime < 900000) g_updtime = utime[5] + 20; // wait 30s for next possible auto_adjust after any move

      }
      else {      // it was a laser command ==> check box instead of actuator for communic.problems
        ncmd = 0;
        for (i = 0; i < 17; i++)
          for (j = 0; j < 17; j++)
            if (tmp_pan[i][j] > 0)
              if (panel[i][j].pan_stat != STAT_OK1 || panel[i][j].box_stat > 2) {
                ncmd++;
                panel[i][j].pan_stat = STAT_TDO;
              }
        if (ncmd > 0) {
          AMC_check_gui(0, -2.);
          numpan = -ncmd;
          sprintf(lstr, "redo laser cmd %d on %d panels", exc_cmd, ncmd);
          put_logfile(LOG_WRN, 0, lstr);
          AMC_exec_cmd(panel, exc_cmd, numpan, ipan, jpan); // <========== redo laser command
        }
      }

//     maybe there was an intrinsic fast-initialization of the panel ...
      int q2 = 0;

      for (i = 0; i < 17; i++)
        for (j = 0; j < 17; j++)
          if (tmp_pan[i][j] > 0) {
            if (panel[i][j].pan_stat == STAT_COM ||  panel[i][j].act_stat > COM_LIMIT) {
              push_pan(NULL, -1 * (i * 100 + j)); //deselect panel ..
              panel[i][j].act_stat = 0;
              log_act(i, j, LOG_SVR, 0, jcmd);
              panel[i][j].pan_stat = STAT_COM;
            }
            /* not allowed for AMC1; always need correct initialization ....  */
            else if ((panel[i][j].mot_stat == STAT_NIN  ||
                      panel[i][j].pc_moveto[0] == -8888 ||
                      panel[i][j].pc_moveto[1] == -8888) &&
                     abs(panel[i][j].act_mot[0]) < 6000 &&
                     abs(panel[i][j].act_mot[1]) < 6000) {
              panel[i][j].pc_mot[0] = panel[i][j].pc_moveto[0] = panel[i][j].act_mot[0];
              panel[i][j].pc_mot[1] = panel[i][j].pc_moveto[1] = panel[i][j].act_mot[1];
              panel[i][j].mot_stat  = STAT_OK_;
              q2++;
            }
            /* */
          }
      if (q2 > 0) {
        sprintf(lstr, " got %d actuators as correctly initialized", q2);
        put_logfile(LOG_WRN, 0, lstr);
      }

      if (exc_cmd != CMD_LSON && exc_cmd != CMD_LOFF && exc_cmd != CMD_ACAD) {
//        now set final status according to actuator values
        for (i = 0; i < 17; i++)
          for (j = 0; j < 17; j++)
            if (panel[i][j].pan_stat > STAT_BAD)
              if (abs(panel[i][j].act_mot[0]) > 6000 ||
                  abs(panel[i][j].act_mot[1]) > 6000)
                panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_NIN;
              else if (
                abs(panel[i][j].act_mot[0] - panel[i][j].pc_moveto[0]) > 10 ||
                abs(panel[i][j].act_mot[1] - panel[i][j].pc_moveto[1]) > 10)
                panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_ERR;
              else
                panel[i][j].mot_stat = panel[i][j].pan_stat = STAT_OK_;
      }
      else if (exc_cmd == CMD_LSON) { //        now set final status according to laser values
        for (i = 0; i < 17; i++)
          for (j = 0; j < 17; j++)
            if (tmp_pan[i][j] > 0)
              if (panel[i][j].pan_stat != STAT_OK1 ||  panel[i][j].box_stat > 2) {
                panel[i][j].pan_stat = STAT_BCM;
                panel[i][j].box_stat = 0;
              }
              else if (panel[i][j].laser != LAS_ON) panel[i][j].pan_stat = STAT_ERR;
              else                                    panel[i][j].pan_stat = STAT_OK_;
      }
      else if (exc_cmd == CMD_LOFF) { //        now set final status according to laser values
        for (i = 0; i < 17; i++)
          for (j = 0; j < 17; j++)
            if (tmp_pan[i][j] > 0)
              if (panel[i][j].pan_stat != STAT_OK1 ||  panel[i][j].box_stat > 2) {
                panel[i][j].pan_stat = STAT_BCM;
                panel[i][j].box_stat = 0;
              }
              else if (panel[i][j].laser != LAS_OFF) panel[i][j].pan_stat = STAT_ERR;
              else                                    panel[i][j].pan_stat = STAT_OK_;
      }

    }
  } //end of direct actions
  AMC_check_gui(0, -2.);

  nok0 = nerr0 = nudf0 = nmov0 = nlas0 = ncom0 = 0;
  nerr1 = nudf1 = nlas1 = ncom1 = 0;

  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (panel[i][j].pan_inst != 0) {
        for (k = 0; k < 3; k++) panel[i][j].err_cnt_tot[k] += panel[i][j].err_cnt[k];
        nokx = nok0;
        nerrx = 0;
        if (panel[i][j].pan_stat == STAT_TDO || panel[i][j].pan_stat == STAT_ACT) {
          if (ob != NULL) panel[i][j].pan_stat = STAT_BRK;
          else                                      nok0++;
        }

        if      (panel[i][j].pan_stat == STAT_OK_) nok0++;
        else if (panel[i][j].pan_stat == STAT_OK1) nok0++;
        else if (panel[i][j].pan_stat == STAT_OK2) nok0++;
        else if (panel[i][j].pan_stat == STAT_ERR) {
          nerr0++;
          nerrx = 1;
          if (tmp_pan[i][j] != 0) nerr1++;
        }
        else if (panel[i][j].pan_stat == STAT_ER1) {
          nerr0++;
          nerrx = 2;
          if (tmp_pan[i][j] != 0) nerr1++;
        }
        else if (panel[i][j].pan_stat == STAT_ER2) {
          nerr0++;
          nerrx = 3;
          if (tmp_pan[i][j] != 0) nerr1++;
        }
        else if (panel[i][j].pan_stat == STAT_COM) {
          ncom0++;
          nerrx = 11;
          if (tmp_pan[i][j] != 0) ncom1++;
        }
        else if (panel[i][j].pan_stat == STAT_BCM) {
          ncom0++;
          nerrx = 11;
          if (tmp_pan[i][j] != 0) ncom1++;
        }
        else if (panel[i][j].pan_stat == STAT_BRK) {
          nbrk0++;
          nerrx = 21;
          if (tmp_pan[i][j] != 0) nbrk1++;
        }
        else if (panel[i][j].pan_stat == STAT_NIN) {
          nudf0++;
          nerrx = 31;
          if (tmp_pan[i][j] != 0) nudf1++;
        }
        if      (panel[i][j].laser    == LAS_ON) {
          nlas0++;
          if (tmp_pan[i][j] != 0) nlas1++;
        }

        if (tmp_mov[i][j] > 0) {    //print only selected panels ....

          if (jcmd == GUIcmd_LOFF || jcmd == GUIcmd_LSON)
            if (nok0 > nokx)  log_act(i, j, LOG_LAS, -2, jcmd);
            else              log_act(i, j, LOG_ERR, 0, jcmd);

          else if (jcmd == GUIcmd_INIT || jcmd == GUIcmd_INFF || jcmd == GUIcmd_INIF)
            upd_act_pan(i, j, 1, -1);

          if (moveflg >= 0)
            if (nerrx == 0)  log_act(i, j, LOG_ACT, -2, jcmd);
            else              log_act(i, j, LOG_ERR, 0, jcmd);

        }
      }

  sprintf(lstr, " CMD %2d: Panels in state: ok %3d, err %3d, undef %3d, com %3d; lasers on %3d", jcmd, nok0, nerr0, nudf0, ncom0, nlas0);
  put_logfile(LOG_INF, 0, lstr);
  sprintf(lstr, " CMD %2d: Selectd Panels in state: err %3d, undef %3d, com %3d; lasers on %3d", jcmd,     nerr1, nudf1, ncom1, nlas1);
  put_logfile(LOG_INF, 0, lstr);

  // put complete GUI back to standard state again if we are not called from a macro
  if (ob != NULL) {
    g_ok  = nok0;                 //set global values only if direct action
    g_err = nerr0 + nbrk0;
    g_com = ncom0;
    g_nin = nudf0;
    g_mov = 0;

    if (nlas0 > 0) g_AMCstat = 5;     //laser is  on ....
    else if (cmd == GUIcmd_LSON || cmd == GUIcmd_LOFF) g_AMCstat = g_AMCstat0;  //set to what was before
    else if (cmd == GUIcmd_INIT || cmd == GUIcmd_INIF || cmd == GUIcmd_INIF ||
             cmd == GUIcmd_CNTR || cmd == GUIcmd_CNT2) g_AMCstat = g_AMCstat0 = 2; //initialized
    else if (cmd == GUIcmd_ADJS || cmd == GUIcmd_LSAD) g_AMCstat = g_AMCstat0 = 4; //adjusted
    else if (g_nin < 20)                               g_AMCstat = g_AMCstat0 = 2;
    else if (cmd != GUIcmd_TEST)                       g_AMCstat = g_AMCstat0 = 1; //undefined position

    fl_hide_object(FL_scroll);

    istat = AMC_check_gui(999, -2.);                //reset 'BREAK' flag
    fl_set_button(ob, 0);                            //reset action-button

//instead of FLhidden/DUMMY re-activate all selectors and EXIT
    fl_hide_object(FLbreak);
    fl_activate_object(FLselect1_grp);
    fl_activate_object(FLselect2_grp);
    fl_activate_object(FLexit);
    fl_set_object_lcol(FLselect1_grp, 900);
    fl_set_object_lcol(FLselect2_grp, 900);
    fl_set_object_lcol(FLexit, 900);
    fl_reset_cursor(FL_ObjWin(FLexit));

    fl_gettime(&isec, &iusec);

    idur = isec - isec0;

    xdur = (isec + iusec / 1000000.) - (isec0 + iusec0 / 1000000.);
    if (xdur > 100.) {
      idur = xdur + 0.5;
      sprintf(str, "%4d sec", idur);
    }
    else sprintf(str, "%4.1f sec", xdur);

    fl_set_object_label(FL_durat, str);

    sprintf(lstr, "<---- action %2d ended after %4.1fs   (for %3d panels)", cmd, xdur, npan);
    put_logfile(LOG_INF, 0, lstr);  //log only if not called from macro
    if (g_updtime < 900000) g_updtime += 30; // wait another 30s for next possible auto_adjust after an CC command

    if (g_cc_cmd < 0) {
      g_cc_cmd = 0;                                 // in case we were executing CC command
      g_updtime += 30;                               // add another 30 sec wait after cc-commands
      fl_set_object_color(FLcmd_cmd, 400, 410);
      fl_set_object_color(FLcmd_time, 400, 410);
    }
    global_action = -1;
    if (g_initgui < 0) g_initgui = -g_initgui;
    mode_cb(NULL, 0);   //activate all action-buttons according to mode
  }
}

//--------------------------------------------------------------------

int  AMC_check_gui(int n, double scroll)
{
  /*  check if 'BREAK' from GUI or CC; return 1 if 'BREAK', else 0
      n <0   :  intermediate  update GUI and return
      n =0   :  update GUI and return immediately
      n =999 :  update GUI, reset 'BREAK'-flag and return
  */

#define MAXSLEEP  5000

  FL_OBJECT *obj;

  upd_all_pan(n);    //update panel display
  if (scroll > 0. && scroll <=  g_scrol_max)
    fl_set_slider_value(FL_scroll, scroll);
  else if (scroll > 0)
    printf("illegal scroll %f %f\n", scroll, g_scrol_max);

  obj = fl_check_forms();      //check GUI and CC
  if ((global_break == 0) && (obj != NULL)) {
    fl_hide_object(FLbreak);
    global_break = 1;
  }

  if (n == 999)
    global_break = 0;

  return global_break;
}

//--------------------------------------------------------------------

void push_dummy(FL_OBJECT *ob, long n)
{
  printf("dummy pressed \n");

  sprintf(lstr, "dummy button %d pressed", n);
  put_logfile(LOG_DB1, 0, lstr);

  return;
}

//--------------------------------------------------------------------

void push_pan(FL_OBJECT *ob, long n)
{
  int i, j, k, l, m, ii, jj, i0, j0, ib, jsele, jqsele, jcolr, jbut, nsele;
  FL_OBJECT *obj;
  char button[20];

  fl_freeze_form(form);
  sprintf(button, " ");

  if (global_action > 0) {
    m = abs(n);
//    command execution is active; just show info about selected panel
//    and we have to toggle the button again ....
    if (m < 9900) {
      ii = m / 100;
      jj = m - 100 * ii;
      if (n < 0) p_but[ii][jj].select = 0;   //unselect panel
      obj = FLpanel[ii][jj];
      if (panel[ii][jj].pan_stat > STAT_NOT) {
        obj = FLpanel[ii][jj];
        if (ob != NULL) {
          jbut = 1 - fl_get_button(obj);
          fl_set_button(obj, jbut);
          upd_act_pan(ii, jj, nsele, 1);
        }
        else {
          fl_set_button(obj, 0);   //always deselect ...
        }
      }
    }
    fl_unfreeze_form(form);
    return;
  }

//make sure all bad panels are unselected ....
//except for single selection or 'com' selection in MODE_MANU
  if (n > 9000 || global_mode < MODE_EXPT) {
    for (j = 0; j < 17; j++)
      for (i = 0; i < 17; i++)
        if (panel[i][j].pan_stat > STAT_NOT && panel[i][j].pan_stat <= STAT_BAD) {
          p_but[i][j].select = 0;
          fl_set_button(FLpanel[i][j], 0);
        }
  }

  nsele = 0;
  jsele = 1;                                     //default: select
  jqsele = 0;                                    //and opposite
  if (n < 0) {
    jsele = 0;
    jqsele = 1;
    n = -n;
  }

  if (ob != NULL && global_mode > MODE_OPER) {  //checking mouse only if button pressed
    jbut = fl_get_button_numb(ob);
    if (jbut == FL_MIDDLE_MOUSE) jsele = 0;   //unselect panel(group)
    if (jbut == FL_LEFT_MOUSE && n != 70000) { //unselect all selected so far, unless invert button pressed
      for (j = 0; j < 17; j++)   {
        for (i = 0; i < 17; i++)  {
          if (panel[i][j].pan_stat >= STAT_NOT && p_but[i][j].select != 0) {
            p_but[i][j].select = 0;
            fl_set_button(FLpanel[i][j], 0);
          }
        }
      }
    }
  }

  ii = jj = -99;

  if (n >= 90000) {             //[un]select all
    sprintf(button, " ALL  %d ", jsele);
    for (j = 0; j < 17; j++)   {
      for (i = 0; i < 17; i++)  {
        if (panel[i][j].pan_stat > STAT_BAD) {
          p_but[i][j].select = jsele;
          fl_set_button(FLpanel[i][j], jsele);
        }
      }
    }
  }
  else if (n >= 70000) {       // invert selection
    sprintf(button, " Invert ");
    if (jbut == FL_LEFT_MOUSE) {             //unselect all selected so far
      for (j = 0; j < 17; j++)   {
        for (i = 0; i < 17; i++)  {
          if (panel[i][j].pan_stat > STAT_BAD && p_but[i][j].select == jsele) {
            p_but[i][j].select = jqsele;
            fl_set_button(FLpanel[i][j], jqsele);
          }
          else if (panel[i][j].pan_stat > STAT_BAD && p_but[i][j].select == jqsele) {
            p_but[i][j].select = jsele;
            fl_set_button(FLpanel[i][j], jsele);
          }
        }
      }
    }
  }
  else if (n >= 60000) {        //Chess Board
    sprintf(button, " Chess ");
    for (j = 1; j < 17; j+=2)   {
      for (i = 1; i < 17; i+=2)  {
        if (panel[i][j].pan_stat > STAT_BAD) {
          p_but[i][j].select = jsele;
          fl_set_button(FLpanel[i][j], jsele);
        }
      }
    }
    for (j = 0; j < 17; j+=2)   {
      for (i = 0; i < 17; i+=2)  {
        if (panel[i][j].pan_stat > STAT_BAD) {
          p_but[i][j].select = jsele;
          fl_set_button(FLpanel[i][j], jsele);
        }
      }
    }
  }
  else if (n >= 50000) {        //[un]select Alu / Glas
    k = n - 50000;            // 1 = Glas; 0 = Alu
    if (k == 0) sprintf(button, " Glas %d ", jsele);
    else          sprintf(button, " Alu  %d ", jsele);
    for (j = 0; j < 17; j++)   {
      for (i = 0; i < 17; i++)  {
        if (panel[i][j].pan_stat > STAT_BAD && panel[i][j].pan_material == k) {
          p_but[i][j].select = jsele;
          fl_set_button(FLpanel[i][j], jsele);
        }
      }
    }
  }
  else if (n >= 40000) {        //[un]select PanelGroup
    k = n - 40000;
    sprintf(button, " PG%d  %d ", k, jsele);
    for (j = 0; j < 17; j++)   {
      for (i = 0; i < 17; i++)  {
        if (panel[i][j].pan_stat > STAT_BAD && panel[i][j].pan_grp == k) {
          p_but[i][j].select = jsele;
          fl_set_button(FLpanel[i][j], jsele);
        }
      }
    }
  }
  else if (n >= 30000) {        //[un]select laser_group
    k = n - 30000;
    sprintf(button, " las%d  %d ", k, jsele);
    for (j = 0; j < 17; j++)   {
      for (i = 0; i < 17; i++)  {
        if (panel[i][j].pan_stat > STAT_BAD && panel[i][j].laser == k) {
          p_but[i][j].select = jsele;
          fl_set_button(FLpanel[i][j], jsele);
        }
      }
    }
  }
  else if (n >= 20000) {        //[un]select port_group
    k = n - 20000;
    sprintf(button, " port%d %d ", k + 1, jsele);
    for (j = 0; j < 17; j++)   {
      for (i = 0; i < 17; i++)  {
        for (l = 0; l < 3; l++)  {
          m = panel[i][j].portflg[l];
          if (panel[i][j].pan_stat > STAT_BAD && panel[i][j].port[m][l] == k) {
            p_but[i][j].select = jsele;
            fl_set_button(FLpanel[i][j], jsele);
          }
        }
      }
    }
  }
  else if (n >=  9900) {        //[un]select status_group
    k = n - 10000;
    sprintf(button, " Stat%d %d ", k, jsele);
    for (j = 0; j < 17; j++)   {
      for (i = 0; i < 17; i++)  {
        if (panel[i][j].pan_stat == k) {
          p_but[i][j].select = jsele;
          fl_set_button(FLpanel[i][j], jsele);
        }
      }
    }
  }
  else {                        //[un]select single panel
    ii = n / 100;
    jj = n - 100 * ii;
    if (panel[ii][jj].pan_stat <= STAT_NOT) jsele = 0; //nonexisting panels
    p_but[ii][jj].select = jsele;
    fl_set_button(FLpanel[ii][jj], jsele);
    sprintf(button, " (%2d,%2d) %d ", ii - 8, jj - 8, jsele);
  }

//to be sure: count number of selected panels
  nsele = 0;
  i0 = j0 = -1;
  for (j = 0; j < 17; j++)
    for (i = 0; i < 17; i++)
      if (panel[i][j].pan_stat >= STAT_NOT && p_but[i][j].select != 0) {
        nsele++;
        i0 = i;
        j0 = j;
      }

  if (nsele == 1 && ii >= 0 && jj >= 0) {
    ii = i0;
    jj = j0;
  }
  g_nsele = nsele;

  sprintf(lstr, "select panel %s; total selected=%d", button, nsele);
  put_logfile(LOG_OK_, 0, lstr);

  fl_unfreeze_form(form);
  upd_act_pan(ii, jj, nsele, 1);

}

//--------------------------------------------------------------------

void push_pwr(FL_OBJECT *obj, long m)
{
  int i, j, ierr, ibut, type, nominal[32], status[32];
  int a, b, c;
  long n;
  FL_OBJECT *ob;

  char str[20];
  union {
    uint8_t b[4];
    uint32_t l;
  } x;


  sprintf(lstr, "push power %d ", m);
  put_logfile(LOG_OK_, 0, lstr);
  n = m;

  ob = NULL;
  if (obj == NULL) {
    n = m;       //no button pressed -> assume correct
    if (n == PWR_SBIG) {
      ibut = 1;
      ob = FLpwr[2];
    }
    else if (n == -PWR_SBIG) {
      ibut = 0;
      ob = FLpwr[2];
    }
    if (n == PWR_AMC) {
      ibut = 1;
      ob = FLpwr[4];
    }
    else if (n == -PWR_AMC) {
      ibut = 0;
      ob = FLpwr[4];
    }
  }
  else {
    ob = obj;
    ibut = fl_get_button(ob);
    if (ibut == 0) n = -m;      //button released -> switch off
    else            n = m;       //button pressed  -> switch on
  }

  if (n == 0 || abs(n) > 8) {
    sprintf(str, "./power.pl \n");
  }
  else if (n > 0) {
    sprintf(str, "./power.pl    -on=%1d", n);
    power[n].request = 1;

    sprintf(lstr, "push ON1   %d ", m);
    put_logfile(LOG_OK_, 0, lstr);
  }
  else {
    sprintf(str, "./power.pl    -off=%1d", -n);
    power[-n].request = 0;

    if (n == -PWR_AMC) {  //we switch off AMC power ==>
      //  we loose all info about panel
      global_break = 1;
      mode_cb(NULL, -9);  //deactivate all action-buttons
      for (a = 0; a < 17; a++)
        for (b = 0; b < 17; b++)
          if (panel[a][b].pan_stat > STAT_DIS) {
            panel[a][b].pan_stat = panel[a][b].mot_stat = STAT_NIN;
            panel[a][b].act_mot[0] =
              panel[a][b].act_mot[1] =
                panel[a][b].pc_mot[0] =
                  panel[a][b].pc_mot[1] =
                    panel[a][b].pc_moveto[0] =
                      panel[a][b].pc_moveto[1] =
                        -8888;
          }
      upd_all_pan(0);
    }
  }
  printf("trying to connect to power-box\n");
  printf(" if it blocks for long time, press <CTRL><C>\n");
  x.l = system(str);

  for (i = 1; i < 9; i++) {
    power[9 - i].actual = x.b[1] % 2;
    x.b[1] = x.b[1] / 2;
  }

  if (m > 999) {   //very special: set all buttons as they are
    for (i = 1; i < 9; i++) {
      ob = FLpwr[i];
      if (ob != NULL) fl_set_button(ob, power[i].actual);
      if (power[i].deflt >= 0) power[i].request = power[i].actual;
    }
  }

  if (ob != NULL) {
    fl_set_button(ob, ibut);
  }

  if (n == PWR_AMC && ob != NULL) {  //we switch on AMC power from panel
    sprintf(lstr, "push ON2   %d ", m);
    put_logfile(LOG_OK_, 0, lstr);
    g_initgui =  3;                 // --> ask to re-initialize
  }

}

//--------------------------------------------------------------------

void log_err_ctr()
{
  int i, j;

  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (panel[i][j].pan_inst != 0) {
        sprintf(lstr, "errcnt: (%2d,%2d) %4d %4d %6d", i - 8, j - 8,
                panel[i][j].err_cnt_tot[0],
                panel[i][j].err_cnt_tot[1],
                panel[i][j].err_cnt_tot[2]);
        put_logfile(LOG_DB1, -1, lstr);
      }

}

//--------------------------------------------------------------------

void exit_cb(FL_OBJECT *obj, long n)
{
  int status, ret, iq;
  int try = 0;
  long utime[5];

  AMC_check_gui(0, -2.);

  if (g_exiton > 0)
    return;

  g_exiton = 1;

  if (obj != NULL) {
    fl_add_timeout(5000, timeout_yesno, 0);
    ret = fl_show_question("Do you really want to quit AMC ?\n(you have 5 seconds to press NO)", 1);
    printf("exit: %d\n", ret);
    if (ret != 1) {
      //release button
      fl_set_button(obj, 0);      //set corresponding action-button
      g_exiton = -1;
      return;
    }
  }

  sprintf(lstr, "EXIT  button pressed; can take upto 30 seconds to ramp");
  put_logfile(LOG_WRN, 0, lstr);
  AMC_check_gui(0, -2.);

  log_err_ctr();    // for all panels, write total amount of errors

  g_sbig_brk = -1; //break any picture taking if still going on ...

  if (n != 0) {
    //usually, switch off SBIG in correct way
    sprintf(lstr, "waiting for SBIG-thread to finish ....");
    put_logfile(LOG_DB1, 0, lstr);
    AMC_check_gui(0, -2.);
    if (TMtemp != 0) {
      fl_remove_timeout(TMtemp);
      TMtemp = 0;
    }
    try = 0;

    if (g_sbig_th != 0)
      ret = pthread_join(sbig_threads[0], (void **)&status);
    pthread_create(&sbig_threads[0], NULL, CloseST7_th, NULL);
    ret = pthread_join(sbig_threads[0], (void **)&status);
  }
  else
    push_pwr(NULL, -PWR_SBIG); //or just kill power ...

  if (myImage)
    free(myImage);     //might still be in use

  g_ccrec = -1;  //flag CC_receive to close connection
  g_ccsnd = -1;  //flag CC_send to close connection

  sleep(1);
  //must!!!! wait for power-thread to finish (else USCM could hang)
  g_exit = -1;

  sprintf(lstr, "waiting for PWR-thread to finish ....");
  put_logfile(LOG_DB1, 0, lstr);
  AMC_check_gui(0, -2.);
  sleep(1);
  pthread_join(pwr_threads[0], (void **)&status);

  //close connections to CC
  sprintf(lstr, "Close connections from CC");
  put_logfile(LOG_DB1, 0, lstr);
  pthread_join(cc_threads[0], (void **)&status);
  sprintf(lstr, "Close connections to CC");
  put_logfile(LOG_DB1, 0, lstr);
//pthread_join(cc_threads[1], (void **)&status);
//might block when no communication ===> close only the socket ...
  close(g_cc_tosock);

  sprintf(lstr, "EXIT the program");
  put_logfile(LOG_DB1, 0, lstr);
  AMC_check_gui(0, -2.);

  AMCtime(utime);
  fl_finish();
//close log-file
  fprintf(f_log, "%06d ---------------e-n-d---------------------- \n", utime[0]);
  fclose(f_log);

  fprintf(f_err, "%06d ---------------e-n-d---------------------- \n", utime[0]);
  fclose(f_err);

  exit(0);
}

//--------------------------------------------------------------------

void lut_cb(FL_OBJECT *ob, long n)
{
  int i, j;
  j = NEW_read(panel);
  i = LUT_read(panel, j);
  sprintf(lstr, "did read %d LUT- and %d NEW-files", i, j);
  put_logfile(LOG_DB1, 0, lstr);

}

//--------------------------------------------------------------------

void mode_cb(FL_OBJECT *ob, long m)
{
  int i, n, flag;
  long utime[5];

  n = m;

  if (ob == NULL && n > 10) {  //set default value ...
    if      (n == MODE_AUTO)  fl_set_button(FLmodA, 1);
    else if (n == MODE_OPER)  fl_set_button(FLmodO, 1);
    else if (n == MODE_EXPT)  fl_set_button(FLmodX, 1);
    else if (n == MODE_MANU)  fl_set_button(FLmodM, 1);
  }

  flag = -1;
  if (n > 0) {                         //mode switched
    flag = 0;
    global_mode = n;

    if (g_ccstat > 0)
      if (n == MODE_MANU) fl_set_object_color(FLin_time, 404, 414);   //blue
      else              fl_set_object_color(FLin_time, 403, 413);     //red

    if (n == MODE_MANU) {              //disable interpretation of commands if MANU
      g_updtime = 999999;                  //   disable autoadjust
      g_ccrec = 0;
      fl_activate_object(FLset_Zd);
      fl_activate_object(FLset_Az);
      fl_activate_object(FLset_Foc);
      fl_activate_object(FLset_X);
      fl_activate_object(FLset_Y);
      fl_set_object_color(FLset_Zd, FL_WHITE        , BUSY_COL);
      fl_set_object_color(FLset_Az, FL_WHITE        , BUSY_COL);
      fl_set_object_color(FLset_Foc, FL_WHITE        , BUSY_COL);
      fl_set_object_color(FLset_X , FL_WHITE        , BUSY_COL);
      fl_set_object_color(FLset_Y , FL_WHITE        , BUSY_COL);
    }
    else {
      g_ccrec = 1;
      AMCtime(utime);
      g_updtime = utime[5] + 30;           // wait 30 sec for next possible auto_adjust
      fl_deactivate_object(FLset_Zd);
      fl_deactivate_object(FLset_Az);
      fl_deactivate_object(FLset_Foc);
      fl_deactivate_object(FLset_X);
      fl_deactivate_object(FLset_Y);
      fl_set_object_color(FLset_Zd, FL_BUTTON_COL1  , BUSY_COL);
      fl_set_object_color(FLset_Az, FL_BUTTON_COL1  , BUSY_COL);
      fl_set_object_color(FLset_Foc, FL_BUTTON_COL1  , BUSY_COL);
      fl_set_object_color(FLset_X , FL_BUTTON_COL1  , BUSY_COL);
      fl_set_object_color(FLset_Y , FL_BUTTON_COL1  , BUSY_COL);
    }

    if (n == MODE_AUTO || n == MODE_OPER) {
      push_pan (NULL, 90000);             //set all selected
      fl_deactivate_object(FLselect1_grp);
      fl_hide_object(FLselect2_grp);
      fl_set_button(FLautoFoc, DEF_AUTOFOC);
      fl_set_button(FLautoAdj, DEF_AUTOADJ);
      fl_set_button(FLstarFoc, DEF_STARFOC);
      global_autoFoc = DEF_AUTOFOC;
      global_autoAdj = DEF_AUTOADJ;
      global_starFoc = DEF_STARFOC;
    }
    else {
      fl_activate_object(FLselect1_grp);
      fl_show_object(FLselect2_grp);
    }
  }

  if (m == 0) n = global_mode; //activate according to global mode
  if (m < 0) n = -999;       //deactivate all

  if (m >= 0) {
    fl_activate_object(FLmod_grp);
    fl_set_object_lcol(FLmod_grp, 900);
  }
  else {
    fl_deactivate_object(FLmod_grp);
    fl_set_object_lcol(FLmod_grp, 901);
  }

  for (i = 0; command[i].grp > 0; i++) {
    if (command[i].grp <= n) {
      fl_activate_object(command[i].obj);
      fl_set_object_lcol(command[i].obj, 900);
    }
    else {
      fl_deactivate_object(command[i].obj);
      fl_set_object_lcol(command[i].obj, 901);
    }
  }

  if (g_nsele == 1 && (n == MODE_EXPT || n == MODE_MANU)) {
    fl_activate_object(FLmov_grp);
    fl_set_object_lcol(FLmov_grp, 900);
  }
  else {                                          //deactivate move-group
    fl_deactivate_object(FLmov_grp);
    fl_set_object_lcol(FLmov_grp, 901);
  }

  if      (global_mode == MODE_AUTO) sprintf(lstr, "set mode: AUTO,%d", m);
  else if (global_mode == MODE_OPER) sprintf(lstr, "set mode: OPER,%d", m);
  else if (global_mode == MODE_EXPT) sprintf(lstr, "set mode: EXPT,%d", m);
  else if (global_mode == MODE_MANU) sprintf(lstr, "set mode: MANU,%d", m);
  else if (global_mode == MODE_OPER) sprintf(lstr, "set mode: %d, %d", global_mode, m);

  put_logfile(LOG_OK_, flag, lstr);
}

//--------------------------------------------------------------------

void break_cb(FL_OBJECT *ob, long n)
{
  int    ibut;
  char   str[10];

  global_break = 1;
  sprintf(lstr, "break button %d pressed", n);
  put_logfile(LOG_DB1, 0, lstr);

}

//--------------------------------------------------------------------

void param_cb(FL_OBJECT *ob, long n)
{
  int    ibut;
  char lstr[LOGLEN];

  ibut = fl_get_button(ob);

  switch (n) {
  case  (1):
    global_autoAdj = ibut;
    sprintf(lstr, "AutoAdj=%d", global_autoAdj);
    break;
  case  (2):
    global_autoFoc = ibut;
    sprintf(lstr, "AutoFoc=%d", global_autoFoc);
    break;
  case  (3):
    global_starFoc = ibut;
    sprintf(lstr, "StarFoc=%d", global_starFoc);
    break;
  default :
    sprintf(lstr, "unknown parameter %d", n);
    break;
  }
  put_logfile(LOG_OK_, 0, lstr);

}

//--------------------------------------------------------------------

void gen_actions(int ix, int iy, int dx, int dy)
{
  int i, jx, jy;
  FL_OBJECT *obj;

  for (i = 0; i < 50; i++)
    command[i].grp = -1;

  i = 0;

  jx = ix;
  jy = iy;
  FL_actIni =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "INIT");
  fl_set_object_callback(obj, action_cb, GUIcmd_INIT);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_INIT;
  command[i].grp = MODE_OPER;
  i++;

  jx = jx + dx;
  FL_actInif =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "INIFast");
  fl_set_object_callback(obj, action_cb, GUIcmd_INIF);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_INIF;
  command[i].grp = MODE_OPER;
  i++;

  jx = jx + dx;
  FL_actAdj =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "ADJUST");
  fl_set_object_callback(obj, action_cb, GUIcmd_ADJS);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_ADJS;
  command[i].grp = MODE_OPER;
  i++;

  jx = jx + dx;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "TstADJ");
  fl_set_object_callback(obj, action_cb, GUIcmd_NEW1);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_NEW1;
  command[i].grp = MODE_EXPT;
  i++;

  jy = jy + dy;
  jx = ix;
  FL_actCen =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "CENTER");
  fl_set_object_callback(obj, action_cb, GUIcmd_CNTR);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_CNTR;
  command[i].grp = MODE_OPER;
  i++;

  jx = jx + dx;
  FL_actZer =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "ZERO");
  fl_set_object_callback(obj, action_cb, GUIcmd_ZERO);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_ZERO;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  FL_actDef =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "Defocus");
  fl_set_object_callback(obj, action_cb, GUIcmd_DFOC);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_DFOC;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  FLautoFoc =
    obj = fl_add_lightbutton(FL_PUSH_BUTTON, jx, jy, dx, dy, "Auto-Foc");
  fl_set_object_callback(obj, param_cb,     2);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_NONE;
  command[i].grp = MODE_UDEF;
  i++;

  jy = jy + dy;
  jx = ix;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "MovMin");
  fl_set_object_callback(obj, action_cb, GUIcmd_MVMI);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_CNTR;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "MovMax");
  fl_set_object_callback(obj, action_cb, GUIcmd_MVMA);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_CNT2;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "Randomize");
  fl_set_object_callback(obj, action_cb, GUIcmd_RAND);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_DFOC;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  FLstarFoc =
    obj = fl_add_lightbutton(FL_PUSH_BUTTON, jx, jy, dx, dy, "Star-Foc");
  fl_set_object_callback(obj, param_cb,     3);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_NONE;
  command[i].grp = MODE_UDEF;
  i++;

  jy = jy + dy;
  jx = ix;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "LasOn");
  fl_set_object_callback(obj, action_cb, GUIcmd_LSON);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_LSON;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  FL_lasoff =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "LasOff");
  fl_set_object_callback(obj, action_cb,  GUIcmd_LOFF);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_LOFF;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "LasADJ");
  fl_set_object_callback(obj, action_cb,  GUIcmd_LSAD);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_LSAD;
  command[i].grp = MODE_UDEF;
  i++;

  jx = jx + dx;
  FLautoAdj =
    obj = fl_add_lightbutton(FL_PUSH_BUTTON, jx, jy, dx, dy, "Auto-Adj");
  fl_set_object_callback(obj, param_cb,     1);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_NONE;
  command[i].grp = MODE_UDEF;
  i++;

  jy = jy + dy;
  jx = ix;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "Calibrate");
  fl_set_object_callback(obj, action_cb, GUIcmd_CALI);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_CALI;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "Single ADJ");
  fl_set_object_callback(obj, action_cb, GUIcmd_PLAD);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_PLAD;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "Group ADJ");
  fl_set_object_callback(obj, action_cb, GUIcmd_STAD);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_STAD;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + dx;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "Roque ADJ");
  fl_set_object_callback(obj, action_cb, GUIcmd_RQAD);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_RQAD;
  command[i].grp = MODE_UDEF;
  i++;

  FLmov_grp = fl_bgn_group();

  jx = ix + 0.5 * dx - 6;
  jy = jy + dy + 3;
  FLmova =
    obj = fl_add_counter(FL_NORMAL_COUNTER, jx, jy, 135, 23, "MovA");
  fl_set_counter_value(obj, 0.);
  fl_set_counter_bounds(obj, -4000., 4000.);
  fl_set_counter_step(obj, 10., 100.);
  fl_set_counter_precision(obj, 0);
  fl_set_object_callback(obj, action_cb, GUIcmd_MOVA);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fl_set_object_color(obj, FL_BUTTON_COL1, FL_BLACK);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_MOVA;
  command[i].grp = MODE_EXPT;
  i++;

  jx = jx + 2 * dx;
  FLmovb =
    obj = fl_add_counter(FL_NORMAL_COUNTER, jx, jy, 135,  23, "MovB");
  fl_set_counter_value(obj, 0.);
  fl_set_counter_bounds(obj, -4000., 4000.);
  fl_set_counter_step(obj, 10., 100.);
  fl_set_counter_precision(obj, 0);
  fl_set_object_callback(obj, action_cb, GUIcmd_MOVB);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fl_set_object_color(obj, FL_BUTTON_COL1, FL_BLACK);
  command[i].obj = obj;
  command[i].cmd = GUIcmd_MOVB;
  command[i].grp = MODE_EXPT;

  fl_end_group();         //FLmov_grp

  fl_set_button(FLautoFoc, DEF_AUTOFOC);
  global_autoFoc = DEF_AUTOFOC;
  fl_set_button(FLautoAdj, DEF_AUTOADJ);
  global_autoAdj = DEF_AUTOADJ;
}

//--------------------------------------------------------------------

void gen_pwr(int jx, int jy, int dxx, int dyy)
{
  int i, ix, iy, dx, dy, ip, jp, ix02, iy02, ix04, iy04;
  int ix1, ix2, iy1, iy2;
  char  str[30];
  float ddy;
  FL_OBJECT *obj;

  dx = dxx;
  dy = dyy;
  ix = jx + dx / 2.;
  iy = jy + dy / 2.;

  FLpwr_grp = fl_bgn_group();

  for (ip = 0; ip < 32; ip++)
    FLpwr[ip] = NULL;

  fl_end_group();
  fl_deactivate_object(FLpwr_grp);
  fl_set_object_lcol(FLpwr_grp,  901);

  jp = 0;
  FLpwr[jp] =
    obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, 2 * dx, dy, power[jp].str);
  fl_set_object_callback(obj, push_pwr, jp);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  iy = iy + dy;

  jp = 2; //sbig
  FLpwr[jp] =
    obj = fl_add_button(FL_PUSH_BUTTON, ix, iy, 2 * dx, dy, power[jp].str);
  fl_set_object_callback(obj, push_pwr, jp);
  fl_set_object_color(obj, 400, 411);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  iy = iy + dy;

  jp = 4; //AMC
  FLpwr[jp] =
    obj = fl_add_button(FL_PUSH_BUTTON, ix, iy, 2 * dx, dy, power[jp].str);
  fl_set_object_callback(obj, push_pwr, jp);
  fl_set_object_color(obj, 403, 410);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  push_pwr(NULL, 1000);

}

//--------------------------------------------------------------------

void setting_cb(FL_OBJECT *ob, long n)
{
  int i;
  int x;
  char* in[20], out[20];

  x = atoi(fl_get_input(ob));
  sprintf(out, "%4d", x);

  if (n == 1) {            //zenith
    global_zenith = x;
    sprintf(lstr, "set Zenit: %d", x);
    put_logfile(LOG_DB1, 0, lstr);
  }
  else if (n == 2) {       //azimuth
    global_azimut = x;
    sprintf(lstr, "set Azimut: %d", x);
    put_logfile(LOG_DB1, 0, lstr);
  }
  else if (n == 3) {       //foclen
    global_foclen = x;
    sprintf(lstr, "set Foclen: %d", x);
    put_logfile(LOG_DB1, 0, lstr);
  }
  else if (n == 4) {       //foclen
    global_xoff   = x;
    sprintf(lstr, "set Xoff  : %d", x);
    put_logfile(LOG_DB1, 0, lstr);
  }
  else if (n == 5) {       //foclen
    global_yoff   = x;
    sprintf(lstr, "set Yoff  : %d", x);
    put_logfile(LOG_DB1, 0, lstr);
  }

  fl_set_focus_object(form,  FLinp_dummy);

}

//--------------------------------------------------------------------

void gen_CC(int ix, int iy, int dx, int dy)
{
  FL_OBJECT *obj;
  int jx, jy;
  //colors  blue /green/orange/red

  jx = ix;
  jy = iy;
  FL_CC_grp = fl_bgn_group();
  obj = fl_add_box(FL_NO_BOX, jx, jy, dx, dy, "From CC:");
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);

  jx = jx + dx;
  jy = jy + 2;
  FLin_time =        //color of time: CC_in:  undef / ok / warn / err
    obj = fl_add_box(FL_BORDER_BOX, jx, jy, dx, dy - 3, "--:--:--");
  fl_set_object_color(obj, 404, 414);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  jx = ix;
  jy = jy + dy;
  FLin_Zd =          //color of Zd/Az:
    obj = fl_add_box(FL_BORDER_BOX, jx, jy, dx / 2, dy - 3, "000");
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  jx = jx + 1.33 * dx;
  FLset_Zd =
    obj = fl_add_input(FL_INT_INPUT, jx, jy, dx * 0.67, dy - 3, ":Zenith:");
  fl_set_input(obj, "20");
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, setting_cb,  1);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);

  jx = ix;
  jy = jy + dy;
  FLin_Az =
    obj = fl_add_box(FL_BORDER_BOX, jx, jy, dx / 2, dy - 3, "000");
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  jx = jx + 1.33 * dx;
  FLset_Az =
    obj = fl_add_input(FL_INT_INPUT, jx, jy, dx * 0.67, dy - 3, ":Azimut:");
  fl_set_input(obj, "0");
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, setting_cb,  2);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);

  jx = ix + 10;
  jy = jy + dy;
  FLset_Foc =
    obj = fl_add_input(FL_INT_INPUT, jx, jy, dx * 0.50, dy - 3, "F");
  fl_set_input(obj, "0");
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, setting_cb,  3);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);

  jx = jx + 15 + 0.5 * dx;
  FLset_X =
    obj = fl_add_input(FL_INT_INPUT, jx, jy, dx * 0.50, dy - 3, "X");
  fl_set_input(obj, "0");
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, setting_cb,  4);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);

  jx = jx + 15 + 0.5 * dx;
  FLset_Y =
    obj = fl_add_input(FL_INT_INPUT, jx, jy, dx * 0.50, dy - 3, "Y");
  fl_set_input(obj, "0");
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, setting_cb,  5);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);

  jx = ix;
  jy = jy + dy;
  obj = fl_add_box(FL_NO_BOX, jx, jy, dx / 2, dy - 3, "CMD:");
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);

  jx = jx + dx / 2;
  FLcmd_cmd =         //color of cmd:     undef / ok / executing / unknown
    obj = fl_add_box(FL_BORDER_BOX, jx, jy, dx / 2, dy - 3, " ---");
  fl_set_object_color(obj, 404, 414);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  jx = jx + dx / 2;
  FLcmd_time =        //color of time: CC_cmd:  undef / ok / busy / error
    obj = fl_add_box(FL_BORDER_BOX, jx + 3, jy, dx - 4, dy - 3, "--:--:--");
  fl_set_object_color(obj, 404, 414);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  jx = ix;
  jy = jy + dy;
  obj = fl_add_box(FL_NO_BOX, jx, jy, 38, dy, "ToCC");
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);

  jx = jx + 39;
  jy = jy + 2;
  FLout_info =        //color of time: CC_in:  undef / ok / warn / err
    obj = fl_add_box(FL_BORDER_BOX, jx, jy, dx * 2 - 40, dy - 3, "---|---|---|---");
  fl_set_object_color(obj, 404, 414);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  jx = ix;
  jy = jy + dy;
  fl_end_group();
}

//--------------------------------------------------------------------

void gen_info(int ix, int iy, int dx, int dy)
{
  FLinfo = fl_add_box(FL_FRAME_BOX, ix, iy, dx, dy, glob_info);
  fl_set_object_lsize(FLinfo, FL_NORMAL_SIZE);
  fl_set_object_lstyle(FLinfo, FL_FIXEDBOLD_STYLE);
}

//--------------------------------------------------------------------

void gen_mode(int ix, int iy, int dx, int dy)
{
  int jx, jy;
  FL_OBJECT *obj;

  jx = ix;
  jy = iy;
  dx = dx * 1.5;

  FLmod_grp = fl_bgn_group();

  /*FLmodA =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, " AUTO ");
  fl_set_object_callback(obj, mode_cb, MODE_AUTO);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);*/

  //jx  = jx + dx;
  FLmodO =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, " OPER ");
  fl_set_object_callback(obj, mode_cb, MODE_OPER);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  /*jx  = jx + dx;
  FLmodX =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, " EXPERT");
  fl_set_object_callback(obj, mode_cb, MODE_EXPT);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);*/

  jx  = jx + dx;
  FLmodM =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "MANUAL");
  fl_set_object_callback(obj, mode_cb, MODE_MANU);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jx  = jx + 2.5 * dx;
  FLread_lut =
    obj = fl_add_button(FL_NORMAL_BUTTON, jx, jy, dx, dy, "load LUT");
  fl_set_object_callback(obj, lut_cb, 0);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  fl_end_group();
}

//--------------------------------------------------------------------

void gen_panel(int jx, int jy, int dxx, int dyy)
{
  long i, j, n, ad;
  long dx, dy, dxy, ix, iy, ip, jp;
  char str[30];
  FL_OBJECT *obj;

  g_all = 0;  // count number of panels ...
  g_nin = 0;  // count number of panels ...

  ix = jx;
  iy = jy + 16 * dyy;

  for (j = 0; j < 17; j++) {              // single panels select and status
    for (i = 0;  i < 17; i++) {
      n =  i * 100 + j;
      if (panel[i][j].pan_stat > STAT_NOT) {
        if (panel[i][j].pan_stat == STAT_NIN) {
          g_nin++;
          g_i = i;
          g_j = j;
        }

        sprintf(str, "%+2d %+2d\n \n%2d%2d%2d", i - 8, j - 8,
                panel[i][j].port[ panel[i][j].portflg[2] ][2] + 1, panel[i][j].cpos[2], panel[i][j].bpos[2]);
        FLpanel[i][j] = fl_add_button(FL_PUSH_BUTTON, ix + dxx * i, iy - dyy * j, dxx, dyy, str);
        fl_set_object_lsize(FLpanel[i][j], FL_SMALL_SIZE);
        fl_set_object_lstyle(FLpanel[i][j], FL_FIXEDBOLD_STYLE);
        fl_set_object_callback(FLpanel[i][j], push_pan, n);
        strncpy(p_but[i][j].str, str, 30);
      }
    }
  }
  g_all = g_nin;

  obj = fl_add_clock(FL_DIGITAL_CLOCK, jx + 7 * dxx, jy + 8 * dyy, 2 * dxx, dyy / 2., " ");
  fl_set_object_color(obj,   FL_BUTTON_COL1, FL_BLACK);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_boxtype(obj, FL_FLAT_BOX);

  FL_durat =
    obj = fl_add_box(FL_FLAT_BOX, jx + 7 * dxx, jy + 8.5 * dyy, 2 * dxx, dyy / 2., "initializing");
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  FL_scroll =
    obj = fl_add_slider(FL_HOR_FILL_SLIDER, jx + 7 * dxx, jy + 8.5 * dyy, 2 * dxx, dyy / 2., " ");
  fl_set_object_color(obj, FL_YELLOW, FL_BLUE);
  fl_hide_object(obj);

  dx = dxx * 1.30;                       // status-group select
  dy = dyy * 0.70;

  ix = jx + 17 * dxx - 1 * dx;
  iy = jy;

  FLselect1_grp = fl_bgn_group();             //selector buttons to be 'deactivated'
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, " OK ");
  fl_set_object_color(obj, 110 + STAT_OK_, BUSY_COL);
  fl_set_object_callback(obj, push_pan, 10000 + STAT_OK_);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  iy  = iy + dy;
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, " ERR");
  fl_set_object_color(obj, 110 + STAT_ERR, BUSY_COL);
  fl_set_object_callback(obj, push_pan, 10000 + STAT_ERR);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  iy  = iy + dy;
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, "COMM");
  fl_set_object_color(obj, 110 + STAT_COM, BUSY_COL);
  fl_set_object_callback(obj, push_pan, 10000 + STAT_COM);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  iy  = iy + dy;
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, "UNDF");
  fl_set_object_color(obj, 110 + STAT_NIN, BUSY_COL);
  fl_set_object_callback(obj, push_pan, 10000 + STAT_NIN);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_end_group();

  FLselect2_grp = fl_bgn_group();             //selector buttons to be 'hidden'
  ix = jx + 17 * dxx - 2 * dx;
  iy = jy;
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, " OnLas ");
  fl_set_object_callback(obj, push_pan, 30000 + LAS_ON);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  iy  = iy + dy;
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, " OffLas ");
  fl_set_object_callback(obj, push_pan, 30000 + LAS_OFF);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  ix = jx + 17 * dxx - 3 * dx;
  iy = jy;
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, " ALL    ");
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_callback(obj, push_pan, 99999);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  dx = dxx * 0.55 * 1.5;                // chain-group select
  dy = dyy * 0.50;
  ix = jx;
  iy = jy + 1;
  for (jp = 0; jp < 4; jp++) {
    ip = jp;
    sprintf(str, "%1d", ip + 1);
    obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, str);
    fl_set_object_callback(obj, push_pan, 20000 + ip);
    fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
    sprintf(str, "%1d", ip + 5);
    obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy + dy, dx, dy, str);
    fl_set_object_callback(obj, push_pan, 20000 + ip + 4); //HA. 16.11.11
    fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
    ix = ix + dx;
  }

  iy = iy + 2 * dy;
  dx = dxx * 0.70;                       // Panel-group select
  ix = jx;
  for (ip = 1; ip < 5; ip++) {
    sprintf(str, "PG%1d", ip);
    obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, str);
    fl_set_object_callback(obj, push_pan, 40000 + ip);
    fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
    sprintf(str, "PG%1d", ip + 4);
    obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy + dy, dx, dy, str);
    fl_set_object_callback(obj, push_pan, 40000 + ip + 4);
    fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
    ix = ix + dx;
  }

  iy = iy + 2 * dy + 1;
  dx = dxx * 0.9;
  ix = jx;
  sprintf(str, "Glas");
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy, dx, dy, str);
  fl_set_object_callback(obj, push_pan, 50000);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  sprintf(str, "Alu");
  obj = fl_add_button(FL_NORMAL_BUTTON, ix + dx, iy, dx, dy, str);
  fl_set_object_callback(obj, push_pan, 50001);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  sprintf(str, "Inv");
  obj = fl_add_button(FL_NORMAL_BUTTON, ix, iy + dy, dx, dy, str);
  fl_set_object_callback(obj, push_pan, 70000);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  sprintf(str, "Chess");
  obj = fl_add_button(FL_NORMAL_BUTTON, ix+dx, iy + dy, dxx, dy, str);
  fl_set_object_callback(obj, push_pan, 60000);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  fl_end_group();
}

//--------------------------------------------------------------------

void temp_tm(int id, void *jtmp)
{
  char   xtmp[100];
  float  temp;

  temp = GetTemperature();
  sprintf(xtmp, "Temp=%5.1fC", temp);

  fl_set_object_label(FLsbig_temp, xtmp);
  TMtemp = fl_add_timeout(30000, temp_tm, NULL); //and reschedule myself at 30 sec
}

//--------------------------------------------------------------------

void end_distth(int i)
{
// set disto-window according to power status

  if (power[PWR_DIST].actual == 1) {
    fl_set_button(FLdist_on, 1);
    fl_activate_object(FLdisto);
    fl_set_object_lcol(FLdisto, 900);
  }
  else {
    fl_set_button(FLdist_on, 0);
    fl_deactivate_object(FLdisto);
    fl_set_object_lcol(FLdisto, 901);
  }
  fl_set_object_color(FLdist_on, FL_BUTTON_COL1, ACTV_COL);
  g_dist_th = S_NULL_TH;
}

//--------------------------------------------------------------------

void end_sbigth(int i)
{
  int abs_sbig, status, ret;
  int activate = 0;
  int activat1 = 0;

//    clean up the sbig-thread
  ret = pthread_join(sbig_threads[0], (void **)&status);

  abs_sbig = abs(g_sbig_th);
  if (abs_sbig > S_FILT_TH) {             // thread was picture
    g_newpic = 1;
    upd_pixmap();
    activate = 1;
  }

  else if (abs_sbig == S_FILT_TH) {
    activate = 1;
  }

  else if (abs_sbig == S_INIT_TH) {       // thread was SBIG_on

    if (g_sbigon < 0) {                  // error while switching on SBIG
      activate = 0;
      activat1 = 1;
    }
    else {
      activate = 1;
      activat1 = 1;
      if (TMtemp == 0) TMtemp = fl_add_timeout(10 , temp_tm, NULL); //start reading and display temp
    }
  }
  else if (abs_sbig == S_STBY_TH) {       // thread was SBIG_standby
    if (TMtemp == 0) TMtemp = fl_add_timeout(1000, temp_tm, NULL); //start reading and display temp
    activat1 = 1;
  }
  else if (abs_sbig == S_CLOS_TH) {       // thread was SBIG_off
    activat1 = 1;
    g_sbig_th = S_NULL_TH;
    push_pwr(NULL, -PWR_SBIG);
  }

  if (FLsbig_th != NULL) {
    fl_set_button(FLsbig_th, 0);
    FLsbig_th = NULL;
  }
  if (FLsbig_sl != NULL) {
    printf("set color ok\n");
    fl_set_object_color(FLsbig_sl, FL_BUTTON_COL1, SELE_COL);
    FLsbig_sl = NULL;
  }


  if (activat1 > 0) {
    fl_activate_object(FLsbig_on);
    fl_set_object_lcol(FLsbig_on, 900);
    if (g_sbigon > 0)
      fl_set_object_color(FLsbig_on, FL_BUTTON_COL1, ACTV_COL);
    else
      fl_set_object_color(FLsbig_on, FL_BUTTON_COL1, FL_RED);
  }
  if (activate > 0) {
    fl_activate_object(FLsbig_cmd);
    fl_activate_object(FLfilt_cmd);
    fl_set_object_lcol(FLsbig_cmd, 900);
    fl_set_object_lcol(FLfilt_cmd, 900);
  }
  g_sbig_th = S_NULL_TH;
  fl_hide_object(FLsbig_scrol);
}

//--------------------------------------------------------------------

void repo_cc(FL_OBJECT *ob, long n)
{
  char xstr[20];
  int col;

  col = 400;
  if (global_mode == MODE_MANU && n < 0) col = 404;
  else if (n == -1) col = 402;
  else if (n == -2) col = 403;

  fl_set_object_color(FLin_time, col, col + 10);

  if (n == -1) return;

  fl_gettime(&g_ccisec, &g_cciusec);

  if (n == -2) {
    g_ccstat = 3;
    sprintf(lstr, "no report from CC");
    if (global_mode != MODE_MANU) put_logfile(LOG_ERR, 0, lstr);
    return;
  }

  g_cc_tim0 = g_cc_time;
  g_ccstat = 0;

  sprintf(xstr, "%02d:%02d:%02d", g_cc_time / 10000, (g_cc_time / 100) % 100, g_cc_time % 100);
  fl_set_object_label(FLin_time, xstr);

  sprintf(xstr, "%d", (int)g_cc_zd);
  fl_set_object_label(FLin_Zd, xstr);
  //check if zenith from CC and set zenith agree within 5 deg
  if((int)g_cc_zd < global_zenith - 5 || (int)g_cc_zd > global_zenith + 5)
    fl_set_object_color(FLin_Zd, FL_RED, FL_RED);
  else
    fl_set_object_color(FLin_Zd, FL_GREEN, FL_GREEN);

  sprintf(xstr, "%d", (int)g_cc_az);
  fl_set_object_label(FLin_Az, xstr);
  fl_set_object_color(FLin_Az, FL_GREEN, FL_GREEN);
}

//--------------------------------------------------------------------
void exec_cc(FL_OBJECT *ob, long n)
{
  char xstr[20];
  int col, cmd = -1;
  FL_OBJECT *obj;

  g_ccstat = 1;
  if (global_mode == MODE_MANU) {                 //skip command ....
    g_cc_cmd = 0;
    sprintf(lstr, "Command skipped... %d %d", n, g_cc_cmd);
    put_logfile(LOG_ERR, 0, lstr);
    return;
  }

  sprintf(lstr, "Exec CC Command %d %d", n, g_cc_cmd);
  put_logfile(LOG_DB1, 0, lstr);

  if (n > 200) {            //do something with SBIG
    if (g_sbig_th > 0) {
      sprintf(lstr, "REJECT CC Command %d %d : SBIG busy", n, g_cc_cmd);
      put_logfile(LOG_ERR, 0, lstr);
      g_cc_cmd = 0;
      return;
    }
    g_cc_cmd = -n;

    if (n == 201) {
      g_sbig_temp = g_cc_temp;
      if (g_sbigon == 1) {
        sbig_cb(NULL, -1);        //already switched on, only set temp
      }
      else {
        sbigonoff_cb(NULL, 1);    //try to switch on
      }
    }
    else if (n == 202) {
      sbigonoff_cb(NULL, -1);     //try to switch of
    }
    else if (n == 203) {
      sbig_cb(NULL, g_cc_filt);   //set filterwheel
    }
    else if (n == 211) {
      sbig_cb(NULL, 2111);        //start takeing pict frame
    }
    else if (n == 212) {
      sbig_cb(NULL, 2112);        //start takeing dark frame
    }
  }
  else if (n > 100) {
    if      (n == 101) {
      printf(" TPTON\n");
    }
    else if (n == 102) {
      printf(" TPOFF\n");
    }
    else if (n == 111) {
      printf(" TPICT %s\n", g_cc_name);
    }
    else                 {
      printf(" UNKNN\n");
    }
    g_cc_cmd = 0;   // claim we have finished the command
    sprintf(lstr, "pseudo finished Command %d %d", n, g_cc_cmd);
    put_logfile(LOG_DB1, 0, lstr);
  }
  else if (global_action > 0) {                    //there is something going on ....
    if (n == 11 || n == 21) {
      g_cc_cmd = -n;
      break_cb(NULL, 0);
      fl_set_object_color(FLcmd_time, FL_MAGENTA, FL_MAGENTA);
      if (n == 21) {
        sleep(1);
        exit_cb(NULL, 1);
      }
      g_cc_cmd = 0;
    }
    else {
      g_cc_cmd = 0;                            //delay operation for some time
      sprintf(lstr, "busy, cannot execute command yet, will try later%d", n);
      put_logfile(LOG_DB1, 0, lstr);
      usleep(100000);                           //avoid too dense loop
      g_cc_cmd = n;
    }
  }
  else {                                            //nothing going on ==> execute new command
    g_cc_cmd = -n;
    sprintf(lstr, "start executing  Command %d %d", n, g_cc_cmd);
    put_logfile(LOG_DB1, 0, lstr);
    col = 400;
    if (n == 11) {
      col = -1;  //    BREAK 'useless'
    }
    else if (n == 21) {
      exit_cb(NULL, 1);  // shutdown ...
      col = -1;
    }
    else if (n == 1) {
      obj = FL_actIni;
      cmd = GUIcmd_INIT;
      sprintf(xstr, "INIT");
    }
    else if (n == 2) {
      obj = FL_actInif;
      cmd = GUIcmd_INFF;
      sprintf(xstr, "INFF");
    }
    else if (n == 3) {
      sprintf(xstr, "%d", global_zenith);
      fl_set_input(FLset_Zd, xstr);
      fl_set_object_color(FLin_Zd, FL_RED, FL_RED);
      sprintf(xstr, "%d", global_azimut);
      fl_set_input(FLset_Az, xstr);
      sprintf(xstr, "%d", global_foclen);
      fl_set_input(FLset_Foc, xstr);
      obj = FL_actAdj;
      cmd =  GUIcmd_ADJS;
      sprintf(xstr, "ADJS");
    }
    else {
      g_cc_cmd = 0;
      g_ccstat = 2;
      sprintf(xstr, "ILLEG", n);
      col = 403;
    }

    if (col > 0) {
      fl_set_object_label(FLcmd_cmd, xstr);
      fl_set_object_color(FLcmd_cmd, col, col + 10);
      sprintf(xstr, "%02d:%02d:%02d", g_cc_timc / 10000, (g_cc_timc / 100) % 100, g_cc_timc % 100);
      fl_set_object_label(FLcmd_time, xstr);
      sprintf(lstr, "executing  Command %d %d", n, cmd);
      put_logfile(LOG_DB1, 0, lstr);
      fl_set_button(obj, 1);   //set corresponding action-button
      action_cb(obj,  cmd);
      fl_set_button(obj, 0);   //deselect action-button
    }
  }
}

//--------------------------------------------------------------------
int chk_adj()
{
  int i, j, jz, n, di, dj;
  jz = global_zenith + 100;
  n  = 0;

  shiftfoc(global_foclen, global_xoff, global_yoff, -99);

  for (i = 0; i < 17; i++)
    for (j = 0; j < 17; j++)
      if (p_but[i][j].select != 0) {
        di = panel[i][j].lut[0][jz][0] + panel[i][j].mov_dab[0] - panel[i][j].act_mot[0];
        dj = panel[i][j].lut[0][jz][1] + panel[i][j].mov_dab[1] - panel[i][j].act_mot[1];
        if (di * di + dj * dj >= 50) n++;
      }
  return n;
}

//--------------------------------------------------------------------

int AMC_idle(XEvent *iq, void *jq)
{
  int ret, cmd, n, m, npwr, nadj;
  FL_OBJECT *obj;

  int i, j, k;
  int first = 0;

  fl_gettime(&isec0, &iusec0);

  if (g_initgui == 0) {   //have to do some initializations ....
    sprintf(lstr, "-------I-N-I------------ %d --", g_initgui);
    put_logfile(LOG_DB1,   0, lstr);
    g_initgui = -1;
    sprintf(lstr, "initializing ...");
    put_logfile(LOG_PWR, 0, lstr);
    g_nin = 999;
    g_err = g_mov = g_ok = 0;
    g_zd0 = g_az0 = g_fc0 = g_zd1 = g_az1 = g_fc1 = -1;
    g_cc_tosec = g_cc_sec = g_ccisec = g_tim0x = g_tim1x = isec0 - 99999;
    g_initgui = 1;
  }
  else if (g_initgui == 1) {
    sprintf(lstr, "-------I-N-I------------ %d --", g_initgui);
    put_logfile(LOG_DB1,   0, lstr);
    g_initgui = -2;
    mode_cb(NULL, -9);  //deactivate all action-buttons

    obj = FL_lasoff;
    cmd = GUIcmd_LOFF; //check if we can comunicate ...
    fl_set_button(obj, 1);   //set corresponding action-button
    action_cb(obj,  cmd);   //this is done in own thread; it will set g_initgui=+2 when done
    fl_set_button(obj, 0);   //deselect action-button
  }
  else if (g_initgui == 2) {
    sprintf(lstr, "-------I-N-I------------ %d --", g_initgui);
    put_logfile(LOG_DB1,   0, lstr);
    g_initgui = -3;
    int qtot = 0, qtot1 = 0;

    for(i = 0; i < 17; i++)
      for(j = 0; j < 17; j++) {
        if (panel[i][j].err_cnt[0] > 0) qtot1++;
        if (panel[i][j].err_cnt[1] > 0) qtot1++;
        if (panel[i][j].err_cnt[2] > 0) qtot1 += 2;
        for(k = 0; k < 3; k++) {
          if (panel[i][j].err_cnt[k] > 0) printf("-> %d %d %d %d\n", i, j, k, panel[i][j].err_cnt[k]);
        }
      }

    printf("-----------------------------------\n");
    printf("errors: %d -> %d\n", qtot, qtot1);
    printf("-----------------------------------\n");

    if (qtot1 > 10) {  //at least 10 bad conections ... try to power cycle
      fl_add_timeout(5000, timeout_yesno, 0);
      ret = fl_show_question("Seem to be a Communication Problem.\nDo you want me to PowerCycle ?\n(you have 5 seconds to press NO)", 1);
      if (ret == 1) {  //do power cycle
        double xcmd = 1, ycmd = 300;
        fl_set_slider_bounds(FL_scroll, 0., ycmd + 1.);
        fl_set_slider_value(FL_scroll, xcmd);
        fl_show_object(FL_scroll);
        sprintf(lstr, "Switchin AMC power off and wait 30 seconds");
        put_logfile(LOG_SVR, 0, lstr);
        push_pwr(NULL, -PWR_AMC);
        for (k = 0; k < ycmd; k++) {
          xcmd = xcmd + 1;
          usleep(100000);
          fl_set_slider_value(FL_scroll, xcmd);
          fl_show_object(FL_scroll);
        }
        g_initgui = 3; // --> power on again
      }
    }
    if (g_initgui < 0) { // power seems ok; initialize ....
      sprintf(lstr, "-------I-N-I------------ %d --", g_initgui);
      put_logfile(LOG_DB1,   0, lstr);
      g_initgui = -4;
      // Init Fast
      obj = FL_actInif;
      cmd = GUIcmd_INIF;
      fl_set_button(obj, 1);   //set corresponding action-button
      fl_set_button(FL_actCen, 1);   //set corresponding action-button
      action_cb(obj,  cmd);   //this is done in own thread; it will set g_initgui=+2 when done
      fl_set_button(obj, 0);   //deselect action-button
      fl_set_button(FL_actCen, 0);   //set corresponding action-button
      // Adjust
      obj = FL_actAdj;
      cmd = GUIcmd_ADJS;
      fl_set_button(obj, 1);   //set corresponding action-button
      action_cb(obj,  cmd);   //this is done in own thread; it will set g_initgui=+2 when done
      fl_set_button(obj, 0);   //deselect action-button
    }
  }
  else if (g_initgui == 3) {   //here we switch power on again
    //and mark all panels as undef
    sprintf(lstr, "-------I-N-I------------ %d --", g_initgui);
    put_logfile(LOG_DB1,   0, lstr);
    g_initgui = -4;
    sprintf(lstr, "Switchin AMC power on");
    put_logfile(LOG_SVR, 0, lstr);
    push_pwr(NULL, PWR_AMC);
    usleep(100000);

    g_err = g_nin = 0;

    for(i = 0; i < 17; i++)
      for(j = 0; j < 17; j++) {
        panel[i][j].err_cnt[0] =
          panel[i][j].err_cnt[1] =
            panel[i][j].err_cnt[2] = 0;

        if (panel[i][j].pan_stat > STAT_DIS) {
          panel[i][j].pan_stat = STAT_NIN;
          panel[i][j].laser    = LAS_UDF;

          panel[i][j].act_mot[0] =
            panel[i][j].act_mot[1] = 7777;
          panel[i][j].pc_mot[0] =
            panel[i][j].pc_mot[1] = -7777;
          g_nin++;
        }
      }
    upd_all_pan(0);
    usleep(100000);
    g_nin = 333;
    global_break = 0;
    g_initgui = 4;

  }
  else if (g_initgui == 4) {
    sprintf(lstr, "-------I-N-I------------ %d --", g_initgui);
    put_logfile(LOG_DB1,   0, lstr);
    g_initgui = -5;
    if ((g_err + g_nin) > 0) {
      mode_cb(NULL, -9);  //deactivate all action-buttons
      fl_add_timeout(5000, timeout_yesno, 0);
      ret = fl_show_question("Do you want to INITialize \n undefined Panels?\n(you have 5 seconds to press NO)", 1);
      if (ret == 1) {
        // select only 'ERR' and 'UNDF' state panels
        push_pan(NULL, -90000);          //unselect all
        push_pan(NULL, 10000 + STAT_NIN); //select undef
        push_pan(NULL, 10000 + STAT_ERR); //select err
        AMC_check_gui(0, -2.);
        // Init Fast
        obj = FL_actInif;
        cmd = GUIcmd_INIF;
        fl_set_button(obj, 1);   //set corresponding action-button
        action_cb(obj,  cmd);   //this is done in own thread; it will set g_initgui=+2 when done
        fl_set_button(obj, 0);   //deselect action-button
        // Adjust
        obj = FL_actAdj;
        cmd = GUIcmd_ADJS;
        fl_set_button(obj, 1);   //set corresponding action-button
        action_cb(obj,  cmd);   //this is done in own thread; it will set g_initgui=+2 when done
        fl_set_button(obj, 0);   //deselect action-button
      }
      else g_initgui = 5;
    }
    else g_initgui = 5;
  }
  else if (g_initgui == 5) {
    sprintf(lstr, "-------I-N-I------------ %d --", g_initgui);
    put_logfile(LOG_DB1,   0, lstr);
    mode_cb(NULL, MODE_OPER);       //set default mode
    push_pan(NULL, 90000);           //select all panels
    g_initgui = 999;
  }
  else if (g_initgui > 99 && global_action < 0) {
    if (g_cc_cmd > 0)   exec_cc(NULL, g_cc_cmd);                 // execute command from CC
    else if (g_cc_utim > g_updtime && g_drvstat == 4) {            //execute autoadjust ...
      g_updtime = g_cc_utim + 10;
      nadj = chk_adj();
      if (nadj > 5 && global_autoAdj > 0) {
        sprintf(lstr, "execute autoAdj (%d panels)", nadj);
        put_logfile(LOG_DB5, 0, lstr);
        obj = FL_actAdj;
        cmd = GUIcmd_ADJS;
        fl_set_button(obj, 1);   //set corresponding action-button
        action_cb(obj,  cmd);
        fl_set_button(obj, 0);   //deselect action-button
      }
      else if (nadj > 5) {
        sprintf(lstr, "propose to do autoAdj (%d panels)", nadj);
        put_logfile(LOG_DB5, 0, lstr);
        g_updtime += 20;
      }
    }
  }

  if (g_cc_time != g_cc_tim0)
    repo_cc(NULL, 0); // show report status

  if (g_ccstat < 3) {
    if (abs(isec0 - g_ccisec) > 30)
      repo_cc(NULL, -2); //set error
    else if (abs(isec0 - g_ccisec) > 15)
      repo_cc(NULL, -1); //set warning
  }

  if (g_cc_tosec < (isec0 - 10) && g_cc_tostat == 0) {
    sprintf(lstr, "Communication to CC seems stuck ...");
    put_logfile(LOG_ERR, 0, lstr);
    if (global_mode == MODE_MANU)
      fl_set_object_color(FLout_info, 404, 404);
    else
      fl_set_object_color(FLout_info, FL_RED, FL_RED);
    g_cc_tostat = 1;
  }

  if (g_dist_th < S_NULL_TH)
    end_distth(0);    // dist thread has finished

  if (g_sbig_th < S_NULL_TH)
    end_sbigth(0);    // sbig thread has finished
  else if (g_sbig_th > S_NULL_TH) {             // sbig thread is active ==> update scrollbar
    fl_set_slider_value(FLsbig_scrol, 1. - g_sbig_scrol);
    fl_show_object(FLsbig_scrol);
  }

  if (g_pwr_th < 0) {
    g_pwr_th = 0;
  }
  else if (g_pwr_th == 0 && (g_pwr_req > 0 || g_pwr_qry > 0 || isec0 > g_pwr_slp)) {
    g_pwr_th = 1;
  }

  if (g_numstr >= 0)
    upd_logout(0);

  return 0;
}

//--------------------------------------------------------------------

void gen_logfil(int ix, int iy, int dx, int dy)
{
  int i, j, istat;
  FL_OBJECT *obj;

  FLlog_short =
    obj = fl_add_browser(FL_NORMAL_BROWSER, ix, iy, dx, dy, "");
  fl_set_object_boxtype(obj, FL_FRAME_BOX);
  fl_set_object_color(obj,   0, BUSY_COL);

  fl_hide_object(FLlog_short);

  FLlog_long =
    obj = fl_add_browser(FL_NORMAL_BROWSER, ix, iy, dx, dy + 500, "");
  fl_set_object_boxtype(obj, FL_FRAME_BOX);
  fl_set_object_color(obj,   0, BUSY_COL);


  sprintf(lstr, "Welcome to the MAGIC-II AMC program");
  put_logfile(LOG_DB1, 0, lstr);
}

//--------------------------------------------------------------------
int AMC_gui_init()
{
  int i, j, istat, ix, iy, dx, dy;
  int ret, cmd;
  FL_OBJECT *obj;

  p_sele.i = -1;
  p_sele.j = -1;

  for (j = 0; j < 17; j++)
    for (i = 0; i < 17; i++)
      if (panel[i][j].pan_stat != STAT_NOT) {
        istat = panel[i][j].pan_stat;
        p_but[i][j].status = -999;
        p_but[i][j].select = 0;
      }
      else {
        p_but[i][j].status = STAT_NOT;
      }
  fl_set_border_width(-3);
  fl_init();
  col_define();

  ix = 0;
  iy = 0;
  dx = 1265;
  dy = 960;

  fl_set_goodies_font(FL_FIXEDBOLD_STYLE, FL_LARGE_SIZE);
  form = fl_bgn_form(FL_FLAT_BOX, dx, dy);

  FLinp_dummy =
    obj = fl_add_input(FL_HIDDEN_INPUT, 10., 10., 10., 10., "");
  fl_set_object_callback(obj, push_dummy, 0);

  fl_add_box(FL_FRAME_BOX, 523, 252, 745, 1000, " "); //panel selector box
  gen_info(523, 182, 745,  72);       //info includes the box
  fl_add_box(FL_FRAME_BOX,  -5, 252, 530,  678, " "); //sbig box
  fl_add_box(FL_FRAME_BOX, 220, 777, 305,  153, " "); //sbig-cmd box
  fl_add_box(FL_FRAME_BOX,  -5, 928, 227, 1000, " "); //Distoguide box

  fl_add_box(FL_FRAME_BOX, 910,  -5, 500,  189, " "); //commands box
  fl_add_box(FL_FRAME_BOX, 523,  -5, 227,  189, " "); //power-cmd box

  gen_mode(5, 5, 70, 30);               //auto or manual
  gen_logfil(0, 39, 525, 215);

  gen_sbig(gpict_x0, gpict_y0, 512, 512);            //sbig area

  gen_CC(755, 5, 75, 29);                //CC-in / -out
  gen_pwr(530, 5, 43, 29);                //power buttons
  gen_actions(916, 5, 86, 29);              //commands
  gen_panel(530, 260, 43, 41);              //panels

  upd_act_pan(g_i, g_j, 999, 1);
  upd_all_pan(0);

  FLexit = fl_bgn_group();

  obj = fl_add_button(FL_PUSH_BUTTON, dx - 74, dy - 74, 69, 69, "EXIT");
  fl_set_object_lsize(obj, FL_LARGE_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_callback(obj, exit_cb, 1);

  fl_end_group();

  FLbreak = fl_add_button(FL_NORMAL_BUTTON, dx - 43 * 17, dy - 74 - 40, 69, 37, "BREAK");
  fl_set_object_color(FLbreak, FL_MAGENTA, BUSY_COL);
  fl_set_object_lsize(FLbreak, FL_LARGE_SIZE);
  fl_set_object_lstyle(FLbreak, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(FLbreak, break_cb, 0);
  fl_hide_object(FLbreak);

  global_action = -1;   //no action going on
  fl_end_form();

  fl_set_idle_callback(AMC_idle, 0);

//do show just some noise
  g_newpic = 1;
  upd_pixmap();

  fl_show_form(form, FL_PLACE_CENTER, FL_TRANSIENT, tstr);
  put_logfile(LOG_OK_, 0, tstr);

  push_pan (NULL, 90000);             //set all selected

  return 0;
}

//--------------------------------------------------------------------

void  draw_pixmap()
{
  img2->modified = 1;
  img2->type = FL_IMAGE_GRAY;
  flimage_display(img2, FL_ObjWin(cnv2));
  img1->modified = 1;
  img1->type = FL_IMAGE_GRAY;
  flimage_display(img1, FL_ObjWin(cnv1));
  image->modified = 1;
  image->type = FL_IMAGE_GRAY;
  flimage_display(image, FL_ObjWin(canvas));
}

//--------------------------------------------------------------------
void  upd_pixmap()      //scale picture in img_buffer, put into image->gray, display image->gray
{
  int i, j, k, isum;
  int kmin = 100000;
  int kmax = -100000;
  int kskip =      4; //skip few very bright/dim pixels
  int mode, dinv, inv;

  double label3[7]     = {7 * 0.};
  double label2[7]     = {0., 51., 102., 153., 204., 255., 99999};
  unsigned long    level1[256]   = {256 * 0};
  unsigned long    level4[256]   = {256 * 0};
  unsigned long    level5[128]   = {128 * 0};
  unsigned long    level0[66001] = {66001 * 0};        //contains translated gray
  unsigned long    levelA[66001] = {66001 * 0};        //contains #pixels per gray
  double scale, sc0, sc1;

  if (g_newpic == 0 && g_viewmode == g_viewold) return; //nothing to do
  g_viewold = g_viewmode;

  mode =  g_viewmode % 10;
  dinv = (g_viewmode / 10) % 10;
  inv  = (g_viewmode / 100) % 10;

// find min and max value and density
  if (inv != 0) {
    levelA[65535 - 100] = 1;
    for (i = g_xmin; i < g_xmax; i++)
      for (j = g_ymin; j < g_ymax; j++)
        levelA[ 65535 - img_buffer[j][i] ]++;
  }
  else {
    levelA[100] = 1;
    for (i = g_xmin; i < g_xmax; i++)
      for (j = g_ymin; j < g_ymax; j++)
        levelA[ img_buffer[j][i] ]++;
  }

  level0[0] = levelA[0];
  for (i = 1; i < 66000; i++)
    level0[i] = level0[i - 1] + levelA[i];

  kmin = 0;
  while (level0[kmin] <= kskip && kmin < 66000) kmin++;

  kmax = 65997;
  while (level0[kmax] >= level0[65999] - kskip && kmax > kmin) kmax--;
  kmax += 2;
  scale = kmax - kmin;

// scale it:
  if (mode == V_HIST) {
    scale = level0[65998] / 256.;
    k  = 0;
    j  = 1;
    sc0 = scale + 0.5;
    for (i = kmin; i < kmax; i++) {
      while (level0[i] >= sc0) {
        if (k == label2[j]) {
          label3[j] = i;
          j++;
        }
        k++;
        sc0 = (k + 1) * scale + 0.5;
      }
      level0[i] = k;
    }
    label3[0] = kmin;
    label3[5] = kmax;
  }
  else if (mode == V_LOG) {
    if (scale < 2) scale = 2;
    scale = 255. / log10(scale);
    for (i = kmin; i < kmax; i++) level0[i] = log10((float) i - kmin) * scale;
    for (k = 0; k < 6; k++)       label3[k] = pow(10., label2[k] / scale) + kmin;
  }
  else if (mode == V_SQRT) {
    if (scale < 1) scale = 1;
    scale = 255. / sqrt(scale);
    for (i = kmin; i < kmax; i++) level0[i] = sqrt((float) i - kmin) * scale;
    for (k = 0; k < 6; k++)       label3[k] = pow(label2[k] / scale, 2.) + kmin;
  }
  else if (mode == V_SQAR) {
    if (scale < 1) scale = 1;
    scale = 255. / (scale * scale);
    for (i = kmin; i < kmax; i++) level0[i] = (double)(i - kmin) * (double)(i - kmin) * scale;
    for (k = 0; k < 6; k++)       label3[k] = sqrt(label2[k] / scale) + kmin;
  }
  else {
    if (scale < 1.) scale = 1.;
    scale = 255. / scale;
    for (i = kmin; i < kmax; i++) level0[i] = (i - kmin) * scale;
    for (k = 0; k < 6; k++)       label3[k] = label2[k] / scale + kmin;
  }

  for (i = 0;    i <= kmin; i++) level0[i] =  0;
  for (i = kmax; i < 66000; i++) level0[i] = 255;

  if (inv != 0)
    for (i = 0; i < 512; i++)
      for (j = 0; j < 512; j++) {
        isum = 65535 - (img_buffer[2 * j  ][2 * i  ] +
                        img_buffer[2 * j + 1][2 * i  ] +
                        img_buffer[2 * j + 1][2 * i + 1] +
                        img_buffer[2 * j  ][2 * i + 1]) / 4;
        image->gray[j][i] = level0[ isum ];
        img_buff2[j][i] = isum;
      }
  else
    for (i = 0; i < 512; i++)
      for (j = 0; j < 512; j++) {
        isum = (img_buffer[2 * j  ][2 * i  ] +
                img_buffer[2 * j + 1][2 * i  ] +
                img_buffer[2 * j + 1][2 * i + 1] +
                img_buffer[2 * j  ][2 * i + 1]) / 4;
        image->gray[j][i] = level0[ isum ];
        img_buff2[j][i] = isum;
      }

//color-histogram:
  for (k = kmin; k < kmax; k++)
    level4[ level0[k] ] += levelA[k];
  i = 0;
  for (k = 0; k < 128; k++) {
    level5[k] = level4[2 * k] + level4[2 * k + 1];
    if (level5[k] > i) i = level5[k];
  }
  scale = 25. / i;
  for (i = 0; i < 128; i++) {
    k = level5[i] * scale;
    for (j = 0; j < k; j++) img2->gray[i][j] = 255;
    for (j = k; j < 25; j++) img2->gray[i][j] = 128;
  }

//do inversion if requested
  if (dinv != 0) {
    for (i = 0; i < C_WID / 2; i++)
      for (j = 0; j < C_HIG / 2; j++)
        image->gray[j][i] = 255 - image->gray[j][i];
    for (i = 0; i < 128; i++)
      for (j = 0; j < 25; j++)
        img1->gray[i][j] = 255 - (i * 2);
  }
  else
    for (i = 0; i < 128; i++)
      for (j = 0; j < 25; j++)
        img1->gray[i][j] = i * 2;

//and show the pictures
  img2->sx =   0;
  img2->sy =   0;
  img2->sw =  25;
  img2->sh = 128;
  img2->wx =   0;
  img2->wy =   0;

  img1->sx =   0;
  img1->sy =   0;
  img1->sw =  25;
  img1->sh = 128;
  img1->wx =   0;
  img1->wy =   0;

  for (k = 0; k < 6; k++) {
    sprintf(xtmp, "%5.0f", label3[k]);
    fl_set_object_label(FLlab[k], xtmp);
  }

  image->sx = 0;
  image->sy = 0;
  image->sw = C_WID / 2;
  image->sh = C_HIG / 2;

  image->wx = 0;
  image->wy = 0;

  draw_pixmap();
  g_newpic = 0;

}

//--------------------------------------------------------------------
int  canvas_motion()
{
  unsigned ix, iy, iv;
  char   xtmp[100];

  fl_get_form_mouse(form,  &xpos, &ypos, &keymask);
  ix = (xpos - gpict_x0);
  iy = (ypos - gpict_y0);
  iv = img_buff2[iy][ix];
  sprintf(xtmp, "X:%3d Y:%3d  V:%5d", ix * 2, iy * 2, iv);
  fl_set_object_label(FLval0, xtmp);
  return 0;
}
//--------------------------------------------------------------------
int  canvas_expose()
{
  draw_pixmap();   //expose
  return 0;
}
//--------------------------------------------------------------------
int  canvas_leave()
{
  sprintf(xtmp, "\n");
  fl_set_object_label(FLval0, xtmp);
  return 0;
}
//--------------------------------------------------------------------
int  canvas_enter()
{
  fl_set_cursor(FL_ObjWin(canvas), myXC_cursor);
  return 0;
}
//--------------------------------------------------------------------
int  canvas_button()
{
  return 0;
}
//--------------------------------------------------------------------
int  canvas_buttof()
{
  unsigned ix, iy, iv;
  double x, y;
  char   xtmp[100];

  fl_get_form_mouse(form,  &xpos, &ypos, &keymask);
  x = (xpos - gpict_x0);
  y = (ypos - gpict_y0);
  ix = (xpos - gpict_x0);
  iy = (ypos - gpict_y0);
  iv = img_buff2[iy][ix];
  sprintf(xtmp, "Marker:  X:%3d Y:%3d  V:%5d", ix * 2, iy * 2, iv);

  put_logfile(LOG_SBG, 0, xtmp);

  return 0;
}

//--------------------------------------------------------------------

void view_cb(FL_OBJECT *ob, long n)
{
  int i;
  int mode, dinv, inv;

  sprintf(lstr, "view button %d pressed", n);
  put_logfile(LOG_DB1, 0, lstr);

  g_viewold = g_viewmode;
  mode =  g_viewmode % 10;
  dinv = (g_viewmode / 10) % 10;
  inv  = (g_viewmode / 100) % 10;

  if (n < 0) {
    if (ob != NULL) i = fl_get_button(ob);
    else              i = 0;
    if (n == -1) dinv = i;
    else            inv = i;
  }
  else mode = n;

  g_viewmode = inv * 100 + dinv * 10 + mode;
  upd_pixmap();

}

//--------------------------------------------------------------------

void save_cb(FL_OBJECT *ob, long n)
{
  FILE* param;
  const char *s;
  char fname[LOGLEN], fnam0[LOGLEN], fnamt[LOGLEN];
  fitsfile *fptr;
  int status = 0, todo = 1, i = 0;
  long exposure, nelements, naxes[2] = {C_WID, C_HIG};
  long utime[5];
  int ii, jj, YY, MM;

  if (n > 0) {
    if(g_pict_filt == 0)
      sprintf(fnam0, "Dark");
    else
      sprintf(fnam0, g_cc_source);
    strncpy(&fnam0[0], (s = fl_show_input("Name: <source>_<special>", &fnam0[0])) ? s : "", 1000);
    if (s == NULL)
      return;

    for (i = 0; fnam0[i] != 0; i++)
      if ((fnam0[i] < 'a' || fnam0[i] > 'z')
          && (fnam0[i] < 'A' || fnam0[i] > 'Z')
          && (fnam0[i] < '0' || fnam0[i] > '9')
          &&  fnam0[i] != '+' && fnam0[i] != '-')
        fnam0[i] = '_';

    int expos = g_expos * 10;

    AMCtime(utime);
    YY = utime[1] / 10000 + 2000;
    MM = (utime[1] / 100) % 100;
    snprintf(fname, LOGLEN, "%s/%04d/%02d/M1_AMC_%06d_%06d_F%d_X%05d_Z%+03d_A%+04d_%s.fits",
             sbig_path, YY, MM, utime[1], utime[0], g_pict_filt, g_pict_expos, (int)pct_zenit, (int)pct_azim, fnam0);
    snprintf(fnamt, LOGLEN, "%s/%04d/%02d/M1_AMC_%06d_%06d_F%d_X%05d_Z%+03d_A%+04d_%s.motor",
             sbig_path, YY, MM, utime[1], utime[0], g_pict_filt, g_pict_expos, (int)pct_zenit, (int)pct_azim, fnam0);
  }
  else if (n < -10) { // SBIG images from SA
    snprintf(fname, LOGLEN, "%s.fits", g_cc_name);
    snprintf(fnamt, LOGLEN, "%s.motor", g_cc_name);
  }
  else if (n == -2) {
    if(g_pict_filt == 0)
      sprintf(fnam0, "Dark");
    else
      sprintf(fnam0, g_cc_source);

    for (i = 0; fnam0[i] != 0; i++)
      if ((fnam0[i] < 'a' || fnam0[i] > 'z')
          && (fnam0[i] < 'A' || fnam0[i] > 'Z')
          && (fnam0[i] < '0' || fnam0[i] > '9')
          &&  fnam0[i] != '+' && fnam0[i] != '-')
        fnam0[i] = '_';

    int expos = g_expos * 10;

    AMCtime(utime);
    YY = utime[1] / 10000 + 2000;
    MM = (utime[1] / 100) % 100;
    snprintf(fname, LOGLEN, "%s/%04d/%02d/M1_CYC_%06d_%06d_F%d_X%05d_Z%+03d_A%+04d_%s.fits",
             sbig_path, YY, MM, utime[1], utime[0], g_pict_filt, g_pict_expos, (int)pct_zenit, (int)pct_azim, fnam0);
    snprintf(fnamt, LOGLEN, "%s/%04d/%02d/M1_CYC_%06d_%06d_F%d_X%05d_Z%+03d_A%+04d_%s.motor",
             sbig_path, YY, MM, utime[1], utime[0], g_pict_filt, g_pict_expos, (int)pct_zenit, (int)pct_azim, fnam0);
  }
  else {
    snprintf(fname, LOGLEN, "%s.fits", a_sb_name);
    snprintf(fnamt, LOGLEN, "%s.motor", a_sb_name);
  }

  naxes[0] = C_WID;
  naxes[1] = C_HIG;

  nelements = naxes[0] * naxes[1];

// and store actuator settings when picture taken

  snprintf(lstr, LOGLEN, "saving %s %d", fnamt, nelements);
  put_logfile(LOG_SBG, 0, lstr);

  param = fopen(fnamt, "w");
  for (ii = 0; ii < 17; ii++)
    for (jj = 0; jj < 17; jj++)
      if (panel[ii][jj].sbg_stat > STAT_NOT) {
        sprintf(lstr, "(%2d,%2d) pc: %5d %5d; stat: %3d %3d",
                ii - 8, jj - 8, panel[ii][jj].pc_mot[0], panel[ii][jj].pc_mot[1],
                panel[ii][jj].sbg_stat, panel[ii][jj].pan_stat);
        fprintf(param, "%s\n", lstr);
      }

  snprintf(lstr, LOGLEN, "saved %s %d", fnamt, nelements);
  put_logfile(LOG_SBG, 0, lstr);

  snprintf(lstr, LOGLEN, "saving %s %d", fname, nelements);
  put_logfile(LOG_SBG, 0, lstr);

  fits_create_file(&fptr, fname, &status);
  fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);
  fits_write_img(fptr, TUSHORT, 1, nelements, img_buffer, &status);

  fits_update_key_str(fptr, "INSTRUME", "SBIG", "MAGIC-II Telescope AMC", &status);
  fits_update_key_dbl(fptr, "EXPTIME", pct_expos, -5, "[s] exposure time ", &status);
  fits_update_key_dbl(fptr, "Y_MIN",   pct_ymin, -5, "first line of pict. ", &status);
  fits_update_key_dbl(fptr, "Y_MAX",   pct_ymax, -5, "last  line of pict. ", &status);
  fits_update_key_dbl(fptr, "EXPTIME", pct_expos, -5, "[s] exposure time ", &status);
  fits_update_key_dbl(fptr, "TEMPERAT", pct_temp, -5, "[deg] temperature ", &status);
  fits_update_key_dbl(fptr, "ZENITH" , pct_zenit, -5, "[deg] zenith from drive", &status);
  fits_update_key_dbl(fptr, "AZIMUTH", pct_azim, -5, "[deg] azimuth from drive", &status);
  fits_update_key_dbl(fptr, "FOCLEN" , pct_foclen, -5, "[mm] focal length", &status);
  fits_update_key_str(fptr, "PANEL"  , pct_pangrp, "Panel (j,i) or Group 0 ... 9", &status);
  fits_update_key_str(fptr, "FILTER" , pct_filter, "Filter used", &status);
  fits_update_key_str(fptr, "IMAGETYP", pct_imgtyp, "Dark or Pict", &status);
  fits_update_key_str(fptr, "DATE"   , pct_fulldat, "YYYY MM SS hh:mm:ss [UTC]", &status);

  fits_close_file(fptr, &status);

  fits_report_error(param, status);
  fclose(param);

  snprintf(lstr, LOGLEN, "saved %s %d", fname, nelements);
  put_logfile(LOG_SBG, 0, lstr);

  g_pictcnt++;

}

//--------------------------------------------------------------------

void sbig_cb(FL_OBJECT *ob, long n)
{
  int i, j, m, ret, kx, ky, cc;
  double x;
  char* in[20], out[20], str[20];

  /* -1: set temp; -2: set exposure; -3: set number cycle images; -4: set filename
    1-5: set filter 1-5

     1xyz  : x 0=don't expose, 1=expose
             y 0=don't read    1=read
             z 1=pict, 2=dark, 3=noShutt

     2xyz  : same, but asked from CC
  */

  fl_set_focus_object(form,  FLinp_dummy);

  FLsbig_th = NULL;
  x = -99999;
  if (n == -1) {      // set temperature
    if (ob != NULL) {
      g_sbig_temp =
        x = atof(fl_get_input(ob));
    }
    else {
      sprintf(str, "%3.1f", g_sbig_temp);
      fl_set_input(FLtemp, str);
    }
    ret = SetCoolingOn(x);
    if (TMtemp == 0)
      TMtemp = fl_add_timeout(1000, temp_tm, NULL);  //timeout in msec
    sprintf(lstr, "set Temp: %d  %f", ret, x);
    put_logfile(LOG_SBG, 0, lstr);
  }

  else if (n == -2) { // set exposure
    g_expos =
      x = atof(fl_get_input(ob));
    sprintf(out, "Expo=%5.2fs", x);
    fl_set_object_label(FLsbig_expo, out);
    sprintf(lstr, "set exposure  %f", g_expos);
    put_logfile(LOG_SBG, 0, lstr);
  }

  else if (n == -3) { // set number of cycle images
    g_cycimg =
      j = atoi(fl_get_input(ob));
    sprintf(out, "CygImg=%4d", j);
    fl_set_object_label(FLsbig_cycimg, out);
    sprintf(lstr, "set number cycle images %d", g_cycimg);
    put_logfile(LOG_SBG, 0, lstr);
  }

  else if (n == -20) { // set ymin
    j = atoi(fl_get_input(ob));
    if (j < 0 || j > 1023) j = 0;
    g_ymin = j;
    sprintf(out, "%d", j);
    fl_set_input(ob, out);
    for (ky = 0; ky < g_ymin; ky++) {
      for (kx = 0; kx < 1024; kx++)
        img_buffer[ky][kx] = 100;
    }
    sprintf(lstr, "set Ymin: %d ", j);
    put_logfile(LOG_SBG, 0, lstr);
  }

  else if (n == -21) { // set ymax
    j = atoi(fl_get_input(ob));
    if (j < 0 || j > 1023) j = 1023;
    g_ymax = j;
    sprintf(out, "%d", j);
    fl_set_input(ob, out);
    for (ky = g_ymax + 1; ky < 1024; ky++)  {
      for (kx = 0; kx < 1024; kx++)
        img_buffer[ky][kx] = 100;
    }
    sprintf(lstr, "set Ymax: %d ", j);
    put_logfile(LOG_SBG, 0, lstr);
  }

  else if (n > 0 && n < 6) { // set filter
    g_pict_filt = g_filter = n;
    g_sbig_th = S_FILT_TH;
    fl_deactivate_object(FLsbig_cmd);
    fl_deactivate_object(FLfilt_cmd);
    fl_set_object_lcol(FLsbig_cmd, 901);
    fl_set_object_lcol(FLfilt_cmd, 901);
    if (ob != NULL) {
      fl_set_object_color(ob, FL_BUTTON_COL1, BUSY_COL);
      FLsbig_sl = ob;
    }
    else {
      FLsbig_sl = FLfilt[n];
      for (j = 1; j < 6; j++) fl_set_button(FLfilt[j], 0);
      fl_set_button(FLfilt[n], 1);
    }
    ret = pthread_create(&sbig_threads[0], NULL, Filter_th, NULL);
    sprintf(lstr, "set Filter: %d ", n);
    put_logfile(LOG_SBG, 0, lstr);
  }

  else if (n > 1000) { // take pictures
    sprintf(lstr, "take picture: %d ", n - 1000);
    put_logfile(LOG_SBG, 0, lstr);
    cc = 0;
    if (n > 2000) {
      cc = 1;
      g_expos = g_cc_expos / 100.;
      sprintf(str, "%3.1f", g_expos);
      fl_set_input(FLsbig_expo, str);
    }
    m = n % 1000;

    if (m > 110)      g_sb_mode = S_PICT_TH;
    else if (m > 100) g_sb_mode = S_EXPO_TH;
    else if (m >  10) g_sb_mode = S_READ_TH;
    else              g_sb_mode = S_NULL_TH;

    m = (n % 10);
    if (m == 1) {
      g_sb_shutt = SC_OPEN_SHUTTER;
      FLsbig_th = FL_pict;
      g_pict_filt = g_filter;
    }
    else if (m == 2) {
      g_sb_shutt = SC_CLOSE_SHUTTER;
      FLsbig_th = FL_dark;
      g_pict_filt = 0;
    }
    else if (m == 3) {
      g_sb_shutt = SC_LEAVE_SHUTTER;
      FLsbig_th = FL_pict;
      g_pict_filt = g_filter;
    }
    else {
      g_sb_mode  = S_NULL_TH;
    }

    if (g_sb_mode > S_NULL_TH && g_sbig_th == S_NULL_TH) {
      g_sbig_th = g_sb_mode;
      g_sbig_cc = cc;
      fl_deactivate_object(FLsbig_cmd);
      fl_deactivate_object(FLfilt_cmd);
      fl_set_object_lcol(FLsbig_cmd, 901);
      fl_set_object_lcol(FLfilt_cmd, 901);
      if (FLsbig_th != NULL)
        fl_set_button(FLsbig_th, 1);
      g_sb_first = 1;
      ret = pthread_create(&sbig_threads[0], NULL, TakePict_th, NULL);
    }
  }

  else if (n == 999) {
    long utime[5], utim1[5], utim2[5];
    int status;
    fl_set_button(FL_cycle, 1);   //set corresponding action-button

    if (g_cycimg > 0) {
      //loop until broken by operator or number of images reached
      sprintf(lstr, "take %d filter cycle images", g_cycimg);
      put_logfile(LOG_SBG, 0, lstr);
      int cf = 0;
      while (cf < g_cycimg) {
        //wait until SBIG status is idle or NULL
        if (g_sbig_th <= 0) {
          g_sb_mode = S_PICT_TH;
          g_sb_shutt = SC_OPEN_SHUTTER;
          FLsbig_th = FL_pict;
        }

        //cycle filter only if SBIG status is idle or NULL and last command was
        //not filter change
        if (g_sbig_th <= 0 && g_sbig_th != -S_FILT_TH) {
          //save image with CYC tag
          /*if (g_sbig_th == -S_PICT_TH) {
            save_cb(NULL, -2);
            cf++;
            //ask if the cycle should be interrupted
            if (cf < g_cycimg) {
              fl_add_timeout(5000, timeout_yesno, 0);
              int qstop = fl_show_question("Stop the filter cycle images?\n(you have 5 seconds to press YES)", 0);
              if (qstop == 1)
                break;
            }
          }*/

          if (g_filter == 3) g_filter = 4;
          else if (g_filter == 4) g_filter = 5;
          else if (g_filter == 5) g_filter = 3;
          else g_filter = 3; // start with blue, M2 starts with green
          g_pict_filt = g_filter;
          sbig_cb(NULL, g_filter);
        }

        //take the image only if SBIG status is idle or NULL and last command was
        //not take image
        if (g_sbig_th <= 0 && g_sbig_th != -S_PICT_TH) {
          g_sbig_th = g_sb_mode;
          g_sbig_cc = 0;
          fl_deactivate_object(FLsbig_cmd);
          fl_deactivate_object(FLfilt_cmd);
          fl_set_object_lcol(FLsbig_cmd, 901);
          fl_set_object_lcol(FLfilt_cmd, 901);
          if (FLsbig_th != NULL)
            fl_set_button(FLsbig_th, 1);
          g_sb_first = 1;
          ret = pthread_create(&sbig_threads[0], NULL, TakePict_th, NULL);

          AMCtime(utim1);
          utim1[2] = utim1[2] + (int)g_expos + 2;
          //wait until picture exposed ....
          do {
            usleep(100000);
            AMC_check_gui(0, -2.);
            AMCtime(utim2);
          }
          while (utim2[2] < utim1[2]);

          if (FLsbig_th != NULL)
            fl_set_button(FLsbig_th, 0);

          //save image with CYC tag
          save_cb(NULL, -2);
          cf++;
          //ask if the cycle should be interrupted
          if (cf < g_cycimg) {
            fl_add_timeout(5000, timeout_yesno, 0);
            int qstop = fl_show_question("Stop the filter cycle images?\n(you have 5 seconds to press YES)", 0);
            if (qstop == 1) {
              ret = pthread_join(sbig_threads[0], (void **)&status);
              break;
            }
          }

          ret = pthread_join(sbig_threads[0], (void **)&status);
        }
      }
    }
    else if (g_cycimg < 0) {
      sprintf(lstr, "take %d dark frames", -g_cycimg);
      put_logfile(LOG_SBG, 0, lstr);
      int df = 0;
      while (df > g_cycimg) {
        //wait until SBIG status is idle or NULL
        if (g_sbig_th <= 0) {
          g_sb_mode = S_PICT_TH;
          g_sb_shutt = SC_CLOSE_SHUTTER;
          FLsbig_th = FL_dark;
          g_pict_filt = 0;
        }

        //save dark frame with CYC tag if image is taken
        /*if (g_sbig_th == -S_PICT_TH && FLsbig_th == FL_dark) {
          save_cb(NULL, -2);
          df--;
          //ask if the cycle should be interrupted
          if (df > g_cycimg) {
            fl_add_timeout(5000, timeout_yesno, 0);
            int qstop = fl_show_question("Stop the filter cycle images?\n(you have 5 seconds to press YES)", 0);
            if (qstop == 1)
              break;
          }
        }*/

        //take the image only if SBIG status is idle or NULL and last command was
        //not take image
        if (g_sbig_th <= 0 && g_sbig_th != -S_PICT_TH) {
          g_sbig_th = g_sb_mode;
          g_sbig_cc = 0;
          fl_deactivate_object(FLsbig_cmd);
          fl_deactivate_object(FLfilt_cmd);
          fl_set_object_lcol(FLsbig_cmd, 901);
          fl_set_object_lcol(FLfilt_cmd, 901);
          if (FLsbig_th != NULL)
            fl_set_button(FLsbig_th, 1);
          g_sb_first = 1;
          ret = pthread_create(&sbig_threads[0], NULL, TakePict_th, NULL);

          AMCtime(utim1);
          utim1[2] = utim1[2] + (int)g_expos + 2;
          //wait until picture exposed ....
          do {
            usleep(100000);
            AMC_check_gui(0, -2.);
            AMCtime(utim2);
          }
          while (utim2[2] < utim1[2]);

          if (FLsbig_th != NULL)
            fl_set_button(FLsbig_th, 0);

          //save dark frame with CYC tag if image is taken
          save_cb(NULL, -2);
          df--;
          //ask if the cycle should be interrupted
          if (df > g_cycimg) {
            fl_add_timeout(5000, timeout_yesno, 0);
            int qstop = fl_show_question("Stop the filter cycle images?\n(you have 5 seconds to press YES)", 0);
            if (qstop == 1) {
              ret = pthread_join(sbig_threads[0], (void **)&status);
              break;
            }
          }

          ret = pthread_join(sbig_threads[0], (void **)&status);
        }
      }
    }

    fl_set_button(FL_cycle, 0);   //set corresponding action-button
    sprintf(lstr, "filter cycle finished");
    put_logfile(LOG_SBG, 0, lstr);
  }
}

//--------------------------------------------------------------------

void disto_cb(FL_OBJECT *ob, long m)
{
  int i, n, icol;
  char str[80];

  n = m;
  if (ob != NULL)
    if (n < 0)
      n = -fl_get_button(ob);

  i = AMC_exec_dist(n);   // -1: laser off // 0: laser on // 1: measure

  if (n > 0) {            //must display value and mark as laser off
    if (i < 16000 || i > 18000) {
      icol = FL_RED, i = -1;
    }
    else                      icol = FL_BUTTON_COL1;
    sprintf(str, "D=%5d", i);
    fl_set_object_color(FLdistance, icol, BUSY_COL);

    fl_set_object_label(FLdistance, str);
    fl_set_button(FLdistance, 0);
    fl_set_button(FLdistlas, 0);
  }

  sprintf(lstr, "DISTO button %d pressed ==> %d", n, i);
  put_logfile(LOG_DB1, 0, lstr);
  AMC_check_gui(0, -2.);
}

//--------------------------------------------------------------------

void sbigonoff_cb(FL_OBJECT *ob, long m)
{
  int i, n;    //n: -1 = off, +1 = on

  if (g_sbig_th > 0) {
    printf(" ??????\n");
    return;
  }

  n = m;
  if (ob == NULL)
    if (n > 0) fl_set_button(FLsbig_on, 1);
    else         fl_set_button(FLsbig_on, 0);
  else if (fl_get_button(ob) == 0) n = -1;
  else                            n = +1;

  if (n > 0) {
    sprintf(lstr, "SBIG on");
    put_logfile(LOG_SBG, 0, lstr);
  }
  else {
    sprintf(lstr, "SBIG off");
    put_logfile(LOG_SBG, 0, lstr);
  }

  fl_set_object_color(FLsbig_on, BUSY_COL, BUSY_COL);
  FLsbig_sl = ob;

  fl_deactivate_object(FLsbig_cmd);
  fl_deactivate_object(FLfilt_cmd);
  fl_deactivate_object(FLsbig_on);
  fl_set_object_lcol(FLsbig_cmd, 901);
  fl_set_object_lcol(FLfilt_cmd, 901);
  fl_set_object_lcol(FLsbig_on, 901);

  if (n < 0) {
//   switch SBIG off
    if (TMtemp != 0) {
      fl_remove_timeout(TMtemp);
      TMtemp = 0;
    }
    g_sbig_th = S_CLOS_TH;
    i = pthread_create(&sbig_threads[0], NULL, CloseST7_th, NULL);
    g_oldfilt = -5;

    fl_activate_object(FLpwr[PWR_SBIG]);
    fl_set_object_lcol(FLpwr[PWR_SBIG], 900);

    fl_hide_object(canvas);
    fl_hide_object(FLlog_short);
    fl_show_object(FLlog_long);

  }
  else {
//   switch SBIG on
    g_sbig_th = S_INIT_TH;
    i = pthread_create(&sbig_threads[0], NULL, InitST7_th, NULL);

    fl_deactivate_object(FLpwr[PWR_SBIG]);
    fl_set_object_lcol(FLpwr[PWR_SBIG], 901);

    fl_hide_object(FLlog_long);
    fl_show_object(canvas);
    fl_show_object(FLlog_short);
  }
}

//--------------------------------------------------------------------
void distonoff_cb(FL_OBJECT *ob, long m)
{
  int i, n;    //n: -1 = off, +1 = on

  if (g_sbig_th > 0) {
    printf(" ??????\n");
    return;
  }

  n = m;
  if (ob == NULL)
    if (n > 0) fl_set_button(FLdist_on, 1);
    else         fl_set_button(FLdist_on, 0);
  else if (fl_get_button(ob) == 0) n = -1;
  else                            n = +1;

  if (n > 0) {
    sprintf(lstr, "DIST on");
    put_logfile(LOG_DB1, 0, lstr);
  }
  else {
    sprintf(lstr, "DIST off");
    put_logfile(LOG_DB1, 0, lstr);
  }

  fl_set_object_color(FLdist_on, BUSY_COL, BUSY_COL);
  FLsbig_sl = ob;

  fl_deactivate_object(FLdisto);
  fl_set_object_lcol(FLdisto, 901);

  if (n < 0) {
//   switch DIST off
    g_dist_th = S_CLOS_TH;
    i = pthread_create(&dist_threads[0], NULL, CloseDST_th, NULL);
  }
  else {
//   switch DIST on
    g_dist_th = S_INIT_TH;
    i = pthread_create(&dist_threads[0], NULL, InitDST_th, NULL);
  }
}

//--------------------------------------------------------------------

void gen_disto(int ix, int iy, int dx, int dy)
{
  int jx, jy;
  FL_OBJECT *obj;


  jx = ix;
  jy = iy;

  jx  = jx + dx;
  obj = fl_add_box(FL_FLAT_BOX, jx, jy, dx, dy, "Disto:");
  fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  FLdist_on =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "On/Off");
  fl_set_object_callback(obj, distonoff_cb, 0);
  fl_set_object_color(obj, FL_BUTTON_COL1, ACTV_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  FLdisto = fl_bgn_group();
  jx  = jx + dx;
  FLdistlas =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "Laser");
  fl_set_object_callback(obj, disto_cb, -1);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  jx  = jx + dx;
  FLdistance =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx + 10, dy, "D=17000");
  fl_set_object_callback(obj, disto_cb, 1);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_end_group();

  fl_deactivate_object(FLdisto);
  fl_set_object_lcol(FLdisto, 901);
}

//--------------------------------------------------------------------

void gen_sbig(int ix, int iy, int dx, int dy)
{
  int k, i, j, jx, jy, jx1, jy1, jx2, jy2;
  char   str[10];
  FL_OBJECT *obj;


//temporary fill img_buffer
  for (i = 0; i < 1024; i++)
    for (j = 0; j < 1024; j++)
      img_buffer[i][j] = 200 + rand() % 65000;

//create image areas
  memset(&mysetup, 0, sizeof(mysetup));
  mysetup.delay = 1;
  mysetup.do_not_clear = 1;
  flimage_setup(&mysetup);

  image = flimage_alloc();
  image->type = FL_IMAGE_GRAY;
  image->w = C_WID / 2;
  image->h = C_HIG / 2;
  flimage_getmem(image);

  img1 = flimage_alloc();
  img1->type = FL_IMAGE_GRAY;
  img1->w = 25;
  img1->h = 128;
  flimage_getmem(img1);

  img2 = flimage_alloc();
  img2->type = FL_IMAGE_GRAY;
  img2->w = 25;
  img2->h = 128;
  flimage_getmem(img2);

  g_x0 = ix;
  g_y0 = iy;
  canvas   = fl_add_canvas(FL_NORMAL_CANVAS, g_x0, g_y0, dx, dy, " ");
  fl_add_canvas_handler(canvas, 12, canvas_expose, 0); //expose
  fl_add_canvas_handler(canvas,  8, canvas_leave, 0); //leave
  fl_add_canvas_handler(canvas,  7, canvas_enter, 0); //enter
  fl_add_canvas_handler(canvas,  6, canvas_motion, 0); //motion
  fl_add_canvas_handler(canvas,  5, canvas_buttof, 0); //button press
  fl_add_canvas_handler(canvas,  4, canvas_button, 0); //button release
  WINccd   = FL_ObjWin(canvas);
  fl_set_object_boxtype(canvas, FL_FLAT_BOX);
  fl_set_object_bw(canvas, -1);

  fl_hide_object(canvas);

  jx = ix + 40;
  jy = iy + dy + 5;

  FLval0 =
    obj = fl_add_box(FL_FLAT_BOX, jx + 150, jy, 1, 12, " ");
  fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy = jy + 15;
  for (k = 0; k < 6; k++) {
    FLlab[k] =
      obj = fl_add_box(FL_FLAT_BOX, jx, jy + k * 25 - 10, 1, 25, " ");
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  }
  jy = jy;

  cnv1     = fl_add_canvas(FL_NORMAL_CANVAS, jx, jy, 25, 128, " ");
  fl_set_object_boxtype(cnv1 , FL_FLAT_BOX);

  jx = jx + 28;
  cnv2     = fl_add_canvas(FL_NORMAL_CANVAS, jx, jy, 25, 128, " ");
  fl_set_object_boxtype(cnv2 , FL_FLAT_BOX);

//define cursor ...
  myXC_cursor = fl_create_bitmap_cursor((char *)cursor1_bits, (char *)curmsk1_bits,
                                        cursor1_width, cursor1_height, cursor1_x_hot, cursor1_y_hot);
  fl_set_cursor_color(myXC_cursor, FL_CYAN, FL_BLUE);

  FLsbig_view = fl_bgn_group();                                 //sbig ViewMode

  dx = 86;
  dy = 26;
  jx = jx + 35;
  jy = jy -  2;
  jy1 = jy;
  obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "Linear");
  fl_set_object_callback(obj, view_cb, V_LIN);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  view_cb(NULL, V_LIN);    // set view to default Mode
  fl_set_button(obj, 1);                                   //mark as default

  jy  = jy + dy;
  obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "Sqrt");
  fl_set_object_callback(obj, view_cb, V_SQRT);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy  = jy + dy;
  obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "Log");
  fl_set_object_callback(obj, view_cb, V_LOG);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy  = jy + dy;
  obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "HistEq");
  fl_set_object_callback(obj, view_cb, V_HIST);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy  = jy + dy + 4;
  obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx / 2, dy, "Inv");
  fl_set_object_callback(obj, view_cb, V_INV2);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  obj = fl_add_button(FL_PUSH_BUTTON, jx + dx / 2, jy, dx / 2, dy, "D_Inv");
  fl_set_object_callback(obj, view_cb, V_INV);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  fl_end_group();

  jy = jy1 - 2;
  jx2 = jx;
  jx = jx + dx * 1.66 - 10;

  obj = fl_add_box(FL_FLAT_BOX, jx, jy, dx / 2, dy, "SBIG:");
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jx  = jx + dx / 2;
  FLsbig_on =
    obj = fl_add_button(FL_PUSH_BUTTON, jx, jy, dx, dy, "ON/OFF");
  fl_set_object_callback(obj, sbigonoff_cb, 1);
  fl_set_object_color(obj, FL_BUTTON_COL1, ACTV_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  FLsbig_cmd = fl_bgn_group();

  jy = jy + dy + 5;
  jy1 = jy;
  jx = jx2 + dx * 2.66;
  sprintf(str, "Temp=%5.1fC", g_sbig_temp);
  FLsbig_temp =
    obj = fl_add_box(FL_FLAT_BOX, jx - dx - 10, jy + 2, dx, dy, str);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  sprintf(str, "%3.1f", g_sbig_temp);
  FLtemp =
    obj = fl_add_input(FL_FLOAT_INPUT, jx, jy + 2, dx * 0.6, dy , "");
  fl_set_input(obj, str);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, sbig_cb, -1);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);

  FLsbig_scrol =
    obj = fl_add_slider(FL_VERT_FILL_SLIDER, jx + 0.65 * dx, jy + 2, 0.2 * dx, dy * 2, "");
  fl_set_object_color(obj, FL_YELLOW, FL_BUTTON_COL1);
  fl_set_slider_bounds(obj, 0., 1.);
  fl_set_slider_value(obj, 0.);
  fl_set_object_callback(obj, push_dummy, 0);
  fl_deactivate_object(obj);
  fl_hide_object(obj);

  jy  = jy + dy;
  FLsbig_expo =
    obj = fl_add_input(FL_FLOAT_INPUT, jx, jy + 2, dx * 0.6, dy, "Expo=  0.5s");
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, sbig_cb, -2);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);
  fl_set_input(obj, "0.5");

  jy  = jy + dy;
  FLsbig_cycimg =
    obj = fl_add_input(FL_FLOAT_INPUT, jx, jy + 2, dx * 0.6, dy, "CycImg=   0");
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, sbig_cb, -3);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);
  fl_set_input(obj, "0");

  jy  = jy + dy;
  FLsbig_filt =
    obj = fl_add_box(FL_FLAT_BOX, jx - 45, jy, 1, dy, "Filter");
  fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  jx2 = jx - 40;
  jy2 = jy;

  jx = jx + dx;
  jy = jy1 - dy - 5;
  obj = fl_add_button(FL_NORMAL_BUTTON, jx + dx / 2, jy, dx / 2, dy, "Save");
  fl_set_object_callback(obj, save_cb, 1);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy = jy + dy + 5;
  FL_pict =
    obj = fl_add_button(FL_NORMAL_BUTTON, jx - 10, jy, dx + 10, dy, "Get Pict");
  fl_set_object_callback(obj, sbig_cb, 1111);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy  = jy + dy;
  FL_dark =
    obj = fl_add_button(FL_NORMAL_BUTTON, jx - 10, jy, dx + 10, dy, "Get Dark");
  fl_set_object_callback(obj, sbig_cb, 1112);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy  = jy + dy;
  obj = fl_add_button(FL_NORMAL_BUTTON, jx - 10, jy, dx + 10, dy, "Get NoShut");
  fl_set_object_callback(obj, sbig_cb, 1113);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  jy  = jy + dy;
  FL_cycle =
    obj = fl_add_button(FL_NORMAL_BUTTON, jx - 10, jy, dx + 10, dy, "Get Cycle");
  fl_set_object_callback(obj, sbig_cb, 999);
  fl_set_object_color(obj, FL_BUTTON_COL1, BUSY_COL);
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);

  FLsbig_ymin =
    obj = fl_add_input(FL_INT_INPUT, jx - 135, jy + 33, dx * 0.6, dy, "Ymin= ");
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, sbig_cb, -20);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);
  fl_set_input(obj, "0");
  FLsbig_ymax =
    obj = fl_add_input(FL_INT_INPUT, jx - 10, jy + 33, dx * 0.6, dy, "Ymax= ");
  fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_object_callback(obj, sbig_cb, -21);
  fl_set_object_color(obj, FL_BUTTON_COL1  , BUSY_COL);
  fl_set_input(obj, "1023");

  fl_end_group();

  dx = 23;
  dy = 21;
  jx = jx2 - 2;
  jy = jy2 + 4;

  FLfilt_cmd = fl_bgn_group();
  FLfilt1 =
    FLfilt[1] =
      obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "C");
  fl_set_object_callback(obj, sbig_cb, 1);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_set_button(obj, 1);                                   //mark as default
  jx  = jx + dx;
  FLfilt[2] =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "L");
  fl_set_object_callback(obj, sbig_cb, 2);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  jx  = jx + dx;
  FLfilt[3] =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "B");
  fl_set_object_callback(obj, sbig_cb, 3);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  jx  = jx + dx;
  FLfilt[4] =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "G");
  fl_set_object_callback(obj, sbig_cb, 4);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  jx  = jx + dx;
  FLfilt[5] =
    obj = fl_add_button(FL_RADIO_BUTTON, jx, jy, dx, dy, "R");
  fl_set_object_callback(obj, sbig_cb, 5);
  fl_set_object_color(obj, FL_BUTTON_COL1, SELE_COL);
  fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_set_object_lstyle(obj, FL_FIXEDBOLD_STYLE);
  fl_end_group();

  fl_deactivate_object(FLsbig_cmd);
  fl_deactivate_object(FLfilt_cmd);
  fl_set_object_lcol(FLsbig_cmd, 901);
  fl_set_object_lcol(FLfilt_cmd, 901);
}

//--------------------------------------------------------------------

short PictureST7(double exposureInSec, unsigned short *pict, int x0, int y0, int xwid, int ywid, int shutter, int mode)
{
  unsigned short row;                            //type: =0  std picture
  static StartExposureParams sePrm;              //      <0  manual shutter
  static QueryCommandStatusParams qPrm;          //      >0  dark frame (closed shutter)
  static QueryCommandStatusResults qRes;         //
  static EndExposureParams eePrm;                //
  static StartReadoutParams srPrm;               //mode: <0  expose only
  static ReadoutLineParams rPrm;                 //      =0  expose+read
  static EndReadoutParams erPrm;                 //      >0  read only
  static MiscellaneousControlParams mscPrm;      //

  unsigned int isleep, st7Error, j;
  long utime[5];
  int ii, jj;

  if (g_sbig_brk != 0) return -1;

  sePrm.openShutter = shutter;
  sePrm.ccd = eePrm.ccd = srPrm.ccd = rPrm.ccd = erPrm.ccd = CCD_IMAGING;
  sePrm.exposureTime = (unsigned long)(100.*exposureInSec + .5);
  sePrm.abgState = ABG_NOT_PRESENT;

  qPrm.command = CC_START_EXPOSURE;

  /*  Expose the Picture  */
  if (mode <= S_PICT_TH) {
    g_sbig_scrol += g_sbig_dscr * 0.5;

    if (g_sb_first > 0 &&  shutter == SC_LEAVE_SHUTTER) { //open shutter in case it is closed and we do no-shutter
      mscPrm.fanEnable = TRUE;
      mscPrm.shutterCommand = SC_OPEN_SHUTTER;
      mscPrm.ledState = 2;
      st7Error = SBIGUnivDrvCommand(CC_MISCELLANEOUS_CONTROL, &mscPrm, NULL);
      if (st7Error != 0) {
        sprintf(lstr, "SBIG error %d  set shutter", st7Error);
        put_logfile(LOG_ERR, 0, lstr);
        g_sbig_err = st7Error + 10000;
      }
      else {
        sprintf(lstr, " \n\n opened shutter and leave it alone\n\n");
        put_logfile(LOG_WRN, 0, lstr);
      }
    }
    isleep = 0;

    sprintf(lstr, "SBIG start exposure %d", sePrm.exposureTime);
    put_logfile(LOG_SBG, 0, lstr);
    if (!(st7Error = SBIGUnivDrvCommand(CC_START_EXPOSURE, &sePrm, NULL))) {
      g_sbig_scrol += g_sbig_dscr * 0.1;

      if      (shutter == SC_LEAVE_SHUTTER) sprintf(tmp_imgtyp, "NOSHUTTTER");
      else if (shutter == SC_CLOSE_SHUTTER) sprintf(tmp_imgtyp, "DARKFRAME");
      else if (shutter == SC_OPEN_SHUTTER)  sprintf(tmp_imgtyp, "IMAGE");
      else                                  sprintf(tmp_imgtyp, "UNDEF");

      if      (g_filter == 1) sprintf(tmp_filter, "CLEAR  (1)");
      else if (g_filter == 2) sprintf(tmp_filter, "LUMIN  (2)");
      else if (g_filter == 3) sprintf(tmp_filter, "BLUE   (3)");
      else if (g_filter == 4) sprintf(tmp_filter, "GREEN  (4)");
      else if (g_filter == 5) sprintf(tmp_filter, "RED    (5)");
      else                    sprintf(tmp_filter, "UNKNWN (%d)", g_filter);

      AMCtime(utime);
      strncpy(tmp_fulldat, g_datstr, 19);
      if (g_sbig_th != 999) sprintf(tmp_pangrp, "UNDEF");  //manual mode ...
      else                  strncpy(tmp_pangrp, act_pangrp, 80);

      tmp_expos = exposureInSec;
      tmp_temp  = g_sbig_temp;
      tmp_azim  = global_azimut;
      tmp_zenit = global_zenith;
      tmp_foclen = 17000 + global_foclen;
      tmp_ymin  = g_ymin + 1;
      tmp_ymax  = g_ymax;

      for (ii = 0; ii < 17; ii++)
        for (jj = 0; jj < 17; jj++) {
          panel[ii][jj].sbg_mot[0] = panel[ii][jj].act_mot[0];
          panel[ii][jj].sbg_mot[1] = panel[ii][jj].act_mot[1];
          panel[ii][jj].sbg_stat  = panel[ii][jj].pan_stat;

        }

      usleep(40000);
      do if (st7Error = SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS, &qPrm, &qRes))
          goto shutit;
        else {
          if (g_sbig_brk != 0) return -1;
          usleep(100000);
          g_sbig_scrol += g_sbig_dscr * 0.1;
          isleep++;
        }
      while ((qRes.status & 0x0003) != 0x0003);

    }
    else {
      sprintf(lstr, "SBIG error %d  start expose", st7Error);
      put_logfile(LOG_ERR, 0, lstr);
      g_sbig_err = st7Error + 20000;
    }

shutit:
    SBIGUnivDrvCommand(CC_END_EXPOSURE, &eePrm, NULL);
    g_pict_expos = 100.*exposureInSec + .5;
    if (st7Error) {
      sprintf(lstr, "SBIG error %d  end_expose", st7Error);
      put_logfile(LOG_ERR, 0, lstr);
      g_sbig_err = st7Error + 30000;
    }
    else {
      sprintf(lstr, "SBIG  end  exposure %d", sePrm.exposureTime);
      put_logfile(LOG_SBG, 0, lstr);
    }
    g_sbig_scrol += g_sbig_dscr * 0.2;

  }

  if (g_sbig_err > 0) return;

  /*  Read the Picture  */
  if (mode >= S_PICT_TH) {  //read picture
    srPrm.ccd = rPrm.ccd = CCD_IMAGING;
    srPrm.readoutMode = 0;    // defaults:
    srPrm.left = x0;          //     0
    srPrm.top =  y0;          //     0
    srPrm.width = xwid;      //  1024
    srPrm.height = ywid;     //  1024
    rPrm.pixelStart = x0;     //     0
    rPrm.pixelLength = xwid;  //  1024

    if (!pict) pict = myImage; /*  set default memory array  */

    pict += y0 * 1024 + x0;

    row = -1;

    st7Error = SBIGUnivDrvCommand(CC_START_READOUT, &srPrm, NULL);
    if (st7Error != 0) {
      sprintf(lstr, "SBIG error %d; retry", st7Error);
      put_logfile(LOG_ERR, 0, lstr);
      usleep(1000); // wait a second and retry
      st7Error = SBIGUnivDrvCommand(CC_START_READOUT, &srPrm, NULL);
    }
    if (st7Error == 0) {
      for (row = y0; row < y0 + ywid; row++, pict += 1024)
        if (st7Error = SBIGUnivDrvCommand(CC_READOUT_LINE, &rPrm, pict))
          goto endit;
        else if (row % 60 == 0) g_sbig_scrol += g_sbig_dscr * 0.2;
    }
    else {
      sprintf(lstr, "SBIG error %d; FAILED", st7Error);
      put_logfile(LOG_ERR, 0, lstr);
      g_sbig_err = st7Error + 50000;
    }

endit:
    if (g_sbig_err == 0 && row != (y0 + ywid)) {
      sprintf(lstr, "SBIG error %d readout row %d", st7Error, row);
      put_logfile(LOG_ERR, 0, lstr);
      g_sbig_err = st7Error + 60000;
    }

    SBIGUnivDrvCommand(CC_END_READOUT, &erPrm, NULL);
  }

enditX:
  if (st7Error > 0) g_sbig_err = st7Error + 90000;
  else {
    pct_expos = tmp_expos;
    pct_temp  = tmp_temp;
    pct_azim  = tmp_azim;
    pct_zenit = tmp_zenit;
    pct_foclen = tmp_foclen;
    pct_ymin  = tmp_ymin;
    pct_ymax  = tmp_ymax;
    strncpy(pct_pangrp, tmp_pangrp, 80);
    strncpy(pct_filter, tmp_filter, 80);
    strncpy(pct_imgtyp, tmp_imgtyp, 80);
    strncpy(pct_fulldat, tmp_fulldat, 80);
  }

  return st7Error;

}   /*  end of ShotAPicture  */

//--------------------------------------------------------------------

short FilterST7(long i)
{
  PulseOutParams p;
  static QueryCommandStatusParams qPrm;
  static QueryCommandStatusResults qRes;
  int k, j, ret;

  if (i < 1 || i > 5) {
    sprintf(lstr, "Invalid CFW-8 filter value");
    put_logfile(LOG_ERR, 0, lstr);
    return -1;
  }
  else {
    sprintf(lstr, "Switch to Filter %d (was %d)", i, g_oldfilt);
    put_logfile(LOG_SBG, 0, lstr);
  }

  j = (i - g_oldfilt);
  if (j == 0) return;
  if (j < 0) j += 5;
  g_sbig_dscr = 1. / (4 * j + 2);
  g_sbig_scrol = g_sbig_dscr;

  p.pulsePeriod = 18270;
  p.numberPulses = 60;
  p.pulseWidth = 500 + 300 * (i - 1);
  qPrm.command = CC_PULSE_OUT;

  ret = SBIGUnivDrvCommand(CC_PULSE_OUT, &p, NULL);
  if (ret != CE_NO_ERROR) {
    sprintf(lstr, "Filter error %d", ret);
    put_logfile(LOG_ERR, 0, lstr);
    g_filter = 9;
    return ret;
  }

  j = (i - g_oldfilt);
  if (j < 0) j += 5;
  g_sbig_dscr = 1. / (4 * j + 3);
  g_sbig_scrol = g_sbig_dscr;
  do {
    usleep(200000);         //need correct timing ....
    g_sbig_scrol += g_sbig_dscr;
    ret = SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS, &qPrm, &qRes);
  }
  while (qRes.status != 0);
  sprintf(lstr, "Filter is %d (was %d)", i, g_oldfilt);
  put_logfile(LOG_SBG, 0, lstr);
  return ret;
}

//--------------------------------------------------------------------

void *StandbyST7_th(void *threadid)
{
  st7Error = SetCoolingOff();
  g_sbig_th = S_STBY_TH * -1;
}

//--------------------------------------------------------------------

void *CloseST7_th(void *threadid)
{
  int k;
  int pwrtry = 0;

  g_sbig_stat = 1;
  g_sbig_scrol = 0;
  st7Error = SetCoolingOff();
  st7Error = SBIGUnivDrvCommand(CC_CLOSE_DEVICE, NULL, NULL);
  st7Error = SBIGUnivDrvCommand(CC_CLOSE_DRIVER, NULL, NULL);


  if (g_sbig_th != 0) {
    g_sbig_scrol += 0.02;
    if (g_sbigon > 0)
      for (k = 0; k < 20; k++) {
        usleep(1000000);    //let some time for cooling before
        printf(".>.");
        g_sbig_scrol += 0.05;
      }
  }
  g_sbigon = 0;                   //mark as off

  push_pwr(NULL, -PWR_SBIG);    //and switch off power
  g_sbig_stat = 0;

  if (g_sbig_th != 0) g_sbig_th = S_CLOS_TH * -1;

}

//--------------------------------------------------------------------

void *InitDST_th(void *threadid)
{

  int pwrtry = 0;

//  switch on power if needed ....
  if (power[PWR_DIST].actual < 1) push_pwr(NULL,  PWR_DIST);

  while (power[PWR_DIST].actual < 1 && pwrtry++ < 20) sleep(1);

  if (pwrtry >= 10) {
    sprintf(lstr, "--- error switching on DIST power ....");
    put_logfile(LOG_ERR, 0, lstr);
  }
//  wait some time for DISTO booting

  sleep(15);
  g_dist_th = S_INIT_TH * -1;
}

//--------------------------------------------------------------------

void *CloseDST_th(void *threadid)
{
  int pwrtry = 0;
  push_pwr(NULL, -PWR_DIST);
  while (power[PWR_DIST].actual > 0 && pwrtry++ < 20) sleep(1);

  g_dist_th = S_CLOS_TH * -1;
}

//--------------------------------------------------------------------

void *InitST7_th(void *threadid)
{
  static GetDriverInfoParams   infoRqs; /* GetInfo structs */
  static GetDriverInfoResults0 infoAns;

  static OpenDeviceParams      openPrm; /* link structs */
  static EstablishLinkParams   linkPrm;
  static EstablishLinkResults  linkAns;
  char lstr[LOGLEN];

  int pwrtry = 0;
  int j;
  g_sbig_scrol = 0.1;
  g_sbig_stat = 2;

//  switch on power if needed ....
  if (power[PWR_SBIG].actual < 1) push_pwr(NULL,  PWR_SBIG);

  while (power[PWR_SBIG].actual < 1 && pwrtry++ < 25) sleep(1);

  if (pwrtry >= 24) {
    sprintf(lstr, "--- error switching on SBIG power ....");
    put_logfile(LOG_ERR, 0, lstr);
    g_sbigon = -990;
    goto enditY;
  }

  g_sbig_scrol = 0.4;
  sleep(1);

  g_sbigon = 0;
  g_sbig_scrol = 0.5;

  st7Error = 0;

  /*  init structs  */
  infoRqs.request = 0;
  openPrm.deviceType = st7;
  /* xmccd doesn't init "linkPrm": let's xfingers.. */
  g_sbig_scrol = 0.6;
  usleep(20000);

  st7Error = SBIGUnivDrvCommand(CC_OPEN_DRIVER, NULL, NULL);
  if (st7Error != 0) printf("error 1 driver %d ....\n", st7Error);
  g_sbig_scrol = 0.65;
  usleep(20000);

  st7Error = SBIGUnivDrvCommand(CC_GET_DRIVER_INFO, &infoRqs, &infoAns);
  if (st7Error != 0) printf("error 2 driver info %d ....\n", st7Error);
  g_sbig_scrol = 0.7;
  usleep(20000);
  sleep(5);

  st7Error = SBIGUnivDrvCommand(CC_OPEN_DEVICE, &openPrm, NULL);
  if (st7Error != 0) printf("error 3 open device %d ....\n", st7Error);
  g_sbig_scrol = 0.75;
  usleep(20000);

  st7Error = SBIGUnivDrvCommand(CC_ESTABLISH_LINK, &linkPrm, &linkAns);
  if (st7Error != 0) printf("error 4 open device %d ....\n", st7Error);
  g_sbig_scrol = 0.8;
  usleep(20000);

  st7Error = SetCoolingOn(g_sbig_temp);
  if (st7Error != 0) printf("error 5 set cool %d ....\n", st7Error);
  g_sbig_scrol = 0.85;
  usleep(20000);


  if (!myImage) myImage = malloc(st7width * st7height * sizeof(unsigned short));
  g_sbig_scrol = 0.9;
  usleep(20000);

  if (st7Error == 0) {
    sleep(1);
    g_sbig_scrol = 0.0;
    st7Error = FilterST7(1);
    if (st7Error != 0) printf("error 6 filter %d ....\n", st7Error);
  }

enditX:
  if (st7Error == 0) {
    g_sbigon = 1;
    g_sbig_stat = 3;
    g_pict_filt = g_filter = 1;
    for (j = 2; j < 6; j++) fl_set_button(FLfilt[j], 0);
    fl_set_button(FLfilt[1], 1);
  }
  else {
    sprintf(lstr, "SBIG on failed %d", st7Error);
    put_logfile(LOG_ERR, 0, lstr);
    g_sbigon = -999;
    g_sbig_stat = 9;
    g_pict_filt = 9;
  }
enditY:
  g_sbig_th = S_INIT_TH * -1;

}

//--------------------------------------------------------------------

void *TakePict_th(void *threadid)
{
  int ret, x0, y0, xwid, ywid, shutt, mode;
  double time = 0.;

  g_sbig_stat = 4;
  x0 = g_xmin;
  xwid = g_xmax - g_xmin;
  if (g_ymin < g_ymax) {
    y0 = g_ymin;
    ywid = g_ymax - g_ymin;
  }
  else {
    y0 = g_ymax;
    ywid = g_ymin - g_ymax;
  }
  shutt = g_sb_shutt;
  mode = g_sb_mode;
  g_sbig_scrol = 0.0;
  if (g_sb_mode == S_READ_TH) time = 3500. / ywid;
  if (g_sb_mode == S_EXPO_TH) time = g_expos + 1.;
  if (g_sb_mode == S_PICT_TH) time = g_expos + 1. + 3500. / ywid;
  g_sbig_dscr  = 1. / time;
  sprintf(lstr, "SBIG Pict %d %d %f", g_sb_shutt, g_sb_mode, g_expos);
  put_logfile(LOG_SBG, 0, lstr);

  g_sbig_th = g_sb_mode;      //flag thread as active
  ret = PictureST7(g_expos, &img_buffer, x0, y0, xwid, ywid, shutt, mode);

  if (g_sbig_cc > 0) {       //was an CC picture mode -> save image
    save_cb(NULL, -11);
    g_sbig_cc = 0;
  }
  g_sbig_stat = 3;

  g_sbig_th = g_sb_mode * -1; //flag thread as finished
}

//--------------------------------------------------------------------

void *Filter_th(void *threadid)
{
  int ret;

  g_sbig_stat = 4;
  g_sbig_th = S_FILT_TH;     //flag thread as active
  ret = FilterST7(g_filter);
  g_oldfilt = g_filter;
  g_sbig_th = S_FILT_TH * -1; //flag thread as finished
  g_sbig_stat = 3;
}

//--------------------------------------------------------------------

int make_socket (uint16_t port)
{
  int sock, ibind;
  struct sockaddr_in name;
  const int y = 1;

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    printf("problem with socket; %d, %d\n", sock, EXIT_FAILURE);
    return (-9);
  }

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));

  /* Give the socket a name. */
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  if (ibind = bind (sock, (struct sockaddr *) &name, sizeof (name)) >= 0)
    return sock;

  printf("problem with bind; %d, %d\n", ibind, EXIT_FAILURE);
  return (-8);
}

//--------------------------------------------------------------------

int read_from_CC(int filedes)
{

  char lstr[LOGLEN], c;
  long k, n, nbytes, nbt;
  char   cmd[10], buffer[MAXMSG];
  long   ccst0, ccyr, ccmon, ccday, cchour, ccmin, ccsec, ccmsec, ccweather;
  long   ccst1[31];
  float  ccM1zd, ccM1az, ccRQzd, ccRQaz, ccT, ccP, ccV, ccH, ccUPS, ccGPS;
  char   ccsrc[1000], ccsched[100], xName[LOGLEN], xCmd[100], ccLight[100], ccGRB[100], ccGPSERROR[100], ccCT[100], ccOVER[100];
  long   cccat, xT;
  long   ccst2[31];
  float  ccM2zd, ccM2az, xZen, xAzi, xFoc, xX, xY;
  long   utime[5];
  int    YY, MM, ifID;

  //clear the buffer first [there could be a long backlog because the CC-FIFOs are queueing]
  if (g_cc_start == 0) {
    do {
      nbytes = read(filedes, buffer, MAXMSG);
    }
    while (nbytes == MAXMSG);
    g_cc_start = 1;
    return(0);
  }

  nbytes = read(filedes, buffer, MAXMSG);

  if (nbytes < 0) return -9;     //error in reading ????
  if (nbytes == 0) return -2;    //found EOF

  AMCtime(utime);
  YY = utime[1] / 10000 + 2000;
  MM = (utime[1] / 100) % 100;

  for (k = 0; k < nbytes; k++) {
    if (buffer[k] != '\n'  &&  buffer[k] != '\0') {
      g_ccbuf[g_ccpos++] = buffer[k];
    }
    else {                          // interpret the actual buffer
      g_ccbuf[g_ccpos++] = '\0'; // mark end of string
      g_ccpos = 0;              // in case something goes wrong, start new line for next iteration
      if (g_ccrec < 0) return -1;   //we have to shutdown ==> stop here

      if (strncmp(g_ccbuf, "CC-", 3) == 0) { // must be a report ...

        strncpy(lstr, g_ccbuf, LOGLEN);
        put_logfile(LOG_CC_, -2, lstr);     //write reports only to log-file (maybe even not there?)

//                                         0  y  m  d  h  m  s ms  w  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8
        n = sscanf(g_ccbuf, "CC-REPORT M0 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f %f %s %s %d %s %d %s %d %d %s",
                   &ccst0, &ccyr, &ccmon, &ccday, &cchour, &ccmin, &ccsec, &ccmsec,        //CCstate, year, month, day, hour, minute, sec, ms
                   &ccweather,                                                             //weather state (wind+humidity, 0=error, 4=ok, 7=warning, 8=alarm)
                   &ccst1[ 0], &ccst1[ 1], &ccst1[ 2], &ccst1[ 3],                         //DAQ1 state, DominoCalibration1, drive1, stg1 state
                   &ccst1[ 4], &ccst1[ 5], &ccst1[ 6], &ccst1[ 7], &ccst1[ 8],             //CaCo1 state, CaCo1 LID, CaCo1 sentine, CaCo1 LV, CaCo1 HV
                   &ccst1[ 9], &ccst1[10], &ccst1[11], &ccst1[12], &ccst1[13], &ccst1[14], //AMC1 state, L2T1, pulsar1, receiver1, DT1, calib1
                   &ccst1[15], &ccst1[16], &ccst1[17],                                     //Lidar state, aux state, GRB state
                   &ccM1zd, &ccM1az, &ccRQzd, &ccRQaz,                                     //current M1 Zd [deg], current M1 Az [deg], req DEC [deg], req RA [deg]
                   &ccT, &ccP, &ccV, &ccH, &ccUPS, &ccGPS,                                 //mean T, pressure, wind speed, mean hum, ups charge, Rub-GPS
                   &ccsched, &ccsrc, &cccat,                                               //SCHEDULE sourcename, category
                   &ccst2[ 0], &ccst2[ 1], &ccst2[ 2], &ccst2[ 3],                         //DAQ2 state, DominoCalibration2, drive2, stg2 state
                   &ccst2[ 4], &ccst2[ 5], &ccst2[ 6], &ccst2[ 7], &ccst2[ 8],             //CaCo2 state, CaCo2 LID, CaCo2 sentinel, CaCo2 LV, CaCo2
                   &ccst2[ 9], &ccst2[10], &ccst2[11], &ccst2[12], &ccst2[13], &ccst2[14], //AMC2 state, L2T2, pulsar2, receiver2, DT2, calib2
                   &ccst2[15],                                                             //readout cooling
                   &ccM2zd, &ccM2az,                                                       //current M2 Zd [deg], current M2 Az [deg]
                   &ccLight,                                                               //LightConditions (“Moon”, “No Moon”, “Twilight”, “Day”)
                   &ccGRB, &ccst2[16],                                                     //GRB dataking state (0=no GRB, 1=GRB alert received; processing it/taking
                   &ccGPSERROR, &ccst2[17],                                                //GPS_ERROR %01d (0=no error, 1=error)
                   &ccCT, &ccst2[18], &ccst2[19],                                          //CT_ACTIVE M1_active, M2_active
                   &ccOVER                                                                 //OVER
                  );

        if (n < 50) {
          if (g_cc_errcnt++ > 1000)
            g_cc_errcnt = 0;
          if (g_cc_errcnt <= 0) {
            put_logfile(LOG_ERR, +1, lstr); //something wrong==> show report on screen
            sprintf(lstr, "Illegal CC-REPORT, n=%d", n);
            put_logfile(LOG_ERR, 0, lstr);  //and give error message
          }
        }
        if (n > 8) {                        //interpret the report and show on the screen
          g_cc_time = cchour * 10000 + ccmin * 100 + ccsec;
          g_cc_sec  = utime[2];
          g_cc_zd   = ccM1zd;
          g_cc_az   = ccM1az;
          g_cc_utim = cchour * 3600 + ccmin * 60 + ccsec;
          if (cchour < 12)
            g_cc_utim = g_cc_utim + 12 * 3600;

          g_yy = ccyr;
          g_mon = ccmon;
          g_dd = ccday;
          g_hh = cchour;
          g_mm = ccmin;
          g_ss = ccsec;
          g_ms = ccmsec;

          g_drvstat = ccst1[2];  // tracking=4, slewing=3
          g_lidstat = ccst1[5];
          sprintf(g_cc_source, "%s", ccsrc);
        }
      }
      else {                     // should be a command
        g_cc_timc = utime[0];
        g_cc_sec  = utime[2];
        strncpy(lstr, g_ccbuf, LOGLEN);
        put_logfile(LOG_CC_, 0, lstr);     //these strings also written to screen ....
        // sscanf command ....
        if (strncmp(g_ccbuf, "SBIG", 4) == 0) { //we have an SBIG command
          g_cc_cmd = 0;
          n = sscanf(g_ccbuf, "SBIG %s %d %s", &xCmd, &xT, &xName);
          if (n > 0) {
            if (strncmp(xCmd, "ON", 2) == 0) {
              g_cc_cmd = 201;
              g_cc_temp = -10;
              if (n > 1) g_cc_temp = xT;
            }
            else if (strncmp(xCmd, "OFF", 3) == 0) {
              g_cc_cmd = 202;
            }
            else if (strncmp(xCmd, "FILT", 4) == 0) {
              g_cc_cmd = 203;
              if (n > 1) g_cc_filt = xT;
              else       g_cc_filt = 1;
              if (g_cc_filt <= 0 || g_cc_filt > 5) g_cc_filt = 1;
            }
            else if (strncmp(xCmd, "PICT", 4) == 0) {
              g_cc_cmd = 211;
              if (n < 3) snprintf(xName, 9, "AUTOPICT");
              if (n > 1) g_cc_expos = xT;
              else       g_cc_expos = 50;
              xName[30] = '\0'; // just in case it is very long ....
              snprintf(g_cc_name, LOGLEN, "%s/%04d/%02d/M1_TPT_%06d_%06d_F%d_X%05d_Z%+03d_A%+04d_%s",
                       sbig_path, YY, MM, utime[1], utime[0], g_pict_filt, (int)g_cc_expos, global_zenith, global_azimut, xName);

            }
            else if (strncmp(xCmd, "DARK", 4) == 0) {
              g_cc_cmd = 212;
              if (n < 3) snprintf(xName, 9, "AUTODARK");
              if (n > 1) g_cc_expos = xT;
              else       g_cc_expos = 50;
              xName[30] = '\0'; // just in case it is very long ....
              snprintf(g_cc_name, LOGLEN, "%s/%04d/%02d/M1_TPT_%06d_%06d_F%d_X%05d_Z%+03d_A%+04d_%s",
                       sbig_path, YY, MM, utime[1], utime[0], 0, (int)g_cc_expos, global_zenith, global_azimut, xName);
            }
          }
        }
        else if (strncmp(g_ccbuf, "BREAK", 5) == 0) {
          g_cc_cmd = 11; //BREAK
        }
        else if (strncmp(g_ccbuf, "INAMC", 5) == 0) {
          g_cc_cmd =  2; //INIFast
        }
        else if (strncmp(g_ccbuf, "SHUTD", 5) == 0) {
          g_cc_cmd = 21; //EXIT
        }
        else if (strncmp(g_ccbuf, "LSADJ", 5) == 0) {
          g_cc_cmd =  4; //LSADJ
        }
        else if (strncmp(g_ccbuf, "ADJST", 5) == 0) {
          n = sscanf(g_ccbuf, "ADJST %f %f %f %f %f %d", &xZen, &xAzi, &xFoc, &xX, &xY, &ifID);

          if (n >=6) g_ifID = ifID ;
          else g_ifID = 0 ;

          if (n >= 5) g_cc_cY = (xY / 100.) - 1000.;
          else g_cc_cY = 0;
          if (n >= 4) g_cc_cX = (xX / 100.) - 1000.;
          else g_cc_cX = 0;
          if (n >= 3) g_cc_cFoc = (xFoc / 100.) - 1000.;
          else g_cc_cFoc = 0;
          if (n >= 2) g_cc_cAzi = (xAzi / 100.) - 1000.;
          else g_cc_cAzi = g_cc_az;
          if (n >= 1) g_cc_cZen = (xZen / 100.) - 1000.;
          else g_cc_cZen = g_cc_zd;
          g_cc_cmd =  3; //ADJST
          if (global_mode != MODE_MANU) {
            global_zenith = (int)g_cc_cZen;
            global_azimut = (int)g_cc_cAzi;
            global_foclen = (int)g_cc_cFoc;
            global_xoff = (int)g_cc_cX;
            global_yoff = (int)g_cc_cY;
          }
        }
        else if (strncmp(g_ccbuf, "TPTON", 5) == 0) {
          g_cc_cmd = 101; //TPTON
        }
        else if (strncmp(g_ccbuf, "TPOFF", 5) == 0) {
          g_cc_cmd = 102; //TPOFF
        }
        else if (strncmp(g_ccbuf, "TPICT", 5) == 0) {
          g_cc_cmd = 111; //TPICT
          n = sscanf(g_ccbuf, "TPICT %s", &xName);
          xName[30] = '\0'; // just in case it is very long ....
          snprintf(g_cc_name, LOGLEN, "%s/%04d/%02d/M1_TPT_%06d_%06d_F%d_X%05d_Z%+03d_A%+04d_%s",
                   sbig_path, YY, MM, utime[1], utime[0], g_pict_filt, g_pict_expos, global_zenith, global_azimut, xName);
        }
        else {
          g_cc_cmd =  0; //nothing to do
        }

        sprintf(lstr, "CC-execute command %d", g_cc_cmd);
        put_logfile(LOG_INF, 0, lstr);

      }
      g_ccpos = 0;              // ready for next line
    }
  }                                // there might now be a half-string in g_ccbuf; will be completed later ...
  if (g_ccpos >= MAXMSG - 100) {  // too long buffer left ?????  ERROR
    g_ccrec = -99;
  }

  return g_ccpos;
}

//--------------------------------------------------------------------

void *FromCC_th(void *threadid)
{
  char lstr[LOGLEN];
  int sock, ilist, isele, iret, fd_max, fd_min;
  fd_set active_fd_set, read_fd_set;
  int i, new;
  struct sockaddr_in clientname;
  struct timeval tv;

  size_t size;

  sprintf(lstr, "start FromCC thread");
  put_logfile(LOG_CC_, 0, lstr);

  /* Create the socket and set it up to accept connections. */
  sock = make_socket (PORT);
  if (sock <= 0) {
    sprintf(lstr, "Problem with Socket %d, sock");
    put_logfile(LOG_ERR, 0, lstr);
    g_ccrec = -11;
    return;
  }

  if (ilist = listen (sock, 1) < 0) {
    sprintf(lstr, "Problem with Listen %d, ilist");
    put_logfile(LOG_ERR, 0, lstr);
    g_ccrec = -12;
    return;
  }

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);
  fd_max = sock;
  fd_min = sock;

  while (g_ccrec >= 0) {            //loop until we have to stop listening ...
    tv.tv_sec = 2;    //2 sec timeout
    tv.tv_usec = 0;   //has to be reset since select overwrites...

    /* Block until input arrives on one or more active sockets. */
    read_fd_set = active_fd_set; //have to reset, since select overwrites
    isele = select (fd_max + 1 , &read_fd_set, NULL, NULL, &tv);
    if (isele < 0) {
      sprintf(lstr, "Problem with Select %d", isele);
      put_logfile(LOG_ERR, 0, lstr);
      g_ccrec = -13;
    }
    else if (isele > 0) {                 //isele==0 is timeout ...
      /* Service all the sockets with input pending. */
      for (i = 0; i <= fd_max; ++i) {
        if (FD_ISSET (i, &read_fd_set)) {
          if (i == sock) {
            /* Connection request on original socket. */
            size = sizeof (clientname);
            new = accept (sock,
                          (struct sockaddr *) &clientname,
                          &size);
            if (new < 0) {
              sprintf(lstr, "Problem with Accept %d", new);
              put_logfile(LOG_ERR, 0, lstr);
              g_ccrec = -14;
            }
            else {

              snprintf (lstr, LOGLEN, "Server: connect from host %s, port %hd  on %d %d",
                        inet_ntoa (clientname.sin_addr),
                        ntohs (clientname.sin_port), i, new);
              put_logfile(LOG_CC_, 0, lstr);

              FD_SET (new, &active_fd_set);
              if (new > fd_max) fd_max = new;
            }
          }
          else {
            /* Data arriving on an already-connected socket. */
            if (iret = read_from_CC (i) < 0) {
              sprintf(lstr, "close the connection; %d, %d", iret, i);
              put_logfile(LOG_CC_, 0, lstr);
              close (i);
              FD_CLR (i, &active_fd_set);
            }
          }
        }
      }
    }
  }
  close(sock);
  sprintf(lstr, "close FromCC thread");
  put_logfile(LOG_CC_, 0, lstr);
  return;
}

//--------------------------------------------------------------------

int init_sockaddCC(struct sockaddr_in *name, const char *hostname, uint16_t port)
{
  struct hostent *hostinfo;

  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL) {
    snprintf (lstr, LOGLEN, "Unknown host %s.", hostname);
    put_logfile(LOG_CC_, 0, lstr);
    return -1;
  }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
  return 0;
}

//--------------------------------------------------------------------

void *ToCC_th(void *threadid)
{
  char lstr[LOGLEN];
  long utime[5], init, iwr;
  int errv, l_AMCstat;
  char report[LOGLEN];
  int  yy,  mon,  dd,  hh,  mm,  ss,  ms;
  int  isnd = 0, jsnd = 0;

  struct sockaddr_in servername;

  sprintf(lstr, "start ToCC thread");
  put_logfile(LOG_CC_, 0, lstr);

  fl_set_object_color(FLout_info, 404, 404);
  sprintf(lstr, "No Connection to CC yet");
  put_logfile(LOG_WRN, 0, lstr);

  if (init_sockaddCC(&servername, SERVERHOST, SERVERPORT) < 0) {
    sprintf(lstr, "hostname %10s not known", SERVERHOST);
    put_logfile(LOG_CC_, 0, lstr);
    g_ccsnd = -1;
  }

  g_cc_tosock = socket (PF_INET, SOCK_STREAM, 0);

  init = -1;
  while (g_ccsnd >= 0) {
    iwr = 0;
    while (init < 0) {            //loop until connection available
      init = connect (g_cc_tosock, (struct sockaddr *) &servername, sizeof (servername));
      errv = errno;
      if (init < 0) {           //flag as error
        if (g_ccsnd < 0) return;
        if (global_mode >= MODE_MANU) fl_set_object_color(FLout_info, 404, 404);
        else                            fl_set_object_color(FLout_info, FL_RED, FL_RED);

        usleep(999000);
      }
      else {
        sprintf(lstr, "To CC connected to %10s ", SERVERHOST);
        put_logfile(LOG_CC_, 0, lstr);
        fl_set_object_color(FLout_info, FL_GREEN, FL_GREEN); //and put info there ...
      }
    }

    //generate report
    AMCtime(utime);       //local time...
    yy = utime[1] / 10000 + 2000;
    mon = (utime[1] / 100) % 100;
    dd = utime[1] % 100;
    hh = utime[0] / 10000;
    mm = (utime[0] / 100) % 100;
    ss = utime[0] % 100;
    ms = utime[3] % 1000;

    g_tim0 = utime[2] - g_tim0x;
    if (g_tim0 > 99999) g_tim0 = 99999;
    g_tim1 = utime[2] - g_tim1x;
    if (g_tim1 > 99999) g_tim1 = 99999;

    if (utime[2] - g_cc_sec > 15) g_COMstat = 2;
    else                          g_COMstat = 1;

    g_all = g_ok + g_err + g_nin + g_mov;
    l_AMCstat = g_AMCstat;
    if (global_mode >= MODE_MANU) l_AMCstat = 0;     //report error in case of manual mode

    sprintf(report, "AMC-REPORT M1 %02d %04d %02d %02d %02d %02d %02d %03d %02d %04d %02d %02d %02d %02d %02d %03d PANELS %03d %03d %03d %03d ADJUST %05d %05d %05d %05d AUTOAD %05d %05d %05d %05d CCD %1d %04d %1d %03d OVER\n",
            l_AMCstat,  yy,  mon,  dd,  hh,  mm,  ss,  ms,
            g_COMstat, g_yy, g_mon, g_dd, g_hh, g_mm, g_ss, g_ms,
            g_all, g_err + g_com, g_mov, g_nin,
            g_zd0, g_az0, g_fc0, g_tim0,
            g_zd1, g_az1, g_fc1, g_tim1,
            g_sbig_stat, (int)g_sbig_temp, g_filter, g_pictcnt);

//  sprintf(report, "AMC-REPORT M1 %02d %04d %02d %02d %02d %02d %02d %03d %02d %04d %02d %02d %02d %02d %02d %03d PANELS %03d %03d %03d %03d ADJUST %05d %05d %05d %05d AUTOAD %05d %05d %05d %05d CCD %1d %04d %1d %03d ifID %05d OVER\n",
//          l_AMCstat,  yy,  mon,  dd,  hh,  mm,  ss,  ms,
//          g_COMstat, g_yy, g_mon, g_dd, g_hh, g_mm, g_ss, g_ms,
//          g_all, g_err + g_com, g_mov, g_nin,
//          g_zd0, g_az0, g_fc0, g_tim0,
//          g_zd1, g_az1, g_fc1, g_tim1,
//          g_sbig_stat, (int)g_sbig_temp, g_filter, g_pictcnt,
//          g_ifIDuse );
//

    sprintf(lstr, "%3d|%3d|%3d|%3d", g_ok, g_err + g_com, g_mov, g_nin);
    fl_set_object_label(FLout_info, lstr); //and put info there ...
    fl_set_object_color(FLout_info, FL_GREEN, FL_GREEN);

    isnd++;
    iwr = send (g_cc_tosock, report, strlen(report) , MSG_NOSIGNAL);
    if (iwr > 0) {
      g_cc_tosec = utime[2];
      g_cc_tostat = 0;
      usleep(999000);
    }
    else {     //have a problem sending report ==> close connection and try to open again
      jsnd++;
      fl_set_object_color(FLout_info, FL_RED, FL_RED);
      sprintf(lstr, "Connection to CC lost");
      put_logfile(LOG_ERR, 0, lstr);

      close(g_cc_tosock);
      usleep(99000);
      g_cc_tosock = socket (PF_INET, SOCK_STREAM, 0);
      init = -9;
    }
    if (isnd > 99) {
      sprintf(lstr, "Successfully sent %d of %d reports", (isnd - jsnd), isnd);
      put_logfile(LOG_CC_, 0, lstr);
      isnd = jsnd = 0;
    }
  }
  sprintf(lstr, "close ToCC thread");
  put_logfile(LOG_CC_, 0, lstr);
}

//--------------------------------------------------------------------
int main(int argc, char *argv[])
{
  int i, j, istat, ret, status;
  int ip, ib, id, try;
  long utime[5];

  sprintf(act_pangrp, "UNDEF");

  AMCtime(utime);
  g_autoadjt = utime[2] + 100000000;
  sprintf(tstr, "MAGIC-I AMC V4.50");
  g_AMCstat = g_AMCstat0 = g_AMCstat1 = 1;

  gen_logfile();

  for (j = 0; j < 17; j++)
    for (i = 0; i < 17; i++) {
      panel[i][j].pan_stat = STAT_NOT;
      panel[i][j].laser    = LAS_NOT;
    }
  global_break = 0;

//initialize AMC
  PWR_read(power);
  AMC_read(panel);

  printf("----------------------------------call AMC_init\n");
  if (AMC_init(panel) != 0) {
    printf("AMC Init failed\n");
    return -1;
  }

//initialize GUI
  fl_gettime(&isec0, &iusec0);
  idlsec - isec0;
  if (AMC_gui_init() != 0) {
    printf("GUI Init failed\n");
    return -1;
  }

  j = NEW_read(panel);

  if (LUT_read(panel, j) < 0) {
    printf("AMC LUT read failed\n");
    return -1;
  }

//initialize communication with CC
  g_ccrec = 0;
  i = pthread_create(&cc_threads[0], NULL, FromCC_th, NULL);
  g_ccsnd = 0;
  i = pthread_create(&cc_threads[1], NULL, ToCC_th, NULL);

//start power thread
  g_pwr_th = PWR_0_TH;
  if (pthread_create(&pwr_threads[0], NULL, Power_th, NULL) != 0) {
    printf("Power Thread could not be started\n");
    return -1;
  }

//fl_do_forms will return only when Exit is pressed
  fl_do_forms();

  printf("end of forms....");

  //end of program: all happens in 'exit_cb()'

  exit(0);
}
