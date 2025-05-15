
typedef struct {
  int request ;             //              -1=not_def   0=off  1=on     (latest user interaction)
  int nominal ;             // 'soll-wert'  -1=not_def   0=off  1=on
  int actual  ;             // 'ist-wert'   -1=error     0=off  1=on
  int old_nom ;             // 'soll-wert' vor der aenderung
  int old_act ;             // 'ist-wert'  vor der aenderung
  int deflt   ;             //                           0=off  1=on  2=always on
  int type    ;             //  0=fixed      1=changeable
  int chan    ;             // which channel in the box is used
  unsigned long errtim ;    // time when inconsistency/error appeared
  char str[16];             // name to print on the button
} AMCpower ;

#define PWR_SBIG  2
#define PWR_AMC   4
#define PWR_DIST 16

#define PWR_0_TH  0
#define PWR_Q_TH  1
#define PWR_S_TH  2

