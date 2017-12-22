//autogenerated by msp430-gen.py
#ifndef _PINS_H
#define _PINS_H

#define CLOCK_PINS        (BIT2|BIT5|BIT4)
#define CLOCK_DIR         P1DIR
#define CLOCK_SEL0        P1SEL0
#define CLOCK_SEL1        P1SEL1

#define UART_TX_PIN       BIT3  //P4.3
#define UART_RX_PIN       BIT4  //P4.4
#define UART_TX_PIN_NUM   3
#define UART_RX_PIN_NUM   4
#define UART_PINS         (UART_RX_PIN|UART_TX_PIN)
#define UART_PORT         4

//define serial pins
#define BUS_PIN_SDA       BIT1
#define BUS_PIN_SCL       BIT0

#define BUS_PINS_I2C      (BUS_PIN_SDA|BUS_PIN_SCL)

#define BUS_PIN_SCK       BIT0  //SPI for the header UCB1
#define BUS_PIN_SOMI      BIT1
#define BUS_PIN_SIMO      BIT2
  
#define BUS_PINS_SPI      (BUS_PIN_SOMI|BUS_PIN_SIMO|BUS_PIN_SCK)  

#define BUS_PINS_SER      (BUS_PINS_SPI|BUS_PINS_I2C)

#endif
