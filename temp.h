#ifndef __TEMP_H
#define __TEMP_H

#define Temp_Sensor1_CS BIT7; // Temp CS select lines on P5
#define Temp_Sensor2_CS BIT6;

extern int temp_select;  // global var to select temp sens 

int set_temp_sel(char *temp);
int temp_SPI_sel (void);
int temp_SPI_desel(void);
int temp_read_reg(void);

#endif
