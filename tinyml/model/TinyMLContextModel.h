#pragma once
// Auto-generated context family prototype model: ContextPrototype
#define CONTEXT_MODEL_METADATA_VERSION 1
#define CONTEXT_MODEL_INPUT_DIM 8
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
// [0] delta_irms_abs
// [1] halfcycle_asymmetry
// [2] suspicious_run_energy
// [3] hf_energy_delta
// [4] midband_residual_ratio
// [5] abs_irms_zscore_vs_baseline
// [6] zcv
// [7] peak_fluct_cv

#include <math.h>

// WARNING: this exported context model has incomplete class coverage.
// Near-zero centroids: rectifier_smps, phase_angle_controlled, other_mixed

static const int context_model_input_feature_ids[CONTEXT_MODEL_INPUT_DIM] = {1, 2, 3, 12, 6, 0, 7, 9};
static const float context_means[8] = {0.393226049f, 6.20365597f, 2.33094507f, 11.6816198f, -52.816785f, 0.950273543f, 0.521364987f, 0.175742457f};
static const float context_stds[8] = {0.243957761f, 3.41903448f, 1.30900095f, 2.05543813f, 3.74323264f, 0.371242958f, 0.103160022f, 0.159839282f};
static const int context_class_active[CONTEXT_MODEL_FAMILY_COUNT] = {1, 1, 0, 0, 1, 0};
static const float context_centroid_resistive_linear[8] = {-0.805723406f, 1.06347977f, 0.933105219f, -0.918070557f, -1.69621179f, -1.45970558f, -0.531029773f, -0.895036356f};
static const float context_centroid_inductive_motor[8] = {-1.35921432f, 1.12131065f, -1.78070541f, 1.7697697f, 0.171633717f, -0.117190022f, -1.23269836f, 1.17481743f};
static const float context_centroid_rectifier_smps[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static const float context_centroid_phase_angle_controlled[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static const float context_centroid_brush_universal_motor[8] = {0.884313489f, -0.89285272f, 0.344075486f, -0.345772991f, 0.624684025f, 0.645614577f, 0.720215828f, -0.112566365f};
static const float context_centroid_other_mixed[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

static inline void context_family_predict(double *input, double *output) {
    const float *centroids[] = {context_centroid_resistive_linear, context_centroid_inductive_motor, context_centroid_rectifier_smps, context_centroid_phase_angle_controlled, context_centroid_brush_universal_motor, context_centroid_other_mixed};
    float logits[6];
    float max_logit = -1e30f;
    for (int c = 0; c < 6; ++c) {
        if (!context_class_active[c]) { logits[c] = -1.0e9f; continue; }
        float d2 = 0.0f;
        for (int i = 0; i < 8; ++i) {
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
