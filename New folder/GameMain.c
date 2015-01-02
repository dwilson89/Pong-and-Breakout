#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "Graphics.h"
#include "usb_serial.h"
#include "ADC.h"
#include "ASCIIFont.h"
#include "LCD.h"
#include "pong.h"
#include "breakout.h"
//define buttons - BUTTON_ONE - (Start, up/right), BUTTON_TWO - (Select, down/left)
#define BUTTON_ONE		(PORTD |= (1<<PD1))
#define BUTTON_TWO		(PORTD |= (1<<PD2))

#define LED1_CONFIG      (DDRF |= (1<<PF7))
#define LED1_OFF         (PORTF &= ~(1<<PF7))

#define LED_PORT		PORTF
#define LED1    		PF7


void GenerateIntroScreen();
void UpdateOptions(int option);
void Countdown();

int main(void){

	BUTTON_ONE;
	BUTTON_TWO;

	LED1_CONFIG;
	LED1_OFF;

	usb_init();
	unsigned char contrast = 100;
	LCDInitialise(contrast);
	int gameState = 0; //0 - Intro, 1 - Pong, 2 - Break Out

	//to keep track of option selected;
	int option = 1;

	while(1){

		GenerateIntroScreen();
		UpdateOptions(option);

		while (gameState == 0){

			//GenerateIntroScreen();
			UpdateOptions(option);

			//look for button press		
			//if button one (start button);
			if(!(PIND & (1 << PD1))){

				gameState = option;
				
			} else if(!(PIND & (1 << PD2))){

				option++;
				_delay_ms(100);
			}

			//resets option choice
			if(option > 2){

				option = 1;

			}
		}

		if(gameState == 1){
			Countdown();
			Pong();

		} else if(gameState == 2){
			Countdown();
			BreakOut();

		}

		gameState = 0;
		option = 1;
	}
	return 0;
}

//Function to generate the Intro Screen
void GenerateIntroScreen(){

	LCDClear();

	unsigned char * studentNumber = "06325157";
	unsigned char * studentName = "D Wilson";

	int xPosStuName = (83 - 8*7)/2;
	int yPosStuName = 0;

	int xPosStuNo = (83 - 8*7)/2;
	int yPosStuNo = 1;

	LCDPosition(xPosStuName, yPosStuName);
	LCDString(studentName);
	LCDPosition(xPosStuNo, yPosStuNo);
	LCDString(studentNumber);

}

void UpdateOptions(int option){

	unsigned char * pongString = " Pong";
	unsigned char * pongStringO = ">Pong";

	unsigned char * breakOutString = " Break Out ";
	unsigned char * breakOutStringO = ">Break Out";

	int xPongStr = (83 - 5*7)/2;
	int yPongStr = 3;

	int xBOStr = (83 - 10*7)/2;
	int yBOStr = 4;

	if (option == 1){
		
		LCDPosition(xBOStr, yBOStr);
		LCDString(breakOutString);
		LCDPosition(xPongStr, yPongStr);
		LCDString(pongStringO);

	} else if (option == 2){

		LCDPosition(xPongStr, yPongStr);
		LCDString(pongString);
		LCDPosition(xBOStr, yBOStr);
		LCDString(breakOutStringO);

	}
}

//Function to Countdown
void Countdown(){

	int countDownInt = 0;

	unsigned char cDChar = '3';

	while(countDownInt < 3){

		LCDClear();
		_delay_ms(500);
		LCDPosition((83-7)/2, 3);
		LCDCharacter(cDChar - countDownInt);
		_delay_ms(500);
		countDownInt++;
	}
}
