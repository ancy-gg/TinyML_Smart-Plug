#pragma once
#include <Arduino.h>
#include "SmartPlugConfig.h"


#ifdef USE_M2CGEN_RF
  #include "TinyML_RF.h" 
#endif

// Defaults
#ifndef ARC_THRESHOLD
  #define ARC_THRESHOLD 0.50f
#endif

// Predeiction Logic
static inline int ArcPredict(float entropy, float thd_pct, float zcv,
                             float v_rms, float i_rms, float temp_c) {

#ifdef DATA_COLLECTION_MODE
// Do not switch off the relay during data collection
    return 0; 
#endif

#ifdef USE_M2CGEN_RF
    // TinyML Model
    double input_features[6] = {
        (double)entropy, (double)thd_pct, (double)zcv, 
        (double)v_rms,   (double)i_rms,   (double)temp_c
    };
    double output_probs[2]; 

    arc_rf_predict(input_features, output_probs);

    if (output_probs[1] >= ARC_THRESHOLD) {
        return 1; // Arc Detected
    }
    return 0;

#else
    // Testing without TinyML
    (void)v_rms; (void)i_rms; (void)temp_c; 
    
    if (entropy > ENTROPY_ARC_H) return 1;
    if (thd_pct > THD_ARC_H_PCT && zcv > ZCV_ARC_H_MS) return 1;
    
    return 0;
#endif
}