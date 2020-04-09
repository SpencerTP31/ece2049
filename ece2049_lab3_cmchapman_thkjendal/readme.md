# ECE 2049 Lab 3: "Time and Temperature Display"
Measure and display time and temperature on an embedded system\
Board: Texas Instruments MSP430F5529

Written by: Tai Kjendal, Colleen Chapman\
Last edit:  February 24, 2020

This program measures tempurature using the board's built in thermistor and ADC, and reads values once per second. Time is counted using TimerA2 (derived from ACLK) with a 1 second resolution. Time is displayed, along with the temperature on an LCD. The time can be adjusted with the analog scroll wheel.
