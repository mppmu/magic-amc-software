    #define n5000 50
    unsigned char globalnech1a, globalnech1b;
    unsigned char globallbuf[4096];
    unsigned char lbuf[256];
    unsigned char cmdbf[60][40];
    char cmdtxt[256][40];
    unsigned char mcntr[2] = { 0xf0, 0xf0 };
//  unsigned char mcntr[2] = { 0x8f, 0x8f };
    int resin[512];
    int  cmdmvx, cmdmvaxy, cmdcntr, cmdr4ms, cmdscms;
    int  cmdresb,cmdresd, cmdkill, cmdlasn, cmdlasf, cmdrdfq, cmdwtfq;
    int  cmdrdwc,cmdwtwc, cmdrdhc, cmdwthc, cmdrdE1, cmdwtE1, cmdrdEa;
    int  cmdwtEa,cmdrdth, cmdrdmt, cmdrsnd, cmdpwof, cmdpwon, cmdsdvn;
    int  cmdrdlsw,cmdwtlsw,cmdrdusw,cmdwtusw,cmdnlasf,cmdnlasn, cmdncnt;
    int  cmdsdui,cmdrdfr, cmdqwdc, cmdwtmt, cmdrdemt, cmdwtemt      ;
    int  cmdrlas                                                   ;
    int  nwd, nwd1, wrdbuf[256];
    long int lwrdbuf[256];
//  float fwrdbuf[125], msenslu[8];
