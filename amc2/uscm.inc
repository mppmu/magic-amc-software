// unsigned char USCMreg[4];        //on/off status
// int prompted =0;                 //1=in prompt mode = MONITOR>

//==============================================================

int compare_string( t1, t2, n )                          //$$$
//internal routine
int n;
unsigned char t1[n], t2[n];                              //t1: buffer
{                                                        //t2: sample
 int i;


 for(i=0;i<n;i++) 
  if(t1[i] != t2[i]) return -i-1;
 return 0;
}


//==============================================================


 int send_string(text, n0 )                         //$$$
//internal routine
 int n0;
 unsigned char text[n0];
 {
  int           ret, j,res,res0,res1,res2,res3, chain, ierr, i, nopro;
  unsigned char echo[2050];                         //ok

  text[n0]=0; printf("write: %s\n",text);
  i=gettimeofday( tv, tz);               //get real time of this action
  tp4=atv.tv_usec ;
  sp4=atv.tv_sec-AMCtimeoff ;
  if(tp4<0) {tp4+=1000000; sp4--;}

//flush everything
morestringflush:
  ioctl(fduscm, FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
  if(res2 > 0 ) {
   if(res2 > 2047) res2 = 2047;
   res3 = read(fduscm,&echo[0],res2);
   printf("\nsend_string: %d bytes flushed, %2.2x, %2.2x\n",res2,echo[0],echo[1]);
   if(res3>0) { echo[res3]=0;  printf("%s|\n",echo); }

   for(i=0;i<res3-8 ;i++)
    if(echo[i] == prompt[2]) {
     if((ret=compare_string(&echo[i],&prompt[2],8 )) == 0) {
      prompted = 1; break;
     }
    }
    goto morestringflush;
   }


//output the string
//  text[n0]=0; printf("write: %s\n",text);
//  for(i=0;i<n0;i++) printf(" %2.2x",text[i]); printf(" fduscm %d\n",fduscm);

  res = write(fduscm,text,n0);

  prompted =0;                 
//  text[n0]=0;   printf("set prompted=0 s:%s\n",text);
  if (res != n0)
  {                                               
     printf("connection problems (send) %d\n",res);   
     return -111 ;                                    
  }                                                   


//read echo from USCM
//                                  res++;  //LF is also returned
  res0 = 0 ;            
  for(j=0;j<9;j++) {
   res1 = res - res0;
   ioctl(fduscm, FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
   if(res2 > 0 ) {
    if(res2 > res1 ) res2 = res1;
    res3 = read(fduscm,&echo[res0],res2);
    res0+=res3;
//    printf("send_string: %d bytes echo ",res3);
//for(i=0;i<res0;i++) printf(" %2.2x",echo[i]); printf(" fduscm %d\n",fduscm);
//for(i=0;i<res0;i++) if(echo[i]==0) echo[i]='@';
//    echo[res0]=0; printf("*| %s|*\n",echo);
    if(res0 >=res) {i= 0; goto prttim;}
   }          
   usleep(100000);
  }                        
  printf("not enough data (echo 1) snd=%d rcv=%d %s\n",res,res0, text);
  sprintf(ascii,"not enough data (echo 1) s=%d r=%d\n",res,res0);
  log_write(fdisk, ascii,  &kascii);
  i=-1;

prttim:
  i=gettimeofday( tv, tz);               //get real time of this action
  tp5=atv.tv_usec           - tp4;
  sp5=atv.tv_sec-AMCtimeoff - sp4;
  if(tp5<0) {tp5+=1000000; sp5--;}
  printf("send_s %d s, %d us\n",sp5,tp5);

  return(i);

 }

//==============================================================


 int recv_allbuf(newcmd,  n, n1 )                            //$$$
//internal routine
 int n, *n1;
 unsigned char newcmd[n];
  {
  int        i,j,res,res0,res2,res3;

//printf("recv_allbuf\n");
  *n1=0;
  res=n;
  res0 = 0;            
  for(j=0;j<2;j++) {
   ioctl(fduscm, FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
   if(res2 > 0 ) {
    if(res2 > res ) res2 = res;
    res3 = read(fduscm,&newcmd[res0],res2);
    if( res3>0 ) res0+=res3;
    *n1 = res0;
    newcmd[res0]=0; 
//    printf("recv_allbuf: %d bytes gotten: ",res3);
//for(i=0;i<*n1;i++) printf(" %2.2x",newcmd[i]); printf("\n"); printf("r:%s\n",newcmd);
    if(res0 == n) goto done;
   }          
   usleep(200000);
  }                        
done: ;
//printf("recv_allbuf: exit\n");
  return res0 ;



  }


//==============================================================


 int recv_string(newcmd,  n, n1 )                            //$$$
//internal routine
 int n, *n1;
 unsigned char newcmd[n];
 {
  int        i,j,res,res0,res2,res3;

  res=n;
  res0 = 0;            
  for(j=0;j<2;j++) {
   ioctl(fduscm, FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
   if(res2 > 0 ) {
    if(res2 > res ) res2 = res;
    res3 = read(fduscm,&newcmd[res0],res2);
    if( res3>0 ) res0+=res3;
    *n1 = res0;
    newcmd[res0]=0; 
//    printf("recv_string: %d bytes gotten: ",res3);
//    for(i=0;i<res3;i++) printf(" %2.2x",newcmd[i]); printf("\n"); printf("r:%s\n",newcmd);
    if(res0 >=res) return 0;
   }          
   usleep(10000);
  }                        
  printf("not enough data (recv) e=%d r=%d\n",res,res0);
  sprintf(ascii,"not enough data (recv) s=%d r=%d\n",res,res0);
  log_write(fdisk, ascii,  &kascii);
//  put_logfile(0, 1, "toggle switches 17/21/22                -\0");
//sleep(1);
//printf("not enough data (recv1) s=%d r=%d\n",res,res0);
  return res0 ;



  }


//==============================================================

int get_prompt(  )                                       //$$$
//internal routine
{
 unsigned char ascii[] = { '.', '\r', '\0' }, buffer[51];
 int ret, j, i, k1, k2, n1;

// printf("------------------------>>> get prompt: prompted= %d\n",prompted);
 send_string(ascii,2);

 for(i=0;i<50 ;i++) buffer[i] = '%';
// j=recv_string(buffer, 35, &n1);
 j=recv_allbuf(buffer, 50, &n1);
 if((compare_string(&buffer[1],&nimple[1],8)) == 0) {prompted=1; goto doreturn; }
 for(i=0;i<n1-8 ;i++) {
  if(buffer[i] == prompt[2]) {
   if((ret=compare_string(&buffer[i],&prompt[2],8)) == 0) {
    prompted=1;
//    printf("got prompt %d, ret=%d\n",prompted,ret);
    goto doreturn;
   }
  }
 }
doreturn: ;
// buffer[j]='\0';
// printf("------------------------>>> exit get prompt: prompted= %d, j=%d, %s\n",prompted,j,buffer);
 return (j);
}

//==============================================================
  
int allstatus_USCM( )                             //$$$
{
//get status for all register 
//i=0 to 3: 07f004+i --> regloc[i]
 unsigned char ascii[20], buffer[20];                      // RS232 text
 int j,k,v, n1, n11;

 printf("enter allstatus_USCM \n");
 if(prompted == 0) {
  printf("not prompted0: get prompt\n");
  get_prompt(  ); 
  if(prompted == 0) get_prompt(  ); 
  if(prompted == 0) {printf("not prompted0\n"); return(-1);}
 }

// printf("============0\n\n");

 n11=12;
 sprintf(ascii,"mx 7f00%1d\r\0",4);
 send_string(ascii,9);
 j=recv_string(buffer, n11, &n1);
 k=-1;
 for(j=7;j<12;j++) if( buffer[j] == 0x3a ) {
  k=buffer[j-1]-0x30; break;               }
 sscanf(&buffer[8],"%2x",&v);
 if( n1 <= 0 )return(-1);
 USCMreg[k-4]=v;

 send_string(&ascii[8],1);
 j=recv_string(buffer, n11, &n1);
 k=-1;
 for(j=7;j<12;j++) if( buffer[j] == 0x3a ) {
  k=buffer[j-1]-0x30; break;               }
 sscanf(&buffer[8],"%2x",&v);
 if( n1 <= 0 )return(-1);
 USCMreg[k-4]=v;

 send_string(&ascii[8],1);
 j=recv_string(buffer, n11, &n1);
 k=-1;
 for(j=7;j<12;j++) if( buffer[j] == 0x3a ) {
  k=buffer[j-1]-0x30; break;               }
 sscanf(&buffer[8],"%2x",&v);
 if( n1 <= 0 )return(-1);
 USCMreg[k-4]=v;

 send_string(&ascii[8],1);
 j=recv_string(buffer, n11, &n1);
 k=-1;
 for(j=7;j<12;j++) if( buffer[j] == 0x3a ) {
  k=buffer[j-1]-0x30; break;               }
 sscanf(&buffer[8],"%2x",&v);
 if( n1 <= 0 )return(-1);
 USCMreg[k-4]=v;

 ascii[7]='.';
 send_string(&ascii[7],2);
 usleep(100000);
 j=recv_string(buffer,  11, &n1);
// printf("all============1\n\n");

 printf("allstatus_USCM: USCMreg %2.2x, %2.2x, %2.2x, %2.2x\n",
   USCMreg[0], USCMreg[1], USCMreg[2], USCMreg[3] );

 get_prompt();      //-

 return 0;

}

//==============================================================
  
int status_USCM( regloc, i )                             //$$$
unsigned char* regloc;
int           i;
{
//get status for register i (=0,1,2,3)
//*regloc[i] is filled
//i=0 to 3: 07f004+i --> regloc[i]
 unsigned char ascii[20], buffer[20];                      // RS232 text
 int j,k,v, n1;

 printf("enter status_USCM R%d\n",i);
 if(prompted == 0) {
  printf("not prompted0: get prompt\n");
  get_prompt(  ); 
  if(prompted == 0) get_prompt(  ); 
  if(prompted == 0) {printf("not prompted0\n"); return(-1);}
 }

 printf("============0\n\n");
 sprintf(ascii,"mx 7f00%1d\r\0",i+4);
 printf("sending: %s\n",ascii);
 send_string(ascii,9);
 j=recv_string(buffer, 11, &n1);
 k=-1;
 for(j=7;j<12;j++) if( buffer[j] == 0x3a ) {
  k=buffer[j-1]-0x30; break;               }
 sscanf(&buffer[8],"%2x",&v);
// printf("n1=%d, j=%d, k=%d,  v=%2.2x\n",n1,j,k,v);
 if( n1 <= 0 )return(-1);
 USCMreg[k-4]=v;


 ascii[7]='.';
 send_string(&ascii[7],2);
 j=recv_string(buffer, 11, &n1);
 printf("============1\n\n");

 *regloc = USCMreg[i];
 printf("status_USCM: USCMreg %2.2x, %2.2x, %2.2x, %2.2x\n",
     USCMreg[0], USCMreg[1], USCMreg[2], USCMreg[3] );
// printf("status_USCM( *regloc=%2.2x, %d )\n",*regloc,i);
 get_prompt();      //-

 return 0;

}


//==============================================================

int init_USCM( text )                                    //$$$
//internal routine
unsigned char text[10];
{
 unsigned char ascii[80], buffer[80];             // RS232 text
 unsigned char echo[2048];
 int i,j,k, n1;
 int ret, res2, res3;


 printf("\ninit_USCM:\n");

//wake up USCM
/*
 ascii[0]='1';
 ascii[1]='\r';
 ascii[2]=0;
 ascii[3]='\r';
 ascii[4]='\n';
 ascii[5]='\r';
 ascii[6]='\r';
 ascii[7]='\r';
  */

 for(i=0;i<80;i++)  ascii[i]=3;
 ascii[ 0]=' 0';
 ascii[ 1]=' ,';
 ascii[ 2]=' 0';
 ascii[77]=26;
 ascii[78]='\n';
 ascii[79]='\r';

 res2= write(fduscm,ascii,80); //send 79 ctrl C
 printf("\ninit_USCM: write res2=%d\n",res2);
 usleep(200000);
 ioctl(fduscm, FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
 printf("\ninit_USCM: ioctl res2=%d\n",res2);
 if(res2 <= 0 ) { 
//  fduscm = fd[7];
  res2= write(fduscm,ascii,80); //send 79 ctrl C
  printf("\ninit_USCM: write res2=%d\n",res2);
 }

 usleep(300000);
morestringflush:
 ioctl(fduscm, FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
 printf("\ninit_USCM: ioctl res2=%d\n",res2);
 if(res2 > 0 ) {
  if(res2 > 2047) res2 = 2047;
  res3 = read(fduscm,&echo[0],res2);
//for(i=0;i<res3;i++) printf(" %2.2x",echo[i]); printf("\n");
  printf("\ninit_USCM: %d bytes flushed %d\n",res3, res2);
  echo[res3]=0;
  printf("%s\n\n",echo);

  for(i=0;i<res3-8 ;i++) {
   if(echo[i] == prompt[2]) {
    if((ret=compare_string(&echo[i],&prompt[2],8)) == 0) {
     prompted = 1;
     goto readval;
    }
   }
  }
  res2= write(fduscm,ascii,80);
  goto morestringflush;
 }

 if(prompted == 1) goto readval;



 ascii[8]=0;
 ascii[0]='0';
 ascii[1]='\r';

next:
 if(i=send_string(ascii,2) <0) {
  printf("can't initialize USCM (no response)\n");
  put_logfile(LOG_SVR, 0, "can't initialize USCM (no response)");
  put_logfile(LOG_SVR, 1, "can't initialize USCM (no response)");
  put_logfile(LOG_SVR, 1, "can't initialize USCM (no response)");
  put_logfile(LOG_SVR, 0, "toggle switches 17/21/22                   -\0");

  ascii[0]++;
//if(ascii[8]++<126) goto next;
  if(ascii[8]++< 2 ) goto next;
  return -1111;
 }
 printf("send_string return code %d\n",i);

//get prompt; MONITOR>
 ascii[0]='.';
 ascii[0]='\r';
 ascii[1]='\r';
 for(i=0;i<10;i++) {
  printf("\ninit_USCM: ------------> get prompt\n");
//  send_string(ascii,2);
  send_string(ascii,1);
for(i=0;i<60 ;i++) text[i] = '%';
printf("call recv_string\n",j);
//usleep(500000);
  j=recv_string(text, 10, &n1);

  compare_string(text,&nimple[1],8); goto readval;

  if((compare_string(text[1],&nimple[1],8)) == 0) goto readval;
  for(i=0;i<j-8 ;i++)
   if(text[i] == prompt[2]) 
    if(compare_string(text[i],&prompt[2],8) == 0) goto readval;
//sleep(1);
 }
 return -j-1;

readval:
printf("INIT: readval\n");
 prompted=1;
 allstatus_USCM(  );

printf("END of INIT\n");
 return 0;
}


//==============================================================
  
int on_USCM( channel )                                   //$$$
//unused routine
unsigned int channel;
{
 unsigned char ascii[20], buffer[20];                      // RS232 text
 int i,j,k,n1;


 printf("on_USCM %d\n",channel);
 if(channel >=32) {
  printf("illegal channel %d\n",channel);
  return -1;
 }

 if(prompted == 0) {
  printf("not prompted1: get prompt\n");
  get_prompt(  ); 
  if(prompted == 0) {printf("not prompted1\n"); return(-1);}
 }

 i=channel/8;
 j=channel%8;
 k=1 << j;
 printf("i %d, j %d, k %d, onreg[i] %d\n",i,j,k,USCMreg[i]);
 status_USCM( &USCMreg[i], i );
// if( (USCMreg[i] & k) == k) return 0;

 sprintf(ascii,"mx 7f00%1d\r",i);
 send_string(ascii,9);
 j=recv_string(buffer, 11, &n1);
 sprintf(ascii,"%2.2x.\r", (USCMreg[i] | k));
 send_string(ascii,4);
//-  get_prompt();
 status_USCM( &USCMreg[i], i );

 return 0;
}


//==============================================================
  
int off_USCM( channel )                                  //$$$
//unused routine
unsigned int channel;
{
 unsigned char ascii[20], buffer[20];                      // RS232 text
 int i,j,k,n1;

 printf("off_USCM %d\n",channel);
 if(channel >=32) {
  printf("illegal channel %d\n",channel);
  return -1;
 }

 if(prompted == 0) {
  printf("not prompted3: get prompt\n");
  get_prompt(  ); 
  if(prompted == 0) {printf("not prompted3\n"); return(-1);}
 }

 i=channel/8;
 j=channel%8;
 k=1 << j;
 printf("i %d, j %d, k %d, ofreg[i] %d\n",i,j,k,USCMreg[i]);

// status_USCM( &USCMreg[i], i );
 if( (USCMreg[i] & k) == 0) return 0;

 sprintf(ascii,"mx 7f00%1d\r",i);
 send_string(ascii,9);
 j=recv_string(buffer, 11, &n1);
 sprintf(ascii,"%2.2x.\r",(USCMreg[i] & -(k+1)));
 send_string(ascii,4);
 status_USCM( &USCMreg[i], i );       //promtps at the beginning an the end

 return 0;
}

//==============================================================
  
int set_USCM( pattern, part )                            //$$$
//called by: burst_amd_get, center, ha_m2, hamain
unsigned int part;
unsigned char pattern;
{
 unsigned char ascii[20], buffer[20];                      // RS232 text
 int i,j,k, n1;

 if(part >=4) {
  printf("illegal partition %d\n",part);
  return -1;
 }

 printf("set_USCM %2.2x in %d; %2.2x\n",pattern, part,USCMreg[part]);

 if( pattern == USCMreg[part] ) return(0);

 if(prompted == 0) {
  printf("not prompted4: get prompt\n");
  get_prompt(  ); 
  if(prompted == 0) {printf("cant prompt4\n"); return(-1);}
 }

 sprintf(ascii,"mx 7f00%1d\r",part);
 send_string(ascii,9);
 j=recv_string(buffer, 11, &n1);

 sprintf(ascii,"%2.2x.\r\0",pattern);
 send_string(ascii,4);
 j=recv_string(buffer, 11, &n1);
 //returns "MONITOR> "
 for(i=0;i<n1-8 ;i++) {
  if(buffer[i] == prompt[2]) {
   if((compare_string(&buffer[i],&prompt[2],8 )) == 0) {
    prompted = 1; break;
   }
  }
 }

 status_USCM( &USCMreg[part], part );

 ascii[0]='.';
 ascii[1]='\r';
 send_string(ascii,2);


 return j;
}

//==============================================================
  
int set_USCM0( pattern, part )                            //$$$
//called by: burst_amd_get, center, ha_m2, hamain
unsigned int part;
unsigned char pattern;
{
 unsigned char ascii[20], buffer[20];                      // RS232 text
 int i,j,k, n1;

 if(part >=4) {
  printf("illegal partition %d\n",part);
  return -1;
 }

 printf("set_USCM0 %2.2x in %d; %2.2x\n",pattern, part,USCMreg[part]);

 if( pattern == USCMreg[part] ) return(0);

 if(prompted == 0) {
  printf("not prompted4: get prompt\n");
  get_prompt(  ); 
  if(prompted == 0) {printf("cant prompt4\n"); return(-1);}
 }

 sprintf(ascii,"mx 7f00%1d\r",part);
 send_string(ascii,9);
 j=recv_string(buffer, 11, &n1);

 sprintf(ascii,"%2.2x.\r\0",pattern);
 send_string(ascii,4);
 j=recv_string(buffer, 11, &n1);
 //returns "MONITOR> "
 for(i=0;i<n1-8 ;i++) {
  if(buffer[i] == prompt[2]) {
   if((compare_string(&buffer[i],&prompt[2],8 )) == 0) {
    prompted = 1; break;
   }
  }
 }

 ascii[0]='.';
 ascii[1]='\r';
 send_string(ascii,2);


 return j;
}

//==============================================================
  
int chain_USCM( chain )                                  //$$$
//unused routine
unsigned int chain;
{
 unsigned char pattern;
 unsigned char ascii[20], buffer[20];                      // RS232 text
 int i,j,k,  part, n1;


 if(prompted == 0) {
  printf("not prompted5: get prompt\n");
  get_prompt(  ); 
  if(prompted == 0) {printf("not prompted5\n"); return(-1);}
 }

 printf("chain_USCM %d\n",chain);
 if(chain >=8) {
  printf("illegal chain %d\n",part);
  return -1;
 }

 part=chain_channel[chain];
 pattern=chain_pattern[chain] | USCMreg[part] ;
 sprintf(ascii,"mx 7f00%1d\r",part);
 send_string(ascii,9);
 j=recv_string(buffer, 11, &n1);
 sprintf(ascii,"%2.2x.\r",pattern);
 send_string(ascii,4);
 get_prompt();             //-
 status_USCM( &USCMreg[part], part );

 return 0;
}

//==============================================================
