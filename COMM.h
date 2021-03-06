#ifndef __COMM_H
#define __COMM_H

typedef int BOOL;
#define TRUE 1
#define FALSE 0
#define CDH_RESET       BIT6 //P6.6

// ARC1(CDH) --> ARC2(COMM) delays code update
#define ANT_DEPLOY_TIME         (10ul*60ul*1024ul) //10 minutes
#define RF_ON_TIME              (45ul*60ul*1024ul) //45 min = 45*60*1024
#define BURN_DELAY              (10ul*1024ul)  //10 sec

//P1IV definitions. These MUST match to the pins above!!!!!
#define CC1101_GDO0_IV      P1IV_P1IFG1  // interrupt on P1.0
#define CC1101_GDO2_IV      P1IV_P1IFG2  // interrupt on P1.1
#define CC2500_1_GDO0_IV    P1IV_P1IFG3  // interrupt on P1.2
#define CC2500_1_GDO2_IV    P1IV_P1IFG4  // interrupt on P1.3


//events for COMM task
extern CTL_EVENT_SET_t COMM_evt;
extern CTL_EVENT_SET_t COMM_evt2;

extern short beacon_on, beacon_flag;

//events for COMM task one
extern CTL_EVENT_SET_t COMM_evt;

//events in COMM_evt one

  enum{COMM_EVT_CC1101_RX_READ=1<<0, COMM_EVT_CC1101_TX_START=1<<1, COMM_EVT_CC1101_TX_THR=1<<2,
   COMM_EVT_CC1101_TX_END=1<<3, COMM_EVT_CC2500_1_RX_READ=1<<4, COMM_EVT_CC2500_1_TX_START=1<<5, 
   COMM_EVT_CC2500_1_TX_THR=1<<6, COMM_EVT_CC2500_1_TX_END=1<<7, COMM_EVT_CC2500_2_RX_READ=1<<8,
   COMM_EVT_CC2500_2_TX_START=1<<9, COMM_EVT_CC2500_2_TX_THR=1<<10, COMM_EVT_CC2500_2_TX_END=1<<11, 
   COMM_EVT_IMG_DAT=1<<12, COMM_EVT_LEDL_DAT=1<<13, COMM_EVT_STATUS_REQ=1<<14, COMM_EVT_GS_DECODE=1<<15};

 #define COMM_EVT_ALL (COMM_EVT_CC1101_RX_READ | COMM_EVT_CC1101_TX_START | COMM_EVT_CC1101_TX_THR | COMM_EVT_CC1101_TX_END | COMM_EVT_CC2500_1_RX_READ | COMM_EVT_CC2500_1_TX_START | COMM_EVT_CC2500_1_TX_THR | COMM_EVT_CC2500_1_TX_END | COMM_EVT_CC2500_2_RX_READ | COMM_EVT_CC2500_2_TX_START | COMM_EVT_CC2500_2_TX_THR | COMM_EVT_CC2500_2_TX_END | COMM_EVT_IMG_DAT | COMM_EVT_LEDL_DAT | COMM_EVT_STATUS_REQ | COMM_EVT_GS_DECODE)

// events for COMM task two 
  extern CTL_EVENT_SET_t COMM_evt2;
  enum{COMM_EVT2_RF_EN=1<<0,COMM_EVT2_BURN_DELAY=1<<1};
  #define COMM_EVT2_ALL (COMM_EVT2_RF_EN | COMM_EVT2_BURN_DELAY)  //flag register for COMM events 2
 
  //structure for status data from COMM
  typedef struct{
    unsigned char CC1101;	//MARCSTATE of CC1101 radio
    unsigned char CC2500_1;     //MARCSTATE of CC2500_1 radio
    unsigned char CC2500_2;     //MARCSTATE of CC2500_2 radio
    unsigned char Num_CMD;      //Number of commands received
    unsigned short ACDS_data;	//#ACDS packets in COMM SD card
    unsigned long LEDL_data;   	//#LEDL packets in COMM SD card
    unsigned short IMG_data;    //#IMG packets in COMM SD card
  }COMM_STAT;

  extern COMM_STAT status;

  extern unsigned char data_seed;

  extern short data_mode;
  //flags for STAT_PACKET

  //parse events from the bus for the subsystem
  void sub_events(void *p);

  //parse COMM specific events
   void COMM_events(void *p);
 //parse COMM 2 specific events
   void COMM_events2(void *p);
  
  // beacon timer setup
  void COMM_beacon_setup(void);

  void Radio_Interrupt_Setup(void);
  void PrintBuffer(char *dat, unsigned int len);
  void PrintBufferBitInv(char *dat, unsigned int len);
  int COMM_parseCmd(unsigned char src,unsigned char cmd,unsigned char *dat,unsigned short len,unsigned char flags);


  extern CMD_PARSE_DAT COMM_parse;
  extern unsigned char Tx1Buffer[];
  extern unsigned char RxBuffer[];
  extern unsigned int Tx1Buffer_Len, TxBufferPos, TxBytesRemaining;
  extern unsigned int RxBuffer_Len,  RxBufferPos, RxBytesRemaining;
  extern unsigned int state, small_packet, PkftLenUpper, PktLenLower, PktLen;
  extern BOOL INFINITE;
  extern char temp_countTX, temp_countRX, RxFIFOLen;
  extern int Tx_Flag; //used in RF_Send_Packet not sure why
  extern unsigned char IMG_Blk;

#endif
