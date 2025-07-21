  
    for(i=0;i<59;++i) {
     for(j=0;j<=40;j++) cmdbf[i][j]= 0x00;   //added 28.10.09 HA
     cmdbf[i][0]= 0xfb;
     cmdbf[i][1]= 0x01;
//   cmdbf[i][2]= xcntr[0];
//   cmdbf[i][3]= xcntr[1];
     cmdbf[i][4]= mcntr[0];
     cmdbf[i][5]= mcntr[1];
     cmdbf[i][6]= 0x03;  //cp
     cmdbf[i][7]= i+1;   //sc
    }
    i=0;
     cmdbf[i][6]= 0x03;      //cp
     cmdbf[i][7]= 0x01;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"move rel x\0");
     cmdmvx=i;
    i=1;                     //centre motors
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x07;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"centre motors \0");
     cmdcntr=i;
    i=2;                     //reset box
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x09;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"reset box \0");
     cmdresb=i;
    i=3;                     //stop moving/kill
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x0C;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"stop moving/kill \0");
     cmdkill=i;
/*
    i=4;                     //laser on
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x0E;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"laser on \0");
     cmdlasn=i;
    i=5;                     //laser off
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x0F;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"laser off \0");
     cmdlasf=i;
*/
    i=6;                     //read  frequency
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x11;      //sc
     resin[i]=14;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read frequency \0");
     cmdrdfq=i;
    i=7;                     //write frequency
     cmdbf[i][6]= 0x03;      //cp
     cmdbf[i][7]= 0x12;      //sc
//old  (*word)=1400;
     (*word)= 700;
     cmdbf[i][8]= byte[0];   //nofbyte = 9 = cmdbf[5]+6
     cmdbf[i][9]= byte[1];
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"write frequency \0");
     cmdwtfq=i;
    i=8 ;                    //read  working current
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x14;      //sc
     resin[i]=14;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read working current \0");
     cmdrdwc=i;
    i=9 ;                    //write working current
     cmdbf[i][6]= 0x03;      //cp
     cmdbf[i][7]= 0x15;      //sc
     (*word)=1000;
     cmdbf[i][8]= byte[0];   //nofbyte = 9 = cmdbf[5]+6
     cmdbf[i][9]= byte[1];
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"write working current \0");
     cmdwtwc=i;
    i=10;                    //read  holding current
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x17;      //sc
     resin[i]=14;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read holding current \0");
     cmdrdhc=i;
    i=11;                    //write holding current
     cmdbf[i][6]= 0x03;      //cp
     cmdbf[i][7]= 0x18;      //sc
     (*word)=400;
     cmdbf[i][8]= byte[0];   //nofbyte = 9 = cmdbf[5]+6
     cmdbf[i][9]= byte[1];
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"write holding current \0");
     cmdwthc=i;
    i=12;                    //read temp humid 
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x1F;      //sc
     resin[i]=16;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read temp humid \0");
     cmdrdth=i;
    i=13;                    //query abs motor pos.
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x21;      //sc
     resin[i]=14;
     sprintf(cmdtxt[ cmdbf[i][7] ],"query abs motor position \0");
     cmdrdmt=i;
    i=14;                    //motor power off
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x30;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"motor power off \0");
     cmdpwof=i;
    i=15;                    //motor power on
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x31;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"motor power on \0");
     cmdpwon=i;
    i=16;                    //read version number
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0xFE;      //sc
     resin[i]=13;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read version number \0");
     cmdsdvn=i;
    i=17;                    //resend last frame
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0xFF;      //sc
     resin[i]=24;
     sprintf(cmdtxt[ cmdbf[i][7] ],"resend last frame \0");
     cmdrsnd=i;
    i=18;                    //send unique ID
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0xFC;      //sc
     resin[i]=14;
     sprintf(cmdtxt[ cmdbf[i][7] ],"send unique ID \0");
     cmdsdui=i;
    i=19;                    //read 4 magnetic sensors
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x41;      //sc
     resin[i]=20;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read 4 magnetic sensors \0");
     cmdr4ms=i;
    i=20;                    //scan magnetic sensors
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x40;      //sc
     resin[i]=27;            //no protocol for response
     sprintf(cmdtxt[ cmdbf[i][7] ],"scan magnetic sensors \0");
     cmdscms=i;
    i=21;                    //full range/endsw values
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x43;      //sc
     resin[i]=30;
     sprintf(cmdtxt[ cmdbf[i][7] ],"range, endsw value \0");
     cmdrdfr=i;
    i=22;                    //watchdog counter and source
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x42;      //sc
     resin[i]=15;
     sprintf(cmdtxt[ cmdbf[i][7] ],"watchdog ctr & src \0");
     cmdqwdc=i;
    i=23;                    //write abs motor pos. to SDRAM
     cmdbf[i][6]= 0x03;      //cp
     cmdbf[i][7]= 0x22;      //sc
     (*word)=9999;
     cmdbf[i][8]= byte[0];   //
     cmdbf[i][9]= byte[1];
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"write abs motor position to SDRAM \0");
     cmdwtmt=i;
    i=24;                    //read abs motor pos. from EEPROM
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x23;      //sc
     resin[i]=14;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read abs motor position from EEPROM\0");
     cmdrdemt=i;
    i=25;                    //write abs motor pos. from EEPROM
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x24;      //sc
//   cmdbf[i][6]= 0x03;      //cp
//   cmdbf[i][7]= 0x24;      //sc
//   (*word)=9999;
//   cmdbf[i][8]= byte[0];   //
//   cmdbf[i][9]= byte[1];
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"write abs motor position to EEPROM \0");
     cmdwtemt=i;
    i=26;                    //lower endswitch: get sensor number and limit
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x44;      //sc
     resin[i]=15;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read sens nmbr and limt of lw endsw \0");
     cmdrdlsw=i;
    i=27;                    //lower endswitch: set sensor number and limit
     cmdbf[i][6]= 0x04;      //cp
     cmdbf[i][7]= 0x45;      //sc
     cmdbf[i][8]= 0;         //  sensor nr 0
     (*word)=610;
     cmdbf[i][9]= byte[0];
     cmdbf[i][10]= byte[1];
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"set sens nmbr and limt of lw endsw \0");
     cmdwtlsw=i;
    i=28;                    //upper endswitch: get sensor number and limit
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x46;      //sc
     resin[i]=15;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read sens nmbr and limt of up endsw \0");
     cmdrdusw=i;
    i=29;                    //upper endswitch: set sensor number and limit
     cmdbf[i][6]= 0x04;      //cp
     cmdbf[i][7]= 0x47;      //sc
     cmdbf[i][8]= 3;         //  sensor nr 0
     (*word)=410;
     cmdbf[i][9]= byte[0];
     cmdbf[i][10]= byte[1];
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"set sens nmbr and limt of up endsw \0");
     cmdwtusw=i;

    i=30;                    //laser on
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x60;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"laser on \0");
     cmdnlasn=i;
    i=31;                    //laser off
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x61;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"laser off \0");
     cmdnlasf=i;
    i=32;                    //new centre motor
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x08;      //sc
     resin[i]=12;
     sprintf(cmdtxt[ cmdbf[i][7] ],"new centre motors \0");
     cmdncnt=i;
    i=33;                    //read laser
     cmdbf[i][6]= 0x01;      //cp
     cmdbf[i][7]= 0x62;      //sc
     resin[i]=132;
     sprintf(cmdtxt[ cmdbf[i][7] ],"read laser\0");
     cmdrlas=i;


/*
    printf("List of commands \n");
    for(j=0;j<=i;j++) {
//   crcword = 0;
     printf("j=%d \r\n",j);
     printf(" %2.2x", cmdbf[j][0] );
     printf(" %2.2x", cmdbf[j][1] );
     for(k=2;k<cmdbf[j][6]+7;k++) {
//    crcword = CRC16(cmdbf[j][k], crcword);
      printf(" %2.2x", cmdbf[j][k] );
     }
//   byte[0]=crcword & 0x0f;
//   byte[1]=(crcword & 0xf0) >> 8;
//   cmdbf[j][k]= byte[0];
//   cmdbf[j][k+1]= byte[1];
//   printf(" %2.2x", cmdbf[j][k] );
//   printf(" %2.2x\n", cmdbf[j][k+1] );
     printf(" %s\n", cmdtxt[cmdbf[j][7]] );
    }


     printf("cmdrdfq=%d\n",cmdrdfq);
     printf("cmdrdmt=%d\n",cmdrdmt);
     printf("cmdrdth=%d\n",cmdrdth);
  */

