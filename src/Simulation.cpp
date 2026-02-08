#include "Simulation.h"

SimData Simulation::getCycleData() {
    SimData data;
    int cyclePhase = (millis() / 6000) % 4;

    // Base random noise
    float noise = random(-10, 10) / 100.0; 

    switch (cyclePhase) {
        case 0: // NORMAL
            data.voltage = 220.5 + noise; 
            data.current = 5.2 + noise; 
            data.temp = 45.0; 
            data.zcv = 0.05;  // Low variance
            data.thd = 2.5;   // Low distortion
            data.entropy = 0.1; 
            data.tinyMLOutput = 0; 
            break;
        case 1: // OVERLOAD
            data.voltage = 218.0; 
            data.current = 15.5; 
            data.temp = 55.0; 
            data.zcv = 0.08; 
            data.thd = 5.0; 
            data.entropy = 0.3; 
            data.tinyMLOutput = 0; 
            break;
        case 2: // HEATING
            data.voltage = 220.0; 
            data.current = 8.0; 
            data.temp = 85.5; 
            data.zcv = 0.05; 
            data.thd = 3.0; 
            data.entropy = 0.2; 
            data.tinyMLOutput = 0; 
            break;
        case 3: // ARCING (Chaos!)
            data.voltage = 215.0 + (random(-50, 50)/10.0); 
            data.current = 2.0 + (random(-10, 10)/10.0); 
            data.temp = 60.0; 
            data.zcv = 12.5;  // High Zero Crossing Variance
            data.thd = 25.0;  // High Distortion
            data.entropy = 0.95; // High Entropy
            data.tinyMLOutput = 1; 
            break;
    }
    return data;
}