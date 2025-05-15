typedef struct  {                //contains info about panel button
   int status   ;                                                                    
   int select   ;
   int laser    ;
   int col1     ;
   int col2     ;
   char str[30] ;
} AMCbutton ;

#define  COL_GRAY    161,161,161
#define  COL_DIS_9   161,161,161
//deactivated button
#define  COL_DIS_1   128,128,128
#define  COL_COM_1   200, 40, 40
#define  COL_BAD_1   130, 35, 35
#define  COL_BCM_1   200, 40, 40
#define  COL_NIN_1    28, 77,255
#define  COL_OK__1    28,228, 28
#define  COL_ERR_1   255,  0,  0
#define  COL_BRK_1   255,000,255
#define  COL_TDO_1   255,255,000
#define  COL_ACT_1    60,255,255
#define  COL_OK1_1   128,255,120
#define  COL_ER1_1   255,128,128
#define  COL_OK2_1   196,255,196
#define  COL_ER2_1   255,196,196
#define  COL_SBG_1   255,255,255

//not used ...
#define  COL_DIS_2   161,161,161
#define  COL_COM_2    52,000,000
#define  COL_BAD_2    44,  3,  3
#define  COL_BCM_2    52,000,000
#define  COL_NIN_2   000, 33, 85
#define  COL_OK__2   000, 85,000
#define  COL_ERR_2    85,000,000
#define  COL_BRK_2    66,000, 66
#define  COL_TDO_2    85, 85,000
#define  COL_ACT_2    18, 85, 85
#define  COL_OK1_2   128,255,120
#define  COL_ER1_2   255,128,128
#define  COL_OK2_2   196,255,196
#define  COL_ER2_2   255,196,196
#define  COL_SBG_2   255,255,255

//activated button
#define  COL_DIS_3   161,161,161
#define  COL_COM_3   240, 80, 80
#define  COL_BAD_3   170, 70, 70
#define  COL_BCM_3   240, 80, 80
#define  COL_NIN_3     0,128,255
#define  COL_OK__3     0,255,  0
#define  COL_ERR_3   255, 48, 48
#define  COL_BRK_3   255, 60,255
#define  COL_TDO_3   255,255, 40
#define  COL_ACT_3   120,255,255
#define  COL_OK1_3   128,255,120
#define  COL_ER1_3   255,128,128
#define  COL_OK2_3   196,255,196
#define  COL_ER2_3   255,196,196
#define  COL_SBG_3   255,255,255


//color settings of AMCpower

#define  PCL_ERR_1   255,000,000
#define  PCL_WRN_1   255,255,000
#define  PCL_NOK_1   000,128,000
#define  PCL_OK_1    000,255,000

#define  PCL_ERR_2   255, 40, 40
#define  PCL_WRN_2   255,255, 40
#define  PCL_NOK_2   000,128, 40
#define  PCL_OK_2     40,255, 40

#define  PCL_BUSY      0,255,255
#define  PCL_NDEF      0,128,255

//label colors
#define  LAB_ACT       0,  0,  0
#define  LAB_DCT     200,200,200

//global button colors
#define  ALL_BUSY    255,255,255
#define  BUSY_COL    FL_CYAN
#define  SELE_COL    FL_WHITE
#define  ACTV_COL    401





// SBIG display modes
#define V_INV2 -2
#define V_INV  -1
#define V_NOT   0
#define V_LIN   1
#define V_LOG   2
#define V_SQRT  3
#define V_SQAR  4
#define V_HIST  5

// SBIG operation status (?)
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

// SBIG size
#define C_WID 1024
#define C_HIG 1024



// SBIG threads and picture modes  ## don't change order !!!
#define S_NULL_TH  0
#define S_UNDF_TH  1
#define S_INIT_TH  2
#define S_STBY_TH  3
#define S_CLOS_TH  4
#define S_FILT_TH 10               //
#define S_EXPO_TH 11               // order of these must be kept
#define S_PICT_TH 12               // and they shall be the last
#define S_READ_TH 13               // allowed settings


//commands possible in GUI

#define  GUIcmd_NONE  -1             //  do nothing
#define  GUIcmd_TEST   0             //**read status etc.
#define  GUIcmd_INIT   1             //**read info from driver; check freq etc
#define  GUIcmd_ADJS   2             //  move mirrors to position from DB
#define  GUIcmd_TPNT   3             //  tpoint (adjust to infin; take sbig)

#define  GUIcmd_LSON   4             //**set laser on
#define  GUIcmd_LOFF   5             //**set laser off
#define  GUIcmd_LSAD   6             //  do laser adjust
#define  GUIcmd_CNTR   7             //**center  (without laser)
#define  GUIcmd_CNT2   8             //**center2 (without laser)
#define  GUIcmd_DFOC   9             //  defocus panel
#define  GUIcmd_RAND  10             //  randomize panel position
#define  GUIcmd_MVTO  11             //**move motors to a position
#define  GUIcmd_MOVE  12             //**move motors by given steps
#define  GUIcmd_MOVA  13             //  move motorA by given steps
#define  GUIcmd_MOVB  14             //  move motorB by given steps
#define  GUIcmd_MVMI  15             //  move motors to minimum
#define  GUIcmd_MVMA  16             //  move motors to maximum

#define  GUIcmd_CALI  17             //  do LUT adjust
#define  GUIcmd_RQAD  18             //  do Roque Adjust
#define  GUIcmd_PLAD  19             //  do Polaris Adjust
#define  GUIcmd_STAD  20             //  do Star Adjust
#define  GUIcmd_ACAD  21             //  get all actuator addresses

#define  GUIcmd_INIF  22             //**fast init
#define  GUIcmd_INFF  23             //**fast init without center
#define  GUIcmd_ZERO  24             //  move motors to ZERO position

#define  GUIcmd_NEW1  25             //do a TPOINT with 'new1' instead of 'lut1'

//log file types

#define  LOG_INF  0   // 0=info
#define  LOG_OK_  1   // 1=ok
#define  LOG_WRN  2   // 2=warning
#define  LOG_ERR  3   // 3=error
#define  LOG_SVR  4   // 4=severe error
#define  LOG_DB1  5   // 5=debug 1
#define  LOG_DB2  6   // 6=debug 2
#define  LOG_DB3  7   // 7=debug 3
#define  LOG_DB4  8   // 8=debug 4
#define  LOG_DB5  9   // 9=debug 5
#define  LOG_DB6 10   //10=debug 6
#define  LOG_DB7 11   //11=debug 7
#define  LOG_DB8 12   //12=debug 8
#define  LOG_DB9 13   //13=debug 9
#define  LOG_HRT 14   //14=heart beat
#define  LOG_CC_ 15   //15=from CC
#define  LOG_ACT 16   //16=actuator info
#define  LOG_PWR 17   //17=power    info
#define  LOG_SBG 18   //18=SBIG     info
#define  LOG_LAS 19   //19=Laser    info



//length of CC-buffer
#define MAXMSG 10000
