#ifndef __COMM_Events_H
#define __COMM_Events_H

  int comm_evt_gs_decode(void);

//Return values from COMM Event functions (RET_SUCCESS = 0 defined in ARCbus.h)
enum{ERR_BAD_COMM_DEST_ADDR=-1,ERR_BAD_COMM_SRC_ADDR=-2,ERR_BAD_COMM_CRC=-3,ERR_UNKNOWN_COMM_CMD=-4,ERR_UNKNOWN_SUBADDR=-5,ERR_DATA_NOT_TRANSFERED=-6};

const char *COMM_error_str(int error);

extern CTL_EVENT_SET_t ev_SPI_data;

//events for ev_SPI_data
enum{SPI_EV_DATA_REC=0x0001,SPI_EV_DATA_TX=0x0002};

  //command table for GS commands
  enum{COMM_RF_OFF=0x00, COMM_RF_ON=0xFF, COMM_BEACON_STATUS=0x0F, COMM_BEACON_HELLO=0xF0, COMM_RESET_CDH=0x33, COMM_GET_DATA=0xAA, COMM_SEND_DATA=0x55, COMM_DEPLOY_UHF = 0x0A};

int COMM_CDH_reset(void);
int COMM_Get_Data(unsigned char *data);
int COMM_Send_Data(unsigned char *data);
int DeployUHF(void);

#endif




