/**********************************************************************************************************************************************
The commands.c file is for commands that will be displayed through the serial terminal. 
In order to add a command you must create a function as seen below.
Then function must be added to the "const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd}" table at the end of the file.
**********************************************************************************************************************************************/
#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <terminal.h>
#include <commandLib.h>
#include <stdlib.h>
#include <ARCbus.h>
#include <SDlib.h>
#include <i2c.h>
#include <Radio_functions.h>
#include <UCA2_uart.h>  
#include "AX25_EncodeDecode.h"
#include "COMM_Events.h"
#include "COMM.h"
#include "temp.h"
#include "Radio_functions.h"

extern CTL_EVENT_SET_t COMM_evt; // define because this lives in COMM.c

//*********************************************************************************** RADIO COMMANDS *****************************************************
//NOTE do not define global vars in a local function ie "radio_select"
int writeReg(char **argv,unsigned short argc){
  char regaddr, regdata;  // expecting [radio] [address] [data]
  int radio_check;

  if(argc>3){ // input checking and set radio address 
    printf("Error : Too many arguments\r\n");
    return -1;
  }

  radio_check = set_radio_path(argv[1]);  // set radio_select  

   if (radio_check==-1) {
      printf("Error: Unknown radio \"%s\"\r\n",argv[1]);
      return -2;
    }
  else{
    //printf("Radio = %i\r\n",radio_select);
    regaddr=strtoul(argv[2],NULL,0);
    regdata = strtoul(argv[3],NULL,0);
    Radio_Write_Registers(regaddr, regdata, radio_select);
    //Radio_Write_Registers(regaddr, regdata, 1);
    printf("Wrote register 0x%02x = 0x%02x [regaddr, regdata]\r\n", regaddr, regdata);
    return 0;

  }

  printf("Error : %s requires 3 arguments but %u given\r\n",argv[0],argc);
  return -2;
}

int readReg(char **argv,unsigned short argc){
  char result, radio, regaddr;  // expecting [radio]  [address]
  int radio_check;

  if(argc>2){
    printf("Error : Too many arguments\r\n");
    return -1;
  }

  radio_check = set_radio_path(argv[1]);  // set radio_select  

   if (radio_check==-1) {
      printf("Error: Unknown radio \"%s\"\r\n",argv[1]);
      return -2;
    }
  else {
    printf("Radio = %i\r\n",radio_select);
    regaddr=strtoul(argv[2],NULL,0);
    result= Radio_Read_Registers(regaddr, radio_select);
    printf("Register 0x%02x = 0x%02x [regaddr, regdata]\r\n", regaddr, result);
  }
  return 0;
}

int status_Cmd(char **argv,unsigned short argc){
char status1, status2, radio, state1, state2;
// state info
 const char* statetbl[32]={"SLEEP","IDLE","XOFF","VCOON_MC","REGON_MC","MANCAL","VCOON","REGON","STARTCAL","BWBOOST","FS_LOCK","IFADCON","ENDCAL","RX","RX_END","RX_RST","TXRX_SWITCH","RXFIFO_OVERFLOW","FSTXON","TX","TX_END","RXTX_SWITCH","TXFIFO_UNDERFLOW"};
// read 0x00 --> 0x2E
 status1=Radio_Read_Status(TI_CCxxx0_MARCSTATE,CC1101);   // get status of CC1101
 status2=Radio_Read_Status(TI_CCxxx0_MARCSTATE,CC2500_1); // get status of CC2500
 state1=status1&(~(BIT7|BIT6|BIT5)); //get state of CC2500_1
 state2=status2&(~(BIT7|BIT6|BIT5)); //get state of CC2500_2
/*  ( printf("The CC1101 is in the SLEEP state or may be unconnected.\r\n");
  }
  else if(0x00==state2){
   printf("The CC2500 is in the SLEEP state or may be unconnected.\r\n");
  }
  else{*/
  // store stat stuff
    printf("The status of the CC1101 is %s.\r\n",statetbl[status1]);
    printf("The state of the CC1101 is %i.\r\n",state1);
    printf("The status of the CC2500_1 is %s.\r\n",statetbl[status2]);
    printf("The state of the CC2500_1 is %i.\r\n",state2);
  //}
return 0;
}

// streams data from radio argv[1]=ADR i.e stream CC1101 random
//TODO   (update for second radio)
int streamCmd(char **argv,unsigned short argc){

// input checking 
  if(!strcmp(argv[1],"value")){
    data_mode=TX_DATA_PATTERN;
    data_seed=atoi(argv[2]); // arg to stream (0xXX)
  }
  else if(!strcmp(argv[1],"random")){
    data_mode=TX_DATA_RANDOM;
    if(argc==2){
      data_seed=atoi(argv[2]);
      if(data_seed==0){
        data_seed=1;
      }
    }
    else{
      data_seed=1;
    }
  }
  // input case statment to pick from enum table in COMM.h
  ctl_events_set_clear(&COMM_evt,COMM_EVT_CC2500_1_TX_START,0); 
  printf("Push any key to stop\r\n");
  getchar(); // waits for any char 
  Radio_Write_Registers(TI_CCxxx0_PKTCTRL0, 0x00, radio_select);         // Fixed byte mode
  state = TX_END;
  return 0;
}

//Select power output from the radio chip
//TODO (update for second radio)
int powerCmd(char **argv,unsigned short argc){
  const int power_dbm[8]=             { -30, -20, -15, -10,   0,   5,   7,  10};
  const unsigned char power_PTABLE[8]={0x12,0x0E,0x1D,0x34,0x60,0x84,0xC8,0xC0};
  unsigned long input;
  int idx,i;
  int pwr;
  char *end;
  unsigned char read;

  if(argc>0){
    input=strtol(argv[1],&end,0);
    if(*end=='\0' || !strcmp(end,"dBm")){
      pwr=input;
    }else{
      printf("Error : unknown suffix \"%s\" for power \"%s\"\r\n",end,argv[1]);
      return -1;
    }
    for(i=0,idx=0;i<8;i++){
      //find the power that is closest to desired
      if(abs(power_dbm[i]-pwr)<abs(power_dbm[idx]-pwr)){
        idx=i;
      }
    }
    printf("Setting radio to %idBm\r\n",power_dbm[idx]);
    Radio_Write_Registers(TI_CCxxx0_PATABLE,power_PTABLE[idx],radio_select);
  }
  read=Radio_Read_Registers(TI_CCxxx0_PATABLE,radio_select);
  for(i=0,idx=-1;i<8;i++){
    if(power_PTABLE[i]==read){
      idx=i;
      break;
    }
  }
  if(idx==-1){
    printf("PTABLE = 0x%02X\r\n",read);
  }else{
    printf("PTABLE = %idBm = 0x%02X\r\n",power_dbm[idx],read);
  }
  return 0;
} 


// reset selected radio, if un-specified all radios rest
//TODO gets stuck in set_radio_path.
int radio_resetCmd(char **argv,unsigned short argc){
  int radio_check;

  if ( argc < 1){           // reset all radios if no args passed 
    Reset_Radio(CC1101);
    Reset_Radio(CC2500_1);
    __delay_cycles(800);                         // Wait for radio to be ready before writing registers.cc1101.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable
    
    Write_RF_Settings(CC1101);                   // Write radios Settings
    Write_RF_Settings(CC2500_1);                 // Write radios Settings

    Radio_Strobe(TI_CCxxx0_SRX, CC1101);          //Initialize CCxxxx in Rx mode
    Radio_Strobe(TI_CCxxx0_SRX, CC2500_1);        //Initialize CCxxxx in Rx mode

  }
  else{                     
    radio_check = set_radio_path(argv[1]);       // reset specified radio
    printf("radio path set to %d.\r\nradio_check = %d.\r\n",radio_select,radio_check);
    if (radio_check && -1){
      printf("Plese enter a valid radio, ex. \"CC1101, CC2500_1, CC2500_2\".\r\n");
      return -1;
    }
    printf("The %s radio has been reset.\r\n");
    Reset_Radio(radio_select);
    __delay_cycles(800);                         // Wait for radio to be ready before writing registers.cc1101.pdf Table 13 indicates a power-on start-up time of 150 us for the crystal to be stable
    Write_RF_Settings(radio_select);             // Write radios Settings
  }
  return 0;
}

// beacon_on COMM's beacon arbiter var 1 = send 
int beacon_onCmd(char **argv,unsigned short argc){
  if(argc>1){
    printf("Error : Too many arguments\r\n");
    return -1;
  }
  if(argc==1){
    if(!strcmp(argv[1],"on")){
      beacon_on=1;
    }else if(!strcmp(argv[1],"off")){
      beacon_on=0;
    }else{
      printf("Error : Unknown argument \"%s\"\r\n",argv[1]);
      return -2;
    }
  }
  printf("Beacon : %s\r\n",beacon_on?"on":"off");
  return 0;
}

// sets COMM's beacon_flag "hello" beacon var 1 = beacon("hello")
int beacon_flagCmd(char **argv,unsigned short argc){
  if(argc>1){
    printf("Error : Too many arguments\r\n");
    return -1;
  }
  if(argc==1){
    if(!strcmp(argv[1],"on")){
      beacon_flag=1;
    }else if(!strcmp(argv[1],"off")){
      beacon_flag=0;
    }else{
      printf("Error : Unknown argument \"%s\"\r\n",argv[1]);
      return -2;
    }
  }
  printf("Beacon_flag : %s\r\n",beacon_flag?"on":"off");
  return 0;
}

int tempCmd(char **argv, unsigned short argc){
  int temp_data[2];

  // setup CS pins as outputs if we keep the temp sensors put this in a setup function
  P5DIR |= BIT6+BIT7;
  temp_select = 1;  //init CC1101 temp sensor
  temp_SPI_desel();
  temp_select = 2;  //init CC2500 temp sensor
  temp_SPI_desel();

  if((argc<1)  && ((!strcmp(argv[1],"temp1") || (!strcmp(argv[1],"temp2"))))){
    printf("Incorrect input arguments./r/ntemp [temp#]/r/n");
    return 0;
  }

  set_temp_sel(argv[1]);  // set temp you want to talk to
  *temp_data = temp_read_reg();  // read temp vals
  printf("Temp on the %i and %i.\r\n",temp_data[0],temp_data[1]);
  return 0;
}


//TODO: Update for ARC 2
int BurnCmd(char **argv, unsigned short argc){
//TODO add I2C command to EPS 
  return 0;
}

//TODO This is a test  command to test code path.
int CMD(char **argv, unsigned short argc)
{
int i;
char CommCommand[] = {0x69, 0x19, 0x66, 0x29, 0x05, 0x02, 0x06, 0x75, 0x19, 0x76, 0x61, 0x0D, 0x21, 0x86, 0xC0, 0x0F, 0xC8, 0x00, 0x00, 0x00, 0x00};
char IMGCommand[] = {0x69, 0x19, 0x66, 0x29, 0x05, 0x02, 0x06, 0x75, 0x19, 0x76, 0x61, 0x0D, 0x21, 0x86, 0xC0, 0x0F, 0x28, 0x00, 0x00, 0x00, 0x00};
char LEDLCommand[] = {0x69, 0x19, 0x66, 0x29, 0x05, 0x02, 0x06, 0x75, 0x19, 0x76, 0x61, 0x0D, 0x21, 0x86, 0xC0, 0x0F, 0x88, 0x00, 0x00, 0x00, 0x00};

if(!strcmp(argv[1],"COMM"))
{
  if(!strcmp(argv[2],"RF_ON"))
  {
   CommCommand[18] = 0xFF;
   CommCommand[19] = 0x70;
   CommCommand[20] = 0xD9;
  }
  else if(!strcmp(argv[2],"RF_OFF"))
  {
  CommCommand[18] = 0x00;
  CommCommand[19] = 0x6E;
  CommCommand[20] = 0x29;
  }
  else if(!strcmp(argv[2],"BEACON_STATUS"))
  {
  CommCommand[18] = 0xF0;
  CommCommand[19] = 0x81;
  CommCommand[20] = 0x36;
  }
  else if(!strcmp(argv[2],"BEACON_HELLO"))
  {
  CommCommand[18] = 0x0F;
  CommCommand[19] = 0x9F;
  CommCommand[20] = 0xC6;
  }
  else if(!strcmp(argv[2],"RESET_CDH"))
  {
  CommCommand[18] = 0xCC;
  CommCommand[19] = 0x76;
  CommCommand[20] = 0xE9;
  }
  else if(!strcmp(argv[2],"SEND_DATA"))
  {
  CommCommand[18] = 0xAA;
  CommCommand[19] = 0x7A;
  CommCommand[20] = 0x89;
  }
  else if(!strcmp(argv[2],"DEPLOY_UHF"))
  {
  CommCommand[18] = 0x50;
  CommCommand[19] = 0x34;
  CommCommand[20] = 0xDC;
  }

  RxBuffer_Len = sizeof(CommCommand);
  for(i=0; i<RxBuffer_Len; i++)
  {
  RxBuffer[i] = CommCommand[i];
  }

}
else if (!strcmp(argv[1],"IMG"))
{
  if(!strcmp(argv[2],"IMG_CLEARPIC"))
  {
  IMGCommand[18] = 0x72;
  IMGCommand[19] = 0x90;
  IMGCommand[20] = 0xCD;
  }

 RxBuffer_Len = sizeof(IMGCommand);

  for(i=0; i<RxBuffer_Len; i++)
  {
  RxBuffer[i] = IMGCommand[i];
  }
}
else if (!strcmp(argv[1],"LEDL"))
{
  if(!strcmp(argv[2],"LEDL_LOADDATA"))
  {
  LEDLCommand[18] = 0xAA;
  LEDLCommand[19] = 0x67;
  LEDLCommand[20] = 0x24;
  }

 RxBuffer_Len = sizeof(LEDLCommand);

  for(i=0; i<RxBuffer_Len; i++)
  {
  RxBuffer[i] = LEDLCommand[i];
  }
}

comm_evt_gs_decode();

}

int SD_read(char **argv,unsigned short argc){
 #define ASCIIOUT_STR  "%c" 
 char buffer[512];
 int resp , i;
  
  //read from SD card
 resp=mmcReadBlock(strtol(argv[1],NULL,10),buffer);
  //print response from SD card
  printf("%s\r\n",SD_error_str(resp));

        for(i=0;i<9;i++){//changed the 512 to 256 which is a result of changing CHAR TO INT

        if(i<8){
          printf(ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR "\r\n",
          buffer[i*28+1],buffer[i*28+2],buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }

        else{
          printf(ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR "\r\n",
          buffer[i*28+1],buffer[i*28+2],buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }
        }
        return 0;
}

//***************************************MUST WRITE IN BLOCKS OF 512 OR IT WONT WORK!!!!!*******************************************
int SD_write(char **argv,unsigned short argc){
char buff[512];
int mmcReturnValue, result , i;

//populate buffer block
  for(i=2;i<=argc;i++) {// ignore 0 *argv input (argv[0]--> "SD_write" )
    strcat(buff,argv[i]); // appends chars from one string to another
    strcat(buff,"|");     // separates strings with | as strcat eats NULL
  }

//write to SD card
  result= mmcWriteBlock(strtol(argv[1],NULL,10),buff); //(unsigned char*) casting my pointer(array) as a char 
 
  if (result>=0){ // check SD write 
  printf("SD card write success.\r\n");
  }
  else{
    printf("SD card write failed.\r\nError %i\r\n",result);
  }
  return 0;
}

int freq_cmd(char**argv,unsigned short argc){
  long fcar=437565000,fh,fl,fo,delta; // use long or freq will not fit
  int change,dsign;
  char radio;
  char * end;
  unsigned char val;

  if(!strcmp(argv[1],"CC1101")){
    radio = CC1101;
  }
  else if(!strcmp(argv[1],"CC2500")){
    radio = CC2500_1;
  }
  else{
    printf("Enter a the correct radio address/r/n");
    return 0;
  }
  fh=strtol(argv[2],&end,10);       // convert string user input to long int val for high freq input
  fl=strtol(argv[3],&end,10);
  if(radio!=255){                  // check radio input, unknown radio is 255
    printf("\r\n##########\r\ninput paramaters are fH %li and fL %li\r\n",fh,fl);
    fo=(fh+fl)/2;  // calc input center freq
    printf("The input center frequency (fo) is %li\r\n",fo);
    printf("High low frequency delta is %li\r\n",llabs(fh-fl)); 
    delta=fcar-fo; // calc diff in center freq's 
    printf("Delta of the center frequencys is %li\r\n",llabs(delta));

 //write reg 0x0f 0x63 where 0xf is the center freq reg and can change by steps of 400 Hz Adjusted in loop
    if(llabs(delta)>397){  //(fxosc/2^16)=396.7285
      change=delta/(RF_OSC_F/65536);    //calc change
      fcar=fcar*(RF_OSC_F/65536);  //(fxosc/2^16)*FERQ[23:0] --> carrier frequency should ==f01+-400 

      if(delta>=0){
        dsign=1;  // sign of delta is positive 
        val=0x63+change; // how many "ticks" to change the default ref 0x63 by.
      }
      else{
        dsign=0;  //sign of delta is negative 
        val=0x63-change; 
      }

      printf("\r\n##########\r\nAdjusted center frequency projection from %li to --> %li\r\n",fo,(dsign)?fo-(change*400):fo+(change*400)); // (dsign)?... selects the correct opp depending on the sign of delta
      printf("Changing reg 0x0f from 0x%02x --> 0x%02X\r\n",Radio_Read_Registers(0x0F,radio),val);
      Radio_Write_Registers(TI_CCxxx0_FREQ0,val,radio); // write reg
      printf("reg 0x0F writen as 0x%02X. \r\nRead back as 0x%02X.\r\n",val,Radio_Read_Registers(0x0F,radio));
    }
    else{
      printf("Frequency Test passed \r\n");
    }
  }
  else 
    printf("bad input\r\n");
return 0;
}


// keep track of comm software version 
//void version_cmd(char** argv,unsigned short argc){
void version_cmd(){
  printf("This current software build is Rev3.1 (5/29/2018)\r\n");
}

//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"status","",status_Cmd},
                   {"stream","[zeros|ones|[value [val]]]\r\n""Stream data from radio.\n\r",streamCmd},
                   {"writereg","Writes data to radio register\r\n [radio] [adress] [data].\n\r",writeReg},
                   {"readreg","reads data from a radio register\r\n [radio] [adrss].\n\r",readReg},
                   {"power","Changes the transmit power of the radio [radio][power].\n\rex. CC2500_1 -24\n\r",powerCmd},
                   {"radio_reset","Reset radios on COMM SPI bus.\n\rradio_reset [radio]. Note if no radio addr included all radios will be reset",radio_resetCmd},
                   {"beacon","Toggles the COMM beacon on or off.\n\rCurrently targeting the CC2500_1",beacon_onCmd},
                   {"beacon_flag","Toggles the COMM beacon \"hello\" packet on or off.\n\rCurrently targeting the CC2500_1",beacon_flagCmd},
                   {"temp","grabbing temp data",tempCmd},
                   {"burn", "Pulse the burn line", BurnCmd},
                   {"CMD", "Test Comands[Subsytem][COMMAND] eg[COMM][RF_OFF]", CMD},
                   {"SDread", "Test Command to read data from the SD card", SD_read},
                   {"SDwrite", "Test Command to Write daya to the SD card", SD_write},
                   {"FrequencyAdjust", "Test Command to read data from the SD card", freq_cmd},
                   {"version", "Display currnt Comm code version", version_cmd},
                   ARC_COMMANDS,//CTL_COMMANDS, ERROR_COMMANDS,
                   //end of list
                   {NULL,NULL,NULL}};

