#pragma once
// Auto-generated C header from scikit-learn RandomForest (native_tree_ensemble)
#define ARC_MODEL_FEATURE_VERSION 5
#define ARC_MODEL_INPUT_DIM 17
#define ARC_THRESHOLD 0.1700f
#define ARC_CONTEXT_CONFIDENCE_MIN 0.4500f
#define ARC_THRESHOLD_UNKNOWN 0.6500f
#define ARC_UNKNOWN_MIN_FEATURE_VOTES 3

// Input Feature Order:
// [0] spectral_flux_midhf
// [1] residual_crest_factor
// [2] edge_spike_ratio
// [3] midband_residual_ratio
// [4] cycle_nmse
// [5] peak_fluct_cv
// [6] thd_i
// [7] hf_energy_delta
// [8] zcv
// [9] abs_irms_zscore_vs_baseline
// [10] ctx_family_resistive_linear
// [11] ctx_family_inductive_motor
// [12] ctx_family_rectifier_smps
// [13] ctx_family_phase_angle_controlled
// [14] ctx_family_brush_universal_motor
// [15] ctx_family_other_mixed
// [16] context_family_confidence

#include <string.h>
static const float arc_family_thresholds[6] = {0.17f, 0.17f, 0.17f, 0.17f, 0.17f, 0.17f};
static inline float arc_context_threshold_for_family(int family, float confidence) {
    if (family >= 0 && family < (int)(sizeof(arc_family_thresholds)/sizeof(arc_family_thresholds[0])) && confidence >= ARC_CONTEXT_CONFIDENCE_MIN) return arc_family_thresholds[family];
    return ARC_THRESHOLD_UNKNOWN;
}

#include <stdint.h>

static inline float arc_tree_predict_proba_pos(
    const int16_t *left,
    const int16_t *right,
    const int16_t *feature,
    const float *threshold,
    const float *leaf_pos,
    const double *input
) {
    int node = 0;
    while (left[node] != -1) {
        const int feat = feature[node];
        node = (((float)input[feat]) <= threshold[node]) ? left[node] : right[node];
    }
    return leaf_pos[node];
}

static const int16_t tree_0_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_0_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_0_feature[11] = {9, -2, 9, 4, -2, 8, 1, -2, -2, -2, -2};
static const float tree_0_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_0_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0800395257f, 1.0f};

static const int16_t tree_1_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_1_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_1_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_1_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_1_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_2_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_2_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_2_feature[11] = {9, -2, 4, 9, -2, -2, 8, 7, -2, -2, -2};
static const float tree_2_threshold[11] = {1.40355951f, -2.0f, 70.9573505f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, 15.3554201f, -2.0f, -2.0f, -2.0f};
static const float tree_2_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_3_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_3_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_3_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_3_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_3_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_4_left[15] = {1, 2, -1, 4, 5, -1, -1, 8, 9, -1, -1, -1, 13, -1, -1};
static const int16_t tree_4_right[15] = {12, 3, -1, 7, 6, -1, -1, 11, 10, -1, -1, -1, 14, -1, -1};
static const int16_t tree_4_feature[15] = {2, 9, -2, 4, 3, -2, -2, 8, 1, -2, -2, -2, 3, -2, -2};
static const float tree_4_threshold[15] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, -41.1395645f, -2.0f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_4_leaf_pos[15] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.148215919f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_5_left[9] = {1, -1, 3, 4, -1, 6, -1, -1, -1};
static const int16_t tree_5_right[9] = {2, -1, 8, 5, -1, 7, -1, -1, -1};
static const int16_t tree_5_feature[9] = {9, -2, 9, 5, -2, 8, -2, -2, -2};
static const float tree_5_threshold[9] = {1.40355951f, -2.0f, 6.27725458f, 5.53060007f, -2.0f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_5_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_6_left[13] = {1, -1, 3, 4, -1, -1, 7, 8, -1, 10, -1, -1, -1};
static const int16_t tree_6_right[13] = {2, -1, 6, 5, -1, -1, 12, 9, -1, 11, -1, -1, -1};
static const int16_t tree_6_feature[13] = {9, -2, 4, 9, -2, -2, 9, 4, -2, 8, -2, -2, -2};
static const float tree_6_threshold[13] = {1.40355951f, -2.0f, 70.9573505f, 6.27725458f, -2.0f, -2.0f, 2.44868445f, 180.835899f, -2.0f, 0.800971985f, -2.0f, -2.0f, -2.0f};
static const float tree_6_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.11543943f, 1.0f};

static const int16_t tree_7_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_7_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_7_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_7_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_7_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_8_left[15] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, 12, -1, -1, -1};
static const int16_t tree_8_right[15] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 14, 13, -1, -1, -1};
static const int16_t tree_8_feature[15] = {9, -2, 5, 3, 1, 2, -2, -2, -2, -2, 2, 5, -2, -2, -2};
static const float tree_8_threshold[15] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, -13.0282044f, -2.0f, -2.0f, -2.0f, -2.0f, -4.31501508f, 9.74874973f, -2.0f, -2.0f, -2.0f};
static const float tree_8_leaf_pos[15] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.258167331f, 1.0f};

static const int16_t tree_9_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_9_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_9_feature[11] = {9, -2, 5, 3, -2, -2, 9, 8, -2, -2, -2};
static const float tree_9_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 2.44868445f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_9_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_10_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_10_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_10_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_10_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_10_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_11_left[19] = {1, 2, 3, -1, -1, 6, -1, 8, 9, 10, 11, -1, -1, -1, -1, -1, 17, -1, -1};
static const int16_t tree_11_right[19] = {16, 5, 4, -1, -1, 7, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, 18, -1, -1};
static const int16_t tree_11_feature[19] = {2, 5, 7, -2, -2, 9, -2, 8, 3, 9, 5, -2, -2, -2, -2, -2, 8, -2, -2};
static const float tree_11_threshold[19] = {-0.910058975f, 0.00270000007f, 16.3341608f, -2.0f, -2.0f, 1.40355951f, -2.0f, 1.39401704f, -56.7690601f, 2.03987408f, 109.480848f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, 0.324927986f, -2.0f, -2.0f};
static const float tree_11_leaf_pos[19] = {0.0f, 0.0f, 0.0f, 0.0891525009f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_12_left[9] = {1, 2, 3, -1, -1, -1, 7, -1, -1};
static const int16_t tree_12_right[9] = {6, 5, 4, -1, -1, -1, 8, -1, -1};
static const int16_t tree_12_feature[9] = {2, 4, 7, -2, -2, -2, 3, -2, -2};
static const float tree_12_threshold[9] = {-0.910058975f, 0.0219999999f, 16.3341608f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_12_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0891525009f, 1.0f, 0.00502416295f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_13_left[13] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, -1, -1};
static const int16_t tree_13_right[13] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 12, -1, -1};
static const int16_t tree_13_feature[13] = {9, -2, 5, 3, 1, 6, -2, -2, -2, -2, 8, -2, -2};
static const float tree_13_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, 116.624443f, -2.0f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_13_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_14_left[13] = {1, -1, 3, 4, -1, -1, 7, 8, 9, -1, -1, -1, -1};
static const int16_t tree_14_right[13] = {2, -1, 6, 5, -1, -1, 12, 11, 10, -1, -1, -1, -1};
static const int16_t tree_14_feature[13] = {9, -2, 5, 3, -2, -2, 9, 2, 8, -2, -2, -2, -2};
static const float tree_14_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 2.44868445f, -4.31501508f, 1.08684501f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_14_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f, 1.0f};

static const int16_t tree_15_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_15_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_15_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_15_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_15_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_16_left[15] = {1, 2, -1, 4, 5, -1, 7, -1, 9, -1, -1, -1, 13, -1, -1};
static const int16_t tree_16_right[15] = {12, 3, -1, 11, 6, -1, 8, -1, 10, -1, -1, -1, 14, -1, -1};
static const int16_t tree_16_feature[15] = {2, 9, -2, 9, 5, -2, 6, -2, 8, -2, -2, -2, 1, -2, -2};
static const float tree_16_threshold[15] = {-0.910058975f, 1.40355951f, -2.0f, 6.27725458f, 5.53060007f, -2.0f, 163.100952f, -2.0f, 1.19457251f, -2.0f, -2.0f, -2.0f, 2.78808141f, -2.0f, -2.0f};
static const float tree_16_leaf_pos[15] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.11543943f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_17_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_17_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_17_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_17_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_17_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_18_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_18_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_18_feature[9] = {2, 9, -2, 16, -2, -2, 8, -2, -2};
static const float tree_18_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, 0.324927986f, -2.0f, -2.0f};
static const float tree_18_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_19_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_19_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_19_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_19_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_19_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_20_left[19] = {1, 2, 3, -1, -1, 6, -1, 8, 9, 10, 11, -1, -1, -1, -1, -1, 17, -1, -1};
static const int16_t tree_20_right[19] = {16, 5, 4, -1, -1, 7, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, 18, -1, -1};
static const int16_t tree_20_feature[19] = {2, 5, 7, -2, -2, 9, -2, 10, 9, 8, 5, -2, -2, -2, -2, -2, 4, -2, -2};
static const float tree_20_threshold[19] = {-0.910058975f, 0.00270000007f, 16.3341608f, -2.0f, -2.0f, 1.40355951f, -2.0f, 0.5f, 5.81565142f, 1.19457251f, 5.53060007f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, 100.0f, -2.0f, -2.0f};
static const float tree_20_leaf_pos[19] = {0.0f, 0.0f, 0.0f, 0.0891525009f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0865384615f, 1.0f, 0.0800395257f, 1.0f, 0.752788104f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_21_left[15] = {1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, -1, 13, -1, -1};
static const int16_t tree_21_right[15] = {12, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, -1, 14, -1, -1};
static const int16_t tree_21_feature[15] = {2, 9, -2, 16, -2, 2, -2, 1, -2, 5, -2, -2, 6, -2, -2};
static const float tree_21_threshold[15] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -13.0282044f, -2.0f, 8.74839973f, -2.0f, 6.03630018f, -2.0f, -2.0f, 150.0f, -2.0f, -2.0f};
static const float tree_21_leaf_pos[15] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.101351351f, 0.730223124f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_22_left[11] = {1, -1, 3, 4, 5, -1, -1, -1, 9, -1, -1};
static const int16_t tree_22_right[11] = {2, -1, 8, 7, 6, -1, -1, -1, 10, -1, -1};
static const int16_t tree_22_feature[11] = {9, -2, 5, 1, 6, -2, -2, -2, 8, -2, -2};
static const float tree_22_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, 8.08057356f, 116.624443f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_22_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0416881112f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_23_left[9] = {1, 2, 3, -1, -1, -1, 7, -1, -1};
static const int16_t tree_23_right[9] = {6, 5, 4, -1, -1, -1, 8, -1, -1};
static const int16_t tree_23_feature[9] = {2, 5, 7, -2, -2, -2, 3, -2, -2};
static const float tree_23_threshold[9] = {-0.910058975f, 0.00270000007f, 16.3341608f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_23_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0891525009f, 1.0f, 0.00502416295f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_24_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_24_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_24_feature[11] = {9, -2, 5, 9, -2, -2, 2, 8, -2, -2, -2};
static const float tree_24_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, -4.31501508f, 1.188972f, -2.0f, -2.0f, -2.0f};
static const float tree_24_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 1.0f};

static const int16_t tree_25_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_25_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_25_feature[11] = {9, -2, 9, 4, -2, 8, 7, -2, -2, -2, -2};
static const float tree_25_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 15.4879923f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_25_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_26_left[13] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, -1, -1};
static const int16_t tree_26_right[13] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 12, -1, -1};
static const int16_t tree_26_feature[13] = {9, -2, 5, 3, 1, 6, -2, -2, -2, -2, 8, -2, -2};
static const float tree_26_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, 116.624443f, -2.0f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_26_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_27_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_27_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_27_feature[9] = {2, 9, -2, 16, -2, -2, 3, -2, -2};
static const float tree_27_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_27_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_28_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_28_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_28_feature[11] = {9, -2, 5, 3, -2, -2, 2, 8, -2, -2, -2};
static const float tree_28_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, -4.31501508f, 1.188972f, -2.0f, -2.0f, -2.0f};
static const float tree_28_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 1.0f};

static const int16_t tree_29_left[15] = {1, -1, 3, 4, -1, -1, 7, 8, -1, 10, 11, -1, -1, -1, -1};
static const int16_t tree_29_right[15] = {2, -1, 6, 5, -1, -1, 14, 9, -1, 13, 12, -1, -1, -1, -1};
static const int16_t tree_29_feature[15] = {9, -2, 5, 9, -2, -2, 9, 4, -2, 2, 8, -2, -2, -2, -2};
static const float tree_29_threshold[15] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 2.44868445f, 180.835899f, -2.0f, -4.31501508f, 1.004848f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_29_leaf_pos[15] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_30_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_30_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_30_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_30_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_30_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_31_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_31_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_31_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_31_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_31_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_32_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_32_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_32_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_32_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_32_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_33_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_33_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_33_feature[9] = {2, 9, -2, 16, -2, -2, 3, -2, -2};
static const float tree_33_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_33_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_34_left[11] = {1, -1, 3, 4, 5, -1, -1, 8, -1, -1, -1};
static const int16_t tree_34_right[11] = {2, -1, 10, 7, 6, -1, -1, 9, -1, -1, -1};
static const int16_t tree_34_feature[11] = {9, -2, 9, 10, 7, -2, -2, 5, -2, -2, -2};
static const float tree_34_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 0.5f, 16.8099537f, -2.0f, -2.0f, 5.53060007f, -2.0f, -2.0f, -2.0f};
static const float tree_34_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.141253601f, 1.0f, 0.0f, 0.310880829f, 1.0f, 1.0f};

static const int16_t tree_35_left[13] = {1, 2, -1, -1, 5, -1, 7, -1, 9, 10, -1, -1, -1};
static const int16_t tree_35_right[13] = {4, 3, -1, -1, 6, -1, 8, -1, 12, 11, -1, -1, -1};
static const int16_t tree_35_feature[13] = {5, 7, -2, -2, 9, -2, 2, -2, 8, 7, -2, -2, -2};
static const float tree_35_threshold[13] = {0.00270000007f, 16.3341608f, -2.0f, -2.0f, 1.40355951f, -2.0f, -12.8594089f, -2.0f, 1.39401704f, 15.3554201f, -2.0f, -2.0f, -2.0f};
static const float tree_35_leaf_pos[13] = {0.0f, 0.0f, 0.0735501836f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_36_left[13] = {1, -1, 3, 4, -1, -1, 7, 8, 9, -1, -1, -1, -1};
static const int16_t tree_36_right[13] = {2, -1, 6, 5, -1, -1, 12, 11, 10, -1, -1, -1, -1};
static const int16_t tree_36_feature[13] = {9, -2, 5, 9, -2, -2, 2, 9, 8, -2, -2, -2, -2};
static const float tree_36_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, -4.31501508f, 2.44868445f, 1.08684501f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_36_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f, 1.0f};

static const int16_t tree_37_left[11] = {1, -1, 3, -1, 5, 6, 7, -1, -1, -1, -1};
static const int16_t tree_37_right[11] = {2, -1, 4, -1, 10, 9, 8, -1, -1, -1, -1};
static const int16_t tree_37_feature[11] = {4, -2, 9, -2, 8, 6, 5, -2, -2, -2, -2};
static const float tree_37_threshold[11] = {0.0219999999f, -2.0f, 1.40355951f, -2.0f, 1.22002703f, 110.147877f, 6.03630018f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_37_leaf_pos[11] = {0.0f, 0.137022348f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.148215919f};

static const int16_t tree_38_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_38_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_38_feature[11] = {9, -2, 4, 3, -2, -2, 8, 3, -2, -2, -2};
static const float tree_38_threshold[11] = {1.40355951f, -2.0f, 70.9573505f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -62.7468605f, -2.0f, -2.0f, -2.0f};
static const float tree_38_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 0.206984668f, 1.0f, 0.148215919f};

static const int16_t tree_39_left[13] = {1, 2, -1, -1, 5, -1, 7, -1, 9, 10, -1, -1, -1};
static const int16_t tree_39_right[13] = {4, 3, -1, -1, 6, -1, 8, -1, 12, 11, -1, -1, -1};
static const int16_t tree_39_feature[13] = {4, 7, -2, -2, 9, -2, 2, -2, 8, 7, -2, -2, -2};
static const float tree_39_threshold[13] = {0.0219999999f, 16.3341608f, -2.0f, -2.0f, 1.40355951f, -2.0f, -12.8594089f, -2.0f, 1.39401704f, 15.3554201f, -2.0f, -2.0f, -2.0f};
static const float tree_39_leaf_pos[13] = {0.0f, 0.0f, 0.0735501836f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_40_left[11] = {1, -1, 3, -1, 5, -1, 7, -1, 9, -1, -1};
static const int16_t tree_40_right[11] = {2, -1, 4, -1, 6, -1, 8, -1, 10, -1, -1};
static const int16_t tree_40_feature[11] = {9, -2, 16, -2, 2, -2, 8, -2, 1, -2, -2};
static const float tree_40_threshold[11] = {1.40355951f, -2.0f, 0.5f, -2.0f, -13.0282044f, -2.0f, 0.992301494f, -2.0f, 10.0206833f, -2.0f, -2.0f};
static const float tree_40_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.171292625f, 0.0f, 0.0f, 0.0f, 0.802377415f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_41_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_41_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_41_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_41_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_41_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_42_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_42_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_42_feature[11] = {9, -2, 5, 3, -2, -2, 9, 8, -2, -2, -2};
static const float tree_42_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 2.44868445f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_42_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_43_left[17] = {1, 2, -1, 4, 5, -1, -1, 8, 9, -1, 11, -1, -1, -1, 15, -1, -1};
static const int16_t tree_43_right[17] = {14, 3, -1, 7, 6, -1, -1, 13, 10, -1, 12, -1, -1, -1, 16, -1, -1};
static const int16_t tree_43_feature[17] = {2, 9, -2, 4, 9, -2, -2, 9, 8, -2, 4, -2, -2, -2, 3, -2, -2};
static const float tree_43_threshold[17] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, 6.27725458f, -2.0f, -2.0f, 2.44868445f, 0.800971985f, -2.0f, 180.835899f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_43_leaf_pos[17] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_44_left[5] = {1, -1, 3, -1, -1};
static const int16_t tree_44_right[5] = {2, -1, 4, -1, -1};
static const int16_t tree_44_feature[5] = {2, -2, 3, -2, -2};
static const float tree_44_threshold[5] = {-0.910058975f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_44_leaf_pos[5] = {0.0f, 0.00545908475f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_45_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_45_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_45_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_45_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_45_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_46_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_46_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_46_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_46_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_46_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_47_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_47_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_47_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_47_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_47_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_48_left[13] = {1, -1, 3, 4, -1, 6, 7, -1, 9, -1, -1, -1, -1};
static const int16_t tree_48_right[13] = {2, -1, 12, 5, -1, 11, 8, -1, 10, -1, -1, -1, -1};
static const int16_t tree_48_feature[13] = {9, -2, 9, 4, -2, 9, 8, -2, 4, -2, -2, -2, -2};
static const float tree_48_threshold[13] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 2.44868445f, 0.800971985f, -2.0f, 180.835899f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_48_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.11543943f, 1.0f, 1.0f};

static const int16_t tree_49_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_49_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_49_feature[9] = {2, 9, -2, 16, -2, -2, 1, -2, -2};
static const float tree_49_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, 2.78808141f, -2.0f, -2.0f};
static const float tree_49_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_50_left[17] = {1, 2, -1, -1, 5, 6, -1, 8, 9, 10, 11, -1, -1, -1, -1, -1, -1};
static const int16_t tree_50_right[17] = {4, 3, -1, -1, 16, 7, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1};
static const int16_t tree_50_feature[17] = {5, 7, -2, -2, 2, 9, -2, 8, 3, 9, 2, -2, -2, -2, -2, -2, -2};
static const float tree_50_threshold[17] = {0.00270000007f, 16.3341608f, -2.0f, -2.0f, -0.910058975f, 1.40355951f, -2.0f, 1.39401704f, -56.7690601f, 2.03987408f, -13.8799596f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_50_leaf_pos[17] = {0.0f, 0.0f, 0.0735501836f, 1.0f, 0.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};

static const int16_t tree_51_left[13] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, -1, -1};
static const int16_t tree_51_right[13] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 12, -1, -1};
static const int16_t tree_51_feature[13] = {9, -2, 5, 3, 1, 2, -2, -2, -2, -2, 8, -2, -2};
static const float tree_51_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, -13.0282044f, -2.0f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_51_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_52_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_52_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_52_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_52_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_52_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_53_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_53_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_53_feature[9] = {2, 9, -2, 16, -2, -2, 3, -2, -2};
static const float tree_53_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_53_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_54_left[9] = {1, -1, 3, 4, -1, 6, -1, -1, -1};
static const int16_t tree_54_right[9] = {2, -1, 8, 5, -1, 7, -1, -1, -1};
static const int16_t tree_54_feature[9] = {9, -2, 9, 5, -2, 8, -2, -2, -2};
static const float tree_54_threshold[9] = {1.40355951f, -2.0f, 6.27725458f, 5.53060007f, -2.0f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_54_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_55_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_55_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_55_feature[11] = {9, -2, 9, 4, -2, 8, 1, -2, -2, -2, -2};
static const float tree_55_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_55_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0800395257f, 1.0f};

static const int16_t tree_56_left[15] = {1, 2, -1, 4, 5, -1, -1, 8, 9, -1, -1, -1, 13, -1, -1};
static const int16_t tree_56_right[15] = {12, 3, -1, 7, 6, -1, -1, 11, 10, -1, -1, -1, 14, -1, -1};
static const int16_t tree_56_feature[15] = {2, 9, -2, 4, 9, -2, -2, 8, 1, -2, -2, -2, 3, -2, -2};
static const float tree_56_threshold[15] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, 6.27725458f, -2.0f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_56_leaf_pos[15] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.148215919f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_57_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_57_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_57_feature[11] = {9, -2, 5, 9, -2, -2, 9, 8, -2, -2, -2};
static const float tree_57_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 2.44868445f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_57_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_58_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_58_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_58_feature[11] = {9, -2, 5, 9, -2, -2, 2, 8, -2, -2, -2};
static const float tree_58_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, -4.31501508f, 1.188972f, -2.0f, -2.0f, -2.0f};
static const float tree_58_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 1.0f};

static const int16_t tree_59_left[15] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, 12, -1, -1, -1};
static const int16_t tree_59_right[15] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 14, 13, -1, -1, -1};
static const int16_t tree_59_feature[15] = {9, -2, 5, 3, 1, 6, -2, -2, -2, -2, 9, 8, -2, -2, -2};
static const float tree_59_threshold[15] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, 116.624443f, -2.0f, -2.0f, -2.0f, -2.0f, 2.44868445f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_59_leaf_pos[15] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_60_left[13] = {1, -1, 3, 4, 5, -1, 7, 8, -1, -1, -1, -1, -1};
static const int16_t tree_60_right[13] = {2, -1, 12, 11, 6, -1, 10, 9, -1, -1, -1, -1, -1};
static const int16_t tree_60_feature[13] = {9, -2, 9, 7, 4, -2, 8, 1, -2, -2, -2, -2, -2};
static const float tree_60_threshold[13] = {1.40355951f, -2.0f, 6.27725458f, 16.8099537f, 70.9573505f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_60_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0800395257f, 1.0f, 1.0f};

static const int16_t tree_61_left[9] = {1, 2, 3, -1, -1, -1, 7, -1, -1};
static const int16_t tree_61_right[9] = {6, 5, 4, -1, -1, -1, 8, -1, -1};
static const int16_t tree_61_feature[9] = {5, 9, 7, -2, -2, -2, 2, -2, -2};
static const float tree_61_threshold[9] = {0.00270000007f, 6.27725458f, 16.3341608f, -2.0f, -2.0f, -2.0f, -0.910058975f, -2.0f, -2.0f};
static const float tree_61_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0502658089f, 1.0f, 1.0f, 0.0f, 0.00502416295f, 1.0f};

static const int16_t tree_62_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_62_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_62_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_62_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_62_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_63_left[15] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, 12, -1, -1, -1};
static const int16_t tree_63_right[15] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 14, 13, -1, -1, -1};
static const int16_t tree_63_feature[15] = {9, -2, 5, 3, 1, 6, -2, -2, -2, -2, 2, 8, -2, -2, -2};
static const float tree_63_threshold[15] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, 116.624443f, -2.0f, -2.0f, -2.0f, -2.0f, -4.31501508f, 1.188972f, -2.0f, -2.0f, -2.0f};
static const float tree_63_leaf_pos[15] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 1.0f};

static const int16_t tree_64_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_64_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_64_feature[11] = {9, -2, 5, 3, -2, -2, 9, 8, -2, -2, -2};
static const float tree_64_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 2.44868445f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_64_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_65_left[17] = {1, 2, -1, 4, 5, -1, -1, 8, 9, -1, 11, -1, -1, -1, 15, -1, -1};
static const int16_t tree_65_right[17] = {14, 3, -1, 7, 6, -1, -1, 13, 10, -1, 12, -1, -1, -1, 16, -1, -1};
static const int16_t tree_65_feature[17] = {2, 9, -2, 4, 9, -2, -2, 9, 8, -2, 4, -2, -2, -2, 4, -2, -2};
static const float tree_65_threshold[17] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, 6.27725458f, -2.0f, -2.0f, 2.44868445f, 0.800971985f, -2.0f, 180.835899f, -2.0f, -2.0f, -2.0f, 100.0f, -2.0f, -2.0f};
static const float tree_65_leaf_pos[17] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_66_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_66_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_66_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_66_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_66_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_67_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_67_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_67_feature[9] = {2, 9, -2, 16, -2, -2, 1, -2, -2};
static const float tree_67_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, 2.78808141f, -2.0f, -2.0f};
static const float tree_67_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_68_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_68_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_68_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_68_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_68_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_69_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_69_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_69_feature[9] = {2, 9, -2, 16, -2, -2, 3, -2, -2};
static const float tree_69_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_69_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_70_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_70_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_70_feature[9] = {2, 9, -2, 16, -2, -2, 3, -2, -2};
static const float tree_70_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_70_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_71_left[13] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, -1, -1};
static const int16_t tree_71_right[13] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 12, -1, -1};
static const int16_t tree_71_feature[13] = {9, -2, 5, 3, 1, 6, -2, -2, -2, -2, 8, -2, -2};
static const float tree_71_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, 116.624443f, -2.0f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_71_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_72_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_72_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_72_feature[9] = {2, 9, -2, 16, -2, -2, 1, -2, -2};
static const float tree_72_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, 2.78808141f, -2.0f, -2.0f};
static const float tree_72_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_73_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_73_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_73_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_73_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_73_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_74_left[11] = {1, -1, 3, 4, 5, -1, -1, -1, 9, -1, -1};
static const int16_t tree_74_right[11] = {2, -1, 8, 7, 6, -1, -1, -1, 10, -1, -1};
static const int16_t tree_74_feature[11] = {9, -2, 5, 1, 8, -2, -2, -2, 8, -2, -2};
static const float tree_74_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, 8.08057356f, 0.62656951f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_74_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0416881112f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_75_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_75_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_75_feature[9] = {2, 9, -2, 16, -2, -2, 1, -2, -2};
static const float tree_75_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, 2.78808141f, -2.0f, -2.0f};
static const float tree_75_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_76_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_76_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_76_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_76_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_76_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_77_left[13] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, -1, -1};
static const int16_t tree_77_right[13] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 12, -1, -1};
static const int16_t tree_77_feature[13] = {9, -2, 5, 3, 1, 6, -2, -2, -2, -2, 8, -2, -2};
static const float tree_77_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, 116.624443f, -2.0f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_77_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_78_left[15] = {1, 2, -1, 4, 5, -1, -1, 8, 9, -1, -1, -1, 13, -1, -1};
static const int16_t tree_78_right[15] = {12, 3, -1, 7, 6, -1, -1, 11, 10, -1, -1, -1, 14, -1, -1};
static const int16_t tree_78_feature[15] = {2, 9, -2, 4, 3, -2, -2, 8, 7, -2, -2, -2, 3, -2, -2};
static const float tree_78_threshold[15] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, -41.1395645f, -2.0f, -2.0f, 1.19457251f, 15.4879923f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_78_leaf_pos[15] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_79_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_79_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_79_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_79_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_79_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_80_left[17] = {1, 2, -1, 4, 5, -1, 7, 8, -1, 10, -1, -1, -1, -1, 15, -1, -1};
static const int16_t tree_80_right[17] = {14, 3, -1, 13, 6, -1, 12, 9, -1, 11, -1, -1, -1, -1, 16, -1, -1};
static const int16_t tree_80_feature[17] = {2, 9, -2, 9, 4, -2, 5, 7, -2, 7, -2, -2, -2, -2, 1, -2, -2};
static const float tree_80_threshold[17] = {-0.910058975f, 1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 123.594551f, 15.4879923f, -2.0f, 16.6420479f, -2.0f, -2.0f, -2.0f, -2.0f, 2.78808141f, -2.0f, -2.0f};
static const float tree_80_leaf_pos[17] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.342978123f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_81_left[15] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, 12, -1, -1, -1};
static const int16_t tree_81_right[15] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 14, 13, -1, -1, -1};
static const int16_t tree_81_feature[15] = {9, -2, 5, 3, 1, 2, -2, -2, -2, -2, 9, 8, -2, -2, -2};
static const float tree_81_threshold[15] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, -13.0282044f, -2.0f, -2.0f, -2.0f, -2.0f, 2.44868445f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_81_leaf_pos[15] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_82_left[19] = {1, 2, 3, -1, -1, 6, -1, 8, 9, 10, 11, -1, -1, -1, -1, -1, 17, -1, -1};
static const int16_t tree_82_right[19] = {16, 5, 4, -1, -1, 7, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, 18, -1, -1};
static const int16_t tree_82_feature[19] = {2, 4, 7, -2, -2, 9, -2, 8, 3, 3, 2, -2, -2, -2, -2, -2, 3, -2, -2};
static const float tree_82_threshold[19] = {-0.910058975f, 0.0219999999f, 16.3341608f, -2.0f, -2.0f, 1.40355951f, -2.0f, 1.39401704f, -56.7690601f, -61.2141705f, -3.64201546f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_82_leaf_pos[19] = {0.0f, 0.0f, 0.0f, 0.0891525009f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_83_left[13] = {1, -1, 3, 4, -1, 6, 7, 8, -1, -1, -1, -1, -1};
static const int16_t tree_83_right[13] = {2, -1, 12, 5, -1, 11, 10, 9, -1, -1, -1, -1, -1};
static const int16_t tree_83_feature[13] = {9, -2, 9, 5, -2, 10, 5, 8, -2, -2, -2, -2, -2};
static const float tree_83_threshold[13] = {1.40355951f, -2.0f, 6.27725458f, 5.53060007f, -2.0f, 0.5f, 123.594551f, 1.52104199f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_83_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.11543943f, 1.0f, 1.0f};

static const int16_t tree_84_left[13] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, -1, -1};
static const int16_t tree_84_right[13] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 12, -1, -1};
static const int16_t tree_84_feature[13] = {9, -2, 5, 3, 1, 6, -2, -2, -2, -2, 8, -2, -2};
static const float tree_84_threshold[13] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, 116.624443f, -2.0f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_84_leaf_pos[13] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_85_left[1] = {-1};
static const int16_t tree_85_right[1] = {-1};
static const int16_t tree_85_feature[1] = {-2};
static const float tree_85_threshold[1] = {-2.0f};
static const float tree_85_leaf_pos[1] = {0.00582770471f};

static const int16_t tree_86_left[9] = {1, -1, 3, -1, 5, -1, 7, -1, -1};
static const int16_t tree_86_right[9] = {2, -1, 4, -1, 6, -1, 8, -1, -1};
static const int16_t tree_86_feature[9] = {9, -2, 16, -2, 4, -2, 8, -2, -2};
static const float tree_86_threshold[9] = {1.40355951f, -2.0f, 0.5f, -2.0f, 70.9573505f, -2.0f, 1.39401704f, -2.0f, -2.0f};
static const float tree_86_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.171292625f, 0.0f, 0.278853602f, 0.0f, 0.87124879f, 0.0f};

static const int16_t tree_87_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_87_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_87_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_87_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_87_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_88_left[11] = {1, -1, 3, 4, 5, -1, -1, -1, 9, -1, -1};
static const int16_t tree_88_right[11] = {2, -1, 8, 7, 6, -1, -1, -1, 10, -1, -1};
static const int16_t tree_88_feature[11] = {9, -2, 5, 1, 2, -2, -2, -2, 8, -2, -2};
static const float tree_88_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, 8.08057356f, -13.0282044f, -2.0f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_88_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0416881112f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_89_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_89_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_89_feature[11] = {9, -2, 4, 9, -2, -2, 8, 7, -2, -2, -2};
static const float tree_89_threshold[11] = {1.40355951f, -2.0f, 70.9573505f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, 15.3554201f, -2.0f, -2.0f, -2.0f};
static const float tree_89_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_90_left[19] = {1, 2, 3, 4, -1, -1, -1, 8, -1, 10, -1, 12, 13, -1, -1, -1, 17, -1, -1};
static const int16_t tree_90_right[19] = {16, 7, 6, 5, -1, -1, -1, 9, -1, 11, -1, 15, 14, -1, -1, -1, 18, -1, -1};
static const int16_t tree_90_feature[19] = {2, 4, 9, 7, -2, -2, -2, 9, -2, 2, -2, 8, 1, -2, -2, -2, 3, -2, -2};
static const float tree_90_threshold[19] = {-0.910058975f, 0.0219999999f, 6.27725458f, 16.3341608f, -2.0f, -2.0f, -2.0f, 1.40355951f, -2.0f, -12.8594089f, -2.0f, 1.39401704f, 13.6169615f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_90_leaf_pos[19] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0612553567f, 1.0f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_91_left[9] = {1, -1, 3, -1, 5, -1, 7, -1, -1};
static const int16_t tree_91_right[9] = {2, -1, 4, -1, 6, -1, 8, -1, -1};
static const int16_t tree_91_feature[9] = {9, -2, 16, -2, 4, -2, 8, -2, -2};
static const float tree_91_threshold[9] = {1.40355951f, -2.0f, 0.5f, -2.0f, 70.9573505f, -2.0f, 1.39401704f, -2.0f, -2.0f};
static const float tree_91_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.171292625f, 0.0f, 0.278853602f, 0.0f, 0.87124879f, 0.0f};

static const int16_t tree_92_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_92_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_92_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_92_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_92_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_93_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_93_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_93_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_93_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_93_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_94_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_94_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_94_feature[11] = {9, -2, 9, 4, -2, 8, 1, -2, -2, -2, -2};
static const float tree_94_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_94_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0800395257f, 1.0f};

static const int16_t tree_95_left[15] = {1, -1, 3, 4, 5, 6, -1, -1, -1, -1, 11, 12, -1, -1, -1};
static const int16_t tree_95_right[15] = {2, -1, 10, 9, 8, 7, -1, -1, -1, -1, 14, 13, -1, -1, -1};
static const int16_t tree_95_feature[15] = {9, -2, 5, 3, 1, 2, -2, -2, -2, -2, 2, 8, -2, -2, -2};
static const float tree_95_threshold[15] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, 7.81931019f, -13.0282044f, -2.0f, -2.0f, -2.0f, -2.0f, -4.31501508f, 1.188972f, -2.0f, -2.0f, -2.0f};
static const float tree_95_leaf_pos[15] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 1.0f};

static const int16_t tree_96_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_96_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_96_feature[9] = {2, 9, -2, 16, -2, -2, 3, -2, -2};
static const float tree_96_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_96_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_97_left[15] = {1, 2, -1, 4, 5, -1, -1, 8, 9, -1, -1, -1, 13, -1, -1};
static const int16_t tree_97_right[15] = {12, 3, -1, 7, 6, -1, -1, 11, 10, -1, -1, -1, 14, -1, -1};
static const int16_t tree_97_feature[15] = {2, 9, -2, 4, 9, -2, -2, 8, 1, -2, -2, -2, 3, -2, -2};
static const float tree_97_threshold[15] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, 6.27725458f, -2.0f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_97_leaf_pos[15] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.148215919f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_98_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_98_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_98_feature[11] = {9, -2, 9, 4, -2, 8, 3, -2, -2, -2, -2};
static const float tree_98_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, -62.7468605f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_98_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 0.206984668f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_99_left[9] = {1, 2, 3, -1, -1, -1, 7, -1, -1};
static const int16_t tree_99_right[9] = {6, 5, 4, -1, -1, -1, 8, -1, -1};
static const int16_t tree_99_feature[9] = {2, 4, 7, -2, -2, -2, 7, -2, -2};
static const float tree_99_threshold[9] = {-0.910058975f, 0.0219999999f, 16.3341608f, -2.0f, -2.0f, -2.0f, 7.68103981f, -2.0f, -2.0f};
static const float tree_99_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0891525009f, 1.0f, 0.00502416295f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_100_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_100_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_100_feature[9] = {2, 9, -2, 16, -2, -2, 3, -2, -2};
static const float tree_100_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_100_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_101_left[13] = {1, 2, -1, -1, 5, -1, 7, 8, 9, -1, -1, -1, -1};
static const int16_t tree_101_right[13] = {4, 3, -1, -1, 6, -1, 12, 11, 10, -1, -1, -1, -1};
static const int16_t tree_101_feature[13] = {4, 7, -2, -2, 9, -2, 8, 5, 6, -2, -2, -2, -2};
static const float tree_101_threshold[13] = {0.0219999999f, 16.3341608f, -2.0f, -2.0f, 1.40355951f, -2.0f, 1.22002703f, 5.53060007f, 116.720131f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_101_leaf_pos[13] = {0.0f, 0.0f, 0.0735501836f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.148215919f};

static const int16_t tree_102_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_102_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_102_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_102_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_102_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_103_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_103_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_103_feature[11] = {9, -2, 5, 9, -2, -2, 2, 8, -2, -2, -2};
static const float tree_103_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, -4.31501508f, 1.188972f, -2.0f, -2.0f, -2.0f};
static const float tree_103_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 1.0f};

static const int16_t tree_104_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_104_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_104_feature[9] = {9, -2, 5, 3, -2, -2, 8, -2, -2};
static const float tree_104_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_104_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_105_left[11] = {1, -1, 3, -1, 5, 6, 7, -1, -1, -1, -1};
static const int16_t tree_105_right[11] = {2, -1, 4, -1, 10, 9, 8, -1, -1, -1, -1};
static const int16_t tree_105_feature[11] = {5, -2, 9, -2, 8, 5, 6, -2, -2, -2, -2};
static const float tree_105_threshold[11] = {0.00270000007f, -2.0f, 1.40355951f, -2.0f, 1.22002703f, 5.53060007f, 116.720131f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_105_leaf_pos[11] = {0.0f, 0.137022348f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.148215919f};

static const int16_t tree_106_left[9] = {1, 2, 3, -1, -1, -1, 7, -1, -1};
static const int16_t tree_106_right[9] = {6, 5, 4, -1, -1, -1, 8, -1, -1};
static const int16_t tree_106_feature[9] = {2, 5, 7, -2, -2, -2, 3, -2, -2};
static const float tree_106_threshold[9] = {-0.910058975f, 0.00270000007f, 16.3341608f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_106_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0891525009f, 1.0f, 0.00502416295f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_107_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_107_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_107_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_107_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_107_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_108_left[15] = {1, 2, -1, -1, 5, 6, -1, 8, -1, 10, 11, -1, -1, -1, -1};
static const int16_t tree_108_right[15] = {4, 3, -1, -1, 14, 7, -1, 9, -1, 13, 12, -1, -1, -1, -1};
static const int16_t tree_108_feature[15] = {4, 7, -2, -2, 2, 9, -2, 2, -2, 8, 1, -2, -2, -2, -2};
static const float tree_108_threshold[15] = {0.0219999999f, 16.3341608f, -2.0f, -2.0f, -0.910058975f, 1.40355951f, -2.0f, -12.8594089f, -2.0f, 1.39401704f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_108_leaf_pos[15] = {0.0f, 0.0f, 0.0735501836f, 1.0f, 0.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_109_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_109_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_109_feature[11] = {9, -2, 9, 4, -2, 8, 1, -2, -2, -2, -2};
static const float tree_109_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_109_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0800395257f, 1.0f};

static const int16_t tree_110_left[13] = {1, 2, -1, 4, -1, 6, 7, -1, -1, -1, 11, -1, -1};
static const int16_t tree_110_right[13] = {10, 3, -1, 5, -1, 9, 8, -1, -1, -1, 12, -1, -1};
static const int16_t tree_110_feature[13] = {2, 9, -2, 4, -2, 8, 7, -2, -2, -2, 3, -2, -2};
static const float tree_110_threshold[13] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, -2.0f, 1.19457251f, 15.4879923f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_110_leaf_pos[13] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.129790359f, 0.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_111_left[11] = {1, -1, 3, 4, -1, -1, 7, 8, -1, -1, -1};
static const int16_t tree_111_right[11] = {2, -1, 6, 5, -1, -1, 10, 9, -1, -1, -1};
static const int16_t tree_111_feature[11] = {9, -2, 5, 3, -2, -2, 9, 8, -2, -2, -2};
static const float tree_111_threshold[11] = {1.40355951f, -2.0f, 5.53060007f, -41.1395645f, -2.0f, -2.0f, 2.44868445f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_111_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_112_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_112_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_112_feature[9] = {2, 9, -2, 16, -2, -2, 1, -2, -2};
static const float tree_112_threshold[9] = {-0.910058975f, 1.40355951f, -2.0f, 0.5f, -2.0f, -2.0f, 2.78808141f, -2.0f, -2.0f};
static const float tree_112_leaf_pos[9] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.140625f, 0.538922156f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_113_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_113_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_113_feature[11] = {9, -2, 9, 4, -2, 8, 1, -2, -2, -2, -2};
static const float tree_113_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_113_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0800395257f, 1.0f};

static const int16_t tree_114_left[19] = {1, 2, 3, 4, -1, -1, -1, 8, -1, 10, -1, 12, 13, -1, -1, -1, 17, -1, -1};
static const int16_t tree_114_right[19] = {16, 7, 6, 5, -1, -1, -1, 9, -1, 11, -1, 15, 14, -1, -1, -1, 18, -1, -1};
static const int16_t tree_114_feature[19] = {2, 5, 9, 7, -2, -2, -2, 9, -2, 2, -2, 8, 1, -2, -2, -2, 3, -2, -2};
static const float tree_114_threshold[19] = {-0.910058975f, 0.00270000007f, 6.27725458f, 16.3341608f, -2.0f, -2.0f, -2.0f, 1.40355951f, -2.0f, -12.8594089f, -2.0f, 1.39401704f, 13.6169615f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_114_leaf_pos[19] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0612553567f, 1.0f, 1.0f, 0.0f, 0.00390936341f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_115_left[9] = {1, -1, 3, 4, -1, -1, 7, -1, -1};
static const int16_t tree_115_right[9] = {2, -1, 6, 5, -1, -1, 8, -1, -1};
static const int16_t tree_115_feature[9] = {9, -2, 5, 9, -2, -2, 8, -2, -2};
static const float tree_115_threshold[9] = {1.40355951f, -2.0f, 5.53060007f, 6.27725458f, -2.0f, -2.0f, 1.22002703f, -2.0f, -2.0f};
static const float tree_115_leaf_pos[9] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.11543943f, 1.0f, 0.0f, 1.0f, 0.148215919f};

static const int16_t tree_116_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_116_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_116_feature[11] = {9, -2, 9, 4, -2, 8, 7, -2, -2, -2, -2};
static const float tree_116_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 15.4879923f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_116_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0800395257f, 1.0f};

static const int16_t tree_117_left[11] = {1, -1, 3, 4, -1, 6, -1, 8, -1, -1, -1};
static const int16_t tree_117_right[11] = {2, -1, 10, 5, -1, 7, -1, 9, -1, -1, -1};
static const int16_t tree_117_feature[11] = {9, -2, 9, 16, -2, 5, -2, 8, -2, -2, -2};
static const float tree_117_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 0.5f, -2.0f, 5.53060007f, -2.0f, 1.19457251f, -2.0f, -2.0f, -2.0f};
static const float tree_117_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.127712855f, 0.0f, 0.252808989f, 0.0f, 1.0f, 0.18404908f, 1.0f};

static const int16_t tree_118_left[15] = {1, 2, -1, 4, 5, -1, -1, 8, 9, -1, -1, -1, 13, -1, -1};
static const int16_t tree_118_right[15] = {12, 3, -1, 7, 6, -1, -1, 11, 10, -1, -1, -1, 14, -1, -1};
static const int16_t tree_118_feature[15] = {2, 9, -2, 4, 3, -2, -2, 8, 7, -2, -2, -2, 3, -2, -2};
static const float tree_118_threshold[15] = {-0.910058975f, 1.40355951f, -2.0f, 70.9573505f, -41.1395645f, -2.0f, -2.0f, 1.19457251f, 15.4879923f, -2.0f, -2.0f, -2.0f, -11.1115923f, -2.0f, -2.0f};
static const float tree_118_leaf_pos[15] = {0.0f, 0.0f, 0.00405677657f, 0.0f, 0.0f, 0.0693988291f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.148215919f, 0.0f, 1.0f, 0.0f};

static const int16_t tree_119_left[11] = {1, -1, 3, 4, -1, 6, 7, -1, -1, -1, -1};
static const int16_t tree_119_right[11] = {2, -1, 10, 5, -1, 9, 8, -1, -1, -1, -1};
static const int16_t tree_119_feature[11] = {9, -2, 9, 4, -2, 8, 1, -2, -2, -2, -2};
static const float tree_119_threshold[11] = {1.40355951f, -2.0f, 6.27725458f, 70.9573505f, -2.0f, 1.19457251f, 13.6169615f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_119_leaf_pos[11] = {0.0f, 0.00405460222f, 0.0f, 0.0f, 0.0693988291f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0800395257f, 1.0f};

static inline void arc_rf_predict(double * input, double * output) {
    float sum_pos = 0.0f;
    sum_pos += arc_tree_predict_proba_pos(tree_0_left, tree_0_right, tree_0_feature, tree_0_threshold, tree_0_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_1_left, tree_1_right, tree_1_feature, tree_1_threshold, tree_1_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_2_left, tree_2_right, tree_2_feature, tree_2_threshold, tree_2_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_3_left, tree_3_right, tree_3_feature, tree_3_threshold, tree_3_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_4_left, tree_4_right, tree_4_feature, tree_4_threshold, tree_4_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_5_left, tree_5_right, tree_5_feature, tree_5_threshold, tree_5_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_6_left, tree_6_right, tree_6_feature, tree_6_threshold, tree_6_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_7_left, tree_7_right, tree_7_feature, tree_7_threshold, tree_7_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_8_left, tree_8_right, tree_8_feature, tree_8_threshold, tree_8_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_9_left, tree_9_right, tree_9_feature, tree_9_threshold, tree_9_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_10_left, tree_10_right, tree_10_feature, tree_10_threshold, tree_10_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_11_left, tree_11_right, tree_11_feature, tree_11_threshold, tree_11_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_12_left, tree_12_right, tree_12_feature, tree_12_threshold, tree_12_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_13_left, tree_13_right, tree_13_feature, tree_13_threshold, tree_13_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_14_left, tree_14_right, tree_14_feature, tree_14_threshold, tree_14_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_15_left, tree_15_right, tree_15_feature, tree_15_threshold, tree_15_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_16_left, tree_16_right, tree_16_feature, tree_16_threshold, tree_16_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_17_left, tree_17_right, tree_17_feature, tree_17_threshold, tree_17_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_18_left, tree_18_right, tree_18_feature, tree_18_threshold, tree_18_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_19_left, tree_19_right, tree_19_feature, tree_19_threshold, tree_19_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_20_left, tree_20_right, tree_20_feature, tree_20_threshold, tree_20_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_21_left, tree_21_right, tree_21_feature, tree_21_threshold, tree_21_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_22_left, tree_22_right, tree_22_feature, tree_22_threshold, tree_22_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_23_left, tree_23_right, tree_23_feature, tree_23_threshold, tree_23_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_24_left, tree_24_right, tree_24_feature, tree_24_threshold, tree_24_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_25_left, tree_25_right, tree_25_feature, tree_25_threshold, tree_25_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_26_left, tree_26_right, tree_26_feature, tree_26_threshold, tree_26_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_27_left, tree_27_right, tree_27_feature, tree_27_threshold, tree_27_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_28_left, tree_28_right, tree_28_feature, tree_28_threshold, tree_28_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_29_left, tree_29_right, tree_29_feature, tree_29_threshold, tree_29_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_30_left, tree_30_right, tree_30_feature, tree_30_threshold, tree_30_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_31_left, tree_31_right, tree_31_feature, tree_31_threshold, tree_31_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_32_left, tree_32_right, tree_32_feature, tree_32_threshold, tree_32_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_33_left, tree_33_right, tree_33_feature, tree_33_threshold, tree_33_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_34_left, tree_34_right, tree_34_feature, tree_34_threshold, tree_34_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_35_left, tree_35_right, tree_35_feature, tree_35_threshold, tree_35_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_36_left, tree_36_right, tree_36_feature, tree_36_threshold, tree_36_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_37_left, tree_37_right, tree_37_feature, tree_37_threshold, tree_37_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_38_left, tree_38_right, tree_38_feature, tree_38_threshold, tree_38_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_39_left, tree_39_right, tree_39_feature, tree_39_threshold, tree_39_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_40_left, tree_40_right, tree_40_feature, tree_40_threshold, tree_40_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_41_left, tree_41_right, tree_41_feature, tree_41_threshold, tree_41_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_42_left, tree_42_right, tree_42_feature, tree_42_threshold, tree_42_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_43_left, tree_43_right, tree_43_feature, tree_43_threshold, tree_43_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_44_left, tree_44_right, tree_44_feature, tree_44_threshold, tree_44_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_45_left, tree_45_right, tree_45_feature, tree_45_threshold, tree_45_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_46_left, tree_46_right, tree_46_feature, tree_46_threshold, tree_46_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_47_left, tree_47_right, tree_47_feature, tree_47_threshold, tree_47_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_48_left, tree_48_right, tree_48_feature, tree_48_threshold, tree_48_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_49_left, tree_49_right, tree_49_feature, tree_49_threshold, tree_49_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_50_left, tree_50_right, tree_50_feature, tree_50_threshold, tree_50_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_51_left, tree_51_right, tree_51_feature, tree_51_threshold, tree_51_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_52_left, tree_52_right, tree_52_feature, tree_52_threshold, tree_52_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_53_left, tree_53_right, tree_53_feature, tree_53_threshold, tree_53_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_54_left, tree_54_right, tree_54_feature, tree_54_threshold, tree_54_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_55_left, tree_55_right, tree_55_feature, tree_55_threshold, tree_55_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_56_left, tree_56_right, tree_56_feature, tree_56_threshold, tree_56_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_57_left, tree_57_right, tree_57_feature, tree_57_threshold, tree_57_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_58_left, tree_58_right, tree_58_feature, tree_58_threshold, tree_58_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_59_left, tree_59_right, tree_59_feature, tree_59_threshold, tree_59_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_60_left, tree_60_right, tree_60_feature, tree_60_threshold, tree_60_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_61_left, tree_61_right, tree_61_feature, tree_61_threshold, tree_61_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_62_left, tree_62_right, tree_62_feature, tree_62_threshold, tree_62_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_63_left, tree_63_right, tree_63_feature, tree_63_threshold, tree_63_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_64_left, tree_64_right, tree_64_feature, tree_64_threshold, tree_64_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_65_left, tree_65_right, tree_65_feature, tree_65_threshold, tree_65_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_66_left, tree_66_right, tree_66_feature, tree_66_threshold, tree_66_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_67_left, tree_67_right, tree_67_feature, tree_67_threshold, tree_67_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_68_left, tree_68_right, tree_68_feature, tree_68_threshold, tree_68_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_69_left, tree_69_right, tree_69_feature, tree_69_threshold, tree_69_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_70_left, tree_70_right, tree_70_feature, tree_70_threshold, tree_70_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_71_left, tree_71_right, tree_71_feature, tree_71_threshold, tree_71_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_72_left, tree_72_right, tree_72_feature, tree_72_threshold, tree_72_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_73_left, tree_73_right, tree_73_feature, tree_73_threshold, tree_73_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_74_left, tree_74_right, tree_74_feature, tree_74_threshold, tree_74_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_75_left, tree_75_right, tree_75_feature, tree_75_threshold, tree_75_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_76_left, tree_76_right, tree_76_feature, tree_76_threshold, tree_76_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_77_left, tree_77_right, tree_77_feature, tree_77_threshold, tree_77_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_78_left, tree_78_right, tree_78_feature, tree_78_threshold, tree_78_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_79_left, tree_79_right, tree_79_feature, tree_79_threshold, tree_79_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_80_left, tree_80_right, tree_80_feature, tree_80_threshold, tree_80_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_81_left, tree_81_right, tree_81_feature, tree_81_threshold, tree_81_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_82_left, tree_82_right, tree_82_feature, tree_82_threshold, tree_82_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_83_left, tree_83_right, tree_83_feature, tree_83_threshold, tree_83_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_84_left, tree_84_right, tree_84_feature, tree_84_threshold, tree_84_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_85_left, tree_85_right, tree_85_feature, tree_85_threshold, tree_85_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_86_left, tree_86_right, tree_86_feature, tree_86_threshold, tree_86_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_87_left, tree_87_right, tree_87_feature, tree_87_threshold, tree_87_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_88_left, tree_88_right, tree_88_feature, tree_88_threshold, tree_88_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_89_left, tree_89_right, tree_89_feature, tree_89_threshold, tree_89_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_90_left, tree_90_right, tree_90_feature, tree_90_threshold, tree_90_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_91_left, tree_91_right, tree_91_feature, tree_91_threshold, tree_91_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_92_left, tree_92_right, tree_92_feature, tree_92_threshold, tree_92_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_93_left, tree_93_right, tree_93_feature, tree_93_threshold, tree_93_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_94_left, tree_94_right, tree_94_feature, tree_94_threshold, tree_94_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_95_left, tree_95_right, tree_95_feature, tree_95_threshold, tree_95_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_96_left, tree_96_right, tree_96_feature, tree_96_threshold, tree_96_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_97_left, tree_97_right, tree_97_feature, tree_97_threshold, tree_97_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_98_left, tree_98_right, tree_98_feature, tree_98_threshold, tree_98_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_99_left, tree_99_right, tree_99_feature, tree_99_threshold, tree_99_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_100_left, tree_100_right, tree_100_feature, tree_100_threshold, tree_100_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_101_left, tree_101_right, tree_101_feature, tree_101_threshold, tree_101_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_102_left, tree_102_right, tree_102_feature, tree_102_threshold, tree_102_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_103_left, tree_103_right, tree_103_feature, tree_103_threshold, tree_103_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_104_left, tree_104_right, tree_104_feature, tree_104_threshold, tree_104_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_105_left, tree_105_right, tree_105_feature, tree_105_threshold, tree_105_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_106_left, tree_106_right, tree_106_feature, tree_106_threshold, tree_106_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_107_left, tree_107_right, tree_107_feature, tree_107_threshold, tree_107_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_108_left, tree_108_right, tree_108_feature, tree_108_threshold, tree_108_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_109_left, tree_109_right, tree_109_feature, tree_109_threshold, tree_109_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_110_left, tree_110_right, tree_110_feature, tree_110_threshold, tree_110_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_111_left, tree_111_right, tree_111_feature, tree_111_threshold, tree_111_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_112_left, tree_112_right, tree_112_feature, tree_112_threshold, tree_112_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_113_left, tree_113_right, tree_113_feature, tree_113_threshold, tree_113_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_114_left, tree_114_right, tree_114_feature, tree_114_threshold, tree_114_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_115_left, tree_115_right, tree_115_feature, tree_115_threshold, tree_115_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_116_left, tree_116_right, tree_116_feature, tree_116_threshold, tree_116_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_117_left, tree_117_right, tree_117_feature, tree_117_threshold, tree_117_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_118_left, tree_118_right, tree_118_feature, tree_118_threshold, tree_118_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_119_left, tree_119_right, tree_119_feature, tree_119_threshold, tree_119_leaf_pos, input);
    const float pos = sum_pos / 120.0f;
    output[0] = (double)(1.0f - pos);
    output[1] = (double)pos;
}
