#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h> // Required to use Arduino types like 'float' or 'pinMode'

// Configuration Constants
const int STATUS_LED = 2; // Built-in LED usually

// Function Declarations (The "Menu")
void initMonitor();
float readSystemVoltage();
void blinkStatus(int delayTime);

#endif