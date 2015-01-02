/*
 * LCD.h
 *
 * Created: 32/13/2015 12:34:56 AM
 *  Author: Michael
 */ 

#ifndef LCD_H_
#define LCD_H_

// What pins on PORTB did we connect D/C and RST to
#define DCPIN		7
#define RSTPIN		3

// What PORTB pins are the SPI lines on
#define MOSIPIN		2
#define SCKPIN		1
#define SCEPIN		0

// LCD Command and Data
#define LCD_C		0
#define LCD_D		1

// LCD Contrast levels, you may have to change these for your display
#define LCD_LOW_CONTRAST		0x2F
#define LCD_DEFAULT_CONTRAST	0x3F
#define LCD_HIGH_CONTRAST		0x4F

// Dimensions of the LCD Screen
#define LCD_X		84
#define LCD_Y		48

// CPU Frequency Defines
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CPU_8MHz        0x01

// Helpful Macros
#define SET_INPUT(portdir, pin)			portdir &= ~(1 << pin) 
#define SET_OUTPUT(portdir, pin)		portdir |= (1 << pin)
#define OUTPUT_WRITE(port, pin, value)	port = (port & ~(1 << pin)) | (value << pin)
#define OUTPUT_LOW(port, pin)			port &= ~(1 << pin)
#define OUTPUT_HIGH(port, pin)			port |= (1 << pin)
#define READ_LINE(port, pin)			port & (1 << pin)

// SPI Helper Functions
void SPIInit(void);
void SPIWriteByte(unsigned char byte);

// LCD General Functions
void LCDWrite(unsigned char dc, unsigned char data);
void LCDInitialise(unsigned char contrast);
void LCDClear(void);

// LCD Text functions
void LCDCharacter(unsigned char character);
void LCDString(unsigned char *characters);

// LCD Memory Functions
void LCDPosition(unsigned char x, unsigned char y);


#endif /* LCD_H_ */