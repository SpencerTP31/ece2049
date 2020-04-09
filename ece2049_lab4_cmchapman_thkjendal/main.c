/*
 * ECE 2049 Lab 4: "Who's Watching the Watchers?"
 *
 * Written by : Tai Kjendal
 *              Colleen Chapman
 *
 *              March 3, 2020
 *
 * Bugs:
 *  The board doesn't seem to be able to output more than a ~300 Hz triangle wave, 
 *  or ~800 Hz when not using scroll.
 *  
 */

// includes
#include <msp430.h>
#include "peripherals.h"

// function prototypes
void startTimerA2(void);
void stopTimerA2(int reset);
void displayWelcome(void);
void outputLoop(void);

// global variables
long unsigned int time_cnt = 0;
int vOut = 0, test = 0;
int m = 0;
int quit;

enum State {
    welcome, dc, square, sawtooth, triangle
};

enum State state = welcome;

// main loop
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;   // Stop WDT

    // local variables
    int pressed;

    // configure IO, DAC
    configDisplay();
	configDAC();
    configButtons();
    configLeds();

    // initialize stuff to 0/off
    setLeds(0);
    Graphics_clearDisplay(&g_sContext);
    setDAC(0);

    _BIS_SR(GIE);       // enable global interrupts

    while (1) {         // main loop

        setLeds(0);
        Graphics_clearDisplay(&g_sContext);

        switch (state) {
        case welcome:
            displayWelcome();       // display welcome and instructions

            pressed = btnPress();
            while (pressed == 0)    // wait for user to select waveform
                pressed = btnPress();

            if (pressed==1)         state = triangle;
            else if (pressed==2)    state = sawtooth;
            else if (pressed==4)    state = square;
            else if (pressed==8)    state = dc;

            break;

        case dc:
            Graphics_drawStringCentered(&g_sContext, "DC Voltage", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "Change V with", AUTO_STRING_LENGTH, 5, 35, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "scroll wheel", AUTO_STRING_LENGTH, 5, 45, OPAQUE_TEXT);
            Graphics_drawStringCentered(&g_sContext, "exit: user btn1", AUTO_STRING_LENGTH, 48, 75, OPAQUE_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(pressed);

            outputLoop();

            break;

        case square:    // 100 Hz, pk-pk, 50% duty square wave
            Graphics_drawStringCentered(&g_sContext, "Square Wave", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "Change freq w/", AUTO_STRING_LENGTH, 5, 35, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "scroll wheel", AUTO_STRING_LENGTH, 5, 45, OPAQUE_TEXT);
            Graphics_drawStringCentered(&g_sContext, "exit: user btn1", AUTO_STRING_LENGTH, 48, 75, OPAQUE_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(pressed);

            outputLoop();

            break;

        case sawtooth:  // 50 Hz, pk-pk, 50% cycle sawtooth wave
            Graphics_drawStringCentered(&g_sContext, "Sawtooth Wave", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "Change freq w/", AUTO_STRING_LENGTH, 5, 35, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "scroll wheel", AUTO_STRING_LENGTH, 5, 45, OPAQUE_TEXT);
            Graphics_drawStringCentered(&g_sContext, "exit: user btn1", AUTO_STRING_LENGTH, 48, 75, OPAQUE_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(pressed);

            outputLoop();

            break;

        case triangle:  // 50 Hz, pk-pk, 50% cycle, triangle wave
            Graphics_drawStringCentered(&g_sContext, "Triangle Wave", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "Change freq w/", AUTO_STRING_LENGTH, 5, 35, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext, "scroll wheel", AUTO_STRING_LENGTH, 5, 45, OPAQUE_TEXT);
            Graphics_drawStringCentered(&g_sContext, "exit: user btn1", AUTO_STRING_LENGTH, 48, 75, OPAQUE_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(pressed);

            outputLoop();

            break;
        }
    }
}   // end main

// prints instructions in the initial state
void displayWelcome(void) {
    Graphics_drawStringCentered(&g_sContext, "MSP430", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, "Func. Generator", AUTO_STRING_LENGTH, 48, 25, OPAQUE_TEXT);
    Graphics_drawString(&g_sContext, "B1 - DC", AUTO_STRING_LENGTH, 6, 35, OPAQUE_TEXT);
    Graphics_drawString(&g_sContext, "B2 - Square", AUTO_STRING_LENGTH, 6, 45, OPAQUE_TEXT);
    Graphics_drawString(&g_sContext, "B3 - Sawtooth", AUTO_STRING_LENGTH, 6, 55, OPAQUE_TEXT);
    Graphics_drawString(&g_sContext, "B4 - Triangle", AUTO_STRING_LENGTH, 6, 65, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
}

// generates selected waveform until user quits
void outputLoop(void) {
    quit = 0;
    startTimerA2();

    while (quit != 2) {     // loop until user quits, quit checked in interrupt
        __no_operation();
    }

    stopTimerA2(1);
    state = welcome;
}

// start timer A2
void startTimerA2(void) {    
    if (state==dc) {
        TA2CTL = TASSEL_1 + ID_0 + MC_1;    // ACLK in up mode
        TA2CCR0 = 327;      // t_int ~0.01 seconds
    }
    
    else if (state==square) {
        TA2CTL = TASSEL_1 + ID_0 + MC_1;
        TA2CCR0 = 163;      // 32768/100Hz / 2 steps = 163
    }
    
    else if (state==sawtooth) {
        TA2CTL = TASSEL_1 + ID_0 + MC_1;
        TA2CCR0 = 6;        // 32768/50Hz / 100 steps = 6
    }    
    
    // triangle @ 100 - 1000 Hz, lower resolution
    else if (state==triangle) {
        TA2CTL = TASSEL_2 + ID_0 + MC_3;    // SMCLK in up-down mode
        TA2CCR0 = 87;                       // 1048576 / 100Hz / 3*2 steps / 2 (up-down) = 873
    }
    
    // triangle @ 50Hz and higher resolution
    // else if (state==triangle) {
    //     TA2CTL = TASSEL_1 + ID_0 + MC_1;    // SMCLK in up-down mode
    //     TA2CCR0 = 6;                        // 32768 / 50Hz / 50*2 steps = 6
    // }

    TA2CCTL0 = CCIE;    // enable TA2CCR0 interrupt
}

// stop and reset timer
void stopTimerA2(int reset) {
    TA2CTL = MC_0;      // stop timer
    TA2CCTL0 &= ~CCIE;  // TA2CCR0 interrupt disabled
    if (reset)
        time_cnt=0;     // reset time_cnt
}

// timer A2 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER_A2_ISR (void) {
    time_cnt++;

    if (state==dc) {
        vOut = configADC12();       // change max voltage with scroll
        test = configADC12_test();  // for lab part 10
    }
    
    else if (state==square) {

        if (time_cnt%2 == 0)
            vOut = 0;
        else if (time_cnt%2 == 1)
            vOut = configADC12();   // change max voltage with scroll
    }

    else if (state==sawtooth) {
        vOut += 41;         // 4095 codes / 100 steps
        if (vOut>=4095)     // max Vcc volts
            vOut = 0;
    }

    else if (state==triangle) {
        TA2CCR0 = (configADC12() / 5) + 54;     // TA2CCRO ranges from 1747 to 174 (1KHz to 100Hz)

        if (m==0) {
            vOut += 1365;   // 4095 codes / 3 steps = 1365, 10 steps = 409
                            // 4095 codes / 50 steps = 82
            if (vOut>=4095)
                m=1;
        } 
        
        else if (m==1) {    // count down
            vOut -= 1365;
            if (vOut<=0)    // min 0 volts
                m=0;
        }
    }

    setDAC(vOut);           // update DAC accordingly

    quit = userBtnPress();
    if (quit == 2)  stopTimerA2(1);     // stop if user "quits"
}
