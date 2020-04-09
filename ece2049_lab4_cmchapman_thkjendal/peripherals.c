/*
 * peripherals.c
 *
 *  Created on: Jan 29, 2014
 *      Author: deemer
 *  Updated on Jan 3, 2016
 *  	smj
 *  Updated on Jan 14, 2018
 *      smj
 *  Updated on Aug 22, 2018
 *      smj
  *  Updated on Jan 9, 2019
 *      smj
 *  Updated on Feb 28, 2020
 *      thk, cmc
 */

#include "peripherals.h"


// Globals
tContext g_sContext;    // user defined type used by graphics library

// configure ADC12
unsigned int configADC12(void) {
    REFCTL0 &= ~REFMSTR;    // Reset REFMSTR
    
    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12REF2_5V | ADC12MSC | ADC12ON;
    ADC12CTL1 = ADC12SHP + ADC12CSTARTADD_0 + ADC12CONSEQ_1;
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_0 + ADC12EOS;
    
    P6SEL |= BIT0;
    ADC12CTL0 &= ~ADC12SC;
    // ADC12IE = BIT1;                     // set interrupt bit
    
    ADC12CTL0 |= ADC12SC + ADC12ENC;    // start conversion

    while (ADC12CTL1 & ADC12BUSY)       // poll busy bit
        __no_operation();

    unsigned int inVal = ADC12MEM0 & 0x0FFF;
    return inVal;
}

// configure ADC12 for the testing thing
unsigned int configADC12_test(void) {
    REFCTL0 &= ~REFMSTR;    // Reset REFMSTR

    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12REF2_5V | ADC12MSC | ADC12ON;
    ADC12CTL1 = ADC12SHP + ADC12CSTARTADD_0 + ADC12CONSEQ_1;
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_0 + ADC12EOS;

    P6SEL |= BIT1;
    ADC12CTL0 &= ~ADC12SC;
    // ADC12IE = BIT1;                     // set interrupt bit

    ADC12CTL0 |= ADC12SC + ADC12ENC;    // start conversion

    while (ADC12CTL1 & ADC12BUSY)       // poll busy bit
        __no_operation();

    unsigned int inVal = ADC12MEM0 & 0x0FFF;
    return inVal;
}

// set digital analog converter for IO
void configDAC(void) {
    DAC_PORT_LDAC_SEL &= ~DAC_PIN_LDAC;
    DAC_PORT_LDAC_DIR |= DAC_PIN_LDAC;  
    DAC_PORT_LDAC_OUT |= DAC_PIN_LDAC;  // Deassert LDAC
    
    DAC_PORT_CS_SEL &= ~DAC_PIN_CS;
    DAC_PORT_CS_DIR |= DAC_PIN_CS;
    DAC_PORT_CS_OUT |= DAC_PIN_CS;      // deassert CS
}

void setDAC(unsigned int dac_code) {
    DAC_PORT_CS_OUT &= ~DAC_PIN_CS;     // start SPI transmission
    
    dac_code |= 0x3000;                 // write in DAC configuration bits
    uint8_t lo_byte = (unsigned char)(dac_code & 0x00FF);
    uint8_t hi_byte = (unsigned char)((dac_code & 0xFF00) >> 8);

    DAC_SPI_REG_TXBUF = hi_byte;        // send high byte

    while(!(DAC_SPI_REG_IFG & UCTXIFG)) {
        __no_operation();               // wait for SPI peripheral to finish transmitting
    }

    DAC_SPI_REG_TXBUF = lo_byte;        // send low byte
    while(!(DAC_SPI_REG_IFG & UCTXIFG)) {
        __no_operation();
    }

    DAC_PORT_CS_OUT |= DAC_PIN_CS;      // deassert CS (set 1)
    
    DAC_PORT_LDAC_OUT &= ~DAC_PIN_LDAC; // assert LDAC
    __delay_cycles(10);                 // small delay
    DAC_PORT_LDAC_OUT |= DAC_PIN_LDAC;  // deassert LDAC
}

// configure digital I/O for buttons
void configButtons(void) {
    P7SEL &= ~(BIT4|BIT0);
    P7DIR &= ~(BIT4|BIT0);
    P7REN |= (BIT4|BIT0);
    P7OUT |= (BIT4|BIT0);

    P3SEL &= ~BIT6;
    P3DIR &= ~BIT6;
    P3REN |= BIT6;
    P3OUT |= BIT6;

    P2SEL &= ~BIT2;
    P2DIR &= ~BIT2;
    P2REN |= BIT2;
    P2OUT |= BIT2;
}

// returns what button is pressed
unsigned char btnPress(void) {  
    unsigned char btn = 0;      

    if(~P7IN & BIT0)
        btn |= BIT3;
    if(~P3IN & BIT6)
        btn |= BIT2;
    if(~P2IN & BIT2)
        btn |= BIT1;
    if(~P7IN & BIT4)
        btn |= BIT0;

    // It can detect and decode when multiple buttons are pressed
    return btn;
}

// configures and returns when either of two user buttons are pressed
unsigned char userBtnPress(void) {
    unsigned char btn = 0;

    if(~P2IN & BIT1)
        btn |= BIT1;
    if(~P1IN & BIT1)
        btn |= BIT0;

    return btn;
}

// configures the two user leds
void configUserLeds(char inbits) {
    P1SEL &= ~BIT0;
    P1DIR |= BIT0;

    P4SEL &= ~BIT7;
    P4DIR |= BIT7;

    P1OUT &= ~BIT0;
    P4OUT &= ~BIT7;

    if (inbits & BIT0) P4OUT |= BIT7;
    if (inbits & BIT1) P1OUT |= BIT0;
}

void configLeds(void) {
    // Configure LEDs as outputs, initialize to logic low (off)
    // Note the assigned port pins are out of order test board
    // Red     P6.2
    // Green   P6.1
    // Blue    P6.3
    // Yellow  P6.4
    // smj -- 27 Dec 2016

    P6SEL &= ~(BIT4|BIT3|BIT2|BIT1);
    P6DIR |=  (BIT4|BIT3|BIT2|BIT1);
    P6OUT &= ~(BIT4|BIT3|BIT2|BIT1);
}

void setLeds(unsigned char state) {
    // Turn on 4 colored LEDs on P6.1-6.4 to match the hex value
    // passed in on low nibble state. Unfortunately the LEDs are
    // out of order with 6.2 is the left most (i.e. what we think
    // of as MSB), then 6.1 followed by 6.3 and finally 6.4 is
    // the right most (i.e.  what we think of as LSB) so we have
    // to be a bit clever in implementing our LEDs
    //
    // Input: state = hex values to display (in low nibble)
    // Output: none
    //
    // smj, ECE2049, 27 Dec 2015

    unsigned char mask = 0;

    // Turn all LEDs off to start
    P6OUT &= ~(BIT4|BIT3|BIT2|BIT1);

    if (state & BIT0)
        mask |= BIT4;   // Right most LED P6.4
    if (state & BIT1)
        mask |= BIT3;   // next most right LED P.3
    if (state & BIT2)
        mask |= BIT1;   // third most left LED P6.1
    if (state & BIT3)
        mask |= BIT2;   // Left most LED on P6.2
    P6OUT |= mask;
}


/*
 * Enable a PWM-controlled buzzer on P3.5
 * This function makes use of TimerB0.
 */
void BuzzerOn(int freq) {
    // Initialize PWM output on P3.5, which corresponds to TB0.5
    P3SEL |= BIT5; // Select peripheral output mode for P3.5
    P3DIR |= BIT5;

    TB0CTL  = (TBSSEL__ACLK|ID__1|MC__UP);  // Configure Timer B0 to use ACLK, divide by 1, up mode
    TB0CTL  &= ~TBIE;                       // Explicitly Disable timer interrupts for safety

    // Now configure the timer period, which controls the PWM period
    // Doing this with a hard coded values is NOT the best method
    // We do it here only as an example. You will fix this in Lab 2.
    TB0CCR0   = 32768 / freq;           // Set the PWM period in ACLK ticks (32768 Hz / Note's frequency)
    TB0CCTL0 &= ~CCIE;                  // Disable timer interrupts

    // Configure CC register 5, which is connected to our PWM pin TB0.5
    TB0CCTL5  = OUTMOD_7;                   // Set/reset mode for PWM
    TB0CCTL5 &= ~CCIE;                      // Disable capture/compare interrupts
    TB0CCR5   = TB0CCR0/2;                  // Configure a 50% duty cycle
}

// Disable the buzzer on P7.5
void BuzzerOff(void) {
    // Disable both capture/compare periods
    TB0CCTL0 = 0;
    TB0CCTL5 = 0;
}


void configKeypad(void) {
    // Configure digital IO for keypad
    // smj -- 27 Dec 2015

    // Col1 = P1.5 =
    // Col2 = P2.4 =
    // Col3 = P2.5 =
    // Row1 = P4.3 =
    // Row2 = P1.2 =
    // Row3 = P1.3 =
    // Row4 = P1.4 =

    P1SEL &= ~(BIT5|BIT4|BIT3|BIT2);
    P2SEL &= ~(BIT5|BIT4);
    P4SEL &= ~(BIT3);

    // Columns are ??
    P2DIR |= (BIT5|BIT4);
    P1DIR |= BIT5;
    P2OUT |= (BIT5|BIT4); //
    P1OUT |= BIT5;        //

    // Rows are ??
    P1DIR &= ~(BIT2|BIT3|BIT4);
    P4DIR &= ~(BIT3);
    P4REN |= (BIT3);  //
    P1REN |= (BIT2|BIT3|BIT4);
    P4OUT |= (BIT3);  //
    P1OUT |= (BIT2|BIT3|BIT4);
}

unsigned char getKey(void) {
    // Returns ASCII value of key pressed from keypad or 0.
    // Does not decode or detect when multiple keys pressed.
    // smj -- 27 Dec 2015
    // Updated -- 14 Jan 2018

    unsigned char ret_val = 0;

    // Set Col1 = ?, Col2 = ? and Col3 = ?
    P1OUT &= ~BIT5;
    P2OUT |= (BIT5|BIT4);
    // Now check value from each rows
    if ((P4IN & BIT3)==0)
        ret_val = '1';
    if ((P1IN & BIT2)==0)
        ret_val = '4';
    if ((P1IN & BIT3)==0)
        ret_val = '7';
    if ((P1IN & BIT4)==0)
        ret_val = '*';
    P1OUT |= BIT5;

    // Set Col1 = ?, Col2 = ? and Col3 = ?
    P2OUT &= ~BIT4;
    // Now check value from each rows
    if ((P4IN & BIT3)==0)
        ret_val = '2';
    if ((P1IN & BIT2)==0)
        ret_val = '5';
    if ((P1IN & BIT3)==0)
        ret_val = '8';
    if ((P1IN & BIT4)==0)
        ret_val = '0';
    P2OUT |= BIT4;

    // Set Col1 = ?, Col2 = ? and Col3 = ?
    P2OUT &= ~BIT5;
    // Now check value from each rows
    if ((P4IN & BIT3)==0)
        ret_val = '3';
    if ((P1IN & BIT2)==0)
        ret_val = '6';
    if ((P1IN & BIT3)==0)
        ret_val = '9';
    if ((P1IN & BIT4)==0)
        ret_val = '#';
    P2OUT |= BIT5;

    return(ret_val);
}


void configDisplay(void) {
    // Enable use of external clock crystals
     P5SEL |= (BIT5|BIT4|BIT3|BIT2);

	// Initialize the display peripheral
	Sharp96x96_Init();

    // Configure the graphics library to use this display.
	// The global g_sContext is a data structure containing information the library uses
	// to send commands for our particular display.
	// You must pass this parameter to each graphics library function so it knows how to
	// communicate with our display.
    Graphics_initContext(&g_sContext, &g_sharp96x96LCD);

    Graphics_setForegroundColor(&g_sContext, ClrBlack);
    Graphics_setBackgroundColor(&g_sContext, ClrWhite);
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
    Graphics_flushBuffer(&g_sContext);
}