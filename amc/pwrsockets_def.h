/*pwrsockets_def.h*/

  #define ETHERPWR2  "ihp-pwr1a"
  #define ETHERPWRPORT 5025
  #define ETHERPWR1  "ihp-pwr2a"

  int fdpwr[2];
  FILE *comfi;
  unsigned char pwr_string[80];

  struct sockaddr_in servernamepwr[2];

