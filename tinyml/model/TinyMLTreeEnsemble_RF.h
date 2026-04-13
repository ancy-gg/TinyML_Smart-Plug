#pragma once
// Auto-generated C header from scikit-learn RandomForest (native_tree_ensemble)
#define ARC_MODEL_FEATURE_VERSION 6
#define ARC_MODEL_INPUT_DIM 19
#define ARC_MODEL_BASE_FEATURE_COUNT 12
#define ARC_MODEL_CONTEXT_FEATURE_COUNT 7
#define ARC_MODEL_CONTEXT_INPUT_OFFSET 12
#define ARC_THRESHOLD 0.5900f
#define ARC_CONTEXT_CONFIDENCE_MIN 0.4500f
#define ARC_THRESHOLD_UNKNOWN 0.9600f
#define ARC_UNKNOWN_MIN_FEATURE_VOTES 3

// Input Feature Order:
// [0] thd_i
// [1] spectral_flux_midhf
// [2] hf_energy_delta
// [3] residual_crest_factor
// [4] peak_fluct_cv
// [5] zcv
// [6] cycle_nmse
// [7] delta_hf_energy
// [8] delta_flux
// [9] delta_irms_abs
// [10] midband_residual_ratio
// [11] edge_spike_ratio
// [12] ctx_family_resistive_linear
// [13] ctx_family_inductive_motor
// [14] ctx_family_rectifier_smps
// [15] ctx_family_phase_angle_controlled
// [16] ctx_family_brush_universal_motor
// [17] ctx_family_other_mixed
// [18] context_family_confidence

#include <string.h>
static const int arc_model_input_feature_ids[ARC_MODEL_INPUT_DIM] = {11, 8, 12, 10, 9, 7, 15, 4, 5, 1, 6, 13, 16, 17, 18, 19, 20, 21, 22};
static const int arc_model_base_feature_ids[ARC_MODEL_BASE_FEATURE_COUNT] = {11, 8, 12, 10, 9, 7, 15, 4, 5, 1, 6, 13};
static const float arc_family_thresholds[6] = {0.59f, 0.59f, 0.59f, 0.95f, 0.95f, 0.59f};
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

static const int16_t tree_0_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_0_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_0_feature[7] = {10, 8, -2, -2, 8, -2, -2};
static const float tree_0_threshold[7] = {-22.1681328f, 10.4044724f, -2.0f, -2.0f, 15.2789798f, -2.0f, -2.0f};
static const float tree_0_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_1_left[3] = {1, -1, -1};
static const int16_t tree_1_right[3] = {2, -1, -1};
static const int16_t tree_1_feature[3] = {9, -2, -2};
static const float tree_1_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_1_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_2_left[9] = {1, 2, -1, -1, 5, 6, -1, -1, -1};
static const int16_t tree_2_right[9] = {4, 3, -1, -1, 8, 7, -1, -1, -1};
static const int16_t tree_2_feature[9] = {7, 9, -2, -2, 4, 8, -2, -2, -2};
static const float tree_2_threshold[9] = {3.2446996f, 0.778583407f, -2.0f, -2.0f, 10.661005f, 10.9449224f, -2.0f, -2.0f, -2.0f};
static const float tree_2_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_3_left[9] = {1, 2, 3, -1, 5, -1, -1, -1, -1};
static const int16_t tree_3_right[9] = {8, 7, 4, -1, 6, -1, -1, -1, -1};
static const int16_t tree_3_feature[9] = {6, 1, 7, -2, 0, -2, -2, -2, -2};
static const float tree_3_threshold[9] = {8.74098396f, 199.870956f, 3.21060848f, -2.0f, 184.875603f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_3_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_4_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_4_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_4_feature[7] = {11, 9, -2, -2, 8, -2, -2};
static const float tree_4_threshold[7] = {-0.134770144f, 0.770513773f, -2.0f, -2.0f, 9.76107359f, -2.0f, -2.0f};
static const float tree_4_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_5_left[3] = {1, -1, -1};
static const int16_t tree_5_right[3] = {2, -1, -1};
static const int16_t tree_5_feature[3] = {8, -2, -2};
static const float tree_5_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_5_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_6_left[9] = {1, 2, 3, -1, -1, 6, -1, -1, -1};
static const int16_t tree_6_right[9] = {8, 5, 4, -1, -1, 7, -1, -1, -1};
static const int16_t tree_6_feature[9] = {6, 6, 1, -2, -2, 0, -2, -2, -2};
static const float tree_6_threshold[9] = {8.74098396f, 8.30864096f, 199.57222f, -2.0f, -2.0f, 181.587883f, -2.0f, -2.0f, -2.0f};
static const float tree_6_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 1.0f, 0.0f, 1.0f};

static const int16_t tree_7_left[3] = {1, -1, -1};
static const int16_t tree_7_right[3] = {2, -1, -1};
static const int16_t tree_7_feature[3] = {8, -2, -2};
static const float tree_7_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_7_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_8_left[3] = {1, -1, -1};
static const int16_t tree_8_right[3] = {2, -1, -1};
static const int16_t tree_8_feature[3] = {9, -2, -2};
static const float tree_8_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_8_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_9_left[3] = {1, -1, -1};
static const int16_t tree_9_right[3] = {2, -1, -1};
static const int16_t tree_9_feature[3] = {8, -2, -2};
static const float tree_9_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_9_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_10_left[3] = {1, -1, -1};
static const int16_t tree_10_right[3] = {2, -1, -1};
static const int16_t tree_10_feature[3] = {9, -2, -2};
static const float tree_10_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_10_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_11_left[3] = {1, -1, -1};
static const int16_t tree_11_right[3] = {2, -1, -1};
static const int16_t tree_11_feature[3] = {8, -2, -2};
static const float tree_11_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_11_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_12_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_12_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_12_feature[7] = {4, 8, -2, -2, 11, -2, -2};
static const float tree_12_threshold[7] = {11.8661442f, 10.9734521f, -2.0f, -2.0f, -4.10353482f, -2.0f, -2.0f};
static const float tree_12_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_13_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_13_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_13_feature[5] = {6, 8, -2, -2, -2};
static const float tree_13_threshold[5] = {8.74098396f, 10.6069479f, -2.0f, -2.0f, -2.0f};
static const float tree_13_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_14_left[3] = {1, -1, -1};
static const int16_t tree_14_right[3] = {2, -1, -1};
static const int16_t tree_14_feature[3] = {8, -2, -2};
static const float tree_14_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_14_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_15_left[7] = {1, 2, 3, -1, -1, -1, -1};
static const int16_t tree_15_right[7] = {6, 5, 4, -1, -1, -1, -1};
static const int16_t tree_15_feature[7] = {6, 1, 4, -2, -2, -2, -2};
static const float tree_15_threshold[7] = {8.74098396f, 199.870956f, 17.2812462f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_15_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 1.0f, 1.0f};

static const int16_t tree_16_left[3] = {1, -1, -1};
static const int16_t tree_16_right[3] = {2, -1, -1};
static const int16_t tree_16_feature[3] = {9, -2, -2};
static const float tree_16_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_16_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_17_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_17_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_17_feature[7] = {3, 8, -2, -2, 6, -2, -2};
static const float tree_17_threshold[7] = {10.6144962f, 10.4044724f, -2.0f, -2.0f, 8.35184002f, -2.0f, -2.0f};
static const float tree_17_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_18_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_18_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_18_feature[5] = {6, 9, -2, -2, -2};
static const float tree_18_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_18_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_19_left[7] = {1, 2, 3, -1, -1, -1, -1};
static const int16_t tree_19_right[7] = {6, 5, 4, -1, -1, -1, -1};
static const int16_t tree_19_feature[7] = {6, 11, 8, -2, -2, -2, -2};
static const float tree_19_threshold[7] = {8.74098396f, 1.30016539f, 10.6069479f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_19_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};

static const int16_t tree_20_left[3] = {1, -1, -1};
static const int16_t tree_20_right[3] = {2, -1, -1};
static const int16_t tree_20_feature[3] = {9, -2, -2};
static const float tree_20_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_20_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_21_left[3] = {1, -1, -1};
static const int16_t tree_21_right[3] = {2, -1, -1};
static const int16_t tree_21_feature[3] = {8, -2, -2};
static const float tree_21_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_21_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_22_left[3] = {1, -1, -1};
static const int16_t tree_22_right[3] = {2, -1, -1};
static const int16_t tree_22_feature[3] = {9, -2, -2};
static const float tree_22_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_22_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_23_left[3] = {1, -1, -1};
static const int16_t tree_23_right[3] = {2, -1, -1};
static const int16_t tree_23_feature[3] = {8, -2, -2};
static const float tree_23_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_23_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_24_left[3] = {1, -1, -1};
static const int16_t tree_24_right[3] = {2, -1, -1};
static const int16_t tree_24_feature[3] = {8, -2, -2};
static const float tree_24_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_24_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_25_left[3] = {1, -1, -1};
static const int16_t tree_25_right[3] = {2, -1, -1};
static const int16_t tree_25_feature[3] = {9, -2, -2};
static const float tree_25_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_25_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_26_left[3] = {1, -1, -1};
static const int16_t tree_26_right[3] = {2, -1, -1};
static const int16_t tree_26_feature[3] = {9, -2, -2};
static const float tree_26_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_26_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_27_left[9] = {1, 2, -1, -1, 5, 6, -1, -1, -1};
static const int16_t tree_27_right[9] = {4, 3, -1, -1, 8, 7, -1, -1, -1};
static const int16_t tree_27_feature[9] = {11, 9, -2, -2, 11, 0, -2, -2, -2};
static const float tree_27_threshold[9] = {-0.134770144f, 0.770513773f, -2.0f, -2.0f, 0.269456029f, 183.487625f, -2.0f, -2.0f, -2.0f};
static const float tree_27_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f};

static const int16_t tree_28_left[3] = {1, -1, -1};
static const int16_t tree_28_right[3] = {2, -1, -1};
static const int16_t tree_28_feature[3] = {9, -2, -2};
static const float tree_28_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_28_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_29_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_29_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_29_feature[7] = {4, 9, -2, -2, 9, -2, -2};
static const float tree_29_threshold[7] = {11.8661442f, 0.770513773f, -2.0f, -2.0f, 0.750401631f, -2.0f, -2.0f};
static const float tree_29_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_30_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_30_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_30_feature[5] = {6, 8, -2, -2, -2};
static const float tree_30_threshold[5] = {8.74098396f, 10.6069479f, -2.0f, -2.0f, -2.0f};
static const float tree_30_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_31_left[3] = {1, -1, -1};
static const int16_t tree_31_right[3] = {2, -1, -1};
static const int16_t tree_31_feature[3] = {9, -2, -2};
static const float tree_31_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_31_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_32_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_32_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_32_feature[5] = {6, 9, -2, -2, -2};
static const float tree_32_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_32_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_33_left[3] = {1, -1, -1};
static const int16_t tree_33_right[3] = {2, -1, -1};
static const int16_t tree_33_feature[3] = {9, -2, -2};
static const float tree_33_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_33_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_34_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_34_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_34_feature[7] = {4, 8, -2, -2, 6, -2, -2};
static const float tree_34_threshold[7] = {11.8661442f, 10.9734521f, -2.0f, -2.0f, 7.10712147f, -2.0f, -2.0f};
static const float tree_34_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_35_left[3] = {1, -1, -1};
static const int16_t tree_35_right[3] = {2, -1, -1};
static const int16_t tree_35_feature[3] = {9, -2, -2};
static const float tree_35_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_35_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_36_left[3] = {1, -1, -1};
static const int16_t tree_36_right[3] = {2, -1, -1};
static const int16_t tree_36_feature[3] = {8, -2, -2};
static const float tree_36_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_36_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_37_left[3] = {1, -1, -1};
static const int16_t tree_37_right[3] = {2, -1, -1};
static const int16_t tree_37_feature[3] = {9, -2, -2};
static const float tree_37_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_37_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_38_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_38_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_38_feature[5] = {6, 9, -2, -2, -2};
static const float tree_38_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_38_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_39_left[3] = {1, -1, -1};
static const int16_t tree_39_right[3] = {2, -1, -1};
static const int16_t tree_39_feature[3] = {8, -2, -2};
static const float tree_39_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_39_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_40_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_40_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_40_feature[7] = {4, 8, -2, -2, 8, -2, -2};
static const float tree_40_threshold[7] = {11.8661442f, 10.9734521f, -2.0f, -2.0f, 8.33791232f, -2.0f, -2.0f};
static const float tree_40_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_41_left[3] = {1, -1, -1};
static const int16_t tree_41_right[3] = {2, -1, -1};
static const int16_t tree_41_feature[3] = {8, -2, -2};
static const float tree_41_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_41_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_42_left[7] = {1, 2, 3, -1, -1, -1, -1};
static const int16_t tree_42_right[7] = {6, 5, 4, -1, -1, -1, -1};
static const int16_t tree_42_feature[7] = {6, 3, 9, -2, -2, -2, -2};
static const float tree_42_threshold[7] = {8.74098396f, 10.8632588f, 0.983708084f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_42_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};

static const int16_t tree_43_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_43_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_43_feature[5] = {6, 9, -2, -2, -2};
static const float tree_43_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_43_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_44_left[3] = {1, -1, -1};
static const int16_t tree_44_right[3] = {2, -1, -1};
static const int16_t tree_44_feature[3] = {9, -2, -2};
static const float tree_44_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_44_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_45_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_45_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_45_feature[7] = {10, 8, -2, -2, 11, -2, -2};
static const float tree_45_threshold[7] = {-22.1681328f, 10.4044724f, -2.0f, -2.0f, 2.32751441f, -2.0f, -2.0f};
static const float tree_45_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_46_left[9] = {1, 2, -1, -1, 5, 6, -1, -1, -1};
static const int16_t tree_46_right[9] = {4, 3, -1, -1, 8, 7, -1, -1, -1};
static const int16_t tree_46_feature[9] = {11, 8, -2, -2, 11, 0, -2, -2, -2};
static const float tree_46_threshold[9] = {-0.134770144f, 10.6069479f, -2.0f, -2.0f, 0.269456029f, 183.487625f, -2.0f, -2.0f, -2.0f};
static const float tree_46_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f};

static const int16_t tree_47_left[3] = {1, -1, -1};
static const int16_t tree_47_right[3] = {2, -1, -1};
static const int16_t tree_47_feature[3] = {8, -2, -2};
static const float tree_47_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_47_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_48_left[3] = {1, -1, -1};
static const int16_t tree_48_right[3] = {2, -1, -1};
static const int16_t tree_48_feature[3] = {9, -2, -2};
static const float tree_48_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_48_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_49_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_49_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_49_feature[7] = {4, 9, -2, -2, 11, -2, -2};
static const float tree_49_threshold[7] = {11.8661442f, 0.770513773f, -2.0f, -2.0f, -4.10353482f, -2.0f, -2.0f};
static const float tree_49_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_50_left[3] = {1, -1, -1};
static const int16_t tree_50_right[3] = {2, -1, -1};
static const int16_t tree_50_feature[3] = {9, -2, -2};
static const float tree_50_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_50_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_51_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_51_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_51_feature[7] = {4, 8, -2, -2, 8, -2, -2};
static const float tree_51_threshold[7] = {11.8661442f, 10.9734521f, -2.0f, -2.0f, 8.33791232f, -2.0f, -2.0f};
static const float tree_51_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_52_left[3] = {1, -1, -1};
static const int16_t tree_52_right[3] = {2, -1, -1};
static const int16_t tree_52_feature[3] = {8, -2, -2};
static const float tree_52_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_52_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_53_left[3] = {1, -1, -1};
static const int16_t tree_53_right[3] = {2, -1, -1};
static const int16_t tree_53_feature[3] = {9, -2, -2};
static const float tree_53_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_53_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_54_left[3] = {1, -1, -1};
static const int16_t tree_54_right[3] = {2, -1, -1};
static const int16_t tree_54_feature[3] = {8, -2, -2};
static const float tree_54_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_54_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_55_left[3] = {1, -1, -1};
static const int16_t tree_55_right[3] = {2, -1, -1};
static const int16_t tree_55_feature[3] = {8, -2, -2};
static const float tree_55_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_55_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_56_left[3] = {1, -1, -1};
static const int16_t tree_56_right[3] = {2, -1, -1};
static const int16_t tree_56_feature[3] = {8, -2, -2};
static const float tree_56_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_56_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_57_left[3] = {1, -1, -1};
static const int16_t tree_57_right[3] = {2, -1, -1};
static const int16_t tree_57_feature[3] = {8, -2, -2};
static const float tree_57_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_57_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_58_left[9] = {1, 2, 3, -1, -1, -1, 7, -1, -1};
static const int16_t tree_58_right[9] = {6, 5, 4, -1, -1, -1, 8, -1, -1};
static const int16_t tree_58_feature[9] = {3, 6, 9, -2, -2, -2, 8, -2, -2};
static const float tree_58_threshold[9] = {10.6144962f, 8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f, 11.694962f, -2.0f, -2.0f};
static const float tree_58_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_59_left[3] = {1, -1, -1};
static const int16_t tree_59_right[3] = {2, -1, -1};
static const int16_t tree_59_feature[3] = {8, -2, -2};
static const float tree_59_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_59_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_60_left[3] = {1, -1, -1};
static const int16_t tree_60_right[3] = {2, -1, -1};
static const int16_t tree_60_feature[3] = {9, -2, -2};
static const float tree_60_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_60_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_61_left[9] = {1, 2, 3, -1, -1, -1, 7, -1, -1};
static const int16_t tree_61_right[9] = {6, 5, 4, -1, -1, -1, 8, -1, -1};
static const int16_t tree_61_feature[9] = {11, 1, 9, -2, -2, -2, 8, -2, -2};
static const float tree_61_threshold[9] = {-0.134770144f, 199.870956f, 0.983708084f, -2.0f, -2.0f, -2.0f, 9.76107359f, -2.0f, -2.0f};
static const float tree_61_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_62_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_62_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_62_feature[7] = {11, 8, -2, -2, 8, -2, -2};
static const float tree_62_threshold[7] = {-0.134770144f, 10.6069479f, -2.0f, -2.0f, 9.76107359f, -2.0f, -2.0f};
static const float tree_62_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_63_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_63_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_63_feature[5] = {6, 9, -2, -2, -2};
static const float tree_63_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_63_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_64_left[3] = {1, -1, -1};
static const int16_t tree_64_right[3] = {2, -1, -1};
static const int16_t tree_64_feature[3] = {9, -2, -2};
static const float tree_64_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_64_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_65_left[9] = {1, 2, 3, -1, -1, 6, -1, -1, -1};
static const int16_t tree_65_right[9] = {8, 5, 4, -1, -1, 7, -1, -1, -1};
static const int16_t tree_65_feature[9] = {6, 5, 6, -2, -2, 9, -2, -2, -2};
static const float tree_65_threshold[9] = {8.74098396f, 0.859479517f, 6.79766083f, -2.0f, -2.0f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_65_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_66_left[9] = {1, 2, -1, -1, 5, 6, -1, -1, -1};
static const int16_t tree_66_right[9] = {4, 3, -1, -1, 8, 7, -1, -1, -1};
static const int16_t tree_66_feature[9] = {7, 9, -2, -2, 4, 8, -2, -2, -2};
static const float tree_66_threshold[9] = {3.2446996f, 0.778583407f, -2.0f, -2.0f, 10.661005f, 10.9449224f, -2.0f, -2.0f, -2.0f};
static const float tree_66_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_67_left[13] = {1, 2, -1, 4, 5, -1, -1, -1, 9, 10, -1, -1, -1};
static const int16_t tree_67_right[13] = {8, 3, -1, 7, 6, -1, -1, -1, 12, 11, -1, -1, -1};
static const int16_t tree_67_feature[13] = {11, 6, -2, 1, 7, -2, -2, -2, 6, 0, -2, -2, -2};
static const float tree_67_threshold[13] = {-0.134770144f, 8.30864096f, -2.0f, 191.951553f, 3.20582473f, -2.0f, -2.0f, -2.0f, 8.19885612f, 186.744492f, -2.0f, -2.0f, -2.0f};
static const float tree_67_leaf_pos[13] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 1.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 1.0f};

static const int16_t tree_68_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_68_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_68_feature[5] = {6, 9, -2, -2, -2};
static const float tree_68_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_68_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_69_left[3] = {1, -1, -1};
static const int16_t tree_69_right[3] = {2, -1, -1};
static const int16_t tree_69_feature[3] = {8, -2, -2};
static const float tree_69_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_69_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_70_left[3] = {1, -1, -1};
static const int16_t tree_70_right[3] = {2, -1, -1};
static const int16_t tree_70_feature[3] = {8, -2, -2};
static const float tree_70_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_70_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_71_left[3] = {1, -1, -1};
static const int16_t tree_71_right[3] = {2, -1, -1};
static const int16_t tree_71_feature[3] = {9, -2, -2};
static const float tree_71_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_71_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_72_left[3] = {1, -1, -1};
static const int16_t tree_72_right[3] = {2, -1, -1};
static const int16_t tree_72_feature[3] = {8, -2, -2};
static const float tree_72_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_72_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_73_left[3] = {1, -1, -1};
static const int16_t tree_73_right[3] = {2, -1, -1};
static const int16_t tree_73_feature[3] = {8, -2, -2};
static const float tree_73_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_73_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_74_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_74_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_74_feature[7] = {0, 9, -2, -2, 6, -2, -2};
static const float tree_74_threshold[7] = {197.566711f, 0.770513773f, -2.0f, -2.0f, 8.81602407f, -2.0f, -2.0f};
static const float tree_74_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_75_left[3] = {1, -1, -1};
static const int16_t tree_75_right[3] = {2, -1, -1};
static const int16_t tree_75_feature[3] = {9, -2, -2};
static const float tree_75_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_75_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_76_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_76_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_76_feature[7] = {3, 8, -2, -2, 8, -2, -2};
static const float tree_76_threshold[7] = {10.6144962f, 10.4044724f, -2.0f, -2.0f, 11.694962f, -2.0f, -2.0f};
static const float tree_76_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_77_left[3] = {1, -1, -1};
static const int16_t tree_77_right[3] = {2, -1, -1};
static const int16_t tree_77_feature[3] = {8, -2, -2};
static const float tree_77_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_77_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_78_left[3] = {1, -1, -1};
static const int16_t tree_78_right[3] = {2, -1, -1};
static const int16_t tree_78_feature[3] = {9, -2, -2};
static const float tree_78_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_78_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_79_left[3] = {1, -1, -1};
static const int16_t tree_79_right[3] = {2, -1, -1};
static const int16_t tree_79_feature[3] = {9, -2, -2};
static const float tree_79_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_79_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_80_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_80_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_80_feature[7] = {3, 8, -2, -2, 9, -2, -2};
static const float tree_80_threshold[7] = {10.6144962f, 10.4044724f, -2.0f, -2.0f, 1.08847046f, -2.0f, -2.0f};
static const float tree_80_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_81_left[11] = {1, 2, 3, -1, 5, -1, -1, -1, 9, -1, -1};
static const int16_t tree_81_right[11] = {8, 7, 4, -1, 6, -1, -1, -1, 10, -1, -1};
static const int16_t tree_81_feature[11] = {11, 1, 4, -2, 8, -2, -2, -2, 9, -2, -2};
static const float tree_81_threshold[11] = {-0.134770144f, 199.870956f, 14.5101953f, -2.0f, 8.37804937f, -2.0f, -2.0f, -2.0f, 0.746599257f, -2.0f, -2.0f};
static const float tree_81_leaf_pos[11] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_82_left[3] = {1, -1, -1};
static const int16_t tree_82_right[3] = {2, -1, -1};
static const int16_t tree_82_feature[3] = {9, -2, -2};
static const float tree_82_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_82_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_83_left[3] = {1, -1, -1};
static const int16_t tree_83_right[3] = {2, -1, -1};
static const int16_t tree_83_feature[3] = {8, -2, -2};
static const float tree_83_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_83_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_84_left[3] = {1, -1, -1};
static const int16_t tree_84_right[3] = {2, -1, -1};
static const int16_t tree_84_feature[3] = {9, -2, -2};
static const float tree_84_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_84_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_85_left[3] = {1, -1, -1};
static const int16_t tree_85_right[3] = {2, -1, -1};
static const int16_t tree_85_feature[3] = {9, -2, -2};
static const float tree_85_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_85_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_86_left[3] = {1, -1, -1};
static const int16_t tree_86_right[3] = {2, -1, -1};
static const int16_t tree_86_feature[3] = {8, -2, -2};
static const float tree_86_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_86_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_87_left[3] = {1, -1, -1};
static const int16_t tree_87_right[3] = {2, -1, -1};
static const int16_t tree_87_feature[3] = {9, -2, -2};
static const float tree_87_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_87_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_88_left[3] = {1, -1, -1};
static const int16_t tree_88_right[3] = {2, -1, -1};
static const int16_t tree_88_feature[3] = {8, -2, -2};
static const float tree_88_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_88_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_89_left[3] = {1, -1, -1};
static const int16_t tree_89_right[3] = {2, -1, -1};
static const int16_t tree_89_feature[3] = {8, -2, -2};
static const float tree_89_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_89_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_90_left[3] = {1, -1, -1};
static const int16_t tree_90_right[3] = {2, -1, -1};
static const int16_t tree_90_feature[3] = {9, -2, -2};
static const float tree_90_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_90_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_91_left[3] = {1, -1, -1};
static const int16_t tree_91_right[3] = {2, -1, -1};
static const int16_t tree_91_feature[3] = {8, -2, -2};
static const float tree_91_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_91_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_92_left[3] = {1, -1, -1};
static const int16_t tree_92_right[3] = {2, -1, -1};
static const int16_t tree_92_feature[3] = {9, -2, -2};
static const float tree_92_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_92_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_93_left[11] = {1, 2, 3, -1, -1, 6, -1, -1, 9, -1, -1};
static const int16_t tree_93_right[11] = {8, 5, 4, -1, -1, 7, -1, -1, 10, -1, -1};
static const int16_t tree_93_feature[11] = {4, 11, 9, -2, -2, 1, -2, -2, 9, -2, -2};
static const float tree_93_threshold[11] = {11.8661442f, -0.00164452614f, 0.770513773f, -2.0f, -2.0f, 186.333847f, -2.0f, -2.0f, 0.750401631f, -2.0f, -2.0f};
static const float tree_93_leaf_pos[11] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_94_left[3] = {1, -1, -1};
static const int16_t tree_94_right[3] = {2, -1, -1};
static const int16_t tree_94_feature[3] = {9, -2, -2};
static const float tree_94_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_94_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_95_left[9] = {1, 2, 3, -1, -1, 6, -1, -1, -1};
static const int16_t tree_95_right[9] = {8, 5, 4, -1, -1, 7, -1, -1, -1};
static const int16_t tree_95_feature[9] = {6, 6, 9, -2, -2, 2, -2, -2, -2};
static const float tree_95_threshold[9] = {8.74098396f, 8.30864096f, 0.489207655f, -2.0f, -2.0f, 15.0872836f, -2.0f, -2.0f, -2.0f};
static const float tree_95_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 1.0f, 0.0f, 1.0f};

static const int16_t tree_96_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_96_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_96_feature[5] = {6, 9, -2, -2, -2};
static const float tree_96_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_96_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_97_left[9] = {1, 2, 3, -1, 5, -1, -1, -1, -1};
static const int16_t tree_97_right[9] = {8, 7, 4, -1, 6, -1, -1, -1, -1};
static const int16_t tree_97_feature[9] = {6, 11, 6, -2, 2, -2, -2, -2, -2};
static const float tree_97_threshold[9] = {8.74098396f, 1.30016539f, 8.30864096f, -2.0f, 15.0872836f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_97_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_98_left[3] = {1, -1, -1};
static const int16_t tree_98_right[3] = {2, -1, -1};
static const int16_t tree_98_feature[3] = {9, -2, -2};
static const float tree_98_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_98_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_99_left[11] = {1, 2, 3, -1, -1, 6, -1, -1, 9, -1, -1};
static const int16_t tree_99_right[11] = {8, 5, 4, -1, -1, 7, -1, -1, 10, -1, -1};
static const int16_t tree_99_feature[11] = {4, 6, 8, -2, -2, 1, -2, -2, 9, -2, -2};
static const float tree_99_threshold[11] = {11.8661442f, 8.48947668f, 9.65366602f, -2.0f, -2.0f, 193.505577f, -2.0f, -2.0f, 0.750401631f, -2.0f, -2.0f};
static const float tree_99_leaf_pos[11] = {0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 0.48427673f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_100_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_100_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_100_feature[5] = {6, 8, -2, -2, -2};
static const float tree_100_threshold[5] = {8.74098396f, 10.6069479f, -2.0f, -2.0f, -2.0f};
static const float tree_100_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_101_left[3] = {1, -1, -1};
static const int16_t tree_101_right[3] = {2, -1, -1};
static const int16_t tree_101_feature[3] = {9, -2, -2};
static const float tree_101_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_101_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_102_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_102_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_102_feature[7] = {10, 9, -2, -2, 11, -2, -2};
static const float tree_102_threshold[7] = {-22.1681328f, 0.770513773f, -2.0f, -2.0f, 2.32751441f, -2.0f, -2.0f};
static const float tree_102_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_103_left[9] = {1, 2, 3, -1, -1, 6, -1, -1, -1};
static const int16_t tree_103_right[9] = {8, 5, 4, -1, -1, 7, -1, -1, -1};
static const int16_t tree_103_feature[9] = {6, 6, 9, -2, -2, 0, -2, -2, -2};
static const float tree_103_threshold[9] = {8.74098396f, 8.30864096f, 0.489207655f, -2.0f, -2.0f, 181.587883f, -2.0f, -2.0f, -2.0f};
static const float tree_103_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 1.0f, 0.0f, 1.0f};

static const int16_t tree_104_left[3] = {1, -1, -1};
static const int16_t tree_104_right[3] = {2, -1, -1};
static const int16_t tree_104_feature[3] = {9, -2, -2};
static const float tree_104_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_104_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_105_left[3] = {1, -1, -1};
static const int16_t tree_105_right[3] = {2, -1, -1};
static const int16_t tree_105_feature[3] = {8, -2, -2};
static const float tree_105_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_105_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_106_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_106_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_106_feature[5] = {6, 8, -2, -2, -2};
static const float tree_106_threshold[5] = {8.74098396f, 10.6069479f, -2.0f, -2.0f, -2.0f};
static const float tree_106_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_107_left[9] = {1, 2, -1, 4, -1, -1, 7, -1, -1};
static const int16_t tree_107_right[9] = {6, 3, -1, 5, -1, -1, 8, -1, -1};
static const int16_t tree_107_feature[9] = {11, 6, -2, 5, -2, -2, 9, -2, -2};
static const float tree_107_threshold[9] = {-0.134770144f, 8.30864096f, -2.0f, 1.37872988f, -2.0f, -2.0f, 0.746599257f, -2.0f, -2.0f};
static const float tree_107_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_108_left[3] = {1, -1, -1};
static const int16_t tree_108_right[3] = {2, -1, -1};
static const int16_t tree_108_feature[3] = {9, -2, -2};
static const float tree_108_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_108_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_109_left[3] = {1, -1, -1};
static const int16_t tree_109_right[3] = {2, -1, -1};
static const int16_t tree_109_feature[3] = {8, -2, -2};
static const float tree_109_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_109_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_110_left[3] = {1, -1, -1};
static const int16_t tree_110_right[3] = {2, -1, -1};
static const int16_t tree_110_feature[3] = {8, -2, -2};
static const float tree_110_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_110_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_111_left[3] = {1, -1, -1};
static const int16_t tree_111_right[3] = {2, -1, -1};
static const int16_t tree_111_feature[3] = {8, -2, -2};
static const float tree_111_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_111_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_112_left[3] = {1, -1, -1};
static const int16_t tree_112_right[3] = {2, -1, -1};
static const int16_t tree_112_feature[3] = {9, -2, -2};
static const float tree_112_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_112_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_113_left[3] = {1, -1, -1};
static const int16_t tree_113_right[3] = {2, -1, -1};
static const int16_t tree_113_feature[3] = {9, -2, -2};
static const float tree_113_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_113_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_114_left[3] = {1, -1, -1};
static const int16_t tree_114_right[3] = {2, -1, -1};
static const int16_t tree_114_feature[3] = {8, -2, -2};
static const float tree_114_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_114_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_115_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_115_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_115_feature[5] = {6, 9, -2, -2, -2};
static const float tree_115_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_115_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_116_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_116_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_116_feature[7] = {4, 8, -2, -2, 6, -2, -2};
static const float tree_116_threshold[7] = {11.8661442f, 10.9734521f, -2.0f, -2.0f, 7.10712147f, -2.0f, -2.0f};
static const float tree_116_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_117_left[9] = {1, 2, 3, -1, -1, 6, -1, -1, -1};
static const int16_t tree_117_right[9] = {8, 5, 4, -1, -1, 7, -1, -1, -1};
static const int16_t tree_117_feature[9] = {6, 4, 1, -2, -2, 9, -2, -2, -2};
static const float tree_117_threshold[9] = {8.74098396f, 16.4507179f, 199.870956f, -2.0f, -2.0f, 0.900287211f, -2.0f, -2.0f, -2.0f};
static const float tree_117_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_118_left[3] = {1, -1, -1};
static const int16_t tree_118_right[3] = {2, -1, -1};
static const int16_t tree_118_feature[3] = {8, -2, -2};
static const float tree_118_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_118_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_119_left[3] = {1, -1, -1};
static const int16_t tree_119_right[3] = {2, -1, -1};
static const int16_t tree_119_feature[3] = {8, -2, -2};
static const float tree_119_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_119_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_120_left[3] = {1, -1, -1};
static const int16_t tree_120_right[3] = {2, -1, -1};
static const int16_t tree_120_feature[3] = {8, -2, -2};
static const float tree_120_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_120_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_121_left[3] = {1, -1, -1};
static const int16_t tree_121_right[3] = {2, -1, -1};
static const int16_t tree_121_feature[3] = {9, -2, -2};
static const float tree_121_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_121_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_122_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_122_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_122_feature[5] = {6, 9, -2, -2, -2};
static const float tree_122_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_122_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_123_left[3] = {1, -1, -1};
static const int16_t tree_123_right[3] = {2, -1, -1};
static const int16_t tree_123_feature[3] = {8, -2, -2};
static const float tree_123_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_123_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_124_left[3] = {1, -1, -1};
static const int16_t tree_124_right[3] = {2, -1, -1};
static const int16_t tree_124_feature[3] = {8, -2, -2};
static const float tree_124_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_124_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_125_left[3] = {1, -1, -1};
static const int16_t tree_125_right[3] = {2, -1, -1};
static const int16_t tree_125_feature[3] = {9, -2, -2};
static const float tree_125_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_125_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_126_left[3] = {1, -1, -1};
static const int16_t tree_126_right[3] = {2, -1, -1};
static const int16_t tree_126_feature[3] = {9, -2, -2};
static const float tree_126_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_126_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_127_left[11] = {1, 2, 3, -1, 5, -1, -1, 8, -1, -1, -1};
static const int16_t tree_127_right[11] = {10, 7, 4, -1, 6, -1, -1, 9, -1, -1, -1};
static const int16_t tree_127_feature[11] = {6, 6, 7, -2, 8, -2, -2, 8, -2, -2, -2};
static const float tree_127_threshold[11] = {8.74098396f, 8.30864096f, 3.24440372f, -2.0f, 8.88097572f, -2.0f, -2.0f, 10.3054385f, -2.0f, -2.0f, -2.0f};
static const float tree_127_leaf_pos[11] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_128_left[11] = {1, 2, 3, -1, -1, 6, -1, -1, 9, -1, -1};
static const int16_t tree_128_right[11] = {8, 5, 4, -1, -1, 7, -1, -1, 10, -1, -1};
static const int16_t tree_128_feature[11] = {4, 1, 5, -2, -2, 10, -2, -2, 6, -2, -2};
static const float tree_128_threshold[11] = {11.8661442f, 195.78894f, 1.67405397f, -2.0f, -2.0f, -50.0611954f, -2.0f, -2.0f, 7.10712147f, -2.0f, -2.0f};
static const float tree_128_leaf_pos[11] = {0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 0.48427673f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_129_left[3] = {1, -1, -1};
static const int16_t tree_129_right[3] = {2, -1, -1};
static const int16_t tree_129_feature[3] = {9, -2, -2};
static const float tree_129_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_129_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_130_left[3] = {1, -1, -1};
static const int16_t tree_130_right[3] = {2, -1, -1};
static const int16_t tree_130_feature[3] = {8, -2, -2};
static const float tree_130_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_130_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_131_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_131_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_131_feature[7] = {4, 8, -2, -2, 6, -2, -2};
static const float tree_131_threshold[7] = {11.8661442f, 10.9734521f, -2.0f, -2.0f, 7.10712147f, -2.0f, -2.0f};
static const float tree_131_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_132_left[3] = {1, -1, -1};
static const int16_t tree_132_right[3] = {2, -1, -1};
static const int16_t tree_132_feature[3] = {9, -2, -2};
static const float tree_132_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_132_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_133_left[21] = {1, 2, 3, 4, -1, 6, -1, -1, 9, -1, 11, 12, -1, -1, -1, 16, -1, -1, 19, -1, -1};
static const int16_t tree_133_right[21] = {18, 15, 8, 5, -1, 7, -1, -1, 10, -1, 14, 13, -1, -1, -1, 17, -1, -1, 20, -1, -1};
static const int16_t tree_133_feature[21] = {3, 4, 7, 3, -2, 9, -2, -2, 1, -2, 11, 1, -2, -2, -2, 5, -2, -2, 6, -2, -2};
static const float tree_133_threshold[21] = {10.6144962f, 17.0308867f, 2.37397408f, 9.02397394f, -2.0f, 1.37170068f, -2.0f, -2.0f, 185.351212f, -2.0f, -0.364624396f, 194.582214f, -2.0f, -2.0f, -2.0f, 0.592260689f, -2.0f, -2.0f, 8.35184002f, -2.0f, -2.0f};
static const float tree_133_leaf_pos[21] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.48427673f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_134_left[3] = {1, -1, -1};
static const int16_t tree_134_right[3] = {2, -1, -1};
static const int16_t tree_134_feature[3] = {9, -2, -2};
static const float tree_134_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_134_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_135_left[3] = {1, -1, -1};
static const int16_t tree_135_right[3] = {2, -1, -1};
static const int16_t tree_135_feature[3] = {9, -2, -2};
static const float tree_135_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_135_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_136_left[15] = {1, 2, 3, 4, -1, -1, 7, -1, -1, 10, -1, -1, 13, -1, -1};
static const int16_t tree_136_right[15] = {12, 9, 6, 5, -1, -1, 8, -1, -1, 11, -1, -1, 14, -1, -1};
static const int16_t tree_136_feature[15] = {4, 3, 6, 1, -2, -2, 8, -2, -2, 2, -2, -2, 9, -2, -2};
static const float tree_136_threshold[15] = {11.8661442f, 10.6144962f, 8.48947668f, 196.072166f, -2.0f, -2.0f, 12.52214f, -2.0f, -2.0f, 13.9827156f, -2.0f, -2.0f, 0.750401631f, -2.0f, -2.0f};
static const float tree_136_leaf_pos[15] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 0.48427673f, 1.0f, 0.0f, 1.0f, 0.652542373f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_137_left[9] = {1, 2, -1, -1, 5, 6, -1, -1, -1};
static const int16_t tree_137_right[9] = {4, 3, -1, -1, 8, 7, -1, -1, -1};
static const int16_t tree_137_feature[9] = {11, 9, -2, -2, 6, 5, -2, -2, -2};
static const float tree_137_threshold[9] = {-0.134770144f, 0.770513773f, -2.0f, -2.0f, 8.19885612f, 1.55060142f, -2.0f, -2.0f, -2.0f};
static const float tree_137_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.48427673f, 0.0f, 1.0f};

static const int16_t tree_138_left[3] = {1, -1, -1};
static const int16_t tree_138_right[3] = {2, -1, -1};
static const int16_t tree_138_feature[3] = {9, -2, -2};
static const float tree_138_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_138_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_139_left[3] = {1, -1, -1};
static const int16_t tree_139_right[3] = {2, -1, -1};
static const int16_t tree_139_feature[3] = {9, -2, -2};
static const float tree_139_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_139_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_140_left[3] = {1, -1, -1};
static const int16_t tree_140_right[3] = {2, -1, -1};
static const int16_t tree_140_feature[3] = {9, -2, -2};
static const float tree_140_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_140_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_141_left[7] = {1, 2, -1, -1, 5, -1, -1};
static const int16_t tree_141_right[7] = {4, 3, -1, -1, 6, -1, -1};
static const int16_t tree_141_feature[7] = {10, 8, -2, -2, 9, -2, -2};
static const float tree_141_threshold[7] = {-22.1681328f, 10.4044724f, -2.0f, -2.0f, 1.32102716f, -2.0f, -2.0f};
static const float tree_141_leaf_pos[7] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.48427673f, 1.0f};

static const int16_t tree_142_left[3] = {1, -1, -1};
static const int16_t tree_142_right[3] = {2, -1, -1};
static const int16_t tree_142_feature[3] = {9, -2, -2};
static const float tree_142_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_142_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_143_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_143_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_143_feature[5] = {6, 9, -2, -2, -2};
static const float tree_143_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_143_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_144_left[3] = {1, -1, -1};
static const int16_t tree_144_right[3] = {2, -1, -1};
static const int16_t tree_144_feature[3] = {8, -2, -2};
static const float tree_144_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_144_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_145_left[13] = {1, 2, 3, -1, -1, 6, -1, 8, -1, -1, 11, -1, -1};
static const int16_t tree_145_right[13] = {10, 5, 4, -1, -1, 7, -1, 9, -1, -1, 12, -1, -1};
static const int16_t tree_145_feature[13] = {4, 11, 1, -2, -2, 5, -2, 5, -2, -2, 6, -2, -2};
static const float tree_145_threshold[13] = {11.8661442f, -0.00164452614f, 198.527405f, -2.0f, -2.0f, 1.34046739f, -2.0f, 1.62216049f, -2.0f, -2.0f, 7.10712147f, -2.0f, -2.0f};
static const float tree_145_leaf_pos[13] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

static const int16_t tree_146_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_146_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_146_feature[5] = {6, 9, -2, -2, -2};
static const float tree_146_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_146_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_147_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_147_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_147_feature[5] = {6, 9, -2, -2, -2};
static const float tree_147_threshold[5] = {8.74098396f, 0.983708084f, -2.0f, -2.0f, -2.0f};
static const float tree_147_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_148_left[3] = {1, -1, -1};
static const int16_t tree_148_right[3] = {2, -1, -1};
static const int16_t tree_148_feature[3] = {8, -2, -2};
static const float tree_148_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_148_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_149_left[3] = {1, -1, -1};
static const int16_t tree_149_right[3] = {2, -1, -1};
static const int16_t tree_149_feature[3] = {9, -2, -2};
static const float tree_149_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_149_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_150_left[3] = {1, -1, -1};
static const int16_t tree_150_right[3] = {2, -1, -1};
static const int16_t tree_150_feature[3] = {8, -2, -2};
static const float tree_150_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_150_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_151_left[5] = {1, 2, -1, -1, -1};
static const int16_t tree_151_right[5] = {4, 3, -1, -1, -1};
static const int16_t tree_151_feature[5] = {6, 8, -2, -2, -2};
static const float tree_151_threshold[5] = {8.74098396f, 10.6069479f, -2.0f, -2.0f, -2.0f};
static const float tree_151_leaf_pos[5] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_152_left[3] = {1, -1, -1};
static const int16_t tree_152_right[3] = {2, -1, -1};
static const int16_t tree_152_feature[3] = {8, -2, -2};
static const float tree_152_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_152_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_153_left[3] = {1, -1, -1};
static const int16_t tree_153_right[3] = {2, -1, -1};
static const int16_t tree_153_feature[3] = {8, -2, -2};
static const float tree_153_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_153_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_154_left[3] = {1, -1, -1};
static const int16_t tree_154_right[3] = {2, -1, -1};
static const int16_t tree_154_feature[3] = {8, -2, -2};
static const float tree_154_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_154_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_155_left[9] = {1, 2, 3, -1, 5, -1, -1, -1, -1};
static const int16_t tree_155_right[9] = {8, 7, 4, -1, 6, -1, -1, -1, -1};
static const int16_t tree_155_feature[9] = {6, 3, 11, -2, 5, -2, -2, -2, -2};
static const float tree_155_threshold[9] = {8.74098396f, 10.8632588f, -0.329470634f, -2.0f, 1.27912623f, -2.0f, -2.0f, -2.0f, -2.0f};
static const float tree_155_leaf_pos[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

static const int16_t tree_156_left[3] = {1, -1, -1};
static const int16_t tree_156_right[3] = {2, -1, -1};
static const int16_t tree_156_feature[3] = {8, -2, -2};
static const float tree_156_threshold[3] = {10.4044724f, -2.0f, -2.0f};
static const float tree_156_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_157_left[3] = {1, -1, -1};
static const int16_t tree_157_right[3] = {2, -1, -1};
static const int16_t tree_157_feature[3] = {9, -2, -2};
static const float tree_157_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_157_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_158_left[3] = {1, -1, -1};
static const int16_t tree_158_right[3] = {2, -1, -1};
static const int16_t tree_158_feature[3] = {9, -2, -2};
static const float tree_158_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_158_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

static const int16_t tree_159_left[3] = {1, -1, -1};
static const int16_t tree_159_right[3] = {2, -1, -1};
static const int16_t tree_159_feature[3] = {9, -2, -2};
static const float tree_159_threshold[3] = {0.770513773f, -2.0f, -2.0f};
static const float tree_159_leaf_pos[3] = {0.0f, 0.0f, 1.0f};

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
    sum_pos += arc_tree_predict_proba_pos(tree_120_left, tree_120_right, tree_120_feature, tree_120_threshold, tree_120_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_121_left, tree_121_right, tree_121_feature, tree_121_threshold, tree_121_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_122_left, tree_122_right, tree_122_feature, tree_122_threshold, tree_122_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_123_left, tree_123_right, tree_123_feature, tree_123_threshold, tree_123_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_124_left, tree_124_right, tree_124_feature, tree_124_threshold, tree_124_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_125_left, tree_125_right, tree_125_feature, tree_125_threshold, tree_125_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_126_left, tree_126_right, tree_126_feature, tree_126_threshold, tree_126_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_127_left, tree_127_right, tree_127_feature, tree_127_threshold, tree_127_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_128_left, tree_128_right, tree_128_feature, tree_128_threshold, tree_128_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_129_left, tree_129_right, tree_129_feature, tree_129_threshold, tree_129_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_130_left, tree_130_right, tree_130_feature, tree_130_threshold, tree_130_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_131_left, tree_131_right, tree_131_feature, tree_131_threshold, tree_131_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_132_left, tree_132_right, tree_132_feature, tree_132_threshold, tree_132_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_133_left, tree_133_right, tree_133_feature, tree_133_threshold, tree_133_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_134_left, tree_134_right, tree_134_feature, tree_134_threshold, tree_134_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_135_left, tree_135_right, tree_135_feature, tree_135_threshold, tree_135_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_136_left, tree_136_right, tree_136_feature, tree_136_threshold, tree_136_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_137_left, tree_137_right, tree_137_feature, tree_137_threshold, tree_137_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_138_left, tree_138_right, tree_138_feature, tree_138_threshold, tree_138_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_139_left, tree_139_right, tree_139_feature, tree_139_threshold, tree_139_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_140_left, tree_140_right, tree_140_feature, tree_140_threshold, tree_140_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_141_left, tree_141_right, tree_141_feature, tree_141_threshold, tree_141_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_142_left, tree_142_right, tree_142_feature, tree_142_threshold, tree_142_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_143_left, tree_143_right, tree_143_feature, tree_143_threshold, tree_143_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_144_left, tree_144_right, tree_144_feature, tree_144_threshold, tree_144_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_145_left, tree_145_right, tree_145_feature, tree_145_threshold, tree_145_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_146_left, tree_146_right, tree_146_feature, tree_146_threshold, tree_146_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_147_left, tree_147_right, tree_147_feature, tree_147_threshold, tree_147_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_148_left, tree_148_right, tree_148_feature, tree_148_threshold, tree_148_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_149_left, tree_149_right, tree_149_feature, tree_149_threshold, tree_149_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_150_left, tree_150_right, tree_150_feature, tree_150_threshold, tree_150_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_151_left, tree_151_right, tree_151_feature, tree_151_threshold, tree_151_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_152_left, tree_152_right, tree_152_feature, tree_152_threshold, tree_152_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_153_left, tree_153_right, tree_153_feature, tree_153_threshold, tree_153_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_154_left, tree_154_right, tree_154_feature, tree_154_threshold, tree_154_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_155_left, tree_155_right, tree_155_feature, tree_155_threshold, tree_155_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_156_left, tree_156_right, tree_156_feature, tree_156_threshold, tree_156_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_157_left, tree_157_right, tree_157_feature, tree_157_threshold, tree_157_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_158_left, tree_158_right, tree_158_feature, tree_158_threshold, tree_158_leaf_pos, input);
    sum_pos += arc_tree_predict_proba_pos(tree_159_left, tree_159_right, tree_159_feature, tree_159_threshold, tree_159_leaf_pos, input);
    const float pos = sum_pos / 160.0f;
    output[0] = (double)(1.0f - pos);
    output[1] = (double)pos;
}
