#pragma once
// Auto-generated C header from scikit-learn ExtraTrees (m2cgen)
#define ARC_MODEL_FEATURE_VERSION 4
#define ARC_THRESHOLD 0.9300

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
    double var121[2];
    double var122[2];
    double var123[2];
    double var124[2];
    double var125[2];
    double var126[2];
    double var127[2];
    double var128[2];
    double var129[2];
    double var130[2];
    double var131[2];
    double var132[2];
    double var133[2];
    double var134[2];
    double var135[2];
    double var136[2];
    double var137[2];
    double var138[2];
    double var139[2];
    double var140[2];
    double var141[2];
    double var142[2];
    double var143[2];
    double var144[2];
    double var145[2];
    double var146[2];
    double var147[2];
    double var148[2];
    double var149[2];
    double var150[2];
    double var151[2];
    double var152[2];
    double var153[2];
    double var154[2];
    double var155[2];
    double var156[2];
    double var157[2];
    double var158[2];
    double var159[2];
    double var160[2];
    double var161[2];
    double var162[2];
    double var163[2];
    double var164[2];
    double var165[2];
    double var166[2];
    double var167[2];
    double var168[2];
    double var169[2];
    double var170[2];
    double var171[2];
    double var172[2];
    double var173[2];
    double var174[2];
    double var175[2];
    double var176[2];
    double var177[2];
    double var178[2];
    double var179[2];
    double var180[2];
    double var181[2];
    double var182[2];
    double var183[2];
    double var184[2];
    double var185[2];
    double var186[2];
    double var187[2];
    double var188[2];
    double var189[2];
    double var190[2];
    double var191[2];
    double var192[2];
    double var193[2];
    double var194[2];
    double var195[2];
    double var196[2];
    double var197[2];
    double var198[2];
    double var199[2];
    double var200[2];
    double var201[2];
    double var202[2];
    double var203[2];
    double var204[2];
    double var205[2];
    double var206[2];
    double var207[2];
    double var208[2];
    double var209[2];
    double var210[2];
    double var211[2];
    double var212[2];
    double var213[2];
    double var214[2];
    double var215[2];
    double var216[2];
    double var217[2];
    double var218[2];
    double var219[2];
    double var220[2];
    double var221[2];
    double var222[2];
    double var223[2];
    double var224[2];
    double var225[2];
    double var226[2];
    double var227[2];
    double var228[2];
    double var229[2];
    double var230[2];
    double var231[2];
    double var232[2];
    double var233[2];
    double var234[2];
    double var235[2];
    double var236[2];
    double var237[2];
    double var238[2];
    double var239[2];
    double var240[2];
    double var241[2];
    double var242[2];
    double var243[2];
    double var244[2];
    double var245[2];
    double var246[2];
    double var247[2];
    double var248[2];
    double var249[2];
    double var250[2];
    double var251[2];
    double var252[2];
    double var253[2];
    double var254[2];
    double var255[2];
    double var256[2];
    double var257[2];
    double var258[2];
    double var259[2];
    double var260[2];
    double var261[2];
    double var262[2];
    double var263[2];
    double var264[2];
    double var265[2];
    double var266[2];
    double var267[2];
    double var268[2];
    double var269[2];
    double var270[2];
    double var271[2];
    double var272[2];
    double var273[2];
    double var274[2];
    double var275[2];
    double var276[2];
    double var277[2];
    double var278[2];
    double var279[2];
    double var280[2];
    if (input[1] <= 8.900599032679938) {
        if (input[7] <= 2.2067762577880936) {
            set_output2(var280, 1.0, 0.0);
        } else {
            set_output2(var280, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        if (input[4] <= 8.004758808623285) {
            set_output2(var280, 0.32217573221757323, 0.6778242677824268);
        } else {
            set_output2(var280, 0.0, 1.0);
        }
    }
    double var281[2];
    if (input[0] <= 7.13477831492735) {
        if (input[7] <= -0.5857373726446284) {
            set_output2(var281, 1.0, 0.0);
        } else {
            if (input[7] <= 0.05234735320875361) {
                set_output2(var281, 1.0, 0.0);
            } else {
                if (input[3] <= -33.142455863542665) {
                    set_output2(var281, 1.0, 0.0);
                } else {
                    if (input[9] <= 1.0197548146244981) {
                        set_output2(var281, 1.0, 0.0);
                    } else {
                        set_output2(var281, 0.32217573221757323, 0.6778242677824268);
                    }
                }
            }
        }
    } else {
        set_output2(var281, 0.0, 1.0);
    }
    add_vectors(var280, var281, 2, var279);
    double var282[2];
    if (input[8] <= 0.6524965230731189) {
        set_output2(var282, 1.0, 0.0);
    } else {
        set_output2(var282, 0.0, 1.0);
    }
    add_vectors(var279, var282, 2, var278);
    double var283[2];
    if (input[5] <= 13.065423310057826) {
        if (input[1] <= 9.99469751925408) {
            set_output2(var283, 1.0, 0.0);
        } else {
            set_output2(var283, 0.0, 1.0);
        }
    } else {
        set_output2(var283, 0.0, 1.0);
    }
    add_vectors(var278, var283, 2, var277);
    double var284[2];
    if (input[2] <= -19.363000885791656) {
        set_output2(var284, 1.0, 0.0);
    } else {
        set_output2(var284, 0.0, 1.0);
    }
    add_vectors(var277, var284, 2, var276);
    double var285[2];
    if (input[5] <= 10.57904185508639) {
        if (input[1] <= 7.706605959369893) {
            set_output2(var285, 1.0, 0.0);
        } else {
            if (input[8] <= 0.3508294534578755) {
                set_output2(var285, 1.0, 0.0);
            } else {
                set_output2(var285, 0.7403846153846154, 0.25961538461538464);
            }
        }
    } else {
        set_output2(var285, 0.0, 1.0);
    }
    add_vectors(var276, var285, 2, var275);
    double var286[2];
    if (input[2] <= -20.68143333303975) {
        set_output2(var286, 1.0, 0.0);
    } else {
        set_output2(var286, 0.0, 1.0);
    }
    add_vectors(var275, var286, 2, var274);
    double var287[2];
    if (input[2] <= -22.21151289166022) {
        set_output2(var287, 1.0, 0.0);
    } else {
        set_output2(var287, 0.0, 1.0);
    }
    add_vectors(var274, var287, 2, var273);
    double var288[2];
    if (input[8] <= 0.5541001896225488) {
        set_output2(var288, 1.0, 0.0);
    } else {
        set_output2(var288, 0.0, 1.0);
    }
    add_vectors(var273, var288, 2, var272);
    double var289[2];
    if (input[4] <= 3.3033971974999865) {
        set_output2(var289, 1.0, 0.0);
    } else {
        set_output2(var289, 0.0, 1.0);
    }
    add_vectors(var272, var289, 2, var271);
    double var290[2];
    if (input[0] <= 4.357577588516333) {
        if (input[6] <= 12.200605480998624) {
            set_output2(var290, 1.0, 0.0);
        } else {
            set_output2(var290, 0.9961219897543927, 0.003878010245607311);
        }
    } else {
        set_output2(var290, 0.0, 1.0);
    }
    add_vectors(var271, var290, 2, var270);
    double var291[2];
    if (input[0] <= 2.8618088932045347) {
        set_output2(var291, 1.0, 0.0);
    } else {
        set_output2(var291, 0.0, 1.0);
    }
    add_vectors(var270, var291, 2, var269);
    double var292[2];
    if (input[5] <= 10.702060564789104) {
        if (input[9] <= 1.6241639197140842) {
            set_output2(var292, 1.0, 0.0);
        } else {
            set_output2(var292, 0.45084745762711853, 0.5491525423728815);
        }
    } else {
        set_output2(var292, 0.0, 1.0);
    }
    add_vectors(var269, var292, 2, var268);
    double var293[2];
    if (input[0] <= 3.4342910507361335) {
        set_output2(var293, 1.0, 0.0);
    } else {
        set_output2(var293, 0.0, 1.0);
    }
    add_vectors(var268, var293, 2, var267);
    double var294[2];
    if (input[1] <= 9.948103576087108) {
        if (input[2] <= -23.3048926221109) {
            set_output2(var294, 1.0, 0.0);
        } else {
            set_output2(var294, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        set_output2(var294, 0.0, 1.0);
    }
    add_vectors(var267, var294, 2, var266);
    double var295[2];
    if (input[4] <= 5.619751520233274) {
        if (input[9] <= 1.3959129650920572) {
            set_output2(var295, 1.0, 0.0);
        } else {
            if (input[4] <= 1.2015484835673576) {
                set_output2(var295, 1.0, 0.0);
            } else {
                set_output2(var295, 0.9117647058823529, 0.08823529411764706);
            }
        }
    } else {
        set_output2(var295, 0.0, 1.0);
    }
    add_vectors(var266, var295, 2, var265);
    double var296[2];
    if (input[5] <= 7.261846102403219) {
        set_output2(var296, 1.0, 0.0);
    } else {
        set_output2(var296, 0.0, 1.0);
    }
    add_vectors(var265, var296, 2, var264);
    double var297[2];
    if (input[4] <= 2.92187918040829) {
        set_output2(var297, 1.0, 0.0);
    } else {
        set_output2(var297, 0.0, 1.0);
    }
    add_vectors(var264, var297, 2, var263);
    double var298[2];
    if (input[4] <= 4.627294352706058) {
        set_output2(var298, 1.0, 0.0);
    } else {
        set_output2(var298, 0.0, 1.0);
    }
    add_vectors(var263, var298, 2, var262);
    double var299[2];
    if (input[8] <= 0.5273009706431141) {
        set_output2(var299, 1.0, 0.0);
    } else {
        set_output2(var299, 0.0, 1.0);
    }
    add_vectors(var262, var299, 2, var261);
    double var300[2];
    if (input[3] <= -31.425502312216345) {
        set_output2(var300, 1.0, 0.0);
    } else {
        if (input[7] <= 2.6632536056634697) {
            set_output2(var300, 0.824295010845987, 0.17570498915401303);
        } else {
            set_output2(var300, 0.0, 1.0);
        }
    }
    add_vectors(var261, var300, 2, var260);
    double var301[2];
    if (input[0] <= 3.880888997224627) {
        set_output2(var301, 1.0, 0.0);
    } else {
        set_output2(var301, 0.0, 1.0);
    }
    add_vectors(var260, var301, 2, var259);
    double var302[2];
    if (input[8] <= 0.4303486518348284) {
        set_output2(var302, 1.0, 0.0);
    } else {
        if (input[5] <= 6.249711794349253) {
            set_output2(var302, 1.0, 0.0);
        } else {
            set_output2(var302, 0.0, 1.0);
        }
    }
    add_vectors(var259, var302, 2, var258);
    double var303[2];
    if (input[0] <= 7.316239029015512) {
        if (input[4] <= 1.87385609259579) {
            set_output2(var303, 1.0, 0.0);
        } else {
            set_output2(var303, 0.6523605150214592, 0.3476394849785408);
        }
    } else {
        set_output2(var303, 0.0, 1.0);
    }
    add_vectors(var258, var303, 2, var257);
    double var304[2];
    if (input[4] <= 1.8231911880384286) {
        set_output2(var304, 1.0, 0.0);
    } else {
        if (input[8] <= 0.6360053474404196) {
            set_output2(var304, 1.0, 0.0);
        } else {
            set_output2(var304, 0.0, 1.0);
        }
    }
    add_vectors(var257, var304, 2, var256);
    double var305[2];
    if (input[9] <= 3.269165498228991) {
        if (input[3] <= -28.67723897212609) {
            set_output2(var305, 1.0, 0.0);
        } else {
            set_output2(var305, 0.0, 1.0);
        }
    } else {
        set_output2(var305, 0.0, 1.0);
    }
    add_vectors(var256, var305, 2, var255);
    double var306[2];
    if (input[0] <= 2.4309228326564294) {
        set_output2(var306, 1.0, 0.0);
    } else {
        set_output2(var306, 0.0, 1.0);
    }
    add_vectors(var255, var306, 2, var254);
    double var307[2];
    if (input[8] <= 0.5538069169819015) {
        set_output2(var307, 1.0, 0.0);
    } else {
        set_output2(var307, 0.0, 1.0);
    }
    add_vectors(var254, var307, 2, var253);
    double var308[2];
    if (input[4] <= 3.721883228014977) {
        set_output2(var308, 1.0, 0.0);
    } else {
        set_output2(var308, 0.0, 1.0);
    }
    add_vectors(var253, var308, 2, var252);
    double var309[2];
    if (input[5] <= 6.500467919689652) {
        set_output2(var309, 1.0, 0.0);
    } else {
        if (input[4] <= 11.246229118691536) {
            if (input[5] <= 12.832389032480322) {
                set_output2(var309, 0.32217573221757323, 0.6778242677824268);
            } else {
                set_output2(var309, 0.0, 1.0);
            }
        } else {
            set_output2(var309, 0.0, 1.0);
        }
    }
    add_vectors(var252, var309, 2, var251);
    double var310[2];
    if (input[3] <= -30.31407233642283) {
        set_output2(var310, 1.0, 0.0);
    } else {
        set_output2(var310, 0.0, 1.0);
    }
    add_vectors(var251, var310, 2, var250);
    double var311[2];
    if (input[0] <= 1.3231641892699608) {
        set_output2(var311, 1.0, 0.0);
    } else {
        if (input[5] <= 11.966178053226896) {
            if (input[3] <= -30.65896553902669) {
                set_output2(var311, 1.0, 0.0);
            } else {
                set_output2(var311, 0.0, 1.0);
            }
        } else {
            if (input[6] <= 50.807253290637405) {
                set_output2(var311, 0.0, 1.0);
            } else {
                set_output2(var311, 0.0, 1.0);
            }
        }
    }
    add_vectors(var250, var311, 2, var249);
    double var312[2];
    if (input[2] <= -19.487560613153544) {
        set_output2(var312, 1.0, 0.0);
    } else {
        set_output2(var312, 0.0, 1.0);
    }
    add_vectors(var249, var312, 2, var248);
    double var313[2];
    if (input[4] <= 7.299741042024406) {
        if (input[3] <= -32.835127715025166) {
            set_output2(var313, 1.0, 0.0);
        } else {
            if (input[9] <= 0.9772694915464146) {
                set_output2(var313, 1.0, 0.0);
            } else {
                set_output2(var313, 0.8089622641509433, 0.19103773584905664);
            }
        }
    } else {
        set_output2(var313, 0.0, 1.0);
    }
    add_vectors(var248, var313, 2, var247);
    double var314[2];
    if (input[2] <= -26.87345814679001) {
        set_output2(var314, 1.0, 0.0);
    } else {
        if (input[5] <= 10.426358487954653) {
            set_output2(var314, 0.8693548387096773, 0.13064516129032258);
        } else {
            set_output2(var314, 0.0, 1.0);
        }
    }
    add_vectors(var247, var314, 2, var246);
    double var315[2];
    if (input[0] <= 1.6951727761911775) {
        set_output2(var315, 1.0, 0.0);
    } else {
        if (input[0] <= 10.22886570056504) {
            if (input[2] <= -16.208927421175297) {
                set_output2(var315, 0.32217573221757323, 0.6778242677824268);
            } else {
                set_output2(var315, 0.0, 1.0);
            }
        } else {
            set_output2(var315, 0.0, 1.0);
        }
    }
    add_vectors(var246, var315, 2, var245);
    double var316[2];
    if (input[2] <= -19.07136567904416) {
        set_output2(var316, 1.0, 0.0);
    } else {
        set_output2(var316, 0.0, 1.0);
    }
    add_vectors(var245, var316, 2, var244);
    double var317[2];
    if (input[0] <= 2.7876789984319825) {
        set_output2(var317, 1.0, 0.0);
    } else {
        set_output2(var317, 0.0, 1.0);
    }
    add_vectors(var244, var317, 2, var243);
    double var318[2];
    if (input[4] <= 3.2133202410061172) {
        set_output2(var318, 1.0, 0.0);
    } else {
        set_output2(var318, 0.0, 1.0);
    }
    add_vectors(var243, var318, 2, var242);
    double var319[2];
    if (input[4] <= 5.074436459118037) {
        set_output2(var319, 1.0, 0.0);
    } else {
        set_output2(var319, 0.0, 1.0);
    }
    add_vectors(var242, var319, 2, var241);
    double var320[2];
    if (input[8] <= 0.6736435207247853) {
        set_output2(var320, 1.0, 0.0);
    } else {
        set_output2(var320, 0.0, 1.0);
    }
    add_vectors(var241, var320, 2, var240);
    double var321[2];
    if (input[2] <= -17.457307292028823) {
        if (input[8] <= 0.4208021222323705) {
            set_output2(var321, 1.0, 0.0);
        } else {
            if (input[7] <= 0.7875745898258258) {
                set_output2(var321, 1.0, 0.0);
            } else {
                set_output2(var321, 0.8693548387096773, 0.13064516129032258);
            }
        }
    } else {
        set_output2(var321, 0.0, 1.0);
    }
    add_vectors(var240, var321, 2, var239);
    double var322[2];
    if (input[1] <= 8.162354026420251) {
        set_output2(var322, 1.0, 0.0);
    } else {
        if (input[5] <= 7.657292168379584) {
            set_output2(var322, 1.0, 0.0);
        } else {
            set_output2(var322, 0.0, 1.0);
        }
    }
    add_vectors(var239, var322, 2, var238);
    double var323[2];
    if (input[2] <= -20.764117012436262) {
        set_output2(var323, 1.0, 0.0);
    } else {
        set_output2(var323, 0.0, 1.0);
    }
    add_vectors(var238, var323, 2, var237);
    double var324[2];
    if (input[4] <= 1.8761470160377367) {
        set_output2(var324, 1.0, 0.0);
    } else {
        if (input[1] <= 12.101646380656339) {
            set_output2(var324, 0.7011070110701106, 0.2988929889298893);
        } else {
            set_output2(var324, 0.0, 1.0);
        }
    }
    add_vectors(var237, var324, 2, var236);
    double var325[2];
    if (input[1] <= 8.069957085092224) {
        set_output2(var325, 1.0, 0.0);
    } else {
        if (input[4] <= 4.278727608073584) {
            set_output2(var325, 1.0, 0.0);
        } else {
            set_output2(var325, 0.0, 1.0);
        }
    }
    add_vectors(var236, var325, 2, var235);
    double var326[2];
    if (input[8] <= 0.9260410621215925) {
        if (input[0] <= 6.1958549758990475) {
            if (input[7] <= 0.3777252159525055) {
                set_output2(var326, 1.0, 0.0);
            } else {
                if (input[1] <= 7.9912615094755095) {
                    set_output2(var326, 1.0, 0.0);
                } else {
                    set_output2(var326, 0.7403846153846154, 0.25961538461538464);
                }
            }
        } else {
            set_output2(var326, 0.0, 1.0);
        }
    } else {
        set_output2(var326, 0.0, 1.0);
    }
    add_vectors(var235, var326, 2, var234);
    double var327[2];
    if (input[3] <= -28.155789688520862) {
        set_output2(var327, 1.0, 0.0);
    } else {
        set_output2(var327, 0.0, 1.0);
    }
    add_vectors(var234, var327, 2, var233);
    double var328[2];
    if (input[7] <= 2.6900065333610783) {
        if (input[8] <= 0.5217754657905943) {
            set_output2(var328, 1.0, 0.0);
        } else {
            set_output2(var328, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        set_output2(var328, 0.0, 1.0);
    }
    add_vectors(var233, var328, 2, var232);
    double var329[2];
    if (input[8] <= 0.6096488499875518) {
        set_output2(var329, 1.0, 0.0);
    } else {
        set_output2(var329, 0.0, 1.0);
    }
    add_vectors(var232, var329, 2, var231);
    double var330[2];
    if (input[9] <= 1.6748890230718871) {
        if (input[9] <= 0.9414353691138567) {
            set_output2(var330, 1.0, 0.0);
        } else {
            set_output2(var330, 0.9938487241798298, 0.0061512758201701144);
        }
    } else {
        if (input[5] <= 10.924564261377325) {
            set_output2(var330, 0.45084745762711853, 0.5491525423728815);
        } else {
            set_output2(var330, 0.0, 1.0);
        }
    }
    add_vectors(var231, var330, 2, var230);
    double var331[2];
    if (input[0] <= 2.241118605128307) {
        set_output2(var331, 1.0, 0.0);
    } else {
        set_output2(var331, 0.0, 1.0);
    }
    add_vectors(var230, var331, 2, var229);
    double var332[2];
    if (input[3] <= -25.997747329380218) {
        if (input[7] <= -1.5777869024588425) {
            set_output2(var332, 1.0, 0.0);
        } else {
            if (input[8] <= 0.4007423638969561) {
                set_output2(var332, 1.0, 0.0);
            } else {
                if (input[6] <= 28.80602544940202) {
                    set_output2(var332, 1.0, 0.0);
                } else {
                    set_output2(var332, 0.7403846153846154, 0.25961538461538464);
                }
            }
        }
    } else {
        set_output2(var332, 0.0, 1.0);
    }
    add_vectors(var229, var332, 2, var228);
    double var333[2];
    if (input[2] <= -11.592624817721724) {
        if (input[1] <= 7.729399917875419) {
            set_output2(var333, 1.0, 0.0);
        } else {
            if (input[3] <= -32.12209167939989) {
                if (input[6] <= 21.099301240792794) {
                    set_output2(var333, 1.0, 0.0);
                } else {
                    set_output2(var333, 1.0, 0.0);
                }
            } else {
                set_output2(var333, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var333, 0.0, 1.0);
    }
    add_vectors(var228, var333, 2, var227);
    double var334[2];
    if (input[8] <= 0.6721965094805288) {
        set_output2(var334, 1.0, 0.0);
    } else {
        set_output2(var334, 0.0, 1.0);
    }
    add_vectors(var227, var334, 2, var226);
    double var335[2];
    if (input[9] <= 2.0709667994905674) {
        if (input[7] <= 1.827876502659926) {
            set_output2(var335, 1.0, 0.0);
        } else {
            set_output2(var335, 0.6553191489361702, 0.3446808510638298);
        }
    } else {
        set_output2(var335, 0.0, 1.0);
    }
    add_vectors(var226, var335, 2, var225);
    double var336[2];
    if (input[8] <= 0.5736801078437104) {
        set_output2(var336, 1.0, 0.0);
    } else {
        set_output2(var336, 0.0, 1.0);
    }
    add_vectors(var225, var336, 2, var224);
    double var337[2];
    if (input[3] <= -25.16210508383434) {
        if (input[8] <= 0.4566006481650282) {
            set_output2(var337, 1.0, 0.0);
        } else {
            set_output2(var337, 0.850828729281768, 0.14917127071823205);
        }
    } else {
        set_output2(var337, 0.0, 1.0);
    }
    add_vectors(var224, var337, 2, var223);
    double var338[2];
    if (input[8] <= 0.6913113454199404) {
        set_output2(var338, 1.0, 0.0);
    } else {
        set_output2(var338, 0.0, 1.0);
    }
    add_vectors(var223, var338, 2, var222);
    double var339[2];
    if (input[3] <= -29.12127470562003) {
        set_output2(var339, 1.0, 0.0);
    } else {
        set_output2(var339, 0.0, 1.0);
    }
    add_vectors(var222, var339, 2, var221);
    double var340[2];
    if (input[9] <= 2.296266239916932) {
        if (input[2] <= -34.2106092859367) {
            set_output2(var340, 1.0, 0.0);
        } else {
            if (input[6] <= 17.744718117853534) {
                set_output2(var340, 1.0, 0.0);
            } else {
                if (input[2] <= -25.82251458579642) {
                    set_output2(var340, 1.0, 0.0);
                } else {
                    if (input[8] <= 0.5121563901004498) {
                        set_output2(var340, 1.0, 0.0);
                    } else {
                        set_output2(var340, 0.32217573221757323, 0.6778242677824268);
                    }
                }
            }
        }
    } else {
        set_output2(var340, 0.0, 1.0);
    }
    add_vectors(var221, var340, 2, var220);
    double var341[2];
    if (input[3] <= -28.73608596514572) {
        set_output2(var341, 1.0, 0.0);
    } else {
        set_output2(var341, 0.0, 1.0);
    }
    add_vectors(var220, var341, 2, var219);
    double var342[2];
    if (input[4] <= 1.8649484024535425) {
        set_output2(var342, 1.0, 0.0);
    } else {
        if (input[0] <= 5.522878076779912) {
            set_output2(var342, 0.9168377823408623, 0.08316221765913759);
        } else {
            set_output2(var342, 0.0, 1.0);
        }
    }
    add_vectors(var219, var342, 2, var218);
    double var343[2];
    if (input[8] <= 0.7338020213481397) {
        set_output2(var343, 0.9972225079724307, 0.0027774920275691877);
    } else {
        set_output2(var343, 0.0, 1.0);
    }
    add_vectors(var218, var343, 2, var217);
    double var344[2];
    if (input[8] <= 0.6563641940749498) {
        set_output2(var344, 1.0, 0.0);
    } else {
        set_output2(var344, 0.0, 1.0);
    }
    add_vectors(var217, var344, 2, var216);
    double var345[2];
    if (input[2] <= -18.28569958044778) {
        set_output2(var345, 1.0, 0.0);
    } else {
        set_output2(var345, 0.0, 1.0);
    }
    add_vectors(var216, var345, 2, var215);
    double var346[2];
    if (input[3] <= -22.551189953909688) {
        if (input[0] <= 2.756356176287554) {
            set_output2(var346, 1.0, 0.0);
        } else {
            set_output2(var346, 0.0, 1.0);
        }
    } else {
        set_output2(var346, 0.0, 1.0);
    }
    add_vectors(var215, var346, 2, var214);
    double var347[2];
    if (input[3] <= -29.672691105152758) {
        set_output2(var347, 1.0, 0.0);
    } else {
        set_output2(var347, 0.0, 1.0);
    }
    add_vectors(var214, var347, 2, var213);
    double var348[2];
    if (input[4] <= 5.735306338699803) {
        if (input[7] <= -0.30355094806566685) {
            set_output2(var348, 1.0, 0.0);
        } else {
            if (input[6] <= 26.407600131616814) {
                set_output2(var348, 1.0, 0.0);
            } else {
                if (input[9] <= 0.7955871648883316) {
                    set_output2(var348, 1.0, 0.0);
                } else {
                    set_output2(var348, 0.7403846153846154, 0.25961538461538464);
                }
            }
        }
    } else {
        set_output2(var348, 0.0, 1.0);
    }
    add_vectors(var213, var348, 2, var212);
    double var349[2];
    if (input[3] <= -17.729645999073835) {
        if (input[4] <= 3.344581275648149) {
            set_output2(var349, 1.0, 0.0);
        } else {
            set_output2(var349, 0.0, 1.0);
        }
    } else {
        set_output2(var349, 0.0, 1.0);
    }
    add_vectors(var212, var349, 2, var211);
    double var350[2];
    if (input[0] <= 4.568073927467642) {
        if (input[1] <= 6.77641076049113) {
            set_output2(var350, 1.0, 0.0);
        } else {
            if (input[6] <= 12.925655938201174) {
                set_output2(var350, 1.0, 0.0);
            } else {
                if (input[6] <= 25.32681169420546) {
                    set_output2(var350, 1.0, 0.0);
                } else {
                    set_output2(var350, 0.6553191489361702, 0.3446808510638298);
                }
            }
        }
    } else {
        set_output2(var350, 0.0, 1.0);
    }
    add_vectors(var211, var350, 2, var210);
    double var351[2];
    if (input[0] <= 5.964221797736941) {
        if (input[7] <= 1.8083013129100634) {
            set_output2(var351, 1.0, 0.0);
        } else {
            set_output2(var351, 0.6553191489361702, 0.3446808510638298);
        }
    } else {
        set_output2(var351, 0.0, 1.0);
    }
    add_vectors(var210, var351, 2, var209);
    double var352[2];
    if (input[0] <= 2.1823920569900257) {
        set_output2(var352, 1.0, 0.0);
    } else {
        set_output2(var352, 0.0, 1.0);
    }
    add_vectors(var209, var352, 2, var208);
    double var353[2];
    if (input[4] <= 3.8647970434312273) {
        set_output2(var353, 1.0, 0.0);
    } else {
        set_output2(var353, 0.0, 1.0);
    }
    add_vectors(var208, var353, 2, var207);
    double var354[2];
    if (input[5] <= 9.965285017486705) {
        if (input[4] <= 1.3986253113861444) {
            set_output2(var354, 1.0, 0.0);
        } else {
            if (input[6] <= 27.40132265724717) {
                set_output2(var354, 1.0, 0.0);
            } else {
                set_output2(var354, 0.7917737789203084, 0.20822622107969152);
            }
        }
    } else {
        set_output2(var354, 0.0, 1.0);
    }
    add_vectors(var207, var354, 2, var206);
    double var355[2];
    if (input[2] <= -15.850153379262373) {
        if (input[9] <= 0.7619331970565345) {
            set_output2(var355, 1.0, 0.0);
        } else {
            if (input[2] <= -26.47903567447279) {
                set_output2(var355, 1.0, 0.0);
            } else {
                set_output2(var355, 0.7038391224862889, 0.2961608775137112);
            }
        }
    } else {
        set_output2(var355, 0.0, 1.0);
    }
    add_vectors(var206, var355, 2, var205);
    double var356[2];
    if (input[8] <= 0.5659870638639626) {
        set_output2(var356, 1.0, 0.0);
    } else {
        set_output2(var356, 0.0, 1.0);
    }
    add_vectors(var205, var356, 2, var204);
    double var357[2];
    if (input[2] <= -15.467082160194671) {
        if (input[2] <= -30.583204255669834) {
            set_output2(var357, 1.0, 0.0);
        } else {
            if (input[3] <= -33.32327715800849) {
                set_output2(var357, 1.0, 0.0);
            } else {
                set_output2(var357, 0.8917112299465241, 0.10828877005347595);
            }
        }
    } else {
        set_output2(var357, 0.0, 1.0);
    }
    add_vectors(var204, var357, 2, var203);
    double var358[2];
    if (input[0] <= 1.9698738345783733) {
        set_output2(var358, 1.0, 0.0);
    } else {
        set_output2(var358, 0.0, 1.0);
    }
    add_vectors(var203, var358, 2, var202);
    double var359[2];
    if (input[3] <= -29.385490328202444) {
        set_output2(var359, 1.0, 0.0);
    } else {
        set_output2(var359, 0.0, 1.0);
    }
    add_vectors(var202, var359, 2, var201);
    double var360[2];
    if (input[3] <= -26.60462421975876) {
        set_output2(var360, 1.0, 0.0);
    } else {
        set_output2(var360, 0.0, 1.0);
    }
    add_vectors(var201, var360, 2, var200);
    double var361[2];
    if (input[4] <= 7.737821825674812) {
        if (input[7] <= 1.8068811733739896) {
            if (input[7] <= 0.7049558879472491) {
                set_output2(var361, 1.0, 0.0);
            } else {
                if (input[7] <= 0.9020587527279814) {
                    if (input[7] <= 0.7802572877334353) {
                        set_output2(var361, 0.8179775280898877, 0.1820224719101124);
                    } else {
                        set_output2(var361, 1.0, 0.0);
                    }
                } else {
                    set_output2(var361, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var361, 0.6553191489361702, 0.3446808510638298);
        }
    } else {
        set_output2(var361, 0.0, 1.0);
    }
    add_vectors(var200, var361, 2, var199);
    double var362[2];
    if (input[0] <= 6.013077150631641) {
        if (input[2] <= -26.066847591141396) {
            set_output2(var362, 1.0, 0.0);
        } else {
            set_output2(var362, 0.8261802575107297, 0.1738197424892704);
        }
    } else {
        set_output2(var362, 0.0, 1.0);
    }
    add_vectors(var199, var362, 2, var198);
    double var363[2];
    if (input[0] <= 3.0323267066603767) {
        set_output2(var363, 1.0, 0.0);
    } else {
        set_output2(var363, 0.0, 1.0);
    }
    add_vectors(var198, var363, 2, var197);
    double var364[2];
    if (input[2] <= -18.454045708099184) {
        set_output2(var364, 1.0, 0.0);
    } else {
        set_output2(var364, 0.0, 1.0);
    }
    add_vectors(var197, var364, 2, var196);
    double var365[2];
    if (input[3] <= -24.948380477025243) {
        if (input[3] <= -32.95935857218362) {
            set_output2(var365, 1.0, 0.0);
        } else {
            set_output2(var365, 0.949501246882793, 0.05049875311720699);
        }
    } else {
        set_output2(var365, 0.0, 1.0);
    }
    add_vectors(var196, var365, 2, var195);
    double var366[2];
    if (input[9] <= 2.2777988723392064) {
        if (input[9] <= 1.5264276868477673) {
            set_output2(var366, 1.0, 0.0);
        } else {
            set_output2(var366, 0.8089622641509433, 0.19103773584905664);
        }
    } else {
        set_output2(var366, 0.0, 1.0);
    }
    add_vectors(var195, var366, 2, var194);
    double var367[2];
    if (input[9] <= 1.0467587155514306) {
        set_output2(var367, 1.0, 0.0);
    } else {
        if (input[5] <= 7.422770838467805) {
            if (input[2] <= -28.822085204335036) {
                set_output2(var367, 1.0, 0.0);
            } else {
                set_output2(var367, 1.0, 0.0);
            }
        } else {
            if (input[6] <= 31.025243362127128) {
                set_output2(var367, 0.0, 1.0);
            } else {
                set_output2(var367, 0.0, 1.0);
            }
        }
    }
    add_vectors(var194, var367, 2, var193);
    double var368[2];
    if (input[8] <= 0.7484148899201672) {
        if (input[9] <= 3.2857609390545655) {
            set_output2(var368, 1.0, 0.0);
        } else {
            set_output2(var368, 0.0, 1.0);
        }
    } else {
        set_output2(var368, 0.0, 1.0);
    }
    add_vectors(var193, var368, 2, var192);
    double var369[2];
    if (input[4] <= 6.122016958096268) {
        if (input[6] <= 27.777675827505437) {
            set_output2(var369, 1.0, 0.0);
        } else {
            set_output2(var369, 0.850828729281768, 0.14917127071823205);
        }
    } else {
        set_output2(var369, 0.0, 1.0);
    }
    add_vectors(var192, var369, 2, var191);
    double var370[2];
    if (input[3] <= -26.837551654334625) {
        set_output2(var370, 1.0, 0.0);
    } else {
        set_output2(var370, 0.0, 1.0);
    }
    add_vectors(var191, var370, 2, var190);
    double var371[2];
    if (input[7] <= 2.582114762241866) {
        if (input[6] <= 30.38030427170763) {
            set_output2(var371, 1.0, 0.0);
        } else {
            set_output2(var371, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        set_output2(var371, 0.0, 1.0);
    }
    add_vectors(var190, var371, 2, var189);
    double var372[2];
    if (input[5] <= 10.089110628594373) {
        if (input[2] <= -26.726124516790257) {
            set_output2(var372, 1.0, 0.0);
        } else {
            set_output2(var372, 0.8693548387096773, 0.13064516129032258);
        }
    } else {
        set_output2(var372, 0.0, 1.0);
    }
    add_vectors(var189, var372, 2, var188);
    double var373[2];
    if (input[4] <= 3.3145552414050994) {
        set_output2(var373, 1.0, 0.0);
    } else {
        set_output2(var373, 0.0, 1.0);
    }
    add_vectors(var188, var373, 2, var187);
    double var374[2];
    if (input[3] <= -28.827578017448957) {
        set_output2(var374, 1.0, 0.0);
    } else {
        set_output2(var374, 0.0, 1.0);
    }
    add_vectors(var187, var374, 2, var186);
    double var375[2];
    if (input[1] <= 9.995014640565287) {
        if (input[2] <= -28.927868344419664) {
            set_output2(var375, 1.0, 0.0);
        } else {
            if (input[1] <= 5.2574860034567035) {
                set_output2(var375, 1.0, 0.0);
            } else {
                if (input[5] <= 3.9298509809939546) {
                    set_output2(var375, 1.0, 0.0);
                } else {
                    if (input[6] <= 21.4114557195373) {
                        set_output2(var375, 1.0, 0.0);
                    } else {
                        set_output2(var375, 0.8829479768786127, 0.11705202312138731);
                    }
                }
            }
        }
    } else {
        set_output2(var375, 0.0, 1.0);
    }
    add_vectors(var186, var375, 2, var185);
    double var376[2];
    if (input[9] <= 1.1209917130155522) {
        set_output2(var376, 1.0, 0.0);
    } else {
        if (input[0] <= 2.6348369566452448) {
            set_output2(var376, 1.0, 0.0);
        } else {
            if (input[9] <= 5.107488059154335) {
                set_output2(var376, 0.0, 1.0);
            } else {
                set_output2(var376, 0.0, 1.0);
            }
        }
    }
    add_vectors(var185, var376, 2, var184);
    double var377[2];
    if (input[4] <= 4.721610855885924) {
        set_output2(var377, 1.0, 0.0);
    } else {
        set_output2(var377, 0.0, 1.0);
    }
    add_vectors(var184, var377, 2, var183);
    double var378[2];
    if (input[0] <= 8.882547042243853) {
        if (input[0] <= 3.8797608652082736) {
            set_output2(var378, 1.0, 0.0);
        } else {
            set_output2(var378, 0.0, 1.0);
        }
    } else {
        set_output2(var378, 0.0, 1.0);
    }
    add_vectors(var183, var378, 2, var182);
    double var379[2];
    if (input[4] <= 2.0906921680802393) {
        set_output2(var379, 1.0, 0.0);
    } else {
        set_output2(var379, 0.0, 1.0);
    }
    add_vectors(var182, var379, 2, var181);
    double var380[2];
    if (input[4] <= 6.328180169670788) {
        if (input[9] <= 1.1652919656242957) {
            set_output2(var380, 1.0, 0.0);
        } else {
            if (input[5] <= 5.448968510013959) {
                set_output2(var380, 1.0, 0.0);
            } else {
                set_output2(var380, 0.9185929648241206, 0.08140703517587941);
            }
        }
    } else {
        set_output2(var380, 0.0, 1.0);
    }
    add_vectors(var181, var380, 2, var180);
    double var381[2];
    if (input[6] <= 34.94928611264331) {
        if (input[8] <= 0.5119168481526015) {
            set_output2(var381, 1.0, 0.0);
        } else {
            if (input[7] <= 4.3478061201467) {
                set_output2(var381, 0.32217573221757323, 0.6778242677824268);
            } else {
                set_output2(var381, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var381, 0.0, 1.0);
    }
    add_vectors(var180, var381, 2, var179);
    double var382[2];
    if (input[4] <= 4.571719055667692) {
        set_output2(var382, 1.0, 0.0);
    } else {
        set_output2(var382, 0.0, 1.0);
    }
    add_vectors(var179, var382, 2, var178);
    double var383[2];
    if (input[5] <= 10.607483048110245) {
        if (input[9] <= 0.9718071447407285) {
            set_output2(var383, 1.0, 0.0);
        } else {
            if (input[1] <= 8.751680640421167) {
                set_output2(var383, 1.0, 0.0);
            } else {
                set_output2(var383, 0.6553191489361702, 0.3446808510638298);
            }
        }
    } else {
        set_output2(var383, 0.0, 1.0);
    }
    add_vectors(var178, var383, 2, var177);
    double var384[2];
    if (input[9] <= 2.38257519700974) {
        if (input[9] <= 1.3510844060974776) {
            set_output2(var384, 1.0, 0.0);
        } else {
            if (input[6] <= 12.854557188805144) {
                set_output2(var384, 1.0, 0.0);
            } else {
                set_output2(var384, 0.5877862595419847, 0.4122137404580153);
            }
        }
    } else {
        set_output2(var384, 0.0, 1.0);
    }
    add_vectors(var177, var384, 2, var176);
    double var385[2];
    if (input[4] <= 3.3853570093796286) {
        set_output2(var385, 1.0, 0.0);
    } else {
        set_output2(var385, 0.0, 1.0);
    }
    add_vectors(var176, var385, 2, var175);
    double var386[2];
    if (input[7] <= 0.1685276785798866) {
        set_output2(var386, 1.0, 0.0);
    } else {
        if (input[0] <= 4.05000196399121) {
            if (input[1] <= 7.069613151023888) {
                set_output2(var386, 1.0, 0.0);
            } else {
                if (input[7] <= 0.9542973142309634) {
                    set_output2(var386, 1.0, 0.0);
                } else {
                    set_output2(var386, 0.6553191489361702, 0.3446808510638298);
                }
            }
        } else {
            if (input[3] <= -15.676119370393078) {
                set_output2(var386, 0.0, 1.0);
            } else {
                set_output2(var386, 0.0, 1.0);
            }
        }
    }
    add_vectors(var175, var386, 2, var174);
    double var387[2];
    if (input[3] <= -30.343151671985822) {
        set_output2(var387, 1.0, 0.0);
    } else {
        set_output2(var387, 0.0, 1.0);
    }
    add_vectors(var174, var387, 2, var173);
    double var388[2];
    if (input[4] <= 3.411989886401564) {
        set_output2(var388, 1.0, 0.0);
    } else {
        set_output2(var388, 0.0, 1.0);
    }
    add_vectors(var173, var388, 2, var172);
    double var389[2];
    if (input[1] <= 11.547158125462458) {
        if (input[8] <= 0.6532810219311826) {
            set_output2(var389, 1.0, 0.0);
        } else {
            set_output2(var389, 0.0, 1.0);
        }
    } else {
        set_output2(var389, 0.0, 1.0);
    }
    add_vectors(var172, var389, 2, var171);
    double var390[2];
    if (input[7] <= 0.9105923375472242) {
        if (input[6] <= 27.69247152432848) {
            set_output2(var390, 1.0, 0.0);
        } else {
            set_output2(var390, 0.7403846153846154, 0.25961538461538464);
        }
    } else {
        if (input[8] <= 0.4728219088181893) {
            set_output2(var390, 1.0, 0.0);
        } else {
            if (input[7] <= 4.092743035806286) {
                set_output2(var390, 0.19201995012468828, 0.8079800498753118);
            } else {
                set_output2(var390, 0.0, 1.0);
            }
        }
    }
    add_vectors(var171, var390, 2, var170);
    double var391[2];
    if (input[5] <= 5.667821153464772) {
        set_output2(var391, 1.0, 0.0);
    } else {
        if (input[4] <= 5.561381068248433) {
            set_output2(var391, 0.7403846153846154, 0.25961538461538464);
        } else {
            set_output2(var391, 0.0, 1.0);
        }
    }
    add_vectors(var170, var391, 2, var169);
    double var392[2];
    if (input[4] <= 5.162516465556681) {
        set_output2(var392, 1.0, 0.0);
    } else {
        set_output2(var392, 0.0, 1.0);
    }
    add_vectors(var169, var392, 2, var168);
    double var393[2];
    if (input[4] <= 5.726265803103827) {
        if (input[6] <= 21.48250878167351) {
            set_output2(var393, 1.0, 0.0);
        } else {
            if (input[7] <= 0.7643050648271515) {
                if (input[6] <= 27.387480675657415) {
                    set_output2(var393, 1.0, 0.0);
                } else {
                    set_output2(var393, 0.6553191489361702, 0.3446808510638298);
                }
            } else {
                set_output2(var393, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var393, 0.0, 1.0);
    }
    add_vectors(var168, var393, 2, var167);
    double var394[2];
    if (input[7] <= 2.6614038899761683) {
        if (input[6] <= 21.91053456370308) {
            set_output2(var394, 1.0, 0.0);
        } else {
            if (input[8] <= 0.45653689172933637) {
                set_output2(var394, 1.0, 0.0);
            } else {
                if (input[7] <= 0.8202076600227796) {
                    set_output2(var394, 0.6553191489361702, 0.3446808510638298);
                } else {
                    set_output2(var394, 0.7403846153846154, 0.25961538461538464);
                }
            }
        }
    } else {
        set_output2(var394, 0.0, 1.0);
    }
    add_vectors(var167, var394, 2, var166);
    double var395[2];
    if (input[5] <= 6.848275558029418) {
        set_output2(var395, 1.0, 0.0);
    } else {
        if (input[1] <= 14.039416224730711) {
            if (input[6] <= 41.977976814030455) {
                set_output2(var395, 0.15975103734439833, 0.8402489626556018);
            } else {
                set_output2(var395, 0.0, 1.0);
            }
        } else {
            set_output2(var395, 0.0, 1.0);
        }
    }
    add_vectors(var166, var395, 2, var165);
    double var396[2];
    if (input[4] <= 2.4128201631613937) {
        set_output2(var396, 1.0, 0.0);
    } else {
        set_output2(var396, 0.0, 1.0);
    }
    add_vectors(var165, var396, 2, var164);
    double var397[2];
    if (input[0] <= 3.2364522028100344) {
        set_output2(var397, 1.0, 0.0);
    } else {
        set_output2(var397, 0.0, 1.0);
    }
    add_vectors(var164, var397, 2, var163);
    double var398[2];
    if (input[8] <= 0.5937903299693205) {
        set_output2(var398, 1.0, 0.0);
    } else {
        set_output2(var398, 0.0, 1.0);
    }
    add_vectors(var163, var398, 2, var162);
    double var399[2];
    if (input[8] <= 0.5112496351337774) {
        set_output2(var399, 1.0, 0.0);
    } else {
        if (input[7] <= 4.700649872988949) {
            if (input[9] <= 3.5060634359875715) {
                set_output2(var399, 0.32217573221757323, 0.6778242677824268);
            } else {
                set_output2(var399, 0.0, 1.0);
            }
        } else {
            set_output2(var399, 0.0, 1.0);
        }
    }
    add_vectors(var162, var399, 2, var161);
    double var400[2];
    if (input[8] <= 0.8248924804934281) {
        if (input[2] <= -20.038353411337688) {
            set_output2(var400, 1.0, 0.0);
        } else {
            set_output2(var400, 0.0, 1.0);
        }
    } else {
        set_output2(var400, 0.0, 1.0);
    }
    add_vectors(var161, var400, 2, var160);
    double var401[2];
    if (input[1] <= 8.643322435296142) {
        set_output2(var401, 1.0, 0.0);
    } else {
        if (input[7] <= 1.1648144361058699) {
            set_output2(var401, 0.6553191489361702, 0.3446808510638298);
        } else {
            set_output2(var401, 0.0, 1.0);
        }
    }
    add_vectors(var160, var401, 2, var159);
    double var402[2];
    if (input[0] <= 5.625088077964185) {
        if (input[6] <= 21.655848310193008) {
            set_output2(var402, 1.0, 0.0);
        } else {
            set_output2(var402, 0.9736499674690957, 0.026350032530904368);
        }
    } else {
        set_output2(var402, 0.0, 1.0);
    }
    add_vectors(var159, var402, 2, var158);
    double var403[2];
    if (input[1] <= 7.442174703424219) {
        set_output2(var403, 1.0, 0.0);
    } else {
        if (input[2] <= -21.904944364796933) {
            set_output2(var403, 1.0, 0.0);
        } else {
            set_output2(var403, 0.0, 1.0);
        }
    }
    add_vectors(var158, var403, 2, var157);
    double var404[2];
    if (input[1] <= 8.624603154070922) {
        set_output2(var404, 1.0, 0.0);
    } else {
        if (input[8] <= 0.9284002961235147) {
            if (input[2] <= -11.008305282149628) {
                if (input[5] <= 12.568842326122006) {
                    set_output2(var404, 0.6553191489361702, 0.3446808510638298);
                } else {
                    set_output2(var404, 0.0, 1.0);
                }
            } else {
                set_output2(var404, 0.0, 1.0);
            }
        } else {
            set_output2(var404, 0.0, 1.0);
        }
    }
    add_vectors(var157, var404, 2, var156);
    double var405[2];
    if (input[8] <= 0.5959297025381302) {
        set_output2(var405, 1.0, 0.0);
    } else {
        set_output2(var405, 0.0, 1.0);
    }
    add_vectors(var156, var405, 2, var155);
    double var406[2];
    if (input[2] <= -21.46415650768512) {
        set_output2(var406, 1.0, 0.0);
    } else {
        set_output2(var406, 0.0, 1.0);
    }
    add_vectors(var155, var406, 2, var154);
    double var407[2];
    if (input[3] <= -29.407761053920797) {
        set_output2(var407, 1.0, 0.0);
    } else {
        set_output2(var407, 0.0, 1.0);
    }
    add_vectors(var154, var407, 2, var153);
    double var408[2];
    if (input[2] <= -23.869788237386523) {
        set_output2(var408, 1.0, 0.0);
    } else {
        if (input[7] <= 1.7816771514735184) {
            set_output2(var408, 0.32217573221757323, 0.6778242677824268);
        } else {
            set_output2(var408, 0.0, 1.0);
        }
    }
    add_vectors(var153, var408, 2, var152);
    double var409[2];
    if (input[4] <= 8.661826229739349) {
        if (input[3] <= -22.078753177832088) {
            set_output2(var409, 1.0, 0.0);
        } else {
            set_output2(var409, 0.0, 1.0);
        }
    } else {
        set_output2(var409, 0.0, 1.0);
    }
    add_vectors(var152, var409, 2, var151);
    double var410[2];
    if (input[4] <= 5.417775863075551) {
        if (input[1] <= 3.444608221500959) {
            set_output2(var410, 1.0, 0.0);
        } else {
            if (input[4] <= 1.4860960076040093) {
                set_output2(var410, 1.0, 0.0);
            } else {
                if (input[8] <= 0.43342795099380566) {
                    set_output2(var410, 1.0, 0.0);
                } else {
                    set_output2(var410, 0.7917737789203084, 0.20822622107969152);
                }
            }
        }
    } else {
        set_output2(var410, 0.0, 1.0);
    }
    add_vectors(var151, var410, 2, var150);
    double var411[2];
    if (input[8] <= 0.710577566880329) {
        if (input[0] <= 1.406048174680028) {
            set_output2(var411, 1.0, 0.0);
        } else {
            if (input[2] <= -28.407043881247613) {
                set_output2(var411, 1.0, 0.0);
            } else {
                set_output2(var411, 0.9185929648241206, 0.08140703517587941);
            }
        }
    } else {
        set_output2(var411, 0.0, 1.0);
    }
    add_vectors(var150, var411, 2, var149);
    double var412[2];
    if (input[2] <= -13.828720003014414) {
        if (input[5] <= 8.499175571102077) {
            set_output2(var412, 1.0, 0.0);
        } else {
            set_output2(var412, 0.0, 1.0);
        }
    } else {
        set_output2(var412, 0.0, 1.0);
    }
    add_vectors(var149, var412, 2, var148);
    double var413[2];
    if (input[4] <= 5.185245508008158) {
        set_output2(var413, 1.0, 0.0);
    } else {
        set_output2(var413, 0.0, 1.0);
    }
    add_vectors(var148, var413, 2, var147);
    double var414[2];
    if (input[4] <= 1.9228845954315288) {
        set_output2(var414, 1.0, 0.0);
    } else {
        set_output2(var414, 0.0, 1.0);
    }
    add_vectors(var147, var414, 2, var146);
    double var415[2];
    if (input[3] <= -27.511613266148036) {
        set_output2(var415, 1.0, 0.0);
    } else {
        set_output2(var415, 0.0, 1.0);
    }
    add_vectors(var146, var415, 2, var145);
    double var416[2];
    if (input[9] <= 1.9451245880411316) {
        if (input[3] <= -33.29686539105273) {
            set_output2(var416, 1.0, 0.0);
        } else {
            if (input[8] <= 0.40995556154557866) {
                set_output2(var416, 1.0, 0.0);
            } else {
                set_output2(var416, 0.7403846153846154, 0.25961538461538464);
            }
        }
    } else {
        set_output2(var416, 0.0, 1.0);
    }
    add_vectors(var145, var416, 2, var144);
    double var417[2];
    if (input[8] <= 0.6820961375790477) {
        set_output2(var417, 1.0, 0.0);
    } else {
        set_output2(var417, 0.0, 1.0);
    }
    add_vectors(var144, var417, 2, var143);
    double var418[2];
    if (input[5] <= 13.739094027718098) {
        if (input[0] <= 3.874199524446847) {
            set_output2(var418, 1.0, 0.0);
        } else {
            set_output2(var418, 0.0, 1.0);
        }
    } else {
        set_output2(var418, 0.0, 1.0);
    }
    add_vectors(var143, var418, 2, var142);
    double var419[2];
    if (input[3] <= -26.243819804801223) {
        set_output2(var419, 1.0, 0.0);
    } else {
        set_output2(var419, 0.0, 1.0);
    }
    add_vectors(var142, var419, 2, var141);
    double var420[2];
    if (input[1] <= 10.852185005218919) {
        if (input[6] <= 19.400553319609102) {
            set_output2(var420, 1.0, 0.0);
        } else {
            if (input[2] <= -22.575510437675025) {
                set_output2(var420, 1.0, 0.0);
            } else {
                set_output2(var420, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var420, 0.0, 1.0);
    }
    add_vectors(var141, var420, 2, var140);
    double var421[2];
    if (input[2] <= -20.596451283553137) {
        set_output2(var421, 1.0, 0.0);
    } else {
        set_output2(var421, 0.0, 1.0);
    }
    add_vectors(var140, var421, 2, var139);
    double var422[2];
    if (input[4] <= 4.573214915691497) {
        set_output2(var422, 1.0, 0.0);
    } else {
        set_output2(var422, 0.0, 1.0);
    }
    add_vectors(var139, var422, 2, var138);
    double var423[2];
    if (input[5] <= 9.531003622260858) {
        if (input[1] <= 8.445693372068511) {
            set_output2(var423, 1.0, 0.0);
        } else {
            set_output2(var423, 0.6553191489361702, 0.3446808510638298);
        }
    } else {
        set_output2(var423, 0.0, 1.0);
    }
    add_vectors(var138, var423, 2, var137);
    double var424[2];
    if (input[4] <= 3.5397071756035055) {
        set_output2(var424, 1.0, 0.0);
    } else {
        set_output2(var424, 0.0, 1.0);
    }
    add_vectors(var137, var424, 2, var136);
    double var425[2];
    if (input[2] <= -20.30156217814567) {
        set_output2(var425, 1.0, 0.0);
    } else {
        set_output2(var425, 0.0, 1.0);
    }
    add_vectors(var136, var425, 2, var135);
    double var426[2];
    if (input[9] <= 2.2619627109577753) {
        if (input[3] <= -38.3113125692655) {
            set_output2(var426, 1.0, 0.0);
        } else {
            if (input[8] <= 0.41813852363383147) {
                set_output2(var426, 1.0, 0.0);
            } else {
                if (input[1] <= 7.256532373861399) {
                    set_output2(var426, 1.0, 0.0);
                } else {
                    set_output2(var426, 0.6553191489361702, 0.3446808510638298);
                }
            }
        }
    } else {
        set_output2(var426, 0.0, 1.0);
    }
    add_vectors(var135, var426, 2, var134);
    double var427[2];
    if (input[2] <= -19.50403470435158) {
        set_output2(var427, 1.0, 0.0);
    } else {
        set_output2(var427, 0.0, 1.0);
    }
    add_vectors(var134, var427, 2, var133);
    double var428[2];
    if (input[4] <= 8.484469444817567) {
        if (input[3] <= -34.59328948177393) {
            set_output2(var428, 1.0, 0.0);
        } else {
            if (input[2] <= -27.354879057118875) {
                set_output2(var428, 1.0, 0.0);
            } else {
                set_output2(var428, 0.32217573221757323, 0.6778242677824268);
            }
        }
    } else {
        set_output2(var428, 0.0, 1.0);
    }
    add_vectors(var133, var428, 2, var132);
    double var429[2];
    if (input[4] <= 6.744315174192097) {
        if (input[3] <= -34.17416252794111) {
            set_output2(var429, 1.0, 0.0);
        } else {
            if (input[0] <= 0.9405770184205045) {
                set_output2(var429, 1.0, 0.0);
            } else {
                if (input[9] <= 0.916098236593071) {
                    set_output2(var429, 1.0, 0.0);
                } else {
                    set_output2(var429, 0.9327800829875518, 0.06721991701244813);
                }
            }
        }
    } else {
        set_output2(var429, 0.0, 1.0);
    }
    add_vectors(var132, var429, 2, var131);
    double var430[2];
    if (input[8] <= 0.6812873313323952) {
        set_output2(var430, 1.0, 0.0);
    } else {
        set_output2(var430, 0.0, 1.0);
    }
    add_vectors(var131, var430, 2, var130);
    double var431[2];
    if (input[2] <= -27.83154746274915) {
        set_output2(var431, 1.0, 0.0);
    } else {
        if (input[9] <= 1.7140757352271778) {
            if (input[6] <= 19.03710305624411) {
                set_output2(var431, 1.0, 0.0);
            } else {
                if (input[2] <= -27.40974219297346) {
                    set_output2(var431, 1.0, 0.0);
                } else {
                    set_output2(var431, 0.850828729281768, 0.14917127071823205);
                }
            }
        } else {
            set_output2(var431, 0.0, 1.0);
        }
    }
    add_vectors(var130, var431, 2, var129);
    double var432[2];
    if (input[2] <= -14.456723535173879) {
        if (input[4] <= 5.5022521945519705) {
            set_output2(var432, 1.0, 0.0);
        } else {
            set_output2(var432, 0.0, 1.0);
        }
    } else {
        set_output2(var432, 0.0, 1.0);
    }
    add_vectors(var129, var432, 2, var128);
    double var433[2];
    if (input[3] <= -27.777286904726537) {
        set_output2(var433, 1.0, 0.0);
    } else {
        set_output2(var433, 0.0, 1.0);
    }
    add_vectors(var128, var433, 2, var127);
    double var434[2];
    if (input[0] <= 2.8156323874617777) {
        set_output2(var434, 1.0, 0.0);
    } else {
        set_output2(var434, 0.0, 1.0);
    }
    add_vectors(var127, var434, 2, var126);
    double var435[2];
    if (input[2] <= -18.28564067800169) {
        set_output2(var435, 1.0, 0.0);
    } else {
        set_output2(var435, 0.0, 1.0);
    }
    add_vectors(var126, var435, 2, var125);
    double var436[2];
    if (input[4] <= 9.198685304481259) {
        if (input[1] <= 11.649697992939283) {
            set_output2(var436, 1.0, 0.0);
        } else {
            set_output2(var436, 0.0, 1.0);
        }
    } else {
        set_output2(var436, 0.0, 1.0);
    }
    add_vectors(var125, var436, 2, var124);
    double var437[2];
    if (input[5] <= 5.86608241897762) {
        set_output2(var437, 1.0, 0.0);
    } else {
        if (input[1] <= 9.244753201936025) {
            set_output2(var437, 0.32217573221757323, 0.6778242677824268);
        } else {
            set_output2(var437, 0.0, 1.0);
        }
    }
    add_vectors(var124, var437, 2, var123);
    double var438[2];
    if (input[2] <= -24.041206857588996) {
        set_output2(var438, 1.0, 0.0);
    } else {
        if (input[2] <= -15.563782467357633) {
            set_output2(var438, 0.32217573221757323, 0.6778242677824268);
        } else {
            set_output2(var438, 0.0, 1.0);
        }
    }
    add_vectors(var123, var438, 2, var122);
    double var439[2];
    if (input[4] <= 2.11520253179886) {
        set_output2(var439, 1.0, 0.0);
    } else {
        set_output2(var439, 0.0, 1.0);
    }
    add_vectors(var122, var439, 2, var121);
    double var440[2];
    if (input[8] <= 0.5775771205931026) {
        set_output2(var440, 1.0, 0.0);
    } else {
        set_output2(var440, 0.0, 1.0);
    }
    add_vectors(var121, var440, 2, var120);
    double var441[2];
    if (input[3] <= -24.893997505523803) {
        if (input[6] <= 10.252915524936475) {
            set_output2(var441, 1.0, 0.0);
        } else {
            if (input[6] <= 25.434119529737014) {
                set_output2(var441, 1.0, 0.0);
            } else {
                if (input[5] <= 4.455808065509398) {
                    set_output2(var441, 1.0, 0.0);
                } else {
                    if (input[6] <= 28.431342414454924) {
                        set_output2(var441, 1.0, 0.0);
                    } else {
                        set_output2(var441, 0.7403846153846154, 0.25961538461538464);
                    }
                }
            }
        }
    } else {
        set_output2(var441, 0.0, 1.0);
    }
    add_vectors(var120, var441, 2, var119);
    double var442[2];
    if (input[0] <= 2.9220136164515296) {
        set_output2(var442, 1.0, 0.0);
    } else {
        set_output2(var442, 0.0, 1.0);
    }
    add_vectors(var119, var442, 2, var118);
    double var443[2];
    if (input[5] <= 8.577208703605942) {
        if (input[2] <= -28.05224141805185) {
            set_output2(var443, 1.0, 0.0);
        } else {
            if (input[5] <= 5.030099260583599) {
                set_output2(var443, 1.0, 0.0);
            } else {
                set_output2(var443, 0.6553191489361702, 0.3446808510638298);
            }
        }
    } else {
        set_output2(var443, 0.0, 1.0);
    }
    add_vectors(var118, var443, 2, var117);
    double var444[2];
    if (input[2] <= -23.898912056805727) {
        set_output2(var444, 1.0, 0.0);
    } else {
        if (input[1] <= 12.525234233910947) {
            if (input[4] <= 10.843176328980343) {
                set_output2(var444, 0.240625, 0.759375);
            } else {
                set_output2(var444, 0.0, 1.0);
            }
        } else {
            set_output2(var444, 0.0, 1.0);
        }
    }
    add_vectors(var117, var444, 2, var116);
    double var445[2];
    if (input[5] <= 8.009424395038232) {
        set_output2(var445, 1.0, 0.0);
    } else {
        set_output2(var445, 0.0, 1.0);
    }
    add_vectors(var116, var445, 2, var115);
    double var446[2];
    if (input[6] <= 23.671861352616062) {
        if (input[5] <= 3.91885486095523) {
            if (input[8] <= 0.16952442359625797) {
                set_output2(var446, 1.0, 0.0);
            } else {
                set_output2(var446, 1.0, 0.0);
            }
        } else {
            if (input[8] <= 0.387760182118737) {
                set_output2(var446, 1.0, 0.0);
            } else {
                if (input[2] <= -29.84952177485903) {
                    set_output2(var446, 1.0, 0.0);
                } else {
                    set_output2(var446, 0.933931484502447, 0.06606851549755303);
                }
            }
        }
    } else {
        if (input[2] <= -18.661155332669843) {
            if (input[6] <= 27.575555559484886) {
                set_output2(var446, 1.0, 0.0);
            } else {
                set_output2(var446, 1.0, 0.0);
            }
        } else {
            set_output2(var446, 0.0, 1.0);
        }
    }
    add_vectors(var115, var446, 2, var114);
    double var447[2];
    if (input[2] <= -25.386226123743587) {
        set_output2(var447, 1.0, 0.0);
    } else {
        if (input[3] <= -29.515611400739118) {
            set_output2(var447, 1.0, 0.0);
        } else {
            set_output2(var447, 0.0, 1.0);
        }
    }
    add_vectors(var114, var447, 2, var113);
    double var448[2];
    if (input[2] <= -21.67606275625765) {
        set_output2(var448, 1.0, 0.0);
    } else {
        set_output2(var448, 0.0, 1.0);
    }
    add_vectors(var113, var448, 2, var112);
    double var449[2];
    if (input[7] <= 4.171904307249703) {
        if (input[9] <= 1.6799362971403635) {
            if (input[9] <= 1.4603565064423527) {
                set_output2(var449, 1.0, 0.0);
            } else {
                set_output2(var449, 0.9281914893617021, 0.07180851063829789);
            }
        } else {
            set_output2(var449, 0.29102844638949665, 0.7089715536105033);
        }
    } else {
        set_output2(var449, 0.0, 1.0);
    }
    add_vectors(var112, var449, 2, var111);
    double var450[2];
    if (input[0] <= 1.5332712228539098) {
        set_output2(var450, 1.0, 0.0);
    } else {
        if (input[0] <= 1.8201381611556788) {
            set_output2(var450, 1.0, 0.0);
        } else {
            set_output2(var450, 0.0, 1.0);
        }
    }
    add_vectors(var111, var450, 2, var110);
    double var451[2];
    if (input[9] <= 2.1441660175896358) {
        if (input[2] <= -26.199591600560847) {
            set_output2(var451, 1.0, 0.0);
        } else {
            set_output2(var451, 0.8261802575107297, 0.1738197424892704);
        }
    } else {
        set_output2(var451, 0.0, 1.0);
    }
    add_vectors(var110, var451, 2, var109);
    double var452[2];
    if (input[8] <= 0.4925082640394899) {
        set_output2(var452, 1.0, 0.0);
    } else {
        if (input[1] <= 9.814618111781261) {
            set_output2(var452, 0.32217573221757323, 0.6778242677824268);
        } else {
            set_output2(var452, 0.0, 1.0);
        }
    }
    add_vectors(var109, var452, 2, var108);
    double var453[2];
    if (input[2] <= -26.766547236845838) {
        set_output2(var453, 1.0, 0.0);
    } else {
        if (input[4] <= 2.4702894014115255) {
            set_output2(var453, 1.0, 0.0);
        } else {
            set_output2(var453, 0.0, 1.0);
        }
    }
    add_vectors(var108, var453, 2, var107);
    double var454[2];
    if (input[6] <= 28.299562343435877) {
        if (input[5] <= 4.975240625687698) {
            if (input[1] <= 7.542277403264015) {
                set_output2(var454, 1.0, 0.0);
            } else {
                set_output2(var454, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 6.564337675067117) {
                set_output2(var454, 1.0, 0.0);
            } else {
                set_output2(var454, 0.32217573221757323, 0.6778242677824268);
            }
        }
    } else {
        if (input[4] <= 2.1865290367421535) {
            set_output2(var454, 1.0, 0.0);
        } else {
            set_output2(var454, 0.0, 1.0);
        }
    }
    add_vectors(var107, var454, 2, var106);
    double var455[2];
    if (input[1] <= 9.890239108889936) {
        if (input[6] <= 27.200810157910457) {
            set_output2(var455, 1.0, 0.0);
        } else {
            if (input[7] <= 1.204930684976763) {
                set_output2(var455, 1.0, 0.0);
            } else {
                set_output2(var455, 0.4873417721518987, 0.5126582278481012);
            }
        }
    } else {
        set_output2(var455, 0.0, 1.0);
    }
    add_vectors(var106, var455, 2, var105);
    double var456[2];
    if (input[4] <= 4.963885891995343) {
        set_output2(var456, 1.0, 0.0);
    } else {
        set_output2(var456, 0.0, 1.0);
    }
    add_vectors(var105, var456, 2, var104);
    double var457[2];
    if (input[8] <= 0.5270127562321796) {
        set_output2(var457, 1.0, 0.0);
    } else {
        set_output2(var457, 0.0, 1.0);
    }
    add_vectors(var104, var457, 2, var103);
    double var458[2];
    if (input[0] <= 2.6756761228775567) {
        set_output2(var458, 1.0, 0.0);
    } else {
        set_output2(var458, 0.0, 1.0);
    }
    add_vectors(var103, var458, 2, var102);
    double var459[2];
    if (input[0] <= 8.341912905621818) {
        if (input[0] <= 2.3611814109081215) {
            set_output2(var459, 1.0, 0.0);
        } else {
            set_output2(var459, 0.0, 1.0);
        }
    } else {
        set_output2(var459, 0.0, 1.0);
    }
    add_vectors(var102, var459, 2, var101);
    double var460[2];
    if (input[4] <= 3.9925966836626707) {
        set_output2(var460, 1.0, 0.0);
    } else {
        set_output2(var460, 0.0, 1.0);
    }
    add_vectors(var101, var460, 2, var100);
    double var461[2];
    if (input[4] <= 2.567665732476834) {
        set_output2(var461, 1.0, 0.0);
    } else {
        set_output2(var461, 0.0, 1.0);
    }
    add_vectors(var100, var461, 2, var99);
    double var462[2];
    if (input[5] <= 8.860706314388363) {
        if (input[3] <= -39.97939051495652) {
            set_output2(var462, 1.0, 0.0);
        } else {
            if (input[2] <= -27.505745310603707) {
                set_output2(var462, 1.0, 0.0);
            } else {
                set_output2(var462, 0.9260948905109488, 0.07390510948905112);
            }
        }
    } else {
        set_output2(var462, 0.0, 1.0);
    }
    add_vectors(var99, var462, 2, var98);
    double var463[2];
    if (input[9] <= 1.3632395854922816) {
        set_output2(var463, 1.0, 0.0);
    } else {
        if (input[5] <= 5.03482781489691) {
            set_output2(var463, 1.0, 0.0);
        } else {
            if (input[9] <= 4.6318272952271204) {
                if (input[9] <= 2.7944237194022645) {
                    set_output2(var463, 0.240625, 0.759375);
                } else {
                    set_output2(var463, 0.0, 1.0);
                }
            } else {
                set_output2(var463, 0.0, 1.0);
            }
        }
    }
    add_vectors(var98, var463, 2, var97);
    double var464[2];
    if (input[2] <= -17.899490723730512) {
        set_output2(var464, 1.0, 0.0);
    } else {
        set_output2(var464, 0.0, 1.0);
    }
    add_vectors(var97, var464, 2, var96);
    double var465[2];
    if (input[4] <= 6.629128466349643) {
        if (input[5] <= 4.229038500390032) {
            set_output2(var465, 1.0, 0.0);
        } else {
            if (input[1] <= 8.02415328592118) {
                set_output2(var465, 1.0, 0.0);
            } else {
                set_output2(var465, 0.6553191489361702, 0.3446808510638298);
            }
        }
    } else {
        set_output2(var465, 0.0, 1.0);
    }
    add_vectors(var96, var465, 2, var95);
    double var466[2];
    if (input[5] <= 7.818366320381782) {
        set_output2(var466, 1.0, 0.0);
    } else {
        set_output2(var466, 0.0, 1.0);
    }
    add_vectors(var95, var466, 2, var94);
    double var467[2];
    if (input[2] <= -18.20310405334451) {
        set_output2(var467, 1.0, 0.0);
    } else {
        set_output2(var467, 0.0, 1.0);
    }
    add_vectors(var94, var467, 2, var93);
    double var468[2];
    if (input[7] <= 3.3729137954017663) {
        if (input[1] <= 8.160623218799984) {
            set_output2(var468, 1.0, 0.0);
        } else {
            if (input[3] <= -28.54669307740784) {
                set_output2(var468, 1.0, 0.0);
            } else {
                set_output2(var468, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var468, 0.0, 1.0);
    }
    add_vectors(var93, var468, 2, var92);
    double var469[2];
    if (input[4] <= 1.9254354193314651) {
        set_output2(var469, 1.0, 0.0);
    } else {
        set_output2(var469, 0.0, 1.0);
    }
    add_vectors(var92, var469, 2, var91);
    double var470[2];
    if (input[4] <= 2.6226923676838902) {
        set_output2(var470, 1.0, 0.0);
    } else {
        set_output2(var470, 0.0, 1.0);
    }
    add_vectors(var91, var470, 2, var90);
    double var471[2];
    if (input[0] <= 6.390978117222759) {
        if (input[2] <= -28.942201116946435) {
            set_output2(var471, 1.0, 0.0);
        } else {
            if (input[1] <= 6.463251496890193) {
                set_output2(var471, 1.0, 0.0);
            } else {
                if (input[1] <= 8.123734041055567) {
                    set_output2(var471, 1.0, 0.0);
                } else {
                    set_output2(var471, 0.6553191489361702, 0.3446808510638298);
                }
            }
        }
    } else {
        set_output2(var471, 0.0, 1.0);
    }
    add_vectors(var90, var471, 2, var89);
    double var472[2];
    if (input[4] <= 4.020036929568968) {
        set_output2(var472, 1.0, 0.0);
    } else {
        set_output2(var472, 0.0, 1.0);
    }
    add_vectors(var89, var472, 2, var88);
    double var473[2];
    if (input[3] <= -23.33771874277278) {
        if (input[0] <= 5.500045382406792) {
            set_output2(var473, 1.0, 0.0);
        } else {
            set_output2(var473, 0.0, 1.0);
        }
    } else {
        set_output2(var473, 0.0, 1.0);
    }
    add_vectors(var88, var473, 2, var87);
    double var474[2];
    if (input[0] <= 4.6071752424374335) {
        if (input[1] <= 3.614199702971601) {
            set_output2(var474, 1.0, 0.0);
        } else {
            if (input[6] <= 23.42045338038225) {
                set_output2(var474, 1.0, 0.0);
            } else {
                if (input[1] <= 7.750544488141546) {
                    set_output2(var474, 1.0, 0.0);
                } else {
                    set_output2(var474, 0.7403846153846154, 0.25961538461538464);
                }
            }
        }
    } else {
        set_output2(var474, 0.0, 1.0);
    }
    add_vectors(var87, var474, 2, var86);
    double var475[2];
    if (input[0] <= 3.7127750421009362) {
        set_output2(var475, 1.0, 0.0);
    } else {
        set_output2(var475, 0.0, 1.0);
    }
    add_vectors(var86, var475, 2, var85);
    double var476[2];
    if (input[5] <= 8.470448604869567) {
        if (input[7] <= -0.09414973834976692) {
            set_output2(var476, 1.0, 0.0);
        } else {
            if (input[6] <= 25.957760728826727) {
                set_output2(var476, 1.0, 0.0);
            } else {
                if (input[5] <= 4.730304045717154) {
                    set_output2(var476, 1.0, 0.0);
                } else {
                    set_output2(var476, 0.7917737789203084, 0.20822622107969152);
                }
            }
        }
    } else {
        set_output2(var476, 0.0, 1.0);
    }
    add_vectors(var85, var476, 2, var84);
    double var477[2];
    if (input[8] <= 0.7256781980292694) {
        if (input[1] <= 3.267107234459652) {
            set_output2(var477, 1.0, 0.0);
        } else {
            if (input[0] <= 1.4771977783158332) {
                set_output2(var477, 1.0, 0.0);
            } else {
                set_output2(var477, 0.7917737789203084, 0.20822622107969152);
            }
        }
    } else {
        set_output2(var477, 0.0, 1.0);
    }
    add_vectors(var84, var477, 2, var83);
    double var478[2];
    if (input[0] <= 2.018606860168674) {
        set_output2(var478, 1.0, 0.0);
    } else {
        set_output2(var478, 0.0, 1.0);
    }
    add_vectors(var83, var478, 2, var82);
    double var479[2];
    if (input[8] <= 0.5919176339438965) {
        set_output2(var479, 1.0, 0.0);
    } else {
        set_output2(var479, 0.0, 1.0);
    }
    add_vectors(var82, var479, 2, var81);
    double var480[2];
    if (input[0] <= 3.340883514540767) {
        set_output2(var480, 1.0, 0.0);
    } else {
        set_output2(var480, 0.0, 1.0);
    }
    add_vectors(var81, var480, 2, var80);
    double var481[2];
    if (input[2] <= -21.351753657849493) {
        set_output2(var481, 1.0, 0.0);
    } else {
        set_output2(var481, 0.0, 1.0);
    }
    add_vectors(var80, var481, 2, var79);
    double var482[2];
    if (input[5] <= 11.368483289768424) {
        if (input[0] <= 1.4868679291995144) {
            set_output2(var482, 1.0, 0.0);
        } else {
            set_output2(var482, 0.6553191489361702, 0.3446808510638298);
        }
    } else {
        set_output2(var482, 0.0, 1.0);
    }
    add_vectors(var79, var482, 2, var78);
    double var483[2];
    if (input[5] <= 11.548103950606572) {
        if (input[5] <= 6.68481567444806) {
            set_output2(var483, 1.0, 0.0);
        } else {
            set_output2(var483, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        set_output2(var483, 0.0, 1.0);
    }
    add_vectors(var78, var483, 2, var77);
    double var484[2];
    if (input[3] <= -33.522464396168715) {
        set_output2(var484, 1.0, 0.0);
    } else {
        if (input[7] <= 1.6116639546711107) {
            set_output2(var484, 0.9694454922670691, 0.03055450773293098);
        } else {
            if (input[9] <= 3.5006225520821275) {
                set_output2(var484, 0.0, 1.0);
            } else {
                set_output2(var484, 0.0, 1.0);
            }
        }
    }
    add_vectors(var77, var484, 2, var76);
    double var485[2];
    if (input[1] <= 18.353571153631368) {
        if (input[4] <= 2.7565572084263903) {
            set_output2(var485, 1.0, 0.0);
        } else {
            set_output2(var485, 0.0, 1.0);
        }
    } else {
        set_output2(var485, 0.0, 1.0);
    }
    add_vectors(var76, var485, 2, var75);
    double var486[2];
    if (input[8] <= 0.7239385063903129) {
        if (input[6] <= 13.769564192370963) {
            set_output2(var486, 1.0, 0.0);
        } else {
            if (input[9] <= 1.0840229815448246) {
                set_output2(var486, 1.0, 0.0);
            } else {
                if (input[1] <= 7.3939827141763335) {
                    set_output2(var486, 1.0, 0.0);
                } else {
                    set_output2(var486, 0.7216494845360825, 0.2783505154639176);
                }
            }
        }
    } else {
        set_output2(var486, 0.0, 1.0);
    }
    add_vectors(var75, var486, 2, var74);
    double var487[2];
    if (input[3] <= -26.945650206697294) {
        set_output2(var487, 1.0, 0.0);
    } else {
        set_output2(var487, 0.0, 1.0);
    }
    add_vectors(var74, var487, 2, var73);
    double var488[2];
    if (input[5] <= 8.303409705660108) {
        set_output2(var488, 1.0, 0.0);
    } else {
        set_output2(var488, 0.0, 1.0);
    }
    add_vectors(var73, var488, 2, var72);
    double var489[2];
    if (input[7] <= 2.5118614652558824) {
        if (input[7] <= -0.17547995810555017) {
            set_output2(var489, 1.0, 0.0);
        } else {
            if (input[5] <= 5.709409429449438) {
                set_output2(var489, 1.0, 0.0);
            } else {
                set_output2(var489, 0.5877862595419847, 0.4122137404580153);
            }
        }
    } else {
        set_output2(var489, 0.0, 1.0);
    }
    add_vectors(var72, var489, 2, var71);
    double var490[2];
    if (input[3] <= -26.966921002795118) {
        set_output2(var490, 1.0, 0.0);
    } else {
        set_output2(var490, 0.0, 1.0);
    }
    add_vectors(var71, var490, 2, var70);
    double var491[2];
    if (input[4] <= 2.641786627490353) {
        set_output2(var491, 1.0, 0.0);
    } else {
        set_output2(var491, 0.0, 1.0);
    }
    add_vectors(var70, var491, 2, var69);
    double var492[2];
    if (input[0] <= 2.2656150097332897) {
        set_output2(var492, 1.0, 0.0);
    } else {
        set_output2(var492, 0.0, 1.0);
    }
    add_vectors(var69, var492, 2, var68);
    double var493[2];
    if (input[0] <= 2.65922910306137) {
        set_output2(var493, 1.0, 0.0);
    } else {
        set_output2(var493, 0.0, 1.0);
    }
    add_vectors(var68, var493, 2, var67);
    double var494[2];
    if (input[5] <= 14.273775356628445) {
        if (input[8] <= 0.6211735780212794) {
            set_output2(var494, 1.0, 0.0);
        } else {
            set_output2(var494, 0.0, 1.0);
        }
    } else {
        set_output2(var494, 0.0, 1.0);
    }
    add_vectors(var67, var494, 2, var66);
    double var495[2];
    if (input[8] <= 0.44917557832933824) {
        set_output2(var495, 1.0, 0.0);
    } else {
        if (input[4] <= 2.1446022749103637) {
            set_output2(var495, 1.0, 0.0);
        } else {
            set_output2(var495, 0.0, 1.0);
        }
    }
    add_vectors(var66, var495, 2, var65);
    double var496[2];
    if (input[9] <= 2.317936992412844) {
        if (input[9] <= 1.239623217434374) {
            set_output2(var496, 1.0, 0.0);
        } else {
            if (input[8] <= 0.2768788542938604) {
                set_output2(var496, 1.0, 0.0);
            } else {
                if (input[6] <= 22.618501342031593) {
                    if (input[5] <= 3.5369675677610672) {
                        set_output2(var496, 1.0, 0.0);
                    } else {
                        set_output2(var496, 1.0, 0.0);
                    }
                } else {
                    set_output2(var496, 0.4873417721518987, 0.5126582278481012);
                }
            }
        }
    } else {
        set_output2(var496, 0.0, 1.0);
    }
    add_vectors(var65, var496, 2, var64);
    double var497[2];
    if (input[4] <= 6.348310317140782) {
        if (input[9] <= 1.4677594596612398) {
            set_output2(var497, 1.0, 0.0);
        } else {
            set_output2(var497, 0.9357652656621729, 0.06423473433782713);
        }
    } else {
        set_output2(var497, 0.0, 1.0);
    }
    add_vectors(var64, var497, 2, var63);
    double var498[2];
    if (input[4] <= 3.4847582943814213) {
        set_output2(var498, 1.0, 0.0);
    } else {
        set_output2(var498, 0.0, 1.0);
    }
    add_vectors(var63, var498, 2, var62);
    double var499[2];
    if (input[2] <= -25.6740858682914) {
        set_output2(var499, 1.0, 0.0);
    } else {
        if (input[4] <= 3.7175108855891716) {
            set_output2(var499, 1.0, 0.0);
        } else {
            set_output2(var499, 0.0, 1.0);
        }
    }
    add_vectors(var62, var499, 2, var61);
    double var500[2];
    if (input[4] <= 4.942492129626743) {
        set_output2(var500, 1.0, 0.0);
    } else {
        set_output2(var500, 0.0, 1.0);
    }
    add_vectors(var61, var500, 2, var60);
    double var501[2];
    if (input[8] <= 0.5748071212891978) {
        set_output2(var501, 1.0, 0.0);
    } else {
        set_output2(var501, 0.0, 1.0);
    }
    add_vectors(var60, var501, 2, var59);
    double var502[2];
    if (input[8] <= 0.4215892224216758) {
        set_output2(var502, 1.0, 0.0);
    } else {
        if (input[9] <= 2.113388462622606) {
            if (input[6] <= 26.373945325784284) {
                set_output2(var502, 1.0, 0.0);
            } else {
                set_output2(var502, 0.7403846153846154, 0.25961538461538464);
            }
        } else {
            set_output2(var502, 0.0, 1.0);
        }
    }
    add_vectors(var59, var502, 2, var58);
    double var503[2];
    if (input[4] <= 2.9675960697851056) {
        set_output2(var503, 1.0, 0.0);
    } else {
        set_output2(var503, 0.0, 1.0);
    }
    add_vectors(var58, var503, 2, var57);
    double var504[2];
    if (input[0] <= 5.753082497592218) {
        if (input[1] <= 8.321447941656555) {
            set_output2(var504, 1.0, 0.0);
        } else {
            set_output2(var504, 0.7403846153846154, 0.25961538461538464);
        }
    } else {
        set_output2(var504, 0.0, 1.0);
    }
    add_vectors(var57, var504, 2, var56);
    double var505[2];
    if (input[0] <= 4.229767465274254) {
        if (input[1] <= 8.549721686397103) {
            set_output2(var505, 1.0, 0.0);
        } else {
            set_output2(var505, 0.6553191489361702, 0.3446808510638298);
        }
    } else {
        set_output2(var505, 0.0, 1.0);
    }
    add_vectors(var56, var505, 2, var55);
    double var506[2];
    if (input[3] <= -23.67215646007601) {
        if (input[2] <= -22.923516397957364) {
            set_output2(var506, 1.0, 0.0);
        } else {
            set_output2(var506, 0.0, 1.0);
        }
    } else {
        set_output2(var506, 0.0, 1.0);
    }
    add_vectors(var55, var506, 2, var54);
    double var507[2];
    if (input[7] <= 1.1761371036463775) {
        if (input[6] <= 8.17290701050913) {
            set_output2(var507, 1.0, 0.0);
        } else {
            if (input[8] <= 0.47559263951870967) {
                set_output2(var507, 1.0, 0.0);
            } else {
                set_output2(var507, 0.6553191489361702, 0.3446808510638298);
            }
        }
    } else {
        if (input[8] <= 0.5822962759142315) {
            set_output2(var507, 1.0, 0.0);
        } else {
            set_output2(var507, 0.0, 1.0);
        }
    }
    add_vectors(var54, var507, 2, var53);
    double var508[2];
    if (input[2] <= -26.204425335476337) {
        set_output2(var508, 1.0, 0.0);
    } else {
        if (input[8] <= 0.6788672964514505) {
            set_output2(var508, 1.0, 0.0);
        } else {
            set_output2(var508, 0.0, 1.0);
        }
    }
    add_vectors(var53, var508, 2, var52);
    double var509[2];
    if (input[4] <= 3.6935481536910273) {
        set_output2(var509, 1.0, 0.0);
    } else {
        set_output2(var509, 0.0, 1.0);
    }
    add_vectors(var52, var509, 2, var51);
    double var510[2];
    if (input[1] <= 7.08524368652933) {
        set_output2(var510, 1.0, 0.0);
    } else {
        if (input[4] <= 2.3208322396356755) {
            set_output2(var510, 1.0, 0.0);
        } else {
            set_output2(var510, 0.0, 1.0);
        }
    }
    add_vectors(var51, var510, 2, var50);
    double var511[2];
    if (input[3] <= -29.062825313088446) {
        set_output2(var511, 1.0, 0.0);
    } else {
        set_output2(var511, 0.0, 1.0);
    }
    add_vectors(var50, var511, 2, var49);
    double var512[2];
    if (input[4] <= 1.9448526413047946) {
        set_output2(var512, 1.0, 0.0);
    } else {
        set_output2(var512, 0.0, 1.0);
    }
    add_vectors(var49, var512, 2, var48);
    double var513[2];
    if (input[3] <= -28.994031210102435) {
        set_output2(var513, 1.0, 0.0);
    } else {
        set_output2(var513, 0.0, 1.0);
    }
    add_vectors(var48, var513, 2, var47);
    double var514[2];
    if (input[3] <= -32.47687355450046) {
        set_output2(var514, 1.0, 0.0);
    } else {
        if (input[5] <= 6.55420544739958) {
            set_output2(var514, 1.0, 0.0);
        } else {
            set_output2(var514, 0.0, 1.0);
        }
    }
    add_vectors(var47, var514, 2, var46);
    double var515[2];
    if (input[2] <= -16.001230618663993) {
        if (input[5] <= 6.947020154342107) {
            set_output2(var515, 1.0, 0.0);
        } else {
            set_output2(var515, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        set_output2(var515, 0.0, 1.0);
    }
    add_vectors(var46, var515, 2, var45);
    double var516[2];
    if (input[8] <= 0.6840440885140419) {
        set_output2(var516, 1.0, 0.0);
    } else {
        set_output2(var516, 0.0, 1.0);
    }
    add_vectors(var45, var516, 2, var44);
    double var517[2];
    if (input[0] <= 2.398391209023544) {
        set_output2(var517, 1.0, 0.0);
    } else {
        set_output2(var517, 0.0, 1.0);
    }
    add_vectors(var44, var517, 2, var43);
    double var518[2];
    if (input[8] <= 0.6499625402322519) {
        set_output2(var518, 1.0, 0.0);
    } else {
        set_output2(var518, 0.0, 1.0);
    }
    add_vectors(var43, var518, 2, var42);
    double var519[2];
    if (input[9] <= 1.457136808493982) {
        set_output2(var519, 1.0, 0.0);
    } else {
        if (input[6] <= 27.571963612829872) {
            set_output2(var519, 0.9357652656621729, 0.06423473433782713);
        } else {
            set_output2(var519, 0.0, 1.0);
        }
    }
    add_vectors(var42, var519, 2, var41);
    double var520[2];
    if (input[5] <= 7.263060004775907) {
        set_output2(var520, 1.0, 0.0);
    } else {
        set_output2(var520, 0.0, 1.0);
    }
    add_vectors(var41, var520, 2, var40);
    double var521[2];
    if (input[3] <= -28.013150941970885) {
        set_output2(var521, 1.0, 0.0);
    } else {
        set_output2(var521, 0.0, 1.0);
    }
    add_vectors(var40, var521, 2, var39);
    double var522[2];
    if (input[8] <= 0.5787048486533529) {
        set_output2(var522, 1.0, 0.0);
    } else {
        set_output2(var522, 0.0, 1.0);
    }
    add_vectors(var39, var522, 2, var38);
    double var523[2];
    if (input[2] <= -22.554893427419046) {
        set_output2(var523, 1.0, 0.0);
    } else {
        set_output2(var523, 0.0, 1.0);
    }
    add_vectors(var38, var523, 2, var37);
    double var524[2];
    if (input[0] <= 5.716929636366916) {
        if (input[0] <= 1.2535199229890965) {
            set_output2(var524, 1.0, 0.0);
        } else {
            if (input[1] <= 6.541226597659875) {
                set_output2(var524, 1.0, 0.0);
            } else {
                set_output2(var524, 0.9517282479141836, 0.04827175208581646);
            }
        }
    } else {
        set_output2(var524, 0.0, 1.0);
    }
    add_vectors(var37, var524, 2, var36);
    double var525[2];
    if (input[8] <= 0.7404724841064354) {
        if (input[4] <= 2.358548939317877) {
            set_output2(var525, 1.0, 0.0);
        } else {
            set_output2(var525, 0.0, 1.0);
        }
    } else {
        set_output2(var525, 0.0, 1.0);
    }
    add_vectors(var36, var525, 2, var35);
    double var526[2];
    if (input[5] <= 7.821059529982448) {
        set_output2(var526, 1.0, 0.0);
    } else {
        set_output2(var526, 0.0, 1.0);
    }
    add_vectors(var35, var526, 2, var34);
    double var527[2];
    if (input[0] <= 2.110403858736526) {
        set_output2(var527, 1.0, 0.0);
    } else {
        set_output2(var527, 0.0, 1.0);
    }
    add_vectors(var34, var527, 2, var33);
    double var528[2];
    if (input[0] <= 3.1969873235597155) {
        set_output2(var528, 1.0, 0.0);
    } else {
        set_output2(var528, 0.0, 1.0);
    }
    add_vectors(var33, var528, 2, var32);
    double var529[2];
    if (input[0] <= 3.2742117105683293) {
        set_output2(var529, 1.0, 0.0);
    } else {
        set_output2(var529, 0.0, 1.0);
    }
    add_vectors(var32, var529, 2, var31);
    double var530[2];
    if (input[3] <= -25.975405478621294) {
        if (input[5] <= 4.332572361931235) {
            set_output2(var530, 1.0, 0.0);
        } else {
            if (input[1] <= 7.541948315927418) {
                set_output2(var530, 1.0, 0.0);
            } else {
                set_output2(var530, 0.6553191489361702, 0.3446808510638298);
            }
        }
    } else {
        set_output2(var530, 0.0, 1.0);
    }
    add_vectors(var31, var530, 2, var30);
    double var531[2];
    if (input[0] <= 2.488163439823226) {
        set_output2(var531, 1.0, 0.0);
    } else {
        set_output2(var531, 0.0, 1.0);
    }
    add_vectors(var30, var531, 2, var29);
    double var532[2];
    if (input[5] <= 8.815751621176277) {
        if (input[1] <= 4.656374203660783) {
            set_output2(var532, 1.0, 0.0);
        } else {
            if (input[3] <= -35.89827307187742) {
                set_output2(var532, 1.0, 0.0);
            } else {
                if (input[6] <= 24.517741817880374) {
                    set_output2(var532, 1.0, 0.0);
                } else {
                    if (input[8] <= 0.4364456330969355) {
                        set_output2(var532, 1.0, 0.0);
                    } else {
                        set_output2(var532, 0.7917737789203084, 0.20822622107969152);
                    }
                }
            }
        }
    } else {
        set_output2(var532, 0.0, 1.0);
    }
    add_vectors(var29, var532, 2, var28);
    double var533[2];
    if (input[3] <= -22.233358750190327) {
        if (input[8] <= 0.584365548455877) {
            set_output2(var533, 1.0, 0.0);
        } else {
            set_output2(var533, 0.0, 1.0);
        }
    } else {
        set_output2(var533, 0.0, 1.0);
    }
    add_vectors(var28, var533, 2, var27);
    double var534[2];
    if (input[2] <= -18.29212590444129) {
        set_output2(var534, 1.0, 0.0);
    } else {
        set_output2(var534, 0.0, 1.0);
    }
    add_vectors(var27, var534, 2, var26);
    double var535[2];
    if (input[8] <= 0.7228581527478106) {
        if (input[7] <= 0.8219958769870663) {
            set_output2(var535, 1.0, 0.0);
        } else {
            if (input[6] <= 26.675209417324634) {
                set_output2(var535, 1.0, 0.0);
            } else {
                set_output2(var535, 0.8261802575107297, 0.1738197424892704);
            }
        }
    } else {
        set_output2(var535, 0.0, 1.0);
    }
    add_vectors(var26, var535, 2, var25);
    double var536[2];
    if (input[3] <= -32.42742677441972) {
        set_output2(var536, 1.0, 0.0);
    } else {
        if (input[3] <= -25.862592369664934) {
            set_output2(var536, 0.9281914893617021, 0.07180851063829789);
        } else {
            set_output2(var536, 0.0, 1.0);
        }
    }
    add_vectors(var25, var536, 2, var24);
    double var537[2];
    if (input[1] <= 13.08498045281159) {
        if (input[9] <= 4.345351806568852) {
            if (input[1] <= 2.470294789551565) {
                set_output2(var537, 1.0, 0.0);
            } else {
                if (input[9] <= 1.0581295765351608) {
                    set_output2(var537, 1.0, 0.0);
                } else {
                    if (input[5] <= 1.8044342683847212) {
                        set_output2(var537, 1.0, 0.0);
                    } else {
                        if (input[3] <= -34.871618084958385) {
                            set_output2(var537, 1.0, 0.0);
                        } else {
                            if (input[1] <= 7.227867470006798) {
                                set_output2(var537, 1.0, 0.0);
                            } else {
                                set_output2(var537, 0.32217573221757323, 0.6778242677824268);
                            }
                        }
                    }
                }
            }
        } else {
            set_output2(var537, 0.0, 1.0);
        }
    } else {
        set_output2(var537, 0.0, 1.0);
    }
    add_vectors(var24, var537, 2, var23);
    double var538[2];
    if (input[5] <= 6.556031463290504) {
        set_output2(var538, 1.0, 0.0);
    } else {
        if (input[4] <= 7.760764370577108) {
            set_output2(var538, 0.32217573221757323, 0.6778242677824268);
        } else {
            set_output2(var538, 0.0, 1.0);
        }
    }
    add_vectors(var23, var538, 2, var22);
    double var539[2];
    if (input[7] <= 2.281992739007501) {
        if (input[6] <= 24.457759086548887) {
            set_output2(var539, 1.0, 0.0);
        } else {
            if (input[8] <= 0.5060172179316957) {
                set_output2(var539, 1.0, 0.0);
            } else {
                set_output2(var539, 0.32217573221757323, 0.6778242677824268);
            }
        }
    } else {
        if (input[9] <= 3.92029942740678) {
            set_output2(var539, 0.19201995012468828, 0.8079800498753118);
        } else {
            set_output2(var539, 0.0, 1.0);
        }
    }
    add_vectors(var22, var539, 2, var21);
    double var540[2];
    if (input[4] <= 7.0207561576414825) {
        if (input[7] <= -0.29441526363709025) {
            set_output2(var540, 1.0, 0.0);
        } else {
            if (input[6] <= 28.145376394475804) {
                set_output2(var540, 1.0, 0.0);
            } else {
                set_output2(var540, 0.8261802575107297, 0.1738197424892704);
            }
        }
    } else {
        set_output2(var540, 0.0, 1.0);
    }
    add_vectors(var21, var540, 2, var20);
    double var541[2];
    if (input[0] <= 3.8117623098819537) {
        set_output2(var541, 1.0, 0.0);
    } else {
        set_output2(var541, 0.0, 1.0);
    }
    add_vectors(var20, var541, 2, var19);
    double var542[2];
    if (input[0] <= 3.4195953554219525) {
        set_output2(var542, 1.0, 0.0);
    } else {
        set_output2(var542, 0.0, 1.0);
    }
    add_vectors(var19, var542, 2, var18);
    double var543[2];
    if (input[0] <= 2.460138911789822) {
        set_output2(var543, 1.0, 0.0);
    } else {
        set_output2(var543, 0.0, 1.0);
    }
    add_vectors(var18, var543, 2, var17);
    double var544[2];
    if (input[4] <= 7.686162078059457) {
        if (input[8] <= 0.48566320081719566) {
            set_output2(var544, 1.0, 0.0);
        } else {
            set_output2(var544, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        set_output2(var544, 0.0, 1.0);
    }
    add_vectors(var17, var544, 2, var16);
    double var545[2];
    if (input[0] <= 7.111486005440358) {
        if (input[5] <= 6.314388758225057) {
            set_output2(var545, 1.0, 0.0);
        } else {
            set_output2(var545, 0.32217573221757323, 0.6778242677824268);
        }
    } else {
        set_output2(var545, 0.0, 1.0);
    }
    add_vectors(var16, var545, 2, var15);
    double var546[2];
    if (input[2] <= -25.439270545364316) {
        set_output2(var546, 1.0, 0.0);
    } else {
        if (input[7] <= 1.5192947343871743) {
            set_output2(var546, 0.7403846153846154, 0.25961538461538464);
        } else {
            set_output2(var546, 0.0, 1.0);
        }
    }
    add_vectors(var15, var546, 2, var14);
    double var547[2];
    if (input[3] <= -25.815343382762947) {
        if (input[5] <= 2.8623754565893123) {
            set_output2(var547, 1.0, 0.0);
        } else {
            if (input[2] <= -25.418315869002363) {
                set_output2(var547, 1.0, 0.0);
            } else {
                set_output2(var547, 0.7403846153846154, 0.25961538461538464);
            }
        }
    } else {
        set_output2(var547, 0.0, 1.0);
    }
    add_vectors(var14, var547, 2, var13);
    double var548[2];
    if (input[4] <= 4.141331958809053) {
        set_output2(var548, 1.0, 0.0);
    } else {
        set_output2(var548, 0.0, 1.0);
    }
    add_vectors(var13, var548, 2, var12);
    double var549[2];
    if (input[8] <= 0.6989827119552091) {
        set_output2(var549, 1.0, 0.0);
    } else {
        set_output2(var549, 0.0, 1.0);
    }
    add_vectors(var12, var549, 2, var11);
    double var550[2];
    if (input[2] <= -16.825298208940808) {
        if (input[2] <= -35.605652988744104) {
            set_output2(var550, 1.0, 0.0);
        } else {
            if (input[2] <= -26.966177035771317) {
                set_output2(var550, 1.0, 0.0);
            } else {
                set_output2(var550, 0.8057553956834533, 0.19424460431654678);
            }
        }
    } else {
        set_output2(var550, 0.0, 1.0);
    }
    add_vectors(var11, var550, 2, var10);
    double var551[2];
    if (input[4] <= 3.58938501250137) {
        set_output2(var551, 1.0, 0.0);
    } else {
        set_output2(var551, 0.0, 1.0);
    }
    add_vectors(var10, var551, 2, var9);
    double var552[2];
    if (input[2] <= -22.635556786477387) {
        set_output2(var552, 1.0, 0.0);
    } else {
        set_output2(var552, 0.0, 1.0);
    }
    add_vectors(var9, var552, 2, var8);
    double var553[2];
    if (input[3] <= -30.951831897938483) {
        set_output2(var553, 1.0, 0.0);
    } else {
        set_output2(var553, 0.0, 1.0);
    }
    add_vectors(var8, var553, 2, var7);
    double var554[2];
    if (input[8] <= 0.5247812107870425) {
        set_output2(var554, 1.0, 0.0);
    } else {
        set_output2(var554, 0.0, 1.0);
    }
    add_vectors(var7, var554, 2, var6);
    double var555[2];
    if (input[3] <= -28.6705641550035) {
        set_output2(var555, 1.0, 0.0);
    } else {
        set_output2(var555, 0.0, 1.0);
    }
    add_vectors(var6, var555, 2, var5);
    double var556[2];
    if (input[6] <= 32.3860666848931) {
        if (input[3] <= -30.359345678034547) {
            set_output2(var556, 1.0, 0.0);
        } else {
            set_output2(var556, 0.0, 1.0);
        }
    } else {
        set_output2(var556, 0.0, 1.0);
    }
    add_vectors(var5, var556, 2, var4);
    double var557[2];
    if (input[8] <= 0.6579484983502575) {
        set_output2(var557, 1.0, 0.0);
    } else {
        set_output2(var557, 0.0, 1.0);
    }
    add_vectors(var4, var557, 2, var3);
    double var558[2];
    if (input[5] <= 6.162993121141707) {
        set_output2(var558, 1.0, 0.0);
    } else {
        if (input[4] <= 7.595022976129846) {
            set_output2(var558, 0.32217573221757323, 0.6778242677824268);
        } else {
            set_output2(var558, 0.0, 1.0);
        }
    }
    add_vectors(var3, var558, 2, var2);
    double var559[2];
    if (input[9] <= 2.3638301181455628) {
        if (input[5] <= 2.648333364474574) {
            set_output2(var559, 1.0, 0.0);
        } else {
            if (input[5] <= 6.179970999512008) {
                set_output2(var559, 1.0, 0.0);
            } else {
                set_output2(var559, 0.32217573221757323, 0.6778242677824268);
            }
        }
    } else {
        set_output2(var559, 0.0, 1.0);
    }
    add_vectors(var2, var559, 2, var1);
    mul_vector_number(var1, 0.0035714285714285713, 2, var0);
    memcpy(output, var0, 2 * sizeof(double));
}
