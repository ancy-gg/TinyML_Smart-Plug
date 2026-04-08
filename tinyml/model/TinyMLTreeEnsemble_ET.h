#pragma once
// Auto-generated C header from scikit-learn ExtraTrees (m2cgen)
#define ARC_MODEL_FEATURE_VERSION 4
#define ARC_THRESHOLD 0.9500

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
    if (input[9] <= 1.0995002934241203) {
        set_output2(var280, 0.993674203189748, 0.006325796810252027);
    } else {
        if (input[2] <= -3.3455834197662533) {
            if (input[1] <= 9.169957504719488) {
                if (input[1] <= 8.056028423916022) {
                    if (input[5] <= 0.4067815559182794) {
                        if (input[0] <= 33.441620772519094) {
                            set_output2(var280, 0.22813036020583194, 0.7718696397941681);
                        } else {
                            set_output2(var280, 0.8375451263537906, 0.1624548736462094);
                        }
                    } else {
                        set_output2(var280, 0.9193692886579465, 0.0806307113420534);
                    }
                } else {
                    set_output2(var280, 0.8230088495575222, 0.17699115044247787);
                }
            } else {
                if (input[4] <= 3.6412333840655924) {
                    if (input[5] <= 0.18903952469174498) {
                        set_output2(var280, 0.9118511263467189, 0.08814887365328108);
                    } else {
                        set_output2(var280, 0.29319371727748694, 0.706806282722513);
                    }
                } else {
                    set_output2(var280, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var280, 0.713375796178344, 0.28662420382165604);
        }
    }
    double var281[2];
    if (input[0] <= 26.25989026534254) {
        set_output2(var281, 0.8968835930339139, 0.10311640696608616);
    } else {
        set_output2(var281, 0.9935682458321883, 0.006431754167811758);
    }
    add_vectors(var280, var281, 2, var279);
    add_vectors(var279, (double[]){0.9920612592235635, 0.007938740776436509}, 2, var278);
    add_vectors(var278, (double[]){0.9916433434704222, 0.008356656529577832}, 2, var277);
    double var282[2];
    if (input[2] <= -0.6396185858966383) {
        set_output2(var282, 0.9927737626843157, 0.007226237315684328);
    } else {
        if (input[7] <= 13.626769371560343) {
            set_output2(var282, 1.0, 0.0);
        } else {
            set_output2(var282, 0.0, 1.0);
        }
    }
    add_vectors(var277, var282, 2, var276);
    add_vectors(var276, (double[]){0.99035721622615, 0.009642783773849938}, 2, var275);
    double var283[2];
    if (input[3] <= -25.66742164818333) {
        if (input[9] <= 1.0971575972540992) {
            set_output2(var283, 0.9933742718418239, 0.006625728158176048);
        } else {
            if (input[6] <= 21.064252044655603) {
                if (input[9] <= 2.649313337080425) {
                    if (input[8] <= 0.7261160115651236) {
                        set_output2(var283, 1.0, 0.0);
                    } else {
                        if (input[6] <= 2.627447274129465) {
                            set_output2(var283, 1.0, 0.0);
                        } else {
                            set_output2(var283, 0.7707006369426752, 0.22929936305732485);
                        }
                    }
                } else {
                    if (input[1] <= 5.335515397415798) {
                        set_output2(var283, 0.0, 1.0);
                    } else {
                        if (input[5] <= 1.4166311168770929) {
                            set_output2(var283, 0.702774108322325, 0.29722589167767505);
                        } else {
                            set_output2(var283, 1.0, 0.0);
                        }
                    }
                }
            } else {
                set_output2(var283, 0.7321428571428572, 0.2678571428571428);
            }
        }
    } else {
        set_output2(var283, 0.9166512317095759, 0.08334876829042415);
    }
    add_vectors(var275, var283, 2, var274);
    add_vectors(var274, (double[]){0.9923777233686706, 0.007622276631329433}, 2, var273);
    add_vectors(var273, (double[]){0.9922455817280758, 0.007754418271924217}, 2, var272);
    add_vectors(var272, (double[]){0.992424577851818, 0.00757542214818198}, 2, var271);
    double var284[2];
    if (input[6] <= 39.69154359758438) {
        if (input[9] <= 5.141848304840665) {
            if (input[9] <= 3.3744984031866574) {
                set_output2(var284, 0.9907684733106307, 0.009231526689369305);
            } else {
                set_output2(var284, 0.569377990430622, 0.43062200956937796);
            }
        } else {
            set_output2(var284, 0.1106719367588933, 0.8893280632411067);
        }
    } else {
        set_output2(var284, 0.9986931900682445, 0.0013068099317554815);
    }
    add_vectors(var271, var284, 2, var270);
    double var285[2];
    if (input[0] <= 9.486184895641234) {
        set_output2(var285, 0.6603773584905661, 0.33962264150943394);
    } else {
        set_output2(var285, 0.99075588821908, 0.009244111780919953);
    }
    add_vectors(var270, var285, 2, var269);
    double var286[2];
    if (input[9] <= 1.8167190611440311) {
        if (input[9] <= 1.416175554064424) {
            set_output2(var286, 0.9940674684216289, 0.005932531578371133);
        } else {
            set_output2(var286, 0.7058823529411765, 0.2941176470588235);
        }
    } else {
        if (input[4] <= 0.9550007753672758) {
            if (input[1] <= 9.288495453012501) {
                set_output2(var286, 0.6363636363636364, 0.36363636363636365);
            } else {
                set_output2(var286, 0.9315589353612167, 0.06844106463878327);
            }
        } else {
            if (input[3] <= -39.29481656477566) {
                if (input[9] <= 4.986157365375122) {
                    if (input[5] <= 1.1419627397914272) {
                        set_output2(var286, 0.773071104387292, 0.22692889561270801);
                    } else {
                        set_output2(var286, 1.0, 0.0);
                    }
                } else {
                    set_output2(var286, 0.0, 1.0);
                }
            } else {
                set_output2(var286, 0.0, 1.0);
            }
        }
    }
    add_vectors(var269, var286, 2, var268);
    double var287[2];
    if (input[2] <= 1.5672543791415308) {
        if (input[9] <= 4.98132505789776) {
            if (input[9] <= 0.7971500631438488) {
                set_output2(var287, 0.9940127357663484, 0.005987264233651495);
            } else {
                if (input[5] <= 2.8714361337066037) {
                    if (input[9] <= 2.247477633243391) {
                        set_output2(var287, 0.9543124016447535, 0.04568759835524647);
                    } else {
                        if (input[0] <= 27.89437071477042) {
                            set_output2(var287, 0.46996466431095407, 0.5300353356890459);
                        } else {
                            set_output2(var287, 0.8389982110912344, 0.16100178890876568);
                        }
                    }
                } else {
                    set_output2(var287, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var287, 0.0, 1.0);
        }
    } else {
        set_output2(var287, 0.29319371727748694, 0.706806282722513);
    }
    add_vectors(var268, var287, 2, var267);
    add_vectors(var267, (double[]){0.9921976322414644, 0.007802367758535586}, 2, var266);
    add_vectors(var266, (double[]){0.9922946203955152, 0.00770537960448476}, 2, var265);
    add_vectors(var265, (double[]){0.9932615890301897, 0.006738410969810363}, 2, var264);
    add_vectors(var264, (double[]){0.9911592969872967, 0.00884070301270335}, 2, var263);
    add_vectors(var263, (double[]){0.9906847553753965, 0.009315244624603548}, 2, var262);
    double var288[2];
    if (input[9] <= 8.11020050109839) {
        if (input[9] <= 4.306156930038927) {
            set_output2(var288, 0.9931255076303395, 0.006874492369660571);
        } else {
            set_output2(var288, 0.0, 1.0);
        }
    } else {
        set_output2(var288, 0.3835616438356165, 0.6164383561643836);
    }
    add_vectors(var262, var288, 2, var261);
    add_vectors(var261, (double[]){0.9909800669848106, 0.009019933015189413}, 2, var260);
    double var289[2];
    if (input[0] <= 13.584166905168052) {
        set_output2(var289, 0.83125, 0.16874999999999998);
    } else {
        if (input[9] <= 5.82097836918112) {
            if (input[3] <= -38.797036300018114) {
                if (input[9] <= 0.9332482757830867) {
                    set_output2(var289, 0.9939435670227225, 0.006056432977277509);
                } else {
                    if (input[3] <= -44.064016793557464) {
                        set_output2(var289, 0.9466535699769679, 0.05334643002303211);
                    } else {
                        set_output2(var289, 0.7471910112359551, 0.25280898876404495);
                    }
                }
            } else {
                if (input[3] <= -36.77104062569689) {
                    set_output2(var289, 1.0, 0.0);
                } else {
                    if (input[5] <= 0.677648733701749) {
                        if (input[9] <= 0.37270716461011166) {
                            set_output2(var289, 1.0, 0.0);
                        } else {
                            set_output2(var289, 0.64, 0.36);
                        }
                    } else {
                        set_output2(var289, 1.0, 0.0);
                    }
                }
            }
        } else {
            set_output2(var289, 0.0, 1.0);
        }
    }
    add_vectors(var260, var289, 2, var259);
    add_vectors(var259, (double[]){0.9932178127828811, 0.00678218721711886}, 2, var258);
    double var290[2];
    if (input[9] <= 3.405776528364875) {
        set_output2(var290, 0.992055260079889, 0.00794473992011103);
    } else {
        if (input[0] <= 28.59825595709533) {
            set_output2(var290, 0.7395079594790159, 0.26049204052098407);
        } else {
            set_output2(var290, 0.3715083798882682, 0.6284916201117319);
        }
    }
    add_vectors(var258, var290, 2, var257);
    double var291[2];
    if (input[9] <= 0.2619868378443582) {
        set_output2(var291, 0.9936367475965698, 0.006363252403430164);
    } else {
        if (input[8] <= 0.0663710462985631) {
            if (input[0] <= 36.08686437116307) {
                set_output2(var291, 1.0, 0.0);
            } else {
                set_output2(var291, 0.5714285714285714, 0.42857142857142855);
            }
        } else {
            set_output2(var291, 0.9862352590861335, 0.013764740913866466);
        }
    }
    add_vectors(var257, var291, 2, var256);
    add_vectors(var256, (double[]){0.9925830594330438, 0.007416940566956229}, 2, var255);
    add_vectors(var255, (double[]){0.9921907719219496, 0.007809228078050435}, 2, var254);
    add_vectors(var254, (double[]){0.9910488607304502, 0.008951139269549787}, 2, var253);
    double var292[2];
    if (input[1] <= 8.44339069892991) {
        set_output2(var292, 0.9945829438570283, 0.00541705614297177);
    } else {
        if (input[0] <= 28.159282374856613) {
            set_output2(var292, 0.92776886035313, 0.07223113964687003);
        } else {
            set_output2(var292, 0.9911709715393671, 0.008829028460632906);
        }
    }
    add_vectors(var253, var292, 2, var252);
    add_vectors(var252, (double[]){0.99057270310645, 0.00942729689355005}, 2, var251);
    double var293[2];
    if (input[3] <= -39.6715927388616) {
        if (input[2] <= -10.984636727219485) {
            set_output2(var293, 0.9972104613896862, 0.002789538610313851);
        } else {
            if (input[9] <= 8.537037542575362) {
                if (input[1] <= 9.3560944256634) {
                    if (input[9] <= 1.0157649610424018) {
                        set_output2(var293, 0.9962964296553107, 0.0037035703446892413);
                    } else {
                        set_output2(var293, 0.8162014976174268, 0.18379850238257317);
                    }
                } else {
                    set_output2(var293, 0.9893086949348292, 0.010691305065170792);
                }
            } else {
                set_output2(var293, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var293, 0.9527161920773353, 0.0472838079226647);
    }
    add_vectors(var251, var293, 2, var250);
    double var294[2];
    if (input[1] <= 8.017084443516792) {
        if (input[9] <= 2.6219418168170106) {
            if (input[9] <= 1.100066880478915) {
                set_output2(var294, 0.9976766713929194, 0.0023233286070806084);
            } else {
                set_output2(var294, 0.8612930569469062, 0.13870694305309395);
            }
        } else {
            set_output2(var294, 0.47572815533980584, 0.5242718446601942);
        }
    } else {
        if (input[9] <= 3.881137389402488) {
            if (input[4] <= 6.620065840298804) {
                if (input[9] <= 0.44940683826720007) {
                    set_output2(var294, 0.9883083064168502, 0.01169169358314984);
                } else {
                    if (input[9] <= 2.0749704703580436) {
                        if (input[9] <= 0.7726116035074092) {
                            set_output2(var294, 0.9914998111069135, 0.008500188893086505);
                        } else {
                            set_output2(var294, 0.8720783322804802, 0.12792166771951988);
                        }
                    } else {
                        if (input[3] <= -55.5762496308415) {
                            set_output2(var294, 1.0, 0.0);
                        } else {
                            set_output2(var294, 0.4049586776859504, 0.5950413223140496);
                        }
                    }
                }
            } else {
                set_output2(var294, 0.9951443543002813, 0.004855645699718779);
            }
        } else {
            set_output2(var294, 0.5964125560538117, 0.4035874439461883);
        }
    }
    add_vectors(var250, var294, 2, var249);
    double var295[2];
    if (input[9] <= 2.8811650031642317) {
        if (input[9] <= 0.79007337240275) {
            if (input[9] <= 0.4897699474451561) {
                set_output2(var295, 0.9954554117629999, 0.00454458823700009);
            } else {
                if (input[7] <= 17.429507473136958) {
                    if (input[3] <= -61.59011978607717) {
                        set_output2(var295, 0.9160186625194401, 0.08398133748055989);
                    } else {
                        set_output2(var295, 0.9934101852177275, 0.006589814782272514);
                    }
                } else {
                    if (input[2] <= -6.735524190492999) {
                        set_output2(var295, 1.0, 0.0);
                    } else {
                        set_output2(var295, 0.7595725734639359, 0.2404274265360641);
                    }
                }
            }
        } else {
            if (input[1] <= 10.751746909016083) {
                if (input[1] <= 8.440825744856316) {
                    set_output2(var295, 1.0, 0.0);
                } else {
                    set_output2(var295, 0.8971428571428571, 0.10285714285714287);
                }
            } else {
                if (input[6] <= 12.055484308629024) {
                    if (input[9] <= 0.9506106991479408) {
                        set_output2(var295, 1.0, 0.0);
                    } else {
                        if (input[3] <= -55.63197927264799) {
                            set_output2(var295, 0.8880597014925373, 0.11194029850746268);
                        } else {
                            set_output2(var295, 0.584295612009238, 0.41570438799076215);
                        }
                    }
                } else {
                    set_output2(var295, 1.0, 0.0);
                }
            }
        }
    } else {
        if (input[9] <= 3.4560862623378115) {
            set_output2(var295, 0.8850574712643678, 0.11494252873563217);
        } else {
            set_output2(var295, 0.3300248138957817, 0.6699751861042184);
        }
    }
    add_vectors(var249, var295, 2, var248);
    add_vectors(var248, (double[]){0.9913396083879895, 0.008660391612010538}, 2, var247);
    add_vectors(var247, (double[]){0.9903807056472695, 0.009619294352730451}, 2, var246);
    add_vectors(var246, (double[]){0.9914836408802062, 0.008516359119793775}, 2, var245);
    double var296[2];
    if (input[2] <= -0.6802385682842953) {
        set_output2(var296, 0.9937424576335413, 0.006257542366458754);
    } else {
        if (input[8] <= 0.7146726561569088) {
            set_output2(var296, 1.0, 0.0);
        } else {
            set_output2(var296, 0.0, 1.0);
        }
    }
    add_vectors(var245, var296, 2, var244);
    add_vectors(var244, (double[]){0.9918236327296955, 0.008176367270304465}, 2, var243);
    add_vectors(var243, (double[]){0.9914723664229399, 0.008527633577060142}, 2, var242);
    double var297[2];
    if (input[8] <= 1.1826397882871078) {
        set_output2(var297, 0.9935053324427889, 0.006494667557211079);
    } else {
        if (input[7] <= 17.47120398652335) {
            set_output2(var297, 0.9885430590032461, 0.011456940996753871);
        } else {
            if (input[9] <= 0.5286093846205621) {
                set_output2(var297, 1.0, 0.0);
            } else {
                set_output2(var297, 0.7094430992736077, 0.29055690072639223);
            }
        }
    }
    add_vectors(var242, var297, 2, var241);
    add_vectors(var241, (double[]){0.9914363811307042, 0.00856361886929577}, 2, var240);
    add_vectors(var240, (double[]){0.991656944689715, 0.00834305531028497}, 2, var239);
    add_vectors(var239, (double[]){0.990698043871788, 0.00930195612821207}, 2, var238);
    add_vectors(var238, (double[]){0.99156162490925, 0.008438375090749933}, 2, var237);
    add_vectors(var237, (double[]){0.9917995954928457, 0.008200404507154323}, 2, var236);
    add_vectors(var236, (double[]){0.9913147028477313, 0.008685297152268739}, 2, var235);
    add_vectors(var235, (double[]){0.9923802834673725, 0.0076197165326275}, 2, var234);
    double var298[2];
    if (input[6] <= 27.821015672674875) {
        if (input[9] <= 2.2073988849411648) {
            set_output2(var298, 0.9916604543527945, 0.00833954564720549);
        } else {
            if (input[6] <= 5.47939427970621) {
                if (input[2] <= -7.508191050922754) {
                    set_output2(var298, 0.7226502311248074, 0.2773497688751926);
                } else {
                    set_output2(var298, 0.3979933110367893, 0.6020066889632106);
                }
            } else {
                set_output2(var298, 1.0, 0.0);
            }
        }
    } else {
        if (input[9] <= 6.227216666649605) {
            set_output2(var298, 0.9976310428149529, 0.0023689571850471426);
        } else {
            set_output2(var298, 0.29319371727748694, 0.706806282722513);
        }
    }
    add_vectors(var234, var298, 2, var233);
    double var299[2];
    if (input[9] <= 10.36245463665689) {
        set_output2(var299, 0.9912606268350085, 0.008739373164991541);
    } else {
        set_output2(var299, 0.5544554455445545, 0.44554455445544555);
    }
    add_vectors(var233, var299, 2, var232);
    double var300[2];
    if (input[0] <= 27.08495389913059) {
        if (input[8] <= 0.7491900380383295) {
            set_output2(var300, 1.0, 0.0);
        } else {
            set_output2(var300, 0.8956642708091815, 0.10433572919081843);
        }
    } else {
        if (input[9] <= 1.9391543706502086) {
            set_output2(var300, 0.9927962557479877, 0.007203744252012328);
        } else {
            if (input[9] <= 5.427791826318312) {
                if (input[3] <= -53.7995316342555) {
                    set_output2(var300, 1.0, 0.0);
                } else {
                    if (input[4] <= 1.013304517447581) {
                        set_output2(var300, 0.4962686567164179, 0.503731343283582);
                    } else {
                        if (input[1] <= 8.9687747008751) {
                            set_output2(var300, 0.3835616438356165, 0.6164383561643836);
                        } else {
                            set_output2(var300, 1.0, 0.0);
                        }
                    }
                }
            } else {
                set_output2(var300, 0.0, 1.0);
            }
        }
    }
    add_vectors(var232, var300, 2, var231);
    double var301[2];
    if (input[9] <= 6.172622556878544) {
        set_output2(var301, 0.9925562891319027, 0.007443710868097247);
    } else {
        set_output2(var301, 0.29319371727748694, 0.706806282722513);
    }
    add_vectors(var231, var301, 2, var230);
    double var302[2];
    if (input[9] <= 4.304643082108648) {
        if (input[9] <= 3.495007000506056) {
            if (input[9] <= 1.9907470529940823) {
                set_output2(var302, 0.9919073348973704, 0.008092665102629617);
            } else {
                if (input[5] <= 0.7405842925744288) {
                    if (input[0] <= 32.49771285358554) {
                        set_output2(var302, 0.91, 0.09);
                    } else {
                        set_output2(var302, 0.4249201277955272, 0.5750798722044729);
                    }
                } else {
                    set_output2(var302, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var302, 0.8389982110912344, 0.16100178890876563);
        }
    } else {
        if (input[6] <= 11.289020400149067) {
            set_output2(var302, 0.0, 1.0);
        } else {
            set_output2(var302, 0.4049586776859504, 0.5950413223140496);
        }
    }
    add_vectors(var230, var302, 2, var229);
    add_vectors(var229, (double[]){0.9914499950993877, 0.008550004900612336}, 2, var228);
    double var303[2];
    if (input[8] <= 0.026619359187283052) {
        if (input[5] <= 0.05487186739676315) {
            if (input[7] <= 9.80701161293488) {
                set_output2(var303, 1.0, 0.0);
            } else {
                set_output2(var303, 0.6065573770491803, 0.39344262295081966);
            }
        } else {
            set_output2(var303, 1.0, 0.0);
        }
    } else {
        set_output2(var303, 0.9898985387721908, 0.010101461227809158);
    }
    add_vectors(var228, var303, 2, var227);
    double var304[2];
    if (input[9] <= 2.282796987464091) {
        set_output2(var304, 0.9935379405115312, 0.0064620594884687525);
    } else {
        if (input[1] <= 9.060802059942727) {
            if (input[0] <= 29.126713020822383) {
                set_output2(var304, 0.7815533980582524, 0.21844660194174756);
            } else {
                if (input[3] <= -57.79554568487896) {
                    set_output2(var304, 0.713375796178344, 0.28662420382165604);
                } else {
                    set_output2(var304, 0.0, 1.0);
                }
            }
        } else {
            set_output2(var304, 0.806243272335845, 0.193756727664155);
        }
    }
    add_vectors(var227, var304, 2, var226);
    add_vectors(var226, (double[]){0.9931587706518173, 0.006841229348182692}, 2, var225);
    double var305[2];
    if (input[0] <= 26.612343611808097) {
        if (input[8] <= 0.7851073619188454) {
            set_output2(var305, 0.9660684662946766, 0.03393153370532348);
        } else {
            set_output2(var305, 0.6557377049180327, 0.3442622950819672);
        }
    } else {
        set_output2(var305, 0.993199962710223, 0.0068000372897769985);
    }
    add_vectors(var225, var305, 2, var224);
    double var306[2];
    if (input[0] <= 25.183284433873478) {
        set_output2(var306, 0.8455730954015099, 0.15442690459849004);
    } else {
        if (input[9] <= 4.551645753972365) {
            if (input[0] <= 30.065578777207744) {
                if (input[9] <= 1.3455154999904604) {
                    set_output2(var306, 0.9880005817899739, 0.011999418210026171);
                } else {
                    set_output2(var306, 0.6976483762597985, 0.3023516237402016);
                }
            } else {
                set_output2(var306, 0.992886627351786, 0.007113372648213987);
            }
        } else {
            set_output2(var306, 0.19762258543833583, 0.8023774145616642);
        }
    }
    add_vectors(var224, var306, 2, var223);
    double var307[2];
    if (input[3] <= -25.288234381963186) {
        if (input[9] <= 1.7164926151215627) {
            set_output2(var307, 0.9935042877521367, 0.0064957122478632515);
        } else {
            if (input[5] <= 2.6576600783088717) {
                if (input[1] <= 6.205637739712737) {
                    set_output2(var307, 0.0, 1.0);
                } else {
                    if (input[8] <= 0.9787068424322365) {
                        set_output2(var307, 0.6590909090909092, 0.34090909090909094);
                    } else {
                        set_output2(var307, 1.0, 0.0);
                    }
                }
            } else {
                set_output2(var307, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var307, 0.8712446351931331, 0.12875536480686695);
    }
    add_vectors(var223, var307, 2, var222);
    add_vectors(var222, (double[]){0.9909535898119191, 0.00904641018808096}, 2, var221);
    double var308[2];
    if (input[9] <= 8.486583611688848) {
        if (input[9] <= 1.2069552433646562) {
            set_output2(var308, 0.9939756857266733, 0.006024314273326735);
        } else {
            if (input[4] <= 5.305623757372642) {
                if (input[4] <= 1.563840867093359) {
                    set_output2(var308, 0.468503937007874, 0.5314960629921259);
                } else {
                    if (input[1] <= 8.184138405318208) {
                        if (input[5] <= 0.8988507380335319) {
                            set_output2(var308, 0.7395079594790159, 0.26049204052098407);
                        } else {
                            set_output2(var308, 0.23728813559322035, 0.7627118644067796);
                        }
                    } else {
                        if (input[1] <= 11.163259980017404) {
                            set_output2(var308, 0.6239554317548747, 0.3760445682451253);
                        } else {
                            set_output2(var308, 1.0, 0.0);
                        }
                    }
                }
            } else {
                set_output2(var308, 0.9483352468427095, 0.051664753157290466);
            }
        }
    } else {
        set_output2(var308, 0.0, 1.0);
    }
    add_vectors(var221, var308, 2, var220);
    add_vectors(var220, (double[]){0.9929934152046378, 0.0070065847953622315}, 2, var219);
    add_vectors(var219, (double[]){0.9928601699348542, 0.0071398300651458685}, 2, var218);
    add_vectors(var218, (double[]){0.9906150676132243, 0.009384932386775763}, 2, var217);
    double var309[2];
    if (input[9] <= 0.524610758537711) {
        set_output2(var309, 0.9942876210026949, 0.005712378997305148);
    } else {
        if (input[6] <= 0.39484602978664635) {
            if (input[4] <= 5.080544088907575) {
                if (input[5] <= 0.2513469435196608) {
                    set_output2(var309, 0.5800933125972006, 0.4199066874027994);
                } else {
                    set_output2(var309, 0.9658728954952222, 0.034127104504777794);
                }
            } else {
                set_output2(var309, 0.664804469273743, 0.33519553072625696);
            }
        } else {
            if (input[2] <= 0.27252923258450323) {
                if (input[2] <= -18.233326733564418) {
                    set_output2(var309, 0.8285714285714286, 0.17142857142857143);
                } else {
                    set_output2(var309, 0.9755381000269328, 0.02446189997306717);
                }
            } else {
                set_output2(var309, 0.17177914110429449, 0.8282208588957055);
            }
        }
    }
    add_vectors(var217, var309, 2, var216);
    double var310[2];
    if (input[2] <= -0.4399261164292483) {
        if (input[9] <= 4.019584342995928) {
            set_output2(var310, 0.9923096890527847, 0.007690310947215257);
        } else {
            set_output2(var310, 0.0, 1.0);
        }
    } else {
        set_output2(var310, 0.6380697050938339, 0.3619302949061662);
    }
    add_vectors(var216, var310, 2, var215);
    add_vectors(var215, (double[]){0.992748356697054, 0.007251643302946074}, 2, var214);
    add_vectors(var214, (double[]){0.9912590505247694, 0.008740949475230637}, 2, var213);
    add_vectors(var213, (double[]){0.991094695861164, 0.008905304138835909}, 2, var212);
    add_vectors(var212, (double[]){0.992820153516911, 0.007179846483089034}, 2, var211);
    add_vectors(var211, (double[]){0.9925785282303962, 0.007421471769603881}, 2, var210);
    add_vectors(var210, (double[]){0.9898822844943984, 0.01011771550560157}, 2, var209);
    add_vectors(var209, (double[]){0.9908803629405905, 0.009119637059409491}, 2, var208);
    double var311[2];
    if (input[2] <= -8.80841371327869) {
        if (input[9] <= 1.8043802483873206) {
            set_output2(var311, 0.9970712743355409, 0.0029287256644591588);
        } else {
            set_output2(var311, 0.6674057649667405, 0.3325942350332594);
        }
    } else {
        if (input[9] <= 0.3013574614429394) {
            set_output2(var311, 0.9944116965261449, 0.005588303473855033);
        } else {
            if (input[0] <= 28.44470282781992) {
                if (input[0] <= 27.972198884767632) {
                    if (input[3] <= -46.324430987297674) {
                        set_output2(var311, 0.9773982923154194, 0.022601707684580613);
                    } else {
                        set_output2(var311, 0.2697768762677485, 0.7302231237322515);
                    }
                } else {
                    set_output2(var311, 1.0, 0.0);
                }
            } else {
                if (input[0] <= 36.68701378892284) {
                    set_output2(var311, 0.9871368431994179, 0.012863156800582098);
                } else {
                    if (input[1] <= 7.155701571012225) {
                        set_output2(var311, 0.7227926078028748, 0.27720739219712526);
                    } else {
                        if (input[1] <= 7.838658669783809) {
                            set_output2(var311, 0.850808122668877, 0.14919187733112307);
                        } else {
                            set_output2(var311, 0.9874438460894556, 0.012556153910544378);
                        }
                    }
                }
            }
        }
    }
    add_vectors(var208, var311, 2, var207);
    double var312[2];
    if (input[9] <= 0.8658314709858184) {
        set_output2(var312, 0.9947611599765929, 0.00523884002340722);
    } else {
        if (input[6] <= 40.66099319396323) {
            if (input[8] <= 1.3890280309151617) {
                if (input[1] <= 9.30027629907028) {
                    set_output2(var312, 0.9327675850116536, 0.06723241498834638);
                } else {
                    if (input[3] <= -36.150211730222935) {
                        set_output2(var312, 0.9313108185460791, 0.068689181453921);
                    } else {
                        set_output2(var312, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var312, 0.5800933125972006, 0.4199066874027994);
            }
        } else {
            set_output2(var312, 0.9475524475524475, 0.05244755244755245);
        }
    }
    add_vectors(var207, var312, 2, var206);
    add_vectors(var206, (double[]){0.9926270875495856, 0.00737291245041437}, 2, var205);
    add_vectors(var205, (double[]){0.9893558750699755, 0.010644124930024484}, 2, var204);
    double var313[2];
    if (input[2] <= -9.012665199385669) {
        set_output2(var313, 0.9968017807684681, 0.003198219231531885);
    } else {
        if (input[9] <= 3.042121154576297) {
            if (input[9] <= 1.4088281783812169) {
                set_output2(var313, 0.9931747437943599, 0.006825256205640129);
            } else {
                if (input[5] <= 1.186965045713525) {
                    if (input[3] <= -44.43464784736207) {
                        if (input[6] <= 17.449372123346205) {
                            set_output2(var313, 0.7670405522001725, 0.2329594477998274);
                        } else {
                            set_output2(var313, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var313, 0.5417515274949084, 0.45824847250509165);
                    }
                } else {
                    set_output2(var313, 1.0, 0.0);
                }
            }
        } else {
            if (input[8] <= 0.7081263235815183) {
                set_output2(var313, 0.8658718330849479, 0.13412816691505214);
            } else {
                set_output2(var313, 0.5121951219512195, 0.4878048780487804);
            }
        }
    }
    add_vectors(var204, var313, 2, var203);
    add_vectors(var203, (double[]){0.9917075535206861, 0.008292446479313945}, 2, var202);
    add_vectors(var202, (double[]){0.9916244262441161, 0.008375573755883943}, 2, var201);
    add_vectors(var201, (double[]){0.9913083726550934, 0.008691627344906598}, 2, var200);
    double var314[2];
    if (input[1] <= 5.916154416043647) {
        if (input[9] <= 3.1075549442821906) {
            set_output2(var314, 0.9981613380882493, 0.0018386619117507635);
        } else {
            set_output2(var314, 0.0, 1.0);
        }
    } else {
        set_output2(var314, 0.9888006154005277, 0.011199384599472353);
    }
    add_vectors(var200, var314, 2, var199);
    add_vectors(var199, (double[]){0.9918894188110029, 0.008110581188997124}, 2, var198);
    add_vectors(var198, (double[]){0.9931055599132649, 0.006894440086735131}, 2, var197);
    add_vectors(var197, (double[]){0.9920889132132963, 0.007911086786703722}, 2, var196);
    double var315[2];
    if (input[9] <= 5.124464086314879) {
        set_output2(var315, 0.9935114875740336, 0.006488512425966357);
    } else {
        if (input[5] <= 0.23879786932054922) {
            set_output2(var315, 0.5544554455445545, 0.44554455445544555);
        } else {
            set_output2(var315, 0.0, 1.0);
        }
    }
    add_vectors(var196, var315, 2, var195);
    double var316[2];
    if (input[9] <= 8.417812552497635) {
        set_output2(var316, 0.9905163505527382, 0.009483649447261766);
    } else {
        set_output2(var316, 0.23728813559322035, 0.7627118644067796);
    }
    add_vectors(var195, var316, 2, var194);
    double var317[2];
    if (input[9] <= 3.8335135108016027) {
        set_output2(var317, 0.9919709554817868, 0.008029044518213164);
    } else {
        if (input[3] <= -50.910920135887864) {
            set_output2(var317, 0.7471910112359551, 0.25280898876404495);
        } else {
            set_output2(var317, 0.0, 1.0);
        }
    }
    add_vectors(var194, var317, 2, var193);
    add_vectors(var193, (double[]){0.9909614769740709, 0.009038523025929147}, 2, var192);
    add_vectors(var192, (double[]){0.9916106189988255, 0.008389381001174532}, 2, var191);
    add_vectors(var191, (double[]){0.9903781972739157, 0.009621802726084226}, 2, var190);
    add_vectors(var190, (double[]){0.9906790579727687, 0.009320942027231343}, 2, var189);
    double var318[2];
    if (input[0] <= 25.15600275636854) {
        if (input[1] <= 2.4501208917386075) {
            set_output2(var318, 1.0, 0.0);
        } else {
            if (input[9] <= 1.1849683823862767) {
                set_output2(var318, 0.8680351906158358, 0.13196480938416422);
            } else {
                set_output2(var318, 0.3323442136498516, 0.6676557863501483);
            }
        }
    } else {
        set_output2(var318, 0.9919879057842467, 0.008012094215753359);
    }
    add_vectors(var189, var318, 2, var188);
    double var319[2];
    if (input[9] <= 3.885596829413249) {
        set_output2(var319, 0.992506477040447, 0.007493522959552957);
    } else {
        if (input[7] <= 16.74736828405159) {
            set_output2(var319, 0.5964125560538117, 0.4035874439461883);
        } else {
            set_output2(var319, 0.23728813559322035, 0.7627118644067796);
        }
    }
    add_vectors(var188, var319, 2, var187);
    double var320[2];
    if (input[9] <= 5.739951893100493) {
        if (input[9] <= 1.1720903308382242) {
            set_output2(var320, 0.9942260491268737, 0.005773950873126303);
        } else {
            if (input[4] <= 7.753406132461043) {
                if (input[6] <= 66.98888157705144) {
                    if (input[8] <= 0.5791198344820954) {
                        set_output2(var320, 0.5544554455445544, 0.44554455445544555);
                    } else {
                        set_output2(var320, 0.9388690779419256, 0.06113092205807437);
                    }
                } else {
                    set_output2(var320, 0.4534412955465587, 0.5465587044534412);
                }
            } else {
                set_output2(var320, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -40.831924622174405) {
            set_output2(var320, 0.0, 1.0);
        } else {
            set_output2(var320, 0.5544554455445545, 0.44554455445544555);
        }
    }
    add_vectors(var187, var320, 2, var186);
    add_vectors(var186, (double[]){0.9914588987858676, 0.008541101214132377}, 2, var185);
    add_vectors(var185, (double[]){0.9932202274109568, 0.00677977258904316}, 2, var184);
    add_vectors(var184, (double[]){0.990660978841346, 0.009339021158654}, 2, var183);
    add_vectors(var183, (double[]){0.9916539621423724, 0.008346037857627503}, 2, var182);
    add_vectors(var182, (double[]){0.9923658232431286, 0.007634176756871369}, 2, var181);
    add_vectors(var181, (double[]){0.990202373110309, 0.009797626889690974}, 2, var180);
    add_vectors(var180, (double[]){0.992619488062744, 0.007380511937256005}, 2, var179);
    double var321[2];
    if (input[9] <= 1.470113229567741) {
        set_output2(var321, 0.9936757693426596, 0.006324230657340482);
    } else {
        if (input[1] <= 9.34058324863815) {
            set_output2(var321, 0.6226415094339623, 0.37735849056603765);
        } else {
            if (input[1] <= 10.478815684858292) {
                set_output2(var321, 0.42748091603053434, 0.5725190839694656);
            } else {
                set_output2(var321, 0.9304482225656877, 0.0695517774343122);
            }
        }
    }
    add_vectors(var179, var321, 2, var178);
    double var322[2];
    if (input[9] <= 0.47349497930775164) {
        set_output2(var322, 0.9955293311173457, 0.004470668882654301);
    } else {
        if (input[3] <= -39.68608246004188) {
            if (input[9] <= 6.191898267154825) {
                if (input[2] <= -14.396241049583251) {
                    set_output2(var322, 0.9397267613179748, 0.06027323868202518);
                } else {
                    if (input[3] <= -58.395691510302335) {
                        set_output2(var322, 0.9436255063264709, 0.056374493673529064);
                    } else {
                        if (input[9] <= 2.0548969679330744) {
                            set_output2(var322, 0.9927697292665292, 0.007230270733470791);
                        } else {
                            set_output2(var322, 0.8651685393258427, 0.1348314606741573);
                        }
                    }
                }
            } else {
                set_output2(var322, 0.0, 1.0);
            }
        } else {
            if (input[7] <= 16.413702934122185) {
                set_output2(var322, 0.715752072641137, 0.284247927358863);
            } else {
                set_output2(var322, 0.9369747899159664, 0.06302521008403361);
            }
        }
    }
    add_vectors(var178, var322, 2, var177);
    double var323[2];
    if (input[9] <= 8.807991542298343) {
        set_output2(var323, 0.9923390536620724, 0.00766094633792757);
    } else {
        set_output2(var323, 0.15094339622641512, 0.8490566037735848);
    }
    add_vectors(var177, var323, 2, var176);
    add_vectors(var176, (double[]){0.9908291894856196, 0.009170810514380345}, 2, var175);
    add_vectors(var175, (double[]){0.9925984098743469, 0.007401590125653097}, 2, var174);
    add_vectors(var174, (double[]){0.9917550056161054, 0.008244994383894616}, 2, var173);
    double var324[2];
    if (input[9] <= 3.6926658957696827) {
        if (input[2] <= -12.058092863075522) {
            set_output2(var324, 0.9977313545703753, 0.002268645429624727);
        } else {
            if (input[9] <= 1.8865994201632432) {
                set_output2(var324, 0.9896837525693905, 0.010316247430609535);
            } else {
                if (input[1] <= 10.423550078036595) {
                    if (input[0] <= 31.13854872751909) {
                        if (input[3] <= -54.98656334709106) {
                            set_output2(var324, 1.0, 0.0);
                        } else {
                            set_output2(var324, 0.19762258543833583, 0.8023774145616642);
                        }
                    } else {
                        set_output2(var324, 1.0, 0.0);
                    }
                } else {
                    if (input[8] <= 0.9609640967752109) {
                        set_output2(var324, 0.775840597758406, 0.224159402241594);
                    } else {
                        set_output2(var324, 1.0, 0.0);
                    }
                }
            }
        }
    } else {
        set_output2(var324, 0.0, 1.0);
    }
    add_vectors(var173, var324, 2, var172);
    add_vectors(var172, (double[]){0.9932428561395988, 0.0067571438604011585}, 2, var171);
    double var325[2];
    if (input[0] <= 29.22444433360488) {
        if (input[0] <= 20.665295650844598) {
            set_output2(var325, 0.7603833865814698, 0.23961661341853033);
        } else {
            set_output2(var325, 0.9864325016959373, 0.013567498304062715);
        }
    } else {
        if (input[9] <= 4.075283974713441) {
            if (input[9] <= 1.4620885422180314) {
                set_output2(var325, 0.9947256379414166, 0.005274362058583453);
            } else {
                if (input[5] <= 1.9131058141073707) {
                    if (input[7] <= 15.69352057342965) {
                        set_output2(var325, 1.0, 0.0);
                    } else {
                        if (input[8] <= 0.7979872860102568) {
                            set_output2(var325, 0.4249201277955272, 0.5750798722044729);
                        } else {
                            set_output2(var325, 0.8699421965317918, 0.13005780346820806);
                        }
                    }
                } else {
                    if (input[9] <= 1.9864728546226502) {
                        set_output2(var325, 0.7567567567567568, 0.24324324324324326);
                    } else {
                        set_output2(var325, 1.0, 0.0);
                    }
                }
            }
        } else {
            set_output2(var325, 0.29687500000000006, 0.703125);
        }
    }
    add_vectors(var171, var325, 2, var170);
    add_vectors(var170, (double[]){0.9918628036234868, 0.008137196376513209}, 2, var169);
    double var326[2];
    if (input[9] <= 2.0608864501402184) {
        set_output2(var326, 0.9931877935507852, 0.006812206449214762);
    } else {
        if (input[5] <= 0.7710763096857729) {
            if (input[1] <= 7.010871352767057) {
                set_output2(var326, 0.28665785997357995, 0.71334214002642);
            } else {
                set_output2(var326, 0.7575213291423439, 0.242478670857656);
            }
        } else {
            set_output2(var326, 0.9032258064516129, 0.09677419354838708);
        }
    }
    add_vectors(var169, var326, 2, var168);
    add_vectors(var168, (double[]){0.9925231367379832, 0.007476863262016786}, 2, var167);
    double var327[2];
    if (input[9] <= 11.304376791838733) {
        set_output2(var327, 0.9908731280878961, 0.009126871912103932);
    } else {
        set_output2(var327, 0.17177914110429449, 0.8282208588957055);
    }
    add_vectors(var167, var327, 2, var166);
    double var328[2];
    if (input[6] <= 10.828088050274783) {
        if (input[9] <= 2.4951128661836353) {
            set_output2(var328, 0.989280971213636, 0.010719028786364026);
        } else {
            if (input[9] <= 3.432726547465141) {
                set_output2(var328, 0.9256198347107438, 0.0743801652892562);
            } else {
                set_output2(var328, 0.3300248138957817, 0.6699751861042184);
            }
        }
    } else {
        set_output2(var328, 0.9956885676783097, 0.004311432321690354);
    }
    add_vectors(var166, var328, 2, var165);
    add_vectors(var165, (double[]){0.991963253163788, 0.008036746836212055}, 2, var164);
    add_vectors(var164, (double[]){0.9917335212804278, 0.008266478719572217}, 2, var163);
    add_vectors(var163, (double[]){0.9919750854774988, 0.008024914522501165}, 2, var162);
    add_vectors(var162, (double[]){0.9917500365504712, 0.008249963449528821}, 2, var161);
    add_vectors(var161, (double[]){0.9917741807847642, 0.008225819215235812}, 2, var160);
    add_vectors(var160, (double[]){0.9934345668319413, 0.006565433168058723}, 2, var159);
    double var329[2];
    if (input[0] <= 20.378800303495943) {
        set_output2(var329, 0.8568095036062792, 0.14319049639372078);
    } else {
        if (input[0] <= 33.41203593848574) {
            if (input[8] <= 1.633152317688776) {
                set_output2(var329, 0.9901433397351843, 0.009856660264815633);
            } else {
                if (input[0] <= 31.975675005233843) {
                    if (input[3] <= -55.242058742109975) {
                        set_output2(var329, 0.7272727272727273, 0.2727272727272727);
                    } else {
                        set_output2(var329, 1.0, 0.0);
                    }
                } else {
                    set_output2(var329, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var329, 0.9943998232833124, 0.0056001767166875115);
        }
    }
    add_vectors(var159, var329, 2, var158);
    add_vectors(var158, (double[]){0.9915047242589652, 0.008495275741034867}, 2, var157);
    add_vectors(var157, (double[]){0.9923696871478822, 0.007630312852117838}, 2, var156);
    add_vectors(var156, (double[]){0.9914074543244491, 0.008592545675550851}, 2, var155);
    double var330[2];
    if (input[4] <= 13.73525209970154) {
        if (input[4] <= 5.7646230048875635) {
            if (input[0] <= 18.443493212067878) {
                set_output2(var330, 0.8200719712115154, 0.1799280287884846);
            } else {
                if (input[9] <= 3.273742467443992) {
                    if (input[8] <= 1.962649775562092) {
                        if (input[9] <= 0.694191608513091) {
                            set_output2(var330, 0.992536308068708, 0.007463691931292031);
                        } else {
                            set_output2(var330, 0.9511660462945881, 0.048833953705411895);
                        }
                    } else {
                        set_output2(var330, 0.674502712477396, 0.325497287522604);
                    }
                } else {
                    if (input[1] <= 5.513441809251921) {
                        set_output2(var330, 0.0, 1.0);
                    } else {
                        set_output2(var330, 0.7536496350364963, 0.2463503649635036);
                    }
                }
            }
        } else {
            set_output2(var330, 0.9975478509638308, 0.0024521490361691977);
        }
    } else {
        set_output2(var330, 0.9982516973105277, 0.0017483026894723026);
    }
    add_vectors(var155, var330, 2, var154);
    double var331[2];
    if (input[0] <= 1.578035680771204) {
        set_output2(var331, 0.812186978297162, 0.18781302170283803);
    } else {
        if (input[9] <= 7.288126912973053) {
            if (input[9] <= 4.687017849762577) {
                if (input[1] <= 9.188034348109168) {
                    if (input[9] <= 1.2236177344318813) {
                        set_output2(var331, 0.9974589074534095, 0.00254109254659055);
                    } else {
                        if (input[1] <= 6.531787881004931) {
                            set_output2(var331, 0.6414342629482072, 0.3585657370517928);
                        } else {
                            set_output2(var331, 0.8611111111111112, 0.1388888888888889);
                        }
                    }
                } else {
                    set_output2(var331, 0.9896821201332408, 0.010317879866759241);
                }
            } else {
                set_output2(var331, 0.3300248138957817, 0.6699751861042184);
            }
        } else {
            set_output2(var331, 0.0, 1.0);
        }
    }
    add_vectors(var154, var331, 2, var153);
    double var332[2];
    if (input[2] <= -6.940367355570153) {
        if (input[9] <= 3.6016163003129558) {
            set_output2(var332, 0.9948393769094956, 0.005160623090504313);
        } else {
            set_output2(var332, 0.689119170984456, 0.31088082901554404);
        }
    } else {
        if (input[1] <= 8.52883604689406) {
            if (input[6] <= 36.452744458663766) {
                if (input[8] <= 1.330084800435206) {
                    if (input[1] <= 7.927926452068513) {
                        if (input[8] <= 0.7022306972720146) {
                            set_output2(var332, 1.0, 0.0);
                        } else {
                            set_output2(var332, 0.4249201277955272, 0.5750798722044729);
                        }
                    } else {
                        set_output2(var332, 1.0, 0.0);
                    }
                } else {
                    set_output2(var332, 0.7112299465240641, 0.2887700534759358);
                }
            } else {
                set_output2(var332, 1.0, 0.0);
            }
        } else {
            set_output2(var332, 0.9889992773404412, 0.011000722659558761);
        }
    }
    add_vectors(var153, var332, 2, var152);
    double var333[2];
    if (input[9] <= 2.405700926857861) {
        set_output2(var333, 0.9933623547883412, 0.0066376452116587645);
    } else {
        if (input[6] <= 2.1380602803650937) {
            set_output2(var333, 0.34978928356411804, 0.650210716435882);
        } else {
            if (input[1] <= 10.121094686492382) {
                set_output2(var333, 0.7354886113152095, 0.2645113886847906);
            } else {
                set_output2(var333, 1.0, 0.0);
            }
        }
    }
    add_vectors(var152, var333, 2, var151);
    add_vectors(var151, (double[]){0.9906896429077049, 0.009310357092295055}, 2, var150);
    add_vectors(var150, (double[]){0.9934340346293878, 0.00656596537061211}, 2, var149);
    add_vectors(var149, (double[]){0.9919032337238852, 0.008096766276114742}, 2, var148);
    add_vectors(var148, (double[]){0.991605656108914, 0.008394343891085995}, 2, var147);
    double var334[2];
    if (input[9] <= 2.977216745716155) {
        set_output2(var334, 0.9938406174508415, 0.006159382549158418);
    } else {
        set_output2(var334, 0.660377358490566, 0.33962264150943394);
    }
    add_vectors(var147, var334, 2, var146);
    double var335[2];
    if (input[9] <= 5.6768620995231265) {
        set_output2(var335, 0.9939652230370328, 0.006034776962967162);
    } else {
        set_output2(var335, 0.09395973154362416, 0.9060402684563759);
    }
    add_vectors(var146, var335, 2, var145);
    double var336[2];
    if (input[9] <= 7.17895905588705) {
        if (input[9] <= 2.4868838903776367) {
            if (input[9] <= 1.1488539040894816) {
                set_output2(var336, 0.9944581628472914, 0.005541837152708578);
            } else {
                if (input[9] <= 1.882538991766559) {
                    set_output2(var336, 0.973142345568487, 0.026857654431512976);
                } else {
                    if (input[3] <= -57.547743230887434) {
                        set_output2(var336, 0.4274809160305344, 0.5725190839694657);
                    } else {
                        set_output2(var336, 0.8880597014925373, 0.11194029850746268);
                    }
                }
            }
        } else {
            if (input[3] <= -57.92338005311367) {
                set_output2(var336, 1.0, 0.0);
            } else {
                set_output2(var336, 0.6585735963581184, 0.34142640364188165);
            }
        }
    } else {
        set_output2(var336, 0.3323442136498516, 0.6676557863501483);
    }
    add_vectors(var145, var336, 2, var144);
    add_vectors(var144, (double[]){0.9915699309497557, 0.008430069050244385}, 2, var143);
    double var337[2];
    if (input[6] <= 30.636608428618224) {
        if (input[0] <= 23.029034865517563) {
            set_output2(var337, 0.9194702934860415, 0.08052970651395848);
        } else {
            if (input[2] <= -20.74468254054861) {
                set_output2(var337, 0.9684708355228586, 0.03152916447714136);
            } else {
                if (input[9] <= 4.119090072112675) {
                    set_output2(var337, 0.9911930718832148, 0.008806928116785162);
                } else {
                    set_output2(var337, 0.5964125560538117, 0.40358744394618834);
                }
            }
        }
    } else {
        set_output2(var337, 0.9962430227565479, 0.003756977243452122);
    }
    add_vectors(var143, var337, 2, var142);
    add_vectors(var142, (double[]){0.9926220530644114, 0.007377946935588602}, 2, var141);
    double var338[2];
    if (input[9] <= 2.453813749576832) {
        set_output2(var338, 0.9922886527466481, 0.007711347253351804);
    } else {
        if (input[4] <= 0.3987662329630387) {
            set_output2(var338, 0.8032786885245902, 0.19672131147540983);
        } else {
            if (input[5] <= 2.5800510921391995) {
                if (input[7] <= 15.848214954317008) {
                    set_output2(var338, 0.4962686567164179, 0.503731343283582);
                } else {
                    if (input[9] <= 2.6384354307794697) {
                        set_output2(var338, 0.5544554455445545, 0.44554455445544555);
                    } else {
                        set_output2(var338, 0.15094339622641512, 0.8490566037735848);
                    }
                }
            } else {
                set_output2(var338, 0.8615384615384616, 0.13846153846153847);
            }
        }
    }
    add_vectors(var141, var338, 2, var140);
    add_vectors(var140, (double[]){0.9903867052313045, 0.009613294768695527}, 2, var139);
    double var339[2];
    if (input[3] <= -48.80683981579581) {
        if (input[9] <= 0.6912962332710446) {
            set_output2(var339, 0.993866097359726, 0.006133902640274016);
        } else {
            set_output2(var339, 0.9423113902954939, 0.057688609704506115);
        }
    } else {
        if (input[0] <= 31.67206132787185) {
            if (input[3] <= -23.539478936230463) {
                if (input[8] <= 0.7617401795200244) {
                    if (input[7] <= 15.585443341840328) {
                        set_output2(var339, 1.0, 0.0);
                    } else {
                        if (input[1] <= 12.431348651466179) {
                            set_output2(var339, 0.9469402193137602, 0.05305978068623984);
                        } else {
                            set_output2(var339, 0.5964125560538117, 0.4035874439461883);
                        }
                    }
                } else {
                    set_output2(var339, 0.9951015076470908, 0.004898492352909162);
                }
            } else {
                set_output2(var339, 0.7964261479303325, 0.2035738520696675);
            }
        } else {
            set_output2(var339, 0.9886216946355616, 0.011378305364438474);
        }
    }
    add_vectors(var139, var339, 2, var138);
    add_vectors(var138, (double[]){0.9915195855694757, 0.008480414430524285}, 2, var137);
    double var340[2];
    if (input[0] <= 22.36505710461784) {
        set_output2(var340, 0.8638838475499093, 0.13611615245009073);
    } else {
        if (input[9] <= 2.7037437766538877) {
            if (input[9] <= 1.2197261672419886) {
                set_output2(var340, 0.9939151676452912, 0.006084832354708848);
            } else {
                if (input[3] <= -53.02458680674987) {
                    if (input[6] <= 24.29685265046575) {
                        set_output2(var340, 0.5764705882352942, 0.4235294117647059);
                    } else {
                        set_output2(var340, 1.0, 0.0);
                    }
                } else {
                    if (input[2] <= -7.48193413151431) {
                        set_output2(var340, 0.42748091603053434, 0.5725190839694656);
                    } else {
                        set_output2(var340, 0.9604395604395605, 0.03956043956043956);
                    }
                }
            }
        } else {
            if (input[3] <= -49.79278033406297) {
                if (input[1] <= 5.231755271782283) {
                    set_output2(var340, 0.0, 1.0);
                } else {
                    set_output2(var340, 0.9447852760736196, 0.05521472392638036);
                }
            } else {
                set_output2(var340, 0.22813036020583194, 0.7718696397941681);
            }
        }
    }
    add_vectors(var137, var340, 2, var136);
    double var341[2];
    if (input[0] <= 28.64938556903917) {
        set_output2(var341, 0.9597642482992944, 0.04023575170070563);
    } else {
        set_output2(var341, 0.9932024201781902, 0.006797579821809831);
    }
    add_vectors(var136, var341, 2, var135);
    double var342[2];
    if (input[9] <= 8.358839903193918) {
        if (input[0] <= 24.750783251619435) {
            set_output2(var342, 0.9220103986135182, 0.07798960138648178);
        } else {
            if (input[9] <= 1.6186648472371246) {
                set_output2(var342, 0.9938287689079665, 0.006171231092033512);
            } else {
                if (input[7] <= 15.910582009011474) {
                    set_output2(var342, 1.0, 0.0);
                } else {
                    if (input[4] <= 8.336853150487244) {
                        if (input[5] <= 2.7172874137421674) {
                            set_output2(var342, 0.6158896913985554, 0.38411030860144457);
                        } else {
                            set_output2(var342, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var342, 1.0, 0.0);
                    }
                }
            }
        }
    } else {
        set_output2(var342, 0.5544554455445545, 0.44554455445544555);
    }
    add_vectors(var135, var342, 2, var134);
    double var343[2];
    if (input[2] <= -1.2447082013275335) {
        if (input[6] <= 33.22560695615858) {
            if (input[9] <= 4.520304641178662) {
                if (input[1] <= 3.8935791104514283) {
                    if (input[5] <= 0.0024474868462674303) {
                        set_output2(var343, 0.5609756097560976, 0.43902439024390244);
                    } else {
                        set_output2(var343, 0.9761999206664023, 0.023800079333597776);
                    }
                } else {
                    if (input[9] <= 2.07401188456183) {
                        set_output2(var343, 0.991792377148213, 0.008207622851787031);
                    } else {
                        if (input[5] <= 1.121796305703249) {
                            set_output2(var343, 0.49845201238390097, 0.5015479876160991);
                        } else {
                            set_output2(var343, 1.0, 0.0);
                        }
                    }
                }
            } else {
                set_output2(var343, 0.4249201277955272, 0.5750798722044729);
            }
        } else {
            set_output2(var343, 0.9983721406952768, 0.0016278593047232039);
        }
    } else {
        set_output2(var343, 0.9575097570187586, 0.04249024298124135);
    }
    add_vectors(var134, var343, 2, var133);
    double var344[2];
    if (input[0] <= 29.518350030239727) {
        if (input[9] <= 1.0400995945205909) {
            set_output2(var344, 0.9872982718593213, 0.012701728140678693);
        } else {
            set_output2(var344, 0.7497219132369299, 0.25027808676307006);
        }
    } else {
        set_output2(var344, 0.9916426490846164, 0.008357350915383588);
    }
    add_vectors(var133, var344, 2, var132);
    add_vectors(var132, (double[]){0.9926335261088883, 0.007366473891111636}, 2, var131);
    add_vectors(var131, (double[]){0.9917195906333316, 0.008280409366668432}, 2, var130);
    add_vectors(var130, (double[]){0.9905124526665223, 0.009487547333477649}, 2, var129);
    add_vectors(var129, (double[]){0.9925523208203778, 0.007447679179622229}, 2, var128);
    add_vectors(var128, (double[]){0.9914731576406124, 0.008526842359387606}, 2, var127);
    double var345[2];
    if (input[0] <= 9.434395116838017) {
        set_output2(var345, 0.9232212932946596, 0.07677870670534037);
    } else {
        if (input[9] <= 5.341263759624259) {
            set_output2(var345, 0.9920662390168447, 0.0079337609831553);
        } else {
            set_output2(var345, 0.0, 1.0);
        }
    }
    add_vectors(var127, var345, 2, var126);
    double var346[2];
    if (input[6] <= 25.147384323714014) {
        if (input[0] <= 24.765332842489826) {
            if (input[4] <= 1.366777533700217) {
                set_output2(var346, 1.0, 0.0);
            } else {
                set_output2(var346, 0.29319371727748694, 0.706806282722513);
            }
        } else {
            set_output2(var346, 0.9895115458158689, 0.010488454184131163);
        }
    } else {
        set_output2(var346, 0.9985308760875597, 0.001469123912440215);
    }
    add_vectors(var126, var346, 2, var125);
    add_vectors(var125, (double[]){0.9909967357932797, 0.009003264206720298}, 2, var124);
    double var347[2];
    if (input[9] <= 8.541359500280283) {
        if (input[9] <= 3.3251346871505376) {
            if (input[4] <= 4.033575178097574) {
                set_output2(var347, 0.9902609569929176, 0.009739043007082341);
            } else {
                if (input[9] <= 2.133368329010516) {
                    set_output2(var347, 0.9973002321800325, 0.0026997678199674915);
                } else {
                    set_output2(var347, 0.7938144329896908, 0.20618556701030924);
                }
            }
        } else {
            if (input[9] <= 4.503546035100742) {
                set_output2(var347, 1.0, 0.0);
            } else {
                set_output2(var347, 0.22813036020583194, 0.7718696397941681);
            }
        }
    } else {
        set_output2(var347, 0.0, 1.0);
    }
    add_vectors(var124, var347, 2, var123);
    double var348[2];
    if (input[9] <= 8.420664855940577) {
        set_output2(var348, 0.9935650247331389, 0.006434975266861116);
    } else {
        set_output2(var348, 0.4534412955465587, 0.5465587044534412);
    }
    add_vectors(var123, var348, 2, var122);
    double var349[2];
    if (input[4] <= 16.23347879413541) {
        if (input[9] <= 4.256839656390641) {
            set_output2(var349, 0.9912922654961864, 0.008707734503813597);
        } else {
            set_output2(var349, 0.4267515923566879, 0.5732484076433121);
        }
    } else {
        set_output2(var349, 1.0, 0.0);
    }
    add_vectors(var122, var349, 2, var121);
    double var350[2];
    if (input[3] <= -57.163878018899496) {
        if (input[9] <= 0.5760633622786033) {
            set_output2(var350, 0.991681462513325, 0.008318537486674975);
        } else {
            if (input[3] <= -63.80025754890205) {
                set_output2(var350, 0.9511930585683297, 0.04880694143167029);
            } else {
                if (input[8] <= 1.235022730256937) {
                    if (input[3] <= -59.0167742978038) {
                        if (input[0] <= 30.803990153109005) {
                            set_output2(var350, 0.6559633027522936, 0.3440366972477064);
                        } else {
                            set_output2(var350, 0.9621371476651241, 0.03786285233487589);
                        }
                    } else {
                        set_output2(var350, 1.0, 0.0);
                    }
                } else {
                    set_output2(var350, 0.4534412955465587, 0.5465587044534412);
                }
            }
        }
    } else {
        set_output2(var350, 0.9944098079044361, 0.005590192095563872);
    }
    add_vectors(var121, var350, 2, var120);
    double var351[2];
    if (input[3] <= -27.056905880057116) {
        if (input[9] <= 4.080469663031876) {
            if (input[9] <= 2.794810803254321) {
                if (input[9] <= 0.5048725565689769) {
                    set_output2(var351, 0.9945761283618179, 0.005423871638182116);
                } else {
                    if (input[9] <= 1.3201667857364994) {
                        set_output2(var351, 0.9912166748716659, 0.008783325128334133);
                    } else {
                        set_output2(var351, 0.8717217787913341, 0.12827822120866592);
                    }
                }
            } else {
                if (input[5] <= 1.6791947332878039) {
                    set_output2(var351, 0.22813036020583194, 0.7718696397941681);
                } else {
                    set_output2(var351, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var351, 0.15592028135990624, 0.8440797186400938);
        }
    } else {
        set_output2(var351, 0.9340852497436648, 0.06591475025633514);
    }
    add_vectors(var120, var351, 2, var119);
    add_vectors(var119, (double[]){0.9919703321733381, 0.008029667826661872}, 2, var118);
    add_vectors(var118, (double[]){0.9928184198164431, 0.007181580183556832}, 2, var117);
    double var352[2];
    if (input[4] <= 4.812844576885902) {
        set_output2(var352, 0.9910727958680793, 0.008927204131920712);
    } else {
        if (input[0] <= 26.86195507873052) {
            set_output2(var352, 0.8238747553816047, 0.17612524461839532);
        } else {
            set_output2(var352, 0.9980950096307847, 0.0019049903692153589);
        }
    }
    add_vectors(var117, var352, 2, var116);
    double var353[2];
    if (input[0] <= 15.410204397478134) {
        set_output2(var353, 0.7873345935727789, 0.21266540642722115);
    } else {
        if (input[9] <= 5.885994176465827) {
            if (input[9] <= 2.913567711933515) {
                if (input[9] <= 1.50465803827447) {
                    set_output2(var353, 0.9950994180935508, 0.004900581906449137);
                } else {
                    if (input[6] <= 22.293522493299022) {
                        if (input[7] <= 15.758143156730275) {
                            set_output2(var353, 1.0, 0.0);
                        } else {
                            set_output2(var353, 0.5635305528612997, 0.43646944713870023);
                        }
                    } else {
                        if (input[9] <= 2.1798195428151783) {
                            set_output2(var353, 1.0, 0.0);
                        } else {
                            set_output2(var353, 0.7313432835820896, 0.26865671641791045);
                        }
                    }
                }
            } else {
                set_output2(var353, 0.9210526315789473, 0.07894736842105265);
            }
        } else {
            set_output2(var353, 0.0, 1.0);
        }
    }
    add_vectors(var116, var353, 2, var115);
    double var354[2];
    if (input[9] <= 3.5286927495751033) {
        set_output2(var354, 0.9911785029919579, 0.008821497008042179);
    } else {
        set_output2(var354, 0.21874999999999997, 0.78125);
    }
    add_vectors(var115, var354, 2, var114);
    add_vectors(var114, (double[]){0.9906479816219415, 0.009352018378058557}, 2, var113);
    add_vectors(var113, (double[]){0.9930287615034108, 0.006971238496589217}, 2, var112);
    double var355[2];
    if (input[9] <= 0.3176163301419543) {
        set_output2(var355, 0.9934968070125295, 0.006503192987470437);
    } else {
        if (input[7] <= 17.559961138892962) {
            set_output2(var355, 0.9856185823463562, 0.014381417653643657);
        } else {
            if (input[3] <= -48.377294454563895) {
                set_output2(var355, 0.9673794853207685, 0.032620514679231605);
            } else {
                set_output2(var355, 0.6520618556701031, 0.34793814432989695);
            }
        }
    }
    add_vectors(var112, var355, 2, var111);
    double var356[2];
    if (input[2] <= -20.772661407106373) {
        if (input[7] <= 15.264789428335245) {
            set_output2(var356, 0.7647058823529411, 0.23529411764705882);
        } else {
            set_output2(var356, 0.9729567307692307, 0.027043269230769232);
        }
    } else {
        set_output2(var356, 0.9929133391875655, 0.007086660812434482);
    }
    add_vectors(var111, var356, 2, var110);
    add_vectors(var110, (double[]){0.9935579254488038, 0.006442074551196201}, 2, var109);
    double var357[2];
    if (input[9] <= 0.629211857742263) {
        set_output2(var357, 0.9945856872088886, 0.005414312791111335);
    } else {
        if (input[1] <= 4.135424054729472) {
            set_output2(var357, 0.4821634062140392, 0.5178365937859609);
        } else {
            if (input[9] <= 7.826392484344006) {
                if (input[9] <= 0.7431231532350875) {
                    set_output2(var357, 0.9801137932939292, 0.01988620670607082);
                } else {
                    if (input[3] <= -35.93418682815502) {
                        set_output2(var357, 0.9450474459633218, 0.05495255403667813);
                    } else {
                        set_output2(var357, 0.0, 1.0);
                    }
                }
            } else {
                set_output2(var357, 0.48275862068965525, 0.5172413793103449);
            }
        }
    }
    add_vectors(var109, var357, 2, var108);
    double var358[2];
    if (input[1] <= 3.815381136539345) {
        if (input[3] <= -60.01319619029683) {
            set_output2(var358, 0.9678226671433678, 0.03217733285663211);
        } else {
            if (input[6] <= 0.790518324428227) {
                set_output2(var358, 0.7508650519031143, 0.24913494809688583);
            } else {
                set_output2(var358, 1.0, 0.0);
            }
        }
    } else {
        if (input[3] <= -26.320707853125406) {
            if (input[9] <= 1.9028211970816729) {
                set_output2(var358, 0.9932950159855334, 0.006704984014466597);
            } else {
                set_output2(var358, 0.8579040852575488, 0.14209591474245115);
            }
        } else {
            set_output2(var358, 0.8375451263537906, 0.16245487364620936);
        }
    }
    add_vectors(var108, var358, 2, var107);
    add_vectors(var107, (double[]){0.9930239015247926, 0.00697609847520747}, 2, var106);
    add_vectors(var106, (double[]){0.9925101511724274, 0.00748984882757263}, 2, var105);
    double var359[2];
    if (input[0] <= 30.1051676583054) {
        if (input[9] <= 3.5222438529703037) {
            if (input[3] <= -45.32551021332212) {
                set_output2(var359, 0.9877782304359098, 0.012221769564090205);
            } else {
                if (input[3] <= -38.18124642929715) {
                    set_output2(var359, 1.0, 0.0);
                } else {
                    set_output2(var359, 0.7475219749392182, 0.25247802506078176);
                }
            }
        } else {
            set_output2(var359, 0.497907949790795, 0.502092050209205);
        }
    } else {
        set_output2(var359, 0.9916111053430269, 0.008388894656973065);
    }
    add_vectors(var105, var359, 2, var104);
    double var360[2];
    if (input[1] <= 3.729015629784131) {
        if (input[1] <= 3.281955398109041) {
            set_output2(var360, 1.0, 0.0);
        } else {
            if (input[7] <= 16.43187760355962) {
                set_output2(var360, 1.0, 0.0);
            } else {
                set_output2(var360, 0.7692307692307693, 0.23076923076923078);
            }
        }
    } else {
        set_output2(var360, 0.9922924269634134, 0.007707573036586565);
    }
    add_vectors(var104, var360, 2, var103);
    add_vectors(var103, (double[]){0.9908447822274191, 0.009155217772580883}, 2, var102);
    add_vectors(var102, (double[]){0.9928079816810841, 0.007192018318915924}, 2, var101);
    add_vectors(var101, (double[]){0.9927853113120744, 0.007214688687925627}, 2, var100);
    double var361[2];
    if (input[9] <= 7.811577435495135) {
        set_output2(var361, 0.9912342968108842, 0.00876570318911579);
    } else {
        set_output2(var361, 0.0, 1.0);
    }
    add_vectors(var100, var361, 2, var99);
    double var362[2];
    if (input[0] <= 29.024721568423992) {
        if (input[9] <= 3.619226826643211) {
            set_output2(var362, 0.9802167366408019, 0.019783263359198118);
        } else {
            set_output2(var362, 0.6447368421052632, 0.35526315789473684);
        }
    } else {
        if (input[9] <= 4.856814926128609) {
            if (input[9] <= 3.059851503418923) {
                if (input[9] <= 1.1905537443847585) {
                    set_output2(var362, 0.9942517427168475, 0.005748257283152492);
                } else {
                    if (input[5] <= 0.5668500189231791) {
                        if (input[2] <= -6.0451769139354035) {
                            set_output2(var362, 0.7142857142857143, 0.28571428571428575);
                        } else {
                            set_output2(var362, 0.9158878504672897, 0.08411214953271028);
                        }
                    } else {
                        set_output2(var362, 0.9575471698113207, 0.04245283018867924);
                    }
                }
            } else {
                set_output2(var362, 0.6853146853146853, 0.3146853146853147);
            }
        } else {
            set_output2(var362, 0.22813036020583194, 0.7718696397941681);
        }
    }
    add_vectors(var99, var362, 2, var98);
    double var363[2];
    if (input[9] <= 1.9242156466093705) {
        set_output2(var363, 0.9928713801287625, 0.0071286198712374495);
    } else {
        if (input[3] <= -63.50640931035745) {
            set_output2(var363, 1.0, 0.0);
        } else {
            if (input[3] <= -45.506741350705056) {
                set_output2(var363, 0.788235294117647, 0.21176470588235294);
            } else {
                set_output2(var363, 0.0, 1.0);
            }
        }
    }
    add_vectors(var98, var363, 2, var97);
    double var364[2];
    if (input[1] <= 8.250164720601758) {
        if (input[3] <= -27.039011275755648) {
            set_output2(var364, 0.993939831710963, 0.0060601682890369735);
        } else {
            set_output2(var364, 0.8047722342733189, 0.19522776572668113);
        }
    } else {
        if (input[0] <= 25.286635567550064) {
            set_output2(var364, 0.4, 0.6);
        } else {
            set_output2(var364, 0.988856433721107, 0.011143566278893053);
        }
    }
    add_vectors(var97, var364, 2, var96);
    add_vectors(var96, (double[]){0.9922874176050697, 0.007712582394930339}, 2, var95);
    add_vectors(var95, (double[]){0.9927156850955982, 0.007284314904401866}, 2, var94);
    double var365[2];
    if (input[2] <= -0.3278682748527615) {
        if (input[9] <= 3.8912415639243614) {
            set_output2(var365, 0.9923129196303463, 0.0076870803696537475);
        } else {
            set_output2(var365, 0.3300248138957817, 0.6699751861042184);
        }
    } else {
        if (input[1] <= 0.9373483419814312) {
            set_output2(var365, 1.0, 0.0);
        } else {
            set_output2(var365, 0.199288256227758, 0.800711743772242);
        }
    }
    add_vectors(var94, var365, 2, var93);
    add_vectors(var93, (double[]){0.9925369043171246, 0.007463095682875364}, 2, var92);
    double var366[2];
    if (input[9] <= 8.694337091266334) {
        if (input[0] <= 26.152907479579298) {
            if (input[4] <= 2.0561352426730037) {
                set_output2(var366, 1.0, 0.0);
            } else {
                set_output2(var366, 0.7804878048780488, 0.21951219512195122);
            }
        } else {
            if (input[9] <= 2.012233047766783) {
                set_output2(var366, 0.993055740657867, 0.0069442593421330575);
            } else {
                if (input[1] <= 10.120857117537426) {
                    set_output2(var366, 0.5212765957446809, 0.4787234042553192);
                } else {
                    if (input[3] <= -47.34496727679449) {
                        set_output2(var366, 0.9447852760736196, 0.05521472392638037);
                    } else {
                        set_output2(var366, 0.5764705882352941, 0.4235294117647059);
                    }
                }
            }
        }
    } else {
        set_output2(var366, 0.23728813559322035, 0.7627118644067796);
    }
    add_vectors(var92, var366, 2, var91);
    add_vectors(var91, (double[]){0.9912093922759863, 0.008790607724013761}, 2, var90);
    add_vectors(var90, (double[]){0.9915649387480334, 0.00843506125196653}, 2, var89);
    add_vectors(var89, (double[]){0.9915777278628182, 0.008422272137181776}, 2, var88);
    add_vectors(var88, (double[]){0.9912854975980874, 0.00871450240191264}, 2, var87);
    double var367[2];
    if (input[9] <= 5.726927265397705) {
        set_output2(var367, 0.9930266267214577, 0.006973373278542287);
    } else {
        set_output2(var367, 0.3323442136498516, 0.6676557863501483);
    }
    add_vectors(var87, var367, 2, var86);
    double var368[2];
    if (input[9] <= 5.781725529014005) {
        if (input[4] <= 10.321584452475738) {
            if (input[9] <= 4.0018852637947155) {
                if (input[8] <= 1.8069394737643085) {
                    if (input[9] <= 2.706358124012803) {
                        set_output2(var368, 0.9926442190210799, 0.007355780978920047);
                    } else {
                        if (input[5] <= 0.7701961482361234) {
                            set_output2(var368, 0.5964125560538117, 0.40358744394618834);
                        } else {
                            set_output2(var368, 1.0, 0.0);
                        }
                    }
                } else {
                    set_output2(var368, 0.9359278595158994, 0.0640721404841006);
                }
            } else {
                set_output2(var368, 0.4249201277955272, 0.5750798722044729);
            }
        } else {
            set_output2(var368, 0.9986514631624687, 0.0013485368375312813);
        }
    } else {
        if (input[4] <= 2.0443377510704783) {
            set_output2(var368, 0.3835616438356165, 0.6164383561643836);
        } else {
            set_output2(var368, 0.0, 1.0);
        }
    }
    add_vectors(var86, var368, 2, var85);
    add_vectors(var85, (double[]){0.992343351616835, 0.00765664838316509}, 2, var84);
    add_vectors(var84, (double[]){0.9927416076855086, 0.007258392314491418}, 2, var83);
    add_vectors(var83, (double[]){0.9931229623592177, 0.006877037640782279}, 2, var82);
    double var369[2];
    if (input[8] <= 1.2566485744652856) {
        if (input[6] <= 89.46776495579934) {
            set_output2(var369, 0.9938443829026123, 0.006155617097387709);
        } else {
            if (input[8] <= 0.8946087020886196) {
                set_output2(var369, 0.9867880211391662, 0.013211978860833822);
            } else {
                set_output2(var369, 0.7530864197530864, 0.24691358024691357);
            }
        }
    } else {
        set_output2(var369, 0.984587382203564, 0.015412617796436018);
    }
    add_vectors(var82, var369, 2, var81);
    double var370[2];
    if (input[0] <= 10.95843428192596) {
        set_output2(var370, 0.7389791183294664, 0.2610208816705336);
    } else {
        if (input[1] <= 8.045072830187333) {
            set_output2(var370, 0.9961246523780061, 0.003875347621993956);
        } else {
            if (input[0] <= 25.797406031968197) {
                set_output2(var370, 0.4249201277955272, 0.5750798722044729);
            } else {
                set_output2(var370, 0.9878624409979772, 0.012137559002022869);
            }
        }
    }
    add_vectors(var81, var370, 2, var80);
    double var371[2];
    if (input[0] <= 22.582188771022903) {
        set_output2(var371, 0.7754491017964072, 0.2245508982035928);
    } else {
        if (input[9] <= 0.5073492907635008) {
            set_output2(var371, 0.9952868069609672, 0.00471319303903276);
        } else {
            if (input[2] <= -15.098716174555122) {
                set_output2(var371, 0.9139702719495292, 0.08602972805047077);
            } else {
                if (input[0] <= 29.52140136311771) {
                    set_output2(var371, 0.9506037321624589, 0.04939626783754117);
                } else {
                    if (input[3] <= -43.97983591396808) {
                        if (input[9] <= 4.05753015726447) {
                            set_output2(var371, 0.9828103683492496, 0.017189631650750332);
                        } else {
                            set_output2(var371, 0.4249201277955272, 0.5750798722044729);
                        }
                    } else {
                        if (input[6] <= 25.565452834565814) {
                            set_output2(var371, 0.9773156899810964, 0.022684310018903593);
                        } else {
                            set_output2(var371, 0.7804878048780488, 0.21951219512195122);
                        }
                    }
                }
            }
        }
    }
    add_vectors(var80, var371, 2, var79);
    add_vectors(var79, (double[]){0.9921134559396895, 0.007886544060310476}, 2, var78);
    add_vectors(var78, (double[]){0.9918298775975476, 0.008170122402452419}, 2, var77);
    double var372[2];
    if (input[6] <= 17.215463019853647) {
        if (input[3] <= -36.21035779827666) {
            set_output2(var372, 0.9891130174837981, 0.010886982516201894);
        } else {
            if (input[3] <= -22.64049546980906) {
                if (input[2] <= -4.137515575687073) {
                    set_output2(var372, 1.0, 0.0);
                } else {
                    if (input[2] <= -2.065679693249237) {
                        if (input[1] <= 10.930201336563208) {
                            set_output2(var372, 0.4962686567164179, 0.503731343283582);
                        } else {
                            set_output2(var372, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var372, 1.0, 0.0);
                    }
                }
            } else {
                set_output2(var372, 0.8409893992932862, 0.15901060070671375);
            }
        }
    } else {
        set_output2(var372, 0.9978928473875209, 0.0021071526124790277);
    }
    add_vectors(var77, var372, 2, var76);
    double var373[2];
    if (input[9] <= 1.2502581236186707) {
        set_output2(var373, 0.9934510178385825, 0.006548982161417553);
    } else {
        if (input[7] <= 16.172487041685862) {
            if (input[1] <= 5.089511594919641) {
                set_output2(var373, 0.906054279749478, 0.0939457202505219);
            } else {
                if (input[9] <= 8.187216612086441) {
                    if (input[3] <= -51.273228549549316) {
                        set_output2(var373, 0.9315589353612167, 0.06844106463878329);
                    } else {
                        if (input[0] <= 31.2724645686978) {
                            set_output2(var373, 0.22813036020583194, 0.7718696397941681);
                        } else {
                            set_output2(var373, 0.7249809014514897, 0.2750190985485103);
                        }
                    }
                } else {
                    set_output2(var373, 0.0, 1.0);
                }
            }
        } else {
            if (input[4] <= 4.477832044096308) {
                if (input[5] <= 0.7598153680314189) {
                    if (input[4] <= 0.8834594939864258) {
                        set_output2(var373, 0.23728813559322035, 0.7627118644067796);
                    } else {
                        if (input[9] <= 2.5884676245674374) {
                            set_output2(var373, 1.0, 0.0);
                        } else {
                            set_output2(var373, 0.0, 1.0);
                        }
                    }
                } else {
                    set_output2(var373, 0.8818897637795275, 0.11811023622047244);
                }
            } else {
                set_output2(var373, 0.9687282835302293, 0.03127171646977067);
            }
        }
    }
    add_vectors(var76, var373, 2, var75);
    add_vectors(var75, (double[]){0.9915892370269824, 0.008410762973017608}, 2, var74);
    add_vectors(var74, (double[]){0.9915844297925214, 0.008415570207478669}, 2, var73);
    double var374[2];
    if (input[0] <= 27.67489174026122) {
        set_output2(var374, 0.9618525530532086, 0.038147446946791376);
    } else {
        if (input[9] <= 5.974117151857895) {
            if (input[9] <= 1.0228747343956066) {
                if (input[9] <= 0.6892590763754891) {
                    set_output2(var374, 0.9936555395796629, 0.006344460420337149);
                } else {
                    if (input[7] <= 16.58220840578727) {
                        if (input[8] <= 1.2395682624077418) {
                            set_output2(var374, 0.9629574751815083, 0.037042524818491636);
                        } else {
                            set_output2(var374, 0.8382022471910112, 0.16179775280898875);
                        }
                    } else {
                        set_output2(var374, 1.0, 0.0);
                    }
                }
            } else {
                if (input[9] <= 1.0719823492954554) {
                    set_output2(var374, 1.0, 0.0);
                } else {
                    if (input[5] <= 1.407760173539298) {
                        if (input[7] <= 15.989807131001331) {
                            set_output2(var374, 0.7175732217573222, 0.2824267782426778);
                        } else {
                            set_output2(var374, 0.8932046515307334, 0.10679534846926668);
                        }
                    } else {
                        set_output2(var374, 0.9144893111638954, 0.08551068883610452);
                    }
                }
            }
        } else {
            set_output2(var374, 0.0, 1.0);
        }
    }
    add_vectors(var73, var374, 2, var72);
    add_vectors(var72, (double[]){0.990922507428357, 0.009077492571642982}, 2, var71);
    add_vectors(var71, (double[]){0.9914281194385862, 0.008571880561413815}, 2, var70);
    add_vectors(var70, (double[]){0.9917014482198383, 0.00829855178016169}, 2, var69);
    add_vectors(var69, (double[]){0.991716318897792, 0.008283681102207972}, 2, var68);
    add_vectors(var68, (double[]){0.9933709638549504, 0.006629036145049589}, 2, var67);
    add_vectors(var67, (double[]){0.9900116807844163, 0.009988319215583746}, 2, var66);
    double var375[2];
    if (input[4] <= 8.115487894733807) {
        if (input[8] <= 1.8258275355424614) {
            set_output2(var375, 0.9909832022361659, 0.009016797763834116);
        } else {
            if (input[4] <= 2.5258531226513594) {
                set_output2(var375, 0.22813036020583194, 0.7718696397941681);
            } else {
                set_output2(var375, 1.0, 0.0);
            }
        }
    } else {
        set_output2(var375, 0.9979817799579538, 0.0020182200420462543);
    }
    add_vectors(var66, var375, 2, var65);
    double var376[2];
    if (input[9] <= 8.567283830403948) {
        set_output2(var376, 0.9929365603865227, 0.007063439613477383);
    } else {
        set_output2(var376, 0.21663442940038685, 0.7833655705996131);
    }
    add_vectors(var65, var376, 2, var64);
    double var377[2];
    if (input[6] <= 2.8632921180051247) {
        if (input[9] <= 0.617601012912586) {
            set_output2(var377, 0.9901010375055452, 0.009898962494454778);
        } else {
            if (input[9] <= 2.327088097834955) {
                if (input[3] <= -56.990175712079676) {
                    if (input[9] <= 1.372662243493059) {
                        set_output2(var377, 1.0, 0.0);
                    } else {
                        set_output2(var377, 0.689119170984456, 0.31088082901554404);
                    }
                } else {
                    set_output2(var377, 1.0, 0.0);
                }
            } else {
                if (input[2] <= -7.946323000542401) {
                    set_output2(var377, 0.0, 1.0);
                } else {
                    set_output2(var377, 0.8880597014925373, 0.11194029850746268);
                }
            }
        }
    } else {
        set_output2(var377, 0.9950758515822274, 0.004924148417772612);
    }
    add_vectors(var64, var377, 2, var63);
    double var378[2];
    if (input[1] <= 7.62630821818693) {
        if (input[8] <= 1.202619544618993) {
            set_output2(var378, 0.9978878660249951, 0.0021121339750048507);
        } else {
            if (input[2] <= -11.221751349702094) {
                set_output2(var378, 0.994842584209139, 0.00515741579086106);
            } else {
                if (input[9] <= 5.67789158827272) {
                    set_output2(var378, 0.9780621572212066, 0.021937842778793428);
                } else {
                    set_output2(var378, 0.0, 1.0);
                }
            }
        }
    } else {
        if (input[9] <= 3.519476554689528) {
            if (input[9] <= 1.4623708010378602) {
                set_output2(var378, 0.9914464629540327, 0.00855353704596733);
            } else {
                if (input[3] <= -42.88771589314831) {
                    set_output2(var378, 0.9260476581758422, 0.07395234182415779);
                } else {
                    set_output2(var378, 0.22813036020583194, 0.7718696397941681);
                }
            }
        } else {
            set_output2(var378, 0.873772791023843, 0.12622720897615705);
        }
    }
    add_vectors(var63, var378, 2, var62);
    double var379[2];
    if (input[2] <= -10.463793620948042) {
        if (input[9] <= 2.631458546534614) {
            set_output2(var379, 0.9959626337265429, 0.004037366273457106);
        } else {
            set_output2(var379, 0.0, 1.0);
        }
    } else {
        if (input[6] <= 84.2666016352673) {
            set_output2(var379, 0.9893458819434137, 0.010654118056586434);
        } else {
            if (input[1] <= 10.23437692479926) {
                if (input[2] <= -6.845145228833218) {
                    if (input[8] <= 0.4425090535204876) {
                        if (input[7] <= 16.167775473988993) {
                            set_output2(var379, 0.5088676671214188, 0.4911323328785812);
                        } else {
                            set_output2(var379, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var379, 0.878158844765343, 0.12184115523465704);
                    }
                } else {
                    set_output2(var379, 1.0, 0.0);
                }
            } else {
                set_output2(var379, 1.0, 0.0);
            }
        }
    }
    add_vectors(var62, var379, 2, var61);
    double var380[2];
    if (input[2] <= -12.088250857337496) {
        set_output2(var380, 0.9962633584933861, 0.003736641506613853);
    } else {
        if (input[0] <= 19.836966285014636) {
            set_output2(var380, 0.7919235511713933, 0.20807644882860668);
        } else {
            if (input[0] <= 26.50932422278089) {
                if (input[0] <= 25.32261182529959) {
                    set_output2(var380, 0.4, 0.6);
                } else {
                    set_output2(var380, 0.9789424426766495, 0.02105755732335049);
                }
            } else {
                if (input[9] <= 5.53059069957984) {
                    if (input[9] <= 1.888160963448697) {
                        set_output2(var380, 0.9922538239865633, 0.00774617601343679);
                    } else {
                        if (input[4] <= 4.9907309387174035) {
                            set_output2(var380, 0.822921790457452, 0.17707820954254797);
                        } else {
                            set_output2(var380, 1.0, 0.0);
                        }
                    }
                } else {
                    set_output2(var380, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var61, var380, 2, var60);
    add_vectors(var60, (double[]){0.9916390348380518, 0.008360965161948196}, 2, var59);
    add_vectors(var59, (double[]){0.9925449577547608, 0.007455042245239193}, 2, var58);
    double var381[2];
    if (input[9] <= 1.187557494820821) {
        set_output2(var381, 0.9931683970280075, 0.0068316029719924735);
    } else {
        if (input[4] <= 6.160790838126976) {
            if (input[3] <= -42.55228065893452) {
                if (input[6] <= 9.040607078467971) {
                    if (input[1] <= 6.245298595546275) {
                        set_output2(var381, 0.0, 1.0);
                    } else {
                        if (input[5] <= 2.2143592185375995) {
                            set_output2(var381, 0.6603773584905661, 0.33962264150943394);
                        } else {
                            set_output2(var381, 1.0, 0.0);
                        }
                    }
                } else {
                    set_output2(var381, 0.88929889298893, 0.1107011070110701);
                }
            } else {
                set_output2(var381, 0.13461538461538464, 0.8653846153846153);
            }
        } else {
            set_output2(var381, 1.0, 0.0);
        }
    }
    add_vectors(var58, var381, 2, var57);
    add_vectors(var57, (double[]){0.9917149108997515, 0.008285089100248446}, 2, var56);
    add_vectors(var56, (double[]){0.992277509403925, 0.007722490596075022}, 2, var55);
    double var382[2];
    if (input[3] <= -24.213191971959347) {
        if (input[9] <= 4.459151502224297) {
            if (input[9] <= 3.2072802653415895) {
                if (input[9] <= 2.8410188303236725) {
                    if (input[0] <= 27.82270421269179) {
                        set_output2(var382, 0.9432713520327766, 0.056728647967223454);
                    } else {
                        set_output2(var382, 0.9943240043922925, 0.005675995607707491);
                    }
                } else {
                    set_output2(var382, 0.8047722342733189, 0.1952277657266811);
                }
            } else {
                if (input[3] <= -63.26472178949645) {
                    set_output2(var382, 1.0, 0.0);
                } else {
                    set_output2(var382, 0.4274809160305344, 0.5725190839694657);
                }
            }
        } else {
            if (input[7] <= 16.051431421231097) {
                set_output2(var382, 0.4249201277955272, 0.5750798722044729);
            } else {
                set_output2(var382, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var382, 0.9340852497436649, 0.06591475025633513);
    }
    add_vectors(var55, var382, 2, var54);
    add_vectors(var54, (double[]){0.9904832133877369, 0.009516786612263094}, 2, var53);
    double var383[2];
    if (input[0] <= 27.743711991258394) {
        if (input[3] <= -19.086435440584587) {
            if (input[9] <= 1.7134879886763124) {
                if (input[5] <= 0.288498455512257) {
                    if (input[5] <= 0.12420746153859204) {
                        set_output2(var383, 0.9468587623996221, 0.0531412376003779);
                    } else {
                        set_output2(var383, 0.674502712477396, 0.325497287522604);
                    }
                } else {
                    set_output2(var383, 1.0, 0.0);
                }
            } else {
                set_output2(var383, 0.5964125560538117, 0.40358744394618834);
            }
        } else {
            set_output2(var383, 1.0, 0.0);
        }
    } else {
        if (input[9] <= 5.083578494127141) {
            set_output2(var383, 0.9937108571923988, 0.006289142807601191);
        } else {
            set_output2(var383, 0.0, 1.0);
        }
    }
    add_vectors(var53, var383, 2, var52);
    double var384[2];
    if (input[9] <= 10.501268452732484) {
        if (input[1] <= 6.960435866930377) {
            if (input[9] <= 0.5545182823750567) {
                set_output2(var384, 0.9996453674348008, 0.0003546325651991985);
            } else {
                if (input[0] <= 33.53564796494822) {
                    if (input[4] <= 7.197966385349495) {
                        if (input[5] <= 0.25471659843533984) {
                            set_output2(var384, 0.6506211180124224, 0.3493788819875776);
                        } else {
                            set_output2(var384, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var384, 0.810924369747899, 0.1890756302521008);
                    }
                } else {
                    if (input[8] <= 1.0788167163627818) {
                        set_output2(var384, 1.0, 0.0);
                    } else {
                        if (input[9] <= 1.1265449948508914) {
                            set_output2(var384, 0.9689548120041394, 0.03104518799586064);
                        } else {
                            set_output2(var384, 0.47572815533980584, 0.5242718446601942);
                        }
                    }
                }
            }
        } else {
            if (input[9] <= 4.464784509366605) {
                set_output2(var384, 0.9911225538957945, 0.008877446104205487);
            } else {
                set_output2(var384, 0.0, 1.0);
            }
        }
    } else {
        set_output2(var384, 0.3835616438356165, 0.6164383561643836);
    }
    add_vectors(var52, var384, 2, var51);
    double var385[2];
    if (input[0] <= 28.87919235832191) {
        if (input[3] <= -54.40185964724413) {
            set_output2(var385, 1.0, 0.0);
        } else {
            if (input[1] <= 10.81690283780934) {
                set_output2(var385, 0.9756013807811172, 0.02439861921888272);
            } else {
                if (input[6] <= 1.3935751356450676) {
                    set_output2(var385, 0.6520618556701031, 0.34793814432989695);
                } else {
                    set_output2(var385, 0.9572284003421728, 0.04277159965782721);
                }
            }
        }
    } else {
        set_output2(var385, 0.9939464724028806, 0.006053527597119408);
    }
    add_vectors(var51, var385, 2, var50);
    add_vectors(var50, (double[]){0.993548049915431, 0.006451950084568949}, 2, var49);
    add_vectors(var49, (double[]){0.993493572751496, 0.006506427248504036}, 2, var48);
    double var386[2];
    if (input[3] <= -36.59932787743472) {
        if (input[9] <= 2.5419325677628235) {
            if (input[6] <= 6.697564388950153) {
                set_output2(var386, 0.9920367191612175, 0.007963280838782455);
            } else {
                if (input[9] <= 1.6275407267708994) {
                    set_output2(var386, 0.9989930334698892, 0.0010069665301108185);
                } else {
                    set_output2(var386, 0.7878609310548026, 0.21213906894519738);
                }
            }
        } else {
            if (input[3] <= -45.20202789362438) {
                if (input[5] <= 2.032459526002709) {
                    if (input[9] <= 3.547578896936294) {
                        set_output2(var386, 0.8850574712643678, 0.11494252873563217);
                    } else {
                        set_output2(var386, 0.4249201277955272, 0.5750798722044729);
                    }
                } else {
                    set_output2(var386, 1.0, 0.0);
                }
            } else {
                set_output2(var386, 0.3300248138957817, 0.6699751861042184);
            }
        }
    } else {
        set_output2(var386, 0.9717832957110609, 0.028216704288939055);
    }
    add_vectors(var48, var386, 2, var47);
    add_vectors(var47, (double[]){0.991862145203442, 0.008137854796558071}, 2, var46);
    add_vectors(var46, (double[]){0.990731455911237, 0.009268544088763007}, 2, var45);
    double var387[2];
    if (input[2] <= -10.171824575719805) {
        if (input[1] <= 4.011676302574221) {
            if (input[7] <= 17.18056278684485) {
                set_output2(var387, 0.9871995448727066, 0.012800455127293416);
            } else {
                if (input[9] <= 0.21948776708373416) {
                    set_output2(var387, 0.5609756097560976, 0.43902439024390244);
                } else {
                    set_output2(var387, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var387, 0.9970381792794784, 0.0029618207205216606);
        }
    } else {
        if (input[5] <= 0.977386693890773) {
            if (input[6] <= 22.053314196902008) {
                set_output2(var387, 0.9864230405381732, 0.013576959461826847);
            } else {
                if (input[9] <= 1.2215844753319152) {
                    if (input[5] <= 0.03274880654636602) {
                        if (input[0] <= 34.843430467314604) {
                            set_output2(var387, 1.0, 0.0);
                        } else {
                            set_output2(var387, 0.5800933125972006, 0.4199066874027994);
                        }
                    } else {
                        set_output2(var387, 1.0, 0.0);
                    }
                } else {
                    set_output2(var387, 0.8615384615384615, 0.13846153846153844);
                }
            }
        } else {
            set_output2(var387, 0.996009966217714, 0.003990033782286026);
        }
    }
    add_vectors(var45, var387, 2, var44);
    double var388[2];
    if (input[0] <= 7.809006778155919) {
        set_output2(var388, 0.9149981110691349, 0.08500188893086512);
    } else {
        if (input[9] <= 2.2177290033687087) {
            set_output2(var388, 0.9937136907433864, 0.006286309256613592);
        } else {
            if (input[4] <= 1.6938621717050917) {
                if (input[5] <= 0.2401286502255809) {
                    set_output2(var388, 0.5964125560538117, 0.4035874439461883);
                } else {
                    set_output2(var388, 1.0, 0.0);
                }
            } else {
                if (input[9] <= 6.994637669322457) {
                    if (input[7] <= 15.90268953155665) {
                        set_output2(var388, 1.0, 0.0);
                    } else {
                        set_output2(var388, 0.4705882352941177, 0.5294117647058824);
                    }
                } else {
                    set_output2(var388, 0.0, 1.0);
                }
            }
        }
    }
    add_vectors(var44, var388, 2, var43);
    add_vectors(var43, (double[]){0.9917505936439892, 0.00824940635601076}, 2, var42);
    double var389[2];
    if (input[9] <= 5.361730025783987) {
        set_output2(var389, 0.9930843384715691, 0.006915661528430957);
    } else {
        set_output2(var389, 0.0, 1.0);
    }
    add_vectors(var42, var389, 2, var41);
    add_vectors(var41, (double[]){0.9933283450357561, 0.006671654964243947}, 2, var40);
    add_vectors(var40, (double[]){0.9912295661071457, 0.008770433892854297}, 2, var39);
    add_vectors(var39, (double[]){0.9918087800543096, 0.008191219945690371}, 2, var38);
    double var390[2];
    if (input[9] <= 2.2742498793053674) {
        set_output2(var390, 0.994387605801137, 0.005612394198863003);
    } else {
        if (input[1] <= 8.639356223520476) {
            set_output2(var390, 0.09395973154362416, 0.9060402684563759);
        } else {
            if (input[5] <= 3.096706722384349) {
                set_output2(var390, 0.7017398508699254, 0.29826014913007454);
            } else {
                set_output2(var390, 1.0, 0.0);
            }
        }
    }
    add_vectors(var38, var390, 2, var37);
    add_vectors(var37, (double[]){0.9919603815025202, 0.008039618497479775}, 2, var36);
    double var391[2];
    if (input[4] <= 8.003458271972343) {
        if (input[1] <= 9.400438107351604) {
            if (input[9] <= 1.847594103722968) {
                set_output2(var391, 0.9952294747371067, 0.004770525262893372);
            } else {
                set_output2(var391, 0.47976878612716767, 0.5202312138728323);
            }
        } else {
            if (input[4] <= 6.704251979572592) {
                if (input[9] <= 2.3396683851210094) {
                    if (input[9] <= 1.25804906029702) {
                        set_output2(var391, 0.9902579830416741, 0.009742016958325857);
                    } else {
                        if (input[7] <= 15.96202267981565) {
                            set_output2(var391, 0.45652173913043476, 0.5434782608695652);
                        } else {
                            set_output2(var391, 1.0, 0.0);
                        }
                    }
                } else {
                    if (input[4] <= 1.8464411471554218) {
                        set_output2(var391, 1.0, 0.0);
                    } else {
                        set_output2(var391, 0.4988864142538975, 0.5011135857461024);
                    }
                }
            } else {
                if (input[4] <= 7.421679420498088) {
                    set_output2(var391, 0.991008991008991, 0.008991008991008992);
                } else {
                    if (input[5] <= 0.4451489677676743) {
                        if (input[2] <= -5.4587300110538575) {
                            set_output2(var391, 0.7049180327868853, 0.29508196721311475);
                        } else {
                            set_output2(var391, 1.0, 0.0);
                        }
                    } else {
                        set_output2(var391, 1.0, 0.0);
                    }
                }
            }
        }
    } else {
        set_output2(var391, 0.9980501332408952, 0.0019498667591047953);
    }
    add_vectors(var36, var391, 2, var35);
    double var392[2];
    if (input[9] <= 1.0765271732008677) {
        set_output2(var392, 0.9939890551591484, 0.006010944840851627);
    } else {
        if (input[0] <= 32.58746965337362) {
            if (input[6] <= 8.923264151045117) {
                if (input[4] <= 4.429160879344661) {
                    set_output2(var392, 0.5529061102831595, 0.44709388971684044);
                } else {
                    set_output2(var392, 0.0, 1.0);
                }
            } else {
                set_output2(var392, 0.8419666374012291, 0.15803336259877085);
            }
        } else {
            if (input[9] <= 6.01376310299957) {
                set_output2(var392, 0.8989704608871356, 0.10102953911286443);
            } else {
                set_output2(var392, 0.0, 1.0);
            }
        }
    }
    add_vectors(var35, var392, 2, var34);
    add_vectors(var34, (double[]){0.9916047752038643, 0.008395224796135747}, 2, var33);
    double var393[2];
    if (input[9] <= 9.70049689509929) {
        set_output2(var393, 0.990843384414608, 0.009156615585392035);
    } else {
        set_output2(var393, 0.6511627906976745, 0.3488372093023256);
    }
    add_vectors(var33, var393, 2, var32);
    add_vectors(var32, (double[]){0.9918681670014389, 0.008131832998561093}, 2, var31);
    double var394[2];
    if (input[0] <= 26.493088109447257) {
        if (input[7] <= 14.45857262740138) {
            set_output2(var394, 1.0, 0.0);
        } else {
            if (input[3] <= -38.68095170365205) {
                set_output2(var394, 0.9777172567467195, 0.022282743253280512);
            } else {
                set_output2(var394, 0.23728813559322035, 0.7627118644067796);
            }
        }
    } else {
        if (input[9] <= 7.648202190632889) {
            if (input[9] <= 4.228071853838229) {
                set_output2(var394, 0.9930486233582639, 0.006951376641736016);
            } else {
                set_output2(var394, 0.0, 1.0);
            }
        } else {
            set_output2(var394, 0.0, 1.0);
        }
    }
    add_vectors(var31, var394, 2, var30);
    add_vectors(var30, (double[]){0.9917546308011277, 0.008245369198872305}, 2, var29);
    add_vectors(var29, (double[]){0.9914286908893258, 0.008571309110674205}, 2, var28);
    double var395[2];
    if (input[4] <= 4.529117741569907) {
        if (input[1] <= 7.847808367268809) {
            if (input[9] <= 2.1823285981886604) {
                set_output2(var395, 0.9974097539074342, 0.0025902460925658066);
            } else {
                set_output2(var395, 0.28, 0.72);
            }
        } else {
            set_output2(var395, 0.985533525279671, 0.01446647472032899);
        }
    } else {
        set_output2(var395, 0.9964767596628128, 0.0035232403371871592);
    }
    add_vectors(var28, var395, 2, var27);
    double var396[2];
    if (input[9] <= 5.731783397481309) {
        set_output2(var396, 0.9911760858777927, 0.008823914122207295);
    } else {
        set_output2(var396, 0.0, 1.0);
    }
    add_vectors(var27, var396, 2, var26);
    add_vectors(var26, (double[]){0.9913119451333636, 0.008688054866636395}, 2, var25);
    double var397[2];
    if (input[0] <= 29.72648717384592) {
        if (input[9] <= 1.232071852464001) {
            set_output2(var397, 0.989155352586599, 0.010844647413400967);
        } else {
            set_output2(var397, 0.622113365990203, 0.3778866340097971);
        }
    } else {
        set_output2(var397, 0.9927288756796797, 0.007271124320320232);
    }
    add_vectors(var25, var397, 2, var24);
    add_vectors(var24, (double[]){0.9914620054615402, 0.008537994538459865}, 2, var23);
    add_vectors(var23, (double[]){0.9914193389961047, 0.008580661003895268}, 2, var22);
    add_vectors(var22, (double[]){0.9922553168528958, 0.007744683147104138}, 2, var21);
    double var398[2];
    if (input[9] <= 6.129899914902588) {
        if (input[9] <= 2.6539128775228193) {
            if (input[9] <= 1.9144285135743315) {
                set_output2(var398, 0.9944416464938443, 0.005558353506155717);
            } else {
                if (input[8] <= 1.0389809268426398) {
                    set_output2(var398, 0.7313432835820896, 0.2686567164179104);
                } else {
                    set_output2(var398, 1.0, 0.0);
                }
            }
        } else {
            if (input[9] <= 3.3980686967369538) {
                set_output2(var398, 1.0, 0.0);
            } else {
                set_output2(var398, 0.29319371727748694, 0.706806282722513);
            }
        }
    } else {
        set_output2(var398, 0.0, 1.0);
    }
    add_vectors(var21, var398, 2, var20);
    add_vectors(var20, (double[]){0.9919388572207601, 0.008061142779239888}, 2, var19);
    add_vectors(var19, (double[]){0.9909831997319145, 0.00901680026808549}, 2, var18);
    double var399[2];
    if (input[0] <= 8.04954870259489) {
        set_output2(var399, 0.6985934360348293, 0.30140656396517074);
    } else {
        set_output2(var399, 0.9923135002404053, 0.007686499759594727);
    }
    add_vectors(var18, var399, 2, var17);
    add_vectors(var17, (double[]){0.9915747108184183, 0.00842528918158174}, 2, var16);
    double var400[2];
    if (input[0] <= 24.979041184153072) {
        if (input[1] <= 0.5781750947148476) {
            set_output2(var400, 1.0, 0.0);
        } else {
            set_output2(var400, 0.199288256227758, 0.800711743772242);
        }
    } else {
        if (input[1] <= 3.5265132385990086) {
            if (input[7] <= 16.590567543928437) {
                set_output2(var400, 1.0, 0.0);
            } else {
                set_output2(var400, 0.5800933125972006, 0.4199066874027994);
            }
        } else {
            set_output2(var400, 0.9938101903617088, 0.006189809638291229);
        }
    }
    add_vectors(var16, var400, 2, var15);
    add_vectors(var15, (double[]){0.9901072833547678, 0.009892716645232202}, 2, var14);
    double var401[2];
    if (input[3] <= -29.201247073238477) {
        if (input[6] <= 2.43438026049844) {
            if (input[0] <= 35.81037011179904) {
                set_output2(var401, 0.9901558527958986, 0.009844147204101395);
            } else {
                if (input[2] <= -12.100597464207206) {
                    if (input[6] <= 0.06439819976387214) {
                        set_output2(var401, 1.0, 0.0);
                    } else {
                        if (input[0] <= 36.97580128729275) {
                            set_output2(var401, 1.0, 0.0);
                        } else {
                            set_output2(var401, 0.6803977272727273, 0.31960227272727276);
                        }
                    }
                } else {
                    if (input[8] <= 1.3794778425252856) {
                        set_output2(var401, 0.9836367886548402, 0.01636321134515986);
                    } else {
                        if (input[6] <= 0.9503891107629037) {
                            set_output2(var401, 0.4962686567164179, 0.503731343283582);
                        } else {
                            set_output2(var401, 1.0, 0.0);
                        }
                    }
                }
            }
        } else {
            set_output2(var401, 0.9936878655527386, 0.006312134447261392);
        }
    } else {
        set_output2(var401, 0.8324958123953099, 0.1675041876046901);
    }
    add_vectors(var14, var401, 2, var13);
    double var402[2];
    if (input[9] <= 7.330161604252719) {
        if (input[9] <= 1.0243398374146508) {
            set_output2(var402, 0.9940463111936438, 0.005953688806356198);
        } else {
            if (input[8] <= 0.4999107861339677) {
                set_output2(var402, 0.47058823529411764, 0.5294117647058824);
            } else {
                set_output2(var402, 0.8903553299492386, 0.10964467005076138);
            }
        }
    } else {
        set_output2(var402, 0.26229508196721313, 0.7377049180327868);
    }
    add_vectors(var13, var402, 2, var12);
    double var403[2];
    if (input[1] <= 3.6036258657854647) {
        if (input[6] <= 1.3856547433266209) {
            if (input[7] <= 16.569878370891484) {
                set_output2(var403, 1.0, 0.0);
            } else {
                set_output2(var403, 0.3684210526315789, 0.631578947368421);
            }
        } else {
            set_output2(var403, 1.0, 0.0);
        }
    } else {
        if (input[0] <= 28.470324756425633) {
            set_output2(var403, 0.9591918988362134, 0.04080810116378661);
        } else {
            if (input[2] <= -19.568021918737628) {
                if (input[7] <= 16.13561349191093) {
                    if (input[0] <= 37.0440117930305) {
                        if (input[9] <= 0.14105938814529634) {
                            set_output2(var403, 1.0, 0.0);
                        } else {
                            set_output2(var403, 0.7142857142857143, 0.2857142857142857);
                        }
                    } else {
                        set_output2(var403, 0.7804878048780488, 0.21951219512195122);
                    }
                } else {
                    set_output2(var403, 1.0, 0.0);
                }
            } else {
                if (input[3] <= -51.53389737013842) {
                    if (input[7] <= 15.76481479427365) {
                        if (input[5] <= 0.5954999354175821) {
                            set_output2(var403, 0.9891561599036103, 0.010843840096389695);
                        } else {
                            set_output2(var403, 0.7715736040609137, 0.22842639593908629);
                        }
                    } else {
                        set_output2(var403, 0.991102369851805, 0.008897630148195012);
                    }
                } else {
                    set_output2(var403, 0.9939798995535092, 0.006020100446490801);
                }
            }
        }
    }
    add_vectors(var12, var403, 2, var11);
    double var404[2];
    if (input[2] <= 1.5414090873742232) {
        if (input[9] <= 7.548544699290101) {
            if (input[1] <= 7.22216077582878) {
                set_output2(var404, 0.996406695306636, 0.003593304693364072);
            } else {
                if (input[2] <= -1.0566057258925472) {
                    set_output2(var404, 0.9915650065349899, 0.0084349934650101);
                } else {
                    set_output2(var404, 0.7988077496274218, 0.20119225037257826);
                }
            }
        } else {
            set_output2(var404, 0.0, 1.0);
        }
    } else {
        set_output2(var404, 0.23728813559322035, 0.7627118644067796);
    }
    add_vectors(var11, var404, 2, var10);
    add_vectors(var10, (double[]){0.9914194028462471, 0.008580597153752936}, 2, var9);
    double var405[2];
    if (input[5] <= 1.8020437054841159) {
        set_output2(var405, 0.9935021573074456, 0.006497842692554415);
    } else {
        if (input[0] <= 34.73447750074822) {
            set_output2(var405, 0.9891369945684972, 0.010863005431502714);
        } else {
            if (input[3] <= -58.961392836263414) {
                set_output2(var405, 0.8132780082987552, 0.18672199170124482);
            } else {
                if (input[0] <= 38.986120796275) {
                    set_output2(var405, 1.0, 0.0);
                } else {
                    set_output2(var405, 0.787359716479622, 0.212640283520378);
                }
            }
        }
    }
    add_vectors(var9, var405, 2, var8);
    add_vectors(var8, (double[]){0.9919284128254611, 0.008071587174538958}, 2, var7);
    add_vectors(var7, (double[]){0.9926705476310863, 0.007329452368913728}, 2, var6);
    double var406[2];
    if (input[3] <= -35.554990592938765) {
        if (input[6] <= 33.8208009646371) {
            if (input[6] <= 32.550152102083274) {
                if (input[2] <= -13.238345871212656) {
                    if (input[9] <= 0.6596582621884702) {
                        set_output2(var406, 0.9980330749376594, 0.0019669250623405954);
                    } else {
                        if (input[9] <= 1.4667782490365564) {
                            set_output2(var406, 1.0, 0.0);
                        } else {
                            set_output2(var406, 0.3300248138957817, 0.6699751861042184);
                        }
                    }
                } else {
                    set_output2(var406, 0.9877369964220291, 0.012263003577970923);
                }
            } else {
                if (input[1] <= 7.676044619039747) {
                    set_output2(var406, 1.0, 0.0);
                } else {
                    set_output2(var406, 0.8091872791519434, 0.19081272084805653);
                }
            }
        } else {
            set_output2(var406, 0.9972202145994329, 0.0027797854005670803);
        }
    } else {
        if (input[4] <= 7.14127142089307) {
            if (input[8] <= 0.7169870885502705) {
                set_output2(var406, 1.0, 0.0);
            } else {
                if (input[7] <= 16.054436798927572) {
                    set_output2(var406, 0.5714285714285714, 0.42857142857142855);
                } else {
                    set_output2(var406, 0.9533437013996889, 0.04665629860031104);
                }
            }
        } else {
            set_output2(var406, 1.0, 0.0);
        }
    }
    add_vectors(var6, var406, 2, var5);
    add_vectors(var5, (double[]){0.9927969500825162, 0.007203049917483729}, 2, var4);
    double var407[2];
    if (input[1] <= 8.220849341203927) {
        if (input[9] <= 3.1641348861922207) {
            if (input[9] <= 0.8927120892780804) {
                set_output2(var407, 0.998821359326289, 0.001178640673711012);
            } else {
                if (input[6] <= 52.07700741948367) {
                    if (input[9] <= 1.2100921797566178) {
                        set_output2(var407, 1.0, 0.0);
                    } else {
                        if (input[6] <= 2.9613109807259033) {
                            set_output2(var407, 1.0, 0.0);
                        } else {
                            set_output2(var407, 0.6056955093099672, 0.39430449069003287);
                        }
                    }
                } else {
                    set_output2(var407, 1.0, 0.0);
                }
            }
        } else {
            set_output2(var407, 0.497907949790795, 0.502092050209205);
        }
    } else {
        set_output2(var407, 0.9903292428398152, 0.009670757160184824);
    }
    add_vectors(var4, var407, 2, var3);
    add_vectors(var3, (double[]){0.9919097289187151, 0.00809027108128496}, 2, var2);
    double var408[2];
    if (input[9] <= 3.429208122518031) {
        set_output2(var408, 0.9939781079999798, 0.006021892000020264);
    } else {
        set_output2(var408, 0.5172824791418356, 0.48271752085816444);
    }
    add_vectors(var2, var408, 2, var1);
    mul_vector_number(var1, 0.0035714285714285713, 2, var0);
    memcpy(output, var0, 2 * sizeof(double));
}
