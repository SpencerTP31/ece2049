# ECE 2049 Lab 4: "Who's Watching the Watchers?"
SPI DAC vs built-in ADC

Written by: Tai Kjendal, Colleen Chapman\
Last change: March 3, 2020

Bugs:
* Target triangle freq range is 100 Hz to 1 KHz. However, the board outputs a maximum ~300 Hz triangle wave using the scroll to change the period. Setting the period directly yielded ~800 Hz wave, despite all efforts and calculations. The resolution is as low as possible, and both ACLK and SMCLK have been tried.