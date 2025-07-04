 unsigned char chain_pattern[8] = { 3, 12, 48, 196, 3, 12, 48, 196 };  //pattern for chains
 int           chain_channel[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };          //channel; for chains
 static unsigned char USCMreg[4] = { 0, 0, 0, 0 };        //on/off status
 unsigned char prompt[12] = { 0x0a,0x0d,0x4d,0x4f,0x4e,0x49,0x54,0x4f,0x52,0x3e,0x20,00 };
 unsigned char nimple[12] = { 0x6e,0x6f,0x74,0x20,0x79,0x65,0x74,0x20,0x69,0x6d,00,00 };
 unsigned char bla[1024];
 static int prompted =0;                 //1=in prompt mode = MONITOR>
 int    kascii;
 char ascii[1024];

