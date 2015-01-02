/*
 * Graphics.c
 *
 * Created: 20/12/2012 11:59:59 PM
 *  Author: Michael a.k.a Not that there's anything wrong with that
 */ 

#include "Graphics.h"
#include "LCD.h"

unsigned char screenBuffer[LCD_BUFFER_SIZE]; // Our screen buffer array

void SetPixel(unsigned char x, unsigned char y, unsigned char value) {
	// Calculate the LCD row
	int row = y/8;
	// Calculate the pixel 'subrow', within that LCD row
	int subRow = y%8;
	
	// Set that particular pixel in our screen buffer
	//if value is set to high
	if(value == 1){
		
		screenBuffer[(84*row)+x] |= (1 << subRow);
	
	//else if value is set to low
	}else if (value == 0){
		
		screenBuffer[(84*row)+x] &= ~(1 << subRow);
	
	}
}

void DrawLine(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2) {
	// Insert algorithm here.
	int dx = x2 - x1;
	int dy = y2 - y1;
	int x;
	int y;
	
	if(x1 == x2){
	
		for(y = y1; y <= y2; y++){
				
			//y = y1 + (dy) * (x-x1)/(dx);
		
			SetPixel(x1, y, 1);
			
		}
	} else {

		for(x = x1; x <= x2; x++){
		
			y = y1 + (dy) * (x-x1)/(dx);
		
			SetPixel(x, y, 1);
	
		}
	}
	//PresentBuffer();
	
}

void PresentBuffer(void) {
	// Reset our position in the LCD RAM
	LCDPosition(0,0);
	
	// Iterate through our buffer and write each byte to the LCD.
	int i;
	for(i = 0; i < LCD_BUFFER_SIZE; i++){
		
		LCDWrite(LCD_D, screenBuffer[i]);
		
	}
}

void ClearBuffer(void) {
	// Iterate through our buffer and set each byte to 0
	int i;
	
	char reset = 0;
	
	for(i = 0; i < LCD_BUFFER_SIZE; i++){
	
		screenBuffer[i] &= reset;
		
	}
}

// Extend this file with whatever other graphical functions you need for your assignment...
void DrawBox(unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2){

	//line 1 x1,y1 - x1,y2
	DrawLine(x1,y1,x1,y2);
	//line 2 x1,y1 - x2,y1
	DrawLine(x1,y1,x2,y1);
	//line 3 x2,y1 - x2,y2
	DrawLine(x2, y1, x2, y2);
	//line 4 x1,y2 - x2, y2
	DrawLine(x1,y2,x2,y2);


}

void DrawFilledBox(unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2){

	//distance between each y
	//int dy = y2-y1;
	
	int y;
	for(y = y1; y <= y2; y++){
		
		DrawLine(x1,y,x2,y);
	
	}
}

int GetPixelValue(unsigned char x, unsigned char y){

	int row = y/8;
	
	// Calculate the pixel 'subrow', within that LCD row
	int subRow = y%8;
	
	// Set that particular pixel in our screen buffer
	//if value is set to high
	
	return (screenBuffer[(84*row)+x] & (1 << subRow));

}