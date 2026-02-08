#ifndef SIMULATION_H
#define SIMULATION_H

#include <Arduino.h>

struct SimData {
    float voltage;
    float current;
    float temp;
    float zcv;      // Zero Crossing Variance
    float thd;      // Total Harmonic Distortion
    float entropy;  // Signal Entropy
    int tinyMLOutput;
};

class Simulation {
public:
    SimData getCycleData();
};

#endif