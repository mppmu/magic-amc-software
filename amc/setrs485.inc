 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>


typedef enum {
	OSCI_IOCTL_RS232 	= 1 << 0,
	OSCI_IOCTL_RS485 	= 1 << 1,
	OSCI_IOCTL_FD	 	= 1 << 2,
	
	OSCI_IOCTL_HW_RTS	= 1 << 0,
	OSCI_IOCTL_HW_CTS	= 1 << 1,
} OSCI_IOCTL_MODE;

#define OIOC_SETMODE		_IOW('O', 0x00, unsigned int)
#define OIOC_GETMODE		_IOR('O', 0x01, unsigned int)
#define OIOC_SETFIFO		_IOW('O', 0x02, unsigned int)
#define OIOC_SETFLOW		_IOW('O', 0x03, unsigned int)
#define OIOC_SETRS485DLY	_IOW('O', 0x04, unsigned int)

static char const *
GetRSMode(OSCI_IOCTL_MODE mode)
{
	if (mode & OSCI_IOCTL_RS232)
		return "RS232";
	else if (mode & OSCI_IOCTL_RS485)
		if (mode & OSCI_IOCTL_FD)
			return "RS485 Full-Duplex";
		else
			return "RS485 Half-Duplex";
	else
		return "OFF";
}


static void
Usage(char * progname)
{
	printf("Usage: %s [options] osci_device\n", progname);
	puts("tool to dump/set mode of an OSCI port");
	puts("options:");
	puts("\t-a [0|1]\t disable/enable auto RTS/CTS handshake");
	puts("\t-d [0-15]\t set RS485 HD Tx shutoff delay");
	puts("\t-f [0|1]\t disable/enable FULL Duplex");
	puts("\t-h\t\t this help");
	puts("\t-r [2|4]\t 2 = RS232/ 4 = RS485");
}

int
main(int argc, char *argv[]) {
	int c;
	int dev;
	char *devicename;
	int autortscts = 0;
	int rs485 = 0;
	int fduplex = 0;
	int set = 0;
	int rs485delay = -1;
	
	printf("OSCIMode tool V"PROGVERS" (c) ELSOFT AG\n");

	opterr = 0;
	while ((c = getopt(argc, argv, "a:d:f:hr:")) != -1) {
		switch (c) {
			char arg;
			case 'a':
				arg = optarg[0];
				if (arg == '0' || arg == '1') {
					autortscts = (optarg[0] == '1');
					set = 1;
				} else {
					Usage(argv[0]);
				}
				break;
				
			case 'd':
				rs485delay = atoi(optarg);
				if ((rs485delay < 0) || (rs485delay > 15))
					Usage(argv[0]);
					
				break;
					
			case 'f':
				arg = optarg[0];
				if (arg == '0' || arg == '1') { 
					fduplex = (optarg[0] == '1');
					set = 1;
				} else {
					Usage(argv[0]);
				}
				break;
			
			case 'h':
				Usage(argv[0]);
				return 0;
				
			case 'r':
				arg = optarg[0];
				if (arg == '2' || arg == '4') { 
					rs485 = (optarg[0] == '4');
					set = 1;
				} else {
					Usage(argv[0]);
				}
				break;
				
			case '?':
				printf("unknown option '%c'\n", optopt);
				Usage(argv[0]);
				return -1;
		} // endswitch
	} // endwhile
		
	if (optind == argc) {
		Usage(argv[0]);
		return -1;
	} else
		devicename = argv[optind];
		
	if ((dev = open(devicename, O_RDWR)) >= 0) {
		unsigned int mode = 0;
		if (set) {
			if (!rs485) {
				mode |= OSCI_IOCTL_RS232;
			} else {
				mode |= OSCI_IOCTL_RS485;
				if (fduplex)
					mode |= OSCI_IOCTL_FD;
			} // endif
			printf("port mode(set): %s\n", GetRSMode(mode)); 
		
			if (ioctl(dev, OIOC_SETMODE, &mode))
				perror("ioctl(OIOC_SETMODE)");
			if (!rs485) {
			        mode = 0;
				if (autortscts)
				    mode = OSCI_IOCTL_HW_RTS | OSCI_IOCTL_HW_CTS;
				    
				if (ioctl(dev, OIOC_SETFLOW, &mode))
					perror("ioctl(OIOC_SETFLOW)");
			} // endif
		} // endif
		
		if (rs485delay != -1)
			if (ioctl(dev, OIOC_SETRS485DLY, &rs485delay))
				perror("ioctl(OIOC_SETRS485DLY)");
		
		if (!ioctl(dev, OIOC_GETMODE, &mode)) {
			printf("port mode: %s\n", GetRSMode(mode));
		} else {
			perror("ioctl(OIOC_GETMODE)");
		} // endif
				
		if (close(dev))
			perror("close");
	} else {
		perror("open");
	} // endif
			
	return 0;
}
