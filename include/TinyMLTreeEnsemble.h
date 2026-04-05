#pragma once
// Auto-generated C header from scikit-learn ExtraTrees (m2cgen)
#define ARC_MODEL_FEATURE_VERSION 4
#define ARC_THRESHOLD 0.1100

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
    if (input[9] <= 0.6247325200013608) {
        if (input[9] <= 0.43022826235064165) {
            if (input[2] <= -14.543651574114923) {
                if (input[3] <= -47.238786959894306) {
                    set_output2(var160, 1.0, 0.0);
                } else {
                    set_output2(var160, 0.8608964451313755, 0.1391035548686244);
                }
            } else {
                if (input[7] <= 13.549507681015909) {
                    if (input[7] <= 11.43399068265754) {
                        set_output2(var160, 0.0, 1.0);
                    } else {
                        if (input[0] <= 33.38562417004748) {
                            set_output2(var160, 0.7804878048780488, 0.21951219512195122);
                        } else {
                            set_output2(var160, 1.0, 0.0);
                        }
                    }
                } else {
                    if (input[2] <= -13.198661918682008) {
                        set_output2(var160, 0.7272727272727273, 0.2727272727272727);
                    } else {
                        if (input[7] <= 14.417676793723073) {
                            set_output2(var160, 0.899188876013905, 0.10081112398609501);
                        } else {
                            set_output2(var160, 0.9233152594887684, 0.0766847405112316);
                        }
                    }
                }
            }
        } else {
            set_output2(var160, 0.0, 1.0);
        }
    } else {
        set_output2(var160, 0.0, 1.0);
    }
    double var161[2];
    if (input[9] <= 8.738169626090329) {
        if (input[2] <= -1.733648059215735) {
            if (input[9] <= 2.678421613323858) {
                if (input[4] <= 18.18627628951516) {
                    if (input[9] <= 1.0066560521448649) {
                        if (input[7] <= 11.3945413382926) {
                            set_output2(var161, 0.0, 1.0);
                        } else {
                            set_output2(var161, 0.9000690579680708, 0.09993094203192915);
                        }
                    } else {
                        set_output2(var161, 0.0, 1.0);
                    }
                } else {
                    set_output2(var161, 0.0, 1.0);
                }
            } else {
                set_output2(var161, 0.0, 1.0);
            }
        } else {
            set_output2(var161, 0.0, 1.0);
        }
    } else {
        set_output2(var161, 0.0, 1.0);
    }
    add_vectors(var160, var161, 2, var159);
    double var162[2];
    if (input[8] <= 1.6067252411202322) {
        if (input[3] <= -41.55280345273525) {
            if (input[9] <= 0.40540027154559743) {
                if (input[1] <= 10.280568972010272) {
                    if (input[9] <= 0.009160334420640502) {
                        set_output2(var162, 0.8, 0.2);
                    } else {
                        if (input[7] <= 13.526021213911852) {
                            set_output2(var162, 0.833743842364532, 0.16625615763546797);
                        } else {
                            set_output2(var162, 0.933491393909536, 0.06650860609046402);
                        }
                    }
                } else {
                    if (input[0] <= 27.33072430153684) {
                        set_output2(var162, 0.7804878048780488, 0.21951219512195122);
                    } else {
                        if (input[0] <= 27.789958349479413) {
                            set_output2(var162, 0.5714285714285714, 0.42857142857142855);
                        } else {
                            set_output2(var162, 0.899365367180417, 0.10063463281958296);
                        }
                    }
                }
            } else {
                if (input[7] <= 13.679875551242835) {
                    set_output2(var162, 0.34782608695652173, 0.6521739130434783);
                } else {
                    set_output2(var162, 0.0, 1.0);
                }
            }
        } else {
            if (input[9] <= 2.2795288390842465) {
                if (input[1] <= 11.401883793096408) {
                    set_output2(var162, 0.8767123287671232, 0.1232876712328767);
                } else {
                    set_output2(var162, 0.6808510638297872, 0.3191489361702128);
                }
            } else {
                set_output2(var162, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var162, 0.0, 1.0);
    }
    add_vectors(var159, var162, 2, var158);
    double var163[2];
    if (input[3] <= -35.750989235158556) {
        if (input[4] <= 20.403124472693296) {
            if (input[1] <= 9.171482669079595) {
                if (input[9] <= 5.6282973773047456) {
                    if (input[4] <= 0.4793927073835166) {
                        set_output2(var163, 0.7272727272727273, 0.2727272727272727);
                    } else {
                        if (input[9] <= 1.2728204987081813) {
                            set_output2(var163, 0.9324234795282894, 0.0675765204717106);
                        } else {
                            set_output2(var163, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var163, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 1.1990301596320756) {
                    if (input[5] <= 0.24743360701624165) {
                        if (input[9] <= 0.6463626547455875) {
                            set_output2(var163, 0.8944723618090452, 0.10552763819095477);
                        } else {
                            set_output2(var163, 0.0, 1.0);
                        }
                    } else {
                        if (input[9] <= 0.45139254953588637) {
                            set_output2(var163, 0.8275862068965517, 0.1724137931034483);
                        } else {
                            set_output2(var163, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var163, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var163, 0.0, 1.0);
        }
    } else {
        set_output2(var163, 0.0, 1.0);
    }
    add_vectors(var158, var163, 2, var157);
    double var164[2];
    if (input[9] <= 8.617987669627635) {
        if (input[4] <= 14.795253477058582) {
            if (input[9] <= 3.412195306725483) {
                if (input[9] <= 2.6486225891389537) {
                    if (input[9] <= 0.6806385507775136) {
                        if (input[1] <= 7.663849962474193) {
                            set_output2(var164, 0.9431219717716453, 0.05687802822835475);
                        } else {
                            set_output2(var164, 0.8924785461887935, 0.10752145381120647);
                        }
                    } else {
                        set_output2(var164, 0.0, 1.0);
                    }
                } else {
                    set_output2(var164, 0.0, 1.0);
                }
            } else {
                set_output2(var164, 0.0, 1.0);
            }
        } else {
            set_output2(var164, 0.0, 1.0);
        }
    } else {
        set_output2(var164, 0.0, 1.0);
    }
    add_vectors(var157, var164, 2, var156);
    double var165[2];
    if (input[9] <= 0.5424463903443342) {
        if (input[2] <= -21.651298864189414) {
            set_output2(var165, 1.0, 0.0);
        } else {
            if (input[7] <= 13.79758649009231) {
                if (input[4] <= 3.7656417464794756) {
                    if (input[0] <= 28.936593824895663) {
                        set_output2(var165, 0.0, 1.0);
                    } else {
                        if (input[7] <= 11.841569978079594) {
                            set_output2(var165, 0.0, 1.0);
                        } else {
                            set_output2(var165, 0.7958115183246073, 0.20418848167539266);
                        }
                    }
                } else {
                    if (input[0] <= 31.613275593452123) {
                        set_output2(var165, 1.0, 0.0);
                    } else {
                        set_output2(var165, 0.8163265306122449, 0.1836734693877551);
                    }
                }
            } else {
                if (input[3] <= -38.80050495906357) {
                    if (input[4] <= 1.0247599504716776) {
                        if (input[0] <= 35.3504269097576) {
                            set_output2(var165, 0.9672637992179686, 0.03273620078203146);
                        } else {
                            set_output2(var165, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        if (input[8] <= 0.7723287796594582) {
                            set_output2(var165, 0.914738929279577, 0.08526107072042301);
                        } else {
                            set_output2(var165, 0.8662420382165605, 0.1337579617834395);
                        }
                    }
                } else {
                    set_output2(var165, 0.8, 0.2);
                }
            }
        }
    } else {
        set_output2(var165, 0.0, 1.0);
    }
    add_vectors(var156, var165, 2, var155);
    double var166[2];
    if (input[9] <= 10.327422216423741) {
        if (input[9] <= 4.127711157502867) {
            if (input[9] <= 0.9754611761100321) {
                if (input[9] <= 0.29309049223108996) {
                    if (input[7] <= 11.945633085410236) {
                        set_output2(var166, 0.0, 1.0);
                    } else {
                        if (input[2] <= -21.632052922276202) {
                            set_output2(var166, 1.0, 0.0);
                        } else {
                            set_output2(var166, 0.9062724291765766, 0.09372757082342345);
                        }
                    }
                } else {
                    set_output2(var166, 0.5333333333333333, 0.4666666666666667);
                }
            } else {
                set_output2(var166, 0.0, 1.0);
            }
        } else {
            set_output2(var166, 0.0, 1.0);
        }
    } else {
        set_output2(var166, 0.0, 1.0);
    }
    add_vectors(var155, var166, 2, var154);
    double var167[2];
    if (input[6] <= 46.751617791958935) {
        if (input[9] <= 0.9123398516236005) {
            if (input[7] <= 12.085688384950602) {
                set_output2(var167, 0.0, 1.0);
            } else {
                if (input[1] <= 7.41748706825121) {
                    if (input[9] <= 0.04999570499732859) {
                        if (input[6] <= 28.06073092240782) {
                            set_output2(var167, 0.845581927366314, 0.154418072633686);
                        } else {
                            set_output2(var167, 1.0, 0.0);
                        }
                    } else {
                        if (input[2] <= -9.565383714354272) {
                            set_output2(var167, 1.0, 0.0);
                        } else {
                            set_output2(var167, 0.9411764705882353, 0.058823529411764705);
                        }
                    }
                } else {
                    if (input[9] <= 0.652513341459492) {
                        if (input[9] <= 0.41773558088871826) {
                            set_output2(var167, 0.9022556390977443, 0.09774436090225563);
                        } else {
                            set_output2(var167, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var167, 0.0, 1.0);
                    }
                }
            }
        } else {
            set_output2(var167, 0.0, 1.0);
        }
    } else {
        set_output2(var167, 0.7272727272727273, 0.2727272727272727);
    }
    add_vectors(var154, var167, 2, var153);
    double var168[2];
    if (input[9] <= 8.666096064488974) {
        if (input[9] <= 0.38723544071420424) {
            if (input[2] <= -16.751018947657656) {
                set_output2(var168, 1.0, 0.0);
            } else {
                if (input[5] <= 0.2266526931602961) {
                    if (input[7] <= 12.824905924900651) {
                        set_output2(var168, 0.5714285714285714, 0.42857142857142855);
                    } else {
                        if (input[5] <= 0.034209775730646284) {
                            set_output2(var168, 0.6153846153846154, 0.38461538461538464);
                        } else {
                            set_output2(var168, 0.9171270718232044, 0.08287292817679558);
                        }
                    }
                } else {
                    if (input[9] <= 0.04456486293731156) {
                        set_output2(var168, 0.47058823529411764, 0.5294117647058824);
                    } else {
                        if (input[0] <= 33.50203384047919) {
                            set_output2(var168, 0.9719626168224299, 0.028037383177570093);
                        } else {
                            set_output2(var168, 0.0, 1.0);
                        }
                    }
                }
            }
        } else {
            if (input[0] <= 29.974755187951175) {
                set_output2(var168, 0.34782608695652173, 0.6521739130434783);
            } else {
                set_output2(var168, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var168, 0.0, 1.0);
    }
    add_vectors(var153, var168, 2, var152);
    double var169[2];
    if (input[9] <= 10.98696317079605) {
        if (input[9] <= 4.324435835718627) {
            if (input[9] <= 0.32762376665183907) {
                if (input[1] <= 5.679605066857922) {
                    if (input[5] <= 0.21213933286732833) {
                        if (input[5] <= 0.1396662602755341) {
                            set_output2(var169, 0.9866151100535395, 0.01338488994646044);
                        } else {
                            set_output2(var169, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        set_output2(var169, 0.7272727272727273, 0.2727272727272727);
                    }
                } else {
                    if (input[7] <= 14.862386993876942) {
                        if (input[7] <= 11.598464158766289) {
                            set_output2(var169, 0.0, 1.0);
                        } else {
                            set_output2(var169, 0.8973603997760591, 0.10263960022394095);
                        }
                    } else {
                        if (input[8] <= 0.6698751587242302) {
                            set_output2(var169, 0.9502762430939227, 0.049723756906077346);
                        } else {
                            set_output2(var169, 0.8484848484848485, 0.15151515151515152);
                        }
                    }
                }
            } else {
                if (input[9] <= 0.7850149796172264) {
                    set_output2(var169, 0.5714285714285714, 0.42857142857142855);
                } else {
                    set_output2(var169, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var169, 0.0, 1.0);
        }
    } else {
        set_output2(var169, 0.0, 1.0);
    }
    add_vectors(var152, var169, 2, var151);
    double var170[2];
    if (input[9] <= 9.187481765143348) {
        if (input[9] <= 6.509310099598324) {
            if (input[9] <= 4.018366928592289) {
                if (input[9] <= 0.2403094651143857) {
                    if (input[7] <= 13.582148827440605) {
                        if (input[6] <= 13.083906211797887) {
                            set_output2(var170, 0.8447271943066638, 0.1552728056933362);
                        } else {
                            set_output2(var170, 0.5400340715502555, 0.45996592844974443);
                        }
                    } else {
                        if (input[0] <= 34.92715929850803) {
                            set_output2(var170, 0.9074620324641395, 0.09253796753586058);
                        } else {
                            set_output2(var170, 0.9552238805970149, 0.04477611940298507);
                        }
                    }
                } else {
                    if (input[9] <= 1.5851911831848233) {
                        if (input[6] <= 8.179346693180626) {
                            set_output2(var170, 0.7920792079207921, 0.2079207920792079);
                        } else {
                            set_output2(var170, 0.3076923076923077, 0.6923076923076923);
                        }
                    } else {
                        set_output2(var170, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var170, 0.0, 1.0);
            }
        } else {
            set_output2(var170, 0.0, 1.0);
        }
    } else {
        set_output2(var170, 0.0, 1.0);
    }
    add_vectors(var151, var170, 2, var150);
    double var171[2];
    if (input[1] <= 13.789333439746418) {
        if (input[2] <= -3.694966490248124) {
            if (input[4] <= 12.64952594174487) {
                if (input[9] <= 4.0817900044607685) {
                    if (input[7] <= 13.867033571283214) {
                        if (input[0] <= 27.84365263006231) {
                            set_output2(var171, 0.47058823529411764, 0.5294117647058824);
                        } else {
                            set_output2(var171, 0.7983946238566362, 0.20160537614336382);
                        }
                    } else {
                        if (input[9] <= 2.3034345788087527) {
                            set_output2(var171, 0.9147373144747121, 0.08526268552528796);
                        } else {
                            set_output2(var171, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var171, 0.0, 1.0);
                }
            } else {
                set_output2(var171, 0.64, 0.36);
            }
        } else {
            if (input[2] <= -1.7673846968554936) {
                if (input[9] <= 0.6358893212711724) {
                    set_output2(var171, 0.8888888888888888, 0.1111111111111111);
                } else {
                    set_output2(var171, 0.0, 1.0);
                }
            } else {
                set_output2(var171, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var171, 0.0, 1.0);
    }
    add_vectors(var150, var171, 2, var149);
    double var172[2];
    if (input[9] <= 1.031560501143746) {
        if (input[9] <= 0.4119832214681686) {
            if (input[7] <= 11.747550804495658) {
                set_output2(var172, 0.0, 1.0);
            } else {
                if (input[1] <= 5.824261651916466) {
                    if (input[5] <= 0.25821286422726186) {
                        if (input[6] <= 67.03155008074532) {
                            set_output2(var172, 0.9789989499474974, 0.021001050052502624);
                        } else {
                            set_output2(var172, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        set_output2(var172, 0.7272727272727273, 0.2727272727272727);
                    }
                } else {
                    if (input[7] <= 13.204633654567317) {
                        set_output2(var172, 0.7529411764705882, 0.24705882352941178);
                    } else {
                        if (input[7] <= 14.049541366530095) {
                            set_output2(var172, 0.862533692722372, 0.13746630727762804);
                        } else {
                            set_output2(var172, 0.9155182744158179, 0.08448172558418214);
                        }
                    }
                }
            }
        } else {
            set_output2(var172, 0.27586206896551724, 0.7241379310344828);
        }
    } else {
        set_output2(var172, 0.0, 1.0);
    }
    add_vectors(var149, var172, 2, var148);
    double var173[2];
    if (input[8] <= 1.3447008410699623) {
        if (input[9] <= 6.279862965182309) {
            if (input[9] <= 2.3066337494070215) {
                if (input[9] <= 0.5137438611041204) {
                    if (input[5] <= 0.3239360835830947) {
                        if (input[2] <= -19.01340176161155) {
                            set_output2(var173, 1.0, 0.0);
                        } else {
                            set_output2(var173, 0.9052092228864219, 0.09479077711357814);
                        }
                    } else {
                        set_output2(var173, 0.7692307692307693, 0.23076923076923078);
                    }
                } else {
                    set_output2(var173, 0.0, 1.0);
                }
            } else {
                set_output2(var173, 0.0, 1.0);
            }
        } else {
            set_output2(var173, 0.0, 1.0);
        }
    } else {
        set_output2(var173, 0.0, 1.0);
    }
    add_vectors(var148, var173, 2, var147);
    double var174[2];
    if (input[9] <= 6.135970252171973) {
        if (input[9] <= 1.3298707636185274) {
            if (input[1] <= 6.3623072849612425) {
                if (input[9] <= 0.04004332585818906) {
                    if (input[8] <= 0.6588322779384408) {
                        set_output2(var174, 0.9255275134464211, 0.07447248655357883);
                    } else {
                        set_output2(var174, 0.7804878048780488, 0.21951219512195122);
                    }
                } else {
                    if (input[0] <= 34.41237285297263) {
                        set_output2(var174, 1.0, 0.0);
                    } else {
                        set_output2(var174, 0.9411764705882353, 0.058823529411764705);
                    }
                }
            } else {
                if (input[9] <= 0.7047875723904553) {
                    if (input[7] <= 13.647961500977155) {
                        if (input[8] <= 0.7875211887212896) {
                            set_output2(var174, 0.6842105263157895, 0.3157894736842105);
                        } else {
                            set_output2(var174, 0.9491525423728814, 0.05084745762711865);
                        }
                    } else {
                        if (input[1] <= 7.955927229736764) {
                            set_output2(var174, 0.8057553956834532, 0.19424460431654678);
                        } else {
                            set_output2(var174, 0.9178082191780822, 0.0821917808219178);
                        }
                    }
                } else {
                    set_output2(var174, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var174, 0.0, 1.0);
        }
    } else {
        set_output2(var174, 0.0, 1.0);
    }
    add_vectors(var147, var174, 2, var146);
    double var175[2];
    if (input[9] <= 2.918107193362144) {
        if (input[9] <= 2.414661540895966) {
            if (input[7] <= 12.895814595859717) {
                set_output2(var175, 0.6557377049180327, 0.3442622950819672);
            } else {
                if (input[7] <= 13.70943814405182) {
                    if (input[5] <= 0.055285595232456616) {
                        set_output2(var175, 0.4, 0.6);
                    } else {
                        if (input[2] <= -11.675225062262495) {
                            set_output2(var175, 1.0, 0.0);
                        } else {
                            set_output2(var175, 0.7804878048780488, 0.21951219512195122);
                        }
                    }
                } else {
                    if (input[4] <= 14.479910171658366) {
                        if (input[9] <= 1.5872239056206061) {
                            set_output2(var175, 0.9131444426014446, 0.08685555739855542);
                        } else {
                            set_output2(var175, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var175, 0.0, 1.0);
                    }
                }
            }
        } else {
            set_output2(var175, 0.0, 1.0);
        }
    } else {
        set_output2(var175, 0.0, 1.0);
    }
    add_vectors(var146, var175, 2, var145);
    double var176[2];
    if (input[9] <= 12.401883308059293) {
        if (input[9] <= 4.791599966714371) {
            if (input[7] <= 11.708847765508978) {
                set_output2(var176, 0.0, 1.0);
            } else {
                if (input[8] <= 1.4539079801013028) {
                    if (input[9] <= 1.1200409409787333) {
                        if (input[9] <= 0.7451478254599134) {
                            set_output2(var176, 0.9033717943490847, 0.09662820565091533);
                        } else {
                            set_output2(var176, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var176, 0.0, 1.0);
                    }
                } else {
                    set_output2(var176, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var176, 0.0, 1.0);
        }
    } else {
        set_output2(var176, 0.0, 1.0);
    }
    add_vectors(var145, var176, 2, var144);
    double var177[2];
    if (input[3] <= -38.58030496826295) {
        if (input[5] <= 0.038167291697147956) {
            set_output2(var177, 0.5, 0.5);
        } else {
            if (input[7] <= 12.176898151939861) {
                set_output2(var177, 0.4, 0.6);
            } else {
                if (input[9] <= 4.296887747214312) {
                    if (input[1] <= 9.475844673695352) {
                        if (input[4] <= 1.6972037670174382) {
                            set_output2(var177, 0.8741098677517802, 0.1258901322482197);
                        } else {
                            set_output2(var177, 0.9507038396231582, 0.04929616037684176);
                        }
                    } else {
                        if (input[9] <= 1.2372799424600873) {
                            set_output2(var177, 0.8964169381107492, 0.10358306188925082);
                        } else {
                            set_output2(var177, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var177, 0.0, 1.0);
                }
            }
        }
    } else {
        set_output2(var177, 0.0, 1.0);
    }
    add_vectors(var144, var177, 2, var143);
    double var178[2];
    if (input[2] <= -1.3941146519175511) {
        if (input[9] <= 0.37946971784351335) {
            if (input[5] <= 0.41579404933858966) {
                if (input[7] <= 13.735106544314865) {
                    if (input[8] <= 0.7557326679318577) {
                        if (input[7] <= 11.568122342355604) {
                            set_output2(var178, 0.0, 1.0);
                        } else {
                            set_output2(var178, 0.7565099763273588, 0.2434900236726412);
                        }
                    } else {
                        set_output2(var178, 0.963855421686747, 0.03614457831325301);
                    }
                } else {
                    if (input[6] <= 75.48958308300212) {
                        if (input[9] <= 0.18356513818812678) {
                            set_output2(var178, 0.9151512953931504, 0.08484870460684954);
                        } else {
                            set_output2(var178, 0.9688581314878892, 0.031141868512110725);
                        }
                    } else {
                        set_output2(var178, 0.7272727272727273, 0.2727272727272727);
                    }
                }
            } else {
                set_output2(var178, 0.5714285714285714, 0.42857142857142855);
            }
        } else {
            if (input[0] <= 26.779385362788318) {
                set_output2(var178, 0.7272727272727273, 0.2727272727272727);
            } else {
                set_output2(var178, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var178, 0.0, 1.0);
    }
    add_vectors(var143, var178, 2, var142);
    double var179[2];
    if (input[9] <= 4.601411798695934) {
        if (input[7] <= 14.262783245452818) {
            if (input[9] <= 1.34790628748658) {
                if (input[1] <= 7.478737025905678) {
                    if (input[5] <= 0.17740330242385496) {
                        if (input[9] <= 0.044339733730260664) {
                            set_output2(var179, 0.9201419698314108, 0.07985803016858917);
                        } else {
                            set_output2(var179, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var179, 0.0, 1.0);
                    }
                } else {
                    if (input[9] <= 0.48631513807709736) {
                        if (input[7] <= 13.32347853244505) {
                            set_output2(var179, 0.7457627118644068, 0.2542372881355932);
                        } else {
                            set_output2(var179, 0.8888888888888888, 0.1111111111111111);
                        }
                    } else {
                        set_output2(var179, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var179, 0.0, 1.0);
            }
        } else {
            if (input[9] <= 0.6373470691845942) {
                if (input[5] <= 0.24445928163998176) {
                    if (input[3] <= -53.27672950359626) {
                        if (input[0] <= 32.79226283865074) {
                            set_output2(var179, 0.7567567567567568, 0.24324324324324326);
                        } else {
                            set_output2(var179, 0.9361702127659575, 0.06382978723404255);
                        }
                    } else {
                        if (input[0] <= 27.704859672181506) {
                            set_output2(var179, 0.7272727272727273, 0.2727272727272727);
                        } else {
                            set_output2(var179, 0.9369369369369369, 0.06306306306306306);
                        }
                    }
                } else {
                    set_output2(var179, 0.810126582278481, 0.189873417721519);
                }
            } else {
                set_output2(var179, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var179, 0.0, 1.0);
    }
    add_vectors(var142, var179, 2, var141);
    double var180[2];
    if (input[7] <= 12.022212585056412) {
        set_output2(var180, 0.0, 1.0);
    } else {
        if (input[4] <= 18.40816826410464) {
            if (input[9] <= 10.765849367157909) {
                if (input[3] <= -35.632272284724635) {
                    if (input[9] <= 4.453980815388608) {
                        if (input[0] <= 28.01874231878354) {
                            set_output2(var180, 0.8163265306122449, 0.1836734693877551);
                        } else {
                            set_output2(var180, 0.8955843502327336, 0.10441564976726633);
                        }
                    } else {
                        set_output2(var180, 0.0, 1.0);
                    }
                } else {
                    set_output2(var180, 0.0, 1.0);
                }
            } else {
                set_output2(var180, 0.0, 1.0);
            }
        } else {
            set_output2(var180, 0.0, 1.0);
        }
    }
    add_vectors(var141, var180, 2, var140);
    double var181[2];
    if (input[3] <= -40.94986553068905) {
        if (input[1] <= 10.050406175902868) {
            if (input[4] <= 6.32438879798877) {
                if (input[8] <= 0.43466294940246125) {
                    set_output2(var181, 0.5714285714285714, 0.42857142857142855);
                } else {
                    if (input[0] <= 30.911265122961055) {
                        if (input[5] <= 0.1456966825193043) {
                            set_output2(var181, 0.958124927300221, 0.04187507269977899);
                        } else {
                            set_output2(var181, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        if (input[9] <= 6.019829449334411) {
                            set_output2(var181, 0.8896954841142641, 0.11030451588573582);
                        } else {
                            set_output2(var181, 0.0, 1.0);
                        }
                    }
                }
            } else {
                set_output2(var181, 0.9696969696969697, 0.030303030303030304);
            }
        } else {
            if (input[9] <= 5.042439753369698) {
                if (input[9] <= 1.233821004777443) {
                    if (input[9] <= 0.8294909745506361) {
                        if (input[7] <= 15.198056750909878) {
                            set_output2(var181, 0.8817204301075269, 0.11827956989247312);
                        } else {
                            set_output2(var181, 0.9411764705882353, 0.058823529411764705);
                        }
                    } else {
                        set_output2(var181, 0.0, 1.0);
                    }
                } else {
                    set_output2(var181, 0.0, 1.0);
                }
            } else {
                set_output2(var181, 0.0, 1.0);
            }
        }
    } else {
        if (input[2] <= -3.3901651982462147) {
            if (input[7] <= 12.176832230027024) {
                set_output2(var181, 0.0, 1.0);
            } else {
                set_output2(var181, 0.8275862068965517, 0.1724137931034483);
            }
        } else {
            set_output2(var181, 0.0, 1.0);
        }
    }
    add_vectors(var140, var181, 2, var139);
    double var182[2];
    if (input[7] <= 12.136065284648096) {
        set_output2(var182, 0.4, 0.6);
    } else {
        if (input[8] <= 0.410163684017114) {
            set_output2(var182, 0.64, 0.36);
        } else {
            if (input[9] <= 2.15557678409217) {
                if (input[9] <= 1.1190219222944269) {
                    if (input[7] <= 14.497399899539356) {
                        if (input[4] <= 5.369797529146792) {
                            set_output2(var182, 0.8622824810504964, 0.13771751894950357);
                        } else {
                            set_output2(var182, 0.961038961038961, 0.03896103896103896);
                        }
                    } else {
                        if (input[7] <= 15.025105922115266) {
                            set_output2(var182, 0.9243027888446215, 0.07569721115537849);
                        } else {
                            set_output2(var182, 0.9032258064516129, 0.0967741935483871);
                        }
                    }
                } else {
                    set_output2(var182, 0.0, 1.0);
                }
            } else {
                set_output2(var182, 0.0, 1.0);
            }
        }
    }
    add_vectors(var139, var182, 2, var138);
    double var183[2];
    if (input[9] <= 1.9329215229193912) {
        if (input[7] <= 13.08903210447747) {
            set_output2(var183, 0.7, 0.3);
        } else {
            if (input[9] <= 0.3877217463458428) {
                if (input[0] <= 30.713682209647004) {
                    if (input[9] <= 0.2289569221900706) {
                        if (input[3] <= -45.936300854862296) {
                            set_output2(var183, 0.9227615142557453, 0.07723848574425479);
                        } else {
                            set_output2(var183, 0.9784172661870504, 0.02158273381294964);
                        }
                    } else {
                        set_output2(var183, 0.8, 0.2);
                    }
                } else {
                    if (input[0] <= 34.15472312695287) {
                        if (input[7] <= 15.425461368367971) {
                            set_output2(var183, 0.8866327404645539, 0.11336725953544616);
                        } else {
                            set_output2(var183, 0.963855421686747, 0.03614457831325301);
                        }
                    } else {
                        if (input[4] <= 3.0483085909569523) {
                            set_output2(var183, 0.9142857142857143, 0.08571428571428572);
                        } else {
                            set_output2(var183, 0.9815950920245399, 0.018404907975460124);
                        }
                    }
                }
            } else {
                set_output2(var183, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var183, 0.0, 1.0);
    }
    add_vectors(var138, var183, 2, var137);
    double var184[2];
    if (input[9] <= 0.14966978649777038) {
        if (input[6] <= 59.4035982967233) {
            if (input[2] <= -17.10347372964955) {
                set_output2(var184, 1.0, 0.0);
            } else {
                if (input[4] <= 7.906209308795943) {
                    if (input[7] <= 13.810703297906706) {
                        if (input[7] <= 13.07275885758886) {
                            set_output2(var184, 0.6153846153846154, 0.38461538461538464);
                        } else {
                            set_output2(var184, 0.8934010152284264, 0.1065989847715736);
                        }
                    } else {
                        if (input[8] <= 0.833704132541518) {
                            set_output2(var184, 0.9138051857042747, 0.0861948142957253);
                        } else {
                            set_output2(var184, 0.8, 0.2);
                        }
                    }
                } else {
                    set_output2(var184, 0.8695652173913043, 0.13043478260869565);
                }
            }
        } else {
            set_output2(var184, 0.7272727272727273, 0.2727272727272727);
        }
    } else {
        if (input[9] <= 9.23638569993903) {
            if (input[9] <= 4.975338031762443) {
                if (input[9] <= 1.275428519428835) {
                    if (input[3] <= -46.766126852719324) {
                        if (input[0] <= 33.70563546846695) {
                            set_output2(var184, 0.8321390006871503, 0.1678609993128497);
                        } else {
                            set_output2(var184, 0.963855421686747, 0.03614457831325301);
                        }
                    } else {
                        if (input[6] <= 2.246443008182907) {
                            set_output2(var184, 0.8421052631578947, 0.15789473684210525);
                        } else {
                            set_output2(var184, 0.9440993788819876, 0.055900621118012424);
                        }
                    }
                } else {
                    set_output2(var184, 0.0, 1.0);
                }
            } else {
                set_output2(var184, 0.0, 1.0);
            }
        } else {
            set_output2(var184, 0.0, 1.0);
        }
    }
    add_vectors(var137, var184, 2, var136);
    double var185[2];
    if (input[3] <= -38.37897327615046) {
        if (input[9] <= 2.7707529859341062) {
            if (input[9] <= 1.0752668826571243) {
                if (input[7] <= 12.516676783028911) {
                    set_output2(var185, 0.5333333333333333, 0.4666666666666667);
                } else {
                    if (input[9] <= 0.6010400093614009) {
                        if (input[9] <= 0.35862635327082254) {
                            set_output2(var185, 0.9145621309209699, 0.08543786907903002);
                        } else {
                            set_output2(var185, 0.5714285714285714, 0.42857142857142855);
                        }
                    } else {
                        set_output2(var185, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var185, 0.0, 1.0);
            }
        } else {
            set_output2(var185, 0.0, 1.0);
        }
    } else {
        set_output2(var185, 0.0, 1.0);
    }
    add_vectors(var136, var185, 2, var135);
    double var186[2];
    if (input[5] <= 0.034982704797208455) {
        set_output2(var186, 0.5, 0.5);
    } else {
        if (input[4] <= 23.274290107084948) {
            if (input[5] <= 0.3409503383248909) {
                if (input[9] <= 4.521959516087417) {
                    if (input[2] <= -21.65539226220423) {
                        set_output2(var186, 1.0, 0.0);
                    } else {
                        if (input[7] <= 15.26192479318502) {
                            set_output2(var186, 0.8865015684852688, 0.11349843151473116);
                        } else {
                            set_output2(var186, 0.9626556016597511, 0.03734439834024896);
                        }
                    }
                } else {
                    set_output2(var186, 0.0, 1.0);
                }
            } else {
                set_output2(var186, 0.7272727272727273, 0.2727272727272727);
            }
        } else {
            set_output2(var186, 0.0, 1.0);
        }
    }
    add_vectors(var135, var186, 2, var134);
    double var187[2];
    if (input[3] <= -41.595472230950534) {
        if (input[2] <= -7.021612759963485) {
            if (input[2] <= -17.023704277868738) {
                set_output2(var187, 1.0, 0.0);
            } else {
                if (input[3] <= -45.3502187667598) {
                    if (input[8] <= 0.45387799389723316) {
                        set_output2(var187, 0.5714285714285714, 0.42857142857142855);
                    } else {
                        if (input[1] <= 7.256480354105051) {
                            set_output2(var187, 0.923943661971831, 0.07605633802816901);
                        } else {
                            set_output2(var187, 0.832, 0.168);
                        }
                    }
                } else {
                    if (input[8] <= 0.7184425886736934) {
                        set_output2(var187, 1.0, 0.0);
                    } else {
                        set_output2(var187, 0.9302325581395349, 0.06976744186046512);
                    }
                }
            }
        } else {
            if (input[2] <= -0.27094250644136686) {
                if (input[2] <= -1.948419749581416) {
                    if (input[9] <= 2.7490606521783545) {
                        if (input[9] <= 1.9629912445969542) {
                            set_output2(var187, 0.8923076923076924, 0.1076923076923077);
                        } else {
                            set_output2(var187, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var187, 0.0, 1.0);
                    }
                } else {
                    set_output2(var187, 0.0, 1.0);
                }
            } else {
                set_output2(var187, 0.0, 1.0);
            }
        }
    } else {
        if (input[9] <= 3.3082155871877577) {
            if (input[3] <= -38.3420524524731) {
                if (input[2] <= -6.526098926842476) {
                    set_output2(var187, 0.9302325581395349, 0.06976744186046512);
                } else {
                    set_output2(var187, 0.810126582278481, 0.189873417721519);
                }
            } else {
                set_output2(var187, 0.0, 1.0);
            }
        } else {
            set_output2(var187, 0.0, 1.0);
        }
    }
    add_vectors(var134, var187, 2, var133);
    double var188[2];
    if (input[9] <= 6.523448636811436) {
        if (input[9] <= 0.8837460887225737) {
            if (input[9] <= 0.6742904842787637) {
                if (input[1] <= 9.586923739652233) {
                    if (input[8] <= 0.5279272244796656) {
                        set_output2(var188, 0.9649396182313985, 0.03506038176860148);
                    } else {
                        if (input[4] <= 0.6924872796048642) {
                            set_output2(var188, 0.7692307692307693, 0.23076923076923078);
                        } else {
                            set_output2(var188, 0.9403825453341061, 0.05961745466589385);
                        }
                    }
                } else {
                    if (input[9] <= 0.3982073245185603) {
                        if (input[7] <= 12.433821554500918) {
                            set_output2(var188, 0.5161290322580645, 0.4838709677419355);
                        } else {
                            set_output2(var188, 0.8961303462321792, 0.10386965376782077);
                        }
                    } else {
                        set_output2(var188, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var188, 0.0, 1.0);
            }
        } else {
            set_output2(var188, 0.0, 1.0);
        }
    } else {
        set_output2(var188, 0.0, 1.0);
    }
    add_vectors(var133, var188, 2, var132);
    double var189[2];
    if (input[9] <= 5.3265303396462125) {
        if (input[9] <= 4.2886337203131975) {
            if (input[9] <= 0.7094223336555818) {
                if (input[7] <= 11.592659070837598) {
                    set_output2(var189, 0.0, 1.0);
                } else {
                    if (input[3] <= -38.525335042108615) {
                        if (input[9] <= 0.3454304713664487) {
                            set_output2(var189, 0.9097539868957844, 0.0902460131042156);
                        } else {
                            set_output2(var189, 0.47058823529411764, 0.5294117647058824);
                        }
                    } else {
                        set_output2(var189, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var189, 0.0, 1.0);
            }
        } else {
            set_output2(var189, 0.0, 1.0);
        }
    } else {
        set_output2(var189, 0.0, 1.0);
    }
    add_vectors(var132, var189, 2, var131);
    double var190[2];
    if (input[9] <= 7.845622527129187) {
        if (input[9] <= 2.807782722634961) {
            if (input[9] <= 1.1968067479300937) {
                if (input[0] <= 35.49125330119872) {
                    if (input[9] <= 0.20957972221240037) {
                        if (input[1] <= 11.850289171997378) {
                            set_output2(var190, 0.9063302613687868, 0.09366973863121317);
                        } else {
                            set_output2(var190, 0.8421052631578947, 0.15789473684210525);
                        }
                    } else {
                        if (input[9] <= 0.29373777363187226) {
                            set_output2(var190, 0.9090909090909091, 0.09090909090909091);
                        } else {
                            set_output2(var190, 0.47058823529411764, 0.5294117647058824);
                        }
                    }
                } else {
                    if (input[4] <= 2.854882732273919) {
                        set_output2(var190, 0.9142857142857143, 0.08571428571428572);
                    } else {
                        set_output2(var190, 1.0, 0.0);
                    }
                }
            } else {
                set_output2(var190, 0.0, 1.0);
            }
        } else {
            set_output2(var190, 0.0, 1.0);
        }
    } else {
        set_output2(var190, 0.0, 1.0);
    }
    add_vectors(var131, var190, 2, var130);
    double var191[2];
    if (input[3] <= -40.83672246143704) {
        if (input[2] <= -2.8395378370000373) {
            if (input[9] <= 3.3931672275958586) {
                if (input[1] <= 6.685641928789784) {
                    if (input[9] <= 1.5853913084722735) {
                        if (input[7] <= 13.525489024158928) {
                            set_output2(var191, 0.8140495867768596, 0.1859504132231405);
                        } else {
                            set_output2(var191, 0.9585138748040933, 0.0414861251959067);
                        }
                    } else {
                        set_output2(var191, 0.0, 1.0);
                    }
                } else {
                    if (input[9] <= 0.8303108452850853) {
                        if (input[1] <= 8.32927220626366) {
                            set_output2(var191, 0.8421052631578947, 0.15789473684210525);
                        } else {
                            set_output2(var191, 0.9023569023569024, 0.09764309764309764);
                        }
                    } else {
                        set_output2(var191, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var191, 0.0, 1.0);
            }
        } else {
            set_output2(var191, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -36.25781384483997) {
            if (input[4] <= 15.33449206144838) {
                if (input[3] <= -38.164699981929516) {
                    set_output2(var191, 0.8275862068965517, 0.1724137931034483);
                } else {
                    set_output2(var191, 0.0, 1.0);
                }
            } else {
                set_output2(var191, 0.0, 1.0);
            }
        } else {
            set_output2(var191, 0.0, 1.0);
        }
    }
    add_vectors(var130, var191, 2, var129);
    double var192[2];
    if (input[9] <= 4.2579308196659955) {
        if (input[4] <= 16.267673168647146) {
            if (input[9] <= 1.5616506058977306) {
                if (input[1] <= 10.275188866255556) {
                    if (input[9] <= 0.8884978999843832) {
                        if (input[4] <= 8.00810391825475) {
                            set_output2(var192, 0.9278164475380253, 0.07218355246197473);
                        } else {
                            set_output2(var192, 0.8, 0.2);
                        }
                    } else {
                        set_output2(var192, 0.0, 1.0);
                    }
                } else {
                    if (input[7] <= 13.40493851090259) {
                        if (input[3] <= -39.967638924746225) {
                            set_output2(var192, 0.7567567567567568, 0.24324324324324326);
                        } else {
                            set_output2(var192, 0.0, 1.0);
                        }
                    } else {
                        if (input[2] <= -5.869985579131034) {
                            set_output2(var192, 0.7692307692307693, 0.23076923076923078);
                        } else {
                            set_output2(var192, 0.8943661971830986, 0.1056338028169014);
                        }
                    }
                }
            } else {
                set_output2(var192, 0.0, 1.0);
            }
        } else {
            set_output2(var192, 0.0, 1.0);
        }
    } else {
        set_output2(var192, 0.0, 1.0);
    }
    add_vectors(var129, var192, 2, var128);
    double var193[2];
    if (input[2] <= -0.02252220974508745) {
        if (input[9] <= 0.9450104535222685) {
            if (input[9] <= 0.6433346554868244) {
                if (input[9] <= 0.5140785767852996) {
                    if (input[7] <= 13.039985820472493) {
                        set_output2(var193, 0.6956521739130435, 0.30434782608695654);
                    } else {
                        if (input[1] <= 7.0541834412687745) {
                            set_output2(var193, 0.9631177772313745, 0.03688222276862552);
                        } else {
                            set_output2(var193, 0.8996960486322189, 0.10030395136778116);
                        }
                    }
                } else {
                    set_output2(var193, 0.0, 1.0);
                }
            } else {
                set_output2(var193, 0.0, 1.0);
            }
        } else {
            set_output2(var193, 0.0, 1.0);
        }
    } else {
        set_output2(var193, 0.0, 1.0);
    }
    add_vectors(var128, var193, 2, var127);
    double var194[2];
    if (input[9] <= 5.8639248381397975) {
        if (input[9] <= 3.1570196325468034) {
            if (input[9] <= 1.294337833934338) {
                if (input[7] <= 11.45065582161844) {
                    set_output2(var194, 0.0, 1.0);
                } else {
                    if (input[0] <= 27.20192706137003) {
                        set_output2(var194, 0.7804878048780488, 0.21951219512195122);
                    } else {
                        if (input[9] <= 0.8595843635408238) {
                            set_output2(var194, 0.9043359423530873, 0.09566405764691266);
                        } else {
                            set_output2(var194, 0.0, 1.0);
                        }
                    }
                }
            } else {
                set_output2(var194, 0.0, 1.0);
            }
        } else {
            set_output2(var194, 0.0, 1.0);
        }
    } else {
        set_output2(var194, 0.0, 1.0);
    }
    add_vectors(var127, var194, 2, var126);
    double var195[2];
    if (input[3] <= -40.35166203514757) {
        if (input[9] <= 8.22875505340214) {
            if (input[7] <= 14.698425470211285) {
                if (input[9] <= 4.143021170946107) {
                    if (input[9] <= 2.304892726458915) {
                        if (input[7] <= 14.29682799325882) {
                            set_output2(var195, 0.8593803589802218, 0.14061964101977822);
                        } else {
                            set_output2(var195, 0.9174311926605505, 0.08256880733944955);
                        }
                    } else {
                        set_output2(var195, 0.0, 1.0);
                    }
                } else {
                    set_output2(var195, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 2.0378281434746603) {
                    if (input[1] <= 7.544444372686941) {
                        if (input[5] <= 0.2536257108303976) {
                            set_output2(var195, 0.9719626168224299, 0.028037383177570093);
                        } else {
                            set_output2(var195, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        if (input[5] <= 0.39254557366108306) {
                            set_output2(var195, 0.9110764430577223, 0.08892355694227769);
                        } else {
                            set_output2(var195, 0.7272727272727273, 0.2727272727272727);
                        }
                    }
                } else {
                    set_output2(var195, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var195, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -6.572571863754044) {
            set_output2(var195, 1.0, 0.0);
        } else {
            if (input[7] <= 14.064276933319123) {
                set_output2(var195, 0.0, 1.0);
            } else {
                set_output2(var195, 0.64, 0.36);
            }
        }
    }
    add_vectors(var126, var195, 2, var125);
    double var196[2];
    if (input[5] <= 0.18152522262722304) {
        if (input[2] <= -2.5881453943058084) {
            if (input[9] <= 1.5951593030119944) {
                if (input[9] <= 0.4532591814980104) {
                    if (input[7] <= 13.150659804586125) {
                        set_output2(var196, 0.7567567567567568, 0.24324324324324326);
                    } else {
                        if (input[0] <= 35.50410161626475) {
                            set_output2(var196, 0.9099578835261655, 0.09004211647383453);
                        } else {
                            set_output2(var196, 0.9770992366412213, 0.022900763358778626);
                        }
                    }
                } else {
                    set_output2(var196, 0.0, 1.0);
                }
            } else {
                set_output2(var196, 0.0, 1.0);
            }
        } else {
            set_output2(var196, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -40.1104898263559) {
            if (input[9] <= 2.021164696955663) {
                if (input[3] <= -49.401445954981995) {
                    set_output2(var196, 0.7272727272727273, 0.2727272727272727);
                } else {
                    set_output2(var196, 0.9696969696969697, 0.030303030303030304);
                }
            } else {
                set_output2(var196, 0.0, 1.0);
            }
        } else {
            set_output2(var196, 0.0, 1.0);
        }
    }
    add_vectors(var125, var196, 2, var124);
    double var197[2];
    if (input[1] <= 12.41423002562711) {
        if (input[9] <= 12.518564775208024) {
            if (input[9] <= 6.732263830255998) {
                if (input[7] <= 14.323973206975886) {
                    if (input[9] <= 4.721306618731237) {
                        if (input[9] <= 0.6670971210899065) {
                            set_output2(var197, 0.8763155039600737, 0.1236844960399262);
                        } else {
                            set_output2(var197, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var197, 0.0, 1.0);
                    }
                } else {
                    if (input[9] <= 1.265056578040454) {
                        if (input[9] <= 0.5735383490545057) {
                            set_output2(var197, 0.9210526315789473, 0.07894736842105263);
                        } else {
                            set_output2(var197, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var197, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var197, 0.0, 1.0);
            }
        } else {
            set_output2(var197, 0.0, 1.0);
        }
    } else {
        set_output2(var197, 0.3076923076923077, 0.6923076923076923);
    }
    add_vectors(var124, var197, 2, var123);
    double var198[2];
    if (input[9] <= 0.8100984526301842) {
        if (input[2] <= -9.57532771613441) {
            if (input[6] <= 63.93026313051672) {
                if (input[0] <= 37.48476818991168) {
                    if (input[9] <= 0.017317392204789964) {
                        set_output2(var198, 0.8608964451313755, 0.1391035548686244);
                    } else {
                        if (input[5] <= 0.13618597018921347) {
                            set_output2(var198, 0.9726443768996961, 0.02735562310030395);
                        } else {
                            set_output2(var198, 0.8608964451313755, 0.1391035548686244);
                        }
                    }
                } else {
                    set_output2(var198, 0.7272727272727273, 0.2727272727272727);
                }
            } else {
                set_output2(var198, 0.7272727272727273, 0.2727272727272727);
            }
        } else {
            if (input[9] <= 0.4375038537551843) {
                if (input[7] <= 13.973192242083734) {
                    if (input[7] <= 13.822225079525627) {
                        if (input[2] <= -4.170140440974573) {
                            set_output2(var198, 0.775330396475771, 0.22466960352422907);
                        } else {
                            set_output2(var198, 0.9230769230769231, 0.07692307692307693);
                        }
                    } else {
                        set_output2(var198, 0.9142857142857143, 0.08571428571428572);
                    }
                } else {
                    if (input[2] <= -7.242330049824666) {
                        if (input[3] <= -46.521840566591166) {
                            set_output2(var198, 0.8351648351648352, 0.16483516483516483);
                        } else {
                            set_output2(var198, 0.96, 0.04);
                        }
                    } else {
                        if (input[6] <= 11.55402189810049) {
                            set_output2(var198, 0.9154228855721394, 0.0845771144278607);
                        } else {
                            set_output2(var198, 0.9620253164556962, 0.0379746835443038);
                        }
                    }
                }
            } else {
                set_output2(var198, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var198, 0.0, 1.0);
    }
    add_vectors(var123, var198, 2, var122);
    double var199[2];
    if (input[9] <= 7.433372987989418) {
        if (input[9] <= 4.944566227631501) {
            if (input[4] <= 20.98771540619932) {
                if (input[9] <= 2.7515230671949773) {
                    if (input[9] <= 0.9126638343134786) {
                        if (input[8] <= 0.8669536156372653) {
                            set_output2(var199, 0.8984184663666019, 0.10158153363339802);
                        } else {
                            set_output2(var199, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var199, 0.0, 1.0);
                    }
                } else {
                    set_output2(var199, 0.0, 1.0);
                }
            } else {
                set_output2(var199, 0.0, 1.0);
            }
        } else {
            set_output2(var199, 0.0, 1.0);
        }
    } else {
        set_output2(var199, 0.0, 1.0);
    }
    add_vectors(var122, var199, 2, var121);
    double var200[2];
    if (input[9] <= 4.978307856442491) {
        if (input[8] <= 1.153880363499924) {
            if (input[9] <= 1.0870848366830563) {
                if (input[9] <= 0.2707619925404706) {
                    if (input[2] <= -8.473153832891366) {
                        if (input[5] <= 0.2113951740551601) {
                            set_output2(var200, 0.9429332318813011, 0.05706676811869888);
                        } else {
                            set_output2(var200, 0.8, 0.2);
                        }
                    } else {
                        if (input[7] <= 11.850585179020626) {
                            set_output2(var200, 0.0, 1.0);
                        } else {
                            set_output2(var200, 0.8996160175534833, 0.10038398244651674);
                        }
                    }
                } else {
                    if (input[9] <= 0.3584755831664426) {
                        set_output2(var200, 0.8888888888888888, 0.1111111111111111);
                    } else {
                        set_output2(var200, 0.27586206896551724, 0.7241379310344828);
                    }
                }
            } else {
                set_output2(var200, 0.0, 1.0);
            }
        } else {
            set_output2(var200, 0.0, 1.0);
        }
    } else {
        set_output2(var200, 0.0, 1.0);
    }
    add_vectors(var121, var200, 2, var120);
    double var201[2];
    if (input[9] <= 0.9318311114238753) {
        if (input[9] <= 0.30525455034294163) {
            if (input[7] <= 13.057525402091409) {
                set_output2(var201, 0.6956521739130435, 0.30434782608695654);
            } else {
                if (input[1] <= 6.645307970241829) {
                    if (input[6] <= 71.53120368447475) {
                        if (input[3] <= -54.006702297655224) {
                            set_output2(var201, 0.7272727272727273, 0.2727272727272727);
                        } else {
                            set_output2(var201, 0.9732699732699732, 0.02673002673002673);
                        }
                    } else {
                        set_output2(var201, 0.7272727272727273, 0.2727272727272727);
                    }
                } else {
                    if (input[0] <= 33.61757789334544) {
                        if (input[5] <= 0.05544299818801642) {
                            set_output2(var201, 0.7032967032967034, 0.2967032967032967);
                        } else {
                            set_output2(var201, 0.9042145593869731, 0.09578544061302682);
                        }
                    } else {
                        if (input[5] <= 0.16299324985106065) {
                            set_output2(var201, 0.9719626168224299, 0.028037383177570093);
                        } else {
                            set_output2(var201, 0.5714285714285714, 0.42857142857142855);
                        }
                    }
                }
            }
        } else {
            set_output2(var201, 0.5714285714285714, 0.42857142857142855);
        }
    } else {
        set_output2(var201, 0.0, 1.0);
    }
    add_vectors(var120, var201, 2, var119);
    double var202[2];
    if (input[9] <= 6.756458954765481) {
        if (input[9] <= 4.7141342247193885) {
            if (input[9] <= 2.212520253808262) {
                if (input[9] <= 0.9009801449483431) {
                    if (input[7] <= 12.667875292925386) {
                        set_output2(var202, 0.6037735849056604, 0.39622641509433965);
                    } else {
                        if (input[1] <= 9.509595828884027) {
                            set_output2(var202, 0.9333111037012337, 0.06668889629876626);
                        } else {
                            set_output2(var202, 0.8906560636182903, 0.10934393638170974);
                        }
                    }
                } else {
                    set_output2(var202, 0.0, 1.0);
                }
            } else {
                set_output2(var202, 0.0, 1.0);
            }
        } else {
            set_output2(var202, 0.0, 1.0);
        }
    } else {
        set_output2(var202, 0.0, 1.0);
    }
    add_vectors(var119, var202, 2, var118);
    double var203[2];
    if (input[9] <= 11.184411094769448) {
        if (input[2] <= -1.9096001901090496) {
            if (input[8] <= 1.5016935198254024) {
                if (input[9] <= 1.2909449002471571) {
                    if (input[2] <= -6.063020859441345) {
                        if (input[9] <= 0.4469687516995182) {
                            set_output2(var203, 0.9211027478008524, 0.07889725219914755);
                        } else {
                            set_output2(var203, 0.0, 1.0);
                        }
                    } else {
                        if (input[9] <= 0.812617563579537) {
                            set_output2(var203, 0.8888888888888888, 0.1111111111111111);
                        } else {
                            set_output2(var203, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var203, 0.0, 1.0);
                }
            } else {
                set_output2(var203, 0.0, 1.0);
            }
        } else {
            set_output2(var203, 0.0, 1.0);
        }
    } else {
        set_output2(var203, 0.0, 1.0);
    }
    add_vectors(var118, var203, 2, var117);
    double var204[2];
    if (input[7] <= 15.275993462307467) {
        if (input[9] <= 2.788146844101358) {
            if (input[9] <= 1.1846614517670238) {
                if (input[7] <= 13.930068642244303) {
                    if (input[7] <= 11.263739298624323) {
                        set_output2(var204, 0.0, 1.0);
                    } else {
                        if (input[7] <= 12.608357799695778) {
                            set_output2(var204, 0.6808510638297872, 0.3191489361702128);
                        } else {
                            set_output2(var204, 0.8544997305550566, 0.1455002694449434);
                        }
                    }
                } else {
                    if (input[9] <= 0.5743397779420818) {
                        if (input[1] <= 9.991896886028686) {
                            set_output2(var204, 0.9345110563124065, 0.06548894368759349);
                        } else {
                            set_output2(var204, 0.896969696969697, 0.10303030303030303);
                        }
                    } else {
                        set_output2(var204, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var204, 0.0, 1.0);
            }
        } else {
            set_output2(var204, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -51.63170520485062) {
            set_output2(var204, 1.0, 0.0);
        } else {
            if (input[5] <= 0.06688271022771627) {
                set_output2(var204, 1.0, 0.0);
            } else {
                set_output2(var204, 0.8695652173913043, 0.13043478260869565);
            }
        }
    }
    add_vectors(var117, var204, 2, var116);
    double var205[2];
    if (input[9] <= 0.22924050815449928) {
        if (input[1] <= 10.841067314599151) {
            if (input[4] <= 1.5777854143792285) {
                if (input[9] <= 0.17352128251069107) {
                    if (input[7] <= 13.622017021587157) {
                        set_output2(var205, 0.46824224519940916, 0.5317577548005908);
                    } else {
                        if (input[7] <= 13.936619212876765) {
                            set_output2(var205, 0.47058823529411764, 0.5294117647058824);
                        } else {
                            set_output2(var205, 0.920323763753636, 0.07967623624636398);
                        }
                    }
                } else {
                    set_output2(var205, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.945659650058666) {
                    if (input[1] <= 10.381420693698974) {
                        if (input[7] <= 14.270399534454222) {
                            set_output2(var205, 0.9682614317620782, 0.03173856823792171);
                        } else {
                            set_output2(var205, 0.9283276450511946, 0.07167235494880546);
                        }
                    } else {
                        if (input[8] <= 0.6412701791048274) {
                            set_output2(var205, 1.0, 0.0);
                        } else {
                            set_output2(var205, 0.8421052631578947, 0.15789473684210525);
                        }
                    }
                } else {
                    set_output2(var205, 0.7272727272727273, 0.2727272727272727);
                }
            }
        } else {
            if (input[7] <= 11.962134984306486) {
                set_output2(var205, 0.0, 1.0);
            } else {
                if (input[5] <= 0.30609176172571817) {
                    if (input[1] <= 11.872500292117689) {
                        if (input[9] <= 0.1329387785534994) {
                            set_output2(var205, 0.9244444444444444, 0.07555555555555556);
                        } else {
                            set_output2(var205, 0.85, 0.15);
                        }
                    } else {
                        if (input[0] <= 32.8050571048873) {
                            set_output2(var205, 0.8073394495412844, 0.1926605504587156);
                        } else {
                            set_output2(var205, 1.0, 0.0);
                        }
                    }
                } else {
                    set_output2(var205, 0.5714285714285714, 0.42857142857142855);
                }
            }
        }
    } else {
        if (input[2] <= -3.697688244560835) {
            if (input[6] <= 4.887612620898885) {
                set_output2(var205, 0.7619047619047619, 0.23809523809523808);
            } else {
                if (input[0] <= 33.536196355491434) {
                    if (input[9] <= 0.4641958102358661) {
                        set_output2(var205, 0.8695652173913043, 0.13043478260869565);
                    } else {
                        set_output2(var205, 0.0, 1.0);
                    }
                } else {
                    set_output2(var205, 0.8, 0.2);
                }
            }
        } else {
            set_output2(var205, 0.0, 1.0);
        }
    }
    add_vectors(var116, var205, 2, var115);
    double var206[2];
    if (input[2] <= -1.5023218631499056) {
        if (input[8] <= 1.6464580190791678) {
            if (input[9] <= 0.46907271380836946) {
                if (input[7] <= 13.963234225100273) {
                    if (input[7] <= 11.879184320394385) {
                        set_output2(var206, 0.0, 1.0);
                    } else {
                        if (input[7] <= 13.677436811964139) {
                            set_output2(var206, 0.801002358490566, 0.19899764150943397);
                        } else {
                            set_output2(var206, 0.9014084507042254, 0.09859154929577464);
                        }
                    }
                } else {
                    if (input[5] <= 0.0529220556754836) {
                        if (input[4] <= 3.911329518988435) {
                            set_output2(var206, 0.8888888888888888, 0.1111111111111111);
                        } else {
                            set_output2(var206, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        if (input[5] <= 0.3455503541423787) {
                            set_output2(var206, 0.9291300535103889, 0.07086994648961112);
                        } else {
                            set_output2(var206, 0.7804878048780488, 0.21951219512195122);
                        }
                    }
                }
            } else {
                set_output2(var206, 0.0, 1.0);
            }
        } else {
            set_output2(var206, 0.0, 1.0);
        }
    } else {
        set_output2(var206, 0.0, 1.0);
    }
    add_vectors(var115, var206, 2, var114);
    double var207[2];
    if (input[9] <= 7.757181912019427) {
        if (input[9] <= 1.7709567382073739) {
            if (input[7] <= 13.909370641010021) {
                if (input[9] <= 0.7038674132634503) {
                    if (input[0] <= 27.796817319388104) {
                        set_output2(var207, 0.47058823529411764, 0.5294117647058824);
                    } else {
                        if (input[4] <= 3.8822620426081733) {
                            set_output2(var207, 0.8161858565228491, 0.1838141434771509);
                        } else {
                            set_output2(var207, 0.9343065693430657, 0.06569343065693431);
                        }
                    }
                } else {
                    set_output2(var207, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 0.5777598303058558) {
                    if (input[1] <= 10.17676680842065) {
                        if (input[4] <= 5.307733092449237) {
                            set_output2(var207, 0.92172892116363, 0.07827107883636997);
                        } else {
                            set_output2(var207, 0.9739130434782609, 0.02608695652173913);
                        }
                    } else {
                        if (input[7] <= 14.33002710773567) {
                            set_output2(var207, 0.8602150537634409, 0.13978494623655913);
                        } else {
                            set_output2(var207, 0.9133858267716536, 0.08661417322834646);
                        }
                    }
                } else {
                    set_output2(var207, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var207, 0.0, 1.0);
        }
    } else {
        set_output2(var207, 0.0, 1.0);
    }
    add_vectors(var114, var207, 2, var113);
    double var208[2];
    if (input[9] <= 5.878957847723306) {
        if (input[9] <= 0.335332547175381) {
            if (input[9] <= 0.16726685228814178) {
                if (input[6] <= 43.063778359115) {
                    if (input[3] <= -38.54392392522453) {
                        if (input[7] <= 14.546315538943418) {
                            set_output2(var208, 0.8864557988645579, 0.11354420113544202);
                        } else {
                            set_output2(var208, 0.9198606271777003, 0.08013937282229965);
                        }
                    } else {
                        set_output2(var208, 0.0, 1.0);
                    }
                } else {
                    set_output2(var208, 0.7272727272727273, 0.2727272727272727);
                }
            } else {
                if (input[6] <= 1.7107481645584162) {
                    set_output2(var208, 0.8, 0.2);
                } else {
                    if (input[9] <= 0.26551390083461546) {
                        if (input[6] <= 26.43013146695905) {
                            set_output2(var208, 0.9655172413793104, 0.034482758620689655);
                        } else {
                            set_output2(var208, 0.8421052631578947, 0.15789473684210525);
                        }
                    } else {
                        set_output2(var208, 0.8695652173913043, 0.13043478260869565);
                    }
                }
            }
        } else {
            if (input[0] <= 34.73137603432049) {
                if (input[7] <= 14.165198229977984) {
                    set_output2(var208, 0.1951219512195122, 0.8048780487804879);
                } else {
                    set_output2(var208, 0.0, 1.0);
                }
            } else {
                set_output2(var208, 0.5714285714285714, 0.42857142857142855);
            }
        }
    } else {
        set_output2(var208, 0.0, 1.0);
    }
    add_vectors(var113, var208, 2, var112);
    double var209[2];
    if (input[3] <= -36.2818822396635) {
        if (input[7] <= 11.536629873168788) {
            set_output2(var209, 0.0, 1.0);
        } else {
            if (input[3] <= -38.78388687676352) {
                if (input[9] <= 0.833145886758264) {
                    if (input[9] <= 0.24829835375970025) {
                        if (input[1] <= 5.42278891780245) {
                            set_output2(var209, 0.9795918367346939, 0.02040816326530612);
                        } else {
                            set_output2(var209, 0.9046191579234228, 0.0953808420765772);
                        }
                    } else {
                        set_output2(var209, 0.7567567567567568, 0.24324324324324326);
                    }
                } else {
                    set_output2(var209, 0.0, 1.0);
                }
            } else {
                set_output2(var209, 0.6666666666666666, 0.3333333333333333);
            }
        }
    } else {
        set_output2(var209, 0.0, 1.0);
    }
    add_vectors(var112, var209, 2, var111);
    double var210[2];
    if (input[9] <= 3.502359516699868) {
        if (input[9] <= 2.6748281605840227) {
            if (input[3] <= -35.11971500902837) {
                if (input[7] <= 13.660739476333436) {
                    if (input[4] <= 3.562091410278152) {
                        if (input[4] <= 2.0007008227951326) {
                            set_output2(var210, 0.4698685999093793, 0.5301314000906208);
                        } else {
                            set_output2(var210, 0.8082045817794352, 0.19179541822056476);
                        }
                    } else {
                        set_output2(var210, 0.88, 0.12);
                    }
                } else {
                    if (input[4] <= 7.1212790931025225) {
                        if (input[3] <= -53.99636474363765) {
                            set_output2(var210, 0.8163265306122449, 0.1836734693877551);
                        } else {
                            set_output2(var210, 0.917201901289674, 0.08279809871032591);
                        }
                    } else {
                        if (input[3] <= -38.18411470958178) {
                            set_output2(var210, 0.875, 0.125);
                        } else {
                            set_output2(var210, 0.0, 1.0);
                        }
                    }
                }
            } else {
                set_output2(var210, 0.0, 1.0);
            }
        } else {
            set_output2(var210, 0.0, 1.0);
        }
    } else {
        set_output2(var210, 0.0, 1.0);
    }
    add_vectors(var111, var210, 2, var110);
    double var211[2];
    if (input[9] <= 2.4427855725433694) {
        if (input[9] <= 2.1954610161191157) {
            if (input[9] <= 0.5546858627503859) {
                if (input[9] <= 0.4794074755511469) {
                    if (input[1] <= 5.432226752528846) {
                        set_output2(var211, 0.9806451612903225, 0.01935483870967742);
                    } else {
                        if (input[7] <= 15.420786047652058) {
                            set_output2(var211, 0.8971126662650044, 0.1028873337349956);
                        } else {
                            set_output2(var211, 0.9552238805970149, 0.04477611940298507);
                        }
                    }
                } else {
                    set_output2(var211, 0.0, 1.0);
                }
            } else {
                set_output2(var211, 0.0, 1.0);
            }
        } else {
            set_output2(var211, 0.0, 1.0);
        }
    } else {
        set_output2(var211, 0.0, 1.0);
    }
    add_vectors(var110, var211, 2, var109);
    double var212[2];
    if (input[9] <= 7.2181906985226805) {
        if (input[9] <= 3.220498262349116) {
            if (input[9] <= 2.001892533314559) {
                if (input[9] <= 0.6684889296179738) {
                    if (input[7] <= 11.50211739098345) {
                        set_output2(var212, 0.0, 1.0);
                    } else {
                        if (input[1] <= 7.414919193579163) {
                            set_output2(var212, 0.9454173299977258, 0.054582670002274285);
                        } else {
                            set_output2(var212, 0.8955223880597015, 0.1044776119402985);
                        }
                    }
                } else {
                    set_output2(var212, 0.0, 1.0);
                }
            } else {
                set_output2(var212, 0.0, 1.0);
            }
        } else {
            set_output2(var212, 0.0, 1.0);
        }
    } else {
        set_output2(var212, 0.0, 1.0);
    }
    add_vectors(var109, var212, 2, var108);
    double var213[2];
    if (input[9] <= 9.4573976139205) {
        if (input[9] <= 2.6159796497090495) {
            if (input[9] <= 0.850999685979172) {
                if (input[7] <= 12.113292880766874) {
                    set_output2(var213, 0.0, 1.0);
                } else {
                    if (input[4] <= 5.663785156857906) {
                        if (input[8] <= 0.5516092591637336) {
                            set_output2(var213, 0.9248837486586384, 0.07511625134136163);
                        } else {
                            set_output2(var213, 0.8902965390456399, 0.10970346095436018);
                        }
                    } else {
                        if (input[1] <= 9.827082840328277) {
                            set_output2(var213, 1.0, 0.0);
                        } else {
                            set_output2(var213, 0.9120521172638436, 0.08794788273615635);
                        }
                    }
                }
            } else {
                set_output2(var213, 0.0, 1.0);
            }
        } else {
            set_output2(var213, 0.0, 1.0);
        }
    } else {
        set_output2(var213, 0.0, 1.0);
    }
    add_vectors(var108, var213, 2, var107);
    double var214[2];
    if (input[9] <= 1.2959338699871388) {
        if (input[9] <= 0.3087530042565584) {
            if (input[3] <= -41.0300380631587) {
                if (input[2] <= -16.10203762489228) {
                    set_output2(var214, 1.0, 0.0);
                } else {
                    if (input[7] <= 14.018415147275556) {
                        if (input[4] <= 8.80791140952358) {
                            set_output2(var214, 0.8452088452088452, 0.1547911547911548);
                        } else {
                            set_output2(var214, 1.0, 0.0);
                        }
                    } else {
                        if (input[3] <= -45.687729088065176) {
                            set_output2(var214, 0.9111257406188282, 0.08887425938117183);
                        } else {
                            set_output2(var214, 0.9620253164556962, 0.0379746835443038);
                        }
                    }
                }
            } else {
                set_output2(var214, 0.8, 0.2);
            }
        } else {
            set_output2(var214, 0.47058823529411764, 0.5294117647058824);
        }
    } else {
        set_output2(var214, 0.0, 1.0);
    }
    add_vectors(var107, var214, 2, var106);
    double var215[2];
    if (input[9] <= 4.339002869837738) {
        if (input[9] <= 2.3235076529198366) {
            if (input[8] <= 1.2824377454136318) {
                if (input[9] <= 0.49604421678268856) {
                    if (input[7] <= 11.255403117168552) {
                        set_output2(var215, 0.0, 1.0);
                    } else {
                        if (input[7] <= 12.596949888012675) {
                            set_output2(var215, 0.6153846153846154, 0.38461538461538464);
                        } else {
                            set_output2(var215, 0.9125619352958321, 0.08743806470416789);
                        }
                    }
                } else {
                    set_output2(var215, 0.0, 1.0);
                }
            } else {
                set_output2(var215, 0.0, 1.0);
            }
        } else {
            set_output2(var215, 0.0, 1.0);
        }
    } else {
        set_output2(var215, 0.0, 1.0);
    }
    add_vectors(var106, var215, 2, var105);
    double var216[2];
    if (input[9] <= 6.541160728948976) {
        if (input[9] <= 3.1606107811055404) {
            if (input[9] <= 2.4949801857020346) {
                if (input[9] <= 1.3224209117287404) {
                    if (input[7] <= 11.89869689741603) {
                        set_output2(var216, 0.0, 1.0);
                    } else {
                        if (input[7] <= 13.835338698722525) {
                            set_output2(var216, 0.8230999625608386, 0.17690003743916136);
                        } else {
                            set_output2(var216, 0.9117884186730323, 0.08821158132696777);
                        }
                    }
                } else {
                    set_output2(var216, 0.0, 1.0);
                }
            } else {
                set_output2(var216, 0.0, 1.0);
            }
        } else {
            set_output2(var216, 0.0, 1.0);
        }
    } else {
        set_output2(var216, 0.0, 1.0);
    }
    add_vectors(var105, var216, 2, var104);
    double var217[2];
    if (input[7] <= 13.412266699692994) {
        if (input[7] <= 12.05446525525368) {
            set_output2(var217, 0.0, 1.0);
        } else {
            if (input[9] <= 0.4068890056023613) {
                if (input[4] <= 6.456191969434243) {
                    if (input[4] <= 3.4906653460695596) {
                        set_output2(var217, 0.6824596774193549, 0.31754032258064513);
                    } else {
                        set_output2(var217, 1.0, 0.0);
                    }
                } else {
                    set_output2(var217, 0.9142857142857143, 0.08571428571428572);
                }
            } else {
                set_output2(var217, 0.5714285714285714, 0.42857142857142855);
            }
        }
    } else {
        if (input[9] <= 1.648584531405756) {
            if (input[1] <= 12.237896524126352) {
                if (input[9] <= 0.8035835897043735) {
                    if (input[7] <= 13.7014997906758) {
                        if (input[4] <= 4.4132704002417515) {
                            set_output2(var217, 0.7, 0.3);
                        } else {
                            set_output2(var217, 1.0, 0.0);
                        }
                    } else {
                        if (input[9] <= 0.4037942824173643) {
                            set_output2(var217, 0.920350336451165, 0.07964966354883501);
                        } else {
                            set_output2(var217, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var217, 0.0, 1.0);
                }
            } else {
                set_output2(var217, 0.5714285714285714, 0.42857142857142855);
            }
        } else {
            set_output2(var217, 0.0, 1.0);
        }
    }
    add_vectors(var104, var217, 2, var103);
    double var218[2];
    if (input[7] <= 11.293904848706035) {
        set_output2(var218, 0.0, 1.0);
    } else {
        if (input[9] <= 2.9527699365885343) {
            if (input[9] <= 1.2365153759804308) {
                if (input[1] <= 9.774853338173529) {
                    if (input[1] <= 6.445265670996997) {
                        if (input[9] <= 0.08788094990936389) {
                            set_output2(var218, 0.9221228728006923, 0.07787712719930776);
                        } else {
                            set_output2(var218, 1.0, 0.0);
                        }
                    } else {
                        if (input[1] <= 8.273756961120355) {
                            set_output2(var218, 0.847926267281106, 0.15207373271889402);
                        } else {
                            set_output2(var218, 0.9482758620689655, 0.05172413793103448);
                        }
                    }
                } else {
                    if (input[7] <= 13.271670413075489) {
                        set_output2(var218, 0.6956521739130435, 0.30434782608695654);
                    } else {
                        if (input[9] <= 0.7766555446012381) {
                            set_output2(var218, 0.8948545861297539, 0.10514541387024609);
                        } else {
                            set_output2(var218, 0.0, 1.0);
                        }
                    }
                }
            } else {
                set_output2(var218, 0.0, 1.0);
            }
        } else {
            set_output2(var218, 0.0, 1.0);
        }
    }
    add_vectors(var103, var218, 2, var102);
    double var219[2];
    if (input[2] <= -9.14272380754359) {
        if (input[9] <= 0.35162909774831447) {
            if (input[3] <= -54.01061861653587) {
                set_output2(var219, 0.8421052631578947, 0.15789473684210525);
            } else {
                if (input[8] <= 0.6024773483134048) {
                    set_output2(var219, 1.0, 0.0);
                } else {
                    if (input[6] <= 17.65769060143915) {
                        if (input[7] <= 15.329287814119947) {
                            set_output2(var219, 0.951331620927403, 0.048668379072596996);
                        } else {
                            set_output2(var219, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        set_output2(var219, 0.7272727272727273, 0.2727272727272727);
                    }
                }
            }
        } else {
            set_output2(var219, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 6.010682572972651) {
            if (input[1] <= 14.000462140014566) {
                if (input[9] <= 4.282878923631845) {
                    if (input[9] <= 0.8346645596847662) {
                        if (input[9] <= 0.44909820704020365) {
                            set_output2(var219, 0.8951898734177215, 0.10481012658227848);
                        } else {
                            set_output2(var219, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var219, 0.0, 1.0);
                    }
                } else {
                    set_output2(var219, 0.0, 1.0);
                }
            } else {
                set_output2(var219, 0.0, 1.0);
            }
        } else {
            set_output2(var219, 0.0, 1.0);
        }
    }
    add_vectors(var102, var219, 2, var101);
    double var220[2];
    if (input[9] <= 4.814907579773158) {
        if (input[9] <= 0.2149985200930508) {
            if (input[1] <= 10.174767968212198) {
                if (input[3] <= -46.14309558806521) {
                    if (input[1] <= 9.522852403200254) {
                        if (input[4] <= 5.148388833325) {
                            set_output2(var220, 0.8968699113443097, 0.10313008865569025);
                        } else {
                            set_output2(var220, 0.9806451612903225, 0.01935483870967742);
                        }
                    } else {
                        if (input[7] <= 15.248318630785649) {
                            set_output2(var220, 0.8163265306122449, 0.1836734693877551);
                        } else {
                            set_output2(var220, 1.0, 0.0);
                        }
                    }
                } else {
                    if (input[6] <= 0.283516245349751) {
                        set_output2(var220, 1.0, 0.0);
                    } else {
                        if (input[5] <= 0.057188806637345276) {
                            set_output2(var220, 0.8421052631578947, 0.15789473684210525);
                        } else {
                            set_output2(var220, 0.9846153846153847, 0.015384615384615385);
                        }
                    }
                }
            } else {
                if (input[7] <= 13.366525988726377) {
                    set_output2(var220, 0.7, 0.3);
                } else {
                    if (input[0] <= 35.319603819548476) {
                        if (input[5] <= 0.29667350080263105) {
                            set_output2(var220, 0.9065155807365439, 0.09348441926345609);
                        } else {
                            set_output2(var220, 0.64, 0.36);
                        }
                    } else {
                        set_output2(var220, 1.0, 0.0);
                    }
                }
            }
        } else {
            if (input[9] <= 2.5991140364582606) {
                if (input[9] <= 1.11858491466768) {
                    if (input[9] <= 0.9870461991119777) {
                        if (input[9] <= 0.8307189875180658) {
                            set_output2(var220, 0.8421052631578947, 0.15789473684210525);
                        } else {
                            set_output2(var220, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var220, 0.0, 1.0);
                    }
                } else {
                    set_output2(var220, 0.0, 1.0);
                }
            } else {
                set_output2(var220, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var220, 0.0, 1.0);
    }
    add_vectors(var101, var220, 2, var100);
    double var221[2];
    if (input[9] <= 2.219144347235467) {
        if (input[9] <= 0.8685550663824496) {
            if (input[6] <= 76.13777196256788) {
                if (input[1] <= 7.361324295066579) {
                    if (input[9] <= 0.01259397820255271) {
                        set_output2(var221, 0.7272727272727273, 0.2727272727272727);
                    } else {
                        if (input[7] <= 13.483000406629722) {
                            set_output2(var221, 0.8756906077348067, 0.12430939226519336);
                        } else {
                            set_output2(var221, 0.9610962220108931, 0.03890377798910694);
                        }
                    }
                } else {
                    if (input[3] <= -39.87092420963019) {
                        if (input[1] <= 11.912361323506243) {
                            set_output2(var221, 0.9003215434083601, 0.09967845659163987);
                        } else {
                            set_output2(var221, 0.7920792079207921, 0.2079207920792079);
                        }
                    } else {
                        set_output2(var221, 0.7619047619047619, 0.23809523809523808);
                    }
                }
            } else {
                set_output2(var221, 0.7272727272727273, 0.2727272727272727);
            }
        } else {
            set_output2(var221, 0.0, 1.0);
        }
    } else {
        set_output2(var221, 0.0, 1.0);
    }
    add_vectors(var100, var221, 2, var99);
    double var222[2];
    if (input[9] <= 6.5147103562414275) {
        if (input[9] <= 1.0362711401034035) {
            if (input[9] <= 0.5586614208755415) {
                if (input[2] <= -16.68068302197388) {
                    set_output2(var222, 1.0, 0.0);
                } else {
                    if (input[7] <= 12.72504340608293) {
                        set_output2(var222, 0.6037735849056604, 0.39622641509433965);
                    } else {
                        if (input[7] <= 14.148756112273727) {
                            set_output2(var222, 0.8547579298831386, 0.14524207011686144);
                        } else {
                            set_output2(var222, 0.9260744985673353, 0.07392550143266476);
                        }
                    }
                }
            } else {
                set_output2(var222, 0.0, 1.0);
            }
        } else {
            set_output2(var222, 0.0, 1.0);
        }
    } else {
        set_output2(var222, 0.0, 1.0);
    }
    add_vectors(var99, var222, 2, var98);
    double var223[2];
    if (input[8] <= 1.8059470840242486) {
        if (input[9] <= 4.446570664856873) {
            if (input[9] <= 1.4645210895254235) {
                if (input[9] <= 0.9727325586442832) {
                    if (input[7] <= 14.050511510473473) {
                        if (input[4] <= 3.8569508418526457) {
                            set_output2(var223, 0.765722615576843, 0.23427738442315701);
                        } else {
                            set_output2(var223, 0.9072164948453608, 0.09278350515463918);
                        }
                    } else {
                        if (input[5] <= 0.4194383987112913) {
                            set_output2(var223, 0.9242879461603173, 0.07571205383968273);
                        } else {
                            set_output2(var223, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var223, 0.0, 1.0);
                }
            } else {
                set_output2(var223, 0.0, 1.0);
            }
        } else {
            set_output2(var223, 0.0, 1.0);
        }
    } else {
        set_output2(var223, 0.0, 1.0);
    }
    add_vectors(var98, var223, 2, var97);
    double var224[2];
    if (input[9] <= 0.29863794553665796) {
        if (input[2] <= -9.700006353467426) {
            if (input[7] <= 14.148037065980695) {
                if (input[4] <= 1.0666475024297752) {
                    set_output2(var224, 0.7557666214382632, 0.24423337856173677);
                } else {
                    set_output2(var224, 0.9106256206554121, 0.08937437934458789);
                }
            } else {
                if (input[5] <= 0.21669981748666148) {
                    if (input[9] <= 0.026009969615486985) {
                        set_output2(var224, 0.9032258064516129, 0.0967741935483871);
                    } else {
                        if (input[7] <= 15.052051727911277) {
                            set_output2(var224, 1.0, 0.0);
                        } else {
                            set_output2(var224, 0.8888888888888888, 0.1111111111111111);
                        }
                    }
                } else {
                    set_output2(var224, 0.8888888888888888, 0.1111111111111111);
                }
            }
        } else {
            if (input[9] <= 0.19742953114245415) {
                if (input[7] <= 14.16134299034056) {
                    if (input[8] <= 0.7438298827780647) {
                        if (input[9] <= 0.05792502298982118) {
                            set_output2(var224, 0.875, 0.125);
                        } else {
                            set_output2(var224, 0.7963800904977375, 0.20361990950226244);
                        }
                    } else {
                        if (input[3] <= -41.330547287781926) {
                            set_output2(var224, 0.9411764705882353, 0.058823529411764705);
                        } else {
                            set_output2(var224, 0.8, 0.2);
                        }
                    }
                } else {
                    if (input[5] <= 0.30005111587092065) {
                        if (input[8] <= 0.5329582790263281) {
                            set_output2(var224, 0.8648648648648649, 0.13513513513513514);
                        } else {
                            set_output2(var224, 0.924187725631769, 0.07581227436823104);
                        }
                    } else {
                        set_output2(var224, 0.64, 0.36);
                    }
                }
            } else {
                if (input[0] <= 26.489402076730734) {
                    set_output2(var224, 0.7272727272727273, 0.2727272727272727);
                } else {
                    if (input[3] <= -45.943270596196385) {
                        if (input[0] <= 30.378457486324674) {
                            set_output2(var224, 1.0, 0.0);
                        } else {
                            set_output2(var224, 0.896551724137931, 0.10344827586206896);
                        }
                    } else {
                        set_output2(var224, 1.0, 0.0);
                    }
                }
            }
        }
    } else {
        if (input[2] <= -1.258821302159296) {
            if (input[9] <= 2.4673606677757496) {
                if (input[9] <= 0.4431393058240282) {
                    set_output2(var224, 0.8888888888888888, 0.1111111111111111);
                } else {
                    set_output2(var224, 0.0, 1.0);
                }
            } else {
                set_output2(var224, 0.0, 1.0);
            }
        } else {
            set_output2(var224, 0.0, 1.0);
        }
    }
    add_vectors(var97, var224, 2, var96);
    double var225[2];
    if (input[9] <= 8.679357203195446) {
        if (input[9] <= 3.4004179905559773) {
            if (input[9] <= 1.2429011464320532) {
                if (input[9] <= 0.48734480495531424) {
                    if (input[3] <= -40.251032300014614) {
                        if (input[7] <= 14.516473657826673) {
                            set_output2(var225, 0.8930862437633642, 0.10691375623663577);
                        } else {
                            set_output2(var225, 0.9272237196765498, 0.07277628032345014);
                        }
                    } else {
                        set_output2(var225, 0.7887323943661971, 0.2112676056338028);
                    }
                } else {
                    set_output2(var225, 0.0, 1.0);
                }
            } else {
                set_output2(var225, 0.0, 1.0);
            }
        } else {
            set_output2(var225, 0.0, 1.0);
        }
    } else {
        set_output2(var225, 0.0, 1.0);
    }
    add_vectors(var96, var225, 2, var95);
    double var226[2];
    if (input[3] <= -40.28034914882133) {
        if (input[9] <= 3.8696251660852443) {
            if (input[9] <= 0.612128502831187) {
                if (input[9] <= 0.44761569888843306) {
                    if (input[1] <= 10.113142204882703) {
                        if (input[1] <= 6.840547833820995) {
                            set_output2(var226, 0.9542799085598171, 0.04572009144018288);
                        } else {
                            set_output2(var226, 0.9095890410958904, 0.09041095890410959);
                        }
                    } else {
                        if (input[0] <= 34.690824085915054) {
                            set_output2(var226, 0.8865979381443299, 0.1134020618556701);
                        } else {
                            set_output2(var226, 1.0, 0.0);
                        }
                    }
                } else {
                    set_output2(var226, 0.0, 1.0);
                }
            } else {
                set_output2(var226, 0.0, 1.0);
            }
        } else {
            set_output2(var226, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -36.243803842025436) {
            set_output2(var226, 0.7272727272727273, 0.2727272727272727);
        } else {
            set_output2(var226, 0.0, 1.0);
        }
    }
    add_vectors(var95, var226, 2, var94);
    double var227[2];
    if (input[9] <= 3.9275036119567677) {
        if (input[9] <= 1.4555790965398385) {
            if (input[9] <= 1.3151673236933468) {
                if (input[9] <= 0.5963678323421298) {
                    if (input[7] <= 12.87538148437979) {
                        set_output2(var227, 0.6557377049180327, 0.3442622950819672);
                    } else {
                        if (input[9] <= 0.27519391203949917) {
                            set_output2(var227, 0.9113376450935881, 0.08866235490641197);
                        } else {
                            set_output2(var227, 0.8421052631578947, 0.15789473684210525);
                        }
                    }
                } else {
                    set_output2(var227, 0.0, 1.0);
                }
            } else {
                set_output2(var227, 0.0, 1.0);
            }
        } else {
            set_output2(var227, 0.0, 1.0);
        }
    } else {
        set_output2(var227, 0.0, 1.0);
    }
    add_vectors(var94, var227, 2, var93);
    double var228[2];
    if (input[1] <= 12.359491843103129) {
        if (input[4] <= 18.448848179051446) {
            if (input[2] <= -1.6866371485260423) {
                if (input[9] <= 5.232861575208192) {
                    if (input[9] <= 2.9729554131475844) {
                        if (input[7] <= 12.536033952306227) {
                            set_output2(var228, 0.5333333333333333, 0.4666666666666667);
                        } else {
                            set_output2(var228, 0.8998640128569663, 0.10013598714303375);
                        }
                    } else {
                        set_output2(var228, 0.0, 1.0);
                    }
                } else {
                    set_output2(var228, 0.0, 1.0);
                }
            } else {
                set_output2(var228, 0.0, 1.0);
            }
        } else {
            set_output2(var228, 0.0, 1.0);
        }
    } else {
        set_output2(var228, 0.3076923076923077, 0.6923076923076923);
    }
    add_vectors(var93, var228, 2, var92);
    double var229[2];
    if (input[9] <= 4.840763274351482) {
        if (input[8] <= 1.4773255347870695) {
            if (input[9] <= 2.4757280102080643) {
                if (input[9] <= 0.6333605691200617) {
                    if (input[9] <= 0.4477726671902463) {
                        if (input[4] <= 4.951233642213691) {
                            set_output2(var229, 0.89289964299881, 0.10710035700119);
                        } else {
                            set_output2(var229, 0.9384164222873901, 0.06158357771260997);
                        }
                    } else {
                        set_output2(var229, 0.0, 1.0);
                    }
                } else {
                    set_output2(var229, 0.0, 1.0);
                }
            } else {
                set_output2(var229, 0.0, 1.0);
            }
        } else {
            set_output2(var229, 0.0, 1.0);
        }
    } else {
        set_output2(var229, 0.0, 1.0);
    }
    add_vectors(var92, var229, 2, var91);
    double var230[2];
    if (input[4] <= 18.308009221074762) {
        if (input[5] <= 0.03560155339792756) {
            set_output2(var230, 0.5, 0.5);
        } else {
            if (input[9] <= 0.10990837299368898) {
                if (input[5] <= 0.2925937659856752) {
                    if (input[9] <= 0.006534861412359376) {
                        set_output2(var230, 0.8, 0.2);
                    } else {
                        if (input[0] <= 33.723248688118225) {
                            set_output2(var230, 0.9072089426703204, 0.09279105732967957);
                        } else {
                            set_output2(var230, 0.9617942549879722, 0.03820574501202773);
                        }
                    }
                } else {
                    set_output2(var230, 0.6666666666666666, 0.3333333333333333);
                }
            } else {
                if (input[2] <= -0.3784230633840302) {
                    if (input[9] <= 0.43422401437696245) {
                        if (input[7] <= 12.466579256217205) {
                            set_output2(var230, 0.7272727272727273, 0.2727272727272727);
                        } else {
                            set_output2(var230, 0.9193516967312113, 0.08064830326878872);
                        }
                    } else {
                        set_output2(var230, 0.0, 1.0);
                    }
                } else {
                    set_output2(var230, 0.0, 1.0);
                }
            }
        }
    } else {
        set_output2(var230, 0.0, 1.0);
    }
    add_vectors(var91, var230, 2, var90);
    double var231[2];
    if (input[2] <= -2.717682749328091) {
        if (input[3] <= -37.23543705876236) {
            if (input[2] <= -15.624053135706227) {
                set_output2(var231, 1.0, 0.0);
            } else {
                if (input[9] <= 2.4641760330894336) {
                    if (input[8] <= 1.233239283397426) {
                        if (input[9] <= 2.1708166558510738) {
                            set_output2(var231, 0.8929460580912864, 0.10705394190871369);
                        } else {
                            set_output2(var231, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var231, 0.0, 1.0);
                    }
                } else {
                    set_output2(var231, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var231, 0.0, 1.0);
        }
    } else {
        set_output2(var231, 0.0, 1.0);
    }
    add_vectors(var90, var231, 2, var89);
    double var232[2];
    if (input[1] <= 12.55261074876791) {
        if (input[4] <= 24.484927708814663) {
            if (input[3] <= -37.25658466198912) {
                if (input[9] <= 8.70720701665161) {
                    if (input[9] <= 4.333951487817825) {
                        if (input[9] <= 0.5943278582957153) {
                            set_output2(var232, 0.9052774018944519, 0.09472259810554803);
                        } else {
                            set_output2(var232, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var232, 0.0, 1.0);
                    }
                } else {
                    set_output2(var232, 0.0, 1.0);
                }
            } else {
                set_output2(var232, 0.0, 1.0);
            }
        } else {
            set_output2(var232, 0.0, 1.0);
        }
    } else {
        set_output2(var232, 0.3076923076923077, 0.6923076923076923);
    }
    add_vectors(var89, var232, 2, var88);
    double var233[2];
    if (input[9] <= 11.719311333998496) {
        if (input[1] <= 12.416152556518067) {
            if (input[7] <= 13.932463365041924) {
                if (input[9] <= 4.783473645151952) {
                    if (input[7] <= 11.777549355740849) {
                        set_output2(var233, 0.0, 1.0);
                    } else {
                        if (input[9] <= 1.0819461882764667) {
                            set_output2(var233, 0.8378909448174227, 0.16210905518257737);
                        } else {
                            set_output2(var233, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var233, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 5.924685320549646) {
                    if (input[2] <= -5.592890051975601) {
                        if (input[1] <= 6.760815258639691) {
                            set_output2(var233, 0.9565930355937108, 0.04340696440628918);
                        } else {
                            set_output2(var233, 0.9054441260744985, 0.09455587392550144);
                        }
                    } else {
                        if (input[9] <= 1.1058128617723992) {
                            set_output2(var233, 0.8986083499005965, 0.10139165009940358);
                        } else {
                            set_output2(var233, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var233, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var233, 0.3076923076923077, 0.6923076923076923);
        }
    } else {
        set_output2(var233, 0.0, 1.0);
    }
    add_vectors(var88, var233, 2, var87);
    double var234[2];
    if (input[9] <= 0.4921885605192282) {
        if (input[4] <= 4.977898325452408) {
            if (input[5] <= 0.2378064710649546) {
                if (input[2] <= -19.065445284672037) {
                    set_output2(var234, 1.0, 0.0);
                } else {
                    if (input[9] <= 0.256394554449971) {
                        if (input[4] <= 4.062957526149695) {
                            set_output2(var234, 0.9042553191489362, 0.09574468085106383);
                        } else {
                            set_output2(var234, 0.8125, 0.1875);
                        }
                    } else {
                        set_output2(var234, 0.7272727272727273, 0.2727272727272727);
                    }
                }
            } else {
                set_output2(var234, 0.7619047619047619, 0.23809523809523808);
            }
        } else {
            if (input[1] <= 9.651514302133457) {
                if (input[9] <= 0.056083118180639255) {
                    set_output2(var234, 0.96, 0.04);
                } else {
                    set_output2(var234, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -6.41348404185344) {
                    set_output2(var234, 0.8, 0.2);
                } else {
                    if (input[1] <= 11.679186155358924) {
                        if (input[0] <= 33.76440976815376) {
                            set_output2(var234, 0.8858131487889274, 0.11418685121107267);
                        } else {
                            set_output2(var234, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var234, 1.0, 0.0);
                    }
                }
            }
        }
    } else {
        set_output2(var234, 0.0, 1.0);
    }
    add_vectors(var87, var234, 2, var86);
    double var235[2];
    if (input[9] <= 1.972263150031084) {
        if (input[3] <= -38.10994356955002) {
            if (input[9] <= 0.8610000242126062) {
                if (input[9] <= 0.32536758745258504) {
                    if (input[1] <= 7.585636107533153) {
                        if (input[8] <= 0.7923917540887414) {
                            set_output2(var235, 0.9519120677810854, 0.04808793221891458);
                        } else {
                            set_output2(var235, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        if (input[7] <= 13.195475325287909) {
                            set_output2(var235, 0.7272727272727273, 0.2727272727272727);
                        } else {
                            set_output2(var235, 0.9052631578947369, 0.09473684210526316);
                        }
                    }
                } else {
                    set_output2(var235, 0.5161290322580645, 0.4838709677419355);
                }
            } else {
                set_output2(var235, 0.0, 1.0);
            }
        } else {
            set_output2(var235, 0.0, 1.0);
        }
    } else {
        set_output2(var235, 0.0, 1.0);
    }
    add_vectors(var86, var235, 2, var85);
    double var236[2];
    if (input[9] <= 7.763992989323287) {
        if (input[9] <= 2.0415194742548413) {
            if (input[9] <= 0.5761333138759024) {
                if (input[4] <= 4.651891713693962) {
                    if (input[7] <= 14.099457375956607) {
                        if (input[4] <= 1.6550279998666368) {
                            set_output2(var236, 0.6742277992277992, 0.3257722007722008);
                        } else {
                            set_output2(var236, 0.8282676843396484, 0.17173231566035163);
                        }
                    } else {
                        if (input[9] <= 0.15100730537558169) {
                            set_output2(var236, 0.9129593810444874, 0.08704061895551257);
                        } else {
                            set_output2(var236, 0.9795918367346939, 0.02040816326530612);
                        }
                    }
                } else {
                    if (input[3] <= -41.30428309798089) {
                        if (input[3] <= -50.550793801529245) {
                            set_output2(var236, 0.9090909090909091, 0.09090909090909091);
                        } else {
                            set_output2(var236, 0.9585253456221198, 0.041474654377880185);
                        }
                    } else {
                        set_output2(var236, 0.7272727272727273, 0.2727272727272727);
                    }
                }
            } else {
                set_output2(var236, 0.0, 1.0);
            }
        } else {
            set_output2(var236, 0.0, 1.0);
        }
    } else {
        set_output2(var236, 0.0, 1.0);
    }
    add_vectors(var85, var236, 2, var84);
    double var237[2];
    if (input[9] <= 10.360092210777799) {
        if (input[9] <= 5.326531844969785) {
            if (input[4] <= 15.515464018601286) {
                if (input[1] <= 7.1109902857667535) {
                    if (input[9] <= 1.1569612910701734) {
                        if (input[5] <= 0.22580302286653367) {
                            set_output2(var237, 0.9631177772313745, 0.03688222276862552);
                        } else {
                            set_output2(var237, 0.8421052631578947, 0.15789473684210525);
                        }
                    } else {
                        set_output2(var237, 0.0, 1.0);
                    }
                } else {
                    if (input[9] <= 1.2696888287796906) {
                        if (input[7] <= 14.006142306325787) {
                            set_output2(var237, 0.8227571115973742, 0.1772428884026258);
                        } else {
                            set_output2(var237, 0.9034700315457413, 0.09652996845425868);
                        }
                    } else {
                        set_output2(var237, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var237, 0.0, 1.0);
            }
        } else {
            set_output2(var237, 0.0, 1.0);
        }
    } else {
        set_output2(var237, 0.0, 1.0);
    }
    add_vectors(var84, var237, 2, var83);
    double var238[2];
    if (input[9] <= 5.796625692187263) {
        if (input[9] <= 1.4063926975886907) {
            if (input[7] <= 14.391420598197339) {
                if (input[9] <= 0.38496586253760967) {
                    if (input[7] <= 13.534232093253753) {
                        if (input[4] <= 0.7571792642560065) {
                            set_output2(var238, 0.0, 1.0);
                        } else {
                            set_output2(var238, 0.8126734505087881, 0.18732654949121186);
                        }
                    } else {
                        if (input[1] <= 11.820323581052003) {
                            set_output2(var238, 0.9021226890079349, 0.09787731099206509);
                        } else {
                            set_output2(var238, 0.8421052631578947, 0.15789473684210525);
                        }
                    }
                } else {
                    set_output2(var238, 0.34782608695652173, 0.6521739130434783);
                }
            } else {
                if (input[8] <= 1.5341955736516866) {
                    if (input[9] <= 0.7085969670228529) {
                        if (input[5] <= 0.24744038620050224) {
                            set_output2(var238, 0.9285165257494236, 0.07148347425057648);
                        } else {
                            set_output2(var238, 0.7887323943661971, 0.2112676056338028);
                        }
                    } else {
                        set_output2(var238, 0.0, 1.0);
                    }
                } else {
                    set_output2(var238, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var238, 0.0, 1.0);
        }
    } else {
        set_output2(var238, 0.0, 1.0);
    }
    add_vectors(var83, var238, 2, var82);
    double var239[2];
    if (input[1] <= 12.933761723625537) {
        if (input[9] <= 8.628518073782132) {
            if (input[9] <= 3.39137955919191) {
                if (input[4] <= 24.604801421770034) {
                    if (input[9] <= 1.2951251338993885) {
                        if (input[1] <= 9.923323804854151) {
                            set_output2(var239, 0.9265329189036451, 0.0734670810963549);
                        } else {
                            set_output2(var239, 0.8743789921930447, 0.1256210078069553);
                        }
                    } else {
                        set_output2(var239, 0.0, 1.0);
                    }
                } else {
                    set_output2(var239, 0.0, 1.0);
                }
            } else {
                set_output2(var239, 0.0, 1.0);
            }
        } else {
            set_output2(var239, 0.0, 1.0);
        }
    } else {
        set_output2(var239, 0.0, 1.0);
    }
    add_vectors(var82, var239, 2, var81);
    double var240[2];
    if (input[1] <= 13.107454341211616) {
        if (input[9] <= 5.269636317650657) {
            if (input[9] <= 2.0212808233438864) {
                if (input[9] <= 1.125581198186758) {
                    if (input[7] <= 12.000857641702304) {
                        set_output2(var240, 0.0, 1.0);
                    } else {
                        if (input[9] <= 0.9637508587963365) {
                            set_output2(var240, 0.9022681923687746, 0.09773180763122533);
                        } else {
                            set_output2(var240, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var240, 0.0, 1.0);
                }
            } else {
                set_output2(var240, 0.0, 1.0);
            }
        } else {
            set_output2(var240, 0.0, 1.0);
        }
    } else {
        set_output2(var240, 0.0, 1.0);
    }
    add_vectors(var81, var240, 2, var80);
    double var241[2];
    if (input[4] <= 16.52858840970203) {
        if (input[2] <= -2.9754235884851212) {
            if (input[7] <= 13.447688841321053) {
                if (input[7] <= 11.81200099664554) {
                    set_output2(var241, 0.0, 1.0);
                } else {
                    if (input[2] <= -7.2549219966093546) {
                        set_output2(var241, 0.9106256206554122, 0.08937437934458789);
                    } else {
                        set_output2(var241, 0.64, 0.36);
                    }
                }
            } else {
                if (input[9] <= 4.539799613597545) {
                    if (input[3] <= -37.64027066702041) {
                        if (input[7] <= 15.761550481036432) {
                            set_output2(var241, 0.8994588477664776, 0.10054115223352242);
                        } else {
                            set_output2(var241, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var241, 0.0, 1.0);
                    }
                } else {
                    set_output2(var241, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var241, 0.0, 1.0);
        }
    } else {
        set_output2(var241, 0.0, 1.0);
    }
    add_vectors(var80, var241, 2, var79);
    double var242[2];
    if (input[4] <= 15.87891832810113) {
        if (input[9] <= 7.0956636806018345) {
            if (input[9] <= 4.257155386061221) {
                if (input[9] <= 2.6757913498215693) {
                    if (input[3] <= -34.93840053324206) {
                        if (input[9] <= 1.613649311951426) {
                            set_output2(var242, 0.8967903832921844, 0.1032096167078156);
                        } else {
                            set_output2(var242, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var242, 0.0, 1.0);
                    }
                } else {
                    set_output2(var242, 0.0, 1.0);
                }
            } else {
                set_output2(var242, 0.0, 1.0);
            }
        } else {
            set_output2(var242, 0.0, 1.0);
        }
    } else {
        set_output2(var242, 0.0, 1.0);
    }
    add_vectors(var79, var242, 2, var78);
    double var243[2];
    if (input[2] <= -2.2637220766323694) {
        if (input[7] <= 11.690194898568892) {
            set_output2(var243, 0.0, 1.0);
        } else {
            if (input[4] <= 16.274622134001866) {
                if (input[9] <= 0.9579294984367215) {
                    if (input[4] <= 5.404383637851929) {
                        if (input[9] <= 0.448903913955524) {
                            set_output2(var243, 0.8965350002694401, 0.1034649997305599);
                        } else {
                            set_output2(var243, 0.0, 1.0);
                        }
                    } else {
                        if (input[1] <= 9.628841046083629) {
                            set_output2(var243, 0.985781990521327, 0.014218009478672985);
                        } else {
                            set_output2(var243, 0.9124668435013262, 0.08753315649867374);
                        }
                    }
                } else {
                    set_output2(var243, 0.0, 1.0);
                }
            } else {
                set_output2(var243, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var243, 0.0, 1.0);
    }
    add_vectors(var78, var243, 2, var77);
    double var244[2];
    if (input[4] <= 24.40160124609707) {
        if (input[3] <= -32.975932185667645) {
            if (input[7] <= 13.698801818074877) {
                if (input[4] <= 5.853306350269785) {
                    if (input[4] <= 0.7086201879439848) {
                        set_output2(var244, 0.0, 1.0);
                    } else {
                        if (input[9] <= 2.096035937305093) {
                            set_output2(var244, 0.7355015673981191, 0.2644984326018809);
                        } else {
                            set_output2(var244, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var244, 0.8888888888888888, 0.1111111111111111);
                }
            } else {
                if (input[9] <= 2.570398879318211) {
                    if (input[5] <= 0.3455624760044688) {
                        if (input[9] <= 0.3214266055806003) {
                            set_output2(var244, 0.9227371345622551, 0.07726286543774487);
                        } else {
                            set_output2(var244, 0.27586206896551724, 0.7241379310344828);
                        }
                    } else {
                        set_output2(var244, 0.7692307692307693, 0.23076923076923078);
                    }
                } else {
                    set_output2(var244, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var244, 0.0, 1.0);
        }
    } else {
        set_output2(var244, 0.0, 1.0);
    }
    add_vectors(var77, var244, 2, var76);
    double var245[2];
    if (input[9] <= 2.9078126173593426) {
        if (input[9] <= 0.762092878495008) {
            if (input[9] <= 0.5330836568086224) {
                if (input[1] <= 12.526503553321152) {
                    if (input[7] <= 13.82840802748896) {
                        if (input[4] <= 2.5749910737981208) {
                            set_output2(var245, 0.7031009456784693, 0.2968990543215307);
                        } else {
                            set_output2(var245, 0.9191665169750315, 0.08083348302496857);
                        }
                    } else {
                        if (input[8] <= 0.7882918950362475) {
                            set_output2(var245, 0.9213141214250887, 0.07868587857491131);
                        } else {
                            set_output2(var245, 0.8524590163934426, 0.14754098360655737);
                        }
                    }
                } else {
                    set_output2(var245, 0.7272727272727273, 0.2727272727272727);
                }
            } else {
                set_output2(var245, 0.0, 1.0);
            }
        } else {
            set_output2(var245, 0.0, 1.0);
        }
    } else {
        set_output2(var245, 0.0, 1.0);
    }
    add_vectors(var76, var245, 2, var75);
    double var246[2];
    if (input[9] <= 4.77589857383104) {
        if (input[9] <= 2.1999549459647025) {
            if (input[4] <= 25.015518194447846) {
                if (input[7] <= 11.472238581151977) {
                    set_output2(var246, 0.0, 1.0);
                } else {
                    if (input[9] <= 1.1704929356015352) {
                        if (input[2] <= -10.004434266245887) {
                            set_output2(var246, 0.9374511336982018, 0.06254886630179829);
                        } else {
                            set_output2(var246, 0.8918789043728976, 0.10812109562710236);
                        }
                    } else {
                        set_output2(var246, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var246, 0.0, 1.0);
            }
        } else {
            set_output2(var246, 0.0, 1.0);
        }
    } else {
        set_output2(var246, 0.0, 1.0);
    }
    add_vectors(var75, var246, 2, var74);
    double var247[2];
    if (input[9] <= 2.1755467592581965) {
        if (input[9] <= 1.1225919388866072) {
            if (input[7] <= 13.160596058000424) {
                if (input[5] <= 0.08729806034578325) {
                    set_output2(var247, 0.8695652173913043, 0.13043478260869565);
                } else {
                    set_output2(var247, 0.5714285714285714, 0.42857142857142855);
                }
            } else {
                if (input[5] <= 0.038361715797472946) {
                    set_output2(var247, 0.6153846153846154, 0.38461538461538464);
                } else {
                    if (input[9] <= 0.851655293304771) {
                        if (input[6] <= 50.844540055218204) {
                            set_output2(var247, 0.9133882018356531, 0.08661179816434696);
                        } else {
                            set_output2(var247, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        set_output2(var247, 0.0, 1.0);
                    }
                }
            }
        } else {
            set_output2(var247, 0.0, 1.0);
        }
    } else {
        set_output2(var247, 0.0, 1.0);
    }
    add_vectors(var74, var247, 2, var73);
    double var248[2];
    if (input[9] <= 10.531699661069734) {
        if (input[9] <= 4.333015480767185) {
            if (input[9] <= 2.2258057039503707) {
                if (input[9] <= 0.8268077322340557) {
                    if (input[7] <= 13.18816642816511) {
                        if (input[7] <= 11.700450610222775) {
                            set_output2(var248, 0.0, 1.0);
                        } else {
                            set_output2(var248, 0.7920792079207921, 0.2079207920792079);
                        }
                    } else {
                        if (input[2] <= -16.92908094107204) {
                            set_output2(var248, 1.0, 0.0);
                        } else {
                            set_output2(var248, 0.90625, 0.09375);
                        }
                    }
                } else {
                    set_output2(var248, 0.0, 1.0);
                }
            } else {
                set_output2(var248, 0.0, 1.0);
            }
        } else {
            set_output2(var248, 0.0, 1.0);
        }
    } else {
        set_output2(var248, 0.0, 1.0);
    }
    add_vectors(var73, var248, 2, var72);
    double var249[2];
    if (input[9] <= 10.074127554003477) {
        if (input[9] <= 2.8749782978711935) {
            if (input[9] <= 2.413957404483579) {
                if (input[4] <= 16.155234919011175) {
                    if (input[9] <= 1.019859323223013) {
                        if (input[9] <= 0.43365015220652897) {
                            set_output2(var249, 0.9055871173417256, 0.09441288265827441);
                        } else {
                            set_output2(var249, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var249, 0.0, 1.0);
                    }
                } else {
                    set_output2(var249, 0.0, 1.0);
                }
            } else {
                set_output2(var249, 0.0, 1.0);
            }
        } else {
            set_output2(var249, 0.0, 1.0);
        }
    } else {
        set_output2(var249, 0.0, 1.0);
    }
    add_vectors(var72, var249, 2, var71);
    double var250[2];
    if (input[1] <= 14.237095834472004) {
        if (input[9] <= 10.0071507014623) {
            if (input[9] <= 8.117721233952567) {
                if (input[9] <= 3.7377949227821197) {
                    if (input[9] <= 0.7090929946694154) {
                        if (input[1] <= 7.27124686910831) {
                            set_output2(var250, 0.9444058373870744, 0.05559416261292564);
                        } else {
                            set_output2(var250, 0.8932806324110671, 0.1067193675889328);
                        }
                    } else {
                        set_output2(var250, 0.0, 1.0);
                    }
                } else {
                    set_output2(var250, 0.0, 1.0);
                }
            } else {
                set_output2(var250, 0.0, 1.0);
            }
        } else {
            set_output2(var250, 0.0, 1.0);
        }
    } else {
        set_output2(var250, 0.0, 1.0);
    }
    add_vectors(var71, var250, 2, var70);
    double var251[2];
    if (input[9] <= 6.8330317408417525) {
        if (input[9] <= 3.8598105911629896) {
            if (input[9] <= 1.7395216045919757) {
                if (input[9] <= 0.6835046011365825) {
                    if (input[9] <= 0.3219274632038365) {
                        if (input[7] <= 13.892689024640589) {
                            set_output2(var251, 0.8376846444520784, 0.16231535554792167);
                        } else {
                            set_output2(var251, 0.917750542617948, 0.082249457382052);
                        }
                    } else {
                        set_output2(var251, 0.64, 0.36);
                    }
                } else {
                    set_output2(var251, 0.0, 1.0);
                }
            } else {
                set_output2(var251, 0.0, 1.0);
            }
        } else {
            set_output2(var251, 0.0, 1.0);
        }
    } else {
        set_output2(var251, 0.0, 1.0);
    }
    add_vectors(var70, var251, 2, var69);
    double var252[2];
    if (input[7] <= 13.938064465045215) {
        if (input[4] <= 1.0135871381458728) {
            set_output2(var252, 0.37209302325581395, 0.627906976744186);
        } else {
            if (input[9] <= 1.0222380273581633) {
                if (input[9] <= 0.43555532849392736) {
                    if (input[8] <= 0.8748487514906224) {
                        if (input[7] <= 11.88777142733369) {
                            set_output2(var252, 0.0, 1.0);
                        } else {
                            set_output2(var252, 0.8699922958397535, 0.1300077041602465);
                        }
                    } else {
                        set_output2(var252, 1.0, 0.0);
                    }
                } else {
                    set_output2(var252, 0.0, 1.0);
                }
            } else {
                set_output2(var252, 0.0, 1.0);
            }
        }
    } else {
        if (input[4] <= 26.22746552131553) {
            if (input[9] <= 4.41548810958419) {
                if (input[0] <= 34.79912019589334) {
                    if (input[7] <= 14.156051189143588) {
                        if (input[7] <= 14.100320187913113) {
                            set_output2(var252, 0.7924832833756053, 0.20751671662439475);
                        } else {
                            set_output2(var252, 0.898876404494382, 0.10112359550561797);
                        }
                    } else {
                        if (input[1] <= 11.39658097288417) {
                            set_output2(var252, 0.9154228855721394, 0.0845771144278607);
                        } else {
                            set_output2(var252, 0.897025171624714, 0.10297482837528604);
                        }
                    }
                } else {
                    if (input[0] <= 37.441573878872056) {
                        if (input[9] <= 0.09142715896854678) {
                            set_output2(var252, 0.9552238805970149, 0.04477611940298507);
                        } else {
                            set_output2(var252, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var252, 0.8421052631578947, 0.15789473684210525);
                    }
                }
            } else {
                set_output2(var252, 0.0, 1.0);
            }
        } else {
            set_output2(var252, 0.0, 1.0);
        }
    }
    add_vectors(var69, var252, 2, var68);
    double var253[2];
    if (input[9] <= 2.2050899785876807) {
        if (input[9] <= 1.8406625422274818) {
            if (input[9] <= 0.7556738761458813) {
                if (input[9] <= 0.3335820049271015) {
                    if (input[7] <= 14.621984761649422) {
                        if (input[7] <= 13.9272518990362) {
                            set_output2(var253, 0.8378909448174227, 0.16210905518257737);
                        } else {
                            set_output2(var253, 0.9131749043316075, 0.08682509566839246);
                        }
                    } else {
                        if (input[8] <= 0.7888523066048407) {
                            set_output2(var253, 0.9308176100628931, 0.06918238993710692);
                        } else {
                            set_output2(var253, 0.7272727272727273, 0.2727272727272727);
                        }
                    }
                } else {
                    set_output2(var253, 0.5714285714285714, 0.42857142857142855);
                }
            } else {
                set_output2(var253, 0.0, 1.0);
            }
        } else {
            set_output2(var253, 0.0, 1.0);
        }
    } else {
        set_output2(var253, 0.0, 1.0);
    }
    add_vectors(var68, var253, 2, var67);
    double var254[2];
    if (input[9] <= 3.2569357484346453) {
        if (input[9] <= 1.1087107230758866) {
            if (input[7] <= 13.675440663321673) {
                if (input[4] <= 0.7953015026042981) {
                    set_output2(var254, 0.0, 1.0);
                } else {
                    if (input[7] <= 11.498012591338815) {
                        set_output2(var254, 0.0, 1.0);
                    } else {
                        if (input[9] <= 0.521068524191698) {
                            set_output2(var254, 0.8342032545287075, 0.16579674547129258);
                        } else {
                            set_output2(var254, 0.0, 1.0);
                        }
                    }
                }
            } else {
                if (input[1] <= 6.814350638724995) {
                    if (input[9] <= 0.15322789527393685) {
                        if (input[9] <= 0.05044690228493031) {
                            set_output2(var254, 0.9090909090909091, 0.09090909090909091);
                        } else {
                            set_output2(var254, 0.9848152522355323, 0.015184747764467689);
                        }
                    } else {
                        set_output2(var254, 1.0, 0.0);
                    }
                } else {
                    if (input[0] <= 35.10758110598437) {
                        if (input[2] <= -7.006159030582585) {
                            set_output2(var254, 0.8615384615384616, 0.13846153846153847);
                        } else {
                            set_output2(var254, 0.9056603773584906, 0.09433962264150944);
                        }
                    } else {
                        set_output2(var254, 0.9719626168224299, 0.028037383177570093);
                    }
                }
            }
        } else {
            set_output2(var254, 0.0, 1.0);
        }
    } else {
        set_output2(var254, 0.0, 1.0);
    }
    add_vectors(var67, var254, 2, var66);
    double var255[2];
    if (input[4] <= 25.534486575636187) {
        if (input[7] <= 12.818572310167518) {
            set_output2(var255, 0.6037735849056604, 0.39622641509433965);
        } else {
            if (input[7] <= 13.969024153417019) {
                if (input[4] <= 1.129986014840199) {
                    set_output2(var255, 0.34782608695652173, 0.6521739130434783);
                } else {
                    if (input[4] <= 3.9609425480679676) {
                        if (input[3] <= -37.04611879837343) {
                            set_output2(var255, 0.8290473407364114, 0.17095265926358855);
                        } else {
                            set_output2(var255, 0.0, 1.0);
                        }
                    } else {
                        if (input[4] <= 6.574142777474517) {
                            set_output2(var255, 0.8695652173913043, 0.13043478260869565);
                        } else {
                            set_output2(var255, 1.0, 0.0);
                        }
                    }
                }
            } else {
                if (input[3] <= -39.012569484287454) {
                    if (input[5] <= 0.21611398035244084) {
                        if (input[1] <= 7.074983276893779) {
                            set_output2(var255, 0.9672637992179686, 0.03273620078203146);
                        } else {
                            set_output2(var255, 0.8929266709928618, 0.10707332900713822);
                        }
                    } else {
                        if (input[6] <= 9.059107853097435) {
                            set_output2(var255, 0.8571428571428571, 0.14285714285714285);
                        } else {
                            set_output2(var255, 0.64, 0.36);
                        }
                    }
                } else {
                    set_output2(var255, 0.7272727272727273, 0.2727272727272727);
                }
            }
        }
    } else {
        set_output2(var255, 0.0, 1.0);
    }
    add_vectors(var66, var255, 2, var65);
    double var256[2];
    if (input[9] <= 2.332350669019109) {
        if (input[7] <= 13.238892033179816) {
            if (input[4] <= 3.156762060499125) {
                set_output2(var256, 0.5253790375741596, 0.4746209624258405);
            } else {
                set_output2(var256, 0.8615384615384616, 0.13846153846153847);
            }
        } else {
            if (input[9] <= 1.2911391891710216) {
                if (input[1] <= 12.46786065619321) {
                    if (input[9] <= 0.2676981405668704) {
                        if (input[7] <= 16.034489441984803) {
                            set_output2(var256, 0.9163025507794048, 0.08369744922059519);
                        } else {
                            set_output2(var256, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        set_output2(var256, 0.6666666666666666, 0.3333333333333333);
                    }
                } else {
                    set_output2(var256, 0.5714285714285714, 0.42857142857142855);
                }
            } else {
                set_output2(var256, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var256, 0.0, 1.0);
    }
    add_vectors(var65, var256, 2, var64);
    double var257[2];
    if (input[2] <= -0.23493405787125354) {
        if (input[9] <= 1.3394556105561335) {
            if (input[9] <= 0.7355052578749236) {
                if (input[1] <= 9.783503730255045) {
                    if (input[7] <= 13.947897237390112) {
                        if (input[4] <= 0.660241365087243) {
                            set_output2(var257, 0.0, 1.0);
                        } else {
                            set_output2(var257, 0.9329025844930418, 0.06709741550695826);
                        }
                    } else {
                        if (input[5] <= 0.27437925802351315) {
                            set_output2(var257, 0.9348993033083336, 0.06510069669166635);
                        } else {
                            set_output2(var257, 1.0, 0.0);
                        }
                    }
                } else {
                    if (input[9] <= 0.4367586403492075) {
                        if (input[0] <= 35.697687470485356) {
                            set_output2(var257, 0.8849557522123894, 0.11504424778761062);
                        } else {
                            set_output2(var257, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var257, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var257, 0.0, 1.0);
            }
        } else {
            set_output2(var257, 0.0, 1.0);
        }
    } else {
        set_output2(var257, 0.0, 1.0);
    }
    add_vectors(var64, var257, 2, var63);
    double var258[2];
    if (input[1] <= 12.101150805501971) {
        if (input[9] <= 6.237219572257817) {
            if (input[9] <= 2.778241840614615) {
                if (input[9] <= 1.1937698666661234) {
                    if (input[9] <= 0.3800114018720333) {
                        if (input[3] <= -39.49795947954566) {
                            set_output2(var258, 0.9125364431486881, 0.08746355685131195);
                        } else {
                            set_output2(var258, 0.6153846153846154, 0.38461538461538464);
                        }
                    } else {
                        set_output2(var258, 0.27586206896551724, 0.7241379310344828);
                    }
                } else {
                    set_output2(var258, 0.0, 1.0);
                }
            } else {
                set_output2(var258, 0.0, 1.0);
            }
        } else {
            set_output2(var258, 0.0, 1.0);
        }
    } else {
        set_output2(var258, 0.47058823529411764, 0.5294117647058824);
    }
    add_vectors(var63, var258, 2, var62);
    double var259[2];
    if (input[7] <= 14.004260915903558) {
        if (input[4] <= 1.7860963674223556) {
            if (input[1] <= 10.460754078159) {
                set_output2(var259, 0.3056894889103182, 0.6943105110896818);
            } else {
                if (input[9] <= 2.939225852740706) {
                    set_output2(var259, 0.7272727272727273, 0.2727272727272727);
                } else {
                    set_output2(var259, 0.0, 1.0);
                }
            }
        } else {
            if (input[2] <= -3.0390321445137616) {
                if (input[9] <= 0.22884890252455486) {
                    if (input[1] <= 11.79844863202533) {
                        if (input[7] <= 12.520378437918602) {
                            set_output2(var259, 0.8, 0.2);
                        } else {
                            set_output2(var259, 0.9467266485142655, 0.053273351485734574);
                        }
                    } else {
                        set_output2(var259, 0.6808510638297872, 0.3191489361702128);
                    }
                } else {
                    set_output2(var259, 0.27586206896551724, 0.7241379310344828);
                }
            } else {
                set_output2(var259, 0.0, 1.0);
            }
        }
    } else {
        if (input[8] <= 1.3683318469969858) {
            if (input[9] <= 1.6654401652106952) {
                if (input[9] <= 0.8592757362080239) {
                    if (input[9] <= 0.5157889852312209) {
                        if (input[5] <= 0.37899302959407644) {
                            set_output2(var259, 0.9231780391621285, 0.07682196083787152);
                        } else {
                            set_output2(var259, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        set_output2(var259, 0.0, 1.0);
                    }
                } else {
                    set_output2(var259, 0.0, 1.0);
                }
            } else {
                set_output2(var259, 0.0, 1.0);
            }
        } else {
            set_output2(var259, 0.0, 1.0);
        }
    }
    add_vectors(var62, var259, 2, var61);
    double var260[2];
    if (input[2] <= -2.818411494135571) {
        if (input[7] <= 13.643245691149136) {
            if (input[9] <= 1.8591236806634626) {
                if (input[3] <= -45.36061786232642) {
                    if (input[4] <= 1.5368746825759139) {
                        set_output2(var260, 0.5, 0.5);
                    } else {
                        set_output2(var260, 0.8359161349134001, 0.16408386508659983);
                    }
                } else {
                    if (input[1] <= 11.085405134091676) {
                        set_output2(var260, 1.0, 0.0);
                    } else {
                        set_output2(var260, 0.6557377049180327, 0.3442622950819672);
                    }
                }
            } else {
                set_output2(var260, 0.0, 1.0);
            }
        } else {
            if (input[9] <= 3.894586248371358) {
                if (input[4] <= 24.13518907777047) {
                    if (input[7] <= 14.165722632755347) {
                        if (input[1] <= 12.52809453008243) {
                            set_output2(var260, 0.8780922506594425, 0.1219077493405575);
                        } else {
                            set_output2(var260, 0.0, 1.0);
                        }
                    } else {
                        if (input[9] <= 1.2823587256046345) {
                            set_output2(var260, 0.9220338983050848, 0.07796610169491526);
                        } else {
                            set_output2(var260, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var260, 0.0, 1.0);
                }
            } else {
                set_output2(var260, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var260, 0.0, 1.0);
    }
    add_vectors(var61, var260, 2, var60);
    double var261[2];
    if (input[9] <= 12.618716982292803) {
        if (input[9] <= 3.737560715555251) {
            if (input[9] <= 2.340032173604928) {
                if (input[9] <= 1.3915620747381585) {
                    if (input[4] <= 14.105563155742198) {
                        if (input[9] <= 0.9000733194856555) {
                            set_output2(var261, 0.9000690579680708, 0.09993094203192916);
                        } else {
                            set_output2(var261, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var261, 0.0, 1.0);
                    }
                } else {
                    set_output2(var261, 0.0, 1.0);
                }
            } else {
                set_output2(var261, 0.0, 1.0);
            }
        } else {
            set_output2(var261, 0.0, 1.0);
        }
    } else {
        set_output2(var261, 0.0, 1.0);
    }
    add_vectors(var60, var261, 2, var59);
    double var262[2];
    if (input[9] <= 1.787977036030707) {
        if (input[1] <= 10.426777609369765) {
            if (input[3] <= -45.81310066291147) {
                if (input[7] <= 12.824575731375809) {
                    set_output2(var262, 0.7272727272727273, 0.2727272727272727);
                } else {
                    if (input[9] <= 0.28893362964558283) {
                        if (input[1] <= 10.386075370524315) {
                            set_output2(var262, 0.9142584947602413, 0.08574150523975865);
                        } else {
                            set_output2(var262, 0.5714285714285714, 0.42857142857142855);
                        }
                    } else {
                        set_output2(var262, 0.4, 0.6);
                    }
                }
            } else {
                if (input[7] <= 14.689011459521648) {
                    set_output2(var262, 1.0, 0.0);
                } else {
                    set_output2(var262, 0.8888888888888888, 0.1111111111111111);
                }
            }
        } else {
            if (input[4] <= 17.046101912947012) {
                if (input[0] <= 32.81278456723723) {
                    if (input[9] <= 0.7132255626992019) {
                        if (input[9] <= 0.4908227947117694) {
                            set_output2(var262, 0.8839779005524862, 0.11602209944751381);
                        } else {
                            set_output2(var262, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var262, 0.0, 1.0);
                    }
                } else {
                    if (input[1] <= 11.454849990436955) {
                        if (input[6] <= 7.967591251775217) {
                            set_output2(var262, 0.7920792079207921, 0.2079207920792079);
                        } else {
                            set_output2(var262, 1.0, 0.0);
                        }
                    } else {
                        if (input[3] <= -50.52737473423409) {
                            set_output2(var262, 0.9719626168224299, 0.028037383177570093);
                        } else {
                            set_output2(var262, 0.8888888888888888, 0.1111111111111111);
                        }
                    }
                }
            } else {
                set_output2(var262, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var262, 0.0, 1.0);
    }
    add_vectors(var59, var262, 2, var58);
    double var263[2];
    if (input[9] <= 0.2696435435430318) {
        if (input[1] <= 12.005743397010058) {
            if (input[2] <= -9.090914194691738) {
                if (input[9] <= 0.04261527293938733) {
                    if (input[8] <= 0.5977397514310974) {
                        set_output2(var263, 1.0, 0.0);
                    } else {
                        set_output2(var263, 0.8421052631578947, 0.15789473684210525);
                    }
                } else {
                    if (input[6] <= 43.03465561154248) {
                        if (input[1] <= 7.34685659649529) {
                            set_output2(var263, 1.0, 0.0);
                        } else {
                            set_output2(var263, 0.8695652173913043, 0.13043478260869565);
                        }
                    } else {
                        set_output2(var263, 0.7272727272727273, 0.2727272727272727);
                    }
                }
            } else {
                if (input[7] <= 12.951228365806301) {
                    set_output2(var263, 0.7692307692307693, 0.23076923076923078);
                } else {
                    if (input[0] <= 27.68976412511037) {
                        set_output2(var263, 0.8163265306122449, 0.1836734693877551);
                    } else {
                        if (input[0] <= 35.475581404037136) {
                            set_output2(var263, 0.897910447761194, 0.10208955223880598);
                        } else {
                            set_output2(var263, 1.0, 0.0);
                        }
                    }
                }
            }
        } else {
            set_output2(var263, 0.8163265306122449, 0.1836734693877551);
        }
    } else {
        if (input[4] <= 26.893799490096345) {
            if (input[9] <= 12.191768147342543) {
                if (input[1] <= 11.269358735374842) {
                    if (input[8] <= 0.729950666550214) {
                        set_output2(var263, 0.3076923076923077, 0.6923076923076923);
                    } else {
                        set_output2(var263, 0.8163265306122449, 0.1836734693877551);
                    }
                } else {
                    set_output2(var263, 0.0, 1.0);
                }
            } else {
                set_output2(var263, 0.0, 1.0);
            }
        } else {
            set_output2(var263, 0.0, 1.0);
        }
    }
    add_vectors(var58, var263, 2, var57);
    double var264[2];
    if (input[9] <= 4.997219791078923) {
        if (input[9] <= 1.9432863750542981) {
            if (input[9] <= 1.252638216727581) {
                if (input[1] <= 10.012544886936116) {
                    if (input[9] <= 0.6686154499203906) {
                        if (input[4] <= 5.168575144151488) {
                            set_output2(var264, 0.9074873566054027, 0.09251264339459726);
                        } else {
                            set_output2(var264, 0.9777777777777777, 0.022222222222222223);
                        }
                    } else {
                        set_output2(var264, 0.0, 1.0);
                    }
                } else {
                    if (input[3] <= -46.89226904824078) {
                        if (input[9] <= 0.13178176660852434) {
                            set_output2(var264, 0.9132947976878613, 0.08670520231213873);
                        } else {
                            set_output2(var264, 0.8145454545454546, 0.18545454545454546);
                        }
                    } else {
                        if (input[3] <= -38.18577895044572) {
                            set_output2(var264, 0.8759124087591241, 0.12408759124087591);
                        } else {
                            set_output2(var264, 0.0, 1.0);
                        }
                    }
                }
            } else {
                set_output2(var264, 0.0, 1.0);
            }
        } else {
            set_output2(var264, 0.0, 1.0);
        }
    } else {
        set_output2(var264, 0.0, 1.0);
    }
    add_vectors(var57, var264, 2, var56);
    double var265[2];
    if (input[8] <= 1.2474859060823584) {
        if (input[3] <= -33.21352963278868) {
            if (input[9] <= 4.7790390179473725) {
                if (input[3] <= -37.937429009674915) {
                    if (input[9] <= 1.161672421823751) {
                        if (input[9] <= 0.80098213498112) {
                            set_output2(var265, 0.9022681923687746, 0.09773180763122533);
                        } else {
                            set_output2(var265, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var265, 0.0, 1.0);
                    }
                } else {
                    set_output2(var265, 0.0, 1.0);
                }
            } else {
                set_output2(var265, 0.0, 1.0);
            }
        } else {
            set_output2(var265, 0.0, 1.0);
        }
    } else {
        set_output2(var265, 0.0, 1.0);
    }
    add_vectors(var56, var265, 2, var55);
    double var266[2];
    if (input[9] <= 11.542565652303878) {
        if (input[4] <= 20.239476106989308) {
            if (input[9] <= 3.782293262326333) {
                if (input[9] <= 0.8488092879153462) {
                    if (input[0] <= 33.92211732373644) {
                        if (input[7] <= 13.82341015652915) {
                            set_output2(var266, 0.7900932918702799, 0.2099067081297201);
                        } else {
                            set_output2(var266, 0.9108244531688166, 0.0891755468311834);
                        }
                    } else {
                        if (input[5] <= 0.14766900287256607) {
                            set_output2(var266, 0.9632540573644993, 0.03674594263550066);
                        } else {
                            set_output2(var266, 0.8163265306122449, 0.1836734693877551);
                        }
                    }
                } else {
                    set_output2(var266, 0.0, 1.0);
                }
            } else {
                set_output2(var266, 0.0, 1.0);
            }
        } else {
            set_output2(var266, 0.0, 1.0);
        }
    } else {
        set_output2(var266, 0.0, 1.0);
    }
    add_vectors(var55, var266, 2, var54);
    double var267[2];
    if (input[4] <= 21.040509687455142) {
        if (input[7] <= 13.498483549085465) {
            if (input[7] <= 11.382468704930524) {
                set_output2(var267, 0.0, 1.0);
            } else {
                if (input[8] <= 0.7275003648607958) {
                    if (input[7] <= 13.002052873150781) {
                        set_output2(var267, 0.0, 1.0);
                    } else {
                        set_output2(var267, 0.6528447444551592, 0.34715525554484095);
                    }
                } else {
                    set_output2(var267, 0.9302325581395349, 0.06976744186046512);
                }
            }
        } else {
            if (input[9] <= 12.662119980513566) {
                if (input[6] <= 0.48767350147975586) {
                    set_output2(var267, 0.96, 0.04);
                } else {
                    if (input[9] <= 7.2647921096890515) {
                        if (input[9] <= 3.332044720430541) {
                            set_output2(var267, 0.8937379163776437, 0.10626208362235637);
                        } else {
                            set_output2(var267, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var267, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var267, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var267, 0.0, 1.0);
    }
    add_vectors(var54, var267, 2, var53);
    double var268[2];
    if (input[9] <= 2.095653770603816) {
        if (input[4] <= 16.6026347532677) {
            if (input[1] <= 10.569766058414851) {
                if (input[9] <= 0.9330763782127013) {
                    if (input[7] <= 14.021322256140508) {
                        if (input[8] <= 0.6910905755695312) {
                            set_output2(var268, 0.8299480396787907, 0.17005196032120926);
                        } else {
                            set_output2(var268, 0.9302325581395349, 0.06976744186046512);
                        }
                    } else {
                        if (input[5] <= 0.04921835569102986) {
                            set_output2(var268, 0.6666666666666666, 0.3333333333333333);
                        } else {
                            set_output2(var268, 0.9399752278718201, 0.06002477212817989);
                        }
                    }
                } else {
                    set_output2(var268, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 0.6120130667086654) {
                    if (input[0] <= 34.509442831559824) {
                        if (input[2] <= -5.734214555241401) {
                            set_output2(var268, 0.6666666666666666, 0.3333333333333333);
                        } else {
                            set_output2(var268, 0.8757396449704142, 0.1242603550295858);
                        }
                    } else {
                        set_output2(var268, 1.0, 0.0);
                    }
                } else {
                    set_output2(var268, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var268, 0.0, 1.0);
        }
    } else {
        set_output2(var268, 0.0, 1.0);
    }
    add_vectors(var53, var268, 2, var52);
    double var269[2];
    if (input[9] <= 6.844593078462234) {
        if (input[9] <= 1.901013495682797) {
            if (input[9] <= 0.9222812494796209) {
                if (input[9] <= 0.49696151859992355) {
                    if (input[4] <= 10.545369178389057) {
                        if (input[7] <= 11.571897572157848) {
                            set_output2(var269, 0.0, 1.0);
                        } else {
                            set_output2(var269, 0.9045106169116723, 0.09548938308832769);
                        }
                    } else {
                        set_output2(var269, 1.0, 0.0);
                    }
                } else {
                    set_output2(var269, 0.0, 1.0);
                }
            } else {
                set_output2(var269, 0.0, 1.0);
            }
        } else {
            set_output2(var269, 0.0, 1.0);
        }
    } else {
        set_output2(var269, 0.0, 1.0);
    }
    add_vectors(var52, var269, 2, var51);
    double var270[2];
    if (input[4] <= 24.34210220238615) {
        if (input[1] <= 9.54177274427879) {
            if (input[3] <= -47.47816963228944) {
                if (input[3] <= -50.52033063442775) {
                    if (input[0] <= 37.229822116620554) {
                        if (input[6] <= 38.343331395052914) {
                            set_output2(var270, 0.9467337426943848, 0.05326625730561515);
                        } else {
                            set_output2(var270, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        set_output2(var270, 0.7272727272727273, 0.2727272727272727);
                    }
                } else {
                    if (input[1] <= 8.882395911096939) {
                        if (input[9] <= 0.7846443208417438) {
                            set_output2(var270, 0.8444401766852314, 0.15555982331476859);
                        } else {
                            set_output2(var270, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var270, 1.0, 0.0);
                    }
                }
            } else {
                if (input[8] <= 0.723850313278228) {
                    if (input[2] <= -14.266706446298706) {
                        set_output2(var270, 0.8608964451313755, 0.1391035548686244);
                    } else {
                        set_output2(var270, 1.0, 0.0);
                    }
                } else {
                    set_output2(var270, 0.9230769230769231, 0.07692307692307693);
                }
            }
        } else {
            if (input[9] <= 4.590510572702977) {
                if (input[9] <= 2.0853814932739136) {
                    if (input[9] <= 0.2721340657799647) {
                        if (input[9] <= 0.22909192406733747) {
                            set_output2(var270, 0.8901408450704226, 0.10985915492957747);
                        } else {
                            set_output2(var270, 0.7804878048780488, 0.21951219512195122);
                        }
                    } else {
                        if (input[1] <= 11.594342514515237) {
                            set_output2(var270, 0.6666666666666666, 0.3333333333333333);
                        } else {
                            set_output2(var270, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var270, 0.0, 1.0);
                }
            } else {
                set_output2(var270, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var270, 0.0, 1.0);
    }
    add_vectors(var51, var270, 2, var50);
    double var271[2];
    if (input[8] <= 1.1211803496729458) {
        if (input[7] <= 11.64176159369946) {
            set_output2(var271, 0.0, 1.0);
        } else {
            if (input[9] <= 1.3267765907202649) {
                if (input[9] <= 1.3000056280198444) {
                    if (input[0] <= 33.38837613602398) {
                        if (input[9] <= 0.2969129185285426) {
                            set_output2(var271, 0.8975912050976822, 0.10240879490231777);
                        } else {
                            set_output2(var271, 0.4, 0.6);
                        }
                    } else {
                        if (input[4] <= 2.176157140147665) {
                            set_output2(var271, 0.9212483593408196, 0.07875164065918039);
                        } else {
                            set_output2(var271, 0.9688581314878892, 0.031141868512110725);
                        }
                    }
                } else {
                    set_output2(var271, 0.0, 1.0);
                }
            } else {
                set_output2(var271, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var271, 0.0, 1.0);
    }
    add_vectors(var50, var271, 2, var49);
    double var272[2];
    if (input[9] <= 1.1700595622348502) {
        if (input[0] <= 35.722018759381555) {
            if (input[9] <= 0.7689412040224166) {
                if (input[7] <= 14.420348589009976) {
                    if (input[7] <= 12.206490697773923) {
                        set_output2(var272, 0.5714285714285714, 0.42857142857142855);
                    } else {
                        if (input[4] <= 6.302696889718126) {
                            set_output2(var272, 0.8723682775171812, 0.12763172248281882);
                        } else {
                            set_output2(var272, 0.9454545454545454, 0.05454545454545454);
                        }
                    }
                } else {
                    if (input[5] <= 0.1760363525397277) {
                        if (input[5] <= 0.10286073200313577) {
                            set_output2(var272, 0.9395017793594306, 0.060498220640569395);
                        } else {
                            set_output2(var272, 0.8860759493670886, 0.11392405063291139);
                        }
                    } else {
                        if (input[8] <= 0.6292718821337352) {
                            set_output2(var272, 0.7272727272727273, 0.2727272727272727);
                        } else {
                            set_output2(var272, 0.9072164948453608, 0.09278350515463918);
                        }
                    }
                }
            } else {
                set_output2(var272, 0.0, 1.0);
            }
        } else {
            if (input[9] <= 0.11263258766111531) {
                set_output2(var272, 0.9230769230769231, 0.07692307692307693);
            } else {
                set_output2(var272, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var272, 0.0, 1.0);
    }
    add_vectors(var49, var272, 2, var48);
    double var273[2];
    if (input[3] <= -41.79659324178182) {
        if (input[9] <= 8.438172761568225) {
            if (input[9] <= 3.8232014311396307) {
                if (input[7] <= 15.18094624188904) {
                    if (input[9] <= 1.1064739352170407) {
                        if (input[9] <= 0.7582480000884757) {
                            set_output2(var273, 0.8968455673493796, 0.10315443265062041);
                        } else {
                            set_output2(var273, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var273, 0.0, 1.0);
                    }
                } else {
                    if (input[3] <= -47.67844713146932) {
                        if (input[9] <= 0.13321220186594138) {
                            set_output2(var273, 0.9552238805970149, 0.04477611940298507);
                        } else {
                            set_output2(var273, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var273, 0.9142857142857143, 0.08571428571428572);
                    }
                }
            } else {
                set_output2(var273, 0.0, 1.0);
            }
        } else {
            set_output2(var273, 0.0, 1.0);
        }
    } else {
        if (input[7] <= 12.11818216333503) {
            set_output2(var273, 0.0, 1.0);
        } else {
            if (input[3] <= -34.86589453281417) {
                if (input[4] <= 19.653970880683733) {
                    if (input[9] <= 2.562941949113405) {
                        if (input[2] <= -5.803582835759467) {
                            set_output2(var273, 1.0, 0.0);
                        } else {
                            set_output2(var273, 0.8571428571428571, 0.14285714285714285);
                        }
                    } else {
                        set_output2(var273, 0.0, 1.0);
                    }
                } else {
                    set_output2(var273, 0.0, 1.0);
                }
            } else {
                set_output2(var273, 0.0, 1.0);
            }
        }
    }
    add_vectors(var48, var273, 2, var47);
    double var274[2];
    if (input[9] <= 6.413244324922075) {
        if (input[2] <= -19.57908393448131) {
            set_output2(var274, 1.0, 0.0);
        } else {
            if (input[9] <= 1.688003027530271) {
                if (input[9] <= 0.23412903376623462) {
                    if (input[1] <= 10.782288103032883) {
                        if (input[6] <= 51.75560474022027) {
                            set_output2(var274, 0.9180327868852459, 0.08196721311475409);
                        } else {
                            set_output2(var274, 0.7272727272727273, 0.2727272727272727);
                        }
                    } else {
                        if (input[7] <= 13.838368983767976) {
                            set_output2(var274, 0.8047337278106509, 0.1952662721893491);
                        } else {
                            set_output2(var274, 0.8985507246376812, 0.10144927536231885);
                        }
                    }
                } else {
                    if (input[1] <= 11.323217717500885) {
                        if (input[9] <= 0.579475360865376) {
                            set_output2(var274, 0.9142857142857143, 0.08571428571428572);
                        } else {
                            set_output2(var274, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var274, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var274, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var274, 0.0, 1.0);
    }
    add_vectors(var47, var274, 2, var46);
    double var275[2];
    if (input[3] <= -35.28454173795393) {
        if (input[2] <= -3.1376048828708) {
            if (input[2] <= -15.14534372488229) {
                set_output2(var275, 1.0, 0.0);
            } else {
                if (input[4] <= 14.822664827302173) {
                    if (input[7] <= 12.15394136390179) {
                        set_output2(var275, 0.4, 0.6);
                    } else {
                        if (input[9] <= 0.9085602472894067) {
                            set_output2(var275, 0.9012658227848102, 0.09873417721518987);
                        } else {
                            set_output2(var275, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var275, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var275, 0.0, 1.0);
        }
    } else {
        set_output2(var275, 0.0, 1.0);
    }
    add_vectors(var46, var275, 2, var45);
    double var276[2];
    if (input[9] <= 7.197581894721388) {
        if (input[9] <= 1.8123798855770228) {
            if (input[9] <= 1.1841544552243801) {
                if (input[9] <= 0.30121905055104736) {
                    if (input[7] <= 12.689671591995076) {
                        set_output2(var276, 0.6037735849056604, 0.39622641509433965);
                    } else {
                        if (input[3] <= -38.69883181697935) {
                            set_output2(var276, 0.9142747473024267, 0.08572525269757325);
                        } else {
                            set_output2(var276, 0.7272727272727273, 0.2727272727272727);
                        }
                    }
                } else {
                    set_output2(var276, 0.47058823529411764, 0.5294117647058824);
                }
            } else {
                set_output2(var276, 0.0, 1.0);
            }
        } else {
            set_output2(var276, 0.0, 1.0);
        }
    } else {
        set_output2(var276, 0.0, 1.0);
    }
    add_vectors(var45, var276, 2, var44);
    double var277[2];
    if (input[9] <= 10.163019453847506) {
        if (input[7] <= 13.719543456899864) {
            if (input[4] <= 5.832322188618895) {
                if (input[8] <= 0.7062814749577898) {
                    if (input[5] <= 0.03232745571136287) {
                        set_output2(var277, 0.0, 1.0);
                    } else {
                        if (input[9] <= 3.69977138219715) {
                            set_output2(var277, 0.6542008196721312, 0.3457991803278689);
                        } else {
                            set_output2(var277, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var277, 0.8615384615384616, 0.13846153846153847);
                }
            } else {
                set_output2(var277, 0.898876404494382, 0.10112359550561797);
            }
        } else {
            if (input[4] <= 8.258827357336312) {
                if (input[9] <= 6.194182803554482) {
                    if (input[9] <= 4.616119489910331) {
                        if (input[9] <= 0.33595272165460804) {
                            set_output2(var277, 0.9215617918772878, 0.07843820812271222);
                        } else {
                            set_output2(var277, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var277, 0.0, 1.0);
                    }
                } else {
                    set_output2(var277, 0.0, 1.0);
                }
            } else {
                set_output2(var277, 0.7567567567567568, 0.24324324324324326);
            }
        }
    } else {
        set_output2(var277, 0.0, 1.0);
    }
    add_vectors(var44, var277, 2, var43);
    double var278[2];
    if (input[9] <= 7.508154957959542) {
        if (input[4] <= 19.849239743528184) {
            if (input[1] <= 12.24853617537584) {
                if (input[9] <= 0.20161468249612452) {
                    if (input[3] <= -38.98335604134439) {
                        if (input[1] <= 10.523581850124385) {
                            set_output2(var278, 0.9225273306361367, 0.07747266936386331);
                        } else {
                            set_output2(var278, 0.8941176470588236, 0.10588235294117647);
                        }
                    } else {
                        set_output2(var278, 0.47058823529411764, 0.5294117647058824);
                    }
                } else {
                    if (input[7] <= 13.861764477806966) {
                        set_output2(var278, 0.42105263157894735, 0.5789473684210527);
                    } else {
                        if (input[9] <= 2.788464727550094) {
                            set_output2(var278, 0.8846153846153846, 0.11538461538461539);
                        } else {
                            set_output2(var278, 0.0, 1.0);
                        }
                    }
                }
            } else {
                set_output2(var278, 0.43243243243243246, 0.5675675675675675);
            }
        } else {
            set_output2(var278, 0.0, 1.0);
        }
    } else {
        set_output2(var278, 0.0, 1.0);
    }
    add_vectors(var43, var278, 2, var42);
    double var279[2];
    if (input[8] <= 1.260629762748655) {
        if (input[9] <= 3.912913745083307) {
            if (input[2] <= -3.420210096586306) {
                if (input[9] <= 2.3232744252686057) {
                    if (input[9] <= 0.856512171750732) {
                        if (input[9] <= 0.42435935475124265) {
                            set_output2(var279, 0.9052774018944519, 0.09472259810554803);
                        } else {
                            set_output2(var279, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var279, 0.0, 1.0);
                    }
                } else {
                    set_output2(var279, 0.0, 1.0);
                }
            } else {
                set_output2(var279, 0.4, 0.6);
            }
        } else {
            set_output2(var279, 0.0, 1.0);
        }
    } else {
        set_output2(var279, 0.0, 1.0);
    }
    add_vectors(var42, var279, 2, var41);
    double var280[2];
    if (input[9] <= 11.446821086779225) {
        if (input[9] <= 4.612078045030073) {
            if (input[9] <= 0.7171709914875519) {
                if (input[9] <= 0.3776502505271475) {
                    if (input[1] <= 5.961321654608456) {
                        if (input[9] <= 0.05208434719085386) {
                            set_output2(var280, 0.8909619578386236, 0.1090380421613763);
                        } else {
                            set_output2(var280, 0.9837074583635047, 0.016292541636495295);
                        }
                    } else {
                        if (input[0] <= 34.06554790430006) {
                            set_output2(var280, 0.8912579957356077, 0.10874200426439233);
                        } else {
                            set_output2(var280, 0.9626556016597511, 0.03734439834024896);
                        }
                    }
                } else {
                    set_output2(var280, 0.4, 0.6);
                }
            } else {
                set_output2(var280, 0.0, 1.0);
            }
        } else {
            set_output2(var280, 0.0, 1.0);
        }
    } else {
        set_output2(var280, 0.0, 1.0);
    }
    add_vectors(var41, var280, 2, var40);
    double var281[2];
    if (input[9] <= 9.281660598939496) {
        if (input[3] <= -37.50601828193473) {
            if (input[7] <= 12.994242846264386) {
                set_output2(var281, 0.6956521739130435, 0.30434782608695654);
            } else {
                if (input[9] <= 7.211442007707779) {
                    if (input[9] <= 5.106475196056178) {
                        if (input[9] <= 2.6495137263373305) {
                            set_output2(var281, 0.898081783154493, 0.10191821684550689);
                        } else {
                            set_output2(var281, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var281, 0.0, 1.0);
                    }
                } else {
                    set_output2(var281, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var281, 0.0, 1.0);
        }
    } else {
        set_output2(var281, 0.0, 1.0);
    }
    add_vectors(var40, var281, 2, var39);
    double var282[2];
    if (input[9] <= 7.057437431567441) {
        if (input[4] <= 16.35947222708049) {
            if (input[7] <= 12.277730734658158) {
                set_output2(var282, 0.5714285714285714, 0.42857142857142855);
            } else {
                if (input[9] <= 3.5161100476294864) {
                    if (input[9] <= 2.1984261154865723) {
                        if (input[9] <= 1.1634247001621085) {
                            set_output2(var282, 0.9016272492519573, 0.09837275074804279);
                        } else {
                            set_output2(var282, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var282, 0.0, 1.0);
                    }
                } else {
                    set_output2(var282, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var282, 0.0, 1.0);
        }
    } else {
        set_output2(var282, 0.0, 1.0);
    }
    add_vectors(var39, var282, 2, var38);
    double var283[2];
    if (input[9] <= 5.811819706502791) {
        if (input[1] <= 13.424423896051794) {
            if (input[9] <= 2.3717255873975955) {
                if (input[9] <= 1.8653099183377178) {
                    if (input[9] <= 1.2749816846378994) {
                        if (input[2] <= -17.827754043079736) {
                            set_output2(var283, 1.0, 0.0);
                        } else {
                            set_output2(var283, 0.8944099378881988, 0.10559006211180125);
                        }
                    } else {
                        set_output2(var283, 0.0, 1.0);
                    }
                } else {
                    set_output2(var283, 0.0, 1.0);
                }
            } else {
                set_output2(var283, 0.0, 1.0);
            }
        } else {
            set_output2(var283, 0.0, 1.0);
        }
    } else {
        set_output2(var283, 0.0, 1.0);
    }
    add_vectors(var38, var283, 2, var37);
    double var284[2];
    if (input[3] <= -33.11370834999205) {
        if (input[9] <= 11.998575725560173) {
            if (input[9] <= 1.8013352288353792) {
                if (input[4] <= 16.91682368228309) {
                    if (input[9] <= 0.6888189398855322) {
                        if (input[9] <= 0.4347313691897163) {
                            set_output2(var284, 0.9055871173417256, 0.09441288265827441);
                        } else {
                            set_output2(var284, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var284, 0.0, 1.0);
                    }
                } else {
                    set_output2(var284, 0.0, 1.0);
                }
            } else {
                set_output2(var284, 0.0, 1.0);
            }
        } else {
            set_output2(var284, 0.0, 1.0);
        }
    } else {
        set_output2(var284, 0.0, 1.0);
    }
    add_vectors(var37, var284, 2, var36);
    double var285[2];
    if (input[2] <= -0.7963198753014566) {
        if (input[9] <= 0.6121462576671217) {
            if (input[1] <= 9.575915633224088) {
                if (input[1] <= 8.731441734788284) {
                    if (input[4] <= 3.466671064125347) {
                        if (input[4] <= 3.0486080101075475) {
                            set_output2(var285, 0.886197840676977, 0.11380215932302307);
                        } else {
                            set_output2(var285, 0.5714285714285714, 0.42857142857142855);
                        }
                    } else {
                        if (input[5] <= 0.1089627875040619) {
                            set_output2(var285, 0.9863013698630136, 0.0136986301369863);
                        } else {
                            set_output2(var285, 0.898876404494382, 0.10112359550561797);
                        }
                    }
                } else {
                    if (input[7] <= 13.20474772408068) {
                        set_output2(var285, 0.7272727272727273, 0.2727272727272727);
                    } else {
                        set_output2(var285, 0.9887640449438202, 0.011235955056179775);
                    }
                }
            } else {
                if (input[9] <= 0.4528019070609185) {
                    if (input[7] <= 12.0035106840156) {
                        set_output2(var285, 0.0, 1.0);
                    } else {
                        if (input[1] <= 12.407883415486658) {
                            set_output2(var285, 0.894878706199461, 0.10512129380053908);
                        } else {
                            set_output2(var285, 0.7272727272727273, 0.2727272727272727);
                        }
                    }
                } else {
                    set_output2(var285, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var285, 0.0, 1.0);
        }
    } else {
        set_output2(var285, 0.0, 1.0);
    }
    add_vectors(var36, var285, 2, var35);
    double var286[2];
    if (input[9] <= 7.221933467535513) {
        if (input[9] <= 4.746041835544071) {
            if (input[4] <= 14.99468422996303) {
                if (input[9] <= 0.4070080179610077) {
                    if (input[1] <= 6.784741688883921) {
                        if (input[8] <= 0.588314005746507) {
                            set_output2(var286, 1.0, 0.0);
                        } else {
                            set_output2(var286, 0.9264004361455636, 0.07359956385443642);
                        }
                    } else {
                        if (input[5] <= 0.3223587602948839) {
                            set_output2(var286, 0.9003470500743679, 0.09965294992563213);
                        } else {
                            set_output2(var286, 0.7272727272727273, 0.2727272727272727);
                        }
                    }
                } else {
                    if (input[9] <= 1.9960566951016645) {
                        set_output2(var286, 0.25, 0.75);
                    } else {
                        set_output2(var286, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var286, 0.0, 1.0);
            }
        } else {
            set_output2(var286, 0.0, 1.0);
        }
    } else {
        set_output2(var286, 0.0, 1.0);
    }
    add_vectors(var35, var286, 2, var34);
    double var287[2];
    if (input[9] <= 7.296989010840181) {
        if (input[9] <= 1.3606271449196554) {
            if (input[7] <= 13.847889319364793) {
                if (input[9] <= 0.2993654225953205) {
                    if (input[8] <= 0.720849256651804) {
                        if (input[4] <= 4.543489654579548) {
                            set_output2(var287, 0.7582378223495702, 0.24176217765042982);
                        } else {
                            set_output2(var287, 1.0, 0.0);
                        }
                    } else {
                        if (input[7] <= 12.54404051237828) {
                            set_output2(var287, 0.6666666666666666, 0.3333333333333333);
                        } else {
                            set_output2(var287, 1.0, 0.0);
                        }
                    }
                } else {
                    set_output2(var287, 0.4, 0.6);
                }
            } else {
                if (input[9] <= 1.325508968889651) {
                    if (input[9] <= 0.49038212280823423) {
                        if (input[0] <= 30.92964401051641) {
                            set_output2(var287, 0.9467455621301775, 0.05325443786982249);
                        } else {
                            set_output2(var287, 0.9042258850237662, 0.09577411497623384);
                        }
                    } else {
                        set_output2(var287, 0.0, 1.0);
                    }
                } else {
                    set_output2(var287, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var287, 0.0, 1.0);
        }
    } else {
        set_output2(var287, 0.0, 1.0);
    }
    add_vectors(var34, var287, 2, var33);
    double var288[2];
    if (input[1] <= 12.605335721755067) {
        if (input[2] <= -2.446616996034745) {
            if (input[3] <= -38.06783638796806) {
                if (input[7] <= 14.519087542061282) {
                    if (input[9] <= 3.292378245871938) {
                        if (input[7] <= 12.834481118973612) {
                            set_output2(var288, 0.6037735849056604, 0.39622641509433965);
                        } else {
                            set_output2(var288, 0.8853979886177594, 0.11460201138224059);
                        }
                    } else {
                        set_output2(var288, 0.0, 1.0);
                    }
                } else {
                    if (input[5] <= 0.10339241842830728) {
                        if (input[3] <= -44.85550851573672) {
                            set_output2(var288, 0.9485179407176287, 0.0514820592823713);
                        } else {
                            set_output2(var288, 0.8767123287671232, 0.1232876712328767);
                        }
                    } else {
                        if (input[5] <= 0.40556677313419554) {
                            set_output2(var288, 0.8888888888888888, 0.1111111111111111);
                        } else {
                            set_output2(var288, 0.5714285714285714, 0.42857142857142855);
                        }
                    }
                }
            } else {
                set_output2(var288, 0.0, 1.0);
            }
        } else {
            set_output2(var288, 0.0, 1.0);
        }
    } else {
        set_output2(var288, 0.34782608695652173, 0.6521739130434783);
    }
    add_vectors(var33, var288, 2, var32);
    double var289[2];
    if (input[9] <= 1.3656486680214532) {
        if (input[7] <= 13.138929100166735) {
            if (input[1] <= 8.805643518519) {
                set_output2(var289, 0.8888888888888888, 0.1111111111111111);
            } else {
                set_output2(var289, 0.6557377049180327, 0.3442622950819672);
            }
        } else {
            if (input[3] <= -37.37692757075671) {
                if (input[9] <= 0.744651798061921) {
                    if (input[1] <= 7.514964579021276) {
                        if (input[9] <= 0.09816974619966222) {
                            set_output2(var289, 0.9276194852941176, 0.07238051470588236);
                        } else {
                            set_output2(var289, 1.0, 0.0);
                        }
                    } else {
                        if (input[1] <= 8.301055763651444) {
                            set_output2(var289, 0.8057553956834532, 0.19424460431654678);
                        } else {
                            set_output2(var289, 0.9066666666666666, 0.09333333333333334);
                        }
                    }
                } else {
                    set_output2(var289, 0.0, 1.0);
                }
            } else {
                set_output2(var289, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var289, 0.0, 1.0);
    }
    add_vectors(var32, var289, 2, var31);
    double var290[2];
    if (input[2] <= -3.3024719785035366) {
        if (input[9] <= 1.4637287933888525) {
            if (input[9] <= 0.659015749144488) {
                if (input[7] <= 12.176925490538348) {
                    set_output2(var290, 0.4, 0.6);
                } else {
                    if (input[9] <= 0.462636712879001) {
                        if (input[7] <= 12.812397658327923) {
                            set_output2(var290, 0.7272727272727273, 0.2727272727272727);
                        } else {
                            set_output2(var290, 0.9119755208115019, 0.08802447918849814);
                        }
                    } else {
                        set_output2(var290, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var290, 0.0, 1.0);
            }
        } else {
            set_output2(var290, 0.0, 1.0);
        }
    } else {
        set_output2(var290, 0.1951219512195122, 0.8048780487804879);
    }
    add_vectors(var31, var290, 2, var30);
    double var291[2];
    if (input[3] <= -39.70872160908915) {
        if (input[7] <= 12.573125352580986) {
            set_output2(var291, 0.6666666666666666, 0.3333333333333333);
        } else {
            if (input[9] <= 5.911006272534622) {
                if (input[9] <= 3.910936716181283) {
                    if (input[9] <= 1.0154596493904577) {
                        if (input[9] <= 0.8447815248436334) {
                            set_output2(var291, 0.9087722262110909, 0.09122777378890908);
                        } else {
                            set_output2(var291, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var291, 0.0, 1.0);
                    }
                } else {
                    set_output2(var291, 0.0, 1.0);
                }
            } else {
                set_output2(var291, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var291, 0.5714285714285714, 0.42857142857142855);
    }
    add_vectors(var30, var291, 2, var29);
    double var292[2];
    if (input[9] <= 2.384319466323523) {
        if (input[1] <= 12.608527384855801) {
            if (input[7] <= 12.124555563304934) {
                set_output2(var292, 0.0, 1.0);
            } else {
                if (input[9] <= 0.6227376941068313) {
                    if (input[9] <= 0.43813724180590563) {
                        if (input[1] <= 7.530659628043471) {
                            set_output2(var292, 0.9463926736654009, 0.053607326334599054);
                        } else {
                            set_output2(var292, 0.9014653865588681, 0.09853461344113189);
                        }
                    } else {
                        set_output2(var292, 0.0, 1.0);
                    }
                } else {
                    set_output2(var292, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var292, 0.4, 0.6);
        }
    } else {
        set_output2(var292, 0.0, 1.0);
    }
    add_vectors(var29, var292, 2, var28);
    double var293[2];
    if (input[9] <= 8.702089391445051) {
        if (input[7] <= 13.69310679592665) {
            if (input[4] <= 2.0577338924284243) {
                if (input[1] <= 7.512461529354221) {
                    set_output2(var293, 0.7788697788697789, 0.22113022113022113);
                } else {
                    if (input[9] <= 0.8130179536938825) {
                        if (input[2] <= -4.6295588482412615) {
                            set_output2(var293, 0.25, 0.75);
                        } else {
                            set_output2(var293, 0.6153846153846154, 0.38461538461538464);
                        }
                    } else {
                        set_output2(var293, 0.0, 1.0);
                    }
                }
            } else {
                if (input[9] <= 0.9444346005502608) {
                    if (input[4] <= 2.343044034889584) {
                        set_output2(var293, 1.0, 0.0);
                    } else {
                        if (input[7] <= 12.087750249061978) {
                            set_output2(var293, 0.0, 1.0);
                        } else {
                            set_output2(var293, 0.9051883065578088, 0.0948116934421912);
                        }
                    }
                } else {
                    set_output2(var293, 0.0, 1.0);
                }
            }
        } else {
            if (input[9] <= 2.4038355213138565) {
                if (input[9] <= 1.2547380042121081) {
                    if (input[9] <= 0.426154618018114) {
                        if (input[9] <= 0.20734374474894365) {
                            set_output2(var293, 0.9150625640691866, 0.08493743593081342);
                        } else {
                            set_output2(var293, 0.9806451612903225, 0.01935483870967742);
                        }
                    } else {
                        set_output2(var293, 0.0, 1.0);
                    }
                } else {
                    set_output2(var293, 0.0, 1.0);
                }
            } else {
                set_output2(var293, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var293, 0.0, 1.0);
    }
    add_vectors(var28, var293, 2, var27);
    double var294[2];
    if (input[9] <= 1.689829569639823) {
        if (input[1] <= 7.542559990330293) {
            if (input[6] <= 70.78144690101067) {
                if (input[9] <= 0.02982855711504808) {
                    if (input[8] <= 0.5716807197227963) {
                        set_output2(var294, 1.0, 0.0);
                    } else {
                        if (input[6] <= 25.16795836401787) {
                            set_output2(var294, 0.737481769567331, 0.262518230432669);
                        } else {
                            set_output2(var294, 1.0, 0.0);
                        }
                    }
                } else {
                    if (input[9] <= 0.06942887292846753) {
                        if (input[1] <= 5.943582067196201) {
                            set_output2(var294, 1.0, 0.0);
                        } else {
                            set_output2(var294, 0.8888888888888888, 0.1111111111111111);
                        }
                    } else {
                        set_output2(var294, 1.0, 0.0);
                    }
                }
            } else {
                set_output2(var294, 0.7272727272727273, 0.2727272727272727);
            }
        } else {
            if (input[9] <= 0.6040457950151003) {
                if (input[0] <= 30.923172727702486) {
                    if (input[6] <= 10.071610760706811) {
                        if (input[1] <= 10.898741644622298) {
                            set_output2(var294, 0.986784140969163, 0.013215859030837005);
                        } else {
                            set_output2(var294, 0.9320388349514563, 0.06796116504854369);
                        }
                    } else {
                        if (input[1] <= 10.206850833788495) {
                            set_output2(var294, 0.9230769230769231, 0.07692307692307693);
                        } else {
                            set_output2(var294, 0.7272727272727273, 0.2727272727272727);
                        }
                    }
                } else {
                    if (input[9] <= 0.48668060384451634) {
                        if (input[4] <= 10.981452094191296) {
                            set_output2(var294, 0.8798114689709348, 0.1201885310290652);
                        } else {
                            set_output2(var294, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var294, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var294, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var294, 0.0, 1.0);
    }
    add_vectors(var27, var294, 2, var26);
    double var295[2];
    if (input[9] <= 3.22114932662451) {
        if (input[9] <= 2.55519616218515) {
            if (input[9] <= 0.3543346452900889) {
                if (input[7] <= 13.91237191410823) {
                    if (input[9] <= 0.22849802011066678) {
                        if (input[4] <= 7.321570831616329) {
                            set_output2(var295, 0.8518016272762495, 0.1481983727237505);
                        } else {
                            set_output2(var295, 0.9411764705882353, 0.058823529411764705);
                        }
                    } else {
                        set_output2(var295, 0.0, 1.0);
                    }
                } else {
                    if (input[1] <= 10.108803041449317) {
                        if (input[0] <= 28.8808726335811) {
                            set_output2(var295, 1.0, 0.0);
                        } else {
                            set_output2(var295, 0.9298295353864692, 0.07017046461353085);
                        }
                    } else {
                        if (input[5] <= 0.271866169373093) {
                            set_output2(var295, 0.9074626865671642, 0.09253731343283582);
                        } else {
                            set_output2(var295, 0.7272727272727273, 0.2727272727272727);
                        }
                    }
                }
            } else {
                if (input[1] <= 9.575820568369474) {
                    set_output2(var295, 0.7272727272727273, 0.2727272727272727);
                } else {
                    set_output2(var295, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var295, 0.0, 1.0);
        }
    } else {
        set_output2(var295, 0.0, 1.0);
    }
    add_vectors(var26, var295, 2, var25);
    double var296[2];
    if (input[9] <= 4.073183740308876) {
        if (input[9] <= 1.295272029152631) {
            if (input[9] <= 0.7955238052688273) {
                if (input[9] <= 0.3968795394782004) {
                    if (input[7] <= 12.607756369790494) {
                        set_output2(var296, 0.6037735849056604, 0.39622641509433965);
                    } else {
                        if (input[2] <= -9.97355023061693) {
                            set_output2(var296, 0.9448384554767534, 0.055161544523246654);
                        } else {
                            set_output2(var296, 0.9057356608478803, 0.0942643391521197);
                        }
                    }
                } else {
                    set_output2(var296, 0.4, 0.6);
                }
            } else {
                set_output2(var296, 0.0, 1.0);
            }
        } else {
            set_output2(var296, 0.0, 1.0);
        }
    } else {
        set_output2(var296, 0.0, 1.0);
    }
    add_vectors(var25, var296, 2, var24);
    double var297[2];
    if (input[7] <= 13.771428862247843) {
        if (input[9] <= 2.337664917011648) {
            if (input[9] <= 0.19918580104161163) {
                if (input[7] <= 12.544571882451283) {
                    set_output2(var297, 0.5333333333333333, 0.4666666666666667);
                } else {
                    if (input[6] <= 6.277202845705743) {
                        if (input[4] <= 1.1148636643296816) {
                            set_output2(var297, 0.47058823529411764, 0.5294117647058824);
                        } else {
                            set_output2(var297, 0.8739495798319328, 0.12605042016806722);
                        }
                    } else {
                        set_output2(var297, 1.0, 0.0);
                    }
                }
            } else {
                set_output2(var297, 0.5, 0.5);
            }
        } else {
            set_output2(var297, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 8.130343058830057) {
            if (input[5] <= 0.15703604873044708) {
                if (input[9] <= 4.500064548017343) {
                    if (input[8] <= 1.5849360791945744) {
                        if (input[5] <= 0.04602281517734382) {
                            set_output2(var297, 0.625, 0.375);
                        } else {
                            set_output2(var297, 0.9205746862700107, 0.07942531372998923);
                        }
                    } else {
                        set_output2(var297, 0.0, 1.0);
                    }
                } else {
                    set_output2(var297, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 1.5374132075021372) {
                    if (input[9] <= 0.5624500291925226) {
                        if (input[8] <= 0.7311644546109752) {
                            set_output2(var297, 0.8543689320388349, 0.14563106796116504);
                        } else {
                            set_output2(var297, 0.9361702127659575, 0.06382978723404255);
                        }
                    } else {
                        set_output2(var297, 0.0, 1.0);
                    }
                } else {
                    set_output2(var297, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var297, 0.0, 1.0);
        }
    }
    add_vectors(var24, var297, 2, var23);
    double var298[2];
    if (input[9] <= 11.144896728637784) {
        if (input[9] <= 2.804087460915572) {
            if (input[9] <= 1.0189289656003324) {
                if (input[9] <= 0.31833085021072655) {
                    if (input[7] <= 12.658251314143572) {
                        set_output2(var298, 0.6037735849056604, 0.39622641509433965);
                    } else {
                        if (input[2] <= -19.840722139887525) {
                            set_output2(var298, 1.0, 0.0);
                        } else {
                            set_output2(var298, 0.909248055315471, 0.09075194468452895);
                        }
                    }
                } else {
                    set_output2(var298, 0.5333333333333333, 0.4666666666666667);
                }
            } else {
                set_output2(var298, 0.0, 1.0);
            }
        } else {
            set_output2(var298, 0.0, 1.0);
        }
    } else {
        set_output2(var298, 0.0, 1.0);
    }
    add_vectors(var23, var298, 2, var22);
    double var299[2];
    if (input[7] <= 12.07588607150626) {
        set_output2(var299, 0.0, 1.0);
    } else {
        if (input[3] <= -34.52109768250276) {
            if (input[9] <= 7.9628900219191205) {
                if (input[2] <= -1.8047691129913979) {
                    if (input[9] <= 2.347195051614841) {
                        if (input[9] <= 0.8367838786734393) {
                            set_output2(var299, 0.9055871173417256, 0.09441288265827441);
                        } else {
                            set_output2(var299, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var299, 0.0, 1.0);
                    }
                } else {
                    set_output2(var299, 0.0, 1.0);
                }
            } else {
                set_output2(var299, 0.0, 1.0);
            }
        } else {
            set_output2(var299, 0.0, 1.0);
        }
    }
    add_vectors(var22, var299, 2, var21);
    double var300[2];
    if (input[9] <= 1.3929397030790638) {
        if (input[9] <= 0.5285744371777739) {
            if (input[3] <= -40.57276037837468) {
                if (input[4] <= 5.071910619449123) {
                    if (input[7] <= 12.596469407685458) {
                        set_output2(var300, 0.64, 0.36);
                    } else {
                        if (input[7] <= 13.735329879792493) {
                            set_output2(var300, 0.7367236723672367, 0.26327632763276326);
                        } else {
                            set_output2(var300, 0.9095902708017184, 0.09040972919828158);
                        }
                    }
                } else {
                    if (input[3] <= -47.3359934470632) {
                        if (input[0] <= 35.59369146319385) {
                            set_output2(var300, 0.923943661971831, 0.07605633802816901);
                        } else {
                            set_output2(var300, 1.0, 0.0);
                        }
                    } else {
                        if (input[0] <= 29.959161918652836) {
                            set_output2(var300, 0.9411764705882353, 0.058823529411764705);
                        } else {
                            set_output2(var300, 1.0, 0.0);
                        }
                    }
                }
            } else {
                set_output2(var300, 0.8, 0.2);
            }
        } else {
            set_output2(var300, 0.0, 1.0);
        }
    } else {
        set_output2(var300, 0.0, 1.0);
    }
    add_vectors(var21, var300, 2, var20);
    double var301[2];
    if (input[9] <= 8.575987972063349) {
        if (input[9] <= 4.524650807190886) {
            if (input[9] <= 1.4708896405874534) {
                if (input[9] <= 1.221566002806859) {
                    if (input[9] <= 0.29498671149571265) {
                        if (input[3] <= -38.56073356761192) {
                            set_output2(var301, 0.9080250238223474, 0.09197497617765256);
                        } else {
                            set_output2(var301, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var301, 0.47058823529411764, 0.5294117647058824);
                    }
                } else {
                    set_output2(var301, 0.0, 1.0);
                }
            } else {
                set_output2(var301, 0.0, 1.0);
            }
        } else {
            set_output2(var301, 0.0, 1.0);
        }
    } else {
        set_output2(var301, 0.0, 1.0);
    }
    add_vectors(var20, var301, 2, var19);
    double var302[2];
    if (input[9] <= 6.406645745865954) {
        if (input[9] <= 3.722264494273757) {
            if (input[3] <= -36.35415055026232) {
                if (input[9] <= 2.751491420130212) {
                    if (input[3] <= -38.12765105010169) {
                        if (input[9] <= 0.620891884599274) {
                            set_output2(var302, 0.9044780993591052, 0.0955219006408948);
                        } else {
                            set_output2(var302, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var302, 0.0, 1.0);
                    }
                } else {
                    set_output2(var302, 0.0, 1.0);
                }
            } else {
                set_output2(var302, 0.0, 1.0);
            }
        } else {
            set_output2(var302, 0.0, 1.0);
        }
    } else {
        set_output2(var302, 0.0, 1.0);
    }
    add_vectors(var19, var302, 2, var18);
    double var303[2];
    if (input[9] <= 9.853848515856008) {
        if (input[9] <= 5.049021034524154) {
            if (input[9] <= 0.9925718321467193) {
                if (input[9] <= 0.49682245641927864) {
                    if (input[9] <= 0.1905602812208957) {
                        if (input[7] <= 12.480003569662466) {
                            set_output2(var303, 0.5333333333333333, 0.4666666666666667);
                        } else {
                            set_output2(var303, 0.9080327633280644, 0.09196723667193563);
                        }
                    } else {
                        if (input[1] <= 9.538153170453668) {
                            set_output2(var303, 1.0, 0.0);
                        } else {
                            set_output2(var303, 0.9142857142857143, 0.08571428571428572);
                        }
                    }
                } else {
                    set_output2(var303, 0.0, 1.0);
                }
            } else {
                set_output2(var303, 0.0, 1.0);
            }
        } else {
            set_output2(var303, 0.0, 1.0);
        }
    } else {
        set_output2(var303, 0.0, 1.0);
    }
    add_vectors(var18, var303, 2, var17);
    double var304[2];
    if (input[8] <= 1.5414220438217268) {
        if (input[9] <= 3.287444018849203) {
            if (input[9] <= 1.6764972182625641) {
                if (input[7] <= 12.951218216238253) {
                    set_output2(var304, 0.6956521739130435, 0.30434782608695654);
                } else {
                    if (input[9] <= 0.3724063338230986) {
                        if (input[1] <= 7.40935649511133) {
                            set_output2(var304, 0.9510146955913226, 0.0489853044086774);
                        } else {
                            set_output2(var304, 0.9030271934325295, 0.0969728065674705);
                        }
                    } else {
                        set_output2(var304, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var304, 0.0, 1.0);
            }
        } else {
            set_output2(var304, 0.0, 1.0);
        }
    } else {
        set_output2(var304, 0.0, 1.0);
    }
    add_vectors(var17, var304, 2, var16);
    double var305[2];
    if (input[9] <= 8.048192609766453) {
        if (input[9] <= 6.007084734374693) {
            if (input[9] <= 2.6017687364257274) {
                if (input[9] <= 1.0745120844688714) {
                    if (input[9] <= 0.668222649340009) {
                        if (input[9] <= 0.5155456290908017) {
                            set_output2(var305, 0.9044780993591052, 0.0955219006408948);
                        } else {
                            set_output2(var305, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var305, 0.0, 1.0);
                    }
                } else {
                    set_output2(var305, 0.0, 1.0);
                }
            } else {
                set_output2(var305, 0.0, 1.0);
            }
        } else {
            set_output2(var305, 0.0, 1.0);
        }
    } else {
        set_output2(var305, 0.0, 1.0);
    }
    add_vectors(var16, var305, 2, var15);
    double var306[2];
    if (input[9] <= 4.742447519649732) {
        if (input[3] <= -37.666258040961324) {
            if (input[8] <= 1.014428256560226) {
                if (input[9] <= 1.4391242167903573) {
                    if (input[7] <= 13.418555401754462) {
                        if (input[4] <= 6.998780765797261) {
                            set_output2(var306, 0.7206546275395034, 0.2793453724604966);
                        } else {
                            set_output2(var306, 0.8888888888888888, 0.1111111111111111);
                        }
                    } else {
                        if (input[7] <= 13.74505269971) {
                            set_output2(var306, 0.8343558282208589, 0.1656441717791411);
                        } else {
                            set_output2(var306, 0.9121570328597766, 0.08784296714022341);
                        }
                    }
                } else {
                    set_output2(var306, 0.0, 1.0);
                }
            } else {
                set_output2(var306, 0.5714285714285714, 0.42857142857142855);
            }
        } else {
            set_output2(var306, 0.0, 1.0);
        }
    } else {
        set_output2(var306, 0.0, 1.0);
    }
    add_vectors(var15, var306, 2, var14);
    double var307[2];
    if (input[3] <= -41.991514868482824) {
        if (input[9] <= 1.6752686735605515) {
            if (input[7] <= 12.864457951606399) {
                set_output2(var307, 0.64, 0.36);
            } else {
                if (input[7] <= 15.210703013513431) {
                    if (input[9] <= 0.48788270934867184) {
                        if (input[3] <= -53.279903753633505) {
                            set_output2(var307, 0.8615384615384616, 0.13846153846153847);
                        } else {
                            set_output2(var307, 0.9092792742341939, 0.09072072576580613);
                        }
                    } else {
                        set_output2(var307, 0.0, 1.0);
                    }
                } else {
                    if (input[2] <= -4.212198765201039) {
                        if (input[0] <= 26.742886204706934) {
                            set_output2(var307, 0.7272727272727273, 0.2727272727272727);
                        } else {
                            set_output2(var307, 0.9872340425531915, 0.01276595744680851);
                        }
                    } else {
                        set_output2(var307, 0.8695652173913043, 0.13043478260869565);
                    }
                }
            }
        } else {
            set_output2(var307, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 8.512919257958037) {
            if (input[8] <= 0.5374694304563488) {
                set_output2(var307, 0.0, 1.0);
            } else {
                if (input[9] <= 1.2291260686841416) {
                    if (input[5] <= 0.28844095679452386) {
                        if (input[3] <= -40.60918496181688) {
                            set_output2(var307, 0.9411764705882353, 0.058823529411764705);
                        } else {
                            set_output2(var307, 0.8571428571428571, 0.14285714285714285);
                        }
                    } else {
                        set_output2(var307, 0.5714285714285714, 0.42857142857142855);
                    }
                } else {
                    set_output2(var307, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var307, 0.0, 1.0);
        }
    }
    add_vectors(var14, var307, 2, var13);
    double var308[2];
    if (input[2] <= -3.464125186847639) {
        if (input[7] <= 12.495643283927098) {
            set_output2(var308, 0.5333333333333333, 0.4666666666666667);
        } else {
            if (input[8] <= 1.2424068023897112) {
                if (input[9] <= 1.7899609511333296) {
                    if (input[1] <= 5.440188476136558) {
                        if (input[3] <= -51.869194490777296) {
                            set_output2(var308, 0.9491525423728814, 0.05084745762711865);
                        } else {
                            set_output2(var308, 1.0, 0.0);
                        }
                    } else {
                        if (input[9] <= 0.24626374845732538) {
                            set_output2(var308, 0.9069745915675476, 0.09302540843245244);
                        } else {
                            set_output2(var308, 0.6597938144329897, 0.3402061855670103);
                        }
                    }
                } else {
                    set_output2(var308, 0.0, 1.0);
                }
            } else {
                set_output2(var308, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var308, 0.3076923076923077, 0.6923076923076923);
    }
    add_vectors(var13, var308, 2, var12);
    double var309[2];
    if (input[9] <= 8.489052687144742) {
        if (input[3] <= -35.43049618174912) {
            if (input[4] <= 17.142272714718352) {
                if (input[9] <= 3.28467326085598) {
                    if (input[9] <= 1.8394777665364823) {
                        if (input[7] <= 12.017715141435968) {
                            set_output2(var309, 0.0, 1.0);
                        } else {
                            set_output2(var309, 0.9000690579680708, 0.09993094203192916);
                        }
                    } else {
                        set_output2(var309, 0.0, 1.0);
                    }
                } else {
                    set_output2(var309, 0.0, 1.0);
                }
            } else {
                set_output2(var309, 0.0, 1.0);
            }
        } else {
            set_output2(var309, 0.0, 1.0);
        }
    } else {
        set_output2(var309, 0.0, 1.0);
    }
    add_vectors(var12, var309, 2, var11);
    double var310[2];
    if (input[8] <= 1.677270997579502) {
        if (input[9] <= 8.724096193351766) {
            if (input[7] <= 11.422914459839657) {
                set_output2(var310, 0.0, 1.0);
            } else {
                if (input[3] <= -34.041438937585326) {
                    if (input[9] <= 2.4722740241742716) {
                        if (input[9] <= 0.5415539853477038) {
                            set_output2(var310, 0.9055871173417256, 0.09441288265827441);
                        } else {
                            set_output2(var310, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var310, 0.0, 1.0);
                    }
                } else {
                    set_output2(var310, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var310, 0.0, 1.0);
        }
    } else {
        set_output2(var310, 0.0, 1.0);
    }
    add_vectors(var11, var310, 2, var10);
    double var311[2];
    if (input[9] <= 1.660653203640146) {
        if (input[9] <= 1.085516534242943) {
            if (input[0] <= 33.46442792794544) {
                if (input[9] <= 0.35275034290385476) {
                    if (input[0] <= 31.489191953626314) {
                        if (input[7] <= 13.487661706238626) {
                            set_output2(var311, 0.7649107531562908, 0.2350892468437092);
                        } else {
                            set_output2(var311, 0.9208025343189018, 0.0791974656810982);
                        }
                    } else {
                        if (input[0] <= 32.09325348574065) {
                            set_output2(var311, 0.9210526315789473, 0.07894736842105263);
                        } else {
                            set_output2(var311, 0.8508579676057091, 0.1491420323942909);
                        }
                    }
                } else {
                    set_output2(var311, 0.27586206896551724, 0.7241379310344828);
                }
            } else {
                if (input[6] <= 76.3009242892844) {
                    if (input[5] <= 0.2970356282634667) {
                        set_output2(var311, 0.9683254733582037, 0.0316745266417963);
                    } else {
                        set_output2(var311, 0.0, 1.0);
                    }
                } else {
                    set_output2(var311, 0.7272727272727273, 0.2727272727272727);
                }
            }
        } else {
            set_output2(var311, 0.0, 1.0);
        }
    } else {
        set_output2(var311, 0.0, 1.0);
    }
    add_vectors(var10, var311, 2, var9);
    double var312[2];
    if (input[3] <= -35.65839666810679) {
        if (input[9] <= 6.456457564570854) {
            if (input[7] <= 14.041671144508365) {
                if (input[8] <= 0.7299703101738366) {
                    if (input[9] <= 0.4243569944668196) {
                        if (input[8] <= 0.6915445112210018) {
                            set_output2(var312, 0.8113415710503089, 0.1886584289496911);
                        } else {
                            set_output2(var312, 0.6153846153846154, 0.38461538461538464);
                        }
                    } else {
                        set_output2(var312, 0.0, 1.0);
                    }
                } else {
                    if (input[2] <= -4.963683201485939) {
                        set_output2(var312, 0.9696969696969697, 0.030303030303030304);
                    } else {
                        set_output2(var312, 0.8615384615384616, 0.13846153846153847);
                    }
                }
            } else {
                if (input[8] <= 0.988156879755857) {
                    if (input[9] <= 1.1794973318150523) {
                        if (input[9] <= 0.5776479160043905) {
                            set_output2(var312, 0.9225433327022049, 0.07745666729779507);
                        } else {
                            set_output2(var312, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var312, 0.0, 1.0);
                    }
                } else {
                    set_output2(var312, 0.5714285714285714, 0.42857142857142855);
                }
            }
        } else {
            set_output2(var312, 0.0, 1.0);
        }
    } else {
        set_output2(var312, 0.0, 1.0);
    }
    add_vectors(var9, var312, 2, var8);
    double var313[2];
    if (input[9] <= 4.119918292396992) {
        if (input[7] <= 12.167451645642226) {
            set_output2(var313, 0.4, 0.6);
        } else {
            if (input[4] <= 27.122483130395) {
                if (input[9] <= 2.0983507100650285) {
                    if (input[3] <= -38.3084030089051) {
                        if (input[3] <= -47.27041605541608) {
                            set_output2(var313, 0.8913387696936109, 0.10866123030638905);
                        } else {
                            set_output2(var313, 0.92172892116363, 0.07827107883636997);
                        }
                    } else {
                        set_output2(var313, 0.0, 1.0);
                    }
                } else {
                    set_output2(var313, 0.0, 1.0);
                }
            } else {
                set_output2(var313, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var313, 0.0, 1.0);
    }
    add_vectors(var8, var313, 2, var7);
    double var314[2];
    if (input[9] <= 4.753943595611344) {
        if (input[9] <= 2.0777278786269653) {
            if (input[4] <= 17.674091165243773) {
                if (input[9] <= 0.23316254164428843) {
                    if (input[7] <= 13.31353362153911) {
                        if (input[7] <= 11.88135805004609) {
                            set_output2(var314, 0.0, 1.0);
                        } else {
                            set_output2(var314, 0.7849747809928325, 0.2150252190071675);
                        }
                    } else {
                        if (input[1] <= 6.609616233705295) {
                            set_output2(var314, 0.9569130601302183, 0.043086939869781696);
                        } else {
                            set_output2(var314, 0.9046849757673667, 0.09531502423263329);
                        }
                    }
                } else {
                    if (input[7] <= 14.293463758336022) {
                        set_output2(var314, 0.4, 0.6);
                    } else {
                        set_output2(var314, 0.9072164948453608, 0.09278350515463918);
                    }
                }
            } else {
                set_output2(var314, 0.0, 1.0);
            }
        } else {
            set_output2(var314, 0.0, 1.0);
        }
    } else {
        set_output2(var314, 0.0, 1.0);
    }
    add_vectors(var7, var314, 2, var6);
    double var315[2];
    if (input[3] <= -39.21383235376851) {
        if (input[9] <= 4.431578543558164) {
            if (input[9] <= 2.390431853924722) {
                if (input[9] <= 0.5202701875489694) {
                    if (input[0] <= 33.61245807624136) {
                        if (input[6] <= 6.426547791386708) {
                            set_output2(var315, 0.8758620689655172, 0.12413793103448276);
                        } else {
                            set_output2(var315, 0.9200642869226865, 0.07993571307731348);
                        }
                    } else {
                        if (input[4] <= 1.0357846033662517) {
                            set_output2(var315, 0.916330957545708, 0.08366904245429191);
                        } else {
                            set_output2(var315, 0.9745042492917847, 0.025495750708215296);
                        }
                    }
                } else {
                    set_output2(var315, 0.0, 1.0);
                }
            } else {
                set_output2(var315, 0.0, 1.0);
            }
        } else {
            set_output2(var315, 0.0, 1.0);
        }
    } else {
        set_output2(var315, 0.5714285714285714, 0.42857142857142855);
    }
    add_vectors(var6, var315, 2, var5);
    double var316[2];
    if (input[2] <= -1.6781254480433176) {
        if (input[9] <= 4.518689157101616) {
            if (input[9] <= 0.9505301074255335) {
                if (input[9] <= 0.4265012711105953) {
                    if (input[3] <= -39.99789526873324) {
                        if (input[7] <= 15.369765746242518) {
                            set_output2(var316, 0.9055096544483499, 0.09449034555165015);
                        } else {
                            set_output2(var316, 0.9513513513513514, 0.04864864864864865);
                        }
                    } else {
                        set_output2(var316, 0.7887323943661971, 0.2112676056338028);
                    }
                } else {
                    set_output2(var316, 0.0, 1.0);
                }
            } else {
                set_output2(var316, 0.0, 1.0);
            }
        } else {
            set_output2(var316, 0.0, 1.0);
        }
    } else {
        set_output2(var316, 0.0, 1.0);
    }
    add_vectors(var5, var316, 2, var4);
    double var317[2];
    if (input[9] <= 4.8459781054954245) {
        if (input[4] <= 25.95927134151836) {
            if (input[9] <= 1.8844725315978534) {
                if (input[9] <= 0.5697959897378053) {
                    if (input[7] <= 15.161074133410603) {
                        if (input[9] <= 0.45726001515631903) {
                            set_output2(var317, 0.898989898989899, 0.10101010101010102);
                        } else {
                            set_output2(var317, 0.0, 1.0);
                        }
                    } else {
                        if (input[3] <= -53.25752725051019) {
                            set_output2(var317, 1.0, 0.0);
                        } else {
                            set_output2(var317, 0.9411764705882353, 0.058823529411764705);
                        }
                    }
                } else {
                    set_output2(var317, 0.0, 1.0);
                }
            } else {
                set_output2(var317, 0.0, 1.0);
            }
        } else {
            set_output2(var317, 0.0, 1.0);
        }
    } else {
        set_output2(var317, 0.0, 1.0);
    }
    add_vectors(var4, var317, 2, var3);
    double var318[2];
    if (input[9] <= 4.777516485831567) {
        if (input[8] <= 1.4980690862892148) {
            if (input[9] <= 0.8201912054155333) {
                if (input[9] <= 0.43390881309036156) {
                    if (input[3] <= -38.19989511261184) {
                        if (input[7] <= 14.316557863472717) {
                            set_output2(var318, 0.8763155039600737, 0.12368449603992623);
                        } else {
                            set_output2(var318, 0.9269256089532587, 0.07307439104674128);
                        }
                    } else {
                        set_output2(var318, 0.0, 1.0);
                    }
                } else {
                    set_output2(var318, 0.0, 1.0);
                }
            } else {
                set_output2(var318, 0.0, 1.0);
            }
        } else {
            set_output2(var318, 0.0, 1.0);
        }
    } else {
        set_output2(var318, 0.0, 1.0);
    }
    add_vectors(var3, var318, 2, var2);
    double var319[2];
    if (input[2] <= 1.1446775629082317) {
        if (input[9] <= 4.026779319418706) {
            if (input[1] <= 12.574316571532362) {
                if (input[7] <= 11.618657771034904) {
                    set_output2(var319, 0.0, 1.0);
                } else {
                    if (input[9] <= 2.2927735025101748) {
                        if (input[9] <= 0.4534782659772796) {
                            set_output2(var319, 0.9086306951475491, 0.09136930485245091);
                        } else {
                            set_output2(var319, 0.0, 1.0);
                        }
                    } else {
                        set_output2(var319, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var319, 0.34782608695652173, 0.6521739130434783);
            }
        } else {
            set_output2(var319, 0.0, 1.0);
        }
    } else {
        set_output2(var319, 0.0, 1.0);
    }
    add_vectors(var2, var319, 2, var1);
    mul_vector_number(var1, 0.00625, 2, var0);
    memcpy(output, var0, 2 * sizeof(double));
}
