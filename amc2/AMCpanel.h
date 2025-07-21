
typedef struct {                  //contains all info about a panel
   int ix, iy, pnmbr;             //x-, y-coordinate (index), panel_number
   long time[2] ;                 //time of last update of this table

// for the following items: index 0=x-motor, 1=y, 2=laser

   unsigned char version[3][2] ;  //version number of drivers
   int port[2][3], addr[3];       //communication address x/y_motor/laser
   int portflg[3];                //0=std port; 1=alt port
   int cpos[3], bpos[3];          //position in chain, line in box
   //
   int freq[2], work[2], hold[2]; //parameters: x/y-motor
   int temp[3], humi[3];          //temperature, humidity: x/y/l
   //
   int pan_stat  ;                //status of panel (todo,ok,err,...)
   int pan_inst  ;                // installed sub-info
   int pan_grp   ;                //Panel Group for Star-Adjust
   int pan_case  ;                //do_case SW in case "case"
   int laser     ;                //0=off
   int ech1_cnt[3][2];            //counter for 1a/1b echo
   int ech2_cnt[3][2];            //counter for 2a/2b echo
   //
   int act_zen   ;                //zenith\azimuth when last
   int act_azi   ;                //     movement/adjust was done
   long act_time  ;               //time when last adj. was done
   int act_flen  ;                //focal length when last move was done
   //
   float hal_endl[2][4];          // values of the 4 hall probes at lower end
   float hal_endu[2][4];          // values of the 4 hall probes at upper end
   float hal_val[2][4];           // x/y values of the 4 hall probes
   int hal_pos[2];                // x/y motor position according to hall probes
   //
   unsigned int range[2];         //range: # steps between endswitches
   //
   int endsw_lwn[2];              //endswitches 'soll': lower: sensor number
   int endsw_lwl[2];              //                           sensor value
   int endsw_upn[2];              //                    upper: sensor number
   int endsw_upl[2];              //                           sensor value
   //
   int rndsw_lwn[2];              //endswitches 'read_back': order same as above
   int rndsw_lwl[2];
   int rndsw_upn[2];
   int rndsw_upl[2];
   //
   //
   int act_mot[2];                // x/y motor position local stored in actuator
   int llw_mot[2];                // x/y lower limit of motor position 
   int lup_mot[2];                // x/y upper limit of motor position
   int pc_mot[2];                 // x/y motor position stored in pc (this program)
   int pc_oldmot[2];              // x/y motor position stored in pc (this program)
   int sbg_mot[2];                // x/y motor at time when SBIG picture taken
   int sbg_stat  ;                // x/y motor at time when SBIG picture taken
   //
   int ladj_zen  ;                //zenith\azimuth when last
   int ladj_azi  ;                //     las-adjustment was done
   long ladj_time ;               //time when last las-adj. was done
   int ladj_flen ;                //focal length when last laser-adjust
   int ladj_xlas ;                // uncorrected x/y laser position for
   int ladj_ylas ;                //     laser adjustment  (PanelPos.txt)
   int ladc_xlas ;                // corrected x/y laser position for
   int ladc_ylas ;                //     laser adjustment   
   //
   int ladj_lut[2] ;              // x/y motor positions predicted
                                  //     by best fitting LUT
   int   mov_dab[2]  ;            // distance between actual position
                                  //     and requested position in Actuator steps
   float mov_dxy[2]  ;            // distance between actual position
                                  //     and requested position in SBIG Coordinates
   //                                
   double cali_slp[2] ;           //calibrated  angle to x-axis for motor
                                  //     movments 1/2  (alpha in rad, not reversed)
   double cali_dca[2] ;           //calibrated distance of closest approach of
                                  // line y(x) for motor 1/2
   double cali_spp[2] ;           //calibrated steps per pixel for motor 1/2
                                  //     movements (factor pixel to steps)
   //
   unsigned char status[3][2];    //status1 (low byte) and status2
/*
   status[][0]: bit 0: verges error, execution of command done/tried
                bit 1: unknown command (inexistent cc)
                bit 2: command rejected, nothing done (on too early move)
                bit 3: illegal/meaningless first parameter (word)
                bit 4: illegal/meaningless second/other parameter (word)
                bit 5: cc and length of frame (number of params) contradicting
                bit 6: cp and length of frame contradicting
                bit 7: parameter value is out of range
                       Comment on bit 5: the command needs eg. 2 parameters, but only one was sent.
                               on bit 6: the reading of the frame was terminated by timeout, but
                                         the CRC16 was correct.
                                         Example: only one parameter was sent, but cp indicates two.
                                         Depending on cc the error is: cp is wrong and should be one
                                         or not enough parameters sent (bit 5 has also to be set).

   status[][1]: bit 0: motor: moving
                bit 1: motor: direction up
                bit 2: motor: endswitch reached
                bit 3: motor: centered
                bit 4: SHT sensor read error (humidity, temp)
                bit 5: multi move command: center
                bit 6: Laser On
                bit 7: Driver reset (indicates that driver has been reset at least once)
                       Comment on bits 2 and 3: If the motor is not moving (bit0 = 0)
                                                bit1 indicates the direction the motor was moving last.
*/
   unsigned int actstat[3];       //e.g. wrong freq; wrong current; ...
   unsigned int watch0[3];        //watchdog value at first call
   unsigned int watch1[3];        //watchdog value at later call
   unsigned char watsrc[3];       //watchdog source
   //
   int move_by[2];                //steps to move (rel. to act_mot[]
   double mov_xlas  ;             //new laser position requested
   double mov_ylas  ;             //     by command

   //
   int lut[2][200][2] ;           // lut[az][zd][mot]   az: dummy (not used so far)
                                  //                    zd: -99deg ... +100deg
                                  //                    0=motA 1=motB
   int new[2][200][2] ;           // same as lut, but for new tests

   //                             //Position from B-value: space for calibration file
   int   numpos[2] ;              //number of measured positions  (x/y)
   float uhr[2][AMCnhall] ;       //Messuhr position (micron)
   float step[2][AMCnhall] ;      //stepping motor position
   float bval[2][AMCnhall][4] ;   //hall value
   float dbds[2][AMCnhall][4] ;   //derivative: d bval/ d step
   int l600_pos[2] ;              //center2 region lower position          (in new_center coords)
   int u600_pos[2] ;              //               upper position          (in new_center coords)
   int endsw_lwp[2];              //position in steps at lower endsw level (in new_center coords)
   int endsw_upp[2];              //position in steps at upper endsw level (in new_center coords)
   int esw_range[2];              //difference between the two above
   int esw_centr[2];              //new/center offset after (new)-center": old_centre_origo in new_centre coords
                                  //if new_center is done: =0; if center is done: offset to new_center
   int pan_oldstat;               //to store status at start of (some) actions
   int pan_oldsta2;               //to store status at start of (some) actions
   int pan_tmpstat;               //to store temporary status
   int pan_wrkstat;               //to store status while working on panel
   int pc_moveto[2] ;             //position we should move to
   int mot_stat;                  //status of motor position
   int act_stat;                  //counter for communic.problems with actuator
   int box_stat;                  //counter for communic.problems with box

   int pan_material ;             //0=glas , >0= alu
   int corr[2] ;                  //global correction to apply to actuators

   int err_cnt[3] ;               //local error count in command
   int err_cnt_tot[3] ;           //total error count in session
   int tmp_cnt ;                  //temporary comunication status
   int cmd_cnt[3] ;               //count how often a command exectued
   int pan_sel ;                  //selected panel flag
   int tmp_sel ;                  //selected panel flag
   int tmp_sl2 ;                  //selected panel flag

   float loc_x ;                  //panel dependent pointing offset x (for IF)
   float loc_y ;                  //panel dependent pointing offset y (for IF)
   float loc_z ;                  //panel dependent pointing offset z (for IF)

   int nothing;                   //dummy address
   
}                                 //
AMCpanel ;                      




// AMCpanel.laser:
#define  LAS_UDF    -16           // laser (panel) is undefined
#define  LAS_NOT     -4           // laser (panel) does not exist
#define  LAS_OFF      0           // laser switched off
#define  LAS_ON      64           // laser switched on

// AMCpanel.pan_stat: also  AMCpanel.drv_stat[] for lines with *
#define  STAT_NOT    -5           // panel does not exist
#define  STAT_DIS    -4           // panel disabled by operator
#define  STAT_COM    -3           // panel disabled by SW (no communication) *
#define  STAT_BAD    -2           // panel disabled by SW (too many errors)  *
#define  STAT_NIN    -1           // panel not initialised
#define  STAT_BCM     0           // box communication problems
#define  STAT_OK_     1           // panel ok  (action successfull)          *
#define  STAT_ERR     2           // panel err (action not successfull)      *
#define  STAT_BRK     3           // action not executed because of 'BREAK'  *
#define  STAT_TDO     4           // panel 'TODO'    [during action only]
#define  STAT_ACT     5           // panel 'ACTIVE'  [during action only]    *
#define  STAT_OK1     6           // panel 'OK1'     [during action only]    *
#define  STAT_ER1     7           // panel 'ER1'     [during action only]    *
#define  STAT_OK2     8           // panel 'OK2'     [during action only]    *
#define  STAT_ER2     9           // panel 'ER2'     [during action only]    *
#define  STAT_SBG    10           // wait for sbig ...
#define  STAT_NOA    11           // no action to be executed                *
/*
  (*)  After a Laser-Adjust, the actual motor positions are
       compared with those predicted for the corresponding LUT
       and the difference stored in AMCpanel.ladj_d*m.
       This value must be added to all future LUT-motor
       positions, until a new Laser-Adjust is done.          */

int AMC_init(AMCpanel panel[17][17]) ;
/* Routine read in configuration files and open rs-485
   communications                                            */

int AMC_check_gui(int n, double scroll) ;
/* Routine to be regularily called during command execution
   to check if BREAK button was pressed or BREAK command
   received from CC
   Return 0    no BREAK
          1    BREAK immediately, and set all LASERS OFF     */


int AMC_exec_cmd(AMCpanel panel[17][17], int cmd, int zen, int azi, int foc ) ;
/*
   cmd = command to be executed ( CMD_**** )
   zen = actual zenith position of telescope
   azi = actual azimuth position ot telescope
   foc = actual focal length for camera

   Routine called to execute command 'cmd' for all panels
   having   panel[i][j].pan_stat == STAT_TDO
   When working with a panel, it must set panel[i][j].pan_stat=STAT_ACT
   and when finished with the panel, set  panel[i][j].pan_stat=STAT_OK_
   or                                     panel[i][j].pan_stat=STAT_ERR
   It must also set panel[i][j].laser accordingly when laser status changes.

   This routine must regularily call AMC_check_gui(); if the
   return is !=0 it must break the execution as fast as possible
   and set for all not finished panels    panel[i][j].pan_stat=STAT_BRK

   It returns !=0 if cmd could not be executed (e.g. unknown command)
*/
#define  CMD_NONE  -1             // do nothing
#define  CMD_QUERY  0             // read actuator position
#define  CMD_INIT   1             // read info from driver; do center if needed

#define  CMD_LSON   2             // set laser on
#define  CMD_LOFF   3             // set laser off
#define  CMD_CNTR   4             // center  
#define  CMD_CNT2   5             // center2 
#define  CMD_MOVE   6             // move motors by given steps (move_by[] )
#define  CMD_DFOC   7             // defocus panel
#define  CMD_ACAD   8             // actuator addresses
#define  CMD_INIF   9             // read info from driver (fast init, with centering if not done)
#define  CMD_INFF  10             // very fast init, same as above without centering

#define  CMD_LADJ 100             // laser adjust
#define  CMD_CALI 101             // laser adjust

//
//

#define COM_LIMIT  10             // set STAT_COM if more than these errors
