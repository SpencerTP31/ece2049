/*
 * ECE 2049 Lab 3: "Time and Temperature Display"
 *     Measure and display time and temperature on an MSP430
 *
 * Written by : Tai Kjendal (thkjendal@wpi.edu)
 *              Colleen Chapman (cmchapman@wpi.edu)
 *
 *              February, 2020
 *
 * Notes:
 * This program measures tempurature using the ADC12 temperature sensor, updating once per second
 *     with Timer A2. Time is displayed, along with the temperature on the LCD
 *
 */

#include <msp430.h>
#include "peripherals.h"

#define MA_PER_BIT 0.244    // 1.0A / 4096
#define CALADC12_25V_30C *((unsigned int *)0x1A22);
#define CALADC12_25V_85C *((unsigned int *)0x1A24);

// function prototypes
void displayTime(long unsigned int inTimer, int sel);
void displayTemp(float inAvgTempC);
void startTimerA2(void);
void stopTimerA2(void);

// global variables
long unsigned int tcount = 29116800,
        mo = 0,
        dd = 0,
        hh = 0,
        mm = 0,
        ss = 0;
volatile unsigned int in_temp, in_current;

// main loop
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;   // Stop WDT

    // local variables
    unsigned char btn;
    int i, select, lastS, month_sel,
    len=0, state = 0;
    int monthDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    float tempVals[36] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float avgC = 0.0;
    volatile float tempC;
    unsigned int bits30, bits85;

    configDisplay();    // initialize display
    config_ADC12();     // initialize ADC12

    bits30 = CALADC12_25V_30C;
    bits85 = CALADC12_25V_85C;

    // enable global interrupts
    _BIS_SR(GIE);

    startTimerA2();

    while (1) {

        tempC = (float)(((long)in_temp - bits30)*(85-30))/(bits85-bits30) + 30.0;

        btn = userBtns();           // check for btn press
        if (btn == 2) state = 1;    // go to edit mode if left btn is pressed
        else state = 0;

        switch (state) {

        case 0: // display mode

            if (len >= 36) len = 36;        // increment length of array until it has 36 values
            else len++;
            for (i=0; i<len; i++) {         // sums len elements in tempVals[]
                tempVals[i] = tempC;        // fill tempVals[] with tempC
                avgC += tempVals[i];
            }
            avgC /= len;                    // average = sum / 4

            displayTime(tcount, 0);         // update time display
            displayTemp(avgC);              // update temperature display

            break;

        case 1: // edit mode

            lastS = 0;
            select = 1;
            stopTimerA2();

            while (1) {

                if (select==1) {                                // change month
                    mo = 0;
                    month_sel = in_current/342;
                    for (i=0; i<month_sel; i++) {
                        mo += monthDays[i] * 86400;
                    }
                } else if (select==2)                           // change day
                    dd = (in_current/ (4095/monthDays[month_sel] + 1)) * 86400;
                else if (select==3) {                           // change hour
                    hh = (in_current/171) * 3600;
                    if (hh==2864) hh = 68400;
                    else if (hh == 6464) hh = 72000;
                    else if (hh == 10064) hh = 75600;
                    else if (hh == 13664) hh = 79200;
                    else if (hh == 17264) hh = 82800;
                } else if (select==4)                           // change minute
                    mm = (in_current/69) * 60;
                else if (select==5)                             // change second
                    ss = (in_current/69);

                tcount = mo + dd + hh + mm + ss;
                displayTime(tcount, select);        // update time display

                if (lastS==0) {
                    btn = userBtns();
                    if (btn==1) break;              // leave edit mode if right btn is pressed
                    else if (btn==2) select++;      // edit next value if left btn is pressed
                    if (select > 5) select=1;       // loop edit values
                    if (btn!=0) lastS = 1;
                } else lastS = 0;
            }

            startTimerA2();

            break;

        default:
            state = 0;
            break;
        } // end switch
    } // end while(1)
} // end main

// display the time
void displayTime(long unsigned int inTime, int sel) {
    unsigned char time[8];
    unsigned char date[6];
    long int days = inTime / 86400 + 1;
    long int day = days;

    if (days < 32) {
        date[0] = 'J'; date[1] = 'A'; date[2] = 'N';
    } else if (days >= 32 && days < 60) {
        date[0] = 'F'; date[1] = 'E'; date[2] = 'B';
        day -= 31;
    } else if (days >= 60 && days < 91) {
        date[0] = 'M'; date[1] = 'A'; date[2] = 'R';
        day -= 59;
    } else if (days >= 91 && days < 121) {
        date[0] = 'A'; date[1] = 'P'; date[2] = 'R';
        day -= 90;
    } else if (days >= 121 && days < 152) {
        date[0] = 'M'; date[1] = 'A'; date[2] = 'Y';
        day -= 120;
    } else if (days >= 152 && days < 182) {
        date[0] = 'J'; date[1] = 'U'; date[2] = 'N';
        day -= 151;
    } else if (days >= 182 && days < 213) {
        date[0] = 'J'; date[1] = 'U'; date[2] = 'L';
        day -= 181;
    } else if (days >= 213 && days < 244) {
        date[0] = 'A'; date[1] = 'U'; date[2] = 'G';
        day -= 212;
    } else if (days >= 244 && days < 274) {
        date[0] = 'S'; date[1] = 'E'; date[2] = 'P';
        day -= 243;
    } else if (days >= 274 && days < 305) {
        date[0] = 'O'; date[1] = 'C'; date[2] = 'T';
        day -= 273;
    } else if (days >= 305 && days < 335) {
        date[0] = 'N'; date[1] = 'O'; date[2] = 'V';
        day -= 304;
    } else if (days >= 335 && days < 366) {
        date[0] = 'D'; date[1] = 'E'; date[2] = 'C';
        day -= 334;
    } else {
        date[0] = 'E'; date[1] = 'R'; date[2] = 'R';
        day = 0;
    }

    date[3] = ' ';
    date[4] = (day / 10) + 0x30;
    date[5] = (day % 10) + 0x30;

    int h = (inTime/3600) % 24;
    int m = (inTime/60) % 60;
    int s = inTime % 60;

    time[0] = (h / 10) + 0x30;
    time[1] = (h % 10) + 0x30;
    time[2] = ':';
    time[3] = (m / 10) + 0x30;
    time[4] = (m % 10) + 0x30;
    time[5] = ':';
    time[6] = (s / 10) + 0x30;
    time[7] = (s % 10) + 0x30;

    Graphics_drawStringCentered(&g_sContext, date, 6, 48, 25, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, time, 8, 48, 35, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);


    if (sel==1) {               // month
        date[0] = ' '; date[1] = ' '; date[2] = ' ';
    } else if (sel==2) {        // day
        date[4] = ' '; date[5] = ' ';
    } else if (sel==3) {        // hour
        time[0] = ' '; time[1] = ' ';
    } else if (sel==4) {        // minute
        time[3] = ' '; time[4] = ' ';
    } else if (sel==5) {        // second
        time[6] = ' '; time[7] = ' ';
    }

    Graphics_drawStringCentered(&g_sContext, date, 6, 48, 25, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, time, 8, 48, 35, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
} // end displayTime

// display the temperature in F and C
void displayTemp(float inAvgTempC) {
    unsigned char tempC[7];
    unsigned char tempF[7];

    int c = (int)(inAvgTempC * 10);
    int f = (int)((inAvgTempC*1.8 + 32) * 10);

    tempC[0] = (c/1000 % 10) + 0x30;
    tempC[1] = (c/100 % 10) + 0x30;
    tempC[2] = (c/10 % 10) + 0x30;
    tempC[3] = '.';
    tempC[4] = (c % 10) + 0x30;
    tempC[5] = ' ';
    tempC[6] = 'C';

    tempF[0] = (f/1000 % 10) + 0x30;
    tempF[1] = (f/100 % 10) + 0x30;
    tempF[2] = (f/10 % 10) + 0x30;
    tempF[3] = '.';
    tempF[4] = (f % 10) + 0x30;
    tempF[5] = ' ';
    tempF[6] = 'F';

   /*char buf[10];
   snprintf(buf, 9, "%d.%d %s", c/10, c%10, "C");
   memcpy(tempC, buf, 10);
   snprintf(buf, 9, "%d.%d %s", f/10, f%10, "F");
   memcpy(tempF, buf, 10);*/

    Graphics_drawStringCentered(&g_sContext, tempC, sizeof(tempC)/sizeof(tempC[0]), 48, 55, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, tempF, sizeof(tempC)/sizeof(tempC[0]), 48, 65, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);

   /*free(tempC);
   free(tempF);*/
} // end displayTemp

// implement timer A2
void startTimerA2(void) {
    TA2CTL = TASSEL_1 + ID_0 + MC_1;
    TA2CCR0 = 32768;    // t_int = 1sec
    TA2CCTL0 = CCIE;    // enable TA2CCR0 interrupt
}   // end startTimerA2

// stop and reset timer
void stopTimerA2() {
    TA2CTL = MC_0;      // stop timer
    TA2CCTL0 &= ~CCIE;  // TA2CCR0 interrupt disabled
    tcount=0;           // reset tcount
}   // end stopTimerA2

// timer A2 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER_A2_ISR (void) {
    long unsigned int year = 31536000;  // 365*24*60*60
    if (tcount >= year) tcount = 0;
    else tcount++;
}   // end TIMER_A2_ISR

#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void) {
    ADC12CTL0 |= ADC12SC + ADC12ENC;
    in_current = ADC12MEM0;
    in_temp = ADC12MEM1;
}
