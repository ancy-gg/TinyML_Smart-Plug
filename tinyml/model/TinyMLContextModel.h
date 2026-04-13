#pragma once
// Auto-generated context family prototype model: ContextPrototype
#define CONTEXT_MODEL_METADATA_VERSION 1
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
// [0] delta_irms_abs
// [1] halfcycle_asymmetry
// [2] suspicious_run_energy
// [3] hf_energy_delta
// [4] midband_residual_ratio
// [5] abs_irms_zscore_vs_baseline
// [6] zcv
// [7] delta_flux
// [8] residual_crest_factor
// [9] peak_fluct_cv

#include <math.h>

static const int context_model_input_feature_ids[CONTEXT_MODEL_INPUT_DIM] = {1, 2, 3, 12, 6, 0, 7, 5, 10, 9};
static const float context_means[10] = {0.264807534f, 15.2688604f, 3.30280775f, 12.8292078f, -40.3441488f, 1.00567546f, 0.820340024f, 6.55661032f, 8.29605186f, 10.3814572f};
static const float context_stds[10] = {0.0998337896f, 6.96023842f, 1.47335342f, 2.64643377f, 10.8863741f, 0.312945734f, 0.388806741f, 1.91922082f, 1.06573405f, 4.02936961f};
static const int context_class_active[CONTEXT_MODEL_FAMILY_COUNT] = {1, 1, 1, 1, 1, 1};
static const float context_centroid_resistive_linear[10] = {-1.06693041f, -0.697359283f, 1.31304898f, -1.45805649f, -1.53564068f, -1.23788205f, -1.10865013f, -0.972134745f, -0.587751593f, -1.6698705f};
static const float context_centroid_inductive_motor[10] = {-1.11648361f, -0.384418959f, -1.70398274f, 0.00940650529f, -0.255990496f, -0.853082508f, -0.837172169f, -1.20632208f, -1.14122638f, 1.24599136f};
static const float context_centroid_rectifier_smps[10] = {-0.0462199229f, 0.523119827f, -0.46012046f, 1.10899221f, 0.841599419f, 0.984208359f, 0.669475364f, 0.56876837f, 0.735494793f, -0.386414597f};
static const float context_centroid_phase_angle_controlled[10] = {1.59596208f, 1.99269354f, 0.173737308f, 1.47938225f, 1.58702758f, 1.69011464f, 1.98343354f, 1.91000984f, 1.93090961f, 0.312297467f};
static const float context_centroid_brush_universal_motor[10] = {0.787510153f, -1.40996592f, 0.920822382f, -0.708636278f, -0.719862619f, -0.332729197f, -0.41082456f, 0.00271875158f, -0.748972356f, 0.847637037f};
static const float context_centroid_other_mixed[10] = {0.442029764f, 0.121612701f, 0.0301344457f, -0.238401704f, 0.299658041f, 0.088717119f, 0.0968901991f, 0.17514201f, 0.107030241f, -0.0596571408f};

static inline void context_family_predict(double *input, double *output) {
    const float *centroids[] = {context_centroid_resistive_linear, context_centroid_inductive_motor, context_centroid_rectifier_smps, context_centroid_phase_angle_controlled, context_centroid_brush_universal_motor, context_centroid_other_mixed};
    float logits[6];
    float max_logit = -1e30f;
    for (int c = 0; c < 6; ++c) {
        if (!context_class_active[c]) { logits[c] = -1.0e9f; continue; }
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
