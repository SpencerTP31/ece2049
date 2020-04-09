///* Tai Kjendal, ECE box 173
// *
// */
//
//#include "msp430.h"
//#include "stdlib.h"
//
//#define MAX_PTS 300
//
//void main(void) {
//    int in[MAX_PTS], parabola[MAX_PTS]; // in: 600 bytes, parabola: 600 bytes
//    int alfa = 4;       // alfa: 2 bytes stored at R10
//    int half_max, i;    // half_max: 2 bytes, i: 2 bytes
//
//    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
//
//    half_max = MAX_PTS/2;
//    for (i=0; i<MAX_PTS; i++) { // repeats until i<300 returns false (300 iterations)
//        in[i] = i-half_max;     // sets the ith element in 'in' to 'i-150'
//        parabola[i] = (in[i]-alfa) * (in[i]-alfa);  // sets the ith element in 'parabola' to (in[i]-4) squared
//    }
//}
//
///* Tai Kjendal, ECE box 173
// *
// */
//
//#include "msp430.h"
//#include <math.h>
//#include <stdlib.h>
//
//#define MAX_PTS 300
//
//void main(void) {
//    float in_x[MAX_PTS], parab[MAX_PTS];    // in_x: 1200 bytes, parab: 1200 bytes
//    float beta; // beta: 4 bytes stored at R9.16, R8.16
//    int i=0;    // i: 2 bytes
//
//    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
//
//    beta=4.0;
//    while (i<MAX_PTS) {
//        in_x[i] = (float)(i-MAX_PTS/2);         // sets element i in in_x to i-150 (float cast)
//        parab[i] = pow((in_x[i]-beta), 2.0);    // sets element i in parab to (in_x[i]-4.0)^2
//        i++;    // increments i by 1
//    }
//}

/* Tai Kjendal, ECE box 173
 *
 */

#include "msp430.h"

// function prototypes
void setupP4_out();
void setupP4_in();
void P4inOut();

void setupP4_out() {
    P4SEL &= ~(BIT7|BIT5|BIT3|BIT1);
    P4DIR &= (BIT7|BIT5|BIT3|BIT1);
}

void setupP4_in() {
    P4SEL &= ~(BIT8|BIT4|BIT2|BIT0);
    P4DIR &= ~(BIT8|BIT4|BIT2|BIT0);
}

void P4inOut() {
    P4OUT = P4IN << 1;
}

// main function
void main(void) {
    setupP4_out();
    setupP4_in();
    while (1) P4inOut();
}

