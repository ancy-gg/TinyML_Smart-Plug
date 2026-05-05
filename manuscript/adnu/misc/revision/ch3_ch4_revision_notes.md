# Chapters 3 and 4 Revision Notes

## Structural Issues Found

- Chapter 3 and Chapter 4 were still in Word/Pandoc-style LaTeX, with manual bold figure labels instead of `figure` environments and manual table labels instead of `table` captions.
- Figure paths pointed to `figures/media/...`, but the project already stores organized figures under `figures/ch3`, `figures/ch4`, and `figures/apx`.
- Chapter 3 mixed system model, firmware flow, hardware, computations, TinyML, and test procedures in a way that made later Chapter 4 results harder to follow.
- Chapter 4 did not consistently follow the Chapter 3 methodology order. It also mixed data collection, protection testing, and model performance in the same flow.
- Socket-heating text did not clearly separate direct NTC temperature, expected normal socket temperature, estimated socket temperature, and reference terminal temperature.
- Several equations lacked variable definitions, labels, or direct connection to firmware constants.
- Several tables used meshed-grid or converted longtable markup instead of clean thesis-style booktabs tables.
- Appendix references were inconsistent with the corrected appendix map. Heating characterization now points to Appendix C, heating validation to Appendix D, arc testing to Appendix E, and arc logs to Appendix F.
- Several computational-method claims needed stronger method citations, especially regression, FFT/windowing, Random Forest, train/test evaluation, error metrics, and TinyML deployment.

## Revised Chapter 3 Section Files

- `chapters/chapter_3/01_system_model.tex`
- `chapters/chapter_3/02_program_flow.tex`
- `chapters/chapter_3/03_physical_setup.tex`
- `chapters/chapter_3/04_pwa_monitoring.tex`
- `chapters/chapter_3/05_tinyml_workflow.tex`
- `chapters/chapter_3/06_sensor_calibration_computations.tex`
- `chapters/chapter_3/07_waveform_features.tex`
- `chapters/chapter_3/08_fault_detection_protection.tex`
- `chapters/chapter_3/09_test_evaluation.tex`

## Revised Chapter 4 Section Files

- `chapters/chapter_4/01_sensor_validation.tex`
- `chapters/chapter_4/02_overload_protection.tex`
- `chapters/chapter_4/03_socket_heating_characterization.tex`
- `chapters/chapter_4/04_socket_heating_protection.tex`
- `chapters/chapter_4/05_series_arc_fault_detection.tex`
- `chapters/chapter_4/06_tinyml_model_performance.tex`
- `chapters/chapter_4/07_overall_feasibility.tex`

## Citation Update Table

| Claim/Method | Current Location | Suggested Source | BibTeX Key | Where to Cite |
|---|---|---|---|---|
| Random Forest classification | Chapter 3 TinyML workflow and arc model | Breiman, "Random Forests" | `breiman2001random` | Ch. 3 TinyML model section |
| Train/test split, model evaluation, scikit-learn implementation | Chapter 3 training split and Chapter 4 model performance | Pedregosa et al., scikit-learn | `pedregosa2011scikit` | Ch. 3 training split; Ch. 4 metrics |
| FFT windowing and Hann/windowed spectral analysis | Chapter 3 waveform features | Harris, DFT windows | `harris1978windows` | Ch. 3 waveform feature computation |
| Discrete-time signal processing, RMS, FFT basis | Chapter 3 waveform features | Oppenheim and Schafer | `oppenheim2010discrete` | Ch. 3 waveform feature computation |
| Least-squares regression and calibration modeling | Chapter 3 calibration and thermal fit discussion | Montgomery, Peck, and Vining | `montgomery2012linear` | Ch. 3 sensor calibration; socket model |
| MAE, MSE, RMSE, MAPE | Chapter 3 evaluation metrics and Chapter 4 sensor validation | Hyndman and Koehler | `hyndman2006accuracy` | Ch. 3 sensor metrics; Ch. 4 sensor interpretation |
| TinyML/microcontroller deployment | Chapter 3 TinyML workflow | Warden and Situnayake | `warden2019tinyml` | Ch. 3 TinyML deployment |
| Accuracy, precision, recall, F1, confusion matrices | Chapter 3 classification metrics and Chapter 4 TinyML metrics | Powers | `powers2011evaluation` | Ch. 3 metrics; Ch. 4 arc interpretation |
| Thermistor measurement and sensor placement limits | Chapter 3 thermal computation and Chapter 4 heating interpretation | Existing thermistor and hotspot references | existing keys from refs [27], [29], [30], [46], [47] | Ch. 3 socket model; Ch. 4 heating results |
| Arc waveform disturbance, THD, spectral features | Chapter 3 waveform features and Chapter 4 arc discussion | Existing arc and THD references | existing keys from refs [34], [35], [50], [53], [54] | Ch. 3 waveform features; Ch. 4 arc results |

## New BibTeX Entries Added

The following entries were added to `references.bib` and manually numbered in `misc/references.tex` as [68]--[75]:

- `breiman2001random`
- `pedregosa2011scikit`
- `harris1978windows`
- `montgomery2012linear`
- `hyndman2006accuracy`
- `warden2019tinyml`
- `powers2011evaluation`
- `oppenheim2010discrete`

## Items Needing Manual Verification

- The original least-squares worksheet or script used to derive the socket-heating constants was not found. Firmware constants were verified, but the derivation source for the normal gain `12.0` and measured compensation gain `1.20` should be checked against Appendix C or the original characterization notebook.
- The IDE tab referenced `c:\Users\annam\Downloads\tinyml_renamed_chapter_figures\tinyml_chapter_figures_renamed\figure_mapping.csv`, but that file was not found at the displayed path. Figure paths were repaired using the organized project folders already present in `manuscript/adnu/figures`.
- The deployed context model uses squared Euclidean centroid distance, not Manhattan/L1 distance. No L1-distance citation was added because the code does not use it.
- Final page layout and long-table fit should be checked after a full `main.tex` build because section wrappers are only preview builds.
