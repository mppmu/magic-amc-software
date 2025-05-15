/*pwrsockets.h*/

// needs the following defs
//#define ETHERPWR2  "ihp-pwr1a"
//#define ETHERPWRPORT 5025
//#define ETHERPWR1  "ihp-pwr2a"

//int fdpwr[2];
//FILE *comfi;

//struct sockaddr_in servernamepwr[2];

//-----------------------------------------------------------    // 

  printf("create sockets for pwr \n");
for(i= 0;i<2;i++){
  printf("pwr port %d\n",i);                                                               //AB

  fdpwr[i]  = socket (PF_INET, SOCK_STREAM, 0);     
  if (fdpwr[i] < 0)                                 
    {                                               
      printf(" can not create socket (client) for pwr%d\n",i); 
    }                                                          
  else {

  // Create the socket.                                          //AB
  // Connect to the server.                                    //AB
   if(i==0) init_sockaddr (&servernamepwr[i], ETHERPWR1, ETHERPWRPORT);              //AB
   if(i==1) init_sockaddr (&servernamepwr[i], ETHERPWR2, ETHERPWRPORT);              //AB
   ierr =  connect (fdpwr[i],
                   (struct sockaddr *) &servernamepwr[i],              //AB
                   sizeof (servernamepwr[i])) ;                        //AB
   if ( ierr < 0)                                                 //AB
    {                                                            //AB
      printf ("connect to etherpwr failed (client) %d\n",ierr ); //AB

    }                                                            //AB
   else {
      printf ("connect to etherpwr%d worked (client) %d\n",i,ierr ); //AB
    }
  }
}                                                                 //AB

