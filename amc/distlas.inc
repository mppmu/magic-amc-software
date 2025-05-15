
//-----------------------------------------------------------------------------------

 int AMC_exec_dist(int cmd )                         //$$$
 {
  unsigned char rescmd[256], answer[256];
  int nn, retcod;

  printf("AMC_exec_dist( %d )\n",cmd);
  if(cmd < 0 ) {           //laser on
   printf("laser on\n");
   rescmd[0]='o' ;
   rescmd[1]='\r' ;
   rescmd[2]='\n' ;
   rescmd[3]=0;
   nn=3;
  }

  if(cmd == 0 ) {          //laser off
   printf("laser off\n");
   rescmd[0]='p' ;
   rescmd[1]='\r' ;
   rescmd[2]='\n' ;
   rescmd[3]=0;
   nn=3;
  }

  if(cmd > 0 ) {           //distance
   printf("laser measure\n");
   rescmd[0]='g' ;
   rescmd[1]='\r' ;
   rescmd[2]='\n' ;
   rescmd[3]=0;
   nn=3;
  }


  retcod=send_string232(rescmd, 3, answer );

  // retcod: <0 error; =0 ok; >0 = dist in mm
  printf("laser retcod =%d\n",retcod);
  if( (cmd > 0) && (retcod>=0) ) {
   retcod=0;
   for(i=7;i<14;i++) retcod = 10* retcod + (answer[i]-'0'); //mm
   sprintf(rescmd,"DISTO measurement =%d",retcod);
   put_logfile(LOG_OK_, 0, rescmd);
  }
  if(retcod < 0 ) { 
   sprintf(rescmd,"DISTO measurement: Error %s",&answer[2]);
   put_logfile(LOG_ERR, 0, rescmd);
  }
  return retcod;

 }

//-----------------------------------------------------------------------------------

 int send_string232(text, n0, answer )                         //$$$
 int n0;
 unsigned char text[n0], answer[256];
 {
  int         i,j,res,res0,res1,res2,res3, chain, ierr;
  unsigned char echo[2048];                         //ok

/*DIMETIX*/

//flush everything
morestringflush:
  ioctl(fd232[0], FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
  if(res2 > 0 ) {
   if(res2 > 2048) res2 = 2048;
   res3 = read(fd232[0],&echo[0],res2);
    echo[res3]=0;
//  printf("IOCTL flush:%s\n",echo);
   goto morestringflush;
  }

//output the string
  res = write(fd232[0],text,n0);
//  prompted =0;
//  text[n0]=0;
//  printf("send232:%s\n",text);
  if (res != n0)
  {                                               
     sprintf(answer,"transmission problems (RS232 send) %d",res);
     put_logfile(LOG_SVR, 0, answer);
     return -111 ;                                    
  }                                                   


//read echo from USCM
  for(j=0;j<25;j++) {
   res1 = res - res0;
   ioctl(fd232[0], FIONREAD, &res2);   //get $ of bytes available in RS232 USCM
//   printf("IOCTL j=%d, res2=%d\n",j,res2);
   if(res2 > 0 ) {
    res3 = read(fd232[0],&answer[0],res2);
//    printf("send_string232: %d bytes echo\n",res3);
    answer[res2]=0;
    printf(" %s\n",answer);
    res0+=res3;
    for(i=0;i<res2;i++) printf("%2.2x ",answer[i]); printf("\n");


    if(answer[0]== '?')  return 0;
    if(answer[0]== '@')  return -1;
    return 1;
   }          
   usleep(100000);
  }                        

 return 0;
 }
