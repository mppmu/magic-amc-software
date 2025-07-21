#define PANELPOSFILE  "Mag2PanelPos6.txt"
#define PI            3.1415926


#define AMC_errmax    10      //stop execution oif #err exceeds this value

#define AMCfreq       750      //frequency
#define AMCfreq_reduct  0.7    //frequency reduction for calc of deltime for "new centre"
#define AMCwrngfrq      1      //wrong frequency status bit
#define AMCwork     0.700      //working current A
#define AMCwrngwrk      2      //wrong working current status bit
#define AMChold     0.070      //holding current A
#define AMCwrnghld      4      //wrong holding current status bit
#define AMCwrngID       8      //wrong ID
#define AMCwrngpos    128      //wrong position: act_mot and pc_mot different
#define AMCwrnglwn    256      //wrong lower sensor number
#define AMCwrnglwl    512      //wrong lower sensor level
#define AMCwrngupn   1024      //wrong upper sensor number
#define AMCwrngupl   2048      //wrong upper sensor level
#define AMClasonoff  4096      //laser on (off)
#define AMCwrngvers 32768      //wrong version

#define AMClowsns       0      //sensor number for lower endswitch
#define AMClowval     570      //default sensor value  for lower endswitch
#define AMCupsns        3      //sensor number for upper endswitch
#define AMCupval      430      //default sensor value  for upper endswitch

#define AMCminvers     28      //minimum version number which is accepted

#define AMCnhall      200      //length of arrays with calibration data for hall probes
#define AMCnohallpos    0      //dont use hall probes for position determination: dont read in "query/moveby"

#define AMCnmintime 25000      //min time diff between commands on same chain  (usec)


#define AMCactmaxadd 0x8240    //max actuator address

#define AMCwritelog2    0      //(0) 1 (dont) write, else write



//maximum current per power supply : macu must be >=11
//final pwr supp gives 50A>=2*macu
//#define macu           23.
//#define macu           14.     //test
#define macu           25.     //actual

//#define ntodomin       11      //bursting if number of panel todo is at least "todomin"
#define ntodomin        2      //bursting if number of panel todo is at least "todomin"

#define niter           0   //AB 20.3.09   //    3      //number of iteration when moving relative
#define nroundmaxmax    3      //max number of iterations of a command:
                               //burst_and_get truncates input value of nroundmax to this

#define AMCstepmin     4      //accept MOVE if abs(act_mot - pc_mot)<AMCstepmin


//#define kactactmax0   22      //max number of simultaneously moving actuators in one chain
#define kactactmax0   14      //max number of simultaneously moving actuators in one chain: 10A/700mA


#define AMCtimeoff 1225817984 //time offset



//TIMES
#define AMC_dofew10000     10000
#define AMC_dofew100000   100000
#define AMC_dofew200000   200000
#define AMC_burst25000      2500

//Adjusting frequency, currents
//#define AMC_doadjfreq         0    //adjust frequency; 0 means: dont do!
//#define AMC_doadjwork         0    //adjust working current
//#define AMC_doadjhold         0    //adjust holding current
#define AMC_doadjall            0    //adjust all that is adjustable if !=0
#define AMC_burst25000     2500    //wait that much ms


#define OPENBLACKBOX0         0    //Blackbox 0 for chains 0=7
                                   //replaced by interface in MPL/PIP blue computer
#define OPENBLACKBOX1         0    //Blackbox 1 for chains 9=15


#define PORTFLAGSETTING       0    //portflag can not be set for PORTFLAGSETTING<=0,


