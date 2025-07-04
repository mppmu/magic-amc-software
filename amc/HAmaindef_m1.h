//  int  i,j,k,l, nled, nact=0, flrd;
    int  i,j,k,l, nled,  flrd, it;
    int  anyaction, portaction[n8], ok, iport, calflg;
    static long mainusec0, mainsec0;
    long mainusec, mainsec;
    FILE *param, *parampos;
    char line[120];
    char  linepos[110];
    int   ii, ij, iX, iY, iIsReversed, iIsInstalled;
    float dLaserX, dLaserY, dAxisX, dSlopeX, dConvX, dAxisY, dSlopeY, dConvY;
    int px,py,po[3],ap[3],ps[3],ad[3],is = 0,npanel;
    int qx,qy;
    int ty,bo[3];        //old
    int ix1,ix2,iy1,iy2;
    int po0, po1, bo1;
    long utime[5];

    char Rascii[512], logfilename[60];                // RS232 text
    int nascii;

    float u, s, b[4], d[4];
    unsigned char calibfile[150];

    int   iel, ieu;             //default: Int_Endswitch_Low, ...
    float rel, rel1, reu, reu1; //Real_Endswitch_Low, .., Real_Endswitch_Up

    time_t  ttt;    /* long int */
    time_t  *tp;
    struct tm      *pttm;

    tv=&atv;
    tz=&atz;
    word = (int *) &byte;
