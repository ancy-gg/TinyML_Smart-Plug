#pragma once
// Auto-generated C header from scikit-learn RandomForest (m2cgen)
#define ARC_MODEL_FEATURE_VERSION 4
#define ARC_MODEL_INPUT_DIM 10
#define ARC_THRESHOLD 0.4600

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
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[2] <= -5.9743804931640625) {
                set_output2(var280, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var280, 0.0, 1.0);
            }
        } else {
            set_output2(var280, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var280, 1.0, 0.0);
            } else {
                if (input[3] <= -64.41340255737305) {
                    set_output2(var280, 1.0, 0.0);
                } else {
                    set_output2(var280, 0.9858839096532197, 0.014116090346780283);
                }
            }
        } else {
            set_output2(var280, 0.3323442136498516, 0.6676557863501483);
        }
    }
    double var281[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[7] <= 17.10542869567871) {
                    set_output2(var281, 0.9936409241856851, 0.0063590758143149875);
                } else {
                    set_output2(var281, 0.8600311041990669, 0.13996889580093314);
                }
            } else {
                if (input[2] <= -12.434505939483643) {
                    set_output2(var281, 1.0, 0.0);
                } else {
                    set_output2(var281, 0.9982227909291249, 0.0017772090708750988);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var281, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var281, 0.987069135823163, 0.012930864176837064);
                }
            } else {
                set_output2(var281, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[4] <= 70.957350499928) {
                    set_output2(var281, 0.8379837983798379, 0.16201620162016203);
                } else {
                    set_output2(var281, 0.28, 0.72);
                }
            } else {
                if (input[9] <= 1.7081345319747925) {
                    set_output2(var281, 0.5964125560538117, 0.40358744394618834);
                } else {
                    set_output2(var281, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var281, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var280, var281, 2, var279);
    double var282[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var282, 0.9974724780948102, 0.00252752190518984);
                } else {
                    set_output2(var282, 0.8695904173106647, 0.1304095826893354);
                }
            } else {
                set_output2(var282, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var282, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var282, 0.987069135823163, 0.01293086417683704);
                } else {
                    set_output2(var282, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var282, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var282, 0.0, 1.0);
                } else {
                    set_output2(var282, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var282, 0.0, 1.0);
            } else {
                set_output2(var282, 1.0, 0.0);
            }
        }
    }
    add_vectors(var279, var282, 2, var278);
    double var283[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var283, 0.9928300183232864, 0.007169981676713483);
                } else {
                    set_output2(var283, 0.0, 1.0);
                }
            } else {
                set_output2(var283, 1.0, 0.0);
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var283, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var283, 0.987069135823163, 0.01293086417683705);
                } else {
                    set_output2(var283, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[8] <= 0.7983885109424591) {
                if (input[8] <= 0.7666110098361969) {
                    set_output2(var283, 0.5384615384615385, 0.46153846153846156);
                } else {
                    set_output2(var283, 0.0, 1.0);
                }
            } else {
                if (input[1] <= 7.8367509841918945) {
                    set_output2(var283, 0.3300248138957817, 0.6699751861042184);
                } else {
                    set_output2(var283, 0.8983816334211516, 0.10161836657884833);
                }
            }
        } else {
            set_output2(var283, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var278, var283, 2, var277);
    double var284[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[1] <= 3.527751088142395) {
                if (input[1] <= 3.445926070213318) {
                    set_output2(var284, 1.0, 0.0);
                } else {
                    set_output2(var284, 0.0, 1.0);
                }
            } else {
                set_output2(var284, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var284, 0.0, 1.0);
                } else {
                    set_output2(var284, 0.9880752725488189, 0.011924727451181182);
                }
            } else {
                set_output2(var284, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[4] <= 70.957350499928) {
                    set_output2(var284, 0.8379837983798379, 0.16201620162016203);
                } else {
                    set_output2(var284, 0.28, 0.72);
                }
            } else {
                if (input[7] <= 15.713719367980957) {
                    set_output2(var284, 0.5544554455445545, 0.44554455445544555);
                } else {
                    set_output2(var284, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var284, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var277, var284, 2, var276);
    double var285[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[3] <= -48.08561134338379) {
                set_output2(var285, 0.8598785614198973, 0.1401214385801028);
            } else {
                set_output2(var285, 0.3715083798882682, 0.6284916201117319);
            }
        } else {
            set_output2(var285, 1.0, 0.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[9] <= 1.4035595059394836) {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var285, 0.9917692331549607, 0.008230766845039294);
                } else {
                    set_output2(var285, 0.9524965691966641, 0.04750343080333578);
                }
            } else {
                if (input[5] <= 172.15264892578125) {
                    set_output2(var285, 0.37608318890814557, 0.6239168110918544);
                } else {
                    set_output2(var285, 0.9463966646813579, 0.053603335318642045);
                }
            }
        } else {
            if (input[9] <= 1.1798875033855438) {
                if (input[4] <= 199.75135040283203) {
                    set_output2(var285, 0.0, 1.0);
                } else {
                    set_output2(var285, 1.0, 0.0);
                }
            } else {
                set_output2(var285, 0.29319371727748694, 0.706806282722513);
            }
        }
    }
    add_vectors(var276, var285, 2, var275);
    double var286[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var286, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var286, 0.0, 1.0);
            }
        } else {
            set_output2(var286, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var286, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var286, 0.9882568690919772, 0.011743130908022797);
                } else {
                    set_output2(var286, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var286, 0.266304347826087, 0.7336956521739131);
                } else {
                    set_output2(var286, 0.6447368421052632, 0.35526315789473684);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var286, 0.0, 1.0);
                } else {
                    set_output2(var286, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var275, var286, 2, var274);
    double var287[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var287, 1.0, 0.0);
            } else {
                set_output2(var287, 0.7439180537772087, 0.2560819462227913);
            }
        } else {
            set_output2(var287, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var287, 1.0, 0.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var287, 0.9902859646657134, 0.009714035334286572);
                } else {
                    set_output2(var287, 0.9428462564297961, 0.05715374357020384);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var287, 0.266304347826087, 0.7336956521739131);
                } else {
                    set_output2(var287, 0.6447368421052632, 0.35526315789473684);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var287, 0.0, 1.0);
                } else {
                    set_output2(var287, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var274, var287, 2, var273);
    double var288[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var288, 0.6232207647222998, 0.37677923527770024);
        } else {
            set_output2(var288, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var288, 0.9972482239578794, 0.0027517760421205623);
                } else {
                    set_output2(var288, 0.9887783113852344, 0.011221688614765609);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var288, 0.34746235359732286, 0.6525376464026771);
                } else {
                    set_output2(var288, 0.9053627760252365, 0.09463722397476339);
                }
            }
        } else {
            set_output2(var288, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var273, var288, 2, var272);
    double var289[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var289, 0.9974724780948102, 0.0025275219051898398);
                } else {
                    set_output2(var289, 0.8695904173106646, 0.1304095826893354);
                }
            } else {
                set_output2(var289, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var289, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var289, 0.987069135823163, 0.012930864176837052);
                }
            } else {
                set_output2(var289, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var289, 1.0, 0.0);
            } else {
                set_output2(var289, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[9] <= 3.002187967300415) {
                set_output2(var289, 0.6495781959766386, 0.3504218040233615);
            } else {
                set_output2(var289, 0.18490566037735848, 0.8150943396226416);
            }
        }
    }
    add_vectors(var272, var289, 2, var271);
    double var290[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[5] <= 0.05140000022947788) {
                if (input[2] <= -4.56547999382019) {
                    set_output2(var290, 0.7967479674796747, 0.20325203252032517);
                } else {
                    set_output2(var290, 1.0, 0.0);
                }
            } else {
                set_output2(var290, 0.0, 1.0);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var290, 1.0, 0.0);
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var290, 0.0, 1.0);
                } else {
                    set_output2(var290, 0.9898111946442208, 0.01018880535577922);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var290, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var290, 0.2245093346098612, 0.7754906653901388);
                } else {
                    set_output2(var290, 1.0, 0.0);
                }
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var290, 1.0, 0.0);
            } else {
                set_output2(var290, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var271, var290, 2, var270);
    double var291[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var291, 0.8657317618976578, 0.13426823810234223);
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var291, 0.9902130829332478, 0.009786917066752188);
                } else {
                    set_output2(var291, 1.0, 0.0);
                }
            } else {
                if (input[8] <= 0.9165079891681671) {
                    set_output2(var291, 0.9913293823148299, 0.008670617685170063);
                } else {
                    set_output2(var291, 0.9841002976777599, 0.015899702322240104);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[1] <= 11.923262119293213) {
                if (input[4] <= 89.85874938964844) {
                    set_output2(var291, 0.7045951859956237, 0.2954048140043764);
                } else {
                    set_output2(var291, 0.1536050156739812, 0.8463949843260188);
                }
            } else {
                set_output2(var291, 0.711769415532426, 0.28823058446757405);
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var291, 0.0, 1.0);
            } else {
                set_output2(var291, 1.0, 0.0);
            }
        }
    }
    add_vectors(var270, var291, 2, var269);
    double var292[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[2] <= -5.9743804931640625) {
                set_output2(var292, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var292, 0.0, 1.0);
            }
        } else {
            set_output2(var292, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var292, 0.9979329320669608, 0.0020670679330392783);
                } else {
                    set_output2(var292, 0.9887643780849196, 0.011235621915080464);
                }
            } else {
                if (input[3] <= -54.70915985107422) {
                    set_output2(var292, 0.8288159771754636, 0.1711840228245364);
                } else {
                    set_output2(var292, 0.37814070351758794, 0.621859296482412);
                }
            }
        } else {
            set_output2(var292, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var269, var292, 2, var268);
    double var293[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var293, 1.0, 0.0);
            } else {
                set_output2(var293, 0.7439180537772087, 0.25608194622279135);
            }
        } else {
            set_output2(var293, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var293, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var293, 0.9882568690919773, 0.011743130908022753);
                } else {
                    set_output2(var293, 1.0, 0.0);
                }
            }
        } else {
            if (input[14] <= 0.5) {
                set_output2(var293, 0.29577464788732394, 0.7042253521126761);
            } else {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var293, 0.5860165593376265, 0.41398344066237347);
                } else {
                    set_output2(var293, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var268, var293, 2, var267);
    double var294[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[3] <= -19.16687774658203) {
            if (input[4] <= 88.75569772720337) {
                if (input[7] <= 16.334160804748535) {
                    set_output2(var294, 0.8110831234256927, 0.1889168765743073);
                } else {
                    set_output2(var294, 0.0, 1.0);
                }
            } else {
                set_output2(var294, 1.0, 0.0);
            }
        } else {
            set_output2(var294, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var294, 1.0, 0.0);
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var294, 0.0, 1.0);
                } else {
                    set_output2(var294, 0.9898111946442208, 0.01018880535577922);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var294, 1.0, 0.0);
                } else {
                    set_output2(var294, 0.31210191082802546, 0.6878980891719745);
                }
            } else {
                if (input[3] <= -46.46929168701172) {
                    set_output2(var294, 1.0, 0.0);
                } else {
                    set_output2(var294, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var267, var294, 2, var266);
    double var295[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var295, 0.9974724780948102, 0.002527521905189839);
                } else {
                    set_output2(var295, 0.8695904173106647, 0.13040958268933536);
                }
            } else {
                set_output2(var295, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var295, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var295, 0.987069135823163, 0.012930864176837047);
                } else {
                    set_output2(var295, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var295, 0.7943640517897943, 0.20563594821020567);
                } else {
                    set_output2(var295, 0.35251798561151076, 0.6474820143884892);
                }
            } else {
                set_output2(var295, 0.0, 1.0);
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var295, 0.0, 1.0);
            } else {
                set_output2(var295, 1.0, 0.0);
            }
        }
    }
    add_vectors(var266, var295, 2, var265);
    double var296[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var296, 0.9974724780948102, 0.0025275219051898415);
                } else {
                    set_output2(var296, 0.8695904173106647, 0.1304095826893354);
                }
            } else {
                set_output2(var296, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var296, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var296, 0.987069135823163, 0.012930864176837056);
                }
            } else {
                set_output2(var296, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var296, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var296, 0.47572815533980584, 0.5242718446601942);
                } else {
                    set_output2(var296, 0.0, 1.0);
                }
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var296, 1.0, 0.0);
            } else {
                set_output2(var296, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var265, var296, 2, var264);
    double var297[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var297, 0.9928300183232865, 0.007169981676713484);
                } else {
                    set_output2(var297, 0.0, 1.0);
                }
            } else {
                set_output2(var297, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var297, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var297, 0.987069135823163, 0.012930864176837047);
                }
            } else {
                set_output2(var297, 1.0, 0.0);
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[3] <= -51.78119659423828) {
                if (input[14] <= 0.5) {
                    set_output2(var297, 0.4249201277955272, 0.5750798722044728);
                } else {
                    set_output2(var297, 0.9562469615945551, 0.04375303840544482);
                }
            } else {
                if (input[1] <= 10.020683288574219) {
                    set_output2(var297, 0.12147505422993493, 0.878524945770065);
                } else {
                    set_output2(var297, 0.711769415532426, 0.28823058446757405);
                }
            }
        } else {
            set_output2(var297, 0.0, 1.0);
        }
    }
    add_vectors(var264, var297, 2, var263);
    double var298[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var298, 0.9928300183232865, 0.007169981676713488);
                } else {
                    set_output2(var298, 0.0, 1.0);
                }
            } else {
                set_output2(var298, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 5.552222490310669) {
                    set_output2(var298, 0.0, 1.0);
                } else {
                    set_output2(var298, 0.9868085480608563, 0.013191451939143737);
                }
            } else {
                set_output2(var298, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var298, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var298, 0.0, 1.0);
                } else {
                    set_output2(var298, 0.5051546391752578, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var298, 1.0, 0.0);
            } else {
                set_output2(var298, 0.0, 1.0);
            }
        }
    }
    add_vectors(var263, var298, 2, var262);
    double var299[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[8] <= 0.1918649971485138) {
                if (input[8] <= 0.18969149887561798) {
                    set_output2(var299, 1.0, 0.0);
                } else {
                    set_output2(var299, 0.0, 1.0);
                }
            } else {
                set_output2(var299, 1.0, 0.0);
            }
        } else {
            if (input[7] <= 17.760960578918457) {
                if (input[7] <= 17.295279502868652) {
                    set_output2(var299, 0.9891622598924485, 0.010837740107551478);
                } else {
                    set_output2(var299, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.764528274536133) {
                    set_output2(var299, 0.0, 1.0);
                } else {
                    set_output2(var299, 0.9565133359103208, 0.04348666408967915);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var299, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var299, 0.4553706505295008, 0.5446293494704992);
                } else {
                    set_output2(var299, 0.0, 1.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var299, 0.0, 1.0);
            } else {
                set_output2(var299, 1.0, 0.0);
            }
        }
    }
    add_vectors(var262, var299, 2, var261);
    double var300[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var300, 1.0, 0.0);
            } else {
                set_output2(var300, 0.7439180537772087, 0.2560819462227913);
            }
        } else {
            set_output2(var300, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var300, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var300, 0.9882568690919772, 0.011743130908022797);
                } else {
                    set_output2(var300, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var300, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var300, 1.0, 0.0);
                }
            } else {
                if (input[3] <= -46.46929168701172) {
                    set_output2(var300, 1.0, 0.0);
                } else {
                    set_output2(var300, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var261, var300, 2, var260);
    double var301[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[4] <= 2.857300043106079) {
                if (input[4] <= 2.4579999446868896) {
                    set_output2(var301, 1.0, 0.0);
                } else {
                    set_output2(var301, 0.0, 1.0);
                }
            } else {
                set_output2(var301, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.744715690612793) {
                set_output2(var301, 0.0, 1.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var301, 0.9880752725488189, 0.011924727451181154);
                } else {
                    set_output2(var301, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[2] <= -13.028204441070557) {
                set_output2(var301, 1.0, 0.0);
            } else {
                if (input[1] <= 9.815945625305176) {
                    set_output2(var301, 0.17314487632508835, 0.8268551236749117);
                } else {
                    set_output2(var301, 0.6100144439094848, 0.38998555609051516);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var301, 1.0, 0.0);
            } else {
                set_output2(var301, 0.0, 1.0);
            }
        }
    }
    add_vectors(var260, var301, 2, var259);
    double var302[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var302, 0.560546875, 0.439453125);
            } else {
                set_output2(var302, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var302, 0.8657317618976578, 0.13426823810234223);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var302, 0.9882711945136788, 0.011728805486321195);
                } else {
                    set_output2(var302, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[6] <= 101.93917083740234) {
            set_output2(var302, 0.0, 1.0);
        } else {
            if (input[3] <= -54.30373573303223) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var302, 0.4962686567164179, 0.503731343283582);
                } else {
                    set_output2(var302, 1.0, 0.0);
                }
            } else {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var302, 0.0, 1.0);
                } else {
                    set_output2(var302, 0.5356835769561479, 0.4643164230438522);
                }
            }
        }
    }
    add_vectors(var259, var302, 2, var258);
    double var303[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[5] <= 0.05140000022947788) {
                if (input[7] <= 16.308215141296387) {
                    set_output2(var303, 1.0, 0.0);
                } else {
                    set_output2(var303, 0.3894165535956581, 0.6105834464043419);
                }
            } else {
                set_output2(var303, 0.0, 1.0);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var303, 0.9927898095975646, 0.0072101904024354305);
                } else {
                    set_output2(var303, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var303, 0.9870506314315944, 0.012949368568405646);
                } else {
                    set_output2(var303, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var303, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var303, 0.0, 1.0);
                } else {
                    set_output2(var303, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var303, 0.0, 1.0);
            } else {
                set_output2(var303, 1.0, 0.0);
            }
        }
    }
    add_vectors(var258, var303, 2, var257);
    double var304[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[3] <= -19.16687774658203) {
            if (input[7] <= 16.476778984069824) {
                if (input[6] <= 140.11605834960938) {
                    set_output2(var304, 1.0, 0.0);
                } else {
                    set_output2(var304, 0.7439180537772087, 0.25608194622279135);
                }
            } else {
                set_output2(var304, 0.0, 1.0);
            }
        } else {
            set_output2(var304, 1.0, 0.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[9] <= 1.4035595059394836) {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var304, 0.9917692331549607, 0.00823076684503931);
                } else {
                    set_output2(var304, 0.9524965691966641, 0.0475034308033358);
                }
            } else {
                if (input[5] <= 172.15264892578125) {
                    set_output2(var304, 0.37608318890814557, 0.6239168110918544);
                } else {
                    set_output2(var304, 0.9463966646813579, 0.053603335318642045);
                }
            }
        } else {
            if (input[9] <= 1.1798875033855438) {
                if (input[3] <= -35.49544334411621) {
                    set_output2(var304, 0.560546875, 0.439453125);
                } else {
                    set_output2(var304, 1.0, 0.0);
                }
            } else {
                set_output2(var304, 0.29319371727748694, 0.706806282722513);
            }
        }
    }
    add_vectors(var257, var304, 2, var256);
    double var305[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[6] <= 184.20638275146484) {
                set_output2(var305, 1.0, 0.0);
            } else {
                set_output2(var305, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var305, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                if (input[4] <= 2.857300043106079) {
                    set_output2(var305, 0.967569904871721, 0.032430095128279046);
                } else {
                    set_output2(var305, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var305, 0.9852096737329875, 0.01479032626701255);
                } else {
                    set_output2(var305, 0.9982046248862929, 0.0017953751137070905);
                }
            }
        } else {
            set_output2(var305, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var256, var305, 2, var255);
    double var306[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[2] <= -3.9435800313949585) {
                set_output2(var306, 0.7678817056396149, 0.23211829436038517);
            } else {
                set_output2(var306, 1.0, 0.0);
            }
        } else {
            set_output2(var306, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var306, 0.9987765888033407, 0.0012234111966592776);
                } else {
                    set_output2(var306, 0.9896207527057276, 0.010379247294272392);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var306, 0.34746235359732286, 0.6525376464026771);
                } else {
                    set_output2(var306, 0.9053627760252365, 0.0946372239747634);
                }
            }
        } else {
            set_output2(var306, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var255, var306, 2, var254);
    double var307[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var307, 1.0, 0.0);
            } else {
                set_output2(var307, 0.7439180537772088, 0.25608194622279135);
            }
        } else {
            set_output2(var307, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var307, 0.9979329320669608, 0.002067067933039282);
                } else {
                    set_output2(var307, 0.9887643780849196, 0.011235621915080488);
                }
            } else {
                if (input[14] <= 0.5) {
                    set_output2(var307, 0.3181818181818182, 0.6818181818181819);
                } else {
                    set_output2(var307, 0.7846031112883924, 0.2153968887116075);
                }
            }
        } else {
            set_output2(var307, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var254, var307, 2, var253);
    double var308[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[6] <= 109.77089309692383) {
                set_output2(var308, 1.0, 0.0);
            } else {
                set_output2(var308, 0.7283155564499899, 0.27168444355001004);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var308, 1.0, 0.0);
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var308, 0.0, 1.0);
                } else {
                    set_output2(var308, 0.9898111946442208, 0.010188805355779217);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var308, 0.6853146853146852, 0.3146853146853147);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var308, 0.0, 1.0);
                } else {
                    set_output2(var308, 0.4553706505295008, 0.5446293494704992);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var308, 1.0, 0.0);
            } else {
                set_output2(var308, 0.0, 1.0);
            }
        }
    }
    add_vectors(var253, var308, 2, var252);
    double var309[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var309, 0.9928169328835492, 0.00718306711645081);
                } else {
                    set_output2(var309, 0.0, 1.0);
                }
            } else {
                set_output2(var309, 1.0, 0.0);
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var309, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[8] <= 0.9165079891681671) {
                    set_output2(var309, 0.9913293823148299, 0.008670617685170069);
                } else {
                    set_output2(var309, 0.98410029767776, 0.015899702322240125);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var309, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var309, 0.0, 1.0);
                } else {
                    set_output2(var309, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[9] <= 1.7081345319747925) {
                set_output2(var309, 0.0, 1.0);
            } else {
                set_output2(var309, 0.946396664681358, 0.05360333531864206);
            }
        }
    }
    add_vectors(var252, var309, 2, var251);
    double var310[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var310, 0.9928169328835492, 0.00718306711645081);
                } else {
                    set_output2(var310, 0.0, 1.0);
                }
            } else {
                set_output2(var310, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var310, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var310, 0.987069135823163, 0.012930864176837044);
                }
            } else {
                set_output2(var310, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var310, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var310, 0.4553706505295008, 0.5446293494704992);
                } else {
                    set_output2(var310, 0.0, 1.0);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var310, 1.0, 0.0);
            } else {
                set_output2(var310, 0.0, 1.0);
            }
        }
    }
    add_vectors(var251, var310, 2, var250);
    double var311[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[1] <= 3.8508650064468384) {
                    set_output2(var311, 0.9503694717105988, 0.04963052828940112);
                } else {
                    set_output2(var311, 0.9947245326815201, 0.005275467318479952);
                }
            } else {
                set_output2(var311, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var311, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var311, 0.987069135823163, 0.012930864176837052);
                }
            } else {
                set_output2(var311, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[8] <= 0.7983885109424591) {
                if (input[2] <= -13.028204441070557) {
                    set_output2(var311, 1.0, 0.0);
                } else {
                    set_output2(var311, 0.32145305003427005, 0.67854694996573);
                }
            } else {
                if (input[6] <= 132.54950332641602) {
                    set_output2(var311, 0.497907949790795, 0.502092050209205);
                } else {
                    set_output2(var311, 0.9231754161331626, 0.07682458386683741);
                }
            }
        } else {
            set_output2(var311, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var250, var311, 2, var249);
    double var312[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var312, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var312, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var312, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var312, 0.9882568690919772, 0.011743130908022825);
                } else {
                    set_output2(var312, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var312, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[2] <= -6.903803110122681) {
                    set_output2(var312, 0.0, 1.0);
                } else {
                    set_output2(var312, 0.3858267716535433, 0.6141732283464567);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var312, 0.0, 1.0);
            } else {
                set_output2(var312, 1.0, 0.0);
            }
        }
    }
    add_vectors(var249, var312, 2, var248);
    double var313[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[9] <= 6.277254581451416) {
            if (input[2] <= -3.9435800313949585) {
                if (input[9] <= 0.12401650100946426) {
                    set_output2(var313, 0.0, 1.0);
                } else {
                    set_output2(var313, 0.8354260636352554, 0.1645739363647446);
                }
            } else {
                set_output2(var313, 1.0, 0.0);
            }
        } else {
            set_output2(var313, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[2] <= -0.9100589752197266) {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var313, 0.9911863073604914, 0.008813692639508581);
                } else {
                    set_output2(var313, 0.6671408250355619, 0.3328591749644382);
                }
            } else {
                set_output2(var313, 0.23728813559322035, 0.7627118644067796);
            }
        } else {
            if (input[2] <= -3.9618570804595947) {
                set_output2(var313, 1.0, 0.0);
            } else {
                if (input[7] <= 16.653852462768555) {
                    set_output2(var313, 0.497907949790795, 0.502092050209205);
                } else {
                    set_output2(var313, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var248, var313, 2, var247);
    double var314[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[2] <= -3.9435800313949585) {
                set_output2(var314, 0.7678817056396149, 0.23211829436038517);
            } else {
                set_output2(var314, 1.0, 0.0);
            }
        } else {
            set_output2(var314, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                if (input[3] <= -63.54202461242676) {
                    set_output2(var314, 0.9853472697079222, 0.014652730292077754);
                } else {
                    set_output2(var314, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var314, 0.0, 1.0);
                } else {
                    set_output2(var314, 0.9898221960263143, 0.010177803973685803);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var314, 1.0, 0.0);
                } else {
                    set_output2(var314, 0.3121019108280255, 0.6878980891719745);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var314, 0.0, 1.0);
                } else {
                    set_output2(var314, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var247, var314, 2, var246);
    double var315[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var315, 0.9974724780948102, 0.00252752190518984);
                } else {
                    set_output2(var315, 0.8695904173106646, 0.1304095826893354);
                }
            } else {
                set_output2(var315, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var315, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var315, 0.987069135823163, 0.012930864176837042);
                }
            } else {
                set_output2(var315, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var315, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[4] <= 195.6771011352539) {
                    set_output2(var315, 0.0, 1.0);
                } else {
                    set_output2(var315, 0.3684210526315789, 0.631578947368421);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var315, 0.0, 1.0);
            } else {
                set_output2(var315, 1.0, 0.0);
            }
        }
    }
    add_vectors(var246, var315, 2, var245);
    double var316[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var316, 0.560546875, 0.439453125);
            } else {
                set_output2(var316, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.744715690612793) {
                set_output2(var316, 0.0, 1.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var316, 0.9901184957056632, 0.009881504294336811);
                } else {
                    set_output2(var316, 0.9428462564297961, 0.05715374357020384);
                }
            }
        }
    } else {
        if (input[1] <= 10.020683288574219) {
            if (input[2] <= -7.96181058883667) {
                set_output2(var316, 0.6853146853146854, 0.3146853146853147);
            } else {
                set_output2(var316, 0.1106719367588933, 0.8893280632411067);
            }
        } else {
            if (input[2] <= -4.609246015548706) {
                set_output2(var316, 1.0, 0.0);
            } else {
                if (input[8] <= 0.9787624776363373) {
                    set_output2(var316, 0.468503937007874, 0.5314960629921259);
                } else {
                    set_output2(var316, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var245, var316, 2, var244);
    double var317[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[7] <= 17.760960578918457) {
            if (input[1] <= 8.246700286865234) {
                if (input[5] <= 2.3686500787734985) {
                    set_output2(var317, 0.978871586196103, 0.021128413803897017);
                } else {
                    set_output2(var317, 0.9979710216807605, 0.002028978319239487);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var317, 0.9856323332645267, 0.01436766673547339);
                } else {
                    set_output2(var317, 1.0, 0.0);
                }
            }
        } else {
            if (input[8] <= 1.2688884735107422) {
                if (input[4] <= 3.746250033378601) {
                    set_output2(var317, 0.7335701598579041, 0.2664298401420959);
                } else {
                    set_output2(var317, 0.9867541871486181, 0.013245812851381975);
                }
            } else {
                set_output2(var317, 0.5964125560538117, 0.40358744394618834);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var317, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var317, 1.0, 0.0);
            }
        } else {
            if (input[6] <= 135.9388885498047) {
                set_output2(var317, 0.0, 1.0);
            } else {
                if (input[1] <= 6.2525246143341064) {
                    set_output2(var317, 0.0, 1.0);
                } else {
                    set_output2(var317, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var244, var317, 2, var243);
    double var318[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[1] <= 5.673432111740112) {
                set_output2(var318, 1.0, 0.0);
            } else {
                set_output2(var318, 0.7964261479303325, 0.2035738520696675);
            }
        } else {
            set_output2(var318, 0.0, 1.0);
        }
    } else {
        if (input[14] <= 0.5) {
            if (input[9] <= 1.4178645014762878) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var318, 0.9916381952376114, 0.008361804762388628);
                } else {
                    set_output2(var318, 0.9743969048702776, 0.025603095129722343);
                }
            } else {
                set_output2(var318, 0.29577464788732394, 0.7042253521126761);
            }
        } else {
            if (input[1] <= 5.063412427902222) {
                if (input[9] <= 4.642098009586334) {
                    set_output2(var318, 0.9989678993770699, 0.0010321006229300732);
                } else {
                    set_output2(var318, 0.0, 1.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var318, 0.9854087968742398, 0.014591203125760238);
                } else {
                    set_output2(var318, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var243, var318, 2, var242);
    double var319[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[1] <= 3.527751088142395) {
                if (input[7] <= 16.999037742614746) {
                    set_output2(var319, 1.0, 0.0);
                } else {
                    set_output2(var319, 0.7335701598579041, 0.2664298401420959);
                }
            } else {
                set_output2(var319, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.413290977478027) {
                    set_output2(var319, 0.9895561761679675, 0.01044382383203243);
                } else {
                    set_output2(var319, 0.9676633328324961, 0.0323366671675039);
                }
            } else {
                set_output2(var319, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var319, 1.0, 0.0);
            } else {
                set_output2(var319, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var319, 0.0, 1.0);
            } else {
                if (input[2] <= -4.315015077590942) {
                    set_output2(var319, 0.7590361445783133, 0.24096385542168675);
                } else {
                    set_output2(var319, 0.13461538461538464, 0.8653846153846153);
                }
            }
        }
    }
    add_vectors(var242, var319, 2, var241);
    double var320[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[7] <= 17.14678382873535) {
                    set_output2(var320, 0.9972867539326994, 0.0027132460673005582);
                } else {
                    set_output2(var320, 0.9410016606939953, 0.058998339306004705);
                }
            } else {
                set_output2(var320, 1.0, 0.0);
            }
        } else {
            if (input[7] <= 16.308735847473145) {
                if (input[5] <= 63.740699768066406) {
                    set_output2(var320, 0.9902936396060105, 0.009706360393989574);
                } else {
                    set_output2(var320, 1.0, 0.0);
                }
            } else {
                if (input[5] <= 0.00839999970048666) {
                    set_output2(var320, 0.0, 1.0);
                } else {
                    set_output2(var320, 0.985117142491358, 0.01488285750864197);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var320, 0.6853146853146852, 0.3146853146853147);
            } else {
                if (input[5] <= 9.748749732971191) {
                    set_output2(var320, 0.0, 1.0);
                } else {
                    set_output2(var320, 0.42634560906515584, 0.5736543909348443);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var320, 1.0, 0.0);
            } else {
                set_output2(var320, 0.0, 1.0);
            }
        }
    }
    add_vectors(var241, var320, 2, var240);
    double var321[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var321, 1.0, 0.0);
            } else {
                set_output2(var321, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var321, 0.9927898095975646, 0.0072101904024354305);
                } else {
                    set_output2(var321, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var321, 0.9924123841132183, 0.007587615886781671);
                } else {
                    set_output2(var321, 0.9842091965639208, 0.015790803436079137);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var321, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var321, 0.0, 1.0);
                } else {
                    set_output2(var321, 0.4553706505295008, 0.5446293494704992);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var321, 1.0, 0.0);
            } else {
                set_output2(var321, 0.0, 1.0);
            }
        }
    }
    add_vectors(var240, var321, 2, var239);
    double var322[2];
    if (input[1] <= 5.543766021728516) {
        if (input[5] <= 21.988649368286133) {
            if (input[9] <= 1.6726675033569336) {
                if (input[9] <= 0.1185465008020401) {
                    set_output2(var322, 1.0, 0.0);
                } else {
                    set_output2(var322, 0.9843607454711325, 0.01563925452886745);
                }
            } else {
                set_output2(var322, 0.3300248138957817, 0.6699751861042184);
            }
        } else {
            if (input[8] <= 1.3909595012664795) {
                set_output2(var322, 1.0, 0.0);
            } else {
                set_output2(var322, 0.9420476497102382, 0.05795235028976175);
            }
        }
    } else {
        if (input[5] <= 0.0027000000700354576) {
            set_output2(var322, 0.6329027872195785, 0.3670972127804215);
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var322, 0.9924211802748584, 0.007578819725141624);
                } else {
                    set_output2(var322, 0.9842303720697692, 0.015769627930230793);
                }
            } else {
                if (input[8] <= 1.23604154586792) {
                    set_output2(var322, 0.5415472779369628, 0.4584527220630372);
                } else {
                    set_output2(var322, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var239, var322, 2, var238);
    double var323[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var323, 0.8657317618976578, 0.13426823810234226);
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var323, 0.9815568731386104, 0.018443126861389655);
                } else {
                    set_output2(var323, 0.9992253733270215, 0.0007746266729784521);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var323, 0.987069135823163, 0.012930864176837052);
                } else {
                    set_output2(var323, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var323, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var323, 0.0, 1.0);
                } else {
                    set_output2(var323, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var323, 1.0, 0.0);
            } else {
                set_output2(var323, 0.0, 1.0);
            }
        }
    }
    add_vectors(var238, var323, 2, var237);
    double var324[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[2] <= -5.9743804931640625) {
                set_output2(var324, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var324, 0.0, 1.0);
            }
        } else {
            set_output2(var324, 1.0, 0.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[2] <= -0.9100589752197266) {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var324, 0.9878013150483584, 0.01219868495164157);
                } else {
                    set_output2(var324, 1.0, 0.0);
                }
            } else {
                set_output2(var324, 0.23728813559322035, 0.7627118644067796);
            }
        } else {
            if (input[3] <= -35.49544334411621) {
                set_output2(var324, 0.3894165535956581, 0.6105834464043419);
            } else {
                if (input[5] <= 227.9127426147461) {
                    set_output2(var324, 0.9801727176594994, 0.019827282340500522);
                } else {
                    set_output2(var324, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var237, var324, 2, var236);
    double var325[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var325, 0.9974724780948102, 0.0025275219051898415);
                } else {
                    set_output2(var325, 0.8695904173106647, 0.13040958268933542);
                }
            } else {
                set_output2(var325, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var325, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var325, 0.987069135823163, 0.01293086417683704);
                }
            } else {
                set_output2(var325, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var325, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var325, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -4.315015077590942) {
                if (input[1] <= 7.8367509841918945) {
                    set_output2(var325, 0.0, 1.0);
                } else {
                    set_output2(var325, 0.863013698630137, 0.13698630136986303);
                }
            } else {
                set_output2(var325, 0.12147505422993493, 0.878524945770065);
            }
        }
    }
    add_vectors(var236, var325, 2, var235);
    double var326[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var326, 0.560546875, 0.439453125);
            } else {
                set_output2(var326, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[7] <= 17.75955295562744) {
                    set_output2(var326, 0.9886128252122895, 0.011387174787710494);
                } else {
                    set_output2(var326, 0.931740614334471, 0.06825938566552898);
                }
            } else {
                set_output2(var326, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var326, 0.747191011235955, 0.25280898876404495);
                } else {
                    set_output2(var326, 0.24843423799582465, 0.7515657620041754);
                }
            } else {
                if (input[3] <= -43.79238700866699) {
                    set_output2(var326, 1.0, 0.0);
                } else {
                    set_output2(var326, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var326, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var235, var326, 2, var234);
    double var327[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[6] <= 140.11605834960938) {
                    set_output2(var327, 1.0, 0.0);
                } else {
                    set_output2(var327, 0.8509440211990725, 0.14905597880092747);
                }
            } else {
                set_output2(var327, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var327, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[7] <= 17.760960578918457) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var327, 0.9985882065046031, 0.0014117934953967902);
                } else {
                    set_output2(var327, 0.9893430980377091, 0.010656901962290853);
                }
            } else {
                if (input[8] <= 1.2688884735107422) {
                    set_output2(var327, 0.9747630531097526, 0.025236946890247313);
                } else {
                    set_output2(var327, 0.5964125560538117, 0.40358744394618834);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var327, 1.0, 0.0);
                } else {
                    set_output2(var327, 0.31210191082802546, 0.6878980891719745);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var327, 0.0, 1.0);
                } else {
                    set_output2(var327, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var234, var327, 2, var233);
    double var328[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[0] <= 100.0) {
            set_output2(var328, 1.0, 0.0);
        } else {
            if (input[7] <= 16.476778984069824) {
                if (input[9] <= 6.339050054550171) {
                    set_output2(var328, 0.8664292074799644, 0.1335707925200356);
                } else {
                    set_output2(var328, 0.0, 1.0);
                }
            } else {
                set_output2(var328, 0.0, 1.0);
            }
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[1] <= 5.543766021728516) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var328, 0.9817229194589984, 0.01827708054100159);
                } else {
                    set_output2(var328, 0.9969091602188314, 0.0030908397811685836);
                }
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var328, 0.9889714164142869, 0.011028583585713176);
                } else {
                    set_output2(var328, 0.7307807358659886, 0.2692192641340114);
                }
            }
        } else {
            set_output2(var328, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var233, var328, 2, var232);
    double var329[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[7] <= 16.334160804748535) {
                    set_output2(var329, 1.0, 0.0);
                } else {
                    set_output2(var329, 0.560546875, 0.439453125);
                }
            } else {
                set_output2(var329, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var329, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var329, 1.0, 0.0);
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var329, 0.0, 1.0);
                } else {
                    set_output2(var329, 0.9898111946442208, 0.010188805355779136);
                }
            }
        } else {
            if (input[4] <= 124.15790176391602) {
                set_output2(var329, 1.0, 0.0);
            } else {
                if (input[9] <= 3.6920344829559326) {
                    set_output2(var329, 0.6422018348623852, 0.35779816513761475);
                } else {
                    set_output2(var329, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var232, var329, 2, var231);
    double var330[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[2] <= -3.9435800313949585) {
                set_output2(var330, 0.7678817056396149, 0.23211829436038517);
            } else {
                set_output2(var330, 1.0, 0.0);
            }
        } else {
            set_output2(var330, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var330, 0.9902130829332478, 0.009786917066752198);
                } else {
                    set_output2(var330, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var330, 0.987069135823163, 0.012930864176837042);
                } else {
                    set_output2(var330, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var330, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var330, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var330, 0.0, 1.0);
                } else {
                    set_output2(var330, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var231, var330, 2, var230);
    double var331[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[6] <= 109.77089309692383) {
                set_output2(var331, 1.0, 0.0);
            } else {
                set_output2(var331, 0.7283155564499899, 0.2716844435500101);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var331, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var331, 0.9882568690919772, 0.011743130908022797);
                } else {
                    set_output2(var331, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[8] <= 0.9768004715442657) {
                if (input[14] <= 0.5) {
                    set_output2(var331, 0.4249201277955272, 0.5750798722044728);
                } else {
                    set_output2(var331, 1.0, 0.0);
                }
            } else {
                set_output2(var331, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var331, 0.0, 1.0);
            } else {
                if (input[2] <= -4.315015077590942) {
                    set_output2(var331, 0.7590361445783133, 0.24096385542168675);
                } else {
                    set_output2(var331, 0.13461538461538464, 0.8653846153846153);
                }
            }
        }
    }
    add_vectors(var230, var331, 2, var229);
    double var332[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[1] <= 5.673432111740112) {
            set_output2(var332, 1.0, 0.0);
        } else {
            set_output2(var332, 0.6329027872195785, 0.3670972127804215);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[9] <= 0.23671899735927582) {
                if (input[8] <= 1.0516844987869263) {
                    set_output2(var332, 0.9916709340276276, 0.008329065972372351);
                } else {
                    set_output2(var332, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -12.434505939483643) {
                    set_output2(var332, 1.0, 0.0);
                } else {
                    set_output2(var332, 0.9853555606878159, 0.014644439312184036);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var332, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var332, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var332, 0.0, 1.0);
                } else {
                    set_output2(var332, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var229, var332, 2, var228);
    double var333[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[1] <= 9.928400039672852) {
                set_output2(var333, 0.9435736677115988, 0.05642633228840126);
            } else {
                set_output2(var333, 0.5792426367461431, 0.42075736325385693);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var333, 0.9926187156565243, 0.007381284343475757);
                } else {
                    set_output2(var333, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var333, 0.9870506314315944, 0.012949368568405646);
                } else {
                    set_output2(var333, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var333, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[2] <= -6.903803110122681) {
                    set_output2(var333, 0.0, 1.0);
                } else {
                    set_output2(var333, 0.3858267716535433, 0.6141732283464567);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var333, 0.0, 1.0);
            } else {
                set_output2(var333, 1.0, 0.0);
            }
        }
    }
    add_vectors(var228, var333, 2, var227);
    double var334[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var334, 0.8354260636352553, 0.1645739363647446);
            } else {
                set_output2(var334, 0.0, 1.0);
            }
        } else {
            set_output2(var334, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[7] <= 17.991198539733887) {
                    set_output2(var334, 0.9986103626799001, 0.0013896373200998915);
                } else {
                    set_output2(var334, 0.9174008810572687, 0.08259911894273127);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var334, 0.9924123841132183, 0.007587615886781687);
                } else {
                    set_output2(var334, 0.9842091965639209, 0.015790803436079113);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var334, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var334, 1.0, 0.0);
                }
            } else {
                if (input[3] <= -46.46929168701172) {
                    set_output2(var334, 1.0, 0.0);
                } else {
                    set_output2(var334, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var227, var334, 2, var226);
    double var335[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[9] <= 0.12401650100946426) {
                set_output2(var335, 0.0, 1.0);
            } else {
                if (input[2] <= -5.9743804931640625) {
                    set_output2(var335, 0.9383814870601123, 0.06161851293988772);
                } else {
                    set_output2(var335, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var335, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var335, 0.9926187156565243, 0.007381284343475756);
                } else {
                    set_output2(var335, 1.0, 0.0);
                }
            } else {
                set_output2(var335, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var335, 0.9924123841132183, 0.007587615886781691);
                } else {
                    set_output2(var335, 0.9842091965639209, 0.01579080343607912);
                }
            } else {
                if (input[5] <= 151.5938491821289) {
                    set_output2(var335, 0.3684210526315789, 0.6315789473684211);
                } else {
                    set_output2(var335, 0.8970251716247138, 0.10297482837528604);
                }
            }
        }
    }
    add_vectors(var226, var335, 2, var225);
    double var336[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var336, 1.0, 0.0);
            } else {
                set_output2(var336, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var336, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var336, 0.9882568690919772, 0.011743130908022823);
                } else {
                    set_output2(var336, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[5] <= 110.48614883422852) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var336, 0.7943640517897943, 0.20563594821020567);
                } else {
                    set_output2(var336, 0.28, 0.72);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var336, 0.3835616438356165, 0.6164383561643836);
                } else {
                    set_output2(var336, 0.9503311258278145, 0.049668874172185434);
                }
            }
        } else {
            set_output2(var336, 0.0, 1.0);
        }
    }
    add_vectors(var225, var336, 2, var224);
    double var337[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[7] <= 16.334160804748535) {
                    set_output2(var337, 1.0, 0.0);
                } else {
                    set_output2(var337, 0.560546875, 0.439453125);
                }
            } else {
                set_output2(var337, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var337, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var337, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var337, 0.9882568690919772, 0.011743130908022809);
                } else {
                    set_output2(var337, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var337, 1.0, 0.0);
                } else {
                    set_output2(var337, 0.3121019108280255, 0.6878980891719745);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var337, 0.0, 1.0);
                } else {
                    set_output2(var337, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var224, var337, 2, var223);
    double var338[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[8] <= 0.9231399893760681) {
                if (input[8] <= 0.922437995672226) {
                    set_output2(var338, 0.9955159780648289, 0.004484021935171019);
                } else {
                    set_output2(var338, 0.0, 1.0);
                }
            } else {
                set_output2(var338, 1.0, 0.0);
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var338, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var338, 0.9924211802748584, 0.007578819725141627);
                } else {
                    set_output2(var338, 0.9842303720697692, 0.015769627930230786);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[8] <= 0.7983885109424591) {
                if (input[2] <= -13.028204441070557) {
                    set_output2(var338, 1.0, 0.0);
                } else {
                    set_output2(var338, 0.3214530500342701, 0.67854694996573);
                }
            } else {
                if (input[1] <= 7.8367509841918945) {
                    set_output2(var338, 0.3300248138957817, 0.6699751861042184);
                } else {
                    set_output2(var338, 0.8983816334211517, 0.1016183665788483);
                }
            }
        } else {
            set_output2(var338, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var223, var338, 2, var222);
    double var339[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[1] <= 3.527751088142395) {
                if (input[5] <= 0.1485000029206276) {
                    set_output2(var339, 0.0, 1.0);
                } else {
                    set_output2(var339, 1.0, 0.0);
                }
            } else {
                set_output2(var339, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var339, 0.0, 1.0);
                } else {
                    set_output2(var339, 0.9880752725488189, 0.011924727451181179);
                }
            } else {
                set_output2(var339, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var339, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var339, 0.0, 1.0);
                } else {
                    set_output2(var339, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var339, 0.0, 1.0);
            } else {
                set_output2(var339, 1.0, 0.0);
            }
        }
    }
    add_vectors(var222, var339, 2, var221);
    double var340[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[9] <= 0.12401650100946426) {
                set_output2(var340, 0.0, 1.0);
            } else {
                if (input[2] <= -5.9743804931640625) {
                    set_output2(var340, 0.9383814870601123, 0.06161851293988773);
                } else {
                    set_output2(var340, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var340, 1.0, 0.0);
        }
    } else {
        if (input[14] <= 0.5) {
            if (input[9] <= 1.4178645014762878) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var340, 0.9916381952376114, 0.008361804762388625);
                } else {
                    set_output2(var340, 0.9743969048702777, 0.025603095129722354);
                }
            } else {
                set_output2(var340, 0.29577464788732394, 0.7042253521126761);
            }
        } else {
            if (input[3] <= -45.28707504272461) {
                if (input[9] <= 0.6078184843063354) {
                    set_output2(var340, 0.9966754227548842, 0.003324577245115822);
                } else {
                    set_output2(var340, 0.9533243439477233, 0.04667565605227671);
                }
            } else {
                if (input[3] <= -45.277408599853516) {
                    set_output2(var340, 0.0, 1.0);
                } else {
                    set_output2(var340, 0.9790937529036455, 0.020906247096354514);
                }
            }
        }
    }
    add_vectors(var221, var340, 2, var220);
    double var341[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var341, 0.9974724780948102, 0.0025275219051898406);
                } else {
                    set_output2(var341, 0.8695904173106647, 0.1304095826893354);
                }
            } else {
                set_output2(var341, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var341, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var341, 0.987069135823163, 0.012930864176837037);
                } else {
                    set_output2(var341, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var341, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var341, 0.47572815533980584, 0.5242718446601942);
                } else {
                    set_output2(var341, 0.0, 1.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var341, 0.0, 1.0);
            } else {
                set_output2(var341, 1.0, 0.0);
            }
        }
    }
    add_vectors(var220, var341, 2, var219);
    double var342[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var342, 1.0, 0.0);
            } else {
                set_output2(var342, 0.7439180537772087, 0.2560819462227913);
            }
        } else {
            set_output2(var342, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var342, 0.9926187156565243, 0.007381284343475749);
                } else {
                    set_output2(var342, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var342, 0.9870506314315943, 0.012949368568405652);
                } else {
                    set_output2(var342, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var342, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var342, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 6.1142754554748535) {
                    set_output2(var342, 0.0, 1.0);
                } else {
                    set_output2(var342, 0.44237918215613387, 0.5576208178438662);
                }
            }
        }
    }
    add_vectors(var219, var342, 2, var218);
    double var343[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[4] <= 3.367400050163269) {
                if (input[7] <= 17.010690689086914) {
                    set_output2(var343, 1.0, 0.0);
                } else {
                    set_output2(var343, 0.6682639144858091, 0.3317360855141909);
                }
            } else {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var343, 0.9855465801602724, 0.014453419839727635);
                } else {
                    set_output2(var343, 1.0, 0.0);
                }
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var343, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var343, 1.0, 0.0);
                } else {
                    set_output2(var343, 0.9880884695834108, 0.01191153041658929);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var343, 0.6853146853146852, 0.3146853146853147);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var343, 0.0, 1.0);
                } else {
                    set_output2(var343, 0.4553706505295008, 0.5446293494704992);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var343, 0.0, 1.0);
            } else {
                set_output2(var343, 1.0, 0.0);
            }
        }
    }
    add_vectors(var218, var343, 2, var217);
    double var344[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var344, 0.9974724780948101, 0.0025275219051898398);
                } else {
                    set_output2(var344, 0.8695904173106646, 0.13040958268933536);
                }
            } else {
                set_output2(var344, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var344, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var344, 0.987069135823163, 0.01293086417683705);
                }
            } else {
                set_output2(var344, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[1] <= 11.923262119293213) {
                if (input[4] <= 89.85874938964844) {
                    set_output2(var344, 0.7045951859956237, 0.2954048140043764);
                } else {
                    set_output2(var344, 0.1536050156739812, 0.8463949843260188);
                }
            } else {
                set_output2(var344, 0.7117694155324259, 0.2882305844675741);
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var344, 0.0, 1.0);
            } else {
                set_output2(var344, 1.0, 0.0);
            }
        }
    }
    add_vectors(var217, var344, 2, var216);
    double var345[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var345, 0.9815568731386104, 0.018443126861389655);
                } else {
                    set_output2(var345, 0.9992260075232069, 0.0007739924767931364);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var345, 0.9865429575708421, 0.01345704242915795);
                } else {
                    set_output2(var345, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 7.8367509841918945) {
                set_output2(var345, 0.33002481389578164, 0.6699751861042184);
            } else {
                if (input[9] <= 2.0398740768432617) {
                    set_output2(var345, 0.3617021276595745, 0.6382978723404256);
                } else {
                    set_output2(var345, 0.8983816334211517, 0.10161836657884835);
                }
            }
        }
    } else {
        set_output2(var345, 0.6945839554771278, 0.30541604452287224);
    }
    add_vectors(var216, var345, 2, var215);
    double var346[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[8] <= 0.9231399893760681) {
                if (input[3] <= -53.952714920043945) {
                    set_output2(var346, 0.9855099297231592, 0.014490070276840837);
                } else {
                    set_output2(var346, 1.0, 0.0);
                }
            } else {
                set_output2(var346, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var346, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var346, 0.987069135823163, 0.012930864176837038);
                }
            } else {
                set_output2(var346, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[1] <= 9.815945625305176) {
                    set_output2(var346, 0.3404397068620919, 0.6595602931379081);
                } else {
                    set_output2(var346, 0.6678966789667896, 0.33210332103321033);
                }
            } else {
                set_output2(var346, 0.0, 1.0);
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var346, 1.0, 0.0);
            } else {
                set_output2(var346, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var215, var346, 2, var214);
    double var347[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[6] <= 112.17279052734375) {
                    set_output2(var347, 1.0, 0.0);
                } else {
                    set_output2(var347, 0.9774121898881902, 0.02258781011180966);
                }
            } else {
                set_output2(var347, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var347, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var347, 0.987069135823163, 0.012930864176837038);
                }
            } else {
                set_output2(var347, 1.0, 0.0);
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[5] <= 110.48614883422852) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var347, 0.7943640517897943, 0.20563594821020567);
                } else {
                    set_output2(var347, 0.28, 0.72);
                }
            } else {
                if (input[3] <= -43.79238700866699) {
                    set_output2(var347, 0.9532224532224532, 0.04677754677754678);
                } else {
                    set_output2(var347, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var347, 0.0, 1.0);
        }
    }
    add_vectors(var214, var347, 2, var213);
    double var348[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var348, 0.9974724780948102, 0.00252752190518984);
                } else {
                    set_output2(var348, 0.8695904173106646, 0.1304095826893354);
                }
            } else {
                set_output2(var348, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var348, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var348, 0.987069135823163, 0.012930864176837052);
                }
            } else {
                set_output2(var348, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var348, 1.0, 0.0);
            } else {
                set_output2(var348, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[2] <= -4.315015077590942) {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var348, 0.0, 1.0);
                } else {
                    set_output2(var348, 0.7159090909090909, 0.28409090909090906);
                }
            } else {
                set_output2(var348, 0.12147505422993493, 0.878524945770065);
            }
        }
    }
    add_vectors(var213, var348, 2, var212);
    double var349[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var349, 0.9904628182372109, 0.009537181762789077);
                } else {
                    set_output2(var349, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var349, 0.9865429575708421, 0.013457042429157953);
                } else {
                    set_output2(var349, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -41.139564514160156) {
                if (input[9] <= 6.339050054550171) {
                    set_output2(var349, 0.7442576367511248, 0.2557423632488752);
                } else {
                    set_output2(var349, 0.0, 1.0);
                }
            } else {
                set_output2(var349, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var349, 0.6945839554771277, 0.3054160445228723);
    }
    add_vectors(var212, var349, 2, var211);
    double var350[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[9] <= 6.277254581451416) {
            if (input[2] <= -3.9435800313949585) {
                if (input[7] <= 16.334160804748535) {
                    set_output2(var350, 0.8839159035212176, 0.1160840964787824);
                } else {
                    set_output2(var350, 0.0, 1.0);
                }
            } else {
                set_output2(var350, 1.0, 0.0);
            }
        } else {
            set_output2(var350, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var350, 1.0, 0.0);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var350, 0.9927608506804799, 0.00723914931952011);
                } else {
                    set_output2(var350, 0.985551767137986, 0.01444823286201399);
                }
            }
        } else {
            if (input[9] <= 4.584504842758179) {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var350, 0.347462353597323, 0.6525376464026771);
                } else {
                    set_output2(var350, 0.9532224532224531, 0.04677754677754677);
                }
            } else {
                set_output2(var350, 0.17177914110429449, 0.8282208588957055);
            }
        }
    }
    add_vectors(var211, var350, 2, var210);
    double var351[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var351, 0.6232207647222998, 0.37677923527770024);
        } else {
            set_output2(var351, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[7] <= 17.760960578918457) {
                if (input[2] <= -9.370177745819092) {
                    set_output2(var351, 0.9971244903957979, 0.0028755096042021324);
                } else {
                    set_output2(var351, 0.9889969606591587, 0.011003039340841318);
                }
            } else {
                if (input[7] <= 17.764528274536133) {
                    set_output2(var351, 0.0, 1.0);
                } else {
                    set_output2(var351, 0.9639442337481972, 0.03605576625180278);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[7] <= 17.21262550354004) {
                    set_output2(var351, 0.4206008583690987, 0.5793991416309012);
                } else {
                    set_output2(var351, 0.0, 1.0);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var351, 0.0, 1.0);
                } else {
                    set_output2(var351, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var210, var351, 2, var209);
    double var352[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var352, 0.9882728516515734, 0.011727148348426603);
                } else {
                    set_output2(var352, 0.0, 1.0);
                }
            } else {
                if (input[4] <= 3.367400050163269) {
                    set_output2(var352, 0.9557043016044886, 0.04429569839551137);
                } else {
                    set_output2(var352, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var352, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var352, 0.987069135823163, 0.012930864176837064);
                }
            } else {
                set_output2(var352, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[4] <= 70.957350499928) {
                    set_output2(var352, 0.8379837983798379, 0.16201620162016203);
                } else {
                    set_output2(var352, 0.28, 0.72);
                }
            } else {
                if (input[7] <= 15.713719367980957) {
                    set_output2(var352, 0.5544554455445545, 0.44554455445544555);
                } else {
                    set_output2(var352, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var352, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var209, var352, 2, var208);
    double var353[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[5] <= 0.060350000858306885) {
                set_output2(var353, 0.7793035801863659, 0.22069641981363414);
            } else {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var353, 1.0, 0.0);
                } else {
                    set_output2(var353, 0.9896095358654377, 0.010390464134562348);
                }
            }
        } else {
            if (input[5] <= 246.27125549316406) {
                if (input[1] <= 10.020683288574219) {
                    set_output2(var353, 0.31257344300822565, 0.6874265569917745);
                } else {
                    set_output2(var353, 0.7341211225997046, 0.2658788774002954);
                }
            } else {
                set_output2(var353, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var353, 0.6945839554771277, 0.30541604452287224);
    }
    add_vectors(var208, var353, 2, var207);
    double var354[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[9] <= 6.277254581451416) {
            if (input[7] <= 16.334160804748535) {
                if (input[9] <= 2.150004506111145) {
                    set_output2(var354, 1.0, 0.0);
                } else {
                    set_output2(var354, 0.7471910112359551, 0.25280898876404495);
                }
            } else {
                set_output2(var354, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            set_output2(var354, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var354, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var354, 0.9882568690919772, 0.011743130908022809);
                } else {
                    set_output2(var354, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var354, 1.0, 0.0);
                } else {
                    set_output2(var354, 0.3121019108280255, 0.6878980891719745);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var354, 0.0, 1.0);
                } else {
                    set_output2(var354, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var207, var354, 2, var206);
    double var355[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var355, 0.7, 0.3);
                } else {
                    set_output2(var355, 0.9927898095975646, 0.007210190402435435);
                }
            } else {
                set_output2(var355, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var355, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var355, 0.987069135823163, 0.012930864176837049);
                }
            } else {
                set_output2(var355, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var355, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var355, 0.0, 1.0);
                } else {
                    set_output2(var355, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var355, 1.0, 0.0);
            } else {
                set_output2(var355, 0.0, 1.0);
            }
        }
    }
    add_vectors(var206, var355, 2, var205);
    double var356[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[6] <= 112.17279052734375) {
                    set_output2(var356, 1.0, 0.0);
                } else {
                    set_output2(var356, 0.9787470186790092, 0.021252981320990863);
                }
            } else {
                set_output2(var356, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var356, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var356, 0.987069135823163, 0.012930864176837044);
                }
            } else {
                set_output2(var356, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var356, 0.7232472324723247, 0.2767527675276753);
                } else {
                    set_output2(var356, 0.33972911963882624, 0.6602708803611739);
                }
            } else {
                set_output2(var356, 0.0, 1.0);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var356, 1.0, 0.0);
            } else {
                set_output2(var356, 0.0, 1.0);
            }
        }
    }
    add_vectors(var205, var356, 2, var204);
    double var357[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var357, 0.9974724780948102, 0.0025275219051898415);
                } else {
                    set_output2(var357, 0.8695904173106647, 0.13040958268933542);
                }
            } else {
                set_output2(var357, 1.0, 0.0);
            }
        } else {
            if (input[7] <= 16.308735847473145) {
                if (input[9] <= 0.3087494969367981) {
                    set_output2(var357, 0.9897633776304208, 0.010236622369579186);
                } else {
                    set_output2(var357, 0.9984238013008893, 0.001576198699110679);
                }
            } else {
                if (input[5] <= 0.00839999970048666) {
                    set_output2(var357, 0.0, 1.0);
                } else {
                    set_output2(var357, 0.985117142491358, 0.014882857508641972);
                }
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[5] <= 110.48614883422852) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var357, 0.7943640517897943, 0.20563594821020567);
                } else {
                    set_output2(var357, 0.28, 0.72);
                }
            } else {
                if (input[3] <= -43.79238700866699) {
                    set_output2(var357, 0.9532224532224532, 0.04677754677754678);
                } else {
                    set_output2(var357, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var357, 0.0, 1.0);
        }
    }
    add_vectors(var204, var357, 2, var203);
    double var358[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[6] <= 184.20638275146484) {
                set_output2(var358, 1.0, 0.0);
            } else {
                set_output2(var358, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var358, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[6] <= 132.94055938720703) {
                if (input[9] <= 1.4412494897842407) {
                    set_output2(var358, 0.9870579364059346, 0.012942063594065257);
                } else {
                    set_output2(var358, 0.3769230769230769, 0.6230769230769231);
                }
            } else {
                if (input[2] <= -12.434505939483643) {
                    set_output2(var358, 1.0, 0.0);
                } else {
                    set_output2(var358, 0.9903596509369677, 0.009640349063032438);
                }
            }
        } else {
            set_output2(var358, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var203, var358, 2, var202);
    double var359[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var359, 1.0, 0.0);
            } else {
                set_output2(var359, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var359, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var359, 0.9902130829332477, 0.009786917066752193);
                } else {
                    set_output2(var359, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var359, 0.9893564684289202, 0.010643531571079822);
                } else {
                    set_output2(var359, 0.9394075403949731, 0.0605924596050269);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var359, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var359, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var359, 0.0, 1.0);
                } else {
                    set_output2(var359, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var202, var359, 2, var201);
    double var360[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var360, 0.560546875, 0.439453125);
            } else {
                set_output2(var360, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.413290977478027) {
                    set_output2(var360, 0.9895561761679675, 0.010443823832032446);
                } else {
                    set_output2(var360, 0.9676633328324961, 0.032336667167503906);
                }
            } else {
                set_output2(var360, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var360, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var360, 0.0, 1.0);
                } else {
                    set_output2(var360, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var360, 0.0, 1.0);
            } else {
                set_output2(var360, 1.0, 0.0);
            }
        }
    }
    add_vectors(var201, var360, 2, var200);
    double var361[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[7] <= 17.760960578918457) {
            if (input[2] <= -9.370177745819092) {
                if (input[8] <= 0.9231399893760681) {
                    set_output2(var361, 0.9941989228044211, 0.005801077195578844);
                } else {
                    set_output2(var361, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -9.353523254394531) {
                    set_output2(var361, 0.5964125560538117, 0.40358744394618834);
                } else {
                    set_output2(var361, 0.989021911923395, 0.010978088076605066);
                }
            }
        } else {
            if (input[8] <= 1.2688884735107422) {
                if (input[7] <= 17.764528274536133) {
                    set_output2(var361, 0.0, 1.0);
                } else {
                    set_output2(var361, 0.987220265818471, 0.01277973418152902);
                }
            } else {
                set_output2(var361, 0.5964125560538117, 0.40358744394618834);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var361, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var361, 0.0, 1.0);
                } else {
                    set_output2(var361, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var361, 1.0, 0.0);
            } else {
                set_output2(var361, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var200, var361, 2, var199);
    double var362[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var362, 0.9882728516515733, 0.011727148348426605);
                } else {
                    set_output2(var362, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 0.5227085053920746) {
                    set_output2(var362, 1.0, 0.0);
                } else {
                    set_output2(var362, 0.9892125134843581, 0.010787486515641856);
                }
            }
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[5] <= 95.24220275878906) {
                    set_output2(var362, 0.9903552973106506, 0.009644702689349369);
                } else {
                    set_output2(var362, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -5.364761590957642) {
                    set_output2(var362, 0.9739706977540085, 0.026029302245991523);
                } else {
                    set_output2(var362, 0.9928777183375012, 0.007122281662498858);
                }
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[5] <= 110.48614883422852) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var362, 0.7943640517897944, 0.20563594821020567);
                } else {
                    set_output2(var362, 0.28, 0.72);
                }
            } else {
                if (input[9] <= 5.61060094833374) {
                    set_output2(var362, 0.9532224532224532, 0.04677754677754678);
                } else {
                    set_output2(var362, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var362, 0.0, 1.0);
        }
    }
    add_vectors(var199, var362, 2, var198);
    double var363[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var363, 1.0, 0.0);
            } else {
                set_output2(var363, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var363, 0.9926187156565243, 0.0073812843434757506);
                } else {
                    set_output2(var363, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var363, 0.9924123841132183, 0.0075876158867816896);
                } else {
                    set_output2(var363, 0.9842091965639209, 0.0157908034360791);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var363, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var363, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var363, 0.0, 1.0);
            } else {
                if (input[2] <= -4.315015077590942) {
                    set_output2(var363, 0.7590361445783133, 0.24096385542168675);
                } else {
                    set_output2(var363, 0.13461538461538464, 0.8653846153846153);
                }
            }
        }
    }
    add_vectors(var198, var363, 2, var197);
    double var364[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var364, 0.8657317618976578, 0.13426823810234223);
        } else {
            if (input[2] <= -12.753203868865967) {
                if (input[1] <= 3.527751088142395) {
                    set_output2(var364, 0.9664354441709555, 0.03356455582904452);
                } else {
                    set_output2(var364, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var364, 0.9927680579583904, 0.007231942041609543);
                } else {
                    set_output2(var364, 0.985568925707381, 0.014431074292619013);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var364, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var364, 0.0, 1.0);
                } else {
                    set_output2(var364, 0.5051546391752578, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var364, 0.0, 1.0);
            } else {
                set_output2(var364, 1.0, 0.0);
            }
        }
    }
    add_vectors(var197, var364, 2, var196);
    double var365[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var365, 0.9882728516515734, 0.011727148348426612);
                } else {
                    set_output2(var365, 0.0, 1.0);
                }
            } else {
                if (input[7] <= 17.991198539733887) {
                    set_output2(var365, 1.0, 0.0);
                } else {
                    set_output2(var365, 0.9174008810572687, 0.08259911894273128);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var365, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var365, 0.987069135823163, 0.012930864176837047);
                }
            } else {
                set_output2(var365, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var365, 0.6853146853146852, 0.3146853146853147);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var365, 0.1891891891891892, 0.8108108108108109);
                } else {
                    set_output2(var365, 1.0, 0.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var365, 0.0, 1.0);
            } else {
                set_output2(var365, 1.0, 0.0);
            }
        }
    }
    add_vectors(var196, var365, 2, var195);
    double var366[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[1] <= 5.673432111740112) {
                set_output2(var366, 1.0, 0.0);
            } else {
                set_output2(var366, 0.7751873438800999, 0.2248126561199001);
            }
        } else {
            set_output2(var366, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var366, 0.560546875, 0.439453125);
                } else {
                    set_output2(var366, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var366, 0.0, 1.0);
                } else {
                    set_output2(var366, 0.9898221960263142, 0.010177803973685808);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var366, 0.266304347826087, 0.7336956521739131);
                } else {
                    set_output2(var366, 0.6447368421052632, 0.35526315789473684);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var366, 0.0, 1.0);
                } else {
                    set_output2(var366, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var195, var366, 2, var194);
    double var367[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var367, 0.9904628182372109, 0.009537181762789077);
                } else {
                    set_output2(var367, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var367, 0.9865429575708421, 0.013457042429157942);
                } else {
                    set_output2(var367, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -41.139564514160156) {
                if (input[5] <= 246.27125549316406) {
                    set_output2(var367, 0.5997628224132819, 0.40023717758671806);
                } else {
                    set_output2(var367, 1.0, 0.0);
                }
            } else {
                set_output2(var367, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var367, 0.6945839554771278, 0.30541604452287224);
    }
    add_vectors(var194, var367, 2, var193);
    double var368[2];
    if (input[1] <= 5.543766021728516) {
        if (input[9] <= 1.7348750233650208) {
            if (input[4] <= 3.367400050163269) {
                if (input[6] <= 112.0386848449707) {
                    set_output2(var368, 1.0, 0.0);
                } else {
                    set_output2(var368, 0.9240826655419654, 0.0759173344580346);
                }
            } else {
                if (input[10] <= 0.5) {
                    set_output2(var368, 1.0, 0.0);
                } else {
                    set_output2(var368, 0.993676578045079, 0.006323421954921025);
                }
            }
        } else {
            set_output2(var368, 0.2697768762677485, 0.7302231237322515);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var368, 0.9865429575708421, 0.013457042429157956);
                } else {
                    set_output2(var368, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 7.8367509841918945) {
                    set_output2(var368, 0.0, 1.0);
                } else {
                    set_output2(var368, 0.7530186608122941, 0.2469813391877058);
                }
            }
        } else {
            set_output2(var368, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var193, var368, 2, var192);
    double var369[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[4] <= 2.857300043106079) {
                if (input[4] <= 2.4579999446868896) {
                    set_output2(var369, 1.0, 0.0);
                } else {
                    set_output2(var369, 0.0, 1.0);
                }
            } else {
                set_output2(var369, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var369, 0.8657317618976578, 0.13426823810234226);
            } else {
                if (input[3] <= -64.41340255737305) {
                    set_output2(var369, 1.0, 0.0);
                } else {
                    set_output2(var369, 0.9890238863190278, 0.01097611368097222);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[5] <= 11.175399780273438) {
                set_output2(var369, 0.4962686567164179, 0.503731343283582);
            } else {
                set_output2(var369, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var369, 0.0, 1.0);
            } else {
                if (input[7] <= 17.21262550354004) {
                    set_output2(var369, 0.6337741607324517, 0.3662258392675483);
                } else {
                    set_output2(var369, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var192, var369, 2, var191);
    double var370[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[3] <= -19.16687774658203) {
            if (input[7] <= 16.476778984069824) {
                if (input[9] <= 6.339050054550171) {
                    set_output2(var370, 0.8664292074799644, 0.13357079252003562);
                } else {
                    set_output2(var370, 0.0, 1.0);
                }
            } else {
                set_output2(var370, 0.0, 1.0);
            }
        } else {
            set_output2(var370, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[9] <= 0.23671899735927582) {
                if (input[6] <= 123.47669982910156) {
                    set_output2(var370, 0.9886058100873394, 0.011394189912660533);
                } else {
                    set_output2(var370, 0.9979089000698195, 0.002091099930180542);
                }
            } else {
                if (input[9] <= 0.23683100193738937) {
                    set_output2(var370, 0.0, 1.0);
                } else {
                    set_output2(var370, 0.9875278775034319, 0.012472122496568124);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var370, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var370, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -4.520525932312012) {
                    set_output2(var370, 1.0, 0.0);
                } else {
                    set_output2(var370, 0.6511627906976745, 0.3488372093023256);
                }
            }
        }
    }
    add_vectors(var191, var370, 2, var190);
    double var371[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[1] <= 9.928400039672852) {
                set_output2(var371, 0.9435736677115988, 0.05642633228840126);
            } else {
                set_output2(var371, 0.5792426367461431, 0.42075736325385693);
            }
        } else {
            if (input[7] <= 17.760960578918457) {
                if (input[7] <= 16.62748622894287) {
                    set_output2(var371, 0.9893624114291947, 0.010637588570805368);
                } else {
                    set_output2(var371, 0.995945279291415, 0.004054720708585054);
                }
            } else {
                if (input[9] <= 0.516086995601654) {
                    set_output2(var371, 0.9862909367859863, 0.013709063214013707);
                } else {
                    set_output2(var371, 0.7335701598579041, 0.2664298401420959);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var371, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[2] <= -6.903803110122681) {
                    set_output2(var371, 0.0, 1.0);
                } else {
                    set_output2(var371, 0.3858267716535433, 0.6141732283464567);
                }
            }
        } else {
            if (input[0] <= 100.0) {
                set_output2(var371, 0.0, 1.0);
            } else {
                set_output2(var371, 0.9463966646813579, 0.053603335318642045);
            }
        }
    }
    add_vectors(var190, var371, 2, var189);
    double var372[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[7] <= 15.96710205078125) {
                if (input[6] <= 184.20638275146484) {
                    set_output2(var372, 1.0, 0.0);
                } else {
                    set_output2(var372, 0.7471910112359551, 0.25280898876404495);
                }
            } else {
                set_output2(var372, 0.7382706475378054, 0.26172935246219464);
            }
        } else {
            set_output2(var372, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var372, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var372, 0.9882568690919773, 0.01174313090802274);
                } else {
                    set_output2(var372, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[14] <= 0.5) {
                    set_output2(var372, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var372, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 6.1142754554748535) {
                    set_output2(var372, 0.0, 1.0);
                } else {
                    set_output2(var372, 0.4423791821561338, 0.5576208178438662);
                }
            }
        }
    }
    add_vectors(var189, var372, 2, var188);
    double var373[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var373, 0.9974724780948102, 0.00252752190518984);
                } else {
                    set_output2(var373, 0.8695904173106646, 0.1304095826893354);
                }
            } else {
                set_output2(var373, 1.0, 0.0);
            }
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[8] <= 1.0516844987869263) {
                    set_output2(var373, 0.9901076816455262, 0.00989231835447377);
                } else {
                    set_output2(var373, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -5.364761590957642) {
                    set_output2(var373, 0.9739706977540085, 0.02602930224599152);
                } else {
                    set_output2(var373, 0.9928777183375012, 0.00712228166249886);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var373, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var373, 0.1891891891891892, 0.8108108108108109);
                } else {
                    set_output2(var373, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var373, 1.0, 0.0);
            } else {
                set_output2(var373, 0.0, 1.0);
            }
        }
    }
    add_vectors(var188, var373, 2, var187);
    double var374[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[1] <= 5.673432111740112) {
                set_output2(var374, 1.0, 0.0);
            } else {
                set_output2(var374, 0.7964261479303325, 0.2035738520696675);
            }
        } else {
            set_output2(var374, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var374, 1.0, 0.0);
                } else {
                    set_output2(var374, 0.9897105744654875, 0.010289425534512512);
                }
            } else {
                if (input[5] <= 172.15264892578125) {
                    set_output2(var374, 0.37608318890814557, 0.6239168110918544);
                } else {
                    set_output2(var374, 0.9463966646813579, 0.053603335318642045);
                }
            }
        } else {
            if (input[4] <= 199.75135040283203) {
                set_output2(var374, 0.3835616438356165, 0.6164383561643836);
            } else {
                if (input[3] <= -23.982419967651367) {
                    set_output2(var374, 0.9801849405548216, 0.019815059445178328);
                } else {
                    set_output2(var374, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var187, var374, 2, var186);
    double var375[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var375, 1.0, 0.0);
            } else {
                set_output2(var375, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var375, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var375, 0.560546875, 0.439453125);
                } else {
                    set_output2(var375, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var375, 0.0, 1.0);
                } else {
                    set_output2(var375, 0.9898221960263143, 0.010177803973685746);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var375, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var375, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.968702793121338) {
                    set_output2(var375, 0.0, 1.0);
                } else {
                    set_output2(var375, 0.468503937007874, 0.5314960629921259);
                }
            }
        }
    }
    add_vectors(var186, var375, 2, var185);
    double var376[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var376, 1.0, 0.0);
            } else {
                set_output2(var376, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[6] <= 132.94055938720703) {
                if (input[6] <= 112.50387954711914) {
                    set_output2(var376, 0.9930908158518178, 0.0069091841481821355);
                } else {
                    set_output2(var376, 0.9794194767973655, 0.020580523202634444);
                }
            } else {
                if (input[1] <= 5.543497562408447) {
                    set_output2(var376, 0.9990242929376154, 0.0009757070623845513);
                } else {
                    set_output2(var376, 0.991482986393697, 0.008517013606302965);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var376, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var376, 0.47572815533980584, 0.5242718446601942);
                } else {
                    set_output2(var376, 0.0, 1.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var376, 0.0, 1.0);
            } else {
                set_output2(var376, 1.0, 0.0);
            }
        }
    }
    add_vectors(var185, var376, 2, var184);
    double var377[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var377, 1.0, 0.0);
            } else {
                set_output2(var377, 0.7439180537772088, 0.25608194622279135);
            }
        } else {
            set_output2(var377, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var377, 1.0, 0.0);
            } else {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var377, 0.9896095358654377, 0.010390464134562339);
                } else {
                    set_output2(var377, 0.6062992125984252, 0.3937007874015749);
                }
            }
        } else {
            set_output2(var377, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var184, var377, 2, var183);
    double var378[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[6] <= 112.17279052734375) {
                    set_output2(var378, 1.0, 0.0);
                } else {
                    set_output2(var378, 0.9774121898881903, 0.022587810111809666);
                }
            } else {
                set_output2(var378, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var378, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var378, 0.987069135823163, 0.012930864176837047);
                }
            } else {
                set_output2(var378, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var378, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var378, 0.0, 1.0);
                } else {
                    set_output2(var378, 0.5051546391752578, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var378, 0.0, 1.0);
            } else {
                set_output2(var378, 1.0, 0.0);
            }
        }
    }
    add_vectors(var183, var378, 2, var182);
    double var379[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var379, 0.9928300183232865, 0.007169981676713488);
                } else {
                    set_output2(var379, 0.0, 1.0);
                }
            } else {
                set_output2(var379, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var379, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var379, 0.987069135823163, 0.012930864176837038);
                }
            } else {
                set_output2(var379, 1.0, 0.0);
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[9] <= 6.339050054550171) {
                if (input[5] <= 98.44184875488281) {
                    set_output2(var379, 0.5300261096605744, 0.46997389033942555);
                } else {
                    set_output2(var379, 0.9532224532224531, 0.04677754677754677);
                }
            } else {
                set_output2(var379, 0.199288256227758, 0.800711743772242);
            }
        } else {
            set_output2(var379, 0.0, 1.0);
        }
    }
    add_vectors(var182, var379, 2, var181);
    double var380[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var380, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var380, 0.0, 1.0);
            }
        } else {
            set_output2(var380, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var380, 0.9979329320669607, 0.002067067933039281);
                } else {
                    set_output2(var380, 0.9887643780849196, 0.011235621915080493);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var380, 0.3474623535973229, 0.6525376464026771);
                } else {
                    set_output2(var380, 0.9053627760252366, 0.09463722397476342);
                }
            }
        } else {
            set_output2(var380, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var181, var380, 2, var180);
    double var381[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[3] <= -19.16687774658203) {
            set_output2(var381, 0.6822033898305084, 0.31779661016949157);
        } else {
            set_output2(var381, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var381, 0.9904387041256992, 0.009561295874300818);
                } else {
                    set_output2(var381, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var381, 0.987069135823163, 0.012930864176837047);
                } else {
                    set_output2(var381, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[14] <= 0.5) {
                    set_output2(var381, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var381, 1.0, 0.0);
                }
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var381, 0.2491657397107898, 0.7508342602892103);
                } else {
                    set_output2(var381, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var180, var381, 2, var179);
    double var382[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var382, 0.6822033898305084, 0.31779661016949157);
        } else {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var382, 0.560546875, 0.439453125);
                } else {
                    set_output2(var382, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var382, 0.9896207527057277, 0.010379247294272385);
                } else {
                    set_output2(var382, 0.6062992125984252, 0.3937007874015749);
                }
            }
        }
    } else {
        set_output2(var382, 0.6945839554771278, 0.30541604452287224);
    }
    add_vectors(var179, var382, 2, var178);
    double var383[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[4] <= 3.367400050163269) {
                if (input[7] <= 17.010690689086914) {
                    set_output2(var383, 1.0, 0.0);
                } else {
                    set_output2(var383, 0.6682639144858091, 0.3317360855141909);
                }
            } else {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var383, 0.9855465801602724, 0.014453419839727635);
                } else {
                    set_output2(var383, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var383, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var383, 0.987069135823163, 0.01293086417683704);
                } else {
                    set_output2(var383, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[5] <= 11.175399780273438) {
                set_output2(var383, 0.4962686567164179, 0.503731343283582);
            } else {
                set_output2(var383, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var383, 0.0, 1.0);
            } else {
                if (input[7] <= 17.21262550354004) {
                    set_output2(var383, 0.6337741607324517, 0.3662258392675483);
                } else {
                    set_output2(var383, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var178, var383, 2, var177);
    double var384[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var384, 0.6232207647222997, 0.37677923527770024);
        } else {
            set_output2(var384, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[8] <= 0.9231399893760681) {
                    set_output2(var384, 0.9939403324737582, 0.006059667526241749);
                } else {
                    set_output2(var384, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var384, 0.9924211802748584, 0.007578819725141627);
                } else {
                    set_output2(var384, 0.9842303720697692, 0.015769627930230786);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[10] <= 0.5) {
                    set_output2(var384, 1.0, 0.0);
                } else {
                    set_output2(var384, 0.3715083798882682, 0.6284916201117319);
                }
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var384, 0.24916573971078979, 0.7508342602892102);
                } else {
                    set_output2(var384, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var177, var384, 2, var176);
    double var385[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var385, 0.8657317618976578, 0.13426823810234223);
        } else {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var385, 0.560546875, 0.439453125);
                } else {
                    set_output2(var385, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var385, 0.9882711945136788, 0.011728805486321173);
                } else {
                    set_output2(var385, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[2] <= -13.028204441070557) {
                    set_output2(var385, 1.0, 0.0);
                } else {
                    set_output2(var385, 0.44508670520231214, 0.5549132947976878);
                }
            } else {
                if (input[9] <= 1.7081345319747925) {
                    set_output2(var385, 0.5964125560538117, 0.40358744394618834);
                } else {
                    set_output2(var385, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var385, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var176, var385, 2, var175);
    double var386[2];
    if (input[14] <= 0.5) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var386, 0.8943165805542508, 0.10568341944574916);
            } else {
                set_output2(var386, 0.3894165535956581, 0.6105834464043419);
            }
        } else {
            if (input[9] <= 1.4178645014762878) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var386, 0.9916381952376113, 0.008361804762388618);
                } else {
                    set_output2(var386, 0.9743969048702777, 0.02560309512972234);
                }
            } else {
                set_output2(var386, 0.29577464788732394, 0.7042253521126761);
            }
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[9] <= 0.39608000218868256) {
                if (input[1] <= 5.543497562408447) {
                    set_output2(var386, 1.0, 0.0);
                } else {
                    set_output2(var386, 0.9945359835836661, 0.0054640164163338705);
                }
            } else {
                if (input[7] <= 16.306074142456055) {
                    set_output2(var386, 0.9960968670853139, 0.0039031329146861854);
                } else {
                    set_output2(var386, 0.977809977544456, 0.022190022455544106);
                }
            }
        } else {
            if (input[3] <= -54.30373573303223) {
                set_output2(var386, 1.0, 0.0);
            } else {
                if (input[7] <= 17.07250213623047) {
                    set_output2(var386, 0.5051546391752577, 0.4948453608247423);
                } else {
                    set_output2(var386, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var175, var386, 2, var174);
    double var387[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var387, 0.9928300183232865, 0.007169981676713484);
                } else {
                    set_output2(var387, 0.0, 1.0);
                }
            } else {
                set_output2(var387, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var387, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var387, 0.9924211802748584, 0.007578819725141627);
                } else {
                    set_output2(var387, 0.9842303720697692, 0.01576962793023079);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var387, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[2] <= -2.5843780040740967) {
                    set_output2(var387, 0.378140703517588, 0.6218592964824121);
                } else {
                    set_output2(var387, 0.0, 1.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var387, 0.0, 1.0);
            } else {
                set_output2(var387, 1.0, 0.0);
            }
        }
    }
    add_vectors(var174, var387, 2, var173);
    double var388[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var388, 1.0, 0.0);
            } else {
                set_output2(var388, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var388, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var388, 0.9882568690919772, 0.011743130908022812);
                } else {
                    set_output2(var388, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var388, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var388, 0.0, 1.0);
                } else {
                    set_output2(var388, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var388, 1.0, 0.0);
            } else {
                set_output2(var388, 0.0, 1.0);
            }
        }
    }
    add_vectors(var173, var388, 2, var172);
    double var389[2];
    if (input[1] <= 5.543766021728516) {
        if (input[5] <= 21.988649368286133) {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var389, 0.7335701598579041, 0.2664298401420959);
                } else {
                    set_output2(var389, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.288375854492188) {
                    set_output2(var389, 0.9768050204244682, 0.02319497957553187);
                } else {
                    set_output2(var389, 0.7324613555291319, 0.267538644470868);
                }
            }
        } else {
            if (input[9] <= 4.642098009586334) {
                set_output2(var389, 1.0, 0.0);
            } else {
                set_output2(var389, 0.0, 1.0);
            }
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var389, 0.6329027872195785, 0.36709721278042146);
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var389, 0.9889851265356375, 0.01101487346436253);
                } else {
                    set_output2(var389, 0.7307807358659886, 0.2692192641340115);
                }
            }
        } else {
            set_output2(var389, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var172, var389, 2, var171);
    double var390[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var390, 0.9928300183232865, 0.007169981676713484);
                } else {
                    set_output2(var390, 0.0, 1.0);
                }
            } else {
                set_output2(var390, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[2] <= -1.3221570253372192) {
                    set_output2(var390, 0.9868085480608563, 0.013191451939143728);
                } else {
                    set_output2(var390, 0.0, 1.0);
                }
            } else {
                set_output2(var390, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var390, 0.6853146853146853, 0.31468531468531474);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var390, 0.1891891891891892, 0.8108108108108109);
                } else {
                    set_output2(var390, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 10.067442417144775) {
                set_output2(var390, 0.7634691195795007, 0.23653088042049933);
            } else {
                set_output2(var390, 1.0, 0.0);
            }
        }
    }
    add_vectors(var171, var390, 2, var170);
    double var391[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var391, 0.8657317618976578, 0.13426823810234223);
        } else {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var391, 0.560546875, 0.439453125);
                } else {
                    set_output2(var391, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var391, 0.9882711945136788, 0.011728805486321183);
                } else {
                    set_output2(var391, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[5] <= 11.175399780273438) {
                set_output2(var391, 0.4962686567164179, 0.503731343283582);
            } else {
                set_output2(var391, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var391, 0.0, 1.0);
            } else {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var391, 0.7815533980582524, 0.2184466019417476);
                } else {
                    set_output2(var391, 0.40079893475366174, 0.5992010652463382);
                }
            }
        }
    }
    add_vectors(var170, var391, 2, var169);
    double var392[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[2] <= -3.9435800313949585) {
                set_output2(var392, 0.791979660233445, 0.20802033976655496);
            } else {
                set_output2(var392, 1.0, 0.0);
            }
        } else {
            set_output2(var392, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[5] <= 21.988649368286133) {
                if (input[6] <= 108.99774169921875) {
                    set_output2(var392, 0.9969970871745593, 0.0030029128254406867);
                } else {
                    set_output2(var392, 0.9797220858502846, 0.020277914149715366);
                }
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var392, 0.9942616313544462, 0.0057383686455538645);
                } else {
                    set_output2(var392, 0.7779483524316025, 0.2220516475683975);
                }
            }
        } else {
            if (input[9] <= 1.1798875033855438) {
                if (input[8] <= 1.069337010383606) {
                    set_output2(var392, 1.0, 0.0);
                } else {
                    set_output2(var392, 0.7313432835820896, 0.26865671641791045);
                }
            } else {
                set_output2(var392, 0.29319371727748694, 0.706806282722513);
            }
        }
    }
    add_vectors(var169, var392, 2, var168);
    double var393[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[7] <= 16.334160804748535) {
                    set_output2(var393, 1.0, 0.0);
                } else {
                    set_output2(var393, 0.560546875, 0.439453125);
                }
            } else {
                set_output2(var393, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var393, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[9] <= 0.23671899735927582) {
                if (input[10] <= 0.5) {
                    set_output2(var393, 0.9976833380524595, 0.002316661947540523);
                } else {
                    set_output2(var393, 0.9899400876330142, 0.010059912366985746);
                }
            } else {
                if (input[9] <= 0.23683100193738937) {
                    set_output2(var393, 0.0, 1.0);
                } else {
                    set_output2(var393, 0.987527877503432, 0.012472122496568122);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var393, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var393, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var393, 0.0, 1.0);
                } else {
                    set_output2(var393, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var168, var393, 2, var167);
    double var394[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[2] <= -3.9435800313949585) {
                set_output2(var394, 0.7919796602334451, 0.20802033976655496);
            } else {
                set_output2(var394, 1.0, 0.0);
            }
        } else {
            set_output2(var394, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var394, 1.0, 0.0);
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var394, 0.0, 1.0);
                } else {
                    set_output2(var394, 0.9898111946442208, 0.010188805355779217);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var394, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var394, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.968702793121338) {
                    set_output2(var394, 0.0, 1.0);
                } else {
                    set_output2(var394, 0.46850393700787407, 0.5314960629921259);
                }
            }
        }
    }
    add_vectors(var167, var394, 2, var166);
    double var395[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var395, 0.9928169328835492, 0.007183067116450807);
                } else {
                    set_output2(var395, 0.0, 1.0);
                }
            } else {
                set_output2(var395, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var395, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var395, 0.987069135823163, 0.012930864176837052);
                }
            } else {
                set_output2(var395, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var395, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var395, 0.47572815533980584, 0.5242718446601942);
                } else {
                    set_output2(var395, 0.0, 1.0);
                }
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var395, 1.0, 0.0);
            } else {
                set_output2(var395, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var166, var395, 2, var165);
    double var396[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[3] <= -19.16687774658203) {
            set_output2(var396, 0.6822033898305084, 0.31779661016949157);
        } else {
            set_output2(var396, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var396, 0.9972482239578794, 0.0027517760421205623);
                } else {
                    set_output2(var396, 0.9887783113852344, 0.011221688614765609);
                }
            } else {
                if (input[3] <= -54.70915985107422) {
                    set_output2(var396, 0.8288159771754636, 0.1711840228245364);
                } else {
                    set_output2(var396, 0.37814070351758794, 0.621859296482412);
                }
            }
        } else {
            set_output2(var396, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var165, var396, 2, var164);
    double var397[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var397, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var397, 0.0, 1.0);
            }
        } else {
            set_output2(var397, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var397, 1.0, 0.0);
            } else {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var397, 0.9896095358654377, 0.010390464134562336);
                } else {
                    set_output2(var397, 0.6062992125984252, 0.3937007874015749);
                }
            }
        } else {
            set_output2(var397, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var164, var397, 2, var163);
    double var398[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[9] <= 0.1185465008020401) {
                    set_output2(var398, 1.0, 0.0);
                } else {
                    set_output2(var398, 0.9828812721116902, 0.01711872788830981);
                }
            } else {
                set_output2(var398, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var398, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var398, 0.987069135823163, 0.012930864176837038);
                }
            } else {
                set_output2(var398, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var398, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var398, 0.0, 1.0);
                } else {
                    set_output2(var398, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var398, 1.0, 0.0);
            } else {
                set_output2(var398, 0.0, 1.0);
            }
        }
    }
    add_vectors(var163, var398, 2, var162);
    double var399[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var399, 0.9928169328835492, 0.0071830671164508095);
                } else {
                    set_output2(var399, 0.0, 1.0);
                }
            } else {
                set_output2(var399, 1.0, 0.0);
            }
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[1] <= 12.596567630767822) {
                    set_output2(var399, 0.9904018320713518, 0.009598167928648179);
                } else {
                    set_output2(var399, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 5.5530314445495605) {
                    set_output2(var399, 0.0, 1.0);
                } else {
                    set_output2(var399, 0.984242612411452, 0.015757387588548066);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var399, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var399, 0.22450933460986122, 0.7754906653901389);
                } else {
                    set_output2(var399, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 10.067442417144775) {
                set_output2(var399, 0.7634691195795007, 0.23653088042049933);
            } else {
                set_output2(var399, 1.0, 0.0);
            }
        }
    }
    add_vectors(var162, var399, 2, var161);
    double var400[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var400, 1.0, 0.0);
            } else {
                set_output2(var400, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var400, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var400, 0.9882568690919772, 0.011743130908022821);
                } else {
                    set_output2(var400, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[1] <= 10.020683288574219) {
            if (input[3] <= -51.78119659423828) {
                set_output2(var400, 0.6853146853146854, 0.3146853146853147);
            } else {
                set_output2(var400, 0.1106719367588933, 0.8893280632411067);
            }
        } else {
            if (input[8] <= 0.9787624776363373) {
                if (input[3] <= -46.10698318481445) {
                    set_output2(var400, 0.6757925072046109, 0.324207492795389);
                } else {
                    set_output2(var400, 0.0, 1.0);
                }
            } else {
                set_output2(var400, 1.0, 0.0);
            }
        }
    }
    add_vectors(var161, var400, 2, var160);
    double var401[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var401, 0.6822033898305084, 0.31779661016949157);
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[6] <= 132.94055938720703) {
                    set_output2(var401, 0.9870579364059348, 0.012942063594065262);
                } else {
                    set_output2(var401, 0.99386216886401, 0.006137831135989934);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var401, 0.347462353597323, 0.6525376464026771);
                } else {
                    set_output2(var401, 0.9053627760252366, 0.09463722397476342);
                }
            }
        }
    } else {
        set_output2(var401, 0.6945839554771277, 0.3054160445228723);
    }
    add_vectors(var160, var401, 2, var159);
    double var402[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var402, 0.560546875, 0.439453125);
            } else {
                set_output2(var402, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.413290977478027) {
                    set_output2(var402, 0.9895561761679676, 0.010443823832032418);
                } else {
                    set_output2(var402, 0.9676633328324961, 0.03233666716750389);
                }
            } else {
                set_output2(var402, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var402, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var402, 0.0, 1.0);
                } else {
                    set_output2(var402, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var402, 1.0, 0.0);
            } else {
                set_output2(var402, 0.0, 1.0);
            }
        }
    }
    add_vectors(var159, var402, 2, var158);
    double var403[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var403, 0.6822033898305084, 0.31779661016949157);
        } else {
            if (input[6] <= 132.94055938720703) {
                if (input[9] <= 1.4412494897842407) {
                    set_output2(var403, 0.9870579364059348, 0.01294206359406525);
                } else {
                    set_output2(var403, 0.3769230769230769, 0.6230769230769231);
                }
            } else {
                if (input[1] <= 4.8168299198150635) {
                    set_output2(var403, 0.9985781676056027, 0.0014218323943973549);
                } else {
                    set_output2(var403, 0.9903167332014685, 0.009683266798531452);
                }
            }
        }
    } else {
        set_output2(var403, 0.6945839554771278, 0.30541604452287224);
    }
    add_vectors(var158, var403, 2, var157);
    double var404[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[2] <= -5.9743804931640625) {
                set_output2(var404, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var404, 0.0, 1.0);
            }
        } else {
            set_output2(var404, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var404, 1.0, 0.0);
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var404, 0.0, 1.0);
                } else {
                    set_output2(var404, 0.9868565967078138, 0.01314340329218625);
                }
            }
        } else {
            set_output2(var404, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var157, var404, 2, var156);
    double var405[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var405, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var405, 0.0, 1.0);
            }
        } else {
            set_output2(var405, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var405, 0.9927898095975646, 0.007210190402435428);
                } else {
                    set_output2(var405, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var405, 0.9924123841132183, 0.007587615886781683);
                } else {
                    set_output2(var405, 0.9842091965639208, 0.015790803436079123);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var405, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var405, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -4.520525932312012) {
                    set_output2(var405, 1.0, 0.0);
                } else {
                    set_output2(var405, 0.6511627906976745, 0.3488372093023256);
                }
            }
        }
    }
    add_vectors(var156, var405, 2, var155);
    double var406[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[9] <= 6.277254581451416) {
            if (input[2] <= -3.9435800313949585) {
                if (input[7] <= 16.334160804748535) {
                    set_output2(var406, 0.8839159035212176, 0.11608409647878241);
                } else {
                    set_output2(var406, 0.0, 1.0);
                }
            } else {
                set_output2(var406, 1.0, 0.0);
            }
        } else {
            set_output2(var406, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[9] <= 0.23671899735927582) {
                if (input[8] <= 1.0516844987869263) {
                    set_output2(var406, 0.992055729937187, 0.007944270062812868);
                } else {
                    set_output2(var406, 1.0, 0.0);
                }
            } else {
                if (input[8] <= 0.9666895270347595) {
                    set_output2(var406, 0.9913814313306332, 0.008618568669366795);
                } else {
                    set_output2(var406, 0.9799553077602654, 0.02004469223973464);
                }
            }
        } else {
            if (input[10] <= 0.5) {
                if (input[9] <= 5.61060094833374) {
                    set_output2(var406, 0.8220640569395018, 0.1779359430604983);
                } else {
                    set_output2(var406, 0.199288256227758, 0.800711743772242);
                }
            } else {
                set_output2(var406, 0.29577464788732394, 0.7042253521126761);
            }
        }
    }
    add_vectors(var155, var406, 2, var154);
    double var407[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var407, 0.8657317618976578, 0.13426823810234226);
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var407, 0.9902130829332477, 0.0097869170667522);
                } else {
                    set_output2(var407, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var407, 0.9924211802748583, 0.007578819725141636);
                } else {
                    set_output2(var407, 0.9842303720697692, 0.015769627930230783);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var407, 0.7943640517897944, 0.20563594821020564);
                } else {
                    set_output2(var407, 0.3525179856115109, 0.6474820143884892);
                }
            } else {
                set_output2(var407, 0.0, 1.0);
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var407, 0.0, 1.0);
            } else {
                set_output2(var407, 1.0, 0.0);
            }
        }
    }
    add_vectors(var154, var407, 2, var153);
    double var408[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[7] <= 17.16385555267334) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var408, 0.9936409241856851, 0.006359075814314986);
                } else {
                    set_output2(var408, 1.0, 0.0);
                }
            } else {
                if (input[5] <= 18.207850456237793) {
                    set_output2(var408, 0.8695904173106646, 0.1304095826893354);
                } else {
                    set_output2(var408, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var408, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var408, 0.987069135823163, 0.012930864176837037);
                } else {
                    set_output2(var408, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[8] <= 0.9768004715442657) {
                if (input[14] <= 0.5) {
                    set_output2(var408, 0.42492012779552724, 0.5750798722044729);
                } else {
                    set_output2(var408, 1.0, 0.0);
                }
            } else {
                set_output2(var408, 1.0, 0.0);
            }
        } else {
            if (input[6] <= 135.9388885498047) {
                set_output2(var408, 0.0, 1.0);
            } else {
                if (input[5] <= 18.7502498626709) {
                    set_output2(var408, 0.8316183348924228, 0.16838166510757718);
                } else {
                    set_output2(var408, 0.2840095465393795, 0.7159904534606205);
                }
            }
        }
    }
    add_vectors(var153, var408, 2, var152);
    double var409[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var409, 1.0, 0.0);
            } else {
                set_output2(var409, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var409, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var409, 0.9882568690919772, 0.011743130908022823);
                } else {
                    set_output2(var409, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var409, 0.7232472324723247, 0.2767527675276753);
                } else {
                    set_output2(var409, 0.33972911963882624, 0.6602708803611739);
                }
            } else {
                set_output2(var409, 0.0, 1.0);
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var409, 1.0, 0.0);
            } else {
                set_output2(var409, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var152, var409, 2, var151);
    double var410[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var410, 1.0, 0.0);
            } else {
                set_output2(var410, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var410, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[5] <= 21.988649368286133) {
                if (input[9] <= 1.6179354786872864) {
                    set_output2(var410, 0.9874037678479421, 0.012596232152057766);
                } else {
                    set_output2(var410, 0.38682816048448143, 0.6131718395155186);
                }
            } else {
                if (input[1] <= 5.543497562408447) {
                    set_output2(var410, 0.9990132704165314, 0.0009867295834685614);
                } else {
                    set_output2(var410, 0.9886545392936162, 0.01134546070638377);
                }
            }
        } else {
            set_output2(var410, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var151, var410, 2, var150);
    double var411[2];
    if (input[1] <= 5.543766021728516) {
        if (input[9] <= 1.7348750233650208) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var411, 0.9928803101020489, 0.0071196898979510965);
                } else {
                    set_output2(var411, 0.0, 1.0);
                }
            } else {
                set_output2(var411, 1.0, 0.0);
            }
        } else {
            set_output2(var411, 0.2697768762677485, 0.7302231237322515);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var411, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var411, 0.987069135823163, 0.01293086417683704);
                } else {
                    set_output2(var411, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 151.5938491821289) {
                if (input[7] <= 17.21262550354004) {
                    set_output2(var411, 0.5263157894736842, 0.47368421052631576);
                } else {
                    set_output2(var411, 0.0, 1.0);
                }
            } else {
                if (input[2] <= 9.712319999933243) {
                    set_output2(var411, 0.9457177322074789, 0.05428226779252111);
                } else {
                    set_output2(var411, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var150, var411, 2, var149);
    double var412[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var412, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var412, 1.0, 0.0);
            }
        } else {
            if (input[7] <= 17.760960578918457) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var412, 0.9985882065046031, 0.0014117934953967902);
                } else {
                    set_output2(var412, 0.9893430980377091, 0.010656901962290853);
                }
            } else {
                if (input[9] <= 0.516086995601654) {
                    set_output2(var412, 0.9862909367859863, 0.013709063214013705);
                } else {
                    set_output2(var412, 0.7335701598579041, 0.2664298401420959);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var412, 0.7943640517897944, 0.20563594821020567);
                } else {
                    set_output2(var412, 0.3525179856115108, 0.6474820143884893);
                }
            } else {
                set_output2(var412, 0.0, 1.0);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var412, 1.0, 0.0);
            } else {
                set_output2(var412, 0.0, 1.0);
            }
        }
    }
    add_vectors(var149, var412, 2, var148);
    double var413[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[6] <= 184.20638275146484) {
                set_output2(var413, 1.0, 0.0);
            } else {
                set_output2(var413, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var413, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[6] <= 132.94055938720703) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var413, 0.9874422401183218, 0.01255775988167821);
                } else {
                    set_output2(var413, 0.970477568740955, 0.029522431259044895);
                }
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var413, 0.994142262854136, 0.005857737145863966);
                } else {
                    set_output2(var413, 0.8313975271637317, 0.1686024728362683);
                }
            }
        } else {
            set_output2(var413, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var148, var413, 2, var147);
    double var414[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[5] <= 6.019749879837036) {
                    set_output2(var414, 0.9653312788906009, 0.034668721109399066);
                } else {
                    set_output2(var414, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.991198539733887) {
                    set_output2(var414, 1.0, 0.0);
                } else {
                    set_output2(var414, 0.9174008810572687, 0.08259911894273127);
                }
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var414, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[8] <= 0.9165079891681671) {
                    set_output2(var414, 0.9913293823148299, 0.008670617685170069);
                } else {
                    set_output2(var414, 0.98410029767776, 0.015899702322240125);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var414, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var414, 0.0, 1.0);
                } else {
                    set_output2(var414, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var414, 1.0, 0.0);
            } else {
                set_output2(var414, 0.0, 1.0);
            }
        }
    }
    add_vectors(var147, var414, 2, var146);
    double var415[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var415, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var415, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var415, 1.0, 0.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var415, 0.9902859646657135, 0.009714035334286535);
                } else {
                    set_output2(var415, 0.9428462564297961, 0.057153743570203835);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var415, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var415, 0.0, 1.0);
                } else {
                    set_output2(var415, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var415, 1.0, 0.0);
            } else {
                set_output2(var415, 0.0, 1.0);
            }
        }
    }
    add_vectors(var146, var415, 2, var145);
    double var416[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var416, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var416, 0.0, 1.0);
            }
        } else {
            set_output2(var416, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var416, 0.9926187156565243, 0.007381284343475749);
                } else {
                    set_output2(var416, 1.0, 0.0);
                }
            } else {
                set_output2(var416, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[2] <= -0.9100589752197266) {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var416, 0.984530198120152, 0.015469801879847958);
                } else {
                    set_output2(var416, 0.9982046248862929, 0.0017953751137070922);
                }
            } else {
                set_output2(var416, 0.3323442136498516, 0.6676557863501483);
            }
        }
    }
    add_vectors(var145, var416, 2, var144);
    double var417[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[3] <= -19.16687774658203) {
            set_output2(var417, 0.6822033898305084, 0.31779661016949157);
        } else {
            set_output2(var417, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var417, 0.9902130829332478, 0.009786917066752191);
                } else {
                    set_output2(var417, 1.0, 0.0);
                }
            } else {
                set_output2(var417, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[3] <= -64.41350936889648) {
                set_output2(var417, 1.0, 0.0);
            } else {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var417, 0.9822760214263204, 0.01772397857367967);
                } else {
                    set_output2(var417, 0.9980509017827752, 0.0019490982172248293);
                }
            }
        }
    }
    add_vectors(var144, var417, 2, var143);
    double var418[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var418, 0.7232472324723248, 0.27675276752767525);
            } else {
                set_output2(var418, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var418, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var418, 0.9882568690919773, 0.011743130908022778);
                } else {
                    set_output2(var418, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[4] <= 70.957350499928) {
                    set_output2(var418, 0.8379837983798379, 0.16201620162016203);
                } else {
                    set_output2(var418, 0.28, 0.72);
                }
            } else {
                if (input[3] <= -43.79238700866699) {
                    set_output2(var418, 1.0, 0.0);
                } else {
                    set_output2(var418, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var418, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var143, var418, 2, var142);
    double var419[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[1] <= 5.673432111740112) {
                set_output2(var419, 1.0, 0.0);
            } else {
                set_output2(var419, 0.7751873438800999, 0.2248126561199001);
            }
        } else {
            set_output2(var419, 0.0, 1.0);
        }
    } else {
        if (input[14] <= 0.5) {
            if (input[3] <= -64.4062271118164) {
                set_output2(var419, 1.0, 0.0);
            } else {
                if (input[6] <= 108.99774169921875) {
                    set_output2(var419, 0.9926269697255184, 0.007373030274481539);
                } else {
                    set_output2(var419, 0.9763303234855786, 0.023669676514421333);
                }
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.434505939483643) {
                    set_output2(var419, 1.0, 0.0);
                } else {
                    set_output2(var419, 0.9920073048389713, 0.007992695161028773);
                }
            } else {
                if (input[9] <= 5.61060094833374) {
                    set_output2(var419, 0.8220640569395018, 0.17793594306049826);
                } else {
                    set_output2(var419, 0.199288256227758, 0.800711743772242);
                }
            }
        }
    }
    add_vectors(var142, var419, 2, var141);
    double var420[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var420, 1.0, 0.0);
            } else {
                set_output2(var420, 0.7439180537772087, 0.2560819462227913);
            }
        } else {
            set_output2(var420, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var420, 0.9928153273017563, 0.007184672698243738);
                } else {
                    set_output2(var420, 1.0, 0.0);
                }
            } else {
                set_output2(var420, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[2] <= -0.9100589752197266) {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var420, 0.9845301981201521, 0.015469801879847962);
                } else {
                    set_output2(var420, 0.9982046248862929, 0.0017953751137070898);
                }
            } else {
                set_output2(var420, 0.3323442136498516, 0.6676557863501483);
            }
        }
    }
    add_vectors(var141, var420, 2, var140);
    double var421[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var421, 0.8657317618976578, 0.13426823810234226);
        } else {
            if (input[6] <= 132.94055938720703) {
                if (input[6] <= 111.77765655517578) {
                    set_output2(var421, 0.9933743957203502, 0.006625604279649665);
                } else {
                    set_output2(var421, 0.9793572217812223, 0.02064277821877773);
                }
            } else {
                if (input[1] <= 5.543497562408447) {
                    set_output2(var421, 0.9990242929376154, 0.0009757070623845513);
                } else {
                    set_output2(var421, 0.9914934853031302, 0.008506514696869779);
                }
            }
        }
    } else {
        if (input[1] <= 10.020683288574219) {
            if (input[2] <= -7.96181058883667) {
                set_output2(var421, 0.6853146853146854, 0.3146853146853147);
            } else {
                set_output2(var421, 0.1106719367588933, 0.8893280632411067);
            }
        } else {
            if (input[2] <= -4.609246015548706) {
                set_output2(var421, 1.0, 0.0);
            } else {
                if (input[3] <= -41.98731803894043) {
                    set_output2(var421, 0.664804469273743, 0.33519553072625696);
                } else {
                    set_output2(var421, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var140, var421, 2, var139);
    double var422[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[6] <= 112.17279052734375) {
                    set_output2(var422, 1.0, 0.0);
                } else {
                    set_output2(var422, 0.9787470186790092, 0.021252981320990863);
                }
            } else {
                set_output2(var422, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.703011989593506) {
                    set_output2(var422, 0.9868085480608563, 0.013191451939143752);
                } else {
                    set_output2(var422, 0.0, 1.0);
                }
            } else {
                set_output2(var422, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var422, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var422, 0.47572815533980584, 0.5242718446601942);
                } else {
                    set_output2(var422, 0.0, 1.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var422, 0.0, 1.0);
            } else {
                set_output2(var422, 1.0, 0.0);
            }
        }
    }
    add_vectors(var139, var422, 2, var138);
    double var423[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[3] <= -19.980281829833984) {
                set_output2(var423, 0.8110831234256927, 0.1889168765743073);
            } else {
                set_output2(var423, 1.0, 0.0);
            }
        } else {
            set_output2(var423, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                if (input[8] <= 0.1918649971485138) {
                    set_output2(var423, 0.9910456670978012, 0.008954332902198783);
                } else {
                    set_output2(var423, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var423, 0.9882711945136788, 0.011728805486321126);
                } else {
                    set_output2(var423, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var423, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var423, 1.0, 0.0);
                }
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var423, 0.2491657397107898, 0.7508342602892103);
                } else {
                    set_output2(var423, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var138, var423, 2, var137);
    double var424[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var424, 1.0, 0.0);
            } else {
                set_output2(var424, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var424, 1.0, 0.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var424, 0.9902859646657135, 0.00971403533428653);
                } else {
                    set_output2(var424, 0.9428462564297961, 0.057153743570203815);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var424, 0.6853146853146853, 0.31468531468531474);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var424, 0.0, 1.0);
                } else {
                    set_output2(var424, 0.4553706505295008, 0.5446293494704992);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var424, 1.0, 0.0);
            } else {
                set_output2(var424, 0.0, 1.0);
            }
        }
    }
    add_vectors(var137, var424, 2, var136);
    double var425[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[7] <= 17.760960578918457) {
            if (input[1] <= 8.246700286865234) {
                if (input[5] <= 2.3686500787734985) {
                    set_output2(var425, 0.978871586196103, 0.021128413803897017);
                } else {
                    set_output2(var425, 0.9979710216807605, 0.002028978319239487);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var425, 0.9856323332645267, 0.01436766673547339);
                } else {
                    set_output2(var425, 1.0, 0.0);
                }
            }
        } else {
            if (input[8] <= 1.2688884735107422) {
                if (input[8] <= 0.42980749905109406) {
                    set_output2(var425, 0.9247050949552413, 0.07529490504475862);
                } else {
                    set_output2(var425, 1.0, 0.0);
                }
            } else {
                set_output2(var425, 0.5964125560538117, 0.40358744394618834);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var425, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var425, 0.47572815533980584, 0.5242718446601942);
                } else {
                    set_output2(var425, 0.0, 1.0);
                }
            }
        } else {
            if (input[0] <= 100.0) {
                set_output2(var425, 0.0, 1.0);
            } else {
                set_output2(var425, 0.9463966646813579, 0.053603335318642045);
            }
        }
    }
    add_vectors(var136, var425, 2, var135);
    double var426[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var426, 0.6232207647222997, 0.37677923527770024);
        } else {
            set_output2(var426, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 21.988649368286133) {
                if (input[9] <= 1.4755340218544006) {
                    set_output2(var426, 0.9909632203066481, 0.009036779693351924);
                } else {
                    set_output2(var426, 0.3300248138957817, 0.6699751861042184);
                }
            } else {
                if (input[9] <= 4.642098009586334) {
                    set_output2(var426, 1.0, 0.0);
                } else {
                    set_output2(var426, 0.0, 1.0);
                }
            }
        } else {
            if (input[3] <= -64.41350936889648) {
                set_output2(var426, 1.0, 0.0);
            } else {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var426, 0.9880884695834107, 0.0119115304165893);
                } else {
                    set_output2(var426, 0.5773407690049898, 0.42265923099501024);
                }
            }
        }
    }
    add_vectors(var135, var426, 2, var134);
    double var427[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var427, 0.9928169328835492, 0.007183067116450811);
                } else {
                    set_output2(var427, 0.0, 1.0);
                }
            } else {
                set_output2(var427, 1.0, 0.0);
            }
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[1] <= 12.596567630767822) {
                    set_output2(var427, 0.9904018320713518, 0.00959816792864819);
                } else {
                    set_output2(var427, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -5.364761590957642) {
                    set_output2(var427, 0.9739706977540086, 0.02602930224599152);
                } else {
                    set_output2(var427, 0.9928777183375012, 0.007122281662498857);
                }
            }
        }
    } else {
        if (input[1] <= 10.020683288574219) {
            if (input[8] <= 0.6116749942302704) {
                set_output2(var427, 0.8448275862068966, 0.15517241379310345);
            } else {
                if (input[9] <= 6.277254581451416) {
                    set_output2(var427, 0.4553706505295008, 0.5446293494704992);
                } else {
                    set_output2(var427, 0.0, 1.0);
                }
            }
        } else {
            if (input[5] <= 151.5938491821289) {
                if (input[9] <= 3.002187967300415) {
                    set_output2(var427, 0.7395079594790159, 0.2604920405209841);
                } else {
                    set_output2(var427, 0.22813036020583194, 0.7718696397941681);
                }
            } else {
                set_output2(var427, 1.0, 0.0);
            }
        }
    }
    add_vectors(var134, var427, 2, var133);
    double var428[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[6] <= 112.17279052734375) {
                    set_output2(var428, 1.0, 0.0);
                } else {
                    set_output2(var428, 0.9774121898881903, 0.022587810111809662);
                }
            } else {
                set_output2(var428, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 5.552222490310669) {
                    set_output2(var428, 0.0, 1.0);
                } else {
                    set_output2(var428, 0.9868085480608563, 0.013191451939143731);
                }
            } else {
                set_output2(var428, 1.0, 0.0);
            }
        }
    } else {
        if (input[6] <= 101.93917083740234) {
            set_output2(var428, 0.0, 1.0);
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var428, 0.7232472324723247, 0.2767527675276753);
                } else {
                    set_output2(var428, 0.32330827067669177, 0.6766917293233083);
                }
            } else {
                if (input[2] <= -4.520525932312012) {
                    set_output2(var428, 1.0, 0.0);
                } else {
                    set_output2(var428, 0.6511627906976745, 0.3488372093023256);
                }
            }
        }
    }
    add_vectors(var133, var428, 2, var132);
    double var429[2];
    if (input[1] <= 5.543766021728516) {
        if (input[9] <= 1.7348750233650208) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var429, 0.9974818835511038, 0.002518116448896224);
                } else {
                    set_output2(var429, 0.8695904173106647, 0.13040958268933542);
                }
            } else {
                set_output2(var429, 1.0, 0.0);
            }
        } else {
            set_output2(var429, 0.2697768762677485, 0.7302231237322515);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var429, 0.6329027872195785, 0.3670972127804215);
            } else {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var429, 0.9845520539553545, 0.015447946044645486);
                } else {
                    set_output2(var429, 0.9982046248862929, 0.0017953751137070914);
                }
            }
        } else {
            set_output2(var429, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var132, var429, 2, var131);
    double var430[2];
    if (input[3] <= -44.63666534423828) {
        if (input[9] <= 1.3964359760284424) {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var430, 0.9901771925324835, 0.009822807467516511);
                } else {
                    set_output2(var430, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var430, 0.9871708000779693, 0.012829199922030592);
                } else {
                    set_output2(var430, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 7.8367509841918945) {
                set_output2(var430, 0.35656836461126007, 0.6434316353887399);
            } else {
                if (input[5] <= 151.5938491821289) {
                    set_output2(var430, 0.5866819747416763, 0.4133180252583238);
                } else {
                    set_output2(var430, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 0.6606000065803528) {
            if (input[7] <= 15.511346340179443) {
                set_output2(var430, 1.0, 0.0);
            } else {
                set_output2(var430, 0.4249201277955272, 0.5750798722044729);
            }
        } else {
            if (input[9] <= 1.4168090224266052) {
                if (input[4] <= 199.75135040283203) {
                    set_output2(var430, 0.9644296893526203, 0.035570310647379644);
                } else {
                    set_output2(var430, 0.994509148368912, 0.005490851631087972);
                }
            } else {
                set_output2(var430, 0.31210191082802546, 0.6878980891719745);
            }
        }
    }
    add_vectors(var131, var430, 2, var130);
    double var431[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var431, 0.9815568731386104, 0.01844312686138966);
                } else {
                    set_output2(var431, 0.9992260075232069, 0.000773992476793136);
                }
            } else {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var431, 0.5964125560538117, 0.40358744394618834);
                } else {
                    set_output2(var431, 0.9887783113852344, 0.0112216886147656);
                }
            }
        } else {
            if (input[5] <= 246.27125549316406) {
                if (input[9] <= 6.277254581451416) {
                    set_output2(var431, 0.5997628224132819, 0.40023717758671806);
                } else {
                    set_output2(var431, 0.0, 1.0);
                }
            } else {
                set_output2(var431, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var431, 0.6945839554771277, 0.30541604452287224);
    }
    add_vectors(var130, var431, 2, var129);
    double var432[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[9] <= 0.1185465008020401) {
                    set_output2(var432, 1.0, 0.0);
                } else {
                    set_output2(var432, 0.9828812721116902, 0.01711872788830981);
                }
            } else {
                set_output2(var432, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[2] <= -1.3221570253372192) {
                    set_output2(var432, 0.9868085480608563, 0.013191451939143731);
                } else {
                    set_output2(var432, 0.0, 1.0);
                }
            } else {
                set_output2(var432, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var432, 1.0, 0.0);
            } else {
                set_output2(var432, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[7] <= 17.21262550354004) {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var432, 0.0, 1.0);
                } else {
                    set_output2(var432, 0.6060311284046693, 0.3939688715953307);
                }
            } else {
                set_output2(var432, 0.0, 1.0);
            }
        }
    }
    add_vectors(var129, var432, 2, var128);
    double var433[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var433, 1.0, 0.0);
            } else {
                set_output2(var433, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var433, 0.9927898095975646, 0.007210190402435428);
                } else {
                    set_output2(var433, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var433, 0.9870506314315944, 0.012949368568405634);
                } else {
                    set_output2(var433, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[8] <= 0.7983885109424591) {
                if (input[2] <= -13.028204441070557) {
                    set_output2(var433, 1.0, 0.0);
                } else {
                    set_output2(var433, 0.32145305003427005, 0.67854694996573);
                }
            } else {
                if (input[9] <= 1.9647245407104492) {
                    set_output2(var433, 0.5764705882352941, 0.4235294117647059);
                } else {
                    set_output2(var433, 0.918552036199095, 0.08144796380090497);
                }
            }
        } else {
            set_output2(var433, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var128, var433, 2, var127);
    double var434[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var434, 1.0, 0.0);
            } else {
                set_output2(var434, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var434, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var434, 0.9987765888033407, 0.001223411196659277);
                } else {
                    set_output2(var434, 0.9896207527057276, 0.0103792472942724);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var434, 0.34746235359732286, 0.6525376464026771);
                } else {
                    set_output2(var434, 0.9053627760252366, 0.09463722397476342);
                }
            }
        } else {
            set_output2(var434, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var127, var434, 2, var126);
    double var435[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[5] <= 0.05140000022947788) {
                if (input[1] <= 9.928400039672852) {
                    set_output2(var435, 1.0, 0.0);
                } else {
                    set_output2(var435, 0.5792426367461431, 0.42075736325385693);
                }
            } else {
                set_output2(var435, 0.0, 1.0);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var435, 1.0, 0.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var435, 0.9902859646657135, 0.009714035334286535);
                } else {
                    set_output2(var435, 0.9428462564297961, 0.05715374357020383);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var435, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var435, 0.47572815533980584, 0.5242718446601942);
                } else {
                    set_output2(var435, 0.0, 1.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var435, 0.0, 1.0);
            } else {
                set_output2(var435, 1.0, 0.0);
            }
        }
    }
    add_vectors(var126, var435, 2, var125);
    double var436[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[5] <= 0.060350000858306885) {
                set_output2(var436, 0.7793035801863659, 0.22069641981363414);
            } else {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var436, 1.0, 0.0);
                } else {
                    set_output2(var436, 0.9896095358654377, 0.01039046413456234);
                }
            }
        } else {
            if (input[9] <= 6.277254581451416) {
                if (input[10] <= 0.5) {
                    set_output2(var436, 0.8611646741226379, 0.1388353258773621);
                } else {
                    set_output2(var436, 0.4789473684210526, 0.5210526315789474);
                }
            } else {
                set_output2(var436, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var436, 0.6945839554771277, 0.3054160445228723);
    }
    add_vectors(var125, var436, 2, var124);
    double var437[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[3] <= -19.16687774658203) {
            set_output2(var437, 0.6822033898305084, 0.31779661016949157);
        } else {
            set_output2(var437, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[9] <= 0.23671899735927582) {
                if (input[5] <= 8.603449821472168) {
                    set_output2(var437, 0.9879135361233069, 0.012086463876693123);
                } else {
                    set_output2(var437, 0.9971150918095318, 0.002884908190468109);
                }
            } else {
                if (input[9] <= 0.23683100193738937) {
                    set_output2(var437, 0.0, 1.0);
                } else {
                    set_output2(var437, 0.9875476877439728, 0.012452312256027186);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var437, 0.266304347826087, 0.7336956521739131);
                } else {
                    set_output2(var437, 0.6447368421052632, 0.35526315789473684);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var437, 0.0, 1.0);
                } else {
                    set_output2(var437, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var124, var437, 2, var123);
    double var438[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[1] <= 10.331897735595703) {
                    set_output2(var438, 1.0, 0.0);
                } else {
                    set_output2(var438, 0.8088360237892948, 0.1911639762107052);
                }
            } else {
                set_output2(var438, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var438, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var438, 1.0, 0.0);
                } else {
                    set_output2(var438, 0.9897105744654875, 0.010289425534512524);
                }
            } else {
                if (input[9] <= 4.584504842758179) {
                    set_output2(var438, 0.6946564885496183, 0.3053435114503818);
                } else {
                    set_output2(var438, 0.0, 1.0);
                }
            }
        } else {
            if (input[3] <= -35.49544334411621) {
                set_output2(var438, 0.3894165535956581, 0.6105834464043419);
            } else {
                if (input[5] <= 227.9127426147461) {
                    set_output2(var438, 0.9801727176594994, 0.019827282340500525);
                } else {
                    set_output2(var438, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var123, var438, 2, var122);
    double var439[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var439, 0.6232207647222997, 0.37677923527770024);
        } else {
            set_output2(var439, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[6] <= 132.94055938720703) {
                if (input[9] <= 1.4412494897842407) {
                    set_output2(var439, 0.9870579364059348, 0.012942063594065271);
                } else {
                    set_output2(var439, 0.3769230769230769, 0.6230769230769231);
                }
            } else {
                if (input[2] <= -12.434505939483643) {
                    set_output2(var439, 1.0, 0.0);
                } else {
                    set_output2(var439, 0.9903596509369675, 0.009640349063032443);
                }
            }
        } else {
            set_output2(var439, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var122, var439, 2, var121);
    double var440[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var440, 0.9928300183232865, 0.007169981676713478);
                } else {
                    set_output2(var440, 0.0, 1.0);
                }
            } else {
                set_output2(var440, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[2] <= -1.3221570253372192) {
                    set_output2(var440, 0.9868085480608563, 0.013191451939143737);
                } else {
                    set_output2(var440, 0.0, 1.0);
                }
            } else {
                set_output2(var440, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var440, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var440, 0.22450933460986122, 0.7754906653901389);
                } else {
                    set_output2(var440, 1.0, 0.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var440, 0.0, 1.0);
            } else {
                set_output2(var440, 1.0, 0.0);
            }
        }
    }
    add_vectors(var121, var440, 2, var120);
    double var441[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[7] <= 17.10542869567871) {
                    set_output2(var441, 0.9936409241856851, 0.006359075814314986);
                } else {
                    set_output2(var441, 0.8600311041990668, 0.1399688958009331);
                }
            } else {
                if (input[4] <= 3.367400050163269) {
                    set_output2(var441, 0.9557043016044886, 0.044295698395511375);
                } else {
                    set_output2(var441, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var441, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var441, 0.987069135823163, 0.012930864176837064);
                }
            } else {
                set_output2(var441, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[8] <= 0.9768004715442657) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var441, 0.4962686567164179, 0.503731343283582);
                } else {
                    set_output2(var441, 1.0, 0.0);
                }
            } else {
                set_output2(var441, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -4.315015077590942) {
                if (input[9] <= 6.277254581451416) {
                    set_output2(var441, 0.7159090909090909, 0.28409090909090906);
                } else {
                    set_output2(var441, 0.0, 1.0);
                }
            } else {
                set_output2(var441, 0.12147505422993493, 0.878524945770065);
            }
        }
    }
    add_vectors(var120, var441, 2, var119);
    double var442[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var442, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var442, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var442, 0.9927898095975646, 0.007210190402435434);
                } else {
                    set_output2(var442, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var442, 0.9870506314315944, 0.012949368568405634);
                } else {
                    set_output2(var442, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[5] <= 110.48614883422852) {
                if (input[2] <= -13.028204441070557) {
                    set_output2(var442, 1.0, 0.0);
                } else {
                    set_output2(var442, 0.45298534143725416, 0.5470146585627458);
                }
            } else {
                if (input[5] <= 290.6855926513672) {
                    set_output2(var442, 0.8114525139664804, 0.18854748603351953);
                } else {
                    set_output2(var442, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var442, 0.0, 1.0);
        }
    }
    add_vectors(var119, var442, 2, var118);
    double var443[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var443, 0.9882728516515733, 0.011727148348426605);
                } else {
                    set_output2(var443, 0.0, 1.0);
                }
            } else {
                if (input[7] <= 17.991198539733887) {
                    set_output2(var443, 1.0, 0.0);
                } else {
                    set_output2(var443, 0.9174008810572687, 0.08259911894273128);
                }
            }
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[5] <= 95.24220275878906) {
                    set_output2(var443, 0.9903552973106506, 0.009644702689349357);
                } else {
                    set_output2(var443, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -5.364761590957642) {
                    set_output2(var443, 0.9739706977540085, 0.026029302245991523);
                } else {
                    set_output2(var443, 0.9928777183375012, 0.007122281662498858);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[2] <= -13.028204441070557) {
                    set_output2(var443, 1.0, 0.0);
                } else {
                    set_output2(var443, 0.44508670520231214, 0.5549132947976878);
                }
            } else {
                if (input[3] <= -43.79238700866699) {
                    set_output2(var443, 1.0, 0.0);
                } else {
                    set_output2(var443, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var443, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var118, var443, 2, var117);
    double var444[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var444, 1.0, 0.0);
            } else {
                set_output2(var444, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var444, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[6] <= 132.94055938720703) {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var444, 0.9853759151478095, 0.01462408485219038);
                } else {
                    set_output2(var444, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.760549545288086) {
                    set_output2(var444, 0.9946821636989894, 0.005317836301010519);
                } else {
                    set_output2(var444, 0.951370627859227, 0.048629372140773015);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var444, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var444, 1.0, 0.0);
                }
            } else {
                if (input[3] <= -46.46929168701172) {
                    set_output2(var444, 1.0, 0.0);
                } else {
                    set_output2(var444, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var117, var444, 2, var116);
    double var445[2];
    if (input[3] <= -44.63666534423828) {
        if (input[7] <= 17.760960578918457) {
            if (input[14] <= 0.5) {
                if (input[9] <= 1.6726675033569336) {
                    set_output2(var445, 0.9876373626373623, 0.012362637362637601);
                } else {
                    set_output2(var445, 0.4429160935350756, 0.5570839064649244);
                }
            } else {
                if (input[9] <= 6.339050054550171) {
                    set_output2(var445, 0.9939290781417581, 0.006070921858241868);
                } else {
                    set_output2(var445, 0.0, 1.0);
                }
            }
        } else {
            if (input[9] <= 0.516086995601654) {
                if (input[7] <= 17.764528274536133) {
                    set_output2(var445, 0.0, 1.0);
                } else {
                    set_output2(var445, 1.0, 0.0);
                }
            } else {
                set_output2(var445, 0.7335701598579041, 0.2664298401420959);
            }
        }
    } else {
        if (input[9] <= 1.4168090224266052) {
            if (input[3] <= -44.63024139404297) {
                set_output2(var445, 0.0, 1.0);
            } else {
                if (input[5] <= 0.6811999976634979) {
                    set_output2(var445, 0.8658718330849479, 0.13412816691505214);
                } else {
                    set_output2(var445, 0.9910630945524597, 0.008936905447540342);
                }
            }
        } else {
            set_output2(var445, 0.3442622950819672, 0.6557377049180328);
        }
    }
    add_vectors(var116, var445, 2, var115);
    double var446[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var446, 0.9882728516515734, 0.011727148348426608);
                } else {
                    set_output2(var446, 0.0, 1.0);
                }
            } else {
                if (input[4] <= 3.367400050163269) {
                    set_output2(var446, 0.9557043016044887, 0.04429569839551138);
                } else {
                    set_output2(var446, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[7] <= 16.308735847473145) {
                    set_output2(var446, 0.9913722615388951, 0.00862773846110485);
                } else {
                    set_output2(var446, 0.98109031824294, 0.01890968175705998);
                }
            } else {
                set_output2(var446, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var446, 0.7943640517897944, 0.20563594821020567);
                } else {
                    set_output2(var446, 0.35251798561151076, 0.6474820143884892);
                }
            } else {
                set_output2(var446, 0.0, 1.0);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var446, 1.0, 0.0);
            } else {
                set_output2(var446, 0.0, 1.0);
            }
        }
    }
    add_vectors(var115, var446, 2, var114);
    double var447[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[9] <= 0.23671899735927582) {
            if (input[6] <= 123.47669982910156) {
                if (input[2] <= -5.353416442871094) {
                    set_output2(var447, 0.9948536580484499, 0.0051463419515500744);
                } else {
                    set_output2(var447, 0.9795872079836698, 0.020412792016330245);
                }
            } else {
                if (input[4] <= 0.042100001126527786) {
                    set_output2(var447, 0.5964125560538117, 0.40358744394618834);
                } else {
                    set_output2(var447, 0.9979119056181339, 0.0020880943818661025);
                }
            }
        } else {
            if (input[2] <= -12.434505939483643) {
                set_output2(var447, 1.0, 0.0);
            } else {
                if (input[9] <= 0.23683100193738937) {
                    set_output2(var447, 0.0, 1.0);
                } else {
                    set_output2(var447, 0.9853730808066506, 0.014626919193349311);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var447, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var447, 0.0, 1.0);
                } else {
                    set_output2(var447, 0.5051546391752578, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var447, 0.0, 1.0);
            } else {
                set_output2(var447, 1.0, 0.0);
            }
        }
    }
    add_vectors(var114, var447, 2, var113);
    double var448[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[6] <= 109.77089309692383) {
                set_output2(var448, 1.0, 0.0);
            } else {
                set_output2(var448, 0.7283155564499899, 0.2716844435500101);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var448, 0.9926187156565243, 0.007381284343475749);
                } else {
                    set_output2(var448, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var448, 0.9870506314315944, 0.012949368568405638);
                } else {
                    set_output2(var448, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var448, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var448, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var448, 0.0, 1.0);
            } else {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var448, 0.0, 1.0);
                } else {
                    set_output2(var448, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var113, var448, 2, var112);
    double var449[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[3] <= -48.08561134338379) {
                set_output2(var449, 0.8598785614198972, 0.14012143858010276);
            } else {
                set_output2(var449, 0.3715083798882682, 0.6284916201117319);
            }
        } else {
            set_output2(var449, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[2] <= -9.043847560882568) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var449, 1.0, 0.0);
                } else {
                    set_output2(var449, 0.9924692998457042, 0.007530700154295699);
                }
            } else {
                set_output2(var449, 0.0, 1.0);
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var449, 1.0, 0.0);
                } else {
                    set_output2(var449, 0.9880727693593861, 0.011927230640613845);
                }
            } else {
                if (input[10] <= 0.5) {
                    set_output2(var449, 0.7408207343412526, 0.2591792656587473);
                } else {
                    set_output2(var449, 0.2538860103626943, 0.7461139896373057);
                }
            }
        }
    }
    add_vectors(var112, var449, 2, var111);
    double var450[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var450, 0.560546875, 0.439453125);
            } else {
                set_output2(var450, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.744715690612793) {
                set_output2(var450, 0.0, 1.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var450, 0.9880752725488189, 0.011924727451181163);
                } else {
                    set_output2(var450, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[5] <= 11.175399780273438) {
                set_output2(var450, 0.4962686567164179, 0.503731343283582);
            } else {
                set_output2(var450, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -4.315015077590942) {
                if (input[9] <= 6.277254581451416) {
                    set_output2(var450, 0.7159090909090909, 0.28409090909090906);
                } else {
                    set_output2(var450, 0.0, 1.0);
                }
            } else {
                set_output2(var450, 0.12147505422993493, 0.878524945770065);
            }
        }
    }
    add_vectors(var111, var450, 2, var110);
    double var451[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[6] <= 184.20638275146484) {
                set_output2(var451, 1.0, 0.0);
            } else {
                set_output2(var451, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var451, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var451, 0.9815568731386103, 0.018443126861389655);
                } else {
                    set_output2(var451, 0.9992262590484705, 0.0007737409515294384);
                }
            } else {
                set_output2(var451, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[1] <= 12.717926025390625) {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var451, 1.0, 0.0);
                } else {
                    set_output2(var451, 0.9822760214263203, 0.01772397857367968);
                }
            } else {
                if (input[9] <= 1.495212972164154) {
                    set_output2(var451, 1.0, 0.0);
                } else {
                    set_output2(var451, 0.8389982110912344, 0.16100178890876563);
                }
            }
        }
    }
    add_vectors(var110, var451, 2, var109);
    double var452[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var452, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var452, 0.0, 1.0);
            }
        } else {
            set_output2(var452, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var452, 1.0, 0.0);
                } else {
                    set_output2(var452, 0.9896095358654378, 0.0103904641345623);
                }
            } else {
                if (input[14] <= 0.5) {
                    set_output2(var452, 0.31818181818181823, 0.6818181818181819);
                } else {
                    set_output2(var452, 0.7846031112883926, 0.21539688871160753);
                }
            }
        } else {
            set_output2(var452, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var109, var452, 2, var108);
    double var453[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var453, 1.0, 0.0);
            } else {
                set_output2(var453, 0.7439180537772088, 0.25608194622279135);
            }
        } else {
            set_output2(var453, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[2] <= -9.043847560882568) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var453, 0.9878022335465684, 0.012197766453431609);
                } else {
                    set_output2(var453, 1.0, 0.0);
                }
            } else {
                set_output2(var453, 0.0, 1.0);
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var453, 0.9924123841132183, 0.007587615886781686);
                } else {
                    set_output2(var453, 0.9842091965639209, 0.01579080343607912);
                }
            } else {
                if (input[10] <= 0.5) {
                    set_output2(var453, 0.7408207343412526, 0.2591792656587473);
                } else {
                    set_output2(var453, 0.2538860103626943, 0.7461139896373057);
                }
            }
        }
    }
    add_vectors(var108, var453, 2, var107);
    double var454[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[5] <= 0.060350000858306885) {
            if (input[5] <= 0.05140000022947788) {
                if (input[9] <= 6.277254581451416) {
                    set_output2(var454, 0.8294970161977835, 0.1705029838022165);
                } else {
                    set_output2(var454, 0.0, 1.0);
                }
            } else {
                set_output2(var454, 0.0, 1.0);
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var454, 0.9939706248844368, 0.006029375115563163);
                } else {
                    set_output2(var454, 0.9871174958258836, 0.012882504174116313);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var454, 0.34746235359732297, 0.6525376464026771);
                } else {
                    set_output2(var454, 0.9053627760252366, 0.09463722397476342);
                }
            }
        }
    } else {
        set_output2(var454, 0.6945839554771277, 0.3054160445228723);
    }
    add_vectors(var107, var454, 2, var106);
    double var455[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var455, 1.0, 0.0);
            } else {
                set_output2(var455, 0.7439180537772087, 0.25608194622279135);
            }
        } else {
            set_output2(var455, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[6] <= 132.94055938720703) {
                if (input[1] <= 9.135465621948242) {
                    set_output2(var455, 0.9954469570496282, 0.004553042950371856);
                } else {
                    set_output2(var455, 0.9830102959005169, 0.016989704099482996);
                }
            } else {
                if (input[1] <= 5.543497562408447) {
                    set_output2(var455, 0.9990242929376154, 0.0009757070623845518);
                } else {
                    set_output2(var455, 0.991482986393697, 0.00851701360630297);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var455, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var455, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 1.7081345319747925) {
                    set_output2(var455, 0.0, 1.0);
                } else {
                    set_output2(var455, 0.946396664681358, 0.05360333531864206);
                }
            }
        }
    }
    add_vectors(var106, var455, 2, var105);
    double var456[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var456, 0.8657317618976578, 0.13426823810234223);
        } else {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var456, 0.560546875, 0.439453125);
                } else {
                    set_output2(var456, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var456, 0.990296602801644, 0.009703397198355996);
                } else {
                    set_output2(var456, 0.9428462564297961, 0.05715374357020384);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var456, 0.7232472324723247, 0.2767527675276753);
                } else {
                    set_output2(var456, 0.33972911963882624, 0.6602708803611739);
                }
            } else {
                set_output2(var456, 0.0, 1.0);
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var456, 1.0, 0.0);
            } else {
                set_output2(var456, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var105, var456, 2, var104);
    double var457[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[2] <= -5.9743804931640625) {
                set_output2(var457, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var457, 0.0, 1.0);
            }
        } else {
            set_output2(var457, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[2] <= -9.043847560882568) {
                if (input[9] <= 1.7348750233650208) {
                    set_output2(var457, 0.9979350346608256, 0.0020649653391744145);
                } else {
                    set_output2(var457, 0.4249201277955272, 0.5750798722044729);
                }
            } else {
                set_output2(var457, 0.0, 1.0);
            }
        } else {
            if (input[2] <= -0.9100589752197266) {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var457, 0.9868443171104275, 0.01315568288957256);
                } else {
                    set_output2(var457, 0.9394075403949731, 0.060592459605026905);
                }
            } else {
                set_output2(var457, 0.3323442136498516, 0.6676557863501483);
            }
        }
    }
    add_vectors(var104, var457, 2, var103);
    double var458[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[4] <= 2.857300043106079) {
                if (input[5] <= 0.22884999960660934) {
                    set_output2(var458, 0.0, 1.0);
                } else {
                    set_output2(var458, 1.0, 0.0);
                }
            } else {
                set_output2(var458, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.413290977478027) {
                    set_output2(var458, 0.9895561761679675, 0.010443823832032453);
                } else {
                    set_output2(var458, 0.9676633328324961, 0.03233666716750391);
                }
            } else {
                set_output2(var458, 1.0, 0.0);
            }
        }
    } else {
        if (input[6] <= 101.93917083740234) {
            set_output2(var458, 0.0, 1.0);
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 89.85874938964844) {
                    set_output2(var458, 0.7943640517897944, 0.20563594821020564);
                } else {
                    set_output2(var458, 0.3379310344827587, 0.6620689655172414);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var458, 0.0, 1.0);
                } else {
                    set_output2(var458, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var103, var458, 2, var102);
    double var459[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[3] <= -48.08561134338379) {
                set_output2(var459, 0.8598785614198973, 0.14012143858010276);
            } else {
                set_output2(var459, 0.3715083798882682, 0.6284916201117319);
            }
        } else {
            set_output2(var459, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var459, 0.9927898095975645, 0.007210190402435431);
                } else {
                    set_output2(var459, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var459, 0.9870506314315944, 0.012949368568405634);
                } else {
                    set_output2(var459, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var459, 1.0, 0.0);
                } else {
                    set_output2(var459, 0.31210191082802546, 0.6878980891719745);
                }
            } else {
                if (input[0] <= 100.0) {
                    set_output2(var459, 0.0, 1.0);
                } else {
                    set_output2(var459, 0.946396664681358, 0.053603335318642045);
                }
            }
        }
    }
    add_vectors(var102, var459, 2, var101);
    double var460[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var460, 0.6232207647222997, 0.37677923527770024);
        } else {
            set_output2(var460, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var460, 0.9902130829332478, 0.009786917066752197);
                } else {
                    set_output2(var460, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var460, 0.987069135823163, 0.012930864176837042);
                } else {
                    set_output2(var460, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[2] <= -0.4311224967241287) {
                    set_output2(var460, 0.8374717832957109, 0.16252821670428896);
                } else {
                    set_output2(var460, 0.0, 1.0);
                }
            } else {
                if (input[7] <= 15.968702793121338) {
                    set_output2(var460, 0.0, 1.0);
                } else {
                    set_output2(var460, 0.468503937007874, 0.5314960629921259);
                }
            }
        }
    }
    add_vectors(var101, var460, 2, var100);
    double var461[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var461, 1.0, 0.0);
            } else {
                set_output2(var461, 0.7439180537772087, 0.25608194622279135);
            }
        } else {
            set_output2(var461, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var461, 0.9926187156565243, 0.007381284343475753);
                } else {
                    set_output2(var461, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var461, 0.9870506314315943, 0.012949368568405652);
                } else {
                    set_output2(var461, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[7] <= 17.21262550354004) {
                    set_output2(var461, 0.4206008583690988, 0.5793991416309013);
                } else {
                    set_output2(var461, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 1.7081345319747925) {
                    set_output2(var461, 0.0, 1.0);
                } else {
                    set_output2(var461, 0.946396664681358, 0.053603335318642045);
                }
            }
        }
    }
    add_vectors(var100, var461, 2, var99);
    double var462[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[6] <= 112.17279052734375) {
                    set_output2(var462, 1.0, 0.0);
                } else {
                    set_output2(var462, 0.9774121898881903, 0.022587810111809662);
                }
            } else {
                set_output2(var462, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var462, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var462, 0.987069135823163, 0.012930864176837038);
                }
            } else {
                set_output2(var462, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var462, 0.42492012779552724, 0.5750798722044729);
            } else {
                set_output2(var462, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var462, 0.0, 1.0);
            } else {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var462, 0.0, 1.0);
                } else {
                    set_output2(var462, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var99, var462, 2, var98);
    double var463[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[7] <= 16.334160804748535) {
                    set_output2(var463, 1.0, 0.0);
                } else {
                    set_output2(var463, 0.560546875, 0.439453125);
                }
            } else {
                set_output2(var463, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var463, 0.0, 1.0);
        }
    } else {
        if (input[3] <= -35.84962844848633) {
            if (input[2] <= -0.9100589752197266) {
                if (input[5] <= 21.988649368286133) {
                    set_output2(var463, 0.9846395009019366, 0.01536049909806331);
                } else {
                    set_output2(var463, 0.992436339188167, 0.0075636608118331075);
                }
            } else {
                set_output2(var463, 0.23728813559322035, 0.7627118644067796);
            }
        } else {
            if (input[3] <= -35.49544334411621) {
                set_output2(var463, 0.3894165535956581, 0.6105834464043419);
            } else {
                if (input[9] <= 1.967637449502945) {
                    set_output2(var463, 1.0, 0.0);
                } else {
                    set_output2(var463, 0.3835616438356165, 0.6164383561643836);
                }
            }
        }
    }
    add_vectors(var98, var463, 2, var97);
    double var464[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var464, 0.8657317618976578, 0.13426823810234226);
        } else {
            if (input[6] <= 132.94055938720703) {
                if (input[6] <= 111.77765655517578) {
                    set_output2(var464, 0.9933743957203504, 0.0066256042796496665);
                } else {
                    set_output2(var464, 0.9793572217812222, 0.02064277821877776);
                }
            } else {
                if (input[9] <= 0.24306800216436386) {
                    set_output2(var464, 0.9977627133812113, 0.0022372866187887757);
                } else {
                    set_output2(var464, 0.9891200870393035, 0.010879912960696484);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var464, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var464, 0.0, 1.0);
                } else {
                    set_output2(var464, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var464, 1.0, 0.0);
            } else {
                set_output2(var464, 0.0, 1.0);
            }
        }
    }
    add_vectors(var97, var464, 2, var96);
    double var465[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[6] <= 109.77089309692383) {
                set_output2(var465, 1.0, 0.0);
            } else {
                set_output2(var465, 0.7283155564499899, 0.2716844435500101);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var465, 0.9927898095975646, 0.007210190402435428);
                } else {
                    set_output2(var465, 1.0, 0.0);
                }
            } else {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var465, 1.0, 0.0);
                } else {
                    set_output2(var465, 0.9880727693593861, 0.01192723064061385);
                }
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[9] <= 6.339050054550171) {
                if (input[10] <= 0.5) {
                    set_output2(var465, 0.8967495219885278, 0.1032504780114723);
                } else {
                    set_output2(var465, 0.457286432160804, 0.542713567839196);
                }
            } else {
                set_output2(var465, 0.199288256227758, 0.800711743772242);
            }
        } else {
            set_output2(var465, 0.0, 1.0);
        }
    }
    add_vectors(var96, var465, 2, var95);
    double var466[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[1] <= 3.8464345932006836) {
                set_output2(var466, 0.0, 1.0);
            } else {
                if (input[2] <= -5.9743804931640625) {
                    set_output2(var466, 0.8839159035212176, 0.11608409647878239);
                } else {
                    set_output2(var466, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var466, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[6] <= 132.94055938720703) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var466, 1.0, 0.0);
                } else {
                    set_output2(var466, 0.9818297263657733, 0.0181702736342267);
                }
            } else {
                if (input[1] <= 4.8168299198150635) {
                    set_output2(var466, 0.9985781676056026, 0.001421832394397354);
                } else {
                    set_output2(var466, 0.990306395452536, 0.009693604547463977);
                }
            }
        } else {
            set_output2(var466, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var95, var466, 2, var94);
    double var467[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[4] <= 2.857300043106079) {
                if (input[7] <= 17.010690689086914) {
                    set_output2(var467, 1.0, 0.0);
                } else {
                    set_output2(var467, 0.7335701598579041, 0.2664298401420959);
                }
            } else {
                set_output2(var467, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.744715690612793) {
                set_output2(var467, 0.0, 1.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var467, 0.9880752725488189, 0.011924727451181154);
                } else {
                    set_output2(var467, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var467, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[3] <= -62.23621368408203) {
                    set_output2(var467, 0.7313432835820896, 0.26865671641791045);
                } else {
                    set_output2(var467, 0.24257425742574257, 0.7574257425742574);
                }
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var467, 1.0, 0.0);
            } else {
                set_output2(var467, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var94, var467, 2, var93);
    double var468[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var468, 1.0, 0.0);
            } else {
                set_output2(var468, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var468, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[6] <= 132.94055938720703) {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var468, 0.9853759151478095, 0.01462408485219038);
                } else {
                    set_output2(var468, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.24306800216436386) {
                    set_output2(var468, 0.9977627133812111, 0.002237286618788775);
                } else {
                    set_output2(var468, 0.9891200870393035, 0.010879912960696507);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var468, 1.0, 0.0);
                } else {
                    set_output2(var468, 0.31210191082802546, 0.6878980891719745);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var468, 0.0, 1.0);
                } else {
                    set_output2(var468, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var93, var468, 2, var92);
    double var469[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var469, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var469, 0.0, 1.0);
            }
        } else {
            set_output2(var469, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var469, 1.0, 0.0);
                } else {
                    set_output2(var469, 0.9896095358654377, 0.010390464134562358);
                }
            } else {
                if (input[10] <= 0.5) {
                    set_output2(var469, 0.7846031112883924, 0.21539688871160753);
                } else {
                    set_output2(var469, 0.3181818181818182, 0.6818181818181819);
                }
            }
        } else {
            set_output2(var469, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var92, var469, 2, var91);
    double var470[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var470, 0.9928300183232865, 0.007169981676713488);
                } else {
                    set_output2(var470, 0.0, 1.0);
                }
            } else {
                set_output2(var470, 1.0, 0.0);
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var470, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var470, 0.9924211802748584, 0.007578819725141632);
                } else {
                    set_output2(var470, 0.9842303720697692, 0.015769627930230765);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var470, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var470, 1.0, 0.0);
            }
        } else {
            if (input[6] <= 135.9388885498047) {
                set_output2(var470, 0.0, 1.0);
            } else {
                if (input[1] <= 6.2525246143341064) {
                    set_output2(var470, 0.0, 1.0);
                } else {
                    set_output2(var470, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var91, var470, 2, var90);
    double var471[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var471, 0.9974724780948102, 0.0025275219051898415);
                } else {
                    set_output2(var471, 0.8695904173106647, 0.13040958268933542);
                }
            } else {
                set_output2(var471, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var471, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var471, 0.9924211802748584, 0.007578819725141627);
                } else {
                    set_output2(var471, 0.9842303720697692, 0.015769627930230786);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var471, 0.7471910112359551, 0.252808988764045);
                } else {
                    set_output2(var471, 0.24843423799582462, 0.7515657620041752);
                }
            } else {
                if (input[7] <= 15.713719367980957) {
                    set_output2(var471, 0.5544554455445545, 0.44554455445544555);
                } else {
                    set_output2(var471, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var471, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var90, var471, 2, var89);
    double var472[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[6] <= 112.17279052734375) {
                    set_output2(var472, 1.0, 0.0);
                } else {
                    set_output2(var472, 0.9774121898881902, 0.02258781011180966);
                }
            } else {
                set_output2(var472, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var472, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var472, 0.987069135823163, 0.012930864176837052);
                }
            } else {
                set_output2(var472, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var472, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[5] <= 9.748749732971191) {
                    set_output2(var472, 0.0, 1.0);
                } else {
                    set_output2(var472, 0.42634560906515584, 0.5736543909348442);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var472, 0.0, 1.0);
            } else {
                set_output2(var472, 1.0, 0.0);
            }
        }
    }
    add_vectors(var89, var472, 2, var88);
    double var473[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[1] <= 10.331897735595703) {
                    set_output2(var473, 1.0, 0.0);
                } else {
                    set_output2(var473, 0.8088360237892948, 0.1911639762107052);
                }
            } else {
                set_output2(var473, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var473, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[1] <= 5.543766021728516) {
                if (input[2] <= -9.043847560882568) {
                    set_output2(var473, 0.9965666100538814, 0.0034333899461187257);
                } else {
                    set_output2(var473, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var473, 0.9889714164142869, 0.01102858358571317);
                } else {
                    set_output2(var473, 0.7307807358659886, 0.2692192641340114);
                }
            }
        } else {
            set_output2(var473, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var88, var473, 2, var87);
    double var474[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var474, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var474, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var474, 0.9926187156565243, 0.007381284343475753);
                } else {
                    set_output2(var474, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var474, 0.9870506314315944, 0.01294936856840563);
                } else {
                    set_output2(var474, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var474, 0.7943640517897944, 0.20563594821020567);
                } else {
                    set_output2(var474, 0.3525179856115108, 0.6474820143884892);
                }
            } else {
                set_output2(var474, 0.0, 1.0);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var474, 1.0, 0.0);
            } else {
                set_output2(var474, 0.0, 1.0);
            }
        }
    }
    add_vectors(var87, var474, 2, var86);
    double var475[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[7] <= 17.16385555267334) {
                if (input[3] <= -60.23581886291504) {
                    set_output2(var475, 0.9965414146382704, 0.0034585853617295956);
                } else {
                    set_output2(var475, 1.0, 0.0);
                }
            } else {
                if (input[14] <= 0.5) {
                    set_output2(var475, 0.7261940979616671, 0.2738059020383328);
                } else {
                    set_output2(var475, 0.9966385299170838, 0.0033614700829162594);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var475, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var475, 0.987069135823163, 0.012930864176837038);
                }
            } else {
                set_output2(var475, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[8] <= 0.9768004715442657) {
                if (input[10] <= 0.5) {
                    set_output2(var475, 1.0, 0.0);
                } else {
                    set_output2(var475, 0.42492012779552724, 0.5750798722044729);
                }
            } else {
                set_output2(var475, 1.0, 0.0);
            }
        } else {
            if (input[6] <= 135.9388885498047) {
                set_output2(var475, 0.0, 1.0);
            } else {
                if (input[2] <= -5.388792037963867) {
                    set_output2(var475, 0.8316183348924228, 0.16838166510757718);
                } else {
                    set_output2(var475, 0.2840095465393795, 0.7159904534606205);
                }
            }
        }
    }
    add_vectors(var86, var475, 2, var85);
    double var476[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[9] <= 0.23671899735927582) {
            if (input[8] <= 1.0516844987869263) {
                if (input[8] <= 1.0513200163841248) {
                    set_output2(var476, 0.991700173374156, 0.008299826625843984);
                } else {
                    set_output2(var476, 0.0, 1.0);
                }
            } else {
                set_output2(var476, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 5.514397859573364) {
                if (input[8] <= 0.11474550142884254) {
                    set_output2(var476, 0.9729583558680368, 0.027041644131963215);
                } else {
                    set_output2(var476, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -5.364761590957642) {
                    set_output2(var476, 0.9739706977540086, 0.02602930224599151);
                } else {
                    set_output2(var476, 0.9928777183375012, 0.007122281662498858);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var476, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var476, 0.0, 1.0);
                } else {
                    set_output2(var476, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var476, 1.0, 0.0);
            } else {
                set_output2(var476, 0.0, 1.0);
            }
        }
    }
    add_vectors(var85, var476, 2, var84);
    double var477[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[4] <= 3.367400050163269) {
                    set_output2(var477, 0.9621132393180384, 0.03788676068196168);
                } else {
                    set_output2(var477, 0.9945334280872465, 0.005466571912753508);
                }
            } else {
                set_output2(var477, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var477, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var477, 0.987069135823163, 0.01293086417683704);
                }
            } else {
                set_output2(var477, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[9] <= 2.0398740768432617) {
                    set_output2(var477, 0.2538860103626943, 0.7461139896373057);
                } else {
                    set_output2(var477, 0.620253164556962, 0.37974683544303806);
                }
            } else {
                if (input[3] <= -43.79238700866699) {
                    set_output2(var477, 1.0, 0.0);
                } else {
                    set_output2(var477, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var477, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var84, var477, 2, var83);
    double var478[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var478, 0.8657317618976578, 0.13426823810234223);
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var478, 0.9902130829332478, 0.009786917066752191);
                } else {
                    set_output2(var478, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var478, 0.9924211802748584, 0.007578819725141639);
                } else {
                    set_output2(var478, 0.9842303720697692, 0.015769627930230793);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[1] <= 11.923262119293213) {
                if (input[4] <= 89.85874938964844) {
                    set_output2(var478, 0.7045951859956237, 0.2954048140043764);
                } else {
                    set_output2(var478, 0.1536050156739812, 0.8463949843260188);
                }
            } else {
                set_output2(var478, 0.711769415532426, 0.2882305844675741);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var478, 1.0, 0.0);
            } else {
                set_output2(var478, 0.0, 1.0);
            }
        }
    }
    add_vectors(var83, var478, 2, var82);
    double var479[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var479, 0.9928169328835492, 0.0071830671164508095);
                } else {
                    set_output2(var479, 0.0, 1.0);
                }
            } else {
                set_output2(var479, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var479, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var479, 0.987069135823163, 0.012930864176837047);
                } else {
                    set_output2(var479, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var479, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[9] <= 2.9800379276275635) {
                    set_output2(var479, 0.4757281553398059, 0.5242718446601942);
                } else {
                    set_output2(var479, 0.0, 1.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var479, 0.0, 1.0);
            } else {
                set_output2(var479, 1.0, 0.0);
            }
        }
    }
    add_vectors(var82, var479, 2, var81);
    double var480[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[7] <= 17.14678382873535) {
                    set_output2(var480, 0.9972867539326994, 0.0027132460673005582);
                } else {
                    set_output2(var480, 0.9410016606939953, 0.0589983393060047);
                }
            } else {
                set_output2(var480, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var480, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var480, 0.987069135823163, 0.01293086417683704);
                } else {
                    set_output2(var480, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var480, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var480, 0.0, 1.0);
                } else {
                    set_output2(var480, 0.4553706505295009, 0.5446293494704992);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var480, 0.0, 1.0);
            } else {
                set_output2(var480, 1.0, 0.0);
            }
        }
    }
    add_vectors(var81, var480, 2, var80);
    double var481[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[8] <= 0.4023445099592209) {
                set_output2(var481, 0.3300248138957817, 0.6699751861042184);
            } else {
                set_output2(var481, 0.7746619929894842, 0.22533800701051576);
            }
        } else {
            set_output2(var481, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var481, 1.0, 0.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var481, 0.9902859646657134, 0.009714035334286615);
                } else {
                    set_output2(var481, 0.9428462564297961, 0.057153743570203835);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[14] <= 0.5) {
                    set_output2(var481, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var481, 1.0, 0.0);
                }
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var481, 0.24916573971078979, 0.7508342602892102);
                } else {
                    set_output2(var481, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var80, var481, 2, var79);
    double var482[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[7] <= 17.760960578918457) {
            if (input[1] <= 8.246700286865234) {
                if (input[3] <= -57.00823974609375) {
                    set_output2(var482, 0.9920957294982984, 0.007904270501701602);
                } else {
                    set_output2(var482, 0.9981335958281027, 0.001866404171897242);
                }
            } else {
                if (input[7] <= 17.185452461242676) {
                    set_output2(var482, 0.9868631513772301, 0.01313684862276976);
                } else {
                    set_output2(var482, 1.0, 0.0);
                }
            }
        } else {
            if (input[9] <= 0.516086995601654) {
                if (input[8] <= 0.42980749905109406) {
                    set_output2(var482, 0.9531201166788207, 0.04687988332117928);
                } else {
                    set_output2(var482, 1.0, 0.0);
                }
            } else {
                set_output2(var482, 0.7335701598579041, 0.2664298401420959);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var482, 1.0, 0.0);
            } else {
                set_output2(var482, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var482, 0.0, 1.0);
            } else {
                if (input[2] <= -4.315015077590942) {
                    set_output2(var482, 0.7590361445783133, 0.24096385542168675);
                } else {
                    set_output2(var482, 0.13461538461538464, 0.8653846153846153);
                }
            }
        }
    }
    add_vectors(var79, var482, 2, var78);
    double var483[2];
    if (input[1] <= 5.543766021728516) {
        if (input[9] <= 1.7348750233650208) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var483, 0.9974818835511038, 0.002518116448896222);
                } else {
                    set_output2(var483, 0.8695904173106646, 0.13040958268933542);
                }
            } else {
                set_output2(var483, 1.0, 0.0);
            }
        } else {
            set_output2(var483, 0.2697768762677485, 0.7302231237322515);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var483, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var483, 1.0, 0.0);
                } else {
                    set_output2(var483, 0.9880884695834108, 0.011911530416589296);
                }
            }
        } else {
            if (input[5] <= 151.5938491821289) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var483, 0.7165354330708661, 0.2834645669291339);
                } else {
                    set_output2(var483, 0.28, 0.72);
                }
            } else {
                if (input[3] <= -46.46929168701172) {
                    set_output2(var483, 1.0, 0.0);
                } else {
                    set_output2(var483, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var78, var483, 2, var77);
    double var484[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var484, 1.0, 0.0);
            } else {
                set_output2(var484, 0.7439180537772087, 0.2560819462227913);
            }
        } else {
            set_output2(var484, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var484, 1.0, 0.0);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var484, 0.9927608506804799, 0.007239149319520123);
                } else {
                    set_output2(var484, 0.985551767137986, 0.014448232862013962);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[2] <= -12.859408855438232) {
                    set_output2(var484, 1.0, 0.0);
                } else {
                    set_output2(var484, 0.3433319821645724, 0.6566680178354277);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var484, 0.0, 1.0);
                } else {
                    set_output2(var484, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var77, var484, 2, var76);
    double var485[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var485, 1.0, 0.0);
            } else {
                set_output2(var485, 0.7439180537772088, 0.25608194622279135);
            }
        } else {
            set_output2(var485, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var485, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var485, 0.9882568690919772, 0.011743130908022823);
                } else {
                    set_output2(var485, 1.0, 0.0);
                }
            }
        } else {
            if (input[14] <= 0.5) {
                set_output2(var485, 0.295774647887324, 0.7042253521126761);
            } else {
                if (input[3] <= -54.35634803771973) {
                    set_output2(var485, 1.0, 0.0);
                } else {
                    set_output2(var485, 0.40079893475366174, 0.5992010652463382);
                }
            }
        }
    }
    add_vectors(var76, var485, 2, var75);
    double var486[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var486, 0.9928300183232865, 0.007169981676713482);
                } else {
                    set_output2(var486, 0.0, 1.0);
                }
            } else {
                set_output2(var486, 1.0, 0.0);
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var486, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var486, 0.987069135823163, 0.012930864176837047);
                } else {
                    set_output2(var486, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var486, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var486, 0.0, 1.0);
                } else {
                    set_output2(var486, 0.4553706505295008, 0.5446293494704992);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var486, 1.0, 0.0);
            } else {
                set_output2(var486, 0.0, 1.0);
            }
        }
    }
    add_vectors(var75, var486, 2, var74);
    double var487[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[5] <= 0.060350000858306885) {
                set_output2(var487, 0.7793035801863659, 0.22069641981363414);
            } else {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var487, 1.0, 0.0);
                } else {
                    set_output2(var487, 0.9896095358654377, 0.01039046413456234);
                }
            }
        } else {
            if (input[5] <= 246.27125549316406) {
                if (input[1] <= 10.020683288574219) {
                    set_output2(var487, 0.31257344300822565, 0.6874265569917745);
                } else {
                    set_output2(var487, 0.7341211225997046, 0.2658788774002954);
                }
            } else {
                set_output2(var487, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var487, 0.6945839554771277, 0.30541604452287224);
    }
    add_vectors(var74, var487, 2, var73);
    double var488[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var488, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var488, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var488, 0.9927898095975646, 0.007210190402435434);
                } else {
                    set_output2(var488, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var488, 0.9870506314315944, 0.012949368568405638);
                } else {
                    set_output2(var488, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var488, 0.747191011235955, 0.25280898876404495);
                } else {
                    set_output2(var488, 0.24843423799582462, 0.7515657620041752);
                }
            } else {
                if (input[1] <= 10.067442417144775) {
                    set_output2(var488, 0.8658718330849479, 0.13412816691505214);
                } else {
                    set_output2(var488, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var488, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var73, var488, 2, var72);
    double var489[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[1] <= 5.673432111740112) {
            set_output2(var489, 1.0, 0.0);
        } else {
            set_output2(var489, 0.6329027872195785, 0.3670972127804215);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var489, 0.9902130829332478, 0.009786917066752193);
                } else {
                    set_output2(var489, 1.0, 0.0);
                }
            } else {
                set_output2(var489, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var489, 0.987069135823163, 0.012930864176837056);
                } else {
                    set_output2(var489, 1.0, 0.0);
                }
            } else {
                if (input[14] <= 0.5) {
                    set_output2(var489, 0.2538860103626943, 0.7461139896373057);
                } else {
                    set_output2(var489, 0.7408207343412527, 0.2591792656587473);
                }
            }
        }
    }
    add_vectors(var72, var489, 2, var71);
    double var490[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[2] <= -7.388551950454712) {
                    set_output2(var490, 0.8868778280542986, 0.11312217194570136);
                } else {
                    set_output2(var490, 1.0, 0.0);
                }
            } else {
                set_output2(var490, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var490, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var490, 1.0, 0.0);
                } else {
                    set_output2(var490, 0.9896095358654377, 0.01039046413456234);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var490, 0.34746235359732297, 0.6525376464026771);
                } else {
                    set_output2(var490, 0.9053627760252365, 0.0946372239747634);
                }
            }
        } else {
            set_output2(var490, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var71, var490, 2, var70);
    double var491[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[1] <= 3.527751088142395) {
                if (input[1] <= 3.445926070213318) {
                    set_output2(var491, 1.0, 0.0);
                } else {
                    set_output2(var491, 0.0, 1.0);
                }
            } else {
                set_output2(var491, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.413290977478027) {
                    set_output2(var491, 0.9895561761679676, 0.010443823832032415);
                } else {
                    set_output2(var491, 0.9676633328324962, 0.03233666716750389);
                }
            } else {
                set_output2(var491, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var491, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var491, 0.0, 1.0);
                } else {
                    set_output2(var491, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var491, 0.0, 1.0);
            } else {
                set_output2(var491, 1.0, 0.0);
            }
        }
    }
    add_vectors(var70, var491, 2, var69);
    double var492[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var492, 0.9928169328835492, 0.007183067116450806);
                } else {
                    set_output2(var492, 0.0, 1.0);
                }
            } else {
                set_output2(var492, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var492, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var492, 0.987069135823163, 0.012930864176837044);
                }
            } else {
                set_output2(var492, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var492, 0.7232472324723246, 0.2767527675276753);
                } else {
                    set_output2(var492, 0.33972911963882624, 0.6602708803611739);
                }
            } else {
                set_output2(var492, 0.0, 1.0);
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var492, 0.0, 1.0);
            } else {
                set_output2(var492, 1.0, 0.0);
            }
        }
    }
    add_vectors(var69, var492, 2, var68);
    double var493[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[2] <= -3.9435800313949585) {
                set_output2(var493, 0.7919796602334451, 0.20802033976655496);
            } else {
                set_output2(var493, 1.0, 0.0);
            }
        } else {
            set_output2(var493, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[6] <= 132.94055938720703) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var493, 1.0, 0.0);
                } else {
                    set_output2(var493, 0.9818297263657733, 0.018170273634226682);
                }
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var493, 0.9941372862423706, 0.00586271375762939);
                } else {
                    set_output2(var493, 0.8313975271637317, 0.1686024728362683);
                }
            }
        } else {
            set_output2(var493, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var68, var493, 2, var67);
    double var494[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[9] <= 6.277254581451416) {
            if (input[2] <= -3.9435800313949585) {
                if (input[4] <= 88.75569772720337) {
                    set_output2(var494, 0.6649791542584872, 0.3350208457415128);
                } else {
                    set_output2(var494, 1.0, 0.0);
                }
            } else {
                set_output2(var494, 1.0, 0.0);
            }
        } else {
            set_output2(var494, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var494, 1.0, 0.0);
            } else {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var494, 0.9851918003581308, 0.014808199641869288);
                } else {
                    set_output2(var494, 0.9982046248862929, 0.0017953751137070898);
                }
            }
        } else {
            set_output2(var494, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var67, var494, 2, var66);
    double var495[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.476778984069824) {
                if (input[7] <= 15.22725772857666) {
                    set_output2(var495, 0.0, 1.0);
                } else {
                    set_output2(var495, 0.8664292074799644, 0.13357079252003562);
                }
            } else {
                set_output2(var495, 0.0, 1.0);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var495, 0.9817229194589984, 0.018277080541001585);
                } else {
                    set_output2(var495, 0.9969091602188315, 0.0030908397811685844);
                }
            } else {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var495, 1.0, 0.0);
                } else {
                    set_output2(var495, 0.9854492403188123, 0.014550759681187742);
                }
            }
        }
    } else {
        set_output2(var495, 0.6945839554771277, 0.30541604452287224);
    }
    add_vectors(var66, var495, 2, var65);
    double var496[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var496, 0.6232207647222997, 0.37677923527770024);
        } else {
            set_output2(var496, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[1] <= 4.112897634506226) {
                    set_output2(var496, 0.9815568731386104, 0.018443126861389655);
                } else {
                    set_output2(var496, 0.9992262590484705, 0.0007737409515294386);
                }
            } else {
                set_output2(var496, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var496, 0.987069135823163, 0.012930864176837056);
                } else {
                    set_output2(var496, 1.0, 0.0);
                }
            } else {
                if (input[5] <= 151.5938491821289) {
                    set_output2(var496, 0.3684210526315789, 0.631578947368421);
                } else {
                    set_output2(var496, 0.897025171624714, 0.10297482837528606);
                }
            }
        }
    }
    add_vectors(var65, var496, 2, var64);
    double var497[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[9] <= 2.150004506111145) {
                if (input[1] <= 10.331897735595703) {
                    set_output2(var497, 1.0, 0.0);
                } else {
                    set_output2(var497, 0.8088360237892948, 0.1911639762107052);
                }
            } else {
                set_output2(var497, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var497, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var497, 1.0, 0.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var497, 0.9902859646657134, 0.009714035334286572);
                } else {
                    set_output2(var497, 0.9428462564297962, 0.05715374357020382);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var497, 0.266304347826087, 0.7336956521739131);
                } else {
                    set_output2(var497, 0.6447368421052632, 0.35526315789473684);
                }
            } else {
                if (input[3] <= -46.46929168701172) {
                    set_output2(var497, 1.0, 0.0);
                } else {
                    set_output2(var497, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var64, var497, 2, var63);
    double var498[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[1] <= 5.673432111740112) {
                set_output2(var498, 1.0, 0.0);
            } else {
                set_output2(var498, 0.7964261479303325, 0.20357385206966752);
            }
        } else {
            set_output2(var498, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var498, 0.9979329320669608, 0.002067067933039282);
                } else {
                    set_output2(var498, 0.9887643780849196, 0.011235621915080495);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var498, 0.347462353597323, 0.6525376464026771);
                } else {
                    set_output2(var498, 0.9053627760252365, 0.0946372239747634);
                }
            }
        } else {
            set_output2(var498, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var63, var498, 2, var62);
    double var499[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[7] <= 17.760960578918457) {
            if (input[5] <= 0.060350000858306885) {
                if (input[6] <= 109.77089309692383) {
                    set_output2(var499, 1.0, 0.0);
                } else {
                    set_output2(var499, 0.7283155564499899, 0.2716844435500101);
                }
            } else {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var499, 0.9985882065046031, 0.0014117934953967902);
                } else {
                    set_output2(var499, 0.9893430980377091, 0.010656901962290853);
                }
            }
        } else {
            if (input[7] <= 17.764528274536133) {
                set_output2(var499, 0.0, 1.0);
            } else {
                if (input[5] <= 40.779701232910156) {
                    set_output2(var499, 0.8510591350397175, 0.14894086496028247);
                } else {
                    set_output2(var499, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var499, 1.0, 0.0);
            } else {
                set_output2(var499, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var499, 0.0, 1.0);
            } else {
                if (input[7] <= 17.21262550354004) {
                    set_output2(var499, 0.6337741607324517, 0.3662258392675483);
                } else {
                    set_output2(var499, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var62, var499, 2, var61);
    double var500[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var500, 0.9974724780948102, 0.0025275219051898406);
                } else {
                    set_output2(var500, 0.8695904173106646, 0.13040958268933536);
                }
            } else {
                set_output2(var500, 1.0, 0.0);
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var500, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var500, 0.987069135823163, 0.012930864176837052);
                } else {
                    set_output2(var500, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var500, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var500, 1.0, 0.0);
            }
        } else {
            if (input[6] <= 135.9388885498047) {
                set_output2(var500, 0.0, 1.0);
            } else {
                if (input[1] <= 6.2525246143341064) {
                    set_output2(var500, 0.0, 1.0);
                } else {
                    set_output2(var500, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var61, var500, 2, var60);
    double var501[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var501, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var501, 0.0, 1.0);
            }
        } else {
            set_output2(var501, 1.0, 0.0);
        }
    } else {
        if (input[14] <= 0.5) {
            if (input[9] <= 1.4178645014762878) {
                if (input[6] <= 108.99774169921875) {
                    set_output2(var501, 0.9969666943034519, 0.003033305696548106);
                } else {
                    set_output2(var501, 0.983941316415543, 0.016058683584457015);
                }
            } else {
                set_output2(var501, 0.29577464788732394, 0.7042253521126761);
            }
        } else {
            if (input[5] <= 170.07354736328125) {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var501, 0.9938052409038565, 0.006194759096143507);
                } else {
                    set_output2(var501, 0.48863636363636365, 0.5113636363636364);
                }
            } else {
                if (input[5] <= 170.48780059814453) {
                    set_output2(var501, 0.0, 1.0);
                } else {
                    set_output2(var501, 0.976997980933882, 0.023002019066118023);
                }
            }
        }
    }
    add_vectors(var60, var501, 2, var59);
    double var502[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[7] <= 17.16385555267334) {
                set_output2(var502, 1.0, 0.0);
            } else {
                if (input[7] <= 17.164725303649902) {
                    set_output2(var502, 0.0, 1.0);
                } else {
                    set_output2(var502, 1.0, 0.0);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.413290977478027) {
                    set_output2(var502, 0.9895561761679676, 0.01044382383203242);
                } else {
                    set_output2(var502, 0.9676633328324961, 0.03233666716750389);
                }
            } else {
                set_output2(var502, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[8] <= 0.7983885109424591) {
                if (input[3] <= -47.967037200927734) {
                    set_output2(var502, 0.5384615384615385, 0.46153846153846156);
                } else {
                    set_output2(var502, 0.0, 1.0);
                }
            } else {
                if (input[5] <= 72.94334983825684) {
                    set_output2(var502, 0.6691176470588235, 0.33088235294117646);
                } else {
                    set_output2(var502, 0.9470588235294117, 0.05294117647058824);
                }
            }
        } else {
            set_output2(var502, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var59, var502, 2, var58);
    double var503[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var503, 1.0, 0.0);
            } else {
                set_output2(var503, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var503, 1.0, 0.0);
            } else {
                if (input[2] <= -12.744715690612793) {
                    set_output2(var503, 0.0, 1.0);
                } else {
                    set_output2(var503, 0.9898111946442208, 0.01018880535577922);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var503, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[2] <= -6.903803110122681) {
                    set_output2(var503, 0.0, 1.0);
                } else {
                    set_output2(var503, 0.38582677165354323, 0.6141732283464567);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var503, 1.0, 0.0);
            } else {
                set_output2(var503, 0.0, 1.0);
            }
        }
    }
    add_vectors(var58, var503, 2, var57);
    double var504[2];
    if (input[14] <= 0.5) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var504, 0.8943165805542508, 0.10568341944574916);
            } else {
                set_output2(var504, 0.3894165535956581, 0.6105834464043419);
            }
        } else {
            if (input[9] <= 1.4178645014762878) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var504, 0.9916381952376113, 0.008361804762388618);
                } else {
                    set_output2(var504, 0.9743969048702776, 0.02560309512972233);
                }
            } else {
                set_output2(var504, 0.29577464788732394, 0.7042253521126761);
            }
        }
    } else {
        if (input[4] <= 3.455999970436096) {
            set_output2(var504, 0.7628146000790618, 0.2371853999209382);
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[7] <= 16.206111907958984) {
                    set_output2(var504, 0.9986237353657194, 0.0013762646342806188);
                } else {
                    set_output2(var504, 0.9924598044743519, 0.00754019552564815);
                }
            } else {
                if (input[3] <= -54.35634803771973) {
                    set_output2(var504, 1.0, 0.0);
                } else {
                    set_output2(var504, 0.40079893475366174, 0.5992010652463382);
                }
            }
        }
    }
    add_vectors(var57, var504, 2, var56);
    double var505[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[5] <= 0.060350000858306885) {
                set_output2(var505, 0.7793035801863659, 0.22069641981363414);
            } else {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var505, 1.0, 0.0);
                } else {
                    set_output2(var505, 0.9896095358654377, 0.010390464134562332);
                }
            }
        } else {
            if (input[5] <= 246.27125549316406) {
                if (input[1] <= 10.020683288574219) {
                    set_output2(var505, 0.31257344300822565, 0.6874265569917745);
                } else {
                    set_output2(var505, 0.7341211225997046, 0.2658788774002954);
                }
            } else {
                set_output2(var505, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var505, 0.6945839554771277, 0.3054160445228723);
    }
    add_vectors(var56, var505, 2, var55);
    double var506[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[6] <= 109.77089309692383) {
                set_output2(var506, 1.0, 0.0);
            } else {
                set_output2(var506, 0.7283155564499899, 0.2716844435500101);
            }
        } else {
            if (input[5] <= 21.310150146484375) {
                if (input[5] <= 21.240949630737305) {
                    set_output2(var506, 0.988063892656376, 0.011936107343623959);
                } else {
                    set_output2(var506, 0.0, 1.0);
                }
            } else {
                if (input[7] <= 17.75955295562744) {
                    set_output2(var506, 0.9945780557971213, 0.005421944202878663);
                } else {
                    set_output2(var506, 0.9592772465385659, 0.040722753461434016);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var506, 0.7232472324723247, 0.2767527675276753);
                } else {
                    set_output2(var506, 0.33972911963882624, 0.6602708803611739);
                }
            } else {
                set_output2(var506, 0.0, 1.0);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var506, 1.0, 0.0);
            } else {
                set_output2(var506, 0.0, 1.0);
            }
        }
    }
    add_vectors(var55, var506, 2, var54);
    double var507[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var507, 1.0, 0.0);
            } else {
                set_output2(var507, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var507, 0.0, 1.0);
        }
    } else {
        if (input[5] <= 21.988649368286133) {
            if (input[6] <= 108.99774169921875) {
                if (input[9] <= 1.9795434474945068) {
                    set_output2(var507, 0.9970419938276271, 0.002958006172372889);
                } else {
                    set_output2(var507, 0.0, 1.0);
                }
            } else {
                if (input[9] <= 1.6179354786872864) {
                    set_output2(var507, 0.9839082651917893, 0.01609173480821065);
                } else {
                    set_output2(var507, 0.4347522679692952, 0.5652477320307048);
                }
            }
        } else {
            if (input[9] <= 1.1597474813461304) {
                if (input[1] <= 5.543497562408447) {
                    set_output2(var507, 1.0, 0.0);
                } else {
                    set_output2(var507, 0.9912670535038519, 0.008732946496147996);
                }
            } else {
                if (input[2] <= -6.977892875671387) {
                    set_output2(var507, 1.0, 0.0);
                } else {
                    set_output2(var507, 0.5399628252788105, 0.4600371747211895);
                }
            }
        }
    }
    add_vectors(var54, var507, 2, var53);
    double var508[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var508, 1.0, 0.0);
            } else {
                set_output2(var508, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var508, 0.9926187156565243, 0.007381284343475756);
                } else {
                    set_output2(var508, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var508, 0.9893430980377091, 0.010656901962290844);
                } else {
                    set_output2(var508, 0.9394075403949732, 0.060592459605026905);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[2] <= -13.028204441070557) {
                set_output2(var508, 1.0, 0.0);
            } else {
                if (input[1] <= 9.815945625305176) {
                    set_output2(var508, 0.17314487632508835, 0.8268551236749117);
                } else {
                    set_output2(var508, 0.6100144439094848, 0.3899855560905152);
                }
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var508, 1.0, 0.0);
            } else {
                set_output2(var508, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var53, var508, 2, var52);
    double var509[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[8] <= 0.0949764996767044) {
                set_output2(var509, 1.0, 0.0);
            } else {
                set_output2(var509, 0.7523844460748349, 0.24761555392516507);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var509, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var509, 0.9882568690919772, 0.011743130908022809);
                } else {
                    set_output2(var509, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var509, 0.6853146853146853, 0.3146853146853147);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var509, 0.1891891891891892, 0.8108108108108109);
                } else {
                    set_output2(var509, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var509, 1.0, 0.0);
            } else {
                set_output2(var509, 0.0, 1.0);
            }
        }
    }
    add_vectors(var52, var509, 2, var51);
    double var510[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var510, 0.560546875, 0.439453125);
            } else {
                set_output2(var510, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.413290977478027) {
                    set_output2(var510, 0.9895561761679675, 0.010443823832032439);
                } else {
                    set_output2(var510, 0.9676633328324961, 0.03233666716750389);
                }
            } else {
                set_output2(var510, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var510, 0.42492012779552724, 0.5750798722044729);
            } else {
                set_output2(var510, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var510, 0.0, 1.0);
            } else {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var510, 0.0, 1.0);
                } else {
                    set_output2(var510, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var51, var510, 2, var50);
    double var511[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[7] <= 16.334160804748535) {
                set_output2(var511, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var511, 0.0, 1.0);
            }
        } else {
            set_output2(var511, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var511, 1.0, 0.0);
            } else {
                if (input[3] <= -64.41340255737305) {
                    set_output2(var511, 1.0, 0.0);
                } else {
                    set_output2(var511, 0.9890113415404329, 0.010988658459567086);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var511, 1.0, 0.0);
                } else {
                    set_output2(var511, 0.3121019108280255, 0.6878980891719745);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var511, 0.0, 1.0);
                } else {
                    set_output2(var511, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var50, var511, 2, var49);
    double var512[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[2] <= -5.9743804931640625) {
                set_output2(var512, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var512, 0.0, 1.0);
            }
        } else {
            set_output2(var512, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var512, 1.0, 0.0);
            } else {
                if (input[1] <= 12.717926025390625) {
                    set_output2(var512, 0.9851918003581308, 0.014808199641869248);
                } else {
                    set_output2(var512, 0.9982046248862929, 0.0017953751137070918);
                }
            }
        } else {
            set_output2(var512, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var49, var512, 2, var48);
    double var513[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var513, 1.0, 0.0);
            } else {
                set_output2(var513, 0.7439180537772087, 0.2560819462227913);
            }
        } else {
            set_output2(var513, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var513, 1.0, 0.0);
            } else {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var513, 0.9896095358654378, 0.010390464134562296);
                } else {
                    set_output2(var513, 0.6062992125984251, 0.3937007874015749);
                }
            }
        } else {
            set_output2(var513, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var48, var513, 2, var47);
    double var514[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var514, 0.9974724780948102, 0.00252752190518984);
                } else {
                    set_output2(var514, 0.8695904173106646, 0.1304095826893354);
                }
            } else {
                set_output2(var514, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var514, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var514, 0.987069135823163, 0.012930864176837044);
                }
            } else {
                set_output2(var514, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var514, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var514, 1.0, 0.0);
            }
        } else {
            if (input[6] <= 135.9388885498047) {
                set_output2(var514, 0.0, 1.0);
            } else {
                if (input[1] <= 6.2525246143341064) {
                    set_output2(var514, 0.0, 1.0);
                } else {
                    set_output2(var514, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var47, var514, 2, var46);
    double var515[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var515, 0.9974724780948102, 0.0025275219051898406);
                } else {
                    set_output2(var515, 0.8695904173106647, 0.13040958268933542);
                }
            } else {
                set_output2(var515, 1.0, 0.0);
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var515, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var515, 1.0, 0.0);
                } else {
                    set_output2(var515, 0.9880884695834107, 0.011911530416589312);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var515, 1.0, 0.0);
            } else {
                set_output2(var515, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var515, 0.0, 1.0);
            } else {
                if (input[2] <= -4.315015077590942) {
                    set_output2(var515, 0.7590361445783133, 0.24096385542168675);
                } else {
                    set_output2(var515, 0.13461538461538464, 0.8653846153846153);
                }
            }
        }
    }
    add_vectors(var46, var515, 2, var45);
    double var516[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[2] <= -3.9435800313949585) {
            if (input[2] <= -5.9743804931640625) {
                set_output2(var516, 0.8354260636352554, 0.1645739363647446);
            } else {
                set_output2(var516, 0.0, 1.0);
            }
        } else {
            set_output2(var516, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[2] <= -12.753203868865967) {
                    set_output2(var516, 1.0, 0.0);
                } else {
                    set_output2(var516, 0.9896095358654378, 0.010390464134562308);
                }
            } else {
                if (input[3] <= -54.70915985107422) {
                    set_output2(var516, 0.8288159771754636, 0.1711840228245364);
                } else {
                    set_output2(var516, 0.37814070351758794, 0.621859296482412);
                }
            }
        } else {
            set_output2(var516, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var45, var516, 2, var44);
    double var517[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var517, 1.0, 0.0);
            } else {
                set_output2(var517, 0.7439180537772087, 0.25608194622279135);
            }
        } else {
            set_output2(var517, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var517, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var517, 0.9882568690919773, 0.011743130908022781);
                } else {
                    set_output2(var517, 1.0, 0.0);
                }
            }
        } else {
            if (input[14] <= 0.5) {
                set_output2(var517, 0.29577464788732394, 0.7042253521126761);
            } else {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var517, 0.5860165593376265, 0.41398344066237347);
                } else {
                    set_output2(var517, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var44, var517, 2, var43);
    double var518[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var518, 1.0, 0.0);
            } else {
                set_output2(var518, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var518, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var518, 0.9882568690919772, 0.011743130908022816);
                } else {
                    set_output2(var518, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var518, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var518, 0.0, 1.0);
                } else {
                    set_output2(var518, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var518, 0.0, 1.0);
            } else {
                set_output2(var518, 1.0, 0.0);
            }
        }
    }
    add_vectors(var43, var518, 2, var42);
    double var519[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var519, 0.8657317618976578, 0.13426823810234226);
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[6] <= 123.47669982910156) {
                    set_output2(var519, 0.9879022884442342, 0.012097711555765877);
                } else {
                    set_output2(var519, 0.9979119056181339, 0.0020880943818661003);
                }
            } else {
                if (input[1] <= 5.514397859573364) {
                    set_output2(var519, 0.9980307207562032, 0.0019692792437967697);
                } else {
                    set_output2(var519, 0.9842303720697692, 0.015769627930230786);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var519, 1.0, 0.0);
            } else {
                set_output2(var519, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var519, 0.0, 1.0);
            } else {
                if (input[3] <= -41.139564514160156) {
                    set_output2(var519, 0.6428571428571428, 0.35714285714285715);
                } else {
                    set_output2(var519, 0.199288256227758, 0.800711743772242);
                }
            }
        }
    }
    add_vectors(var42, var519, 2, var41);
    double var520[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[2] <= -3.9435800313949585) {
                set_output2(var520, 0.791979660233445, 0.20802033976655498);
            } else {
                set_output2(var520, 1.0, 0.0);
            }
        } else {
            set_output2(var520, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[7] <= 17.991198539733887) {
                    set_output2(var520, 0.9986117880786529, 0.0013882119213470194);
                } else {
                    set_output2(var520, 0.9174008810572687, 0.08259911894273128);
                }
            } else {
                set_output2(var520, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[2] <= -0.9100589752197266) {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var520, 0.9889714164142869, 0.011028583585713134);
                } else {
                    set_output2(var520, 0.7307807358659886, 0.26921926413401137);
                }
            } else {
                set_output2(var520, 0.3323442136498516, 0.6676557863501483);
            }
        }
    }
    add_vectors(var41, var520, 2, var40);
    double var521[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var521, 1.0, 0.0);
            } else {
                set_output2(var521, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var521, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var521, 0.560546875, 0.439453125);
                } else {
                    set_output2(var521, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var521, 0.990296602801644, 0.009703397198355957);
                } else {
                    set_output2(var521, 0.9428462564297961, 0.05715374357020382);
                }
            }
        } else {
            if (input[9] <= 4.584504842758179) {
                if (input[14] <= 0.5) {
                    set_output2(var521, 0.3181818181818182, 0.6818181818181819);
                } else {
                    set_output2(var521, 0.8220640569395017, 0.17793594306049823);
                }
            } else {
                set_output2(var521, 0.17177914110429449, 0.8282208588957055);
            }
        }
    }
    add_vectors(var40, var521, 2, var39);
    double var522[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var522, 1.0, 0.0);
            } else {
                set_output2(var522, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var522, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[6] <= 132.94055938720703) {
                    set_output2(var522, 0.9870579364059348, 0.012942063594065262);
                } else {
                    set_output2(var522, 0.99386216886401, 0.006137831135989934);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var522, 0.34746235359732297, 0.6525376464026771);
                } else {
                    set_output2(var522, 0.9053627760252365, 0.09463722397476339);
                }
            }
        } else {
            set_output2(var522, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var39, var522, 2, var38);
    double var523[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var523, 0.9882728516515734, 0.01172714834842661);
                } else {
                    set_output2(var523, 0.0, 1.0);
                }
            } else {
                if (input[8] <= 0.11126700043678284) {
                    set_output2(var523, 0.9878552343939762, 0.0121447656060238);
                } else {
                    set_output2(var523, 1.0, 0.0);
                }
            }
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[7] <= 15.497403144836426) {
                    set_output2(var523, 1.0, 0.0);
                } else {
                    set_output2(var523, 0.9906330317644298, 0.009366968235570124);
                }
            } else {
                if (input[2] <= -5.364761590957642) {
                    set_output2(var523, 0.9739706977540085, 0.026029302245991523);
                } else {
                    set_output2(var523, 0.9928777183375012, 0.007122281662498862);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var523, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[2] <= -6.903803110122681) {
                    set_output2(var523, 0.0, 1.0);
                } else {
                    set_output2(var523, 0.38582677165354323, 0.6141732283464567);
                }
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var523, 1.0, 0.0);
            } else {
                set_output2(var523, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var38, var523, 2, var37);
    double var524[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[1] <= 5.673432111740112) {
                set_output2(var524, 1.0, 0.0);
            } else {
                set_output2(var524, 0.7751873438800999, 0.2248126561199001);
            }
        } else {
            set_output2(var524, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var524, 0.9904387041256992, 0.009561295874300818);
                } else {
                    set_output2(var524, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var524, 0.987069135823163, 0.012930864176837052);
                } else {
                    set_output2(var524, 1.0, 0.0);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[14] <= 0.5) {
                    set_output2(var524, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var524, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 15.968702793121338) {
                    set_output2(var524, 0.0, 1.0);
                } else {
                    set_output2(var524, 0.468503937007874, 0.5314960629921259);
                }
            }
        }
    }
    add_vectors(var37, var524, 2, var36);
    double var525[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var525, 0.9882728516515734, 0.011727148348426607);
                } else {
                    set_output2(var525, 0.0, 1.0);
                }
            } else {
                if (input[7] <= 17.991198539733887) {
                    set_output2(var525, 1.0, 0.0);
                } else {
                    set_output2(var525, 0.9174008810572687, 0.08259911894273128);
                }
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var525, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var525, 0.987069135823163, 0.01293086417683704);
                }
            } else {
                set_output2(var525, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[5] <= 5.530600070953369) {
                set_output2(var525, 0.6853146853146852, 0.3146853146853147);
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var525, 0.1891891891891892, 0.8108108108108109);
                } else {
                    set_output2(var525, 1.0, 0.0);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var525, 0.0, 1.0);
            } else {
                set_output2(var525, 1.0, 0.0);
            }
        }
    }
    add_vectors(var36, var525, 2, var35);
    double var526[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var526, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var526, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var526, 1.0, 0.0);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var526, 0.9927608506804799, 0.0072391493195201285);
                } else {
                    set_output2(var526, 0.985551767137986, 0.014448232862013959);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var526, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var526, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -4.315015077590942) {
                if (input[1] <= 7.8367509841918945) {
                    set_output2(var526, 0.0, 1.0);
                } else {
                    set_output2(var526, 0.863013698630137, 0.136986301369863);
                }
            } else {
                set_output2(var526, 0.12147505422993493, 0.878524945770065);
            }
        }
    }
    add_vectors(var35, var526, 2, var34);
    double var527[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var527, 0.6822033898305084, 0.31779661016949157);
        } else {
            if (input[14] <= 0.5) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var527, 0.9878478253131023, 0.0121521746868976);
                } else {
                    set_output2(var527, 0.969151733148378, 0.030848266851621887);
                }
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var527, 0.9937595051981009, 0.006240494801899116);
                } else {
                    set_output2(var527, 0.8462064251537935, 0.15379357484620648);
                }
            }
        }
    } else {
        set_output2(var527, 0.6945839554771277, 0.30541604452287224);
    }
    add_vectors(var34, var527, 2, var33);
    double var528[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var528, 1.0, 0.0);
            } else {
                set_output2(var528, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var528, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var528, 0.9882568690919773, 0.011743130908022778);
                } else {
                    set_output2(var528, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var528, 1.0, 0.0);
            } else {
                set_output2(var528, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var528, 0.0, 1.0);
            } else {
                if (input[9] <= 3.002187967300415) {
                    set_output2(var528, 0.6898690558235699, 0.31013094417643);
                } else {
                    set_output2(var528, 0.28, 0.72);
                }
            }
        }
    }
    add_vectors(var33, var528, 2, var32);
    double var529[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[5] <= 0.07090000063180923) {
                set_output2(var529, 0.560546875, 0.439453125);
            } else {
                set_output2(var529, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var529, 0.8657317618976578, 0.13426823810234223);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var529, 0.9882711945136788, 0.011728805486321131);
                } else {
                    set_output2(var529, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var529, 1.0, 0.0);
            } else {
                set_output2(var529, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[9] <= 3.002187967300415) {
                set_output2(var529, 0.6495781959766386, 0.3504218040233615);
            } else {
                set_output2(var529, 0.18490566037735848, 0.8150943396226416);
            }
        }
    }
    add_vectors(var32, var529, 2, var31);
    double var530[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[6] <= 184.20638275146484) {
                set_output2(var530, 1.0, 0.0);
            } else {
                set_output2(var530, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var530, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var530, 0.560546875, 0.439453125);
                } else {
                    set_output2(var530, 1.0, 0.0);
                }
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var530, 0.990296602801644, 0.009703397198355996);
                } else {
                    set_output2(var530, 0.9428462564297962, 0.05715374357020382);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var530, 0.3059125964010283, 0.6940874035989717);
                } else {
                    set_output2(var530, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 14.33062195777893) {
                    set_output2(var530, 0.946396664681358, 0.05360333531864206);
                } else {
                    set_output2(var530, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var31, var530, 2, var30);
    double var531[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var531, 1.0, 0.0);
            } else {
                set_output2(var531, 0.7439180537772088, 0.25608194622279135);
            }
        } else {
            set_output2(var531, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var531, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var531, 0.9882568690919772, 0.011743130908022812);
                } else {
                    set_output2(var531, 1.0, 0.0);
                }
            }
        } else {
            if (input[5] <= 172.15264892578125) {
                if (input[1] <= 11.934772968292236) {
                    set_output2(var531, 0.266304347826087, 0.7336956521739131);
                } else {
                    set_output2(var531, 0.6447368421052632, 0.35526315789473684);
                }
            } else {
                if (input[7] <= 15.72079086303711) {
                    set_output2(var531, 0.0, 1.0);
                } else {
                    set_output2(var531, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var30, var531, 2, var29);
    double var532[2];
    if (input[14] <= 0.5) {
        if (input[5] <= 0.060350000858306885) {
            if (input[8] <= 0.4023445099592209) {
                set_output2(var532, 0.5140388768898488, 0.48596112311015116);
            } else {
                set_output2(var532, 0.937603993344426, 0.06239600665557403);
            }
        } else {
            if (input[9] <= 1.4178645014762878) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var532, 0.9916381952376114, 0.00836180476238862);
                } else {
                    set_output2(var532, 0.9743969048702777, 0.025603095129722343);
                }
            } else {
                set_output2(var532, 0.29577464788732394, 0.7042253521126761);
            }
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[4] <= 3.455999970436096) {
                set_output2(var532, 0.6414342629482072, 0.3585657370517928);
            } else {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var532, 0.994050090306407, 0.005949909693593028);
                } else {
                    set_output2(var532, 0.8462064251537935, 0.15379357484620648);
                }
            }
        } else {
            set_output2(var532, 0.6720714155583896, 0.32792858444161044);
        }
    }
    add_vectors(var29, var532, 2, var28);
    double var533[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var533, 0.8657317618976578, 0.13426823810234226);
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var533, 0.9904387041256992, 0.009561295874300818);
                } else {
                    set_output2(var533, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var533, 0.9924211802748584, 0.0075788197251416296);
                } else {
                    set_output2(var533, 0.9842303720697693, 0.015769627930230772);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var533, 1.0, 0.0);
            } else {
                set_output2(var533, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var533, 0.0, 1.0);
            } else {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var533, 0.0, 1.0);
                } else {
                    set_output2(var533, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var28, var533, 2, var27);
    double var534[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[2] <= -12.753203868865967) {
            if (input[4] <= 2.857300043106079) {
                if (input[5] <= 0.22884999960660934) {
                    set_output2(var534, 0.0, 1.0);
                } else {
                    set_output2(var534, 1.0, 0.0);
                }
            } else {
                set_output2(var534, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.744715690612793) {
                set_output2(var534, 0.0, 1.0);
            } else {
                if (input[7] <= 17.760960578918457) {
                    set_output2(var534, 0.9901184957056632, 0.009881504294336776);
                } else {
                    set_output2(var534, 0.9428462564297961, 0.05715374357020382);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[9] <= 2.2410025596618652) {
                set_output2(var534, 0.5806150978564772, 0.4193849021435228);
            } else {
                set_output2(var534, 0.9433249370277078, 0.05667506297229219);
            }
        } else {
            if (input[6] <= 135.9388885498047) {
                set_output2(var534, 0.0, 1.0);
            } else {
                if (input[1] <= 6.2525246143341064) {
                    set_output2(var534, 0.0, 1.0);
                } else {
                    set_output2(var534, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var27, var534, 2, var26);
    double var535[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[3] <= -19.16687774658203) {
            if (input[7] <= 16.476778984069824) {
                if (input[7] <= 15.22725772857666) {
                    set_output2(var535, 0.0, 1.0);
                } else {
                    set_output2(var535, 0.8664292074799644, 0.13357079252003562);
                }
            } else {
                set_output2(var535, 0.0, 1.0);
            }
        } else {
            set_output2(var535, 1.0, 0.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var535, 1.0, 0.0);
            } else {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var535, 0.9896095358654377, 0.010390464134562334);
                } else {
                    set_output2(var535, 0.6062992125984251, 0.3937007874015748);
                }
            }
        } else {
            set_output2(var535, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var26, var535, 2, var25);
    double var536[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var536, 1.0, 0.0);
            } else {
                set_output2(var536, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var536, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var536, 0.9902130829332478, 0.009786917066752195);
                } else {
                    set_output2(var536, 1.0, 0.0);
                }
            } else {
                set_output2(var536, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[1] <= 12.717926025390625) {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var536, 0.9873576389449275, 0.012642361055072555);
                } else {
                    set_output2(var536, 0.6290189612530914, 0.37098103874690863);
                }
            } else {
                if (input[3] <= -35.84962844848633) {
                    set_output2(var536, 1.0, 0.0);
                } else {
                    set_output2(var536, 0.8939679547596607, 0.10603204524033931);
                }
            }
        }
    }
    add_vectors(var25, var536, 2, var24);
    double var537[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var537, 0.8657317618976578, 0.13426823810234223);
        } else {
            if (input[2] <= -12.753203868865967) {
                if (input[5] <= 0.07090000063180923) {
                    set_output2(var537, 0.560546875, 0.439453125);
                } else {
                    set_output2(var537, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var537, 0.9882711945136788, 0.011728805486321183);
                } else {
                    set_output2(var537, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var537, 1.0, 0.0);
            } else {
                set_output2(var537, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var537, 0.0, 1.0);
            } else {
                if (input[2] <= -4.315015077590942) {
                    set_output2(var537, 0.7590361445783131, 0.24096385542168672);
                } else {
                    set_output2(var537, 0.13461538461538464, 0.8653846153846153);
                }
            }
        }
    }
    add_vectors(var24, var537, 2, var23);
    double var538[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[2] <= -4.56547999382019) {
                set_output2(var538, 0.7232472324723247, 0.27675276752767525);
            } else {
                set_output2(var538, 1.0, 0.0);
            }
        } else {
            if (input[2] <= -12.753203868865967) {
                set_output2(var538, 1.0, 0.0);
            } else {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var538, 0.9882568690919772, 0.011743130908022812);
                } else {
                    set_output2(var538, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var538, 0.7434069850320741, 0.25659301496792586);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var538, 0.0, 1.0);
                } else {
                    set_output2(var538, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var538, 0.0, 1.0);
            } else {
                set_output2(var538, 1.0, 0.0);
            }
        }
    }
    add_vectors(var23, var538, 2, var22);
    double var539[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var539, 1.0, 0.0);
            } else {
                set_output2(var539, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var539, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[6] <= 132.94055938720703) {
                if (input[1] <= 12.70374345779419) {
                    set_output2(var539, 0.9853759151478095, 0.014624084852190388);
                } else {
                    set_output2(var539, 1.0, 0.0);
                }
            } else {
                if (input[1] <= 5.543497562408447) {
                    set_output2(var539, 0.9990242929376154, 0.0009757070623845513);
                } else {
                    set_output2(var539, 0.9914934853031302, 0.008506514696869805);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var539, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var539, 1.0, 0.0);
                }
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var539, 0.2491657397107898, 0.7508342602892103);
                } else {
                    set_output2(var539, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var22, var539, 2, var21);
    double var540[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 0.05140000022947788) {
                if (input[6] <= 140.11605834960938) {
                    set_output2(var540, 1.0, 0.0);
                } else {
                    set_output2(var540, 0.7439180537772087, 0.2560819462227913);
                }
            } else {
                set_output2(var540, 0.0, 1.0);
            }
        } else {
            set_output2(var540, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[7] <= 17.760960578918457) {
                if (input[1] <= 5.543766021728516) {
                    set_output2(var540, 0.9985882065046032, 0.001411793495396791);
                } else {
                    set_output2(var540, 0.9893430980377091, 0.010656901962290856);
                }
            } else {
                if (input[8] <= 1.2688884735107422) {
                    set_output2(var540, 0.9747630531097526, 0.025236946890247313);
                } else {
                    set_output2(var540, 0.5964125560538117, 0.40358744394618834);
                }
            }
        } else {
            if (input[10] <= 0.5) {
                if (input[3] <= -54.35634803771973) {
                    set_output2(var540, 1.0, 0.0);
                } else {
                    set_output2(var540, 0.40079893475366185, 0.5992010652463382);
                }
            } else {
                set_output2(var540, 0.29577464788732394, 0.7042253521126761);
            }
        }
    }
    add_vectors(var21, var540, 2, var20);
    double var541[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[4] <= 0.02199999988079071) {
            set_output2(var541, 0.6822033898305084, 0.3177966101694915);
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[6] <= 132.94055938720703) {
                    set_output2(var541, 0.9870579364059349, 0.012942063594065243);
                } else {
                    set_output2(var541, 0.9938621688640101, 0.006137831135989947);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var541, 0.34746235359732286, 0.6525376464026771);
                } else {
                    set_output2(var541, 0.9053627760252366, 0.09463722397476343);
                }
            }
        }
    } else {
        set_output2(var541, 0.6945839554771277, 0.30541604452287224);
    }
    add_vectors(var20, var541, 2, var19);
    double var542[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[6] <= 109.77089309692383) {
                set_output2(var542, 1.0, 0.0);
            } else {
                set_output2(var542, 0.7283155564499899, 0.2716844435500101);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var542, 0.9927898095975646, 0.00721019040243543);
                } else {
                    set_output2(var542, 1.0, 0.0);
                }
            } else {
                if (input[3] <= -64.41350936889648) {
                    set_output2(var542, 1.0, 0.0);
                } else {
                    set_output2(var542, 0.9880727693593861, 0.011927230640613835);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var542, 0.7943640517897944, 0.20563594821020564);
                } else {
                    set_output2(var542, 0.3525179856115108, 0.6474820143884892);
                }
            } else {
                set_output2(var542, 0.0, 1.0);
            }
        } else {
            if (input[2] <= -4.520525932312012) {
                set_output2(var542, 1.0, 0.0);
            } else {
                set_output2(var542, 0.6511627906976745, 0.3488372093023256);
            }
        }
    }
    add_vectors(var19, var542, 2, var18);
    double var543[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var543, 0.9928169328835492, 0.007183067116450804);
                } else {
                    set_output2(var543, 0.0, 1.0);
                }
            } else {
                set_output2(var543, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[7] <= 16.308735847473145) {
                    set_output2(var543, 0.9913722615388951, 0.008627738461104846);
                } else {
                    set_output2(var543, 0.98109031824294, 0.018909681757059944);
                }
            } else {
                set_output2(var543, 1.0, 0.0);
            }
        }
    } else {
        if (input[7] <= 17.21262550354004) {
            if (input[5] <= 110.48614883422852) {
                if (input[7] <= 16.633155822753906) {
                    set_output2(var543, 0.4200563834071688, 0.5799436165928312);
                } else {
                    set_output2(var543, 0.8448275862068966, 0.15517241379310345);
                }
            } else {
                if (input[1] <= 10.067442417144775) {
                    set_output2(var543, 0.6827262044653349, 0.31727379553466506);
                } else {
                    set_output2(var543, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var543, 0.0, 1.0);
        }
    }
    add_vectors(var18, var543, 2, var17);
    double var544[2];
    if (input[2] <= -0.9100589752197266) {
        if (input[9] <= 1.4035595059394836) {
            if (input[1] <= 5.543766021728516) {
                if (input[3] <= -54.02068901062012) {
                    set_output2(var544, 0.9904628182372108, 0.009537181762789082);
                } else {
                    set_output2(var544, 1.0, 0.0);
                }
            } else {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var544, 0.5964125560538117, 0.40358744394618834);
                } else {
                    set_output2(var544, 0.9887783113852344, 0.011221688614765602);
                }
            }
        } else {
            if (input[9] <= 6.277254581451416) {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var544, 0.5300261096605744, 0.46997389033942555);
                } else {
                    set_output2(var544, 0.9503311258278145, 0.04966887417218542);
                }
            } else {
                set_output2(var544, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var544, 0.6945839554771277, 0.30541604452287224);
    }
    add_vectors(var17, var544, 2, var16);
    double var545[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 0.05140000022947788) {
                if (input[6] <= 140.11605834960938) {
                    set_output2(var545, 1.0, 0.0);
                } else {
                    set_output2(var545, 0.7439180537772087, 0.25608194622279135);
                }
            } else {
                set_output2(var545, 0.0, 1.0);
            }
        } else {
            set_output2(var545, 0.0, 1.0);
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[2] <= -12.753203868865967) {
                set_output2(var545, 1.0, 0.0);
            } else {
                if (input[9] <= 1.4035595059394836) {
                    set_output2(var545, 0.9896095358654378, 0.010390464134562257);
                } else {
                    set_output2(var545, 0.6062992125984252, 0.3937007874015749);
                }
            }
        } else {
            set_output2(var545, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var16, var545, 2, var15);
    double var546[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var546, 0.9928300183232865, 0.007169981676713484);
                } else {
                    set_output2(var546, 0.0, 1.0);
                }
            } else {
                set_output2(var546, 1.0, 0.0);
            }
        } else {
            if (input[9] <= 0.23671899735927582) {
                if (input[8] <= 1.0516844987869263) {
                    set_output2(var546, 0.9901076816455262, 0.009892318354473776);
                } else {
                    set_output2(var546, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -5.364761590957642) {
                    set_output2(var546, 0.9739706977540085, 0.026029302245991516);
                } else {
                    set_output2(var546, 0.9928777183375012, 0.007122281662498857);
                }
            }
        }
    } else {
        if (input[6] <= 101.93917083740234) {
            set_output2(var546, 0.0, 1.0);
        } else {
            if (input[1] <= 10.020683288574219) {
                if (input[8] <= 0.6116749942302704) {
                    set_output2(var546, 0.8448275862068966, 0.15517241379310345);
                } else {
                    set_output2(var546, 0.33972911963882624, 0.6602708803611739);
                }
            } else {
                if (input[2] <= -2.5843780040740967) {
                    set_output2(var546, 0.8890258939580764, 0.11097410604192358);
                } else {
                    set_output2(var546, 0.23728813559322035, 0.7627118644067796);
                }
            }
        }
    }
    add_vectors(var15, var546, 2, var14);
    double var547[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[1] <= 4.112897634506226) {
                if (input[1] <= 4.112581014633179) {
                    set_output2(var547, 0.9882728516515733, 0.011727148348426608);
                } else {
                    set_output2(var547, 0.0, 1.0);
                }
            } else {
                if (input[4] <= 3.367400050163269) {
                    set_output2(var547, 0.9557043016044886, 0.04429569839551137);
                } else {
                    set_output2(var547, 1.0, 0.0);
                }
            }
        } else {
            if (input[4] <= 0.02199999988079071) {
                set_output2(var547, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var547, 0.9924211802748583, 0.007578819725141636);
                } else {
                    set_output2(var547, 0.9842303720697692, 0.015769627930230783);
                }
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[14] <= 0.5) {
                set_output2(var547, 0.4249201277955272, 0.5750798722044728);
            } else {
                set_output2(var547, 1.0, 0.0);
            }
        } else {
            if (input[9] <= 3.002187967300415) {
                set_output2(var547, 0.6495781959766386, 0.3504218040233615);
            } else {
                set_output2(var547, 0.18490566037735848, 0.8150943396226416);
            }
        }
    }
    add_vectors(var14, var547, 2, var13);
    double var548[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[7] <= 17.16385555267334) {
                    set_output2(var548, 0.9974724780948102, 0.0025275219051898398);
                } else {
                    set_output2(var548, 0.8695904173106646, 0.1304095826893354);
                }
            } else {
                set_output2(var548, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[1] <= 12.703011989593506) {
                    set_output2(var548, 0.9868085480608563, 0.013191451939143726);
                } else {
                    set_output2(var548, 0.0, 1.0);
                }
            } else {
                set_output2(var548, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[1] <= 11.923262119293213) {
                if (input[4] <= 89.85874938964844) {
                    set_output2(var548, 0.7045951859956237, 0.29540481400437635);
                } else {
                    set_output2(var548, 0.1536050156739812, 0.8463949843260188);
                }
            } else {
                set_output2(var548, 0.7117694155324259, 0.2882305844675741);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var548, 1.0, 0.0);
            } else {
                set_output2(var548, 0.0, 1.0);
            }
        }
    }
    add_vectors(var13, var548, 2, var12);
    double var549[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var549, 0.9928169328835492, 0.007183067116450811);
                } else {
                    set_output2(var549, 0.0, 1.0);
                }
            } else {
                set_output2(var549, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var549, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var549, 0.987069135823163, 0.012930864176837052);
                }
            } else {
                set_output2(var549, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[7] <= 17.21262550354004) {
                if (input[4] <= 88.36415100097656) {
                    set_output2(var549, 0.7943640517897943, 0.20563594821020567);
                } else {
                    set_output2(var549, 0.35251798561151076, 0.6474820143884892);
                }
            } else {
                set_output2(var549, 0.0, 1.0);
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var549, 1.0, 0.0);
            } else {
                set_output2(var549, 0.0, 1.0);
            }
        }
    }
    add_vectors(var12, var549, 2, var11);
    double var550[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[7] <= 16.334160804748535) {
            if (input[9] <= 2.150004506111145) {
                set_output2(var550, 1.0, 0.0);
            } else {
                set_output2(var550, 0.6633416458852868, 0.33665835411471323);
            }
        } else {
            set_output2(var550, 0.0, 1.0);
        }
    } else {
        if (input[9] <= 1.4035595059394836) {
            if (input[7] <= 17.760960578918457) {
                if (input[1] <= 8.246700286865234) {
                    set_output2(var550, 0.9966721883689286, 0.0033278116310714776);
                } else {
                    set_output2(var550, 0.9884290253978185, 0.011570974602181396);
                }
            } else {
                if (input[9] <= 0.516086995601654) {
                    set_output2(var550, 0.9862909367859863, 0.013709063214013708);
                } else {
                    set_output2(var550, 0.7335701598579041, 0.2664298401420959);
                }
            }
        } else {
            if (input[3] <= -54.35634803771973) {
                if (input[5] <= 11.175399780273438) {
                    set_output2(var550, 0.3715083798882682, 0.6284916201117319);
                } else {
                    set_output2(var550, 1.0, 0.0);
                }
            } else {
                if (input[8] <= 1.3940170407295227) {
                    set_output2(var550, 0.24916573971078979, 0.7508342602892102);
                } else {
                    set_output2(var550, 1.0, 0.0);
                }
            }
        }
    }
    add_vectors(var11, var550, 2, var10);
    double var551[2];
    if (input[4] <= 0.02199999988079071) {
        if (input[2] <= -3.9435800313949585) {
            set_output2(var551, 0.6232207647222997, 0.37677923527770024);
        } else {
            set_output2(var551, 1.0, 0.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[2] <= -9.043847560882568) {
                if (input[9] <= 1.7348750233650208) {
                    set_output2(var551, 0.9972510186503113, 0.0027489813496888007);
                } else {
                    set_output2(var551, 0.4249201277955272, 0.5750798722044729);
                }
            } else {
                set_output2(var551, 0.0, 1.0);
            }
        } else {
            if (input[1] <= 12.717926025390625) {
                if (input[9] <= 1.1597474813461304) {
                    set_output2(var551, 0.9873576389449275, 0.012642361055072552);
                } else {
                    set_output2(var551, 0.6290189612530915, 0.3709810387469086);
                }
            } else {
                if (input[9] <= 1.495212972164154) {
                    set_output2(var551, 1.0, 0.0);
                } else {
                    set_output2(var551, 0.8389982110912344, 0.16100178890876563);
                }
            }
        }
    }
    add_vectors(var10, var551, 2, var9);
    double var552[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var552, 1.0, 0.0);
            } else {
                set_output2(var552, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[1] <= 5.543766021728516) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var552, 0.9926187156565243, 0.007381284343475747);
                } else {
                    set_output2(var552, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var552, 0.9924123841132183, 0.007587615886781687);
                } else {
                    set_output2(var552, 0.9842091965639208, 0.015790803436079123);
                }
            }
        }
    } else {
        if (input[9] <= 6.277254581451416) {
            if (input[5] <= 123.59455108642578) {
                if (input[5] <= 5.530600070953369) {
                    set_output2(var552, 0.747191011235955, 0.25280898876404495);
                } else {
                    set_output2(var552, 0.24843423799582465, 0.7515657620041754);
                }
            } else {
                if (input[9] <= 1.7081345319747925) {
                    set_output2(var552, 0.5964125560538117, 0.40358744394618834);
                } else {
                    set_output2(var552, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var552, 0.15094339622641512, 0.8490566037735848);
        }
    }
    add_vectors(var9, var552, 2, var8);
    double var553[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var553, 0.9928300183232865, 0.007169981676713484);
                } else {
                    set_output2(var553, 0.0, 1.0);
                }
            } else {
                set_output2(var553, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var553, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var553, 0.987069135823163, 0.01293086417683705);
                }
            } else {
                set_output2(var553, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -54.30373573303223) {
            if (input[10] <= 0.5) {
                set_output2(var553, 1.0, 0.0);
            } else {
                set_output2(var553, 0.42492012779552724, 0.5750798722044729);
            }
        } else {
            if (input[1] <= 6.2525246143341064) {
                set_output2(var553, 0.0, 1.0);
            } else {
                if (input[6] <= 135.9388885498047) {
                    set_output2(var553, 0.0, 1.0);
                } else {
                    set_output2(var553, 0.664179104477612, 0.3358208955223881);
                }
            }
        }
    }
    add_vectors(var8, var553, 2, var7);
    double var554[2];
    if (input[1] <= 5.543766021728516) {
        if (input[5] <= 21.988649368286133) {
            if (input[9] <= 1.6726675033569336) {
                if (input[1] <= 3.8508650064468384) {
                    set_output2(var554, 0.9594740634005764, 0.04052593659942362);
                } else {
                    set_output2(var554, 0.995058094840651, 0.0049419051593489775);
                }
            } else {
                set_output2(var554, 0.3300248138957817, 0.6699751861042184);
            }
        } else {
            if (input[9] <= 4.642098009586334) {
                set_output2(var554, 1.0, 0.0);
            } else {
                set_output2(var554, 0.0, 1.0);
            }
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[1] <= 5.552222490310669) {
                set_output2(var554, 0.0, 1.0);
            } else {
                if (input[5] <= 0.0027000000700354576) {
                    set_output2(var554, 0.6329027872195785, 0.3670972127804215);
                } else {
                    set_output2(var554, 0.9865445815132862, 0.013455418486713836);
                }
            }
        } else {
            set_output2(var554, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var7, var554, 2, var6);
    double var555[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[7] <= 16.476778984069824) {
            if (input[6] <= 140.11605834960938) {
                set_output2(var555, 1.0, 0.0);
            } else {
                set_output2(var555, 0.7439180537772088, 0.25608194622279135);
            }
        } else {
            set_output2(var555, 0.0, 1.0);
        }
    } else {
        if (input[1] <= 5.543766021728516) {
            if (input[9] <= 1.7348750233650208) {
                if (input[5] <= 16.759300231933594) {
                    set_output2(var555, 0.9926187156565243, 0.007381284343475747);
                } else {
                    set_output2(var555, 1.0, 0.0);
                }
            } else {
                set_output2(var555, 0.2697768762677485, 0.7302231237322515);
            }
        } else {
            if (input[9] <= 1.4035595059394836) {
                if (input[8] <= 0.9165079891681671) {
                    set_output2(var555, 0.9913167434893665, 0.008683256510633551);
                } else {
                    set_output2(var555, 0.9840868504340751, 0.01591314956592493);
                }
            } else {
                if (input[5] <= 151.5938491821289) {
                    set_output2(var555, 0.3684210526315789, 0.631578947368421);
                } else {
                    set_output2(var555, 0.897025171624714, 0.10297482837528606);
                }
            }
        }
    }
    add_vectors(var6, var555, 2, var5);
    double var556[2];
    if (input[5] <= 0.060350000858306885) {
        if (input[0] <= 100.0) {
            set_output2(var556, 1.0, 0.0);
        } else {
            if (input[9] <= 6.277254581451416) {
                if (input[7] <= 16.476778984069824) {
                    set_output2(var556, 0.8664292074799644, 0.13357079252003562);
                } else {
                    set_output2(var556, 0.0, 1.0);
                }
            } else {
                set_output2(var556, 0.0, 1.0);
            }
        }
    } else {
        if (input[2] <= -0.9100589752197266) {
            if (input[9] <= 1.4035595059394836) {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var556, 0.9939706248844369, 0.006029375115563167);
                } else {
                    set_output2(var556, 0.9871174958258837, 0.012882504174116315);
                }
            } else {
                if (input[5] <= 123.59455108642578) {
                    set_output2(var556, 0.347462353597323, 0.6525376464026771);
                } else {
                    set_output2(var556, 0.9053627760252365, 0.0946372239747634);
                }
            }
        } else {
            set_output2(var556, 0.3323442136498516, 0.6676557863501483);
        }
    }
    add_vectors(var5, var556, 2, var4);
    double var557[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[3] <= -54.02068901062012) {
                if (input[3] <= -54.048912048339844) {
                    set_output2(var557, 0.9928300183232865, 0.007169981676713478);
                } else {
                    set_output2(var557, 0.0, 1.0);
                }
            } else {
                set_output2(var557, 1.0, 0.0);
            }
        } else {
            if (input[1] <= 12.70374345779419) {
                if (input[4] <= 0.02199999988079071) {
                    set_output2(var557, 0.4249201277955272, 0.5750798722044729);
                } else {
                    set_output2(var557, 0.987069135823163, 0.012930864176837052);
                }
            } else {
                set_output2(var557, 1.0, 0.0);
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var557, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var557, 0.0, 1.0);
                } else {
                    set_output2(var557, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[0] <= 100.0) {
                set_output2(var557, 0.0, 1.0);
            } else {
                set_output2(var557, 0.9463966646813579, 0.053603335318642045);
            }
        }
    }
    add_vectors(var4, var557, 2, var3);
    double var558[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[1] <= 5.543766021728516) {
            if (input[5] <= 16.759300231933594) {
                if (input[5] <= 16.685500144958496) {
                    set_output2(var558, 0.9928169328835492, 0.007183067116450811);
                } else {
                    set_output2(var558, 0.0, 1.0);
                }
            } else {
                set_output2(var558, 1.0, 0.0);
            }
        } else {
            if (input[5] <= 0.0027000000700354576) {
                set_output2(var558, 0.5964125560538117, 0.40358744394618834);
            } else {
                if (input[9] <= 0.23671899735927582) {
                    set_output2(var558, 0.9924211802748584, 0.007578819725141618);
                } else {
                    set_output2(var558, 0.9842303720697692, 0.015769627930230786);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var558, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var558, 0.0, 1.0);
                } else {
                    set_output2(var558, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[3] <= -46.46929168701172) {
                set_output2(var558, 1.0, 0.0);
            } else {
                set_output2(var558, 0.0, 1.0);
            }
        }
    }
    add_vectors(var3, var558, 2, var2);
    double var559[2];
    if (input[9] <= 1.4035595059394836) {
        if (input[5] <= 0.060350000858306885) {
            if (input[7] <= 16.308215141296387) {
                set_output2(var559, 1.0, 0.0);
            } else {
                set_output2(var559, 0.2983367983367983, 0.7016632016632016);
            }
        } else {
            if (input[14] <= 0.5) {
                if (input[8] <= 0.9106729924678802) {
                    set_output2(var559, 0.9916381952376113, 0.008361804762388632);
                } else {
                    set_output2(var559, 0.9743969048702775, 0.025603095129722378);
                }
            } else {
                if (input[2] <= -12.434505939483643) {
                    set_output2(var559, 1.0, 0.0);
                } else {
                    set_output2(var559, 0.9920073048389713, 0.007992695161028768);
                }
            }
        }
    } else {
        if (input[5] <= 172.15264892578125) {
            if (input[4] <= 88.36415100097656) {
                set_output2(var559, 0.7434069850320741, 0.2565930149679259);
            } else {
                if (input[1] <= 9.819966793060303) {
                    set_output2(var559, 0.0, 1.0);
                } else {
                    set_output2(var559, 0.5051546391752577, 0.4948453608247423);
                }
            }
        } else {
            if (input[7] <= 15.72079086303711) {
                set_output2(var559, 0.0, 1.0);
            } else {
                set_output2(var559, 1.0, 0.0);
            }
        }
    }
    add_vectors(var2, var559, 2, var1);
    mul_vector_number(var1, 0.0035714285714285713, 2, var0);
    memcpy(output, var0, 2 * sizeof(double));
}
