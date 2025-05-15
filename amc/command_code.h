  
    for(i=0;i<59;++i) {
     for(j=0;j<=40;j++) cmdbf[i][j]= 0x00;   //added 28.10.09 HA
     cmdbf[i][0]= 0xfb;
     cmdbf[i][1]= 0x01;
     cmdbf[i][4]= mcntr[0];
     cmdbf[i][5]= 0x03;  //cp
     cmdbf[i][6]= i+1;   //sc
    }
    i=0;
     cmdbf[i][5]= 0x05;      //cp
     cmdbf[i][6]= 0x03;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"move rel xy\0");
     cmdmvx=i;
    i=1;                     //centre motors
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x07;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"centre motors \0");
     cmdcntr=i;
    i=2;                     //reset driver
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x0A;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"reset driver\0");
     cmdresd=i;
    i=3;                     //stop moving/kill
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x0C;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"stop moving/kill \0");
     cmdkill=i;
    i=4;                     //laser on
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x0E;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"laser on \0");
     cmdlasn=i;
    i=5;                     //laser off
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x0F;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"laser off \0");
     cmdlasf=i;
    i=6;                     //read  frequency
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x11;      //sc
     resin[i]=13;
     sprintf(cmdtxt[ cmdbf[i][6] ],"read frequency \0");
     cmdrdfq=i;
    i=7;                     //write frequency
     cmdbf[i][5]= 0x03;      //cp
     cmdbf[i][6]= 0x12;      //sc
     (*word)= 700;
     cmdbf[i][7]= byte[0];   //nofbyte = 9 = cmdbf[5]+6
     cmdbf[i][8]= byte[1];
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"write frequency \0");
     cmdwtfq=i;
    i=8 ;                    //read  working current
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x14;      //sc
     resin[i]=13;
     sprintf(cmdtxt[ cmdbf[i][6] ],"read working current \0");
     cmdrdwc=i;
    i=9 ;                    //write working current
     cmdbf[i][5]= 0x03;      //cp
     cmdbf[i][6]= 0x15;      //sc
     (*word)= 700;
     cmdbf[i][7]= byte[0];   //nofbyte = 9 = cmdbf[5]+6
     cmdbf[i][8]= byte[1];
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"write working current \0");
     cmdwtwc=i;
    i=10;                    //read  holding current
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x17;      //sc
     resin[i]=13;
     sprintf(cmdtxt[ cmdbf[i][6] ],"read holding current \0");
     cmdrdhc=i;
    i=11;                    //write holding current
     cmdbf[i][5]= 0x03;      //cp
     cmdbf[i][6]= 0x18;      //sc
     (*word)= 70;
     cmdbf[i][7]= byte[0];   //nofbyte = 9 = cmdbf[5]+6
     cmdbf[i][8]= byte[1];
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"write holding current \0");
     cmdwthc=i;
    i=12;                    //read temp humid curr v1 v2 vlog
     cmdbf[i][4]= 0xf0;      //cp
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x1F;      //sc
     resin[i]=23;
     sprintf(cmdtxt[ cmdbf[i][6] ],"read temp humid etc\0");
     cmdrdth=i;
    i=13;                    //query abs motor pos.
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x21;      //sc
     resin[i]=15;
     sprintf(cmdtxt[ cmdbf[i][6] ],"query abs motor position \0");
     cmdrdmt=i;
    i=14;                    //motor power off
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x30;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"motor power off \0");
     cmdpwof=i;
    i=15;                    //motor power on
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x31;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"motor power on \0");
     cmdpwon=i;
    i=16;                    //read version number
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0xFE;      //sc
     resin[i]=13;
     sprintf(cmdtxt[ cmdbf[i][6] ],"read version number \0");
     cmdsdvn=i;
    i=17;                    //resend latest frame
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0xFF;      //sc
     resin[i]=24;
     sprintf(cmdtxt[ cmdbf[i][6] ],"resend last frame \0");
     cmdrsnd=i;
    i=18;                    //reset box
     cmdbf[i][5]= 0x01;      //cp
     cmdbf[i][6]= 0x09;      //sc
     resin[i]=11;
     sprintf(cmdtxt[ cmdbf[i][6] ],"reset box\0");
     cmdresb=i;



