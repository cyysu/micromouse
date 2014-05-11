#include <msp430.h>
#include "micromouse.h"

#define min_dist(i,j) min(min(abs(i-center)+abs(j-center),abs(i-center+1)+abs(j-center)),min(abs(i-center)+abs(j-center+1),abs(i-center+1)+abs(j-center+1)));
#define STEP_LENGTH 110
#define LEFT_SENSOR sensorVal[1]
#define RIGHT_SENSOR sensorVal[4]
#define BACK_SENSOR sensorVal[6]
#define FRONT_SENSOR sensorVal[0]
#define SENSOR_HIGH 800                                                /*determine this value*/
#define MAPSIZE 16

extern volatile int encoder0, encoder5, encoder6, encoder7;

int abs(int input) {
  if (input < 0) {
    input = -input;
  }
  return input;
}

struct Node {
  char walls;
  char cost;
};

int min(int first, int second) {
  if(first > second) {
    return second;
  }
  return first;
}

//Search
char TO_CENTER = 1;
char TO_START = 0;

//Map
char center = 8;
struct Node maze[MAPSIZE][MAPSIZE];

// Distance
// Ignore times due to having encoder.
// Encoder can just give the amount of time running.
int tempDist=0; // delta distance
int botDistX=0, botDistY=300; // may use struct
int curNodeX=0, curNodeY=0;

// Directions
enum Direction {
  NORTH = 0, EAST, SOUTH, WEST};
// Great for easy calculations of cost.
enum Direction next_dir = NORTH;
enum Direction past_dir = NORTH;
// Constants
const int tileSize = 18; // change to 18 for centimeters

int main() {
  adc_init();
  motorInit();
  encoderInit();

  int oldDist;

  for(int i=0; i<MAPSIZE; i++) {
    for(int j=0; j<MAPSIZE; j++) {
      // reference all the costs with respect to center.
      maze[i][j].cost = min_dist(i,j);
      maze[i][j].walls = 0;
    }
  }
  for(;;) {
    // Calculate Distance and determine current square      /*change to encoders*/
    tempDist = encoder0 - oldDist;                          /*average of 2 wheels encoders*/
    oldDist = encoder0;
    // addDist may be needed
    addDist(tempDist);
    curNodeX = botDistX / STEP_LENGTH;
    curNodeY = botDistY / STEP_LENGTH;

    // Get wall information and update node costs
    getWalls(maze[curNodeX][curNodeY]);
    updateCosts();
    getNextDirection();
    move();

    //Check for maze completion
    if(TO_CENTER && inCenter(curNodeX,curNodeY)) {
      TO_START = 1;
      TO_CENTER = 0;
      for(int i=0; i<MAPSIZE; i++) {
        for(int j=0; j<MAPSIZE; j++){
          // only need absolute distance from the center
          maze[i][j].cost = abs(i+j);
        }
      }
      updateCosts();
    }
    //Check for return trip
    else if(TO_START && curNodeX == 0 && curNodeY == 0) {
      TO_START = 0;
      TO_CENTER = 1;
      for(int i=0; i < MAPSIZE; i++) {
        for(int j=0; j < MAPSIZE; j++) {
          maze[i][j].cost = min_dist(i,j);
        }
      }
      updateCosts();
    }
  }
  return 0;
}

//Add distance traveled since last loop iteration based on current direction
void addDist(int dist) {
  if(next_dir==NORTH) {
    botDistY -= dist;
  }
  else if(next_dir==SOUTH) {
    botDistY += dist;
  }
  else if(next_dir==EAST) {
    botDistX += dist;
  }
  else
    botDistX -= dist;
}

void getWalls(struct Node node) {                                 /*use sensors here*/
  char temp = 0x0;
  if(FRONT_SENSOR >= SENSOR_HIGH) {
    temp |= BIT0;
  }
  if(RIGHT_SENSOR >= SENSOR_HIGH) {
    temp |= BIT1;
  }
  if(BACK_SENSOR >= SENSOR_HIGH) {
    temp |= BIT2;
  }
  if(LEFT_SENSOR >= SENSOR_HIGH) {
    temp |= BIT3;
  }
  node.walls = (temp >> next_dir) + (temp << (4 - next_dir));
}

//Iterate through each square until all squares have at least one square of lower value adjacent to them.
//This is accomplished by comparing the cost of each adjacent neighbor determined by the surrounding walls.
//Once the minimum value of the neighbors is taken, it's compared to the current square's cost.
void updateCosts() {
  // Serial.println("Entering updateCosts()");
  char check = 1;
  while(check) {
    check = 0;
    for(int i=0; i<MAPSIZE; i++) {
      for(int j=0; j<MAPSIZE; j++) {
        // Initialize to impossible value.
        int neighbors[4] = {-1,-1,-1,-1};
        // Initialize to impossible value. The cost can only be at max as
        // tempCost - 1.
        int tempCost = MAPSIZE*MAPSIZE;
        char walls = maze[i][j].walls;
        if(!(walls && NORTH) && i-1 >= 0){
          neighbors[NORTH] = maze[i-1][j].cost;
        }
        if(!(walls && EAST) && j+1 < MAPSIZE) {
          neighbors[EAST] = maze[i][j+1].cost;
        }
        if(!(walls && SOUTH) && i+1 < MAPSIZE) {
          neighbors[SOUTH] = maze[i+1][j].cost;
        }
        if(!(walls && WEST) && j-1 >= 0) {
          neighbors[WEST] = maze[i][j-1].cost;
        }
        for(int k=0; k<4; k++) {
	  // use define for impossible value.
          if(neighbors[k] != -1 && neighbors[k] < tempCost) {
            tempCost = neighbors[k];
          }
        }
        // If you're going to the center.
        if(TO_CENTER) {
          if(tempCost >= maze[i][j].cost && !inCenter(i,j) && tempCost != MAPSIZE*MAPSIZE) {
            // Serial.println("Changing Node Cost");
            maze[i][j].cost = tempCost + 1;
            // To escape the function.
            check = 1;
          }
        }
        // If you're heading back to the start.
        else if(TO_START) {
          if(tempCost >= maze[i][j].cost && !(i == 0 && j == 0) && tempCost != MAPSIZE*MAPSIZE) {
            maze[i][j].cost = tempCost + 1;
            // To escape the function.
            check = 1;
          }
        }
      }
    }
  }
}

// The lowest cost square is determined by using wall data for the current square.
void getNextDirection() {
  char walls = maze[curNodeY][curNodeX].walls;
  // set it to MAPSIZE * map size
  int tempCost = 1000;
  past_dir = next_dir;
  if(!(walls && 0) && curNodeY > 0) {
    next_dir = NORTH;
    tempCost = maze[curNodeY-1][curNodeX].cost;
  }
  if(!(walls && 1) && maze[curNodeY][curNodeX+1].cost < tempCost && curNodeX < MAPSIZE-1) {
    next_dir = EAST;
    tempCost = maze[curNodeY][curNodeX+1].cost;
  }
  if(!(walls && 2) && maze[curNodeY+1][curNodeX].cost < tempCost && curNodeY < MAPSIZE-1) {
    next_dir = SOUTH;
    tempCost = maze[curNodeY+1][curNodeX].cost;
  }
  if(!(walls && 3) && maze[curNodeY][curNodeX-1].cost < tempCost && curNodeX > 0) {
    next_dir = WEST;
    tempCost = maze[curNodeY][curNodeX-1].cost;
  }
}

//once a direction is given, need to move robot in real life
void move() {
  if(next_dir == past_dir) {
    forward();
  } else if(next_dir == (past_dir + 1) || next_dir == (past_dir - 3)) {
    backUpOneNode();
    right90();
  } else if(next_dir == (past_dir + 2) || next_dir == (past_dir - 2)) {
    backUpOneNode();
  } else {
    backUpOneNode();
    left90();
  }

}

// Checks to see if current square is at the center of the maze.
char inCenter(int x, int y) {
  if((x == center && y == center) || (x == center-1 && y == center)
     || (x == center && y == center-1) || (x == center-1 && y == center-1)) {
    return 1;
  }
  return 0;
}

//interrupts for motor pwm
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ccr0_clear_pulse (void) {
  P1OUT &= ~(BIT2 + BIT1);      // turn off p1.1 and p1.2
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void ccr1_2_set_pulse (void) {
  switch(TAIV) {
  case 2: P1OUT |= BIT1;        // turn on p1.1 (left motor)
    break;
  case 4: P1OUT |= BIT2;        // turn on p1.2 (right motor)
    break;
  }
}

//interrupt for the encoder
#pragma vector=PORT1_VECTOR
__interrupt void falling_edge_capture() {  // captures falling edge from encoder wave
  if(P1IFG & BIT0) {    // if button interrupt is on BIT0,
    encoder0++;        // increment encoder0 count
    P1IFG &= ~BIT0;    // clear interrupt flag for BIT0
  } else if(P1IFG & BIT5) {
    encoder5++;
    P1IFG &= ~BIT5;
  } else if(P1IFG & BIT6) {
    encoder6++;
    P1IFG &= ~BIT6;
  } else if(P1IFG & BIT7) {
    encoder7++;
    P1IFG &= ~BIT7;
  }
}

//interrupt for turning on LEDs
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR (void) {
   P2OUT = P2OUT << 1;
   if(!P2OUT) {
      P2OUT = 0x01;
   }

   ADC10CTL0 |= ADC10SC;
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
   sensorVal[conv_int(P2OUT)] = ADC10MEM;
   ADC10CTL0 &= ~ADC10IFG;
}
