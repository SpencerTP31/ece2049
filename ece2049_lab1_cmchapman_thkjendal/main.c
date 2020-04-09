/** ECE 2049 - Lab 1
 * Hunter Kjendal, Colleen Chapman
 * Blackjack program on MSP430
 *
 * Future dev ideas/notes:
 * - define Card and Deck as data types, to compact code and make using them easier
 * - check win/loss (state 8) should be a function
 * - fix the cpu betting system
 * - clean up the restart functionality
 */


#include <msp430.h>
#include "peripherals.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void restart(unsigned char key);    // pauses program until '*' or '#' is pressed (restarts or continues respectively)
unsigned char *string(unsigned char *string, char *text, int num);   // creates string of unsigned chars to display
void dispCards(bool cpu, bool end, int place, int hand[], unsigned char disp[]);    // displays player and CPU cards
void buzzerthing(unsigned char period); // rings buzzer and LEDs
unsigned char cutDeck(unsigned char disp);  // returns 2 digit input (to seed rand() )
void initDeck(int deck[]);                  // fills deck with card IDs (0-51)
void swapCards(int *card1, int *card2);     // swaps 2 given cards
void shuffle(int deck[], unsigned char disp[]);         // call rand() and swapCards() to randomly switch cards in the deck
unsigned char *cSuit(int id);   // returns a card's suit from id
unsigned char *cFace(int id);   // returns card's face
unsigned char *cVal(int id);    // returns card's actual value (1-10, aces are 1 and are dealt with separately)
int value(int place, int hand[]);   // return numerical value of a hand
int betPlayer(int balance);     // prompts player to place a bet
int betCpu(int handVal);        // CPU places bet based on its hand
void swDelay(char numLoops);    // provide some delay

void main(void) {
    int state=0;
    int i, placeDeck=0, placeP=0, placeC=0;  // counters to keep track of position in deck and hands
    int playerBalance = 16, playerBet, cpuBet, pool=0;  // some variables to store things
    unsigned char currKey, start, disp[4]; // some variables to store chars
    unsigned char *str = malloc(13);    // create and allocates usable string
    int handVal, handValPlayer;             // used to evaluate value of hand

    int deck[52], pHand[11] = {0,0,0,0,0,0,0,0,0,0,0}, cHand[11] = {0,0,0,0,0,0,0,0,0,0,0}; // initialize deck and hands

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer. Always need to stop this!!

    initLeds();
    configDisplay();
    configKeypad();
    Graphics_clearDisplay(&g_sContext);

    while (1) {
        if (currKey=='*') {             // if the last key press was
            playerBalance=16; pool=0;   // reset balance and pool
            state=0;                    // go to welcome screen
        }
        currKey=' '; start=' '; // reset currKey, start
        Graphics_clearDisplay(&g_sContext); setLeds(0); // clear display and LEDs

        switch (state) {    // state machine
            case 0: // welcome screen
                Graphics_drawStringCentered(&g_sContext, "MSP430 Blackjack", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Press * to start", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);  // print to LCD

                while(start!='*') start=getKey();   // wait for '*' to be pressed
                state++;    // go to next state
                break;

            case 1: // cut and shuffle deck

                disp[0]=' '; disp[1]=' '; disp[2]=' '; disp[3]=' '; // set dips[4] to spaces
                initDeck(deck);         // fill deck with card IDs
                shuffle(deck, disp);    // shuffle "cards"

                Graphics_drawStringCentered(&g_sContext, disp, AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);   // display the number entered
                Graphics_flushBuffer(&g_sContext);
                buzzerthing((disp[1]-0x30)*10 + (disp[2]-0x30));    // set LEDs and ring buzzer according to input

                restart(currKey);   // wait for '#' to continue or '*' to restart
                state++;    // next state
                break;

            case 2: // deal table
                pHand[0] = deck[0]; // deal the first four cards to player and CPU
                cHand[0] = deck[1];
                pHand[1] = deck[2];
                cHand[1] = deck[3];
                placeDeck+=4; placeP+=2; placeC+=2; // increment counters to keep track of position

                Graphics_clearDisplay(&g_sContext);
                dispCards(false, false, placeP, pHand, disp);   // show player hand
                dispCards(true, false, placeC, cHand, disp);    // show CPU hand

                restart(currKey);
                state++;
                break;

            case 3: // player places bet
                Graphics_drawStringCentered(&g_sContext, "Enter bet amount", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "1, 2, 4, or 8", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);

                str = string(str, "Balance:", playerBalance);

                Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                disp[0]=' '; disp[1]=' ';disp[2]=' '; disp[3]=' ';
                disp[1] = betPlayer(playerBalance) + '0';   // player enters bet

                // increment and set appropriate values
                playerBet = disp[1] - 0x30;
                pool += playerBet;
                playerBalance -= playerBet;

                Graphics_drawStringCentered(&g_sContext, disp, AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                buzzerthing(disp[1] - 0x30);

                while(currKey!='#') currKey=getKey();

                str = string(str, "Balance:", playerBalance);

                Graphics_clearDisplay(&g_sContext); // update and display new balance
                Graphics_drawStringCentered(&g_sContext, disp, AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                restart(currKey);
                state++;
                break;

            case 4: // cpu places bet
                Graphics_drawStringCentered(&g_sContext, "CPU bet amount", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

                handVal = value(placeC, cHand);  // evaluate value of hand

                disp[0]=' ';disp[2]=' '; disp[3]=' ';
                disp[1] = betCpu(handVal) + '0';    // bets based on hand value

                cpuBet = disp[1] - 0x30;
                pool += cpuBet;

                str = string(str, "Pool:", pool);

                Graphics_drawStringCentered(&g_sContext, disp, AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                buzzerthing(disp[1] - 0x30);

                restart(currKey);
                state++;
                break;

            case 5: // gives option to meet bet if CPU bet higher
                if (cpuBet > playerBet) {
                    Graphics_drawStringCentered(&g_sContext, "Meet CPU bet?", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "(enter diff)", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "Fold (press *)", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);

                    Graphics_drawStringCentered(&g_sContext, "bets:", AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);

                    str = string(str, "Player:", playerBet);

                    Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);

                    str = string(str, "CPU:", playerBet);

                    Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 75, TRANSPARENT_TEXT);
                    Graphics_flushBuffer(&g_sContext);

                    unsigned char diff = (cpuBet-playerBet)+'0';
                    currKey = ' ';
                    while (currKey == ' ') {
                        currKey = getKey();
                        if (currKey=='*' || currKey==diff) break;
                        else currKey=' ';
                    }
                    if (currKey=='*') {
                        state=0; break;
                    }
                    else {

                        playerBet = cpuBet;
                        pool+=(diff-0x30);
                        playerBalance-=(diff-0x30);

                        Graphics_clearDisplay(&g_sContext);

                        str = string(str, "Balance:", playerBalance);

                        Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

                        str = string(str, "Pool:", pool);

                        Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);
                        Graphics_flushBuffer(&g_sContext);
                    }

                    if (currKey != '*') {restart(currKey); state++; break;}
                    else break;
                }
                else {
                    state++; break;
                }

            case 6: // player action
                Graphics_drawStringCentered(&g_sContext, "Hit (press 1)", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Hold (press #)", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                currKey = ' ';      // prompts user to press '#' (hold) or '1' (hit)
                while (currKey == ' ') {
                    currKey = getKey();
                    if (currKey=='1' || currKey=='#') break;
                    else currKey=' ';
                }

                disp[0]=' '; disp[1]=currKey; disp[2]=' '; disp[3]=' ';
                Graphics_drawStringCentered(&g_sContext, disp, AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                if (currKey=='1') { // if player hits
                    pHand[placeP] = deck[placeDeck];
                    placeDeck+=1; placeP+=1;

                    Graphics_clearDisplay(&g_sContext);
                    dispCards(false, false, placeP, pHand, disp);    // display new hand

                    while (currKey!='#') currKey=getKey();
                    currKey=' ';
                    swDelay(1);

                    handValPlayer = value(placeP, pHand);   // value hand, if its more than 21, player busts and round ends
                    if (handValPlayer > 21) {               // hard loss
                        state=9;
                        break;
                    } else {
                        state=6; break;
                    }

                } else {    // if player holds
                    currKey=' ';
                    state=7;    // cpu's turn
                    break;
                }

            case 7: // cpu action
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "CPU Action:", AUTO_STRING_LENGTH, 48, 5, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                handVal = value(placeC, cHand);  // value cpu's cards

                if(handVal<=16) {   // cpu hits on 16 or less
                    cHand[placeC] = deck[placeDeck];
                    placeDeck+=1; placeC+=1;

                    Graphics_clearDisplay(&g_sContext);
                    Graphics_drawStringCentered(&g_sContext, "CPU Action: Hit", AUTO_STRING_LENGTH, 48, 5, TRANSPARENT_TEXT);
                    dispCards(true, false, placeC, cHand, disp);    // display new hand

                    restart(currKey);
                } else {    // hand is more than 16 so it holds
                    Graphics_clearDisplay(&g_sContext);
                    Graphics_drawStringCentered(&g_sContext, "CPU Action: Hold", AUTO_STRING_LENGTH, 48, 5, TRANSPARENT_TEXT);
                    dispCards(false, false, placeP, pHand, disp);   // display new hand
                    dispCards(true, true, placeC, cHand, disp);     // display new hand

                    restart(currKey);
                    Graphics_clearDisplay(&g_sContext);
                    state=8; break;
                }

                handVal = 0;
                for (i=0; i<placeC; i++) {
                    if (*cVal(cHand[i])!='X') handVal += *cVal(cHand[i])-0x30;
                    else handVal += 10;
                }
                if (handVal > 21) {       // hard loss for CPU
                    state=10; Graphics_clearDisplay(&g_sContext);
                    break;
                }
                else {state=7; break;}


            case 8:  // check win loss

                if (handValPlayer > 21 && handVal <=21) {       // hard loss
                    state=9;
                    break;
                }
                else if (handValPlayer <= 21 && handVal >21) {  // hard win
                    state=10;
                    break;
                }
                else if (handVal == handValPlayer) {            // draw?
                    state=11;
                    break;
                }
                else if (handVal > handValPlayer) {             // soft loss
                    state=9;
                    break;
                }
                else if (handVal < handValPlayer) {             // soft win
                    state=10;
                    break;
                }



            case 9:    // player loses
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "You Lost!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

                str = string(str, "Lost $", playerBet);

                Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                buzzerthing(9);
                restart(currKey);
                currKey='*';
                break;

            case 10:   // player wins
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "You Won!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);

                str = string(str, "Made $", playerBet);

                Graphics_drawStringCentered(&g_sContext, str, AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                buzzerthing(1);
                restart(currKey);
                currKey='*';
                break;

            case 11:  // draw
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "Draw!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);

                buzzerthing(7);
                restart(currKey);
                currKey='*';
                break;
            default:    // welcome screen
                state=0;
                break;
        }
    }

    free(str);
}

// waits until '*' or '#' is pressed to continue
void restart(unsigned char key) {
    Graphics_drawStringCentered(&g_sContext, "Press # to cont.", AUTO_STRING_LENGTH, 48, 75, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    key = ' ';
    while (key == ' ') {
        key = getKey();
        if (key=='*' || key=='#') break;
        else key=' ';
    }
}

// performs some fancy c stuff that returns a usable string
unsigned char *string(unsigned char *string, char *text, int num) {
    char buf[13];   // create a buffer to hold things
    int i;
    for (i=0; i <= num; i++) {
        snprintf(buf, 12, "%s %d", text, i);
        string=memcpy(string, buf, 13);
    }

    return string;
}

// cpu-is it the cpu's hand?    end-is it the end of the game?  place-deck counter  disp-char to display    hand-hand to display
void dispCards(bool cpu, bool end, int place, int hand[], unsigned char disp[]) {
    if (cpu && !end) {
        Graphics_drawString(&g_sContext, "CPU:", AUTO_STRING_LENGTH, 45, 15, TRANSPARENT_TEXT);
        if (*cFace(hand[place])=='1') disp[1]='0'; else disp[1]=' ';
        disp[0]=*cFace(hand[0]); disp[2]='-'; disp[3]=*cSuit(hand[0]);
        Graphics_drawString(&g_sContext, disp, AUTO_STRING_LENGTH, 45, 25+(place*10), TRANSPARENT_TEXT);
        for (place; place-1>0; place--) {
            Graphics_drawString(&g_sContext, "xxx", AUTO_STRING_LENGTH, 45, 25+(place*10), TRANSPARENT_TEXT);
        }
    }
    else if (cpu && end) {
        Graphics_drawString(&g_sContext, "CPU:", AUTO_STRING_LENGTH, 45, 15, TRANSPARENT_TEXT);
        for (place; place>0; place--) { // for each card in given hand
            if (*cFace(hand[place])=='1') disp[1]='0'; else disp[1]=' ';   // deal with any 10s
            disp[0]=*cFace(hand[place]); disp[2]='-'; disp[3]=*cSuit(hand[place]);  // get card faces and suits from id
            Graphics_drawString(&g_sContext, disp, AUTO_STRING_LENGTH, 45, 25+(place*10), TRANSPARENT_TEXT); // show each card on its own line
        }
    }
    else {
        Graphics_drawString(&g_sContext, "Player Hand:", AUTO_STRING_LENGTH, 5, 15, TRANSPARENT_TEXT);
        for (place; place>0; place--) { // for each card in given hand
            if (*cFace(hand[place])=='1') disp[1]='0'; else disp[1]=' ';   // deal with any 10s
            disp[0]=*cFace(hand[place]); disp[2]='-'; disp[3]=*cSuit(hand[place]);  // get card faces and suits from id
            Graphics_drawString(&g_sContext, disp, AUTO_STRING_LENGTH, 5, 25+(place*10), TRANSPARENT_TEXT); // show each card on its own line
        }
    }
    Graphics_flushBuffer(&g_sContext);
}

// plays buzzer sound
void buzzerthing(unsigned char period) {
    setLeds(period);
    BuzzerOn(period*16);
    swDelay(6);
    BuzzerOff(); setLeds(0);
}

// player cuts the deck
unsigned char cutDeck(unsigned char disp) {
    Graphics_drawStringCentered(&g_sContext, "Enter number", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "00-15", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
     unsigned char key;

    while(disp == ' ') {
        key = getKey();
        if ((key>= '0') && (key <= '9')) disp = key;
    }
    swDelay(1);

    return disp;
}

// fill the deck of cards
void initDeck(int deck[]) {
    int i;
    for (i=0; i<52; i++) {  // fill deck
        deck[i] = i;
    }
}

// swaps two cards
void swapCards(int * card1, int * card2) {
    int temp = *card1;
    *card1 = *card2;
    *card2 = temp;
}

// shuffles the elements of an existing array
void shuffle(int deck[], unsigned char disp[]) {
    int i;
    for (i=0; i<2; i++) disp[i] = cutDeck(disp[i]); // set disp[1] and disp[2] to pressed keys

    srand((disp[1]-0x30)*10 + (disp[2]-0x30));
    for(i= 52-1; i>0; i--) {
        int randCard = rand() % (i+1);
        swapCards(&deck[i], &deck[randCard]);
    }
}

// returns suit of a card given deck id
unsigned char *cSuit(int id) {
    unsigned char *cSuits[] = {"H","C","D","S"};
    return cSuits[id%4];
}

// returns face of a card given deck id
unsigned char *cFace(int id) {
    unsigned char *cValues[] = {"A","2","3","4","5","6","7","8","9","1","J","Q","K"};
    return cValues[id%13];
}

// returns value of a card given deck id
unsigned char *cVal(int id) {
    unsigned char *cValues[] = {"1","2","3","4","5","6","7","8","9","X","X","X","X"};
    return cValues[id%13];
}

int value(int place, int hand[]) {    // return numerical value of a hand
    int value=0;
    int count_aces=0;

    for (place; place>0; place--) {
        if (*cVal(hand[place])!='X') value += *cVal(hand[place])-0x30;
        else value += 10;
    }

    for (place; place>0; place--) { // count the aces in a given hand
        if (*cVal(hand[place]) == '1') count_aces++;
    }

    if (count_aces > 0) {   // if there are aces
        for (count_aces; count_aces>0; count_aces--)    // for every ace:
            if (value+10 <= 21) value+=10;              // add 10 to value if it does not go over 21
    }
    return value;
}

// checks that the player enters a valid bet
int betPlayer(int balance) {
    unsigned char p='x';
    int bet, n;
    while(p=='x') {
        p = getKey(); n = balance - (p-0x30);
        if (n <= 0) p='x';
        switch (p) {
            case '1': bet = p-0x30; break;
            case '2': bet = p-0x30; break;
            case '4': bet = p-0x30; break;
            case '8': bet = p-0x30; break;
            default: p='x'; break;
        }
    }
    return bet;
}

int betCpu(int handVal) {
    int bet;

    if (handVal<=21 && handVal>=17) bet = 8;
    else if (handVal<17 && handVal>=13) bet = 4;
    else if (handVal<13 && handVal>=10) bet = 2;
    else bet = 1;

    return bet;
}

void swDelay(char numLoops) {
    volatile unsigned int i,j;  // volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code
    for (j=0; j<numLoops; j++) {
        i = 25000 ;                 // SW Delay
        while (i > 0) i--;          // could also have used while (i)
    }
}
