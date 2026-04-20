#pragma once
// Auto-generated context family prototype model: ContextPrototype
#define CONTEXT_MODEL_METADATA_VERSION 1
#define CONTEXT_MODEL_INPUT_DIM 6
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
// [1] midband_residual_ratio
// [2] abs_irms_zscore_vs_baseline
// [3] residual_crest_factor
// [4] thd_i
// [5] spectral_flux_midhf

#include <math.h>

static const int context_model_input_feature_ids[CONTEXT_MODEL_INPUT_DIM] = {1, 6, 0, 10, 11, 8};
static const float context_means[6] = {0.183199571f, -40.755909f, 1.813028f, 7.72716619f, 36.1116107f, 33.4589692f};
static const float context_stds[6] = {0.142295798f, 15.4806048f, 1.54649379f, 0.959818877f, 27.8521169f, 15.256421f};
static const int context_class_active[CONTEXT_MODEL_FAMILY_COUNT] = {1, 1, 1, 1, 1, 1};
static const float context_centroid_resistive_linear[6] = {0.87274821f, -0.853454504f, -0.413807496f, -0.349604304f, -0.567257213f, -0.660235043f};
static const float context_centroid_inductive_motor[6] = {-1.08688998f, -0.145479451f, -0.0262636887f, -0.538875758f, -0.529253289f, -0.237521862f};
static const float context_centroid_rectifier_smps[6] = {-1.01900294f, 0.689129037f, -0.0939697619f, 0.821675383f, 0.788000673f, 0.180119795f};
static const float context_centroid_phase_angle_controlled[6] = {-1.02891878f, 2.30648772f, -0.456552253f, 1.93272376f, 2.54608708f, 2.66161867f};
static const float context_centroid_brush_universal_motor[6] = {0.657250148f, -0.356350326f, -0.402900189f, -0.795545502f, -0.392530476f, -0.200409687f};
static const float context_centroid_other_mixed[6] = {0.302859866f, 0.449075638f, 2.39190061f, 0.0311527124f, -0.201095531f, -0.0410362782f};

static inline void context_family_predict(double *input, double *output) {
    const float *centroids[] = {context_centroid_resistive_linear, context_centroid_inductive_motor, context_centroid_rectifier_smps, context_centroid_phase_angle_controlled, context_centroid_brush_universal_motor, context_centroid_other_mixed};
    float logits[6];
    float max_logit = -1e30f;
    for (int c = 0; c < 6; ++c) {
        if (!context_class_active[c]) { logits[c] = -1.0e9f; continue; }
        float d2 = 0.0f;
        for (int i = 0; i < 6; ++i) {
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
