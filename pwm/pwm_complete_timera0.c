#include <msp430.h>

#define TURN_CONST 5000
#define HALF_PERIOD 15000

void setLeftTurn() {
   TA0CCR1 = HALF_PERIOD - TURN_CONST;
   TA0CCR2 = HALF_PERIOD + TURN_CONST;
}
void setRightTurn() {
   TA0CCR1 = HALF_PERIOD + TURN_CONST;
   TA0CCR2 = HALF_PERIOD - TURN_CONST;
}
void setForward() {
   TA0CCR1 = HALF_PERIOD;     // CCR1 = wave 1 off time
   TA0CCR2 = HALF_PERIOD;     // CCR2 = wave 2 off time
}
void setStop() {
   TA0CCR1 = HALF_PERIOD * 2 + 10;  // CCR1 > CCR0, never on
   TA0CCR2 = HALF_PERIOD * 2 + 10;  // CCR2 > CCR0, never on
}
void invertHigh() {
   P1OUT |= BIT3;
}
void invertLow() {
   P1OUT &= ~BIT3;
}
void setSlow() {
   TA0CCR1 = 2*HALF_PERIOD - 1000;
   TA0CCR2 = 2*HALF_PERIOD - 1010;
}

void clockInit() {
   BCSCTL1 = CALBC1_16MHZ;
   DCOCTL = CALDCO_16MHZ;
}

void main(void) {

   WDTCTL = WDTPW + WDTHOLD;  // Stop WDT
   clockInit();               // 16 MHz

   P1DIR |= BIT3+ BIT2 + BIT1; // P1.3, P1.2, P1.1 output
   P1OUT &= ~(BIT2 + BIT1);   // init output, off

   TA0CCR1 = HALF_PERIOD;     // CCR1 = wave 1 off time
   TA0CCR2 = HALF_PERIOD;     // CCR2 = wave 2 off time
   TA0CCR0 = HALF_PERIOD * 2; // CCR0 = period

   TA0CCTL1 |= CCIE;          //enable interrupts when TAR = CCTL1
   TA0CCTL2 |= CCIE;          //enable interrupts when TAR = CCTL2
   TA0CCTL0 |= CCIE;          //enable interrupts when TAR = CCTL0

   TA0CTL = TASSEL_2 + MC_1;  // T_A, select 16MHz clock, up mode

   __bis_SR_register(GIE);

   while(1) {
      __delay_cycles(8000000);
      setForward();
      __delay_cycles(8000000);
      setStop();
   }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void CCR0_detect (void) {
   P1OUT &= ~(BIT2 + BIT1);   // turn off p1.1 and p1.2
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void CCR1_CCR2_detect (void) {
   switch(TAIV) {
   case 2: P1OUT |= BIT1;     // turn on p1.1
         break;
   case 4: P1OUT |= BIT2;     // turn on p1.2
         break;
   }
}
