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

//in the form of x1, y1, x2, y2
int paddleOne[4];
int paddleTwo[4];
int ballPos[4];
int nextBallPos[4];

int paddleOneMovement;


//Ball direction, x can be -1 or 1, y can be 1, 0 or -1

int ballDirection[2];

//int xDirection;
//int yDirection;

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
int movementIn[2];
int currentMovementIn[2];

//range of 1-9 different possible angles
int currentAngle;

int collision;

int gameCondition;

int currentGame;

int gameWinner;

int brickCount;

//breakout CONSTANTS and vars
const int BRICK_LENGTH = 10;
const int BRICK_HEIGHT = 4;
const int BW_GAP = 2;
const int BH_GAP = 1;

const int INITIAL_NO_BRICKS = 21;
const int ROW_COUNT = 7;
const int BRICK_SP = 1;

int bricks[21];

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
	
	paddleOne[0] = 0;
	paddleOne[1] = 16;
	paddleOne[2] = 1;
	paddleOne[3] = 30;

	paddleOneMovement = 0;
	
	paddleTwo[0] = 82;
	paddleTwo[1] = 16;
	paddleTwo[2] = 83;
	paddleTwo[3] = 30;
	
	ballPos[0] = 40;
	ballPos[1] = 22;
	ballPos[2] = 42;
	ballPos[3] = 24;
	
	currentAngle = 0;
	
	nextBallPos[0] = ballPos[0] - 1;
	nextBallPos[1] = ballPos[1] - 0;
	nextBallPos[2] = ballPos[2] - 1;
	nextBallPos[3] = ballPos[3] - 0;

	//Speed tracking
	ballAngleSpeed = 0;
	adjustableSpeed = 0;

	collision = 0;
}

void initBallDirection(){
	
	movementIn[0] = 1;
	movementIn[1] = 0;
	currentMovementIn[0] = 0;
	currentMovementIn[1] = 0;

	
	ballDirection[0] = -1;
	ballDirection[1] = 0;
}

void PlayGame(int gameChoice){
	
	currentGame = gameChoice;
	gameCondition = 1;
	unsigned char mux = 0;//pf1
	unsigned char interrupt = 1;
	ADCInitialise(mux, interrupt);
	ballCount = 0;
	
	if(currentGame == 1){
		
		paddleOneCount = 0;
		initPositions();
		initBallDirection();
	
	} else if (currentGame == 2){
		
		BreakOut();
	
	}

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
			gameWinner = 1;
			gameCondition = 0;

		}

		if(ballCount >= speedReduce){
			
			UpdateBall();
			ballCount = 0;
			//usb_serial_putchar( speedReduce +'0');

		}
		
		if(currentGame == 1){
		
			if(paddleOneCount > PADDLE_SPEED){
			
				UpdatePaddleTwo();
				paddleOneCount = 0;
		
			}
			paddleOneCount++;
		
		}

		if(currentGame == 2){
			
			if(brickCount == INITIAL_NO_BRICKS){
				
				gameCondition = 0;
				
				gameWinner = 1;
			}
		}
		
		UpdateGame();
		_delay_ms(10);
		ballCount++;
	}
	
	//cli();
	ClearBuffer();
	WinMessage();
	GameOverScreen();
}

void GameOverScreen(){
	
	int state = 0;

	while(state != 2) {
		
		_delay_ms(250);
		
		if(!(PIND & (1 << PD1)) && !(PIND & (1 << PD2))){

			state = 1;

		}

		if((state == 1) && ((PIND & (1 << PD1)) && (PIND & (1 << PD2)))){

			state = 2;
		}

		
		PORTF ^= (1<<PF7);
		_delay_ms(250);

	}
	
	PORTF &= ~(1<<PF7);
	
}

void WinMessage(){
	
	//unsigned char * winner = "";
	LCDClear();
	unsigned char * wins = "Wins";
	LCDPosition((83 - 8*7)/2, 2);
	
	if (gameWinner == 1){
		
		unsigned char * winner = "Player 1";
		LCDString(winner);
	} else if (gameWinner == 2){
	
		unsigned char *winner = "Player 2";
		LCDString(winner);
		
	}
	//LCDString(winner);
	LCDPosition((83 - 4*7)/2, 3);
	LCDString(wins);

}

void UpdateGame(){
	
	if(currentGame == 1){
	
		UpdatePong();
	
	} else {
	
		UpdateBreakOut();
	
	}
}

void UpdatePong(){

	ClearBuffer();

	//draw paddle 1
	DrawFilledBox(paddleOne[0], paddleOne[2], paddleOne[1], paddleOne[3]);

	//draw paddle 2
	DrawFilledBox(paddleTwo[0], paddleTwo[2], paddleTwo[1], paddleTwo[3]);
	
	//draw ball
	DrawFilledBox(ballPos[0], ballPos[2], ballPos[1], ballPos[3]);

	PresentBuffer();

}

//updates the ball coords
void UpdateBall(){

	if((collision == 1) || (collision == 2)){
		
		DeterminePaddleAngle(collision);
	}

	//if collides with end condition - do nothing
	if(collision == 5){

		gameCondition = 0;
	
	} else if (collision == 3){//top wall

		ballDirection[1] = 1;

	} else if (collision == 4){//bottom wall

		ballDirection[1] = -1;

	} else if(collision == 6){//right wall - breakout
		
		ballDirection[0] = -1;
	
	} else if (collision == 7){//left wall - breakout
	
		ballDirection[0] = 1;
		
	} else if (collision == 8){//brick
		//destroy brick function that recalls this function with collision type
		//BrickBounce();
	}
	
	MoveBall();
	DetectCollision();
}

//updates the padle coords
void UpdatePaddleOne(){
	
	int boundaryCaseOne = TOPWALL;
	int boundaryCaseTwo = BOTTOMWALL;
	
	//keeps track of the indexes for either x or y depending on game
	int coordOne = 1;
	int coordTwo = 3;
	
	//breakout
	if(currentGame == 2){
		boundaryCaseOne = LEFTWALL;
		boundaryCaseTwo = RIGHTWALL;
		coordOne = 0;//x1
		coordTwo = 2;//x2
		
		//need to invert movement
		paddleOneMovement = paddleOneMovement * -1;
	}
	
	//update paddle one coords
	if(paddleOneMovement == -1){

		if(paddleOne[coordOne] > boundaryCaseOne){
			paddleOne[coordOne] += paddleOneMovement;
			paddleOne[coordTwo] += paddleOneMovement;
		}

	} else if(paddleOneMovement == 1){

		if(paddleOne[coordTwo] < boundaryCaseTwo){
			paddleOne[coordOne] += paddleOneMovement;
			paddleOne[coordTwo] += paddleOneMovement;
		}
	}
	
	paddleOneMovement = 0;
	
}

void UpdatePaddleTwo(){

	//update paddle two coords
	if(ballPos[1] > (paddleTwo[1] + 7)){

		if(paddleTwo[3] < BOTTOMWALL){

			//move down
			paddleTwo[1] += 1;
			paddleTwo[3] += 1;
		}

	} else if (ballPos[1] < (paddleTwo[1] + 7)){

		if(paddleTwo[1] > TOPWALL){

			//move up
			paddleTwo[1] += -1;
			paddleTwo[3] += -1;
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

void BreakOutCollisions(){
	
	//how many pixels the ball hits
	//int collisionCount = 0;
	
	int row = 0, column = 0;
	
	//only interested when ball is greater than paddle location
	if(nextBallPos[3] < paddleOne[1]){
		
		//check the column
		if(((ballPos[0]+1)%12) != 0){
			
			
			column =(ballPos[0]+1)/12 + 1;
		
		} else {
	
			column =(ballPos[0]+1)/12;
		
		}
		
		//might need to fix row and column +1 to make sure numbers are whole
		
		row = ballPos[1]/5;
		
		if(row < 3){
		
			//check bricks
			if(bricks[column + row*7 - 1] == 1){
				
				bricks[column + row*7 - 1] = 0;
				brickCount++;
				
				collision =	BrickBounce(row, column);
			}
		}
	}
	
	//rightwall
	if(ballPos[2] >= RIGHTWALL){
		
		collision = 6;
	
	} else if (ballPos[0] <= LEFTWALL){
	
		collision = 7;
		
	} else if(ballPos[1] <= TOPWALL){

		collision = 3;
	
	//paddle collision
	} else if((nextBallPos[3] >= paddleOne[1]) && 
		((nextBallPos[2] >= paddleOne[0])&&(nextBallPos[0] <= paddleOne[2]))){

		collision = 1;

	} else if ((nextBallPos[3] >= BOTTOMWALL) && //For Wall Behind Player
		((nextBallPos[2] < paddleOne[0])||(nextBallPos[0] > paddleOne[2]))){
		collision = 5;
		gameWinner = 2;
	}
	
}


void DetectCollision(){

	collision = 0;
	
	if(currentGame == 2){
		
		BreakOutCollisions();
		
	} else {
	
		PongCollisions();
	
	}
}

void PongCollisions(){

//For top
	if(ballPos[1] == TOPWALL){

		collision = 3;

	} else if (ballPos[3] == BOTTOMWALL){//For Bottom

		collision = 4;
	
	}else if((nextBallPos[0] <= paddleOne[2]) && 
		((nextBallPos[3] >= paddleOne[1])&&(nextBallPos[1] <= paddleOne[3]))){

		collision = 1;

	} else if ((nextBallPos[2] >= paddleTwo[0]) && 
		((nextBallPos[3] >= paddleTwo[1])&&(nextBallPos[1] <= paddleTwo[3]))){

		collision = 2;

	} else if ((nextBallPos[2] >= RIGHTWALL) && 
		((nextBallPos[3] < paddleTwo[1])||(nextBallPos[1] > paddleTwo[3]))) {
		collision = 5;
		gameWinner = 1;

	} else if ((nextBallPos[0] <= LEFTWALL) && //For Walls Behind Players
		((nextBallPos[3] < paddleOne[1])||(nextBallPos[1] > paddleOne[3]))){
		collision = 5;
		gameWinner = 2;
	}	

}

void DeterminePaddleAngle(int paddle){
	
	currentMovementIn[1] = 0;
	currentMovementIn[0] = 0;
	
	//range between the center where angle is 0
	int paddleCenter1 = 0;
	int paddleCenter2 = 0;
	
	int coordOne;
	int coordTwo;
	
	int directionOne;
	int directionTwo;
	
		//pong
	if(currentGame == 1){
		//paddle
		coordOne = 1;//y1
		coordTwo = 3;
		
		//ball
		//coordBall = 1;//y1
		
		directionOne = 0;//x dir
		directionTwo = 1;//y dir
		
	//breakout
	} else if(currentGame == 2){
		//paddle
		coordOne = 0;//x1
		coordTwo = 2;
		//ball
		//coordBall = 0;//x2
		
		directionOne = 1;//y dir
		directionTwo = 0;//x dir

	}
	int distance = 0;

	if(paddle == 1){

		paddleCenter1 = paddleOne[coordOne] + 5;
		paddleCenter2 = paddleOne[coordOne] + 8;
		
		ballDirection[directionOne] = 1;
		
		if(currentGame == 2){
			
			ballDirection[1] = -1;
		
		}
		
	} else if(paddle == 2) {

		paddleCenter1 = paddleTwo[coordOne] + 5;
		paddleCenter2 = paddleTwo[coordOne] + 8;

		ballDirection[directionOne] = -1;

	}

	//determine which side of the paddle and find distance
	if(ballPos[coordOne] < paddleCenter1){

		//tophalf
		ballDirection[directionTwo] = -1;
		
		distance = paddleCenter1 - ballPos[coordOne];


	} else if (ballPos[coordTwo] > paddleCenter2) {

		//bottomhalf
		ballDirection[directionTwo] = 1;

		distance = ballPos[coordTwo] - paddleCenter2;

	} else {

		ballDirection[directionTwo] = 0;
		distance = 0;
	}
	
	currentAngle = distance;

	switch(distance){

		case 0:
			movementIn[1] = 0;
			movementIn[0] = 1;
			ballAngleSpeed = 0;
			break;
		case 1:
			movementIn[1] = 1;
			movementIn[0] = 4;
			ballAngleSpeed = 1; 
			break;
		case 2:
			movementIn[1] = 1;
			movementIn[0] = 3;
			ballAngleSpeed = 1; 
			break;
		case 3:
			movementIn[1] = 1;
			movementIn[0] = 2;
			ballAngleSpeed = 2; 
			break;
		case 4:
			movementIn[1] = 2;
			movementIn[0] = 3;
			ballAngleSpeed = 2; 
			break;
		case 5: 
			movementIn[1] = 1;
			movementIn[0] = 1;
			ballAngleSpeed = 3;
			break;
		case 6: 
			movementIn[1] = 3;
			movementIn[0] = 2;
			ballAngleSpeed = 3;
			break;
		case 7: 
			movementIn[1] = 2;
			movementIn[0] = 1;
			ballAngleSpeed = 4;
			break;
		case 8:
			movementIn[1] = 3;
			movementIn[0] = 1;
			ballAngleSpeed = 4; 
			break;
		case 9:
			movementIn[1] = 4;
			movementIn[0] = 1;
			ballAngleSpeed = 5; 
			break;
		case 10:
			movementIn[1] = 4;
			movementIn[0] = 1;
			ballAngleSpeed = 5; 
			break;

	}
	
	int temp = movementIn[0];
	
	//need to invert the angles
	if(currentGame == 2){
		
		//handle special cases
		if(currentAngle == 0){
			
			currentAngle = currentAngle;
			
		} if (currentAngle == 10){
			
			currentAngle = 1;
			movementIn[0] = 1;
			movementIn[1] = 4;
		
		} else {
			movementIn[0] = movementIn[1];
			movementIn[1] = temp;
			
			currentAngle = 10 - currentAngle;
			//usb_serial_putchar(currentAngle + '0');
			//usb_serial_putchar(movementIn[0] + '0');
			//usb_serial_putchar(movementIn[1] + '0');
		}
	}
}

//moves the ball to next position and determines next position
void MoveBall(){
	
	if(currentAngle == 0){
	
		//will need to update both directions
		
		//x change
		ballPos[0] += (1 * ballDirection[0]);
		ballPos[2] += (1 * ballDirection[0]);
		
		//y change
		ballPos[1] += (1 * ballDirection[1]);
		ballPos[3] += (1 * ballDirection[1]);
		
		nextBallPos[0] = ballPos[0] + (1 * ballDirection[0]);
		nextBallPos[2] = ballPos[2] + (1 * ballDirection[0]);
		nextBallPos[1] = ballPos[1] + (1 * ballDirection[1]);
		nextBallPos[3] = ballPos[3] + (1 * ballDirection[1]);

	} else if (currentAngle == 5){

		//need to update both x and y at the same time
		//x change
		ballPos[0] += (1 * ballDirection[0]);
		ballPos[2] += (1 * ballDirection[0]);
		//y change
		ballPos[1] += (1 * ballDirection[1]);
		ballPos[3] += (1 * ballDirection[1]);
		
		nextBallPos[0] = ballPos[0] + (1 * ballDirection[0]);
		nextBallPos[2] = ballPos[2] + (1 * ballDirection[0]);
		nextBallPos[1] = ballPos[1] + (1 * ballDirection[1]);
		nextBallPos[3] = ballPos[3] + (1 * ballDirection[1]);

	} else if(currentAngle < 4){
		
		ballPos[0] += (1 * ballDirection[0]);
		ballPos[2] += (1 * ballDirection[0]);
		
		nextBallPos[0] = ballPos[0] + (1 * ballDirection[0]);
		nextBallPos[2] = ballPos[2] + (1 * ballDirection[0]);

		currentMovementIn[0]++;

		if(currentMovementIn[0] == movementIn[0]){

			//shift it up one place
			ballPos[1] += (1 * ballDirection[1]);
			ballPos[3] += (1 * ballDirection[1]);
			nextBallPos[1] = ballPos[1] + (1 * ballDirection[1]);
			nextBallPos[3] = ballPos[3] + (1 * ballDirection[1]);

			currentMovementIn[0] = 0;
		}


	} else if(currentAngle == 4){

		ballPos[0] += (1 * ballDirection[0]);
		ballPos[2] += (1 * ballDirection[0]);
		
		nextBallPos[0] = ballPos[0] + (1 * ballDirection[0]);
		nextBallPos[2] = ballPos[2] + (1 * ballDirection[0]);

		currentMovementIn[0]++;

		if(currentMovementIn[0]%2 == 1){

			//shift up one
			ballPos[1] += (1 * ballDirection[1]);
			ballPos[3] += (1 * ballDirection[1]);
			nextBallPos[1] = ballPos[1] + (1 * ballDirection[1]);
			nextBallPos[3] = ballPos[3] + (1 * ballDirection[1]);

			currentMovementIn[0] = 0;

		}

	} else if(currentAngle > 6){
	
		ballPos[1] += (1 * ballDirection[1]);
		ballPos[3] += (1 * ballDirection[1]);
		nextBallPos[1] = ballPos[1] + (1 * ballDirection[1]);
		nextBallPos[3] = ballPos[3] + (1 * ballDirection[1]);

		currentMovementIn[1]++;

		if(currentMovementIn[1] == movementIn[1]){

			//shift it across one place
			ballPos[0] += (1 * ballDirection[0]);
			ballPos[2] += (1 * ballDirection[0]);
		
			nextBallPos[0] = ballPos[0] + (1 * ballDirection[0]);
			nextBallPos[2] = ballPos[2] + (1 * ballDirection[0]);

			currentMovementIn[1] = 0;

		}


	} else if(currentAngle == 6){

		ballPos[1] += (1 * ballDirection[1]);
		ballPos[3] += (1 * ballDirection[1]);
		nextBallPos[1] = ballPos[1] + (1 * ballDirection[1]);
		nextBallPos[3] = ballPos[3] + (1 * ballDirection[1]);

		currentMovementIn[1]++;

		if(currentMovementIn[1]%2 == 1){

			//shift across one
			ballPos[0] += (1 * ballDirection[0]);
			ballPos[2] += (1 * ballDirection[0]);
		
			nextBallPos[0] = ballPos[0] + (1 * ballDirection[0]);
			nextBallPos[2] = ballPos[2] + (1 * ballDirection[0]);

			currentMovementIn[1] = 0;

		}
	}
}

void InitBreakOutPositions(){
	
	paddleOne[0] = 34;
	paddleOne[2] = 48;
	paddleOne[1] = 46;
	paddleOne[3] = 47;
	
	ballPos[0] = 40;
	ballPos[2] = 42;
	ballPos[1] = 22;
	ballPos[3] = 24;
	
	nextBallPos[0] = 40;
	nextBallPos[2] = 42;
	nextBallPos[1] = 23;
	nextBallPos[3] = 25;
	
	ballDirection[0] = 0;
	ballDirection[1] = 1;
	
	movementIn[0] = 0;
	movementIn[1] = 1;
	currentMovementIn[0] = 0;
	currentMovementIn[1] = 0;
	
	currentAngle = 0;
		//Speed tracking
	ballAngleSpeed = 0;
	adjustableSpeed = 0;

	collision = 0;
	
	brickCount = 0;
	
	int i;
	for(i=0; i < INITIAL_NO_BRICKS; i++){
		
		bricks[i] = 1;
	
	}
	
}

void UpdateBreakOut(){
	
	ClearBuffer();
	
	//draw paddle
	DrawFilledBox(paddleOne[0], paddleOne[2], paddleOne[1], paddleOne[3]);
	
	//draw ball
	DrawFilledBox(ballPos[0], ballPos[2], ballPos[1], ballPos[3]);
	
	//draw bricks
	int i;
	int x1 = 0;
	int x2 = 0;
	int y1 = 1;
	int y2 = 0;
	while(i < INITIAL_NO_BRICKS){
		
		if(i >= 7){
		
			if((i%ROW_COUNT) == 0){
			
				y1 = y2 + 2;
				x1 = 0;
				x2 = 0;
			}
		}
		
		x1 = x2 + 1;
		x2 = x1 + 9;
		y2 = y1 + 3;
		
		if(bricks[i] == 1){
			
			DrawFilledBox(x1, x2, y1, y2);
		
		}
		
		x2 += 2;
		i++;
		//add in gap
	
	}
	
	//update the screen
	PresentBuffer();

}

void BreakOut(){

	InitBreakOutPositions();
	UpdateBreakOut();

}

void BrickBounce(int row, int column){
	
	int collisionType = 0;
	
	//this just determines the direction in which to send the ball after it hits
	if(nextBallPos[1] > row*5){
		
		collisionType = 3;
	
	} else if(nextBallPos[3] < row*5){
		
		collisionType = 4;
	
	} else if(nextBallPos[0] <= column*12){
		
		collisionType = 7;
	
	} else if(nextBallPos[2] >= column*12) {
	
		collisionType = 6;
	}
	usb_serial_putchar(collisionType + '0');
	
	collision = collisionType;
	UpdateBall();
	return collisionType;
	
}

ISR(TIMER1_OVF_vect){

	ballCount++;
	paddleOneCount++;
	//paddleTwoCount++;

}

