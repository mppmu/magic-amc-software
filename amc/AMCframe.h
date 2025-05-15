// frame structures for sending
//send 1 byte and 1 word: set lower/upper endswitch number (b) and value (w)
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header[2] ;      //header bytes: send:fb 01
   unsigned short dst_addr ;      //destination address
   unsigned char src_addr ;       //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char index ;          //byte data
   unsigned short value ;         //parameter
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_snd_bw ;


//send 1 word: write working current (w)
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header[2] ;      //header bytes: send:fb 01
   unsigned short dst_addr ;      //destination address
   unsigned char src_addr ;       //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned short word0 ;         //word 0
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_snd_w ;


//send no data
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header[2] ;      //header bytes: send:fb 01
   unsigned short dst_addr ;      //destination address
   unsigned char src_addr ;       //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_snd_0 ;


//====================================================================

//the following structures omit/skip "fb"
//they only have as header  "85" or "15"

//receive status only (no data)
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_0 ;

typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned char box_addr ;       //source: box address
   unsigned char drv_addr ;       //source: driver address
// unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_1 ;


//receive status plus 1 byte
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned char byte0 ;          //byte 0 (version)
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_b ;

//receive status plus 2 byte
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned char byte0 ;          //byte 0 (version)
   unsigned char byte1 ;          //byte 1 (version)
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_bb ;


//receive status plus 1 unsigned (2 byte) word
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned short word0 ;         //word 0 (unique address)
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_uw ;


//receive status plus 1 (2 byte) word                                         
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   short word0 ;                  //word 0 (position)
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_w ;


//receive status plus 1 unsigned byte & 1 (2 byte) word
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned char num ;            //byte 0 (sensor number)
   unsigned char val[2] ;         //word 0 (sensor value)
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_bw ;


//receive status plus 1 (2 byte) word plus 1 unsigned byte
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK fb (85)
                                  //                               NAK fb (15)
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   short word0 ;                  //word 0 (watchdog counter)
   unsigned char byte0 ;          //byte 0 (watchdog status)
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_wb ;


//$receive status plus 2 (2 byte) words
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   short xpos ;                   //xposition
   short ypos ;                   //yposition
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_ww ;



//receive status plus 4 (2 byte) words
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned short hallval[4] ;    //hall probe values
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_wwww ;


//receive status plus 6 (2 byte) words
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned short temp ;          //temperature
   unsigned short humid ;         //humidity
   unsigned short curr ;          //current
   unsigned short volt1;          //voltage 1
   unsigned short volt2;          //voltage 2
   unsigned short voltlog;        //voltage for logic
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_wwwwww ;


//receive status plus 9 words (=18 bytes) 
typedef struct {                  //contains the structure of a frame (comminucation)
   unsigned char header ;         //header bytes: send:fb 01, recv:ACK (fb) 85
                                  //                               NAK (fb) 15
   unsigned char dst_addr ;       //destination address
   unsigned short src_addr ;      //source address
   unsigned char counter ;        //number of bytes to follow
   unsigned char function ;       //function to be executed
   unsigned char status[2] ;      //status
   unsigned short length ;        //number of steps low to high
   unsigned short lmag[4] ;       //lower endswitch: magsensor values
   unsigned short umag[4] ;       //upper endswitch: magsensor values
   unsigned char crc[2] ;         //CRC
}                                 //
AMCframe_rcv_9w ;

