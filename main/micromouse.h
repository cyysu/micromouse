#ifndef MICROMOUSE_H
#define MICROMOUSE_H

//declaring global variables
extern volatile int sensorVal[8];          //global variable storing sensor values
extern volatile int encoder0;              //golbal variable storing encoder values
extern volatile int encoder5;
extern volatile int encoder6;
extern volatile int encoder7;

//function prototypes for adc
int conv_int(int input);
void adc_init();

//function prototypes for motor
void onlyRight();
void onlyLeft();
void moveRight();                          // do not use
void moveLeft();                           // do not use
void forward();                            // 40% duty
void stop();                               // 0% duty
void easeStop();                           // do not use! has _delay_cycles()
void slow();                               // 25% duty
void fast();                               // do not use
void torque();                             // full speed, only use to start motors
void invertHigh();                         // set invert high
void invertLow();                          // set invert low
void left90();                             // turn profile, test me
void right90();                            // turn profile, test me
void left45();                             // turn profile, not written yet
void right45();                            // turn profile, not written yet
void clockInit();                          // do not use
void timerA0Init();                        // do not use
void motorInit();                          // USE ME TO INIT!!!
void backUpOneNode();                      // I MOVE BACK ONE NODE;

//function prototypes for encoder
void encoderInit();
void encoderReset(int encoderBit);         // ENC0, ENC5, ENC6, ENC7 

//function prototypes for floodfill
void addDist(int dist);
void getWalls();
void updateCosts();
void getNextDirection();
void move(); 
char inCenter(int x, int y);

#endif
