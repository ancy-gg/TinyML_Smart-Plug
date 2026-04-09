#pragma once
// Auto-generated context family prototype model: ContextPrototype
#define CONTEXT_MODEL_INPUT_DIM 10
#define CONTEXT_MODEL_FAMILY_COUNT 6
#define CONTEXT_UNKNOWN_CONFIDENCE 0.4500f

#define CONTEXT_FAMILY_UNKNOWN -1
#define CONTEXT_FAMILY_RESISTIVE_LINEAR 0
#define CONTEXT_FAMILY_INDUCTIVE_MOTOR 1
#define CONTEXT_FAMILY_RECTIFIER_SMPS 2
#define CONTEXT_FAMILY_PHASE_ANGLE_CONTROLLED 3
#define CONTEXT_FAMILY_BRUSH_UNIVERSAL_MOTOR 4
#define CONTEXT_FAMILY_OTHER_MIXED 5

// Input Feature Order:
// [0] residual_crest_factor
// [1] edge_spike_ratio
// [2] midband_residual_ratio
// [3] thd_i
// [4] hf_energy_delta
// [5] zcv
// [6] i_rms
// [7] v_rms
// [8] cycle_nmse
// [9] peak_fluct_cv

#include <math.h>

static const float context_means[10] = {9.12299821f, -7.51110088f, -55.2594051f, 150.277315f, 16.1726849f, 0.78515333f, 5.30509452f, 226.4254f, 143.348094f, 41.4644565f};
static const float context_stds[10] = {0.936559326f, 1.22236958f, 3.51810232f, 30.0490874f, 0.343351154f, 0.0968095646f, 0.492092235f, 1.00494707f, 6.89873087f, 36.1269405f};
static const float context_centroid_resistive_linear[10] = {0.719761914f, 0.174162904f, -0.97370564f, -0.997498437f, -0.932753076f, -0.953921809f, 0.99114192f, 0.273492017f, -0.61890996f, -0.94164658f};
static const float context_centroid_inductive_motor[10] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static const float context_centroid_rectifier_smps[10] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static const float context_centroid_phase_angle_controlled[10] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static const float context_centroid_brush_universal_motor[10] = {-0.719761914f, -0.174162904f, 0.97370564f, 0.997498437f, 0.932753076f, 0.953921809f, -0.99114192f, -0.273492017f, 0.61890996f, 0.94164658f};
static const float context_centroid_other_mixed[10] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

static inline void context_family_predict(double *input, double *output) {
    const float *centroids[] = {context_centroid_resistive_linear, context_centroid_inductive_motor, context_centroid_rectifier_smps, context_centroid_phase_angle_controlled, context_centroid_brush_universal_motor, context_centroid_other_mixed};
    float logits[6];
    float max_logit = -1e30f;
    for (int c = 0; c < 6; ++c) {
        float d2 = 0.0f;
        for (int i = 0; i < 10; ++i) {
            const float z = (((float)input[i]) - context_means[i]) / context_stds[i];
            const float d = z - centroids[c][i];
            d2 += d * d;
        }
        logits[c] = -d2;
        if (logits[c] > max_logit) max_logit = logits[c];
    }
    float sum = 0.0f;
    for (int c = 0; c < 6; ++c) {
        logits[c] = expf(logits[c] - max_logit);
        sum += logits[c];
    }
    if (sum <= 1e-12f) sum = 1.0f;
    for (int c = 0; c < 6; ++c) output[c] = (double)(logits[c] / sum);
}

static inline int context_family_best(double *output, double *confidence) {
    int best = 0;
    double best_v = output[0];
    for (int c = 1; c < 6; ++c) { if (output[c] > best_v) { best_v = output[c]; best = c; } }
    if (confidence) *confidence = best_v;
    return (best_v < 0.4500) ? CONTEXT_FAMILY_UNKNOWN : best;
}
