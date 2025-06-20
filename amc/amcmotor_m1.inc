// Magic 2
//
//
#define n5000 50
#define r100000 5000   //100000    //HA 16.5.2005
#define blocking 1     //0 =non-blocking; !=0 =blocking

  #include <sys/socket.h>                                        //AB
  #include <netinet/in.h>                                        //AB
  #include <netdb.h>                                             //AB

  struct sockaddr_in servername[16];
  uint16_t  blackport[16];


int write_I_working(int ip, AMCpanel* ptr, int it, int current ) ;
int read_I_working(int ip, AMCpanel* ptr, int it, int* current ) ;
int write_I_holding(int ip, AMCpanel* ptr, int it, int current ) ;
int read_I_holding(int ip, AMCpanel* ptr, int it, int* current ) ;
int write_frequency(int ip, AMCpanel* ptr, int it, int frequency ) ;
int read_frequency(int ip, AMCpanel* ptr, int it, int* frequency ) ;
int read_temp(int ip, AMCpanel* ptr, int it, int wrdbuf[10] ) ;
int query_absxy(int ip, AMCpanel* ptr, int* x, int* y ) ;
int read_version_nr(int ip, AMCpanel* ptr, int it, unsigned char version[2] ) ;
int resend_last(int ip, AMCpanel* ptr, int it, int resin ) ;
int reset_box(int ip, AMCpanel* ptr, int it ) ;
int center_motors(int ip, AMCpanel* ptr, int it ) ;
int move_xy(int ip, AMCpanel* ptr, int x, int y ) ;    //steps[2]$
int move_xyabs(int ip, AMCpanel* ptr, int x, int y ) ;    //steps[2]$
int reset_driver(int ip, AMCpanel* ptr, int it ) ;
int kill_motion(int ip, AMCpanel* ptr, int it ) ;
int power_on(int ip, AMCpanel* ptr, int it ) ;
int power_off(int ip, AMCpanel* ptr, int it) ;
int laser_on(int ip, AMCpanel* ptr ) ;
int laser_on_parallel(int ip, AMCpanel* ptr, int* subcase ) ;
int laser_off(int ip, AMCpanel* ptr) ;
void fill_CRC(unsigned char cmdframe[20]) ;
unsigned short CRC16(unsigned char chByte, unsigned short oldCRC);
//void init_frame(unsigned char frame[20], int ip, int ib, int it, char comcod) ;
void init_frame(unsigned char frame[20], int ib, char comcod) ;
int do_retry(int retcod, int ip, AMCpanel* ptr, int it, char err_mess[100], int resp);



//-----------------------------------------------------------------------------------

#include "distlas.inc"      //distance laser
#include "RS232lib.inc"     //read write library for RS232 on central boxes computer

//-----------------------------------------------------------------------------------

int ready_response(int ip, int nbyte) {
    int nready;

    ioctl(fd[ip], FIONREAD, &nready);
//  printf("nready %d, nbyte %d \n",nready, nbyte);
    if(nready < nbyte) nready=-1;

    return(nready);
}


//=====================================================================



  int get_response1( int globalchain, unsigned char lbufout[256], int* res0, int resin){
  long t1,t2,t12,s1,s2,s12,dt12,ds12;

  unsigned int icon, acon[61];
  unsigned char st1, st2, debug[2560];
//$$
    int j, i, k, *res, retcod, idebug, i2 ;
    unsigned char lbuf[256];
    ushort   crcword;
    int  *word;
    unsigned char byte[4];
    int             res1,res2,res3, iloop ;              //AB

  sprintf(&debug[0],"enter get_response1 %d\n\0",globalchain );
  log_write(fdisk, debug, &i2);

    s12=0;
    t12=0;
    word=&byte;
    retcod=0;
    res=res0;
//    usleep(10000);
//    usleep( 2000);
    *res=0;
//  *res = read(fd,lbuf,resin );                         //AB more complicated
    j=gettimeofday( tv, tz);
    t1=atv.tv_usec;
    s1=atv.tv_sec-1100100000;
//printf("get_response1: s1=%d, t1=%d\n",s1,t1);
  sprintf(&debug[0],"get_response1: s1=%ld, t1=%ld\n",s1,t1);
  log_write(fdisk, debug, &i2);

    res1 = 0 ;                                           //AB
    iloop=0;                                             //AB allow for //ha
    for(i=0;i<256;i++) lbuf[i]=-1;
listen:
       res3 = read(fd[globalchain],&lbuf[res1],resin-res1);               //AB//ha
       if (res3 < 0  )                                       //AB//ha
       {                                                 //AB
          retcod=res3*10;
          printf("read returned error code %d\n",res3); //AB
          goto endlisten;                                //AB
       } else if(res3 == 0 ) {
          iloop++;
       } else {                                          //AB
          if( (res1==0) && (lbuf[res1]==00) ) {
morezer:
           for(k=1;k<res3;k++) lbuf[k-1]=lbuf[k];
           --res3;
           if( (lbuf[res1]==00) && (res3>0) ) goto morezer;
          }
          iloop=0;
          res1 += res3 ;                                 //AB
       }
       if ( res1 < resin) 
//      if( iloop<30)                                    //AB
        if( iloop<5 )                                    //AB
        {
         usleep(100000) ;                                  //AB
         goto listen;                                     //AB
        } else {
         retcod=-2;
        }

endlisten:


  idebug=0; j=-2;
  sprintf(&debug[idebug],"R   :j=%3d, port=%2d; ",j, globalchain); idebug +=21;
  for(i=0;i<resin;i++) {
   sprintf(&debug[idebug],"%2.2x ", lbuf[i] );
   idebug += 3;                       
  }
  sprintf(&debug[idebug],"\n\0" );
  log_write(fdisk, debug, &i2);

    *res = res1 ;                                        //AB

//  printf("globalchain=%d; Bytes received %d, exspected %d \n",globalchain , *res, resin);
    lbuf[*res]=0;
    for(j=0;j<*res;j++) {
     globallbuf[j]=lbuf[j];
//   printf("%2.2x ",lbuf[ j]);
    }
    j=gettimeofday( tv, tz);
    t2=atv.tv_usec;
    s2=atv.tv_sec-1100100000;
    if(t2 < t1 ) {
     t2=t2+1000000;
     --s2;
    }
    if(*res <= 0 ) {
//   printf("NO  2. echo, res= %d: at %d sec %d usec \n", *res,s2-s1,t2-t1 );
     return(-1+retcod);
    } 
//  printf("  2. echo: %d: at %d sec %d usec \n", *res,s2-s1,t2-t1 );
    ds12=s2-s1;   //-s12;
    dt12=t2-t1;   //-t12;
    if(dt12<0) {
     ds12=ds12-1;
     dt12=1000000+dt12;
    }

    icon=10*ds12 + (dt12/100000);
    if(icon < 0) icon=0;
    if(icon>60) icon=60;
    ++acon[icon];

    if((ds12 > 1) || (dt12 > 520000) ) {
     printf("connection delay %ld sec %ld usec \n",ds12,dt12);
    }
    if(lbuf[1] == 0x15 ) { 
//   printf("NAK %3x %3x \n", lbuf[7],lbuf[8]);
     *res=-3;
    } else {
     if(lbuf[1] == 0x85 ) { 
//    if(lbuf[5]+8 != *res) {
//     printf("cp error cp+9= %d  *res= %d \n",lbuf[6]+9,*res);
//    }
      crcword = 0;
      for(k=2;k<lbuf[5]+6;k++) crcword = CRC16(lbuf[k], crcword);
      (*word)=crcword;
      if( (byte[0] != lbuf[lbuf[5]+6]) || (byte[1] != lbuf[lbuf[5]+7]) ) {
       printf("0my CRC: %2.2x %2.2x; CRC received: %2.2x %2.2x \n", byte[0],byte[1],
                 lbuf[lbuf[5]+6],lbuf[lbuf[5]+7]);
       printf("ACK   response CRC16 wrong \n");
       *res=-2;
//---for now only----------------------------
       if(lbuf[5] > 2 ) {   //got status
/* 31.1.2011
        if((lbuf[8] & 0x40) != 0 ) { printf("  Laser on  \n");
        } else                       printf("  Laser off \n");
        if((lbuf[8] & 0x01) != 0 ) {
         printf("  moving ");
         if((lbuf[8] & 0x02) != 0 )  printf(" up   \n");
         else                        printf(" down \n");
        } else {
         if((lbuf[88 & 0x04) != 0 ) {
          if((lbuf[9] & 0x02) != 0 ) { printf(" upper endswitch \n");}
          else printf(" lower endswitch \n");
         }
         else {
          printf(" not moving \n");
         }
        }
        if((lbuf[8] & 0x08) != 0 ) printf(" centered\n");
        if((lbuf[8] & 0x10) != 0 ) printf(" SHT error ");
        if((lbuf[8] & 0x80) != 0 ) printf(" Reset done ");
*/
        *res=lbuf[5]-3;
        for(j=0;j<*res;j++) lbufout[j]=lbuf[j+10];
       }

//-------------------------------------------
      } else {
//     printf("ACK   response CRC16 ok ");
       if(lbuf[5] > 2 ) {
        st1=lbuf[7];
        st2=lbuf[8];
// printf("lbuf[5]= %d, lbuf[7]= %x,lbuf[8]= %x \n",lbuf[6],lbuf[8],lbuf[9]);
        if((lbuf[8] & 0x40) != 0 ) { printf("  Laser on  \n");
        } else                       printf("  Laser off \n");

        if((lbuf[8] & 0x01) != 0 ) {
         printf(" x moving ");
         if((lbuf[8] & 0x04) != 0 )  printf(" up   \n");
         else                        printf(" down \n");
        } 
        if((lbuf[8] & 0x02) != 0 ) {
         printf(" y moving ");
         if((lbuf[8] & 0x08) != 0 )  printf(" up   \n");
         else                        printf(" down \n");
        } 

        if((lbuf[8] & 0x10) != 0 ) {
         printf(" x reached endswitch ");
         if((lbuf[8] & 0x04) != 0 )  printf(" up   \n");
         else                        printf(" down \n");
        } 
        if((lbuf[8] & 0x20) != 0 ) {
         printf(" y reached endswitch ");
         if((lbuf[8] & 0x08) != 0 )  printf(" up   \n");
         else                        printf(" down \n");
        } 

        if((lbuf[8] & 0x03) == 0 ) { printf(" x/y not moving \n");  }

        if((lbuf[8] & 0x80) != 0 ) printf(" No response from driver  BAD \n");
       } else printf(" \n");
       if(lbuf[7] != 0 ) {
//      printf(" %2.2x \n",lbuf[ 8]);
        if( (lbuf[7] & 0x01) != 0 ) printf("Verges error \n" );
        if( (lbuf[7] & 0x02) != 0 ) printf("Unknown command (cc) \n" );
        if( (lbuf[7] & 0x04) != 0 ) printf("Command rejected, nothing done \n" );
        if( (lbuf[7] & 0x08) != 0 ) printf("Illegal/Meaningless 1. parameter \n" );
        if( (lbuf[7] & 0x10) != 0 ) printf("Illegal/Meaningless 2. parameter \n" );
        if( (lbuf[7] & 0x20) != 0 ) printf("cc and length of frame contradicting \n" );
        if( (lbuf[7] & 0x40) != 0 ) printf("CP and length of frame contradicting \n" );
        if( (lbuf[7] & 0x80) != 0 ) printf("Bad driver address \n");
        *res=-1;
       } else {
        if((lbuf[8] & 0x80) == 0 ) printf("No error \n");
        *res=lbuf[5]-3;          //*res means now DATA received, strip frame stuff!!
        for(j=0;j<*res;j++) lbufout[j]=lbuf[j+10];
       }
      }
      ;
     } else { printf("Unknown 2nd byte in response %d\n",lbuf[1]);  retcod=-3;
//---debug CRC
       crcword = 0;
       for(k=2;k<lbuf[5]+6;k++) crcword = CRC16(lbuf[k], crcword);
       (*word)=crcword;
       if( (byte[0] != lbuf[lbuf[5]+6]) || (byte[1] != lbuf[lbuf[5]+7]) ) {
        printf("my CRC: %2.2x %2.2x; CRC received: %2.2x %2.2x \n", byte[0],byte[1],
                  lbuf[lbuf[5]+6],lbuf[lbuf[5]+7]);
        printf("  response CRC16 wrong \n");
       }
//---debug CRC
     }

    }
//    printf("got resp:%d:%s \n", *res, lbuf );
    return(retcod);
  }



  int get_response1alt( int globalchain, unsigned char lbufout[256], int* res0, int resin){
  long t1,t2,t12,s1,s2,s12,dt12,ds12;
  unsigned int icon, acon[61];
  unsigned char st1, st2, debug[2560];
//$$
    int j, i, k, *res, retcod, idebug, i2 ;
    unsigned char lbuf[256];
    ushort   crcword;
    int  *word;
    unsigned char byte[4];
    int             res1,res2,res3, iloop ;              //AB

    if(OPENBLACKBOX1 == 0 ) return(-4);


    s12=0;
    t12=0;
    word=&byte;
    retcod=0;
    res=res0;
//    usleep(10000);
    usleep( 2000);
    *res=0;
//  *res = read(fdalt,lbuf,resin );                         //AB more complicated
    j=gettimeofday( tv, tz);
    t1=atv.tv_usec;
    s1=atv.tv_sec-1100100000;

    res1 = 0 ;                                           //AB
    iloop=0;                                             //AB allow for //ha
    for(i=0;i<256;i++) lbuf[i]=-1;
listen:
       res3 = read(fdalt[globalchain],&lbuf[res1],resin-res1);               //AB//ha
       if (res3 < 0  )                                       //AB//ha
       {                                                 //AB
          retcod=res3*10;
          printf("readalt returned error code %d\n",res3); //AB
          goto endlisten;                                //AB
       } else if(res3 == 0 ) {
          iloop++;
       } else {                                          //AB
          if( (res1==0) && (lbuf[res1]==00) ) {
morezero:
           for(k=1;k<res3;k++) lbuf[k-1]=lbuf[k];
           --res3;
           if( (lbuf[res1]==00) && (res3>0) ) goto morezero;
          }
          iloop=0;
          res1 += res3 ;                                 //AB
       }
       if ( res1 < resin) 
        if( iloop<5 )                                    //AB
//      if( iloop<20)                                    //AB
        {
         usleep(10000) ;                                  //AB
         goto listen;                                     //AB
        } else {
         retcod=-2;
        }

if( (globalchain != 0) && (globalchain != 2)) {
 for(k=1;k<res1-1;k++) {
  if( (lbuf[k]==0xfb) && (lbuf[k+1]==0x85) ) {
   res1 += read(fdalt[globalchain],&lbuf[res1],k);
   break;
  }
 }
}

endlisten:

    idebug=0; j=-2;
    sprintf(&debug[idebug],"Ra  :j=%3d, port=%2d; ",j,globalchain); idebug +=21;
    for(i=0;i<res1;i++) {
     sprintf(&debug[idebug],"%2.2x ", lbuf[i] );
     idebug += 3;
    }
    sprintf(&debug[idebug],"\n\0" );
    log_write(fdisk, debug, &i2);

if(res1>resin) {
 for(k=0;k<resin;k++) lbuf[k]=lbuf[k+res1-resin];
 res1=resin;
}

    *res = res1 ;                                        //AB
//  printf("globalchain=%d; Bytesalt received %d, exspected %d \n",globalchain ,*res, resin);
    lbuf[*res]=0;
//  for(j=0;j<*res;j++) {globallbuf[j]=lbuf[j]; printf("%2.2x ",lbuf[ j]);} printf("\n");
    j=gettimeofday( tv, tz);
    t2=atv.tv_usec;
    s2=atv.tv_sec-1100100000;
    if(t2 < t1 ) {
     t2=t2+1000000;
     --s2;
    }
    if(*res <= 0 ) {
//   printf("NO  2b echo, res= %d: at %d sec %d usec \n", *res,s2-s1,t2-t1 );
     return(-1+retcod);
    } 
    printf("  2b echo: %d: at %d sec %d usec \n", *res,s2-s1,t2-t1 );
    ds12=s2-s1;  //-s12;
    dt12=t2-t1;  //-t12;
    if(dt12<0) {
     ds12=ds12-1;
     dt12=1000000+dt12;
    }

    icon=10*ds12 + (dt12/100000);
    if(icon < 0) icon=0;
    if(icon>60) icon=60;
    ++acon[icon];

    if((ds12 > 1) || (dt12>500000) ) {
     printf("connection1 delay %d sec %d usec \n",ds12,dt12);
    }
    if(lbuf[1] == 0x15 ) { 
     printf("NAK %3x %3x \n", lbuf[7],lbuf[8]);
     *res=-3;
    } else {
     if(lbuf[1] == 0x85 ) { 
      if(lbuf[6]+9 != *res) {
       printf("cp error cp+9= %d  *res= %d \n",lbuf[6]+9,*res);
      }
      crcword = 0;
      for(k=2;k<lbuf[5]+6;k++) crcword = CRC16(lbuf[k], crcword);
      (*word)=crcword;
      if( (byte[0] != lbuf[lbuf[6]+7]) || (byte[1] != lbuf[lbuf[6]+8]) ) {
       printf("1my CRC: %2.2x %2.2x; CRC received: %2.2x %2.2x \n", byte[0],byte[1],
                 lbuf[lbuf[6]+7],lbuf[lbuf[6]+8]);
       printf("ACK   response CRC16 wrong \n");
       *res=-2;
//---for now only----------------------------

       if(lbuf[6] > 2 ) {   //got status
        if((lbuf[9] & 0x40) != 0 ) { printf("  Laser on  \n");
        } else                       printf("  Laser off \n");
        if((lbuf[9] & 0x01) != 0 ) {
         printf("  moving ");
         if((lbuf[9] & 0x02) != 0 )  printf(" up   \n");
         else                        printf(" down \n");
        } else {
         if((lbuf[9] & 0x04) != 0 ) {
          if((lbuf[9] & 0x02) != 0 ) { printf(" upper endswitch \n");}
          else printf(" lower endswitch \n");
         }
         else {
          printf(" not moving \n");
         }
        }
        if((lbuf[9] & 0x08) != 0 ) printf(" centered\n");
        if((lbuf[9] & 0x10) != 0 ) printf(" SHT error ");
        if((lbuf[9] & 0x80) != 0 ) printf(" Reset done ");

        *res=lbuf[6]-3;
        for(j=0;j<*res;j++) lbufout[j]=lbuf[j+10];
       }

//-------------------------------------------
      } else {
       printf("ACK   response CRC16 ok ");
       if(lbuf[6] > 2 ) {
        st1=lbuf[8];
        st2=lbuf[9];
// printf("lbuf[6]= %d, lbuf[8]= %x,lbuf[9]= %x \n",lbuf[6],lbuf[8],lbuf[9]);
        if((lbuf[9] & 0x40) != 0 ) { printf("  Laser on  \n");
        } else                       printf("  Laser off \n");
        if((lbuf[9] & 0x01) != 0 ) {
         printf(" moving ");
         if((lbuf[9] & 0x02) != 0 )  printf(" up   \n");
         else                        printf(" down \n");
        } else {
         if((lbuf[9] & 0x04) != 0 ) {
          if((lbuf[9] & 0x02) != 0 ) { printf(" upper endswitch \n");}
          else printf(" lower endswitch \n");
      // }
        } else {
          printf(" x: not moving \n");
         }
        }
        if((lbuf[9] & 0x08) != 0 ) printf(" centered\n");
        if((lbuf[9] & 0x10) != 0 ) printf(" SHT error ");
        if((lbuf[9] & 0x80) != 0 ) printf(" Reset done ");
       } else printf(" \n");
       if(lbuf[8] != 0 ) {
//      printf(" %2.2x \n",lbuf[ 8]);
        if( (lbuf[8] & 0x01) != 0 ) printf("Verges error \n" );
        if( (lbuf[8] & 0x02) != 0 ) printf("Unknown command (cc) \n" );
        if( (lbuf[8] & 0x04) != 0 ) printf("Command rejected, nothing done \n" );
        if( (lbuf[8] & 0x08) != 0 ) printf("Illegal/Meaningless 1. parameter \n" );
        if( (lbuf[8] & 0x10) != 0 ) printf("Illegal/Meaningless 2. parameter \n" );
        if( (lbuf[8] & 0x20) != 0 ) printf("cc and length of frame contradicting \n" );
        if( (lbuf[8] & 0x40) != 0 ) printf("CP and length of frame contradicting \n" );
        if( (lbuf[8] & 0x80) != 0 ) printf("Parameter value out of range \n");
        *res=-1;
       } else {
        printf("No error in alt\n");
        *res=lbuf[6]-3;
        for(j=0;j<*res;j++) lbufout[j]=lbuf[j+10];
//        status1=lbuf[8];
//        status2=lbuf[9];
       }
      }
     } else { printf("Unknown 2nd byte in alt response \n");  retcod=-3;
     }
    }
//    printf("got alt resp:%d:%s \n", *res, lbuf );
    return(retcod);
  }




//=====================================================================


//30.3.05 HA added
int get_response( unsigned char cmd[20], int ip,
                   int resin, unsigned char lbufout[256], int* res0,
                   unsigned char* status1, unsigned char* status2 ){
//(*res) contains the number of valid bytes in lbufout (=data like x/y, freq, etc.; not status)
//(*res) is also set, when retcod is -1 or -2, but data (and status) are not reliable.
//       for retcod = -2:
//               *res= -100:  0th byte not BY0 (start of frame)
//               *res= -101:  1st byte neither ACK nor  NAK
//               *res= -102:  CRC is wrong
//               *res= -103:  address of controller/driver in response different from commands address
//               *res= -104:  command_code in response different from that in the command 
//               *res= -105:  data counter in response (CP) does not correspond to frame length
//
//retcod contains error return code (<=0):
//retcod = -5    retcod did not get set in the routine; no retcod.
//retcod = -4    "read" returned number_of_bytes_read != resin after 3 tries.
//               If *res=0: controller does not exist for communication.?
//retcod = -3    correct NAK response: CRC16 of command was wrong; receiver did not execute the command
//retcod = -2    CRC16 was wrong for response, or: frame header of response (2 bytes) not found
//               or: address in response different from address in command
//retcod = -1    one or more error bit set in status1 and/or status2 (see below)
//retcod =  0    no errors, response ok
// 
//               status1: bit 0: verges error, execution of command done/tried
//                        bit 1: unknown command (inexistent cc)
//                        bit 2: command rejected, nothing done (on too early move/center)
//                        bit 3: illegal/meaningless first parameter (word)
//                        bit 4: illegal/meaningless second/other parameter (word)
//                        bit 5: cc and length of frame (number of params) contradicting
//                        bit 6: cp and length of frame contradicting
//                        bit 7: parameter value out of range
//               status2: bit 0: motor: moving
//                        bit 1: motor: direction up
//                        bit 2: motor: endswitch reached
//                        bit 3: motor: centered
//                        bit 4: SHT sensor read error
//                        bit 5: multi move command: center
//                        bit 6: laser on
//                        bit 7: Reset done
// 
//action:
//retcod  =-4    report: This box is not working
//retcod  =-3    resend command (2 retrys)
//retcod  =-2    send "resend last/latest response"
//-----retcod  =-1 && status2: bit 7 set:  send (slow) "reset_box", do retry of last command
//               status1: bit 0 set:  get temp., humidity, etc. info, NO retry
//                        bit 2 set:  send "query" to see motion; retry when motion finished
//                                    if motion does not finish within 10 sec,
//                                       send (slow) "reset_box", do retry of last command
//                                    if no motion visible: 
//                                       send (slow) "reset_box", do retry of last command
//                        bit 1 and/or bit 3/4/5/6/7 set: software bug: report in log-file
//
//
    int j, i, k, *res, i_rtry, need_byte, retcod, nread;
    unsigned char lbuf[256];
    ushort   crcword;
    unsigned char byte[2];
    long mainusec, mainsec;
    long mainusec0, mainsec0;


    #define NAK 0x15
    #define ACK 0x85
    #define BY0 0xfb
    #define NAKlen 8
    #define resin_err 11  //in some error cases, the box sends 11 bytes (no data)
                          //(when driver does not exist/respond)
    #define n_rtry 3

    printf("get_response %d\n",resin);
//initialize
    for(j=0;j<resin;j++) lbuf[j]=0;
    res=res0;
    retcod=-5;

//read response
    i_rtry=0;
    *res = 0;
    if( blocking == 0 ) {
//   ausleep( 50000);  //<----
     ausleep(  5000);  //<---- //HA 16.6.2005
    }
    j=gettimeofday( tv, tz);
    mainusec0=atv.tv_usec;
    mainsec0=atv.tv_sec ;
r0: 
    j=gettimeofday( tv, tz);
    mainusec0=atv.tv_usec;
    mainsec0=atv.tv_sec ;
    printf("get_response read from ip=%d\n",ip);
usleep(200000);
    nread = read(fd[ip],&lbuf[*res],resin);
printf("get_response:just read %d\n",nread);
    *res = *res + nread;
    i_rtry+=1;
    lbuf[*res]=0;
    lbufout[0]=0;
    for(j=0;j<*res;j++) lbufout[j]=lbuf[ j];
//debug
for(j=0;j<*res;j++) printf("%2.2x ",lbuf[ j]);

//first deal with frame length
  //nothing received
    if(*res <= 0 ) {
     printf("NO response, res= %d: at %d sec %d usec \n", *res, atv.tv_sec, atv.tv_usec);
     retcod=-4;
     if(i_rtry < n_rtry ) {
      *res = 0;
      if( blocking == 0 ) {
       ausleep(r100000);
      } else {
//HA   ausleep(100000);
       ausleep( 50000);
      }
      goto r0;   // try again to receive the response
     }
//report this error
     goto omega;
    } 

//not enough received
    if( *res != resin ) {
//leave the following two cases for later setting of "retcod"
     if((*res == NAKlen) && (lbuf[1] == NAK) ) goto lab_byte0;               //8 is allowed for NAK
     if(( (lbuf[8] & 0x02) != 0 ) && (*res == resin_err)) goto lab_byte0;    //"resin_err" is allowed when
///     if(( (lbuf[9] & 0x80) != 0 ) && (*res == resin_err)) goto lab_byte0;    //"resin_err" is allowed when
                                                                             //driver (box) does not respond
     printf("  response: wrong length: %d, exspected %d \n", *res, resin );
     retcod=-4;
     if(*res < resin) {
      if(i_rtry < n_rtry ) {
       ausleep( 30000);  //<----
       goto r0;   // try again to receive more of the response
      }
//report this error
      goto omega;
     }     // accept up to here frames which are too long
    }

lab_byte0:
//is the first byte a "start_of_frame"?
    if(lbuf[0] != BY0 )   {
     *res = -100;
     retcod=-2;
     goto omega;
    }
//is the second byte an ACK or a NAK?
    if((lbuf[1] != ACK) && (lbuf[1] != NAK) ) {
     *res = -101;
     retcod=-2;
     goto omega;
    }

//check if CRC16 ok
    crcword = 0;
    for(k=2;k<lbuf[6]+7;k++) crcword = CRC16(lbuf[k], crcword);
    byte[0]= crcword & 0x00ff;
    byte[1]=(crcword & 0xff00) >> 8;

  //CRC wrong
    if( (byte[0] != lbuf[lbuf[6]+7]) || (byte[1] != lbuf[lbuf[6]+8]) ) {      //------HA/1/2/08
     *res = -102;
     retcod=-2;
     printf("%2.2x %2.2x    %2.2x %2.2x \n", byte[0],byte[1],
               lbuf[lbuf[6]+7],lbuf[lbuf[6]+8]);
     printf(" response: CRC16 wrong \n");
//

//report this error
     goto omega;
    }

//CRC ok: no problems due to transmission of data

//Did we have transmission problems?
    if(lbuf[1] == NAK ) {           //NAK
 //  qDebug("CRC16 ok and NAK %3x %3x \n", lbuf[7],lbuf[8]);  $ //
     printf("CRC16 ok and NAK %3x %3x \n", lbuf[8],lbuf[9]);
     retcod=-3;
     goto omega;
    }

//Did the addressed controller/driver respond?
    if( (cmd[2] != lbuf[4]) || (cmd[3] != lbuf[5]) ) {
 //  qDebug(" response from other box/driver: b/d echoed %x, %x to b/d sent %x, %x \n", $
 //   lbuf[3], lbuf[4], cmd[2], cmd[3] );                                                $
     printf(" response from other box/driver: b/d echoed %x, %x to b/d sent %x, %x \n",  
      lbuf[4], lbuf[5], cmd[2], cmd[3] );
     *res = -103;
     retcod=-2;
     goto omega;                                            
    }

//Is it the answer to the command sent?
    if( cmd[7] != lbuf[7] ) {
 //  qDebug(" response to other command: function code echoed %x to code sent %x \n", lbuf[6], cmd[6] );$
     printf(" response to other command: function code echoed %x to code sent %x \n", lbuf[7], cmd[7] );
     *res = -104;
     retcod=-2;
//report this error
     goto omega;
    }
    if(lbuf[1] == ACK ) {          // ACK
//     printf("CRC16 ok and ACK --> valid response");
     if(lbuf[6] > 2 ) {
      *status1=lbuf[8];
      *status2=lbuf[9];
     }

     if(lbuf[6]+9 != *res) {
      *res = -105;
      retcod=-2;
      printf("cp error cp+8= %d  *res= %d \n",lbuf[6]+9,*res);
//report this error
     goto omega;
     }
//
     retcod=0;
     if(lbuf[6] > 2 ) {
      *res=lbuf[6]-3;
      for(j=0;j<*res;j++) lbufout[j]=lbuf[j+9];
//      if( (*status2 & 0x80) != 0 ) retcod=-1;
      if(*status1 != 0 )           retcod=-1;
     } // else printf(" \n");

    }   //ACK
    j=gettimeofday( tv, tz);
    mainusec=atv.tv_usec;
    mainsec =atv.tv_sec ;
    printf("get_respose: omega, %d %d\n",mainusec0,mainusec);
    mainusec=(mainusec-mainusec0)/1000.+1000.*(mainsec-mainsec0);
//  printf("get_respose: omega, %d\n",mainusec);

omega: 
printf("omega:s1: %2.2x, s2: %2.2x \n", *status1, *status2);  //debugging
    return(retcod);
                                                                
}


//-----------------------------------------------------------------------------------


void interpret_s1(int ip){

     if(status1[ip] != 0 ) {
      if( (status1[ip] & 0x01) != 0 ) printf(" Verges error \n" );
      if( (status1[ip] & 0x02) != 0 ) printf(" Unknown command (cc) \n" );
      if( (status1[ip] & 0x04) != 0 ) printf(" Command rejected, nothing done \n" );
      if( (status1[ip] & 0x08) != 0 ) printf(" Illegal/Meaningless 1. parameter \n" );
      if( (status1[ip] & 0x10) != 0 ) printf(" Illegal/Meaningless 2. parameter \n" );
      if( (status1[ip] & 0x20) != 0 ) printf(" cc and length of frame contradicting \n" );
      if( (status1[ip] & 0x40) != 0 ) printf(" cp and length of frame contradicting \n" );
      if( (status1[ip] & 0x80) != 0 ) printf(" Parameter value out of range \n" );
     } else {
//    if( (status2[ip] & 0x80) == 0 ) printf(" No error \n");
      printf(" No error \n");
     }
}


//-----------------------------------------------------------------------------------


void interpret_s2(int ip){

//      if( (status2[ip] & 0x80) != 0 ) {
//       printf(" No response from driver; serious hardware problem \n" );
//      }
//Laser on/off
      if((status2[ip] & 0x40) != 0 ) { printf("  Laser on  \n");
      } else                           printf("  Laser off \n");
//motion
      if((status2[ip] & 0x01) != 0 ) {
       printf(" motor moving ");
       if((status2[ip] & 0x02) != 0 )  printf(" up   \n");
       else                            printf(" down \n");
      } else {
       if((status2[ip] & 0x04) != 0 ) {
        printf(" lower  endswitch ");
       } else if((status2[ip] & 0x08) != 0 ) {
        printf(" upper  endswitch ");
       } else {
        printf(" not moving \n");
       }
      }

}



//-----------------------------------------------------------------------------------

/************************************************************************/
/*                                                                      */
/* ushort CDCPImpl::CRC16(uchar ch, ushort oldCRC) const                */
/* Description:            - Calculates CRC using the old one as well   */
/*                           as one byte data provided.                 */
/* Arguments:                                                           */
/* chByte                  - Byte on which to calculate CRC.            */
/* oldCRC                  - Old CRC                                    */
/* Return Value:           - New CRC (16 bit).                          */
/************************************************************************/

// CRC holder initially must be set to 0
#define DEFAULT_CRC_POLY 0x1021

unsigned short CRC16(unsigned char chByte, unsigned short oldCRC)
{
 int  chMSB,i;
 oldCRC ^= (unsigned short)(chByte << 8);
 for (i = 0; i < 8; ++i)
 {
  chMSB = (oldCRC & 0x8000) ? TRUE : FALSE;
  oldCRC <<= 1;
  if (!chMSB) continue;
  oldCRC        ^= (unsigned short) DEFAULT_CRC_POLY;
 }
 return oldCRC;
}

//=--------------------------------------------------------------------------

int send_command(int ip, unsigned char *comd, AMCpanel* ptr ) {
  int        res,nofbyte,nread,j,k, res0,res1,res2,res3,iloop,ii,idebug,i2,it;
  unsigned char rescmd[256], newcmd[1200], *comd0;
  ushort   crcword;
  unsigned char debug[2560];
  int  *word;
  unsigned char byte[4];


  printf("send_command(ip= %d ==================%2.2x \n",ip,(*(comd+6))); //@
  if(ip>8) sleep(2);                                     //@
  word=&byte;
  byte[0]=(*(comd+2));
  byte[1]=(*(comd+3));
  byte[2]=0;
  byte[3]=0;
  for(it=0;it<3;it++){
   if( (*word) == (*ptr).addr  ) goto it_break;
  }
 it=2;
it_break: ;

  nofbyte=8+(*(comd+5));
  res=0; nread=0;

//flush buffers
flushagain:
  res1=0;
  ioctl(fd[ip], FIONREAD, &res1);   //get $ of bytes to flush
  if(res1>0) {
   if(res1>1024) res1=1023;
   res2=read(fd[ip],&newcmd[0],res1);
   printf("send: flushed %d bytes %2.2x\n",res2,newcmd[0]);
//----
   idebug=0; j=-2;
   sprintf(&debug[idebug],"F   :j=%3d, port=%2d; ",j, ip); idebug +=21;
   for(i=0;i<imin(100,res2);i++) {
    sprintf(&debug[idebug],"%2.2x ", newcmd[i] );
    idebug += 3;
   }
   sprintf(&debug[idebug],"\n\0" );
   log_write(fdisk, debug, &i2);
//---
   if(res1>1000) goto flushagain;
  }


goto noaltport1;
flushaltagain:
  ioctl(fdalt[ip], FIONREAD, &res1);   //get $ of bytes to flush in alt channel
  if(res1>0) {
   if(res1>1024) res1=1023;
   res2=read(fdalt[ip],&newcmd[0],res1);
//----
   idebug=0; j=-2;
   sprintf(&debug[idebug],"Fa  :j=%3d, port=%2d; ",j, ip); idebug +=21;
   for(i=0;i<imin(100,res2);i++) {
    sprintf(&debug[idebug],"%2.2x ", newcmd[i] );
    idebug += 3;
   }
   sprintf(&debug[idebug],"\n\0" );
   log_write(fdisk, debug, &i2);
//---
   if(res1>1000) goto flushaltagain;
  }
noaltport1:;

//printf("crucial:sending %d bytes to ip=%d\n",nofbyte,ip);
//printf("amcmotor:crucial write to fd[%d]=%d\n",ip,fd[ip]);
//for(i=0;i<nofbyte;i++) printf("%2.2x ", comd[i] ); printf("\n");

//res = write(fd[ip], comd, nofbyte);   //added 28.10.09 HA


//  res = write(fd[ip], comd, nofbyte+1); //added 28.10.09 HA
  res = write(fd[ip], comd-1, nofbyte+2); //added 16.11.09 HA


//  printf("amcmotor:crucial write retcode %d\n",res);
  idebug=0; j=-2;
  sprintf(&debug[idebug],"sW  :j=%3d, port=%2d; ",j,ip); idebug +=21;
  for(i=0;i<nofbyte;i++) {
   sprintf(&debug[idebug],"%2.2x ", comd[i] );
   idebug += 3;
  }
  sprintf(&debug[idebug],"\n\0" );
  log_write(fdisk, debug, &i2);


  if(ip <= 8 ) goto standard_channel;  //skip alternate channel

// read 1st echo 
//alt channel
again1:
  for(j=0;j<res+4;j++) newcmd[j]=0xff;                  //HA
  res2 = res ;                                         //AB
  res0 = 0 ;                                           //AB


goto noaltport2;
ausleep( 900);
iloop=11;
onemore1:
  ioctl(fdalt[ip], FIONREAD, &res1);   //get # of bytes available in alt channel
  iloop--;
  if((res1 < res) && iloop>0) {
   usleep( 2000);
//   usleep(10000);
   goto onemore1;
  }
//  printf("ip=%d, iloop %d ms,  res1 %d\n",ip,(10-iloop)*10,res1);

  res1=0;
  for (iloop=0 ;; iloop++)                             //AB allow for
  {                                                    //AB some delay
     res3 = read(fdalt[ip],&newcmd[res0],res2+res1);               //AB
//--   >> if (res3 < 0 && iloop >= 50 )                     //AB
     if (res3 < 0 && iloop >= 5  )                     //AB
     {                                                 //AB
//        printf("alt connection problems (recv) %d, fdalt[%d] %d  ",res3,ip,fdalt[ip]);//AB
//        printf("  res0,res2 %d %d \n",     res0,res2);          //AB
        goto again;
     }                                                 //AB
     if (res3 == 0 && iloop >= 10 ) break;
     if (res3 >0 )                                     //AB
     {                                                 //AB
        res0 += res3 ;                                 //AB
        res2 -= res3 ;                                 //AB
//      for(j=0;j<res0;j++) printf("%2.2x ", newcmd[j]);//AB
//      printf("alt with zero: %d %d\n",res0,res);        //AB
        for(j=0;j<res0;j++) {
         if(newcmd[j]!=00) break;
         res1=j+1;
        }
//    printf("alt new res1 %d\n",res1);
     }                                                 //AB
//   printf("... %d %d %d %d\n",res3,res0,res2,res);   //AB
     if (res0 == res+res1)                                  //AB
     {                                                 //AB
      for(j=0;j<res;j++) newcmd[j]=newcmd[j+res1];
//      for(j=0;j<res+2;j++) printf("%2.2x ", newcmd[j]);
//      printf("   1b. echo: %d %d res1 %d\n",res0-res1,res,res1);
      for(j=0;j<res;j++)                             //HA
       if(newcmd[j] != *(comd+j)) {                  //HA
//       printf("wrong 1b. echo\n");                   //HA
      }                                             //

      globalnech1b++;
      (*ptr).ech1_cnt[it][1]++;
      goto again;
     }                                                 //AB
     usleep(100000);                                   //AB//ha
  }                                                    //AB
again:
  idebug=0; j=-2;
  sprintf(&debug[idebug],"Ea  :j=%3d, port=%2d; ",j,ip); idebug +=21;
  for(i=0;i<res+2;i++) {
   sprintf(&debug[idebug],"%2.2x ", newcmd[i] );
   idebug += 3;                       
  }
  sprintf(&debug[idebug],"\n\0" );
  log_write(fdisk, debug, &i2);
noaltport2:;


//0-----------------------------------------------------

standard_channel:
  ii=-1; 
  AMC_check_gui( ii, AMC_scroll);

  if(ip <= 8 ) goto no_echo1a;

//standard channel
  for(j=0;j<res+2;j++) newcmd[j]=0xff;                   //HA
  res2 = res ;                                         //AB
  res0 = 0 ;                                           //AB

iloop=11;
onemore:
  ioctl(fd[ip], FIONREAD, &res1);   //get $ of bytes available in dev2
  iloop--;
  if((res1 < res) && iloop>0) {
   if( (ip == 1) || (ip==3) )  usleep(10000);
   usleep( 20000);

//  printf("reading echo 1: found %d bytes to read\n",res1);
// sleep( 1);

   goto onemore;
  }
//  printf("reading echo 1: found %d bytes to read\n",res1);


  res1=0;
  for (iloop=0 ;; iloop++)                             //AB allow for
  {                                                    //AB some delay
     res3 = read(fd[ip],&newcmd[res0],res2+res1);               //AB
//     printf("port: fd[%d]=%d res3 %d  res %d  iloop %d\n",ip,fd[ip],res3,res,iloop);
//     printf("res3 %d  res %d  iloop %d\n",res3,res,iloop);
     if (res3 < 0 && iloop == 11 )                     //AB
     {                                                 //AB
//        printf("connection problems (recv) %d  ",res3);//AB
//        printf("   %d %d \n",     res0,res2);          //AB
        return -222 ;                                  //AB
     }                                                 //AB
     if (res3 == 0 && iloop >= 10 ) break;
     if (res3 >0 )                                     //AB
     {                                                 //AB
        res0 += res3 ;                                 //AB
        res2 -= res3 ;                                 //AB
        for(j=0;j<res0;j++) {
         if(newcmd[j]!=00) break;
         res1=j+1;
        }
     }                                                 //AB
     if (res0 == res+res1)                                  //AB
     {                                                 //AB
      for(j=0;j<res;j++) newcmd[j]=newcmd[j+res1];
//        for(j=0;j<res+2;j++) printf("%2.2x ", newcmd[j]);//AB
//        printf("   1. echo: %d %d res1 %d\n",res0-res1,res,res1);        //AB
        for(j=0;j<res;j++)                             //HA
         if(newcmd[j] != *(comd+j)) {                  //HA
//          printf("wrong 1. echo\n");                   //HA
         }                                             //
        globalnech1a++;
        (*ptr).ech1_cnt[it][0]++;
        goto again2;                                  //AB
     }                                                 //AB
//     usleep(10000);                                    //AB
     usleep(100000);                                   //AB//ha
  }          //iloop
again2:
  idebug=0; j=-2;
  sprintf(&debug[idebug],"E   :j=%3d, port=%2d; ",j,ip); idebug +=21;
  for(i=0;i<res+2;i++) {
   sprintf(&debug[idebug],"%2.2x ", newcmd[i] );
   idebug += 3;                       
  }
  sprintf(&debug[idebug],"\n\0" );
  log_write(fdisk, debug, &i2);
no_echo1a:;

  n_sndcmd+=1;
//  for(k=0;k<11;k++) printf("%2.2x ", *(comd+k) ); printf(" sent\n" );

  return(res);
}


//=====================================================================


/** Calculate the time needed by the motors to complete the number of steps.
 * We add 200 ms for the communications overhead. The returned value
 * is the next high integer number of seconds.
 */
int $calc_timeout( int ip, AMCpanel* ptr, int it, int p_iSteps )
{
 int frequency, steps, ib;
 int iTimeout;
 // timeout for negative number of steps
 if( p_iSteps < 0)
  return(0);

 steps=p_iSteps;                  //HA 12.5.05 steps <= center steps
 if( steps > 17500) steps=17500;

// if( m_iCurrFreq <= 0 ) {
  read_frequency( ip, ptr, it, &frequency );
//  m_iCurrFreq = frequency;
// }

 // The timeout is calculated by deviding the number of steps by the frequency.
 // We add 200ms to account for the communications overhead.
 // We convert the result to the next higher integer and return the timeout
 // in seconds.
//HA int iTimeout = (int) ceil( ((double) p_iSteps / (double) frequency) + 0.2 );
     iTimeout = (int) ceil( ((double) steps / (double) frequency) + 0.2 );

 if(iTimeout > 15) iTimeout=15;   //HA 12.5.05 max timeout = center timeout

 return(iTimeout);

}

//=====================================================================


//=====================================================================

/** Wait for the motor movement to complete. */                                         //HA 26.4.05

/*

void $waitForMotors_new( int ip, AMCpanel* ptr, int it, int p_iTimeout , int *ixpos, int *iypos)  //AB
{
 QTime qtStop, qtStart;
 QCString sComm, sResp;
 char ch;
 int x, y, intrv, ib;

 ib = (*ptr).addr[it];
 *ixpos = 99999 ;
 *iypos = 99999 ;

 intrv=50000*p_iTimeout;      //HA
 if(intrv <= 30000) intrv=30000;  //HA
 qtStart.start();
 qtStop = qtStart.addSecs( p_iTimeout );
 // near midnight the time may wrap. In this case we sleep
 // timeout seconds to be sure to be in the next day.
 if (qtStop < qtStart) sleep( p_iTimeout );

int n_again = 3; //HA 10.5.05
for_again:
 for(;;)
 {
//Check for a timeout during centering/ HA: do it at the end of the loop

   query_absxy( ip, ptr, &x, &y );
   *ixpos = x ;
   *iypos = y ;

//debug
// printf("status1 %2.2x, status2 %2.2x \n",status1[ip], status2[ip]);
//old logic
//not moving, not endswitch
//  if( (status2[ip]&0x01) == 0 && (status2[ip]&0x0C) == 0) break;    //HA 11.5.05
//new logic
  if( (status2[ip]&0x01) == 0 || (status2[ip]&0x0C) != 0) break;    //HA 11.5.05

  if( qtStart.elapsed() > p_iTimeout*1000 )   //think in milliseconds
  {
   // Kill the current movemnet
   kill_motion( ip, ib, it);
//HA 10.5.05 reset_box( ib, id); //<----------------------
   // throw the exception
//$   AMCError aErr( IDS_ERROR_MOVEMENT_TIMEOUT, m_iCurrDriver );
//   throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
  }

  ausleep(intrv);                //HA 31.3.2005

 }  //for

 ausleep(intrv);                        //HA 28.4.05
 query_absxy( ip, ptr, &x, &y );
//old logic
// if( (status2[ip]&0x01) != 0 || (status2[ip]&0x0C) != 0 )
//new logic
 if( (status2[ip]&0x01) != 0 && (status2[ip]&0x0C) == 0 )
                    if( --n_again >= 0 ) goto for_again;  //HA 10.5.05
 *ixpos = x ;
 *iypos = y ;

return;

}
  */
//=====================================================================

//=::==================================================================
//31.3.05 HA added the following code and the corresponding calls


int do_retry(int retcod, int ip, AMCpanel* ptr, int it, char err_mess[100], int resp)   {
//returns:  0 verges error; do nothing at the moment; accept as response ok
//         -1 resend previous command if s_retry<3 else throw error
//         -2 send "resend last command" if r_retry<3 else throw error
//         -3 throw error
//         -4 send "resend last command"  if r_retry<3
//                  else resend previous command if s_retry<3
//                       else throw error

int rtrycod, j;
int x_intermed , y_intermed;
int x_intermed1, y_intermed1;
static int reseted_driver =0;

     err_mess[0]=0;
     rtrycod=0;
     switch( retcod ) {
      case (-1) :
/* no more used
       if( (status2[ip] & 0x80) != 0 ) {
        sprintf(err_mess, "3x no response from driver " );
        if(reseted_driver == 0 ) {
         reseted_driver=1;
         reset_driver(ip, ptr, it);
         rtrycod=-1;
         return(rtrycod);
        } else {
         reseted_driver=0;
         rtrycod=-3;
         return(rtrycod);
        }
       }
 */

       if( (status1[ip] & 0x01) !=0 )  {
// verges error
        rtrycod=0;
        return(rtrycod);
       }

       if( (status1[ip] & 0x04) != 0) {
//        aErr.setErrorText( "Command rejected " );
        printf("Command rejected \n" );
        sprintf(err_mess, "Command rejected " );
        rtrycod=-1;
//check if rejection of command is due to motion in progress
        if( (status2[ip] & 0x01) != 0 ) {
         for(j=0;j<10;j++) {
          query_absxy(ip, ptr, &x_intermed, &y_intermed );
          if( (status2[ip] & 0x01) == 0 ) return(rtrycod);
          ausleep(50000);
//        ausleep(500000);
          query_absxy(ip, ptr, &x_intermed1, &y_intermed1 );
          if( (status2[ip] & 0x01) == 0 ) return(rtrycod);
          if( (x_intermed == x_intermed1) && (y_intermed == y_intermed1) ) {
//           aErr.setErrorText( "Rejected: says moving, but isn't " );
           sprintf(err_mess, "Rejected: says moving, but isn't " );
           goto lab_rej;
          }
          ausleep(50000);
//        ausleep(500000);
         } //for
        }  //moving
lab_rej:
        reset_box(ip, ptr, it);
        rtrycod=-3;
        return(rtrycod);
       }   //rejected

       if( status1[ip]  != 0) {
//        aErr.setErrorText( "SW bug; check status1 " );
        sprintf(err_mess, "SW bug; check status1 %2.2x",status1[ip] );
        rtrycod=-3;
        return(rtrycod);
       }   //software bug
       break;

      case (-2) :
//       aErr.setErrorText( "3x wrong response:code %d ", *res );
//     sprintf(err_mess, "3x wrong response " );     //output also *res 100/101,etc ??
       sprintf(err_mess, "3x wrong response: code %d ", resp );
       rtrycod=-2;
       return(rtrycod);
       break;

      case (-3) :
//       aErr.setErrorText( "3x NAK received     " );
       sprintf(err_mess, "3x NAK received     " );
       rtrycod=-2;
       return(rtrycod);
       break;

      case (-4) :
//       aErr.setErrorText( "3x no or too short response received " );
       sprintf(err_mess, "3x no or too short response received n=%d", resp );
       rtrycod=-4;
       return(rtrycod);
       break;
     }
     return(rtrycod);
}


//-----------------------------------------------------------------------------------

int write_I_working(int iip, AMCpanel* ptr, int it, int current ) {
    int resp, rtrycod, retcod, r_retry, s_retry, k;
//    static int n_byte_resp;              //HA 29.4.05 remove the "static" in all functions
//    static unsigned char cmdframe[20];
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256];
    char chsend[20], chrecv[60], err_mess[100];

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it]+1;
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x15);
    cmdframe[6]= 0x03;
    cmdframe[8]=  current & 0x00ff;
    cmdframe[9]= (current & 0xff00) >> 8;
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}
//-----------------------------------------------------------------------------------

int read_I_working(int iip, AMCpanel* ptr, int it, int* current ) {
    int n_byte_resp, k, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, nwd, r_retry, s_retry;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x14);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=14;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }
     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    if(retcod == 0 ) {
     nwd= get_int( resp, lbufrsp, current);
    }

    return(retcod);
}



//-----------------------------------------------------------------------------------

int write_I_holding(int iip, AMCpanel* ptr, int it, int current ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x18);
    cmdframe[6]= 0x03;
    cmdframe[8]=  current & 0x00ff;
    cmdframe[9]= (current & 0xff00) >> 8;
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}

//-----------------------------------------------------------------------------------

int read_I_holding(int iip, AMCpanel* ptr, int it, int* current ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, nwd, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x17);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=14;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    if(retcod == 0 ) {
     nwd= get_int( resp, lbufrsp, current);
    }

    return(retcod);
}


//-----------------------------------------------------------------------------------

int write_frequency(int iip, AMCpanel* ptr, int it, int frequency ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x12);
    cmdframe[6]= 0x03;
    cmdframe[8]= frequency & 0x00ff;
    cmdframe[9]= (frequency & 0xff00) >> 8;
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}

//-----------------------------------------------------------------------------------

int read_frequency(int iip, AMCpanel* ptr, int it, int* frequency ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, nwd, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
//preset frequency to 1400
    *frequency = 1400;     //HA 28.4.05

    init_frame(cmdframe, ib, 0x11);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=14;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    if(retcod == 0 ) {
     nwd= get_int( resp, lbufrsp, frequency);
    }

    return(retcod);
}

//-----------------------------------------------------------------------------------

int read_temp(int iip, AMCpanel* ptr, int it, int wrdbuf[10] ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, nwd, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x1F);
//    cmdframe[7]= 0x1F;  //command code
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=16;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(320000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    if(retcod == 0 ) {
     nwd= get_int( resp, lbufrsp, wrdbuf);
     if(nwd == 6 ) {
//      printf("Temperature %d \n",wrdbuf[0]);
//      printf("Humidity    %d \n",wrdbuf[1]);
//      printf("Current     %d \n",wrdbuf[2]);
//      printf("Spl Voltage %d \n",wrdbuf[3]);
//      printf("2. Sup Volt %d \n",wrdbuf[4]);
//      printf("Log Voltage %d \n",wrdbuf[5]);
     }
    }

    return(retcod);
}

//-----------------------------------------------------------------------------------


int query_absxy(int iip, AMCpanel* ptr, int* x, int* y ) {
    int retval[5];
    int n_byte_resp, ib, ip, it, port, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, nwd, r_retry, s_retry, k;

   for(it=0;it<2;it++) {
#include "getip.inc"
    port = ip;
//    pf = (*ptr).portflg[it];
//    if(pf>1) pf=0;
//    if(pf<0) pf=0;
//    port = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x21);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=14;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
    *x=99999 ; //AB
    *y=99999 ; //AB
resend:
    send_command( port, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, port, n_byte_resp, lbufrsp, &resp,
                     &status1[port], &status2[port] );

    if((status2[port] & 0x30) != 0)
       printf("SSS query: status2 %2.2x, b %d d %d\n",status2[port], ib,it);  //HA 10.5.05

    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, port, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( port, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( port, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    if(retcod == 0) {
     nwd= get_int( resp, lbufrsp, retval);
     if(nwd > 0 ) {
      if(it == 0) *x=retval[0];
      else        *y=retval[0];
     }
    }
   } //end for(it=0;
     printf(" query\n");
    return(retcod);
}


//-----------------------------------------------------------------------------------


int read_version_nr(int iip, AMCpanel* ptr, int it, unsigned char version[2] ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0xFE);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=13;     //# bytes exspected in response

    version[0] = 0;
    version[1] = 0;
    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    if(retcod == 0 ) {
     version[0] = lbufrsp[0];
     version[1] = lbufrsp[1];
    }

    return(retcod);
}


//-----------------------------------------------------------------------------------


int resend_last(int iip, AMCpanel* ptr, int it, int resin ) {
    int resp, retcod = 0, k;
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0xFF);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=resin;     //# bytes exspected in response

    send_command( ip, cmdframe, ptr );
    ausleep(n5000);
//no get_response here!!!!           
    return(retcod);
}


//-----------------------------------------------------------------------------------


int reset_box(int iip, AMCpanel* ptr, int it ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    printf("reset_box ip %d, ib %d, it %d \n",ip, ib, it);

    init_frame(cmdframe, ib, 0x09);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
//  if( ip == 0 )  ausleep(900000);   //<-----------
    if( ip == 0 )  ausleep(9000);   //<-----------
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

//Dont do "retry": It could loop tightly
//     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
//     switch( rtrycod ) {
//      case ( 0) : goto nothing;
//      case (-1) : if( s_retry < 3 ) goto resend;  break;
//      case (-2) :
//       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
//      case (-3) :                                 break;
//      case (-4) :
//       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
//       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
//     }

//$     aErr.setErrorText( "Reset Box failed" );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

//nothing:
    return(retcod);
}


//-----------------------------------------------------------------------------------


int center_motors(int iip, AMCpanel* ptr, int it ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
//  init_frame(cmdframe, ib, 0x07);
    init_frame(cmdframe, ib, 0x08);  // new centre
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
  motorx[0][ib][iip]=0;    //for now, delete later  $$$
  motorx[1][ib][iip]=0;
  motexx[0][ib][iip]=0;
  motexx[1][ib][iip]=0;
    return(retcod);
}




//-----------------------------------------------------------------------------------

int move_xyabs(int iip, AMCpanel* ptr, int x, int y ) {    //steps[2]$
    int n_byte_resp, ib, it, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

//HAS TO BE CHANGED to 2 CALLS move_xabs and move_yabs


   printf("move_xyabs: x %d, y %d\n",x, y);
   for(it=0; it<2; it++) {
#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x01);
    cmdframe[6]= 0x03;
    cmdframe[9]=  x & 0x00ff;
    cmdframe[10]= (x & 0xff00) >> 8;
//    cmdframe[11]=   y & 0x00ff;
//    cmdframe[12]= (y & 0xff00) >> 8;
    if(it>0) {
     cmdframe[9]=  y & 0x00ff;
     cmdframe[10]= (y & 0xff00) >> 8;
    }
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

//  motexx[it][ib][iip]=x;
//  motexy[it][ib][iip]=y;

    (*ptr).act_mot[0]=x;
    (*ptr).act_mot[1]=y;

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    printf("\n");
   }
    return(retcod);
}



//-----------------------------------------------------------------------------------

int move_xy(int iip, AMCpanel* ptr, int x, int y ) {    //steps[2]$
    int n_byte_resp, ib, it, xx, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

//HAS TO BE CHANGED to 2 CALLS move_xrel and move_yrel  /code 1 / two addresses!!

   printf("move_xy: x %d, y %d\n",x, y);
   for(it=0; it<2; it++) {
#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    xx=x; if(it>0) xx=y;
    printf("move_xy: it %d. step %d\n",it,xx);
    if(abs(xx)<2) continue;   //next "it"
    printf("move_xy: it %d\n",it);
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x01);
    cmdframe[6]= 0x03;
    cmdframe[8]=  x & 0x00ff;
    cmdframe[9]= (x & 0xff00) >> 8;
//    cmdframe[11]=   y & 0x00ff;
//    cmdframe[12]= (y & 0xff00) >> 8;
    if(it>0) {
     cmdframe[8]=  y & 0x00ff;
     cmdframe[9]= (y & 0xff00) >> 8;
    }
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    motexx[0][ib][iip]=motorx[0][ib][iip]+x;
    motexx[1][ib][iip]=motorx[1][ib][iip]+y;

    r_retry=0;
    s_retry=0;
resend:
   printf("move_xy: send command\n");
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    printf("\n");
   }
    return(retcod);
}



//-----------------------------------------------------------------------------------

int move_x(int iip, AMCpanel* ptr, int it, int x, int y ) {    //steps[2]$
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
//  init_framex(cmdframe, ib, 0x01);
    cmdframe[6]= 0x03;
    cmdframe[9]=  x & 0x00ff;
    cmdframe[10]= (x & 0xff00) >> 8;
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    motexx[it][ib][iip]=motorx[it][ib][iip]+x;

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}


//-----------------------------------------------------------------------------------


int reset_driver(int iip, AMCpanel* ptr, int it ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

//DOES NO MORE EXIST


#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    printf("reset_driver ip %d, ib %d, it %d \n",ip, ib, it);

    init_frame(cmdframe, ib, 0x0A);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
//  if(ip == 0) ausleep(900000);   //<-------
    if(ip == 0) ausleep(9000);   //<-------
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

//dont do retry for "reset": it loops tightly, when the driver does not exist
//     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
//     switch( rtrycod ) {
//      case ( 0) : goto nothing;
//      case (-1) : if( s_retry < 3 ) goto resend;  break;
//      case (-2) :
//       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
//      case (-3) :                                 break;
//      case (-4) :
//       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
//       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
//     }

//$     aErr.setErrorText("Reset Driver failed" );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}


//-----------------------------------------------------------------------------------


int kill_motion(int iip, AMCpanel* ptr, int it ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x0C);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}


//-----------------------------------------------------------------------------------


int power_on(int iip, AMCpanel* ptr, int it ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x31);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}

//-----------------------------------------------------------------------------------

int power_off(int iip, AMCpanel* ptr, int it) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    init_frame(cmdframe, ib, 0x30);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}


//-----------------------------------------------------------------------------------


int laser_on_parallel(int iip, AMCpanel* ptr, int* subcase ) {
    static unsigned char cmdframe[nport][20];
    static char chsend[nport][20], chrecv[nport][60], err_mess[nport][100];
    static r_retry[nport], s_retry[nport];
    static long mainusec0[nport], mainsec0[nport];
    static int  res[nport],nofbyte,nread;
    int n_byte_resp, ib, it, ip, pf;
    unsigned char lbufrsp[256]; 
    int  resp, rtrycod, retcod, k;
    long mainusec, mainsec;
    unsigned char rescmd[256], newcmd[256];

    it=2;
#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];

    if(*subcase == 0 ) *subcase = 1 ;
    printf("laser_on_parallel ip %d, ib %d, it %d; subcase %d \n",ip, ib, it,*subcase);

    if(*subcase == 1 ) {
     init_frame(cmdframe[iip], ib, 0x60);
     fill_CRC(cmdframe[iip]);
     for(k=2;k<8;k++) sprintf(&chsend[iip][(k-2)*3],"%2.2x ", cmdframe[iip][k] );
     sprintf(&chsend[iip][(k-2)*3],"\n" );

     r_retry[iip]=0;
     s_retry[iip]=0;
resend:
     gettimeofday( tv, tz);
     mainusec0[iip]=atv.tv_usec;
     mainsec0[iip]=atv.tv_sec ;
     printf("laser_on_parallel (re)send %d %d\n",s_retry[iip],atv.tv_usec);

//    send_command( ip, cmdframe, ptr );
//------------------------$$-

     nofbyte=9+cmdframe[iip][6];
     res[iip]=0; nread=0;
     tcflush(fd[ip], TCIOFLUSH);
printf("crucial:laspar:sending %d bytes to ip=%d\n",nofbyte,ip);
     res[iip] = write(fd[ip], cmdframe[iip], nofbyte);
     s_retry[iip]+=1;
     for(k=0;k<11;k++) printf("%2.2x ", cmdframe[iip][k]); printf("\n" );
     *subcase=2;
     return(0);
    }

//============================================
    if(*subcase == 2 ) {
// read 1st echo 
     if(iip == 0) {
      k=50;
wait_again:
      if(blocking == 0 ) {
//   ausleep(10000);  //HA 7.6.05
       ausleep( 1000);  //HA 16.6.05
       if( (ready_response( ip, res[iip]) < 0) && (k-- > 0) )goto wait_again;
      }
      nread=read(fd[ip], newcmd, res[iip]);
     }
     for(k=0;k<11;k++) printf("%2.2x ", newcmd[k]); printf("\n" );
     *subcase=3;
     return(0);
    }    
//-------------------------

    if(*subcase == 3 ) {
     ausleep(n5000);
reread:
     gettimeofday( tv, tz);
     mainusec=atv.tv_usec;
     mainsec=atv.tv_sec ;
     mainusec=(mainusec-mainusec0[ip])/1000.+1000.*(mainsec-mainsec0[iip]);
     status1[iip]=0;  status2[iip]=0;
     n_byte_resp=11;     //# bytes exspected in response
     retcod=get_response(cmdframe[iip], ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
     gettimeofday( tv, tz);
     mainusec=atv.tv_usec;
     mainsec=atv.tv_sec ;
//    mainusec=(mainusec-mainusec0)/1000.+1000.*(mainsec-mainsec0);
     r_retry[iip]+=1;
     if(retcod < 0 ) {
      chrecv[iip][0]=lbufrsp[0];
      if(chrecv[iip][0] != 0) {
       for(k=0;k<12;k++) sprintf(&chrecv[iip][(k  )*3],"%2.2x ", lbufrsp[k] );
       sprintf(&chrecv[iip][(k  )*3],"\n" );
      }

      rtrycod=do_retry(retcod, ip, ptr, it, err_mess[ip], resp);
      switch( rtrycod ) {
       case ( 0) : goto nothing;
       case (-1) : if( s_retry[iip] < 3 ) goto resend;  break;
       case (-2) :
        if(r_retry[ip] < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
       case (-3) :                                 break;
       case (-4) :
        if(r_retry[iip] < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
        if(s_retry[iip] < 3 ) {ausleep(r100000); *subcase = 1; goto resend;}        break;
      }
//$      aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
      printf(" %s |\n",err_mess[iip]);
      printf(" %s |\n",chsend[iip]);
      printf(" %s |\n",chrecv[iip]);
      retcod=-9999;
     }
    }

nothing:
     *subcase = 0;
     return(retcod);
}
//end laser_on_parallel 

//-----------------------------------------------------------------------------------


int laser_on(int iip, AMCpanel* ptr ) {
    int n_byte_resp, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k, it;
    long mainusec, mainsec;
    long mainusec0, mainsec0;

    it=2;
#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    printf("laser_on ip %d, ib %2.2x \n",ip, ib);
    init_frame(cmdframe, ib, 0x60);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    gettimeofday( tv, tz);
    mainusec0=atv.tv_usec;
    mainsec0=atv.tv_sec ;
    printf("laser_on (re)send at %d usec\n",atv.tv_usec);
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
    gettimeofday( tv, tz);
    mainusec=atv.tv_usec;
    mainsec=atv.tv_sec ;
    mainusec=(mainusec-mainusec0)/1000.+1000.*(mainsec-mainsec0);
    printf("laser_on (re)read %d %d\n",mainusec, atv.tv_usec);
    status1[iip]=0;  status2[iip]=0;
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
    gettimeofday( tv, tz);
    mainusec=atv.tv_usec;
    mainsec=atv.tv_sec ;
//    mainusec=(mainusec-mainusec0)/1000.+1000.*(mainsec-mainsec0);
    printf("laser_on get_resp retcod %d, %d\n",retcod,mainusec);
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s |\n",err_mess);
     printf(" %s |\n",chsend);
     printf(" %s |\n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}

//-----------------------------------------------------------------------------------

int laser_off(int iip, AMCpanel* ptr) {
    int n_byte_resp, it, ib, ip, pf;
    unsigned char cmdframe[20];
    unsigned char lbufrsp[256]; 
    char chsend[20], chrecv[60], err_mess[100];
    int resp, rtrycod, retcod, r_retry, s_retry, k;

    it=2;
#include "getip.inc"
//    pf = (*ptr).portflg[it];
//    if(pf != 0) pf=1;
//    ip = (*ptr).port[pf][it];
    ib = (*ptr).addr[it];
    printf("laser_off ip %d, ib %d, it %d \n",ip, ib, it);
    init_frame(cmdframe, ib, 0x61);
    fill_CRC(cmdframe);
    for(k=2;k<8;k++) sprintf(&chsend[(k-2)*3],"%2.2x ", cmdframe[k] );
    sprintf(&chsend[(k-2)*3],"\n" );
    n_byte_resp=12;     //# bytes exspected in response

    r_retry=0;
    s_retry=0;
resend:
    send_command( ip, cmdframe, ptr );
    s_retry+=1;
    ausleep(n5000);
reread:
printf("do get_response\n");
    retcod=get_response(cmdframe, ip, n_byte_resp, lbufrsp, &resp,
                  &status1[iip], &status2[iip] );
printf("retcod get_response: retcod %d, resp %d\n",retcod, resp);
    r_retry+=1;
    if(retcod < 0 ) {
     chrecv[0]=lbufrsp[0];
     if(chrecv[0] != 0) {
      for(k=0;k<12;k++) sprintf(&chrecv[(k  )*3],"%2.2x ", lbufrsp[k] );
      sprintf(&chrecv[(k  )*3],"\n" );
     }

     rtrycod=do_retry(retcod, ip, ptr, it, err_mess, resp);
     switch( rtrycod ) {
      case ( 0) : goto nothing;
      case (-1) : if( s_retry < 3 ) goto resend;  break;
      case (-2) : 
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;} break;
      case (-3) :                                 break;
      case (-4) :
       if(r_retry < 3 ) {resend_last( ip, ptr, it, n_byte_resp ); goto reread;}
       if(s_retry < 3 ) {ausleep(r100000); goto resend;}        break;
     }
//$     aErr.setErrorText( err_mess );
//     aErr.setCommand(  chsend );
//     aErr.setResponse( chrecv );
//     throw aErr;
     printf(" %s \n",err_mess);
     printf(" %s \n",chsend);
     printf(" %s \n",chrecv);
     retcod=-9999;
    }

nothing:
    return(retcod);
}

//-----------------------------------------------------------------------------------


void fill_CRC(unsigned char cmdframe[20]) {
    int k;
    ushort   crcword;

    crcword=0;
    for(k=2;k<cmdframe[6]+7;k++) crcword = CRC16(cmdframe[k], crcword);
    cmdframe[k]  =  crcword & 0x00ff;
    cmdframe[k+1]= (crcword & 0xff00) >> 8;
    return;
}


//-----------------------------------------------------------------------------------


void init_frame(unsigned char frame[20], int ib, char comcod) {

    frame[0]= 0xfb;
    frame[1]= 0x01;
    frame[2]=  ib & 0x00ff;         //box  low
    frame[3]= (ib & 0xff00) >>8;    //box  high
    frame[4]= 0xf0;
    frame[5]= 0x01;
    frame[6]= comcod;     //command code
    return;
}

//-----------------------------------------------------------------------------------

int get_int( int res, unsigned char lbuf[250], int intbuf[10])  {
    int  word, nwd, i;

    nwd=0;
    for(i=0;i<res;i+=2) {
     word = 0;
     if(lbuf[i+1] > 127 ) word = (-1 & 0xffff0000);
     word=word | lbuf[i] | (lbuf[i+1]<<8);
//printf("get_int %d %x %x \n",word, lbuf[i] , (lbuf[i+1]<<8) );
     intbuf[nwd]=word;
     nwd+=1;
    }
    return(nwd);
}




//-----------------------------------------------------------
int init_sockaddr                                                //AB
(struct sockaddr_in *name, const char *hostname, uint16_t port)  //AB
{                                                                //AB
  struct hostent *hostinfo;                                      //AB
                                                                 //AB
  name->sin_family = AF_INET;                                    //AB
  name->sin_port = htons (port);                                 //AB
  hostinfo = gethostbyname (hostname);                           //AB
  if (hostinfo == NULL)                                          //AB
    {                                                            //AB
      printf ("Unknown host %s.\n", hostname);                   //AB
      return -1 ;                                                //AB
    }                                                            //AB
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;         //AB
  printf("--->  %s %d \n",hostname,port) ;                       //AB
                                                                 //AB
  return 0 ;                                                     //AB
}                                                                //AB
//-----------------------------------------------------------    //AB

//-----------------------------------------------------------------------------------


//open RS485 Ethernet boxes
int open_485() {
  #define MODEMDEVICE1 "/dev/ttyS0"
  #define BAUDRATE B19200

  unsigned char rescmd[256];
//fd232[] defined in HAdef.h


//LaPalma (should now also work in ZH)
  #define ETHERBOX1 "amc2sr1"
  #define ETHERBOX2 "amc2sr2"

/*
//Zurich:
  #define ETHERBOX1 "amc-sr2a"          //chain 0- 7
  #define ETHERBOX2 "amc-sr1a"          //chain 9-16 // uscm
  */

  #define ETHERPORT0   8801  //loop test                         
  #define ETHERPORT1   8802  //loop test                         
  #define ETHERPORT2   8803  //loop test                         
  #define ETHERPORT3   8804  //loop test                         
  #define ETHERPORT4   8805  //loop test                         
  #define ETHERPORT5   8806  //loop test                         
  #define ETHERPORT6   8807  //loop test                         
  #define ETHERPORT7   8808  //loop test                         
  #include <stdlib.h>                                            //AB
  #include <sys/socket.h>                                        //AB
  #include <netinet/in.h>                                        //AB
  #include <netdb.h>                                             //AB

  int chain, ierr, i;
  unsigned char chbuf[256];

//=========================

//see posix manulal: 
//Serial Programming Guide for POSIX Operationg Systems; Michael R. Sweet
    /* open the device to be non-blocking (read will return immediatly) */
    printf ("open RS232\n" ); 
    fd232[0] = open(MODEMDEVICE1, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (fd232[0] <0) {perror(MODEMDEVICE1); exit(-1); }
    else {printf(MODEMDEVICE1); printf("   opened \n");}
    tcgetattr(fd232[0],&oldtio0); /* save current port settings */

    /* set new port settings for distance sensor */
    newtio0.c_cflag = BAUDRATE | CS7 | CLOCAL | CREAD | PARENB; //even parity is default

    newtio0.c_iflag = IGNPAR ;                                  //ignor parity errors
    newtio0.c_oflag = 0;
    newtio0.c_lflag = 0;
    newtio0.c_cc[VMIN]=0;
    newtio0.c_cc[VTIME]=1;     //for polling
    tcflush(fd232[0], TCIFLUSH);
    tcsetattr(fd232[0],TCSANOW,&newtio0);
    fcntl(fd232[0], F_SETFL, 0);





//================================================



  blackport[ 0]=ETHERPORT1;
  blackport[ 1]=ETHERPORT2;
  blackport[ 2]=ETHERPORT3;
  blackport[ 3]=ETHERPORT4;
  blackport[ 4]=ETHERPORT5;
  blackport[ 5]=ETHERPORT6;
  blackport[ 6]=ETHERPORT7;
  blackport[ 7]=ETHERPORT0;
  blackport[ 8]=ETHERPORT0;
  blackport[ 9]=ETHERPORT1;
  blackport[10]=ETHERPORT2;
  blackport[11]=ETHERPORT3;
  blackport[12]=ETHERPORT4;
  blackport[13]=ETHERPORT5;
  blackport[14]=ETHERPORT6;
  blackport[15]=ETHERPORT7;

for(chain=0;chain<16;chain++) {fd[chain]=-1; fdalt[chain]=-1;}
                               fduscm=-1;

if(OPENBLACKBOX0 > 0 )
for(chain=0;chain<7;chain++) {                         
  printf("create socket for chain %d\n",chain);

tryagain:
  // Create the socket.                                          //AB
  fd[chain]  = socket (PF_INET, SOCK_STREAM, 0);                        //AB
  if (fd[chain] < 0)                                                    //AB
    {                                                            //AB
      printf(" can not create socket (client)\n");               //AB
      return -1;                                                 //AB
    }                                                            //AB
                                                                 //AB
  // Connect to the server.                                    //AB
  init_sockaddr (&servername[chain], ETHERBOX2, blackport[chain]);              //AB
//  init_sockaddr (&servername[chain], ETHERBOX1, blackport[chain]);              //AB
  ierr =  connect (fd[chain],                                           //AB
                   (struct sockaddr *) &servername[chain],              //AB
                   sizeof (servername[chain])) ;                        //AB
  if ( ierr < 0)                                                 //AB
    {                                                            //AB
      printf ("connect to etherbox failed (client) %d\n",ierr ); //AB
//    return -2;                                                 //AB
      sleep(10);
      goto tryagain;

    }                                                            //AB
                                                                 //AB
                                                                 //AB
   ierr = fcntl(fd[chain], F_SETFL, O_NONBLOCK) ; //non-blocking        //AB
   if (ierr < 0) printf("cannot set NONBLOCK\n");                //AB
   ierr = fcntl(fd[chain], F_SETFD, FD_CLOEXEC) ;  //close on exe     //AB
   if (ierr < 0) printf("cannot set CLOEXEC \n");                //AB
                                                                 //AB
                                                                 //AB
  printf ("connect to etherbox successful (client) chain %d, fd[%d]=%d\n",chain,chain,fd[chain] );         //AB
}                                                                 //AB

//0-----------------------------------------------------------    //AB

if(OPENBLACKBOX1 > 0 )
for(chain=9;chain<16;chain++) {                                   //AB
  printf("create socket for chain %d\n",chain);

tryagain1:
  // Create the socket.                                          //AB
  fd[chain]  = socket (PF_INET, SOCK_STREAM, 0);                        //AB
  if (fd[chain] < 0)                                                    //AB
    {                                                            //AB
      printf(" can not create socket (client)\n");               //AB
      return -1;                                                 //AB
    }                                                            //AB
                                                                 //AB
  // Connect to the server.                                    //AB
  init_sockaddr (&servername[chain], ETHERBOX1, blackport[chain]);              //AB
//init_sockaddr (&servername[chain], ETHERBOX2, ETHERPORT2);              //AB
  ierr =  connect (fd[chain],                                           //AB
                   (struct sockaddr *) &servername[chain],              //AB
                   sizeof (servername[chain])) ;                        //AB
  if ( ierr < 0)                                                 //AB
    {                                                            //AB
      printf ("connect to etherbox failed (client) %d\n",ierr ); //AB
//    return -2;                                                 //AB
      sleep(10);
      goto tryagain1;

    }                                                            //AB
                                                                 //AB
                                                                 //AB
   ierr = fcntl(fd[chain], F_SETFL, O_NONBLOCK) ; //non-blocking     //AB
   if (ierr < 0) printf("cannot set NONBLOCK\n");                //AB
   ierr = fcntl(fd[chain], F_SETFD, FD_CLOEXEC) ;  //close on exe    //AB
   if (ierr < 0) printf("cannot set CLOEXEC \n");                //AB
                                                                 //AB
                                                                 //AB
  printf ("connect to etherbox successful (client) chain %d, fd[%d]=%d\n",chain,chain,fd[chain] );         //AB
}                                                                 //AB

//1-----------------------------------------------------------    //AB


  fd[8]=-1;

  fdalt[6]=fd[15];
  fdalt[5]=fd[14];
  fdalt[4]=fd[13];
  fdalt[3]=fd[12];
  fdalt[2]=fd[11];
  fdalt[1]=fd[10];
  fdalt[0]=fd[ 9];


  fdalt[15]=fd[6];
  fdalt[14]=fd[5];
  fdalt[13]=fd[4];
  fdalt[12]=fd[3];
  fdalt[11]=fd[2];
  fdalt[10]=fd[1];
  fdalt[ 9]=fd[0];

//0-----------------------------------------------------------    //AB


if(OPENBLACKBOX0 > 0 ) {
  chain=7;
  printf("create socket for RS232 (uscm) %d\n",chain);

  // Create the socket.                                          //AB
//fd[chain]  = socket (PF_INET, SOCK_STREAM, 0);                        //AB
  fduscm  = socket (PF_INET, SOCK_STREAM, 0);                        //AB
  if (fduscm < 0)                                                    //AB
    {                                                            //AB
      printf(" can not create socket (client)\n");               //AB
      return -1;                                                 //AB
    }                                                            //AB
  fd[chain]  = fduscm;                                           //AB

  // Connect to the server.                                      //AB
  init_sockaddr (&servername[chain], ETHERBOX2, blackport[chain]);              //AB
  ierr =  connect (fduscm,                                           //AB
                   (struct sockaddr *) &servername[chain],              //AB
                   sizeof (servername[chain])) ;                        //AB
  if ( ierr < 0)                                                 //AB
    {                                                            //AB
      printf ("connect to etherbox failed (client) %d\n",ierr ); //AB
      sleep(10);
      goto tryagain;

    }                                                            //AB
                                                                 //AB
   ierr = fcntl(fduscm, F_SETFL, O_NONBLOCK) ; //non-blocking        //AB
   if (ierr < 0) printf("cannot set NONBLOCK\n");                //AB
   ierr = fcntl(fduscm, F_SETFD, FD_CLOEXEC) ;  //cose on exe        //AB
   if (ierr < 0) printf("cannot set CLOEXEC \n");                //AB
                                                                 //AB
  printf ("connect to etherbox successful (client) chain %d\n",chain );         //AB
                                                                  //AB
}     //if(OPENBLACKBOX0 > 0 ) 

//0-----------------------------------------------------------    //AB


  for(i=0;i<16;i++) printf("fd[%d]=%d, fdalt[%d]=%d\n",i,fd[i],i,fdalt[i]);
                    printf("amcmotor:fduscm=%d\n",fduscm);
  

//0-----------------------------------------------------------    //AB


 chain=0 ;
// open_RS232(chain);      //HA removed 13.7.09  : on this connector is the DISTANCE SENSOR (CS7|parity_even)
 open_RS232(chain);
 chain=1 ;
 open_RS232(chain);
 chain=3 ;
 open_RS232(chain);


  fdalt[6]=fd[15];
  fdalt[5]=fd[14];
  fdalt[4]=fd[13];
  fdalt[3]=fd[12];
  fdalt[2]=fd[11];
  fdalt[1]=fd[10];
  fdalt[0]=fd[ 9];

  fdalt[15]=fd[6];
  fdalt[14]=fd[5];
  fdalt[13]=fd[4];
  fdalt[12]=fd[3];
  fdalt[11]=fd[2];
  fdalt[10]=fd[1];
  fdalt[ 9]=fd[0];

/*
 chain=2 ;
 open_RS232(chain);
 for(;;) {
  for(i=0;i<256;i++) chbuf[i]=0;
  ierr=read_RS232(chain,  chbuf, 10);
  chbuf[10]=0;
  printf("i=%d, chbuf[0]=%2.2x, %2.2x, %2.2x, %2.2x,  %s\n",ierr,chbuf[0],chbuf[1],chbuf[2],chbuf[3],chbuf);
  for(i=0;i<ierr;i++) printf("%2.2x ",chbuf[i]); printf("\n");
  usleep(100000);
 }
*/


 printf("amcmotor: open blue\n");
 open_485_blueboard();    //open 8-fold interface for 7 chains plus USCM
 fdalt[ 9]=fd[0];
 fdalt[10]=fd[1];
 fdalt[11]=fd[2];
 fdalt[12]=fd[3];
 fdalt[13]=fd[4];
 fdalt[14]=fd[5];
 fdalt[15]=fd[6];
 printf("amcmotor:fduscm=%d\n",fduscm);


// fduscm is last channel 8-fold interface called "blueboard"
// fduscm = fd232[2];   //HA 10.6.09    //blue box serial1
// fduscm = fd[7];      //HA 10.6.09    //black box
                                                                  //AB
  for(i=0;i<16;i++) printf("fd[%d]=%d, fdalt[%d]=%d\n",i,fd[i],i,fdalt[i]);
                    printf("amcmotor when returning:fduscm=%d\n",fduscm);

      return  0;                                                 //AB

}

//-----------------------------------------------------------------------------------


    #include "blueboard485.inc"      // the other port on the blue computer
