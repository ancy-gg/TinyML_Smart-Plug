#include "Simulation.h"

// --- Tune these thresholds anytime ---
static const float ENTROPY_HIGH = 0.80f;   // arc if entropy >= this
static const float THD_HIGH     = 10.0f;   // "high THD"
static const float ZCV_HIGH     = 5.0f;    // "high ZCV"

// Apply your TinyML decision rule:
// 1) If entropy is high -> arc
// 2) Else if BOTH thd and zcv are high -> arc
// 3) Else -> no arc
static int inferArc(float entropy, float thd, float zcv) {
    const bool entropyHigh = (entropy >= ENTROPY_HIGH);
    const bool thdHigh     = (thd >= THD_HIGH);
    const bool zcvHigh     = (zcv >= ZCV_HIGH);

    if (entropyHigh) return 1;
    if (thdHigh && zcvHigh) return 1;
    return 0;
}

SimData Simulation::getCycleData() {
    SimData data;

    // 4 phases, each lasts 6 seconds
    int cyclePhase = (millis() / 6000) % 4;

    // Base random noise (small)
    float noise = random(-10, 10) / 100.0f;

    switch (cyclePhase) {
        case 0: // NORMAL
            data.voltage  = 220.5f + noise;
            data.current  = 5.2f + noise;
            data.temp     = 45.0f;

            data.zcv      = 0.05f;  // low
            data.thd      = 2.5f;   // low
            data.entropy  = 0.10f;  // low
            break;

        case 1: // OVERLOAD (high current, but NOT arc)
            data.voltage  = 218.0f + noise;
            data.current  = 15.5f + noise;
            data.temp     = 55.0f;

            // keep arc features not-high
            data.zcv      = 0.08f;
            data.thd      = 5.0f;
            data.entropy  = 0.30f;
            break;

        case 2: // HEATING (high temp, but NOT arc)
            data.voltage  = 220.0f + noise;
            data.current  = 8.0f + noise;
            data.temp     = 85.5f;

            data.zcv      = 0.05f;
            data.thd      = 3.0f;
            data.entropy  = 0.20f;
            break;

        case 3: // ARCING-like signal (chaotic)
            data.voltage  = 215.0f + (random(-50, 50) / 10.0f);
            data.current  = 2.0f + (random(-10, 10) / 10.0f);
            data.temp     = 60.0f;

            // Choose values that satisfy your arc rule:
            // Example: entropy high (auto arc), plus high THD and high ZCV
            data.zcv      = 12.5f;   // high
            data.thd      = 25.0f;   // high
            data.entropy  = 0.95f;   // high
            break;
    }

    // Apply your TinyML output rule AFTER features are set
    data.tinyMLOutput = inferArc(data.entropy, data.thd, data.zcv);

    return data;
}