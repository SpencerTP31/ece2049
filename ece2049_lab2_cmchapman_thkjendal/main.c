/*
 * ECE 2049 Lab 2: "MSP430 Hero"
 *     A Guitar Hero style game to run on an MSP430
 *
 * Written by : Tai Kjendal (thkjendal@wpi.edu)
 *              Colleen Chapman (cmchapman@wpi.edu)
 *
 *              February, 2020
 *
 * Notes:
 * This program contains 3 songs of increasing difficulty. Furthermore, each song can be played at 3 different songs. There is also an
 *      'endless' mode, which will loop indefinitely, increasing the speed each iteration. It ends when the player quits or loses. A
 *      player can quit any time by holding '#'. A player earns a point for every note they successfully hit. Points are displayed on
 *      the winning/losing screen.
 *
 */

#include <msp430.h>
#include "peripherals.h"
#include <stdio.h>
#include <stdlib.h>

// data type for note consists values for frequency and LEDs/button press
struct Note {
    int freq;
    char led_bp;
};

// enumerate state machine
enum status {
    welcome, playsong, lose, win,
};

// data type for playSong return
struct Song {
    enum status state;
    int score;
    int keyPress;
};

// function prototypes
void swDelay(char numLoops);
int start(void);
int chooseMode(void);
void countdown(void);
int playNote(struct Note note, int dur);
struct Song playSong(struct Note song[], size_t sz, int dur[], int pause[], int mode);
unsigned char *string(unsigned char *string, int num);
void printScore(int score);
void startTimerA2(void);
void stopTimerA2(void);
void tDelay(int d);

// global counter
unsigned long int tcount = 0;

// main loop
void main(void) {

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // initialize IO ports
    initButtons();
    configUserLeds(0);
    configDisplay();
    initLeds();
    configKeypad();

    _BIS_SR(GIE);   // enable global interrupts

    // local variables
    unsigned char currKey;
    enum status state = welcome;    // keep track of game state
    int i, song, mode;              // miscellaneous counters and things
    struct Song v;

    // note definitions
    struct Note C4  = {262, 1};
    struct Note D4  = {294, 4};
    struct Note E4  = {330, 2};
    struct Note G4  = {392, 1};
    struct Note G4s = {415, 1};
    struct Note A4  = {440, 2};
    struct Note B4b = {466, 4};
    struct Note B4  = {494, 8};
    struct Note C5  = {523, 2};
    struct Note D5  = {587, 8};
    struct Note E5  = {659, 4};
    struct Note F5  = {698, 8};
    struct Note F5s = {740, 1};
    struct Note G5  = {784, 2};
    struct Note A5b = {831, 4};
    struct Note A5  = {880, 8};
    struct Note D6  = {1174, 1};

    // song 1: Hot Cross Buns from every elementary school every
    struct Note song1[17] = {B4,A4,G4,B4,A4,G4,G4,G4,G4,G4,A4,A4,A4,A4,B4,A4,G4};
    int         dur1[17] = {75,75,150,75,75,150,37,37,37,37,37,37,37,37,75,75,150};
    int         pause1[17] = {20,20,20,20,20,20,10,10,10,10,10,10,10,10,20,20,20};

    // song 2: Megalovania from Undertale
    struct Note song2[40] = {D5,D5,D6,A5,A5b,G5,F5,D5,F5,G5, C5,C5,D6,A5,A5b,G5,F5,D5,F5,G5, B4,B4,D6,A5,A5b,G5,F5,D5,F5,G5, B4b,B4b,D6,A5,A5b,G5,F5,D5,F5,G5};
    int         dur2[40] = {20,20,20,20,20,20,50,20,20,20, 20,20,20,20,20,20,50,20,20,20, 20,20,20,20,20,20,50,20,20,20, 20,20,20,20,20,20,50,20,20,20};
    int         pause2[40] = {10,10,30,60,30,30,10,10,10,10, 10,10,30,60,30,30,10,10,10,10, 10,10,30,60,30,30,10,10,10,10, 10,10,30,60,30,30,10,10,10,10};

    // song3: Paranoid by Black Sabbath
    struct Note song3[36] = {G5,G5,G5,G5,G5,E5,E5,E5,D5,D5,D5,F5s,G5,D5,E5, G5,G5,G5,G5,G5,E5,E5,E5,D5,D5,D5,F5s,G5,D5,E5, E4,E4,C4,C4,D4,E4};
    int         dur3[36] = {20,40,40,40,40,40,40,40,20,40,40,40,20,40,40, 20,40,40,40,40,40,40,40,20,40,40,40,20,40,40, 80,20,20,80,80,160};
    int         pause3[36] = {10,10,10,10,10,10,10,10,10,10,10,10,10,10,40, 10,10,10,10,10,10,10,10,10,10,10,10,10,10,40, 10,0,5,10,10,40};

    // songLose: losing tune
    struct Note songLose[4] = {B4b,A4,G4s,G4};
    int         durLose[4] = {40,40,40,180};

    // songWin: winning tune
    struct Note songWin[6] = {G5,G5,G5,A5,A5,D6};
    int         durWin[6] = {20,20,20,40,40,180};

    while (1) {
        if (currKey == '#') {
            state = welcome;    // reset state
        }

        currKey = ' ';          // reset currKey
        setLeds(0);
        configUserLeds(0);      // reset/clear any IO
        Graphics_clearDisplay(&g_sContext);

        switch (state) {
            case welcome:   // welcome screen
                song = start();    // display welcome screen, wait for player action
                mode = chooseMode();
                state = playsong;
                countdown();    // initiate count down
                break;

            case playsong: // play song

                if (song==1) {  // switch depending on song chosen
                    Graphics_drawStringCentered(&g_sContext, "Hot Cross Buns", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    v = playSong(song1, (sizeof(song1) / sizeof(song1[0])), dur1, pause1, mode);
                }
                else if (song==2) {
                    Graphics_drawStringCentered(&g_sContext, "Megalovania", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Undertale", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    v = playSong(song2, (sizeof(song2) / sizeof(song2[0])), dur2, pause2, mode);
                }
                else if (song==3) {
                    Graphics_drawStringCentered(&g_sContext, "Paranoid", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Black Sabbath", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    v = playSong(song3, (sizeof(song3) / sizeof(song3[0])), dur3, pause3, mode);
                }

                currKey = v.keyPress;   // check if player quit
                state = v.state;        // get win/lose state

                break;

            case lose:  // losing screen
                printScore(v.score);
                Graphics_drawStringCentered(&g_sContext, "You Lose!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Press #", AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "to play again", AUTO_STRING_LENGTH, 48, 75, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                startTimerA2();
                for (i=0; i<4; i++) {
                    tcount=0;
                    while(tcount<durLose[i])
                        BuzzerOn(songLose[i].freq);
                    BuzzerOff();
                    tDelay(5);
                }
                stopTimerA2();

                currKey = getKey();
                while(currKey != '#') currKey = getKey();

                state = welcome;
                break;

            case win:   // winning screen
                printScore(v.score);
                Graphics_drawStringCentered(&g_sContext, "You Win!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Press #", AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "to play again", AUTO_STRING_LENGTH, 48, 75, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                startTimerA2();
                for (i=0; i<6; i++) {
                    tcount=0;
                    while(tcount<durWin[i])
                        BuzzerOn(songWin[i].freq);
                    BuzzerOff();
                    tDelay(5);
                }
                stopTimerA2();

                currKey = getKey();
                while(currKey != '#') currKey = getKey();

                state = welcome;
                break;

            default:    // default state is welcome screen
                state = welcome;
                break;

        }   // end state machine
    }   // end while(1)
}   // end main

// provides a software delay (replaced by timer interrupts
void swDelay(char numLoops) {
    volatile unsigned int i,j;
    for (j=0; j<numLoops; j++) {
        i = 5000;
        while (i > 0) i--;  // Do nothing
    }
}   // end swDelay

// displays welcome screen
int start(void) {
    Graphics_drawStringCentered(&g_sContext, "MSP430 Hero", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "Choose Song:", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "1 Hot Cross Buns", AUTO_STRING_LENGTH, 3, 35, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "2 Megalovania", AUTO_STRING_LENGTH, 3, 45, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "3 Paranoid", AUTO_STRING_LENGTH, 3, 55, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);

    int ret;
    unsigned char key = ' ';

    while (key==' ') {  // wait for player to choose song
        key = getKey();
        if (key=='1') { // select next song
            ret = 1; break; }
        else if (key=='2') {
            ret = 2; break; }
        else if (key=='3') {
            ret = 3; break; }
        else key = ' ';
    }

    return ret;
}   // end start

// player chooses difficulty
int chooseMode(void) {
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "Choose Mode:", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "1 Easy", AUTO_STRING_LENGTH, 18, 35, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "2 Medium", AUTO_STRING_LENGTH, 18, 45, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "3 Hard", AUTO_STRING_LENGTH, 18, 55, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "4 Endless", AUTO_STRING_LENGTH, 18, 65, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);

    int ret;
    unsigned char key = ' ';

    while (key==' ') {  // wait for player to choose mode
        key = getKey();
        if (key=='1') { // select next song
            ret = 1; break; }
        else if (key=='2') {
            ret = 2; break; }
        else if (key=='3') {
            ret = 3; break; }
        else if (key=='4') {
            ret = 4; break; }
        else key = ' ';
    }

    return ret;
}   // end chooseMode

// displays countdown after '*' is pressed
void countdown(void) {
    startTimerA2();

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "3...", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    configUserLeds(3);
    Graphics_flushBuffer(&g_sContext);
    tDelay(200);

    Graphics_drawStringCentered(&g_sContext, "2...", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
    configUserLeds(2);
    Graphics_flushBuffer(&g_sContext);
    tDelay(200);

    Graphics_drawStringCentered(&g_sContext, "1...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
    configUserLeds(1);
    Graphics_flushBuffer(&g_sContext);
    tDelay(200);

    stopTimerA2();
}   // end countdown

// plays a given note: setting the corresponding I/O values
int playNote(struct Note note, int dur) {
    int ret;
    unsigned char key = btnPress();

    setLeds(note.led_bp);   // set LEDs to corresponding note
    BuzzerOn(note.freq);    // set buzzer to corresponding note

    tcount=0;
    while(tcount<dur) {
        key = btnPress();   // keep checking for button press until note is finished
        if(key==note.led_bp)
            configUserLeds(1);
        else
            configUserLeds(2);
    }
    BuzzerOff(); setLeds(0);    // turn off IO after note is finished

    if (key==note.led_bp) ret = 0;  // check if correct button was eventually pressed
    else ret = 1;
    return ret;     // return 1 if the note was missed, 0 otherwise
}   // end playNote

struct Song playSong(struct Note song[], size_t sz, int dur[], int pause[], int mode) {
    int i, miss=0, n=0;
    unsigned char key;
    struct Song ret;

    startTimerA2();             // start timer

    for (i=0; i<sz; i++) {      // for every note in song
        key=getKey();
        if (key=='#') break;    // exit if player pressed '#'

        if (mode==1) {          // switch speeds between nodes
            miss += playNote(song[i], dur[i]*3);    // play a note, increments miss if the note is missed
            tDelay(pause[i]*2);                     // add a pause between notes
        }
        else if (mode==2) {
            miss += playNote(song[i], dur[i]*2);
            tDelay(pause[i]*2);
        }
        else if (mode==3) {
            miss += playNote(song[i], dur[i]);
            tDelay(pause[i]);
        } else break;

        n++;
        if (miss>=3) break;     // exit if player missed 3 notes
    }

    // endless mode
    if (mode==4) {
        int speed=1, d, p;
        while (miss<3) {
            for (i=0; i<sz; i++) {
                key=getKey();
                if (key=='#') break;

                d = dur[i]*2 / speed;
                if (d<=2) d=2;
                p = pause[i]*2 / speed;
                if (p<=1) p=1;

                miss += playNote(song[i], d);
                tDelay(p);

                n++;
                if (miss>=3) break;
            }

            if (key=='#') break;
            speed++;
        }
    }

    stopTimerA2();              // stop timer

    if (miss>=3) ret.state = lose;
    else ret.state = win;       // player wins if they missed less than 3 notes
    ret.score = n-miss;         // score = notes played - notes missed
    ret.keyPress = key;         // return last key press in case player quit

    return ret;
}

// creates an string of unsigned chars using c magic
unsigned char *string(unsigned char *string, int num) {
    char buf[5];    // create a buffer to hold things
    int i;          // counter
    for (i=0; i <= num; i++) {
        snprintf(buf, 4, "%d", i);      // print num as a string in buf
        string=memcpy(string, buf, 5);  // copy buffer to string
    }

    return string;
}   // end string

// prints the player's current score to the display
void printScore(int score) {
    unsigned char *str = malloc(5); // allocate a string to display score
    str = string(str, score);       // convert int to unsigned char*

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "Score:", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);

    free(str);  // prevent memory leaks
}   // end getScore

// implement timer A2
void startTimerA2(void) {
    TA2CTL = TASSEL_1 + ID_0 + MC_1;
    TA2CCR0 = 163;      // 32768 Hz * 0.005 seconds
    TA2CCTL0 = CCIE;    // enable TA2CCR0 interrupt
}   // end startTimerA2

// stop and reset timer
void stopTimerA2() {
    TA2CTL = MC_0;  // stop timer
    TA2CCTL0 &= ~CCIE;  // TA2CCR0 interrupt disabled
    tcount=0;
}   // end stopTimerA2

// timer A2 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER_A2_ISR (void) {
    tcount++;
}   // end TIMER_A2_ISR

// timer delay (for adding pauses between notes)
void tDelay(int d) {
    tcount=0;
    while (tcount<d) {
        // do nothing
    }
}   // end tDelay
