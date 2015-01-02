#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "pong.h"
#include "Graphics.h"
#include "LCD.h"
#include "ADC.h"
#include "usb_serial.h"

volatile int ballCount;
volatile int paddleOneCount = 0;
//volatile int paddleTwoCount = 0;

//paddle One 
int paddleOneX1;
int paddleOneY1;
int paddleOneX2;
int paddleOneY2;

int paddleOneMovement;

//paddle Two
int paddleTwoX1;
int paddleTwoX2;
int paddleTwoY1;
int paddleTwoY2;

//ball location
int ballX1;
int ballX2;
int ballY1;
int ballY2;

int nextBallPositionX1;
int nextBallPositionX2;
int nextBallPositionY1;
int nextBallPositionY2;

//Ball direction, x can be -1 or 1, y can be 1, 0 or -1
int xDirection;
int yDirection;

//Speed tracking
int ballAngleSpeed;
double adjustableSpeed;

//CONSTANTS
int MAX_ADC_VALUE = 1023;
int TOPWALL = 0;
int BOTTOMWALL = 47;
int LEFTWALL = 0;
int RIGHTWALL = 83;
int MAX_SPEED = 12;

int PADDLE_SPEED = 3;

//Angle Direction - maximum movement
int movementInY;
int movementInX;
int currentMovementInX;
int currentMovementInY;

//range of 1-9 different possible angles
int currentAngle;

int collision;

int gameCondition;

int currentGame;

//turn on timer 1
void initTimer(){
	
	cli();
	//enable CTC - configure timer1 - timer 1 is a 16 bit counter?
	//wgm12 =1
	TCCR1A =(1<<WGM12);
	//need to set cs02, cs01, cs00 valaues
	//for 8 0 1 0
	TCCR1B = (1<<CS11);
	OCR1A = 0x4E1F;
	TCNT1 = 0xB1E0;
	TIMSK1 = (1<<TOIE1);
	sei();
	
}

void initPositions(){
	//paddle One 
	paddleOneX1 = 0;
	paddleOneY1 = 16;
	paddleOneX2 = 1;
	paddleOneY2 = 30;

	paddleOneMovement = 0;

	//paddle Two
	paddleTwoX1 = 82;
	paddleTwoX2 = 83;
	paddleTwoY1 = 16;
	paddleTwoY2 = 30;

	//ball location
	ballX1 = 40;
	ballX2 = 42;
	ballY1 = 22;
	ballY2 = 24;

	currentAngle = 0;

	nextBallPositionX1 = ballX1 - 1;
	nextBallPositionX2 = ballX2 - 1;
	nextBallPositionY1 = ballY1 - 0;
	nextBallPositionY2 = ballY2 - 0;

	//Speed tracking
	ballAngleSpeed = 0;
	adjustableSpeed = 0;

	collision = 0;
}

void initBallDirection(){

	movementInY = 0;
	movementInX = 1;
	currentMovementInX = 0;
	currentMovementInY = 0;
	//Ball direction, x can be -1 or 1, y can be 1, 0 or -1
	xDirection = -1;
	yDirection = 0;
}

void PlayGame(int gameChoice){
	
	currentGame = gameChoice();
	
	if(gameChoice == 1){
	
		Pong();	
		//cli();
		ClearBuffer();
		PongWinMessage();
	
	} else if (gameChoice == 2){
	
		BreakOut();
		
	}


	GameOverScreen();
	
}

void Pong(){

	paddleOneCount = 0;
	//paddleTwoCount = 0;
	ballCount = 0;
	gameCondition = 1;
	unsigned char mux = 0;//pf1
	unsigned char interrupt = 1;

	initPositions();
	initBallDirection();
	ADCInitialise(mux, interrupt);
	UpdateGame();
	//initTimer();

	int speedReduce = 0;

	while(gameCondition != 0){

		//keep reading in ADC values - basically updates the speed adjuster
		speedReduce = MAX_SPEED - ballAngleSpeed;
		adjustableSpeed = AverageADC();
		speedReduce = speedReduce - (speedReduce*adjustableSpeed); 
		
		//need to add in a speed adjuster for the CPU
		
		DetectCollision();

		//keep track of paddle movements - button presses
		if(!(PIND & (1 << PD1))){

			paddleOneMovement = -1;
			UpdatePaddleOne();
			
		} else if(!(PIND & (1 << PD2))){

			paddleOneMovement = 1;
			UpdatePaddleOne();
		}
        
        //prob isnt needed                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
		if(!(PIND & (1 << PD1)) && !(PIND & (1 << PD2))){
			winner = "Player 1";
			gameCondition = 0;

		}

		if(ballCount >= speedReduce){
			
			UpdateBall();
			ballCount = 0;
			//usb_serial_putchar( speedReduce +'0');

		}

		if(paddleOneCount > PADDLE_SPEED){
			
			UpdatePaddleTwo();
			paddleOneCount = 0;
		
		}

		UpdateGame();
		_delay_ms(10);
		ballCount++;
		paddleOneCount++;
	}
}

void GameOverScreen(){
	
	int state = 0;

	while(state != 2) {

		if(!(PIND & (1 << PD1)) && !(PIND & (1 << PD2))){

			state = 1;

		}

		if((state == 1) && ((PIND & (1 << PD1)) && (PIND & (1 << PD2)))){

			state = 2;
		}

		_delay_ms(250);
		PORTF ^= (1<<PF7);
		_delay_ms(250);

	}
}

void PongWinMessage(){
	
	//unsigned char * winner = "";
	LCDClear();
	unsigned char * wins = "Wins";
	LCDPosition((83 - 8*7)/2, 2);
	
	if (collision == 5){
		
		//winner = "Player 1";
		LCDString("Player 1");
	} else if (collision == 6){
	
		//winner = "Player 2";
		LCDString("Player 2");
		
	}
	//LCDString(winner);
	LCDPosition((83 - 4*7)/2, 3);
	LCDString(wins);

}

void UpdateGame(){

	ClearBuffer();

	//draw paddle 1
	DrawFilledBox(paddleOneX1, paddleOneX2, paddleOneY1, paddleOneY2);

	//draw paddle 2
	DrawFilledBox(paddleTwoX1, paddleTwoX2, paddleTwoY1, paddleTwoY2);
	
	//draw ball
	DrawFilledBox(ballX1, ballX2, ballY1, ballY2);

	PresentBuffer();

}

//upates the ball coords
void UpdateBall(){

	if((collision == 1) || (collision == 2)){
		
		DeterminePaddleAngle(collision);
		//currentMovementInX = 0;
		//currentMovementInY = 0;

	}

	//if collides with end condition - do nothing
	if(collision >= 5){

		gameCondition = 0;
	
	} else if (collision == 3){//top wall

		yDirection = 1;
		//currentMovementInX = 0;
		//currentMovementInY = 0;

	} else if (collision == 4){//bottom wall

		yDirection = -1;
		//currentMovementInX = 0;
		//currentMovementInY = 0;

	} 
	
	MoveBall();
	DetectCollision();
}

//updates the padle coords
void UpdatePaddleOne(){

	//update paddle one coords
	if(paddleOneMovement == -1){

		if(paddleOneY1 > TOPWALL){
			paddleOneY1 += paddleOneMovement;
			paddleOneY2 += paddleOneMovement;
		}

	} else if(paddleOneMovement == 1){

		if(paddleOneY2 < BOTTOMWALL){
			paddleOneY1 += paddleOneMovement;
			paddleOneY2 += paddleOneMovement;
		}
	}
	
	paddleOneMovement = 0;
	
}

void UpdatePaddleTwo(){

	//update paddle two coords
	if(ballY1 > (paddleTwoY1 + 7)){

		if(paddleTwoY2 < BOTTOMWALL){

			//move down
			paddleTwoY1 += 1;
			paddleTwoY2 += 1;
		}

	} else if (ballY1 < (paddleTwoY1 + 7)){

		if(paddleTwoY1 > TOPWALL){

			//move up
			paddleTwoY1 += -1;
			paddleTwoY2 += -1;
		}
	}
}

double AverageADC(){
	
	double average = 0;
	int count = 0;
	
	ClearBuffer();
	
	while(count < 5){
		
		average += ADCRead();
		
		count++;
	
	}
	
	average = average/5;
	
	average = (average/1023);
	
	return average;
}

void DetectCollision(){

	collision = 0;

	//For top
	if(ballY1 == TOPWALL){

		collision = 3;

	} else if (ballY2 == BOTTOMWALL){//For Bottom

		collision = 4;
	
	}else if((nextBallPositionX1 <= paddleOneX2) && 
		((nextBallPositionY2 >= paddleOneY1)&&(nextBallPositionY1 <= paddleOneY2))){

		collision = 1;

	} else if ((nextBallPositionX2 >= paddleTwoX1) && 
		((nextBallPositionY2 >= paddleTwoY1)&&(nextBallPositionY1 <= paddleTwoY2))){

		collision = 2;

	} else if ((nextBallPositionX2 >= RIGHTWALL) && 
		((nextBallPositionY2 < paddleTwoY1)||(nextBallPositionY1 > paddleTwoY2))) {
		collision = 5;
		

	} else if ((nextBallPositionX1 <= LEFTWALL) && //For Walls Behind Players
		((nextBallPositionY2 < paddleOneY1)||(nextBallPositionY1 > paddleOneY2))){
		collision = 6;
	}	

}

void DeterminePaddleAngle(int paddle){

	//range between the center where angle is 0
	int paddleCenter1 = 0;
	int paddleCenter2 = 0;

	int distance = 0;

	if(paddle == 1){

		paddleCenter1 = paddleOneY1 + 5;
		paddleCenter2 = paddleOneY1 + 8;
		
		xDirection = 1;

	} else if(paddle == 2) {

		paddleCenter1 = paddleTwoY1 + 5;
		paddleCenter2 = paddleTwoY1 + 8;

		xDirection = -1;

	}

	//determine which side of the paddle and find distance
	if(ballY1 < paddleCenter1){

		//tophalf
		yDirection = -1;
		distance = paddleCenter1 - ballY1;


	} else if (ballY2 > paddleCenter2) {

		//bottomhalf
		yDirection = 1;

		distance = ballY2 - paddleCenter2;

	} else {

		yDirection = 0;
		distance = 0;
	}

	currentAngle = distance;

	switch(distance){

		case 0:
			movementInY = 0;
			movementInX = 1;
			ballAngleSpeed = 0;
			break;
		case 1:
			movementInY = 1;
			movementInX = 4;
			ballAngleSpeed = 1; 
			break;
		case 2:
			movementInY = 1;
			movementInX = 3;
			ballAngleSpeed = 1; 
			break;
		case 3:
			movementInY = 1;
			movementInX = 2;
			ballAngleSpeed = 2; 
			break;
		case 4:
			movementInY = 2;
			movementInX = 3;
			ballAngleSpeed = 2; 
			break;
		case 5: 
			movementInY = 1;
			movementInX = 1;
			ballAngleSpeed = 3;
			break;
		case 6: 
			movementInY = 3;
			movementInX = 2;
			ballAngleSpeed = 3;
			break;
		case 7: 
			movementInY = 2;
			movementInX = 1;
			ballAngleSpeed = 4;
			break;
		case 8:
			movementInY = 3;
			movementInX = 1;
			ballAngleSpeed = 4; 
			break;
		case 9:
			movementInY = 4;
			movementInX = 1;
			ballAngleSpeed = 5; 
			break;
		case 10:
			movementInY = 4;
			movementInX = 1;
			ballAngleSpeed = 5; 
			break;

	}

}

//moves the ball to next position and determines next position
void MoveBall(){

	if(currentAngle == 0){

		ballX1 += (1 * xDirection);
		ballX2 += (1 * xDirection);

		//only need to update x positions
		nextBallPositionX1 = ballX1 + (1 * xDirection);
		nextBallPositionX2 = ballX2 + (1 * xDirection);


	} else if (currentAngle == 5){

		//need to update both x and y at the same time
		ballX1 += (1 * xDirection);
		ballX2 += (1 * xDirection);
		ballY1 += (1 * yDirection);
		ballY2 += (1 * yDirection);

		nextBallPositionX1 = ballX1 + (1 * xDirection);
		nextBallPositionX2 = ballX2 + (1 * xDirection);
		nextBallPositionY1 = ballY1 + (1 * yDirection);
		nextBallPositionY2 = ballX2 + (1 * yDirection);

	} else if(currentAngle < 4){

		ballX1 += (1 * xDirection);
		ballX2 += (1 * xDirection);
		nextBallPositionX1 = ballX1 + (1 * xDirection);
		nextBallPositionX2 = ballX2 + (1 * xDirection);

		currentMovementInX++;

		if(currentMovementInX == movementInX){

			//shift it up one place
			ballY1 += (1 * yDirection);
			ballY2 += (1 * yDirection);
			nextBallPositionY1 = ballY1 + (1 * yDirection);
			nextBallPositionY2 = ballY2 + (1 * yDirection);

			currentMovementInX = 0;
		}


	} else if(currentAngle == 4){

		ballX1 += (1 * xDirection);
		ballX2 += (1 * xDirection);
		nextBallPositionX1 = ballX1 + (1 * xDirection);
		nextBallPositionX2 = ballX2 + (1 * xDirection);
		
		currentMovementInX++;

		if(currentMovementInX%2 == 1){

			//shift up one
			ballY1 += (1 * yDirection);
			ballY2 += (1 * yDirection);
			nextBallPositionY1 = ballY1 + (1 * yDirection);
			nextBallPositionY2 = ballY2 + (1 * yDirection);

			currentMovementInX = 0;

		}


	} else if(currentAngle > 6){

		ballY1 += (1 * yDirection);
		ballY2 += (1 * yDirection);
		nextBallPositionY1 = ballY1 + (1 * yDirection);
		nextBallPositionY2 = ballY2 + (1 * yDirection);

		currentMovementInY++;

		if(currentMovementInY == movementInY){

			//shift it across one place
			ballX1 += (1 * xDirection);
			ballX2 += (1 * xDirection);
			nextBallPositionX1 = ballX1 + (1 * xDirection);
			nextBallPositionX2 = ballX2 + (1 * xDirection);

			currentMovementInY = 0;

		}


	} else if(currentAngle == 6){

		ballY1 += (1 * yDirection);
		ballY2 += (1 * yDirection);
		nextBallPositionY1 = ballY1 + (1 * yDirection);
		nextBallPositionY2 = ballY2 + (1 * yDirection);

		currentMovementInY++;

		if(currentMovementInY%2 == 1){

			//shift across one
			ballX1 += (1 * xDirection);
			ballX2 += (1 * xDirection);
			nextBallPositionX1 = ballX1 + (1 * xDirection);
			nextBallPositionX2 = ballX2 + (1 * xDirection);

			currentMovementInY = 0;

		}
	}
}

ISR(TIMER1_OVF_vect){

	ballCount++;
	paddleOneCount++;
	//paddleTwoCount++;

}

