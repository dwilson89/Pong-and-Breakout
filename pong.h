/*
pong.h

functions for operating pong


*/
#ifndef PONG_H_
#define PONG_H_

void initTimer();
void initPositions();
void initBallDirection();
void Pong();
void GameOverScreen();
void UpdateGame();
void UpdateBall();
void UpdatePaddleOne();
void UpdatePaddleTwo();
double AverageADC();
void DetectCollision();
void PongCollisions();
void BreakOutCollisions();
void DeterminePaddleAngle();
void MoveBall();
void WinMessage();
void BreakOut();
void UpdateBreakOut();
void InitBreakOutPositions();
void UpdatePong();
void PlayGame(int gameChoice);
void DestroyBrick();

#endif/* PONG_H_ */