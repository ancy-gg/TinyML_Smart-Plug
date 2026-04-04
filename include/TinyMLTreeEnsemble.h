#pragma once
// Auto-generated C header from scikit-learn RandomForest (m2cgen)
#define ARC_MODEL_FEATURE_VERSION 3
#define ARC_THRESHOLD 0.7900

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

#include <string.h>
#include <string.h>
static inline void add_vectors(double *v1, double *v2, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] + v2[i];
}
static inline void mul_vector_number(double *v1, double num, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] * num;
}
static inline void set_output2(double *dst, double a, double b) {
    dst[0] = a;
    dst[1] = b;
}
static inline void arc_rf_predict(double * input, double * output) {
    double var0[2];
    double var1[2];
    double var2[2];
    double var3[2];
    double var4[2];
    double var5[2];
    double var6[2];
    double var7[2];
    double var8[2];
    double var9[2];
    double var10[2];
    double var11[2];
    double var12[2];
    double var13[2];
    double var14[2];
    double var15[2];
    double var16[2];
    double var17[2];
    double var18[2];
    double var19[2];
    double var20[2];
    double var21[2];
    double var22[2];
    double var23[2];
    double var24[2];
    double var25[2];
    double var26[2];
    double var27[2];
    double var28[2];
    double var29[2];
    double var30[2];
    double var31[2];
    double var32[2];
    double var33[2];
    double var34[2];
    double var35[2];
    double var36[2];
    double var37[2];
    double var38[2];
    double var39[2];
    double var40[2];
    double var41[2];
    double var42[2];
    double var43[2];
    double var44[2];
    double var45[2];
    double var46[2];
    double var47[2];
    double var48[2];
    double var49[2];
    double var50[2];
    double var51[2];
    double var52[2];
    double var53[2];
    double var54[2];
    double var55[2];
    double var56[2];
    double var57[2];
    double var58[2];
    double var59[2];
    double var60[2];
    double var61[2];
    double var62[2];
    double var63[2];
    double var64[2];
    double var65[2];
    double var66[2];
    double var67[2];
    double var68[2];
    double var69[2];
    double var70[2];
    double var71[2];
    double var72[2];
    double var73[2];
    double var74[2];
    double var75[2];
    double var76[2];
    double var77[2];
    double var78[2];
    double var79[2];
    double var80[2];
    double var81[2];
    double var82[2];
    double var83[2];
    double var84[2];
    double var85[2];
    double var86[2];
    double var87[2];
    double var88[2];
    double var89[2];
    double var90[2];
    double var91[2];
    double var92[2];
    double var93[2];
    double var94[2];
    double var95[2];
    double var96[2];
    double var97[2];
    double var98[2];
    double var99[2];
    double var100[2];
    double var101[2];
    double var102[2];
    double var103[2];
    double var104[2];
    double var105[2];
    double var106[2];
    double var107[2];
    double var108[2];
    double var109[2];
    double var110[2];
    double var111[2];
    double var112[2];
    double var113[2];
    double var114[2];
    double var115[2];
    double var116[2];
    double var117[2];
    double var118[2];
    double var119[2];
    double var120[2];
    if (input[4] <= 0.07754699885845184) {
        if (input[6] <= -1.89262455701828) {
            if (input[5] <= 0.011713000014424324) {
                set_output2(var120, 1.0, 0.0);
            } else {
                set_output2(var120, 0.8272921108742004, 0.17270788912579962);
            }
        } else {
            set_output2(var120, 0.0, 1.0);
        }
    } else {
        set_output2(var120, 0.0, 1.0);
    }
    double var121[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var121, 1.0, 0.0);
    } else {
        set_output2(var121, 0.0, 1.0);
    }
    add_vectors(var120, var121, 2, var119);
    double var122[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var122, 1.0, 0.0);
    } else {
        if (input[3] <= -5.405116558074951) {
            set_output2(var122, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var122, 0.0, 1.0);
        }
    }
    add_vectors(var119, var122, 2, var118);
    double var123[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[3] <= -9.598759651184082) {
            set_output2(var123, 1.0, 0.0);
        } else {
            set_output2(var123, 0.9482163406214039, 0.051783659378596095);
        }
    } else {
        set_output2(var123, 0.0, 1.0);
    }
    add_vectors(var118, var123, 2, var117);
    double var124[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var124, 1.0, 0.0);
        } else {
            set_output2(var124, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -4.815688133239746) {
            set_output2(var124, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var124, 0.0, 1.0);
        }
    }
    add_vectors(var117, var124, 2, var116);
    double var125[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[1] <= 11.03041124343872) {
            set_output2(var125, 1.0, 0.0);
        } else {
            set_output2(var125, 0.9458917835671343, 0.054108216432865744);
        }
    } else {
        set_output2(var125, 0.0, 1.0);
    }
    add_vectors(var116, var125, 2, var115);
    double var126[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var126, 1.0, 0.0);
    } else {
        set_output2(var126, 0.0, 1.0);
    }
    add_vectors(var115, var126, 2, var114);
    double var127[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var127, 0.9987824345896804, 0.001217565410319543);
    } else {
        set_output2(var127, 0.0, 1.0);
    }
    add_vectors(var114, var127, 2, var113);
    double var128[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var128, 1.0, 0.0);
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var128, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var128, 0.0, 1.0);
        }
    }
    add_vectors(var113, var128, 2, var112);
    double var129[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[0] <= 0.03955350071191788) {
            set_output2(var129, 1.0, 0.0);
        } else {
            set_output2(var129, 0.5863125638406538, 0.4136874361593463);
        }
    } else {
        if (input[1] <= 8.854729175567627) {
            set_output2(var129, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var129, 0.0, 1.0);
        }
    }
    add_vectors(var112, var129, 2, var111);
    double var130[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var130, 1.0, 0.0);
    } else {
        set_output2(var130, 0.0, 1.0);
    }
    add_vectors(var111, var130, 2, var110);
    double var131[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var131, 1.0, 0.0);
    } else {
        if (input[6] <= -3.2444640398025513) {
            set_output2(var131, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var131, 0.0, 1.0);
        }
    }
    add_vectors(var110, var131, 2, var109);
    double var132[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var132, 0.9987824345896805, 0.0012175654103195432);
    } else {
        set_output2(var132, 0.0, 1.0);
    }
    add_vectors(var109, var132, 2, var108);
    double var133[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var133, 1.0, 0.0);
        } else {
            set_output2(var133, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var133, 0.0, 1.0);
    }
    add_vectors(var108, var133, 2, var107);
    double var134[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var134, 1.0, 0.0);
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var134, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var134, 0.0, 1.0);
        }
    }
    add_vectors(var107, var134, 2, var106);
    double var135[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var135, 1.0, 0.0);
        } else {
            set_output2(var135, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 9.572930812835693) {
            set_output2(var135, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var135, 0.0, 1.0);
        }
    }
    add_vectors(var106, var135, 2, var105);
    double var136[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var136, 1.0, 0.0);
    } else {
        set_output2(var136, 0.0, 1.0);
    }
    add_vectors(var105, var136, 2, var104);
    double var137[2];
    if (input[4] <= 0.07754699885845184) {
        if (input[9] <= 2.229249596595764) {
            if (input[0] <= 0.038907499983906746) {
                set_output2(var137, 1.0, 0.0);
            } else {
                set_output2(var137, 0.7294589178356714, 0.2705410821643287);
            }
        } else {
            set_output2(var137, 0.0, 1.0);
        }
    } else {
        set_output2(var137, 0.0, 1.0);
    }
    add_vectors(var104, var137, 2, var103);
    double var138[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var138, 1.0, 0.0);
    } else {
        if (input[7] <= 0.11837499588727951) {
            set_output2(var138, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var138, 0.0, 1.0);
        }
    }
    add_vectors(var103, var138, 2, var102);
    double var139[2];
    if (input[9] <= 2.037452459335327) {
        if (input[6] <= -0.6762200593948364) {
            set_output2(var139, 0.9987795065590221, 0.0012204934409778408);
        } else {
            set_output2(var139, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 9.572930812835693) {
            set_output2(var139, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var139, 0.0, 1.0);
        }
    }
    add_vectors(var102, var139, 2, var101);
    double var140[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var140, 1.0, 0.0);
        } else {
            set_output2(var140, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -4.815688133239746) {
            set_output2(var140, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var140, 0.0, 1.0);
        }
    }
    add_vectors(var101, var140, 2, var100);
    double var141[2];
    if (input[5] <= 0.012003500014543533) {
        if (input[0] <= 0.04826050065457821) {
            set_output2(var141, 1.0, 0.0);
        } else {
            set_output2(var141, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var141, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var141, 0.0, 1.0);
        }
    }
    add_vectors(var100, var141, 2, var99);
    double var142[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var142, 1.0, 0.0);
    } else {
        if (input[4] <= 0.048498500138521194) {
            set_output2(var142, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var142, 0.0, 1.0);
        }
    }
    add_vectors(var99, var142, 2, var98);
    double var143[2];
    if (input[9] <= 2.037452459335327) {
        if (input[3] <= -6.88375997543335) {
            set_output2(var143, 1.0, 0.0);
        } else {
            set_output2(var143, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var143, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var143, 0.0, 1.0);
        }
    }
    add_vectors(var98, var143, 2, var97);
    double var144[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var144, 1.0, 0.0);
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var144, 0.18576598311218337, 0.8142340168878167);
        } else {
            set_output2(var144, 0.0, 1.0);
        }
    }
    add_vectors(var97, var144, 2, var96);
    double var145[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[5] <= 0.011713000014424324) {
            set_output2(var145, 1.0, 0.0);
        } else {
            set_output2(var145, 0.8272921108742004, 0.17270788912579962);
        }
    } else {
        if (input[9] <= 1.2207329869270325) {
            set_output2(var145, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var145, 0.0, 1.0);
        }
    }
    add_vectors(var96, var145, 2, var95);
    double var146[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var146, 1.0, 0.0);
    } else {
        set_output2(var146, 0.0, 1.0);
    }
    add_vectors(var95, var146, 2, var94);
    double var147[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var147, 1.0, 0.0);
        } else {
            set_output2(var147, 0.0, 1.0);
        }
    } else {
        if (input[6] <= -3.105059027671814) {
            set_output2(var147, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var147, 0.0, 1.0);
        }
    }
    add_vectors(var94, var147, 2, var93);
    double var148[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[1] <= 11.227663516998291) {
            set_output2(var148, 0.9987528983565972, 0.0012471016434028321);
        } else {
            set_output2(var148, 0.8909531502423264, 0.10904684975767369);
        }
    } else {
        if (input[3] <= -5.951765537261963) {
            set_output2(var148, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var148, 0.0, 1.0);
        }
    }
    add_vectors(var93, var148, 2, var92);
    double var149[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[0] <= 0.03955350071191788) {
            set_output2(var149, 1.0, 0.0);
        } else {
            set_output2(var149, 0.5863125638406537, 0.4136874361593463);
        }
    } else {
        if (input[7] <= 0.03630349854938686) {
            set_output2(var149, 0.43749999999999994, 0.5625000000000001);
        } else {
            set_output2(var149, 0.0, 1.0);
        }
    }
    add_vectors(var92, var149, 2, var91);
    double var150[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var150, 1.0, 0.0);
    } else {
        set_output2(var150, 0.0, 1.0);
    }
    add_vectors(var91, var150, 2, var90);
    double var151[2];
    if (input[8] <= 0.19659500569105148) {
        if (input[4] <= 0.07754699885845184) {
            if (input[6] <= -4.284214377403259) {
                set_output2(var151, 1.0, 0.0);
            } else {
                set_output2(var151, 0.8272921108742004, 0.17270788912579962);
            }
        } else {
            set_output2(var151, 0.0, 1.0);
        }
    } else {
        set_output2(var151, 0.0, 1.0);
    }
    add_vectors(var90, var151, 2, var89);
    double var152[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var152, 1.0, 0.0);
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var152, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var152, 0.0, 1.0);
        }
    }
    add_vectors(var89, var152, 2, var88);
    double var153[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var153, 1.0, 0.0);
    } else {
        if (input[1] <= 8.905364990234375) {
            set_output2(var153, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var153, 0.0, 1.0);
        }
    }
    add_vectors(var88, var153, 2, var87);
    double var154[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var154, 1.0, 0.0);
        } else {
            set_output2(var154, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var154, 0.0, 1.0);
    }
    add_vectors(var87, var154, 2, var86);
    double var155[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[7] <= 1.287097990512848) {
            set_output2(var155, 1.0, 0.0);
        } else {
            set_output2(var155, 0.8908649959579629, 0.1091350040420372);
        }
    } else {
        if (input[7] <= 0.03630349854938686) {
            set_output2(var155, 0.43749999999999994, 0.5625000000000001);
        } else {
            set_output2(var155, 0.0, 1.0);
        }
    }
    add_vectors(var86, var155, 2, var85);
    double var156[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var156, 1.0, 0.0);
    } else {
        if (input[3] <= -5.405116558074951) {
            set_output2(var156, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var156, 0.0, 1.0);
        }
    }
    add_vectors(var85, var156, 2, var84);
    double var157[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var157, 0.9987824345896805, 0.0012175654103195432);
    } else {
        set_output2(var157, 0.0, 1.0);
    }
    add_vectors(var84, var157, 2, var83);
    double var158[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var158, 1.0, 0.0);
    } else {
        if (input[6] <= -3.2444640398025513) {
            set_output2(var158, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var158, 0.0, 1.0);
        }
    }
    add_vectors(var83, var158, 2, var82);
    double var159[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var159, 1.0, 0.0);
    } else {
        set_output2(var159, 0.0, 1.0);
    }
    add_vectors(var82, var159, 2, var81);
    double var160[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var160, 1.0, 0.0);
    } else {
        if (input[1] <= 8.905364990234375) {
            set_output2(var160, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var160, 0.0, 1.0);
        }
    }
    add_vectors(var81, var160, 2, var80);
    double var161[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var161, 1.0, 0.0);
    } else {
        set_output2(var161, 0.0, 1.0);
    }
    add_vectors(var80, var161, 2, var79);
    double var162[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var162, 1.0, 0.0);
    } else {
        set_output2(var162, 0.0, 1.0);
    }
    add_vectors(var79, var162, 2, var78);
    double var163[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var163, 0.9987824345896805, 0.0012175654103195425);
    } else {
        set_output2(var163, 0.0, 1.0);
    }
    add_vectors(var78, var163, 2, var77);
    double var164[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var164, 0.9987824345896805, 0.0012175654103195425);
    } else {
        set_output2(var164, 0.0, 1.0);
    }
    add_vectors(var77, var164, 2, var76);
    double var165[2];
    if (input[4] <= 0.07754699885845184) {
        if (input[9] <= 2.229249596595764) {
            if (input[5] <= 0.01147049991413951) {
                set_output2(var165, 1.0, 0.0);
            } else {
                set_output2(var165, 0.8857868020304569, 0.11421319796954318);
            }
        } else {
            set_output2(var165, 0.0, 1.0);
        }
    } else {
        set_output2(var165, 0.0, 1.0);
    }
    add_vectors(var76, var165, 2, var75);
    double var166[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var166, 1.0, 0.0);
    } else {
        set_output2(var166, 0.0, 1.0);
    }
    add_vectors(var75, var166, 2, var74);
    double var167[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[0] <= 0.03955350071191788) {
            set_output2(var167, 1.0, 0.0);
        } else {
            set_output2(var167, 0.5863125638406538, 0.4136874361593463);
        }
    } else {
        if (input[9] <= 1.2207329869270325) {
            set_output2(var167, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var167, 0.0, 1.0);
        }
    }
    add_vectors(var74, var167, 2, var73);
    double var168[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var168, 1.0, 0.0);
    } else {
        set_output2(var168, 0.0, 1.0);
    }
    add_vectors(var73, var168, 2, var72);
    double var169[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var169, 0.9987824345896805, 0.001217565410319543);
    } else {
        set_output2(var169, 0.0, 1.0);
    }
    add_vectors(var72, var169, 2, var71);
    double var170[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[3] <= -9.598759651184082) {
            set_output2(var170, 1.0, 0.0);
        } else {
            set_output2(var170, 0.9482163406214039, 0.0517836593785961);
        }
    } else {
        set_output2(var170, 0.0, 1.0);
    }
    add_vectors(var71, var170, 2, var70);
    double var171[2];
    if (input[5] <= 0.012003500014543533) {
        if (input[2] <= -1.6648629307746887) {
            set_output2(var171, 1.0, 0.0);
        } else {
            set_output2(var171, 0.0, 1.0);
        }
    } else {
        if (input[8] <= 0.13853850215673447) {
            set_output2(var171, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var171, 0.0, 1.0);
        }
    }
    add_vectors(var70, var171, 2, var69);
    double var172[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var172, 1.0, 0.0);
    } else {
        set_output2(var172, 0.0, 1.0);
    }
    add_vectors(var69, var172, 2, var68);
    double var173[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var173, 1.0, 0.0);
    } else {
        set_output2(var173, 0.0, 1.0);
    }
    add_vectors(var68, var173, 2, var67);
    double var174[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var174, 1.0, 0.0);
    } else {
        if (input[9] <= 1.2207329869270325) {
            set_output2(var174, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var174, 0.0, 1.0);
        }
    }
    add_vectors(var67, var174, 2, var66);
    double var175[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var175, 1.0, 0.0);
    } else {
        if (input[4] <= 0.048498500138521194) {
            set_output2(var175, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var175, 0.0, 1.0);
        }
    }
    add_vectors(var66, var175, 2, var65);
    double var176[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var176, 1.0, 0.0);
        } else {
            set_output2(var176, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var176, 0.0, 1.0);
    }
    add_vectors(var65, var176, 2, var64);
    double var177[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var177, 0.9987824345896805, 0.0012175654103195432);
    } else {
        set_output2(var177, 0.0, 1.0);
    }
    add_vectors(var64, var177, 2, var63);
    double var178[2];
    if (input[4] <= 0.07754699885845184) {
        if (input[2] <= -1.8099299669265747) {
            set_output2(var178, 1.0, 0.0);
        } else {
            set_output2(var178, 0.0, 1.0);
        }
    } else {
        set_output2(var178, 0.0, 1.0);
    }
    add_vectors(var63, var178, 2, var62);
    double var179[2];
    if (input[9] <= 2.037452459335327) {
        if (input[8] <= 0.21198400110006332) {
            if (input[6] <= -4.816070795059204) {
                set_output2(var179, 1.0, 0.0);
            } else {
                set_output2(var179, 0.9276139410187668, 0.07238605898123325);
            }
        } else {
            set_output2(var179, 0.0, 1.0);
        }
    } else {
        if (input[6] <= -3.105059027671814) {
            set_output2(var179, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var179, 0.0, 1.0);
        }
    }
    add_vectors(var62, var179, 2, var61);
    double var180[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var180, 0.9987824345896805, 0.0012175654103195428);
    } else {
        set_output2(var180, 0.0, 1.0);
    }
    add_vectors(var61, var180, 2, var60);
    double var181[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var181, 1.0, 0.0);
    } else {
        set_output2(var181, 0.0, 1.0);
    }
    add_vectors(var60, var181, 2, var59);
    double var182[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var182, 1.0, 0.0);
    } else {
        set_output2(var182, 0.0, 1.0);
    }
    add_vectors(var59, var182, 2, var58);
    double var183[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[3] <= -8.460973739624023) {
            set_output2(var183, 1.0, 0.0);
        } else {
            set_output2(var183, 0.8272921108742004, 0.17270788912579962);
        }
    } else {
        if (input[1] <= 8.854729175567627) {
            set_output2(var183, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var183, 0.0, 1.0);
        }
    }
    add_vectors(var58, var183, 2, var57);
    double var184[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[1] <= 11.03041124343872) {
            set_output2(var184, 1.0, 0.0);
        } else {
            set_output2(var184, 0.9458917835671343, 0.054108216432865744);
        }
    } else {
        set_output2(var184, 0.0, 1.0);
    }
    add_vectors(var57, var184, 2, var56);
    double var185[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var185, 1.0, 0.0);
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var185, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var185, 0.0, 1.0);
        }
    }
    add_vectors(var56, var185, 2, var55);
    double var186[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[1] <= 11.03041124343872) {
            set_output2(var186, 1.0, 0.0);
        } else {
            set_output2(var186, 0.9458917835671343, 0.05410821643286575);
        }
    } else {
        set_output2(var186, 0.0, 1.0);
    }
    add_vectors(var55, var186, 2, var54);
    double var187[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var187, 1.0, 0.0);
    } else {
        set_output2(var187, 0.0, 1.0);
    }
    add_vectors(var54, var187, 2, var53);
    double var188[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var188, 1.0, 0.0);
    } else {
        set_output2(var188, 0.0, 1.0);
    }
    add_vectors(var53, var188, 2, var52);
    double var189[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var189, 1.0, 0.0);
        } else {
            set_output2(var189, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var189, 0.0, 1.0);
    }
    add_vectors(var52, var189, 2, var51);
    double var190[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var190, 1.0, 0.0);
        } else {
            set_output2(var190, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var190, 0.0, 1.0);
    }
    add_vectors(var51, var190, 2, var50);
    double var191[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var191, 1.0, 0.0);
    } else {
        set_output2(var191, 0.0, 1.0);
    }
    add_vectors(var50, var191, 2, var49);
    double var192[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var192, 1.0, 0.0);
    } else {
        if (input[9] <= 1.2207329869270325) {
            set_output2(var192, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var192, 0.0, 1.0);
        }
    }
    add_vectors(var49, var192, 2, var48);
    double var193[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var193, 0.9987824345896805, 0.0012175654103195432);
    } else {
        set_output2(var193, 0.0, 1.0);
    }
    add_vectors(var48, var193, 2, var47);
    double var194[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var194, 1.0, 0.0);
        } else {
            set_output2(var194, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var194, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var194, 0.0, 1.0);
        }
    }
    add_vectors(var47, var194, 2, var46);
    double var195[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var195, 1.0, 0.0);
        } else {
            set_output2(var195, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var195, 0.0, 1.0);
    }
    add_vectors(var46, var195, 2, var45);
    double var196[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var196, 1.0, 0.0);
    } else {
        set_output2(var196, 0.0, 1.0);
    }
    add_vectors(var45, var196, 2, var44);
    double var197[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var197, 1.0, 0.0);
        } else {
            set_output2(var197, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var197, 0.0, 1.0);
    }
    add_vectors(var44, var197, 2, var43);
    double var198[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var198, 0.9987824345896805, 0.0012175654103195434);
    } else {
        set_output2(var198, 0.0, 1.0);
    }
    add_vectors(var43, var198, 2, var42);
    double var199[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[3] <= -8.460973739624023) {
            set_output2(var199, 1.0, 0.0);
        } else {
            set_output2(var199, 0.8272921108742004, 0.17270788912579962);
        }
    } else {
        if (input[7] <= 0.03630349854938686) {
            set_output2(var199, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var199, 0.0, 1.0);
        }
    }
    add_vectors(var42, var199, 2, var41);
    double var200[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var200, 1.0, 0.0);
    } else {
        set_output2(var200, 0.0, 1.0);
    }
    add_vectors(var41, var200, 2, var40);
    double var201[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var201, 1.0, 0.0);
        } else {
            set_output2(var201, 0.0, 1.0);
        }
    } else {
        if (input[6] <= -3.105059027671814) {
            set_output2(var201, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var201, 0.0, 1.0);
        }
    }
    add_vectors(var40, var201, 2, var39);
    double var202[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var202, 1.0, 0.0);
    } else {
        set_output2(var202, 0.0, 1.0);
    }
    add_vectors(var39, var202, 2, var38);
    double var203[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[0] <= 0.03811050020158291) {
            set_output2(var203, 1.0, 0.0);
        } else {
            set_output2(var203, 0.8672566371681416, 0.13274336283185842);
        }
    } else {
        set_output2(var203, 0.0, 1.0);
    }
    add_vectors(var38, var203, 2, var37);
    double var204[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var204, 1.0, 0.0);
    } else {
        set_output2(var204, 0.0, 1.0);
    }
    add_vectors(var37, var204, 2, var36);
    double var205[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var205, 1.0, 0.0);
    } else {
        set_output2(var205, 0.0, 1.0);
    }
    add_vectors(var36, var205, 2, var35);
    double var206[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var206, 1.0, 0.0);
    } else {
        set_output2(var206, 0.0, 1.0);
    }
    add_vectors(var35, var206, 2, var34);
    double var207[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var207, 1.0, 0.0);
        } else {
            set_output2(var207, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 9.572930812835693) {
            set_output2(var207, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var207, 0.0, 1.0);
        }
    }
    add_vectors(var34, var207, 2, var33);
    double var208[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var208, 1.0, 0.0);
        } else {
            set_output2(var208, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 9.572930812835693) {
            set_output2(var208, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var208, 0.0, 1.0);
        }
    }
    add_vectors(var33, var208, 2, var32);
    double var209[2];
    if (input[9] <= 2.037452459335327) {
        if (input[2] <= -0.8140194416046143) {
            set_output2(var209, 1.0, 0.0);
        } else {
            set_output2(var209, 0.0, 1.0);
        }
    } else {
        if (input[6] <= -3.105059027671814) {
            set_output2(var209, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var209, 0.0, 1.0);
        }
    }
    add_vectors(var32, var209, 2, var31);
    double var210[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var210, 1.0, 0.0);
    } else {
        set_output2(var210, 0.0, 1.0);
    }
    add_vectors(var31, var210, 2, var30);
    double var211[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var211, 1.0, 0.0);
    } else {
        set_output2(var211, 0.0, 1.0);
    }
    add_vectors(var30, var211, 2, var29);
    double var212[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[1] <= 11.03041124343872) {
            set_output2(var212, 1.0, 0.0);
        } else {
            set_output2(var212, 0.9458917835671343, 0.05410821643286575);
        }
    } else {
        set_output2(var212, 0.0, 1.0);
    }
    add_vectors(var29, var212, 2, var28);
    double var213[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var213, 1.0, 0.0);
    } else {
        set_output2(var213, 0.0, 1.0);
    }
    add_vectors(var28, var213, 2, var27);
    double var214[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[7] <= 1.287097990512848) {
            set_output2(var214, 1.0, 0.0);
        } else {
            set_output2(var214, 0.8908649959579629, 0.1091350040420372);
        }
    } else {
        if (input[1] <= 8.854729175567627) {
            set_output2(var214, 0.43749999999999994, 0.5625000000000001);
        } else {
            set_output2(var214, 0.0, 1.0);
        }
    }
    add_vectors(var27, var214, 2, var26);
    double var215[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[0] <= 0.03955350071191788) {
            set_output2(var215, 1.0, 0.0);
        } else {
            set_output2(var215, 0.5863125638406537, 0.4136874361593463);
        }
    } else {
        if (input[3] <= -5.951765537261963) {
            set_output2(var215, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var215, 0.0, 1.0);
        }
    }
    add_vectors(var26, var215, 2, var25);
    double var216[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var216, 1.0, 0.0);
    } else {
        set_output2(var216, 0.0, 1.0);
    }
    add_vectors(var25, var216, 2, var24);
    double var217[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[1] <= 11.227663516998291) {
            if (input[3] <= -9.598759651184082) {
                set_output2(var217, 1.0, 0.0);
            } else {
                set_output2(var217, 0.9482163406214039, 0.0517836593785961);
            }
        } else {
            set_output2(var217, 0.8909531502423264, 0.10904684975767369);
        }
    } else {
        if (input[3] <= -5.951765537261963) {
            set_output2(var217, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var217, 0.0, 1.0);
        }
    }
    add_vectors(var24, var217, 2, var23);
    double var218[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[0] <= 0.03955350071191788) {
            set_output2(var218, 1.0, 0.0);
        } else {
            set_output2(var218, 0.5863125638406538, 0.4136874361593463);
        }
    } else {
        if (input[1] <= 8.854729175567627) {
            set_output2(var218, 0.43749999999999994, 0.5625000000000001);
        } else {
            set_output2(var218, 0.0, 1.0);
        }
    }
    add_vectors(var23, var218, 2, var22);
    double var219[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var219, 1.0, 0.0);
    } else {
        if (input[1] <= 8.905364990234375) {
            set_output2(var219, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var219, 0.0, 1.0);
        }
    }
    add_vectors(var22, var219, 2, var21);
    double var220[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var220, 1.0, 0.0);
    } else {
        set_output2(var220, 0.0, 1.0);
    }
    add_vectors(var21, var220, 2, var20);
    double var221[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var221, 1.0, 0.0);
    } else {
        set_output2(var221, 0.0, 1.0);
    }
    add_vectors(var20, var221, 2, var19);
    double var222[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var222, 1.0, 0.0);
    } else {
        if (input[1] <= 8.905364990234375) {
            set_output2(var222, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var222, 0.0, 1.0);
        }
    }
    add_vectors(var19, var222, 2, var18);
    double var223[2];
    if (input[4] <= 0.07754699885845184) {
        if (input[2] <= -1.8099299669265747) {
            set_output2(var223, 1.0, 0.0);
        } else {
            set_output2(var223, 0.0, 1.0);
        }
    } else {
        set_output2(var223, 0.0, 1.0);
    }
    add_vectors(var18, var223, 2, var17);
    double var224[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var224, 1.0, 0.0);
    } else {
        set_output2(var224, 0.0, 1.0);
    }
    add_vectors(var17, var224, 2, var16);
    double var225[2];
    if (input[0] <= 0.04826050065457821) {
        if (input[3] <= -9.598759651184082) {
            set_output2(var225, 1.0, 0.0);
        } else {
            set_output2(var225, 0.9482163406214039, 0.0517836593785961);
        }
    } else {
        set_output2(var225, 0.0, 1.0);
    }
    add_vectors(var16, var225, 2, var15);
    double var226[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[0] <= 0.03955350071191788) {
            set_output2(var226, 1.0, 0.0);
        } else {
            set_output2(var226, 0.5863125638406537, 0.4136874361593463);
        }
    } else {
        if (input[9] <= 1.2207329869270325) {
            set_output2(var226, 0.43749999999999994, 0.5625000000000001);
        } else {
            set_output2(var226, 0.0, 1.0);
        }
    }
    add_vectors(var15, var226, 2, var14);
    double var227[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var227, 1.0, 0.0);
    } else {
        set_output2(var227, 0.0, 1.0);
    }
    add_vectors(var14, var227, 2, var13);
    double var228[2];
    if (input[9] <= 2.037452459335327) {
        if (input[0] <= 0.06066049821674824) {
            set_output2(var228, 1.0, 0.0);
        } else {
            set_output2(var228, 0.0, 1.0);
        }
    } else {
        if (input[6] <= -3.105059027671814) {
            set_output2(var228, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var228, 0.0, 1.0);
        }
    }
    add_vectors(var13, var228, 2, var12);
    double var229[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var229, 1.0, 0.0);
    } else {
        set_output2(var229, 0.0, 1.0);
    }
    add_vectors(var12, var229, 2, var11);
    double var230[2];
    if (input[0] <= 0.04826050065457821) {
        set_output2(var230, 0.9987824345896805, 0.0012175654103195434);
    } else {
        set_output2(var230, 0.0, 1.0);
    }
    add_vectors(var11, var230, 2, var10);
    double var231[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var231, 1.0, 0.0);
    } else {
        set_output2(var231, 0.0, 1.0);
    }
    add_vectors(var10, var231, 2, var9);
    double var232[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var232, 1.0, 0.0);
    } else {
        if (input[4] <= 0.048498500138521194) {
            set_output2(var232, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var232, 0.0, 1.0);
        }
    }
    add_vectors(var9, var232, 2, var8);
    double var233[2];
    if (input[3] <= -7.178012132644653) {
        set_output2(var233, 1.0, 0.0);
    } else {
        set_output2(var233, 0.0, 1.0);
    }
    add_vectors(var8, var233, 2, var7);
    double var234[2];
    if (input[4] <= 0.07754699885845184) {
        if (input[2] <= -1.8099299669265747) {
            set_output2(var234, 1.0, 0.0);
        } else {
            set_output2(var234, 0.0, 1.0);
        }
    } else {
        set_output2(var234, 0.0, 1.0);
    }
    add_vectors(var7, var234, 2, var6);
    double var235[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[2] <= -4.027828574180603) {
            set_output2(var235, 1.0, 0.0);
        } else {
            set_output2(var235, 0.8874687413170326, 0.1125312586829675);
        }
    } else {
        if (input[2] <= -2.1490129828453064) {
            set_output2(var235, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var235, 0.0, 1.0);
        }
    }
    add_vectors(var6, var235, 2, var5);
    double var236[2];
    if (input[9] <= 2.037452459335327) {
        if (input[0] <= 0.06066049821674824) {
            set_output2(var236, 1.0, 0.0);
        } else {
            set_output2(var236, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -1.5876445174217224) {
            set_output2(var236, 0.28267800212539845, 0.7173219978746016);
        } else {
            set_output2(var236, 0.0, 1.0);
        }
    }
    add_vectors(var5, var236, 2, var4);
    double var237[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[0] <= 0.03955350071191788) {
            set_output2(var237, 1.0, 0.0);
        } else {
            set_output2(var237, 0.5863125638406538, 0.4136874361593463);
        }
    } else {
        if (input[9] <= 1.2207329869270325) {
            set_output2(var237, 0.4375, 0.5625000000000001);
        } else {
            set_output2(var237, 0.0, 1.0);
        }
    }
    add_vectors(var4, var237, 2, var3);
    double var238[2];
    if (input[2] <= -3.4005664587020874) {
        set_output2(var238, 1.0, 0.0);
    } else {
        if (input[0] <= 0.06341450102627277) {
            set_output2(var238, 0.18576598311218334, 0.8142340168878166);
        } else {
            set_output2(var238, 0.0, 1.0);
        }
    }
    add_vectors(var3, var238, 2, var2);
    double var239[2];
    if (input[6] <= -3.7013875246047974) {
        if (input[5] <= 0.011713000014424324) {
            set_output2(var239, 1.0, 0.0);
        } else {
            set_output2(var239, 0.8272921108742004, 0.17270788912579962);
        }
    } else {
        if (input[2] <= -2.1490129828453064) {
            set_output2(var239, 0.43749999999999994, 0.5625000000000001);
        } else {
            set_output2(var239, 0.0, 1.0);
        }
    }
    add_vectors(var2, var239, 2, var1);
    mul_vector_number(var1, 0.008333333333333333, 2, var0);
    memcpy(output, var0, 2 * sizeof(double));
}
