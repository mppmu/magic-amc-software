// amcab.c
int AMCtime(long utime[5]);
void put_logfile(int type, int flag, char* str);

// ab.c
int IF_read(AMCpanel panel[17][17], int fileID);
int NEW_read(AMCpanel p[17][17]);
int LUT_read(AMCpanel p[17][17], int nnew);
int PWR_read(AMCpower  p[32]);
int AMC_read(AMCpanel panel[17][17]);

// ha_m1.c
int AMC_exec_pwr(int type, int soll[32], int ist[32]);
int ausleep(int i);

// distlas.inc
int AMC_exec_dist(int cmd );

// amcmotor_m1.inc
int send_command(int ip, unsigned char *comd, AMCpanel* ptr );
int get_response1( int globalchain, unsigned char lbufout[256], int* res0, int resin);
int get_int( int res, unsigned char lbuf[250], int intbuf[10]);

// distlas.inc
int send_string232(char *text, int n0, char answer[256]);

// haroutines_m1.inc
int imin(int a, int b);
int imax(int a, int b);

// blueboard485.inc
int open_485_blueboard();

int aioctl(int fildes, int op, ... /* arg */);

