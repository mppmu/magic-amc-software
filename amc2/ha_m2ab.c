// dummy interface to read panel definitions and execute commands


#include <stdio.h>
#include "AMCconst.h"
#include "AMCpanel.h"
#include "AMCpower.h"

int  AMC_init(AMCpanel panel[17][17])
{
// AMC_read(panel) ;
   return(0);
}



int AMC_exec_cmd(AMCpanel panel[17][17], int cmd, int numpan, int ipan, int jpan )
{
   int i,j,e ;
printf(" ..CMD %d\n",cmd);
   for ( i=0; i<17; i++ )
      for ( j=0; j<17; j++) 
         if ( panel[i][j].pan_stat == STAT_TDO ) {
            if ( cmd == CMD_MOVE ) {
               panel[i][j].act_mot[0]=panel[i][j].pc_mot[0]=panel[i][j].pc_moveto[0];
               panel[i][j].act_mot[1]=panel[i][j].pc_mot[1]=panel[i][j].pc_moveto[1];
            } else if ( cmd==CMD_INIT || cmd==CMD_INIF || cmd==CMD_INFF ) {
               panel[i][j].act_mot[0]=panel[i][j].pc_mot[0]=panel[i][j].pc_moveto[0] = 0;
               panel[i][j].act_mot[1]=panel[i][j].pc_mot[1]=panel[i][j].pc_moveto[1] = 0;
               if (rand()%100 < 5 )
                  panel[i][j].act_mot[0]= 1234;
               else if (rand()%101 < 5 ) {
                  panel[i][j].act_mot[0]=-7777;
                  panel[i][j].act_stat += 10  ;
                  panel[i][j].box_stat += 10  ;
               }


            } else if ( cmd==CMD_LSON ) {
               panel[i][j].laser=1;
            } else if ( cmd==CMD_LOFF ) {
               panel[i][j].laser=0;
            }
            panel[i][j].pan_stat = STAT_OK_ ;


            panel[i][j].humi[0] = rand()%99 ;
            panel[i][j].humi[1] = rand()%99 ;
            panel[i][j].humi[2] = rand()%99 ;
            panel[i][j].humi[2] = 88        ;



//          printf(" %d %d\n",panel[i][j].act_mot[0],panel[i][j].pc_mot[0]);
                    usleep(1000);
//                  usleep(100000);
                    e=AMC_check_gui( 0, -1. );
                    if (e != 0 ) return(1) ;
/*
            if (rand()%1000 < 5 ) {
              if ( cmd == CMD_MOVE ) {
                 panel[i][j].act_mot[0]= -123 ;
                 panel[i][j].act_mot[1]= -456 ;
              } else if ( cmd==CMD_INIT || cmd==CMD_INIF) {
                 panel[i][j].act_mot[0]= -8888;
                 panel[i][j].act_mot[1]= -8888;
              } else if ( cmd==CMD_LSON ) {
                 panel[i][j].laser=0;
              } else if ( cmd==CMD_LOFF ) {
                 panel[i][j].laser=1;
              } else if ( cmd==CMD_QUERY ) {
                 panel[i][j].act_mot[0]= -9000 ;
              }
              panel[i][j].pan_stat = STAT_ERR ;
              printf("error %d %d\n",i,j) ;
            }
*/
         }

}



int AMC_exec_dist(int cmd)
{
    int i;

    if (cmd < 0) { printf("exec laser on\n"); return(0); }
    else if (cmd == 0 ) { printf("exec laser off\n"); return(0); }
    else { i = 17000 + rand()%1000; printf("dist = %d\n",i); return(i); }
}


int AMC_exec_pwr(int type, int nominal[32], int status[32] )
{
   int i;
// if ( type == 1 )
//    for (i=0; i<32; i++)
//       if (nominal[i] != status[i])  printf(" chanel: %d  switched to %d\n", i, nominal[i]) ;

   for (i=0; i<32; i++)  
      if (rand()%500  >10) status[i] = nominal[i] ;
      else                 status[i] = (nominal[i]+1)%2 ;
   for (i=0; i<32; i++)
      if (nominal[i] != status[i])  printf(" chanel: %d  switched to %d\n", i, nominal[i]) ;

   printf("...\n");
// usleep( 1000000 );
// AMC_check_gui( 0, -1. );
// usleep( 1000000 );
// AMC_check_gui( 0, -1. );
// usleep( 1000000 );
// AMC_check_gui( 0, -1. );
// usleep( 1000000 );

   printf("...\n");

// if ( type == 0 ) return(-1);
   if ( rand()%10 > 7) return(-1) ;
   return(0);
}



int AMC_dummy_cmd(AMCpanel panel[17][17], int cmd, int zen, int azi, int foc )
{
  int i,j,k,it,jt,istat,iq=0 ;
  double scroll ;

       AMC_check_gui(-1, -2.) ;      //check GUI and CC

  printf("===1====== start cmd  %d\n",cmd);

  it = 0 ;  
  for ( i=0; i<17; i++ )
  {
     for ( j=0; j<17; j++ )
     {
        if (panel[i][j].pan_stat == STAT_TDO )
        {
           scroll = scroll+1.;
           panel[i][j].act_mot[0] += panel[i][j].move_by[0] ;
           panel[i][j].act_mot[1] += panel[i][j].move_by[1] ;
if ( i==-5 && j==-5 ) printf("move %d %d; act %d %d\n", panel[i][j].move_by[0],panel[i][j].move_by[1],panel[i][j].act_mot[0],panel[i][j].act_mot[1]);
           jt = rand()/10000000   ;
           panel[i][j].pan_stat = STAT_ACT ;
           if ( jt == 0 ) jt = 1  ;
//         usleep(jt*100) ;
           if ( rand()<10000000 ) 
           {
              panel[i][j].pan_stat = STAT_ERR ;
              if ( cmd == CMD_LSON ) panel[i][j].laser = LAS_ON ;
           }
           else 
           {
              panel[i][j].pan_stat = STAT_OK_ ;
              if ( cmd == CMD_LSON )     panel[i][j].laser = LAS_ON ;
              else if ( cmd == CMD_LOFF) panel[i][j].laser = LAS_OFF ;

              else if ( cmd == CMD_CNTR) { panel[i][j].act_mot[0]= 0; panel[i][j].act_mot[1]= 0; }
              else if ( cmd == CMD_CNT2) { panel[i][j].act_mot[0]=20; panel[i][j].act_mot[1]=30; }

           }

           it = it + jt ;
           if ( it > 100 )
           {
              it = 0 ;
//            panel[6][6].pan_stat = iq ;
              iq = (iq++ ) % 4 ;
//            istat = AMC_check_gui( -1, scroll) ;
              istat = AMC_check_gui(  0, scroll) ;
              if ( istat != 0 ) return 1 ;
           }  
//         panel[i][j].pan_stat = (i+j)%10 ;
//         panel[i][j].pan_stat = STAT_ERR ;

        }
  }  }
//printf("stop  cmd  %d\n",cmd);
}


