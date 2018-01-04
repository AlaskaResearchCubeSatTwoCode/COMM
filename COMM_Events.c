#include <msp430.h>
#include <stdio.h>
#include <ARCbus.h>
#include <string.h>
#include <SDlib.h>
#include "COMM.h"
#include "AX25_EncodeDecode.h"
#include "Radio_functions.h"
#include "COMM_Events.h"
#include "SD-dat.h"
#include "i2c.h"

CTL_EVENT_SET_t ev_SPI_data;

int comm_evt_gs_decode(void){// this is called from RX_event! 
  int i, len, resp;
  unsigned char FCS[2];
  unsigned char buf[BUS_I2C_HDR_LEN+30+BUS_I2C_CRC_LEN],*ptr;

        printf("COMM_EVT_GS_DECODE\r\n");

    // Check destination address (all bit reversed!)
        if((RxBuffer[0] == 0x69) && (RxBuffer[1] == 0x19) && (RxBuffer[2] == 0x66) && (RxBuffer[3] == 0x29) && (RxBuffer[4] == 0x05) && (RxBuffer[5] == 0x02)){
           printf("Destination address good!\r\n");
        } else{ return ERR_BAD_COMM_DEST_ADDR; }
    // Check source address (all bit reversed!)
        if((RxBuffer[7] == 0x75) && (RxBuffer[8] == 0x19) && (RxBuffer[9] == 0x76) && (RxBuffer[10] == 0x61) && (RxBuffer[11] == 0x0D) && (RxBuffer[12] == 0x21)){
        printf("Source address good!\r\n");
        } else{ return ERR_BAD_COMM_SRC_ADDR;}
    // Verify CRC  
        FCS[0]=RxBuffer[RxBuffer_Len-2];
        FCS[1]=RxBuffer[RxBuffer_Len-1];
        RxBuffer[RxBuffer_Len-2]=0x00;
        RxBuffer[RxBuffer_Len-1]=0x00;
        RxBuffer_Len = RxBuffer_Len-2;
        CRC_CCITT_Generator(RxBuffer, &RxBuffer_Len);
        if((FCS[0] == RxBuffer[RxBuffer_Len-2]) && (FCS[1] == RxBuffer[RxBuffer_Len-1])){
        printf("CRC checked\r\n");
        } 
        else
        { return ERR_BAD_COMM_CRC;
        printf("CRC bad\r\n");
        }
    //GS command to COMM

    status.Num_CMD++; //Increment number of commands received

        if(RxBuffer[16] == 0xC8) 
        { 
          printf("subsystem address: 0x%02x\r\n",RxBuffer[16]);
//          printf("num: 0x%02x\r\n",RxBuffer[17]);
//          printf("cmd: 0x%02x\r\n",RxBuffer[18]);
          switch(__bit_reverse_char(RxBuffer[18]))
          {
          //NOTE Add code to deal with turning off amplifiers during RF_OFF.
            case COMM_RF_OFF:
              beacon_on=0;
               printf("COMM_RF_OFF\r\n"); 
              return RET_SUCCESS;
            case COMM_RF_ON:
              beacon_on=1;
              printf("COMM_RF_ON\r\n");
              return RET_SUCCESS;
            case COMM_BEACON_STATUS:
              beacon_flag=1;
              printf("COMM_BEACON_STATUS\r\n");
              return RET_SUCCESS;
            case COMM_BEACON_HELLO:
            printf("COMM_BEACON_HELLO\r\n");
              beacon_flag=0;
              return RET_SUCCESS;
            case COMM_RESET_CDH:
            printf("COMM_RESET_CDH\r\n");
              return COMM_CDH_reset();
            case COMM_SEND_DATA:
              len = __bit_reverse_char(RxBuffer[17]);
              for(i=0;i<len;i++){
                  buf[i]=__bit_reverse_char(RxBuffer[19+i]);
              }
              printf("COMM_SEND_DATA\r\n");
              return COMM_Send_Data(buf);
            case COMM_DEPLOY_UHF:
              return DeployUHF();
            default:
              return ERR_UNKNOWN_COMM_CMD;
          }
       } 
       //NOTE: BUS_cmd_init(buf, RxBuffer[18]));   should pass the non-Comm command at position RXbuffer[18] to the correct subsystem.
       else 
       { 
         printf("Sending GS CMD to Arcbus\r\n");
         len = __bit_reverse_char(RxBuffer[17])+3;
         printf("subsystem address: 0x%02x, len: %d\r\n",__bit_reverse_char(RxBuffer[16]), len);
       
         ptr=BUS_cmd_init(buf, RxBuffer[18]);        
         for(i=0;i<len;i++)
         {             //fill in telemetry data
           ptr[i]=__bit_reverse_char(RxBuffer[16+i]);
          }

           resp=BUS_cmd_tx(__bit_reverse_char(RxBuffer[16]),buf,len,0);
          if(resp!=RET_SUCCESS){ printf("Failed to send GS CMD to 0x%02x, %s\r\n",__bit_reverse_char(RxBuffer[16]), BUS_error_str(resp));}
       }
      return RET_SUCCESS;
}

//return error strings for error code
const char *COMM_error_str(int error){
  //check for error
  switch(error){
    case RET_SUCCESS:
      return "SUCCESS";
    case ERR_BAD_COMM_DEST_ADDR:
      return "ERROR BAD DESTINATION ADDRESS";
    case ERR_BAD_COMM_SRC_ADDR:
      return "ERROR BAD SOURCE ADDRESS";
    case ERR_BAD_COMM_CRC:
      return "ERROR BAD CRC";
    case ERR_UNKNOWN_COMM_CMD:
      return "ERROR UNKNOWN COMM COMMAND";
    case ERR_UNKNOWN_SUBADDR:
      return "ERROR UNKNOWN SUBSYSTEM ADDRESS";
    case ERR_DATA_NOT_TRANSFERED:
      return "ERROR TIMEOUT DATA NOT TRANSFERED";
    //Error was not found
    default:
      return "UNKNOWN ERROR";
  }
}

int COMM_CDH_reset(void){
    P6DIR |= CDH_RESET;
    ctl_timeout_wait(ctl_get_current_time()+3);
    P6DIR &= ~CDH_RESET;
    return RET_SUCCESS;
}

//TODO: Might have to rewrite this for ARC 2 specifics.
int COMM_Send_Data(unsigned char *data){
  int i,j;
  unsigned long start;
  unsigned int e;
  beacon_on = 0;

  for(i=0;i<COMM_TXHEADER_LEN;i++){                                             //LOAD UP HEADER
    Tx1Buffer[i]=__bit_reverse_char(Tx2_Header[i]);                             //AX.25 octets are sent LSB first
  }
  Tx1Buffer_Len=COMM_TXHEADER_LEN+(512)+1; // Set length of message: HeaderLen+Blocksize+1

  start = ((unsigned long) data[1])<<16;
  start |=((unsigned long) data[2])<<8;
  start |=((unsigned long) data[3]);

  for(i=0;i<data[4];i++){
     readSD_Data(data[0],start,Tx1Buffer+COMM_TXHEADER_LEN);
     for(j=0;j<512;j++) { //data needs to be bit reversed
        Tx1Buffer[j+COMM_TXHEADER_LEN]=__bit_reverse_char(Tx1Buffer[j+COMM_TXHEADER_LEN]);
     }
     Tx1Buffer[Tx1Buffer_Len-1]=__bit_reverse_char(COMM_CR);                     //Add carriage return

     //**** Create AX.25 packet (needs to include FCS, bit stuffed, flags) ***
     CRC_CCITT_Generator(Tx1Buffer, &Tx1Buffer_Len);                           //Generate FCS
     Stuff_Transition_Scramble(Tx1Buffer, &Tx1Buffer_Len);                     //Bit stuff - Encode for transitions - Scramble data
     ctl_events_set_clear(&COMM_evt,COMM_EVT_CC2500_1_TX_START,0);                     //TODO make a function to select event Send to Radio to transmit 
     e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&ev_SPI_data,SPI_EV_DATA_TX,CTL_TIMEOUT_DELAY,1024);
     if(!(e&SPI_EV_DATA_TX)){
          //data not received
          beacon_on = 1;
          return ERR_DATA_NOT_TRANSFERED;
      }
      start++;
  }

  beacon_on = 1;
  return RET_SUCCESS;
}

//TODO Update with the correct Code
//First initialize 5 V EPS PDM?? timer
//2 I2C commands AXE Brandt.
int DeployUHF(void)
{
printf("MIKE IS THE BEST");
return 1;
}
