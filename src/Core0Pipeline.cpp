#include "Core0Pipeline.h"
#include "CurrentSensor.h"

static uint16_t s_raw[N_SAMP];

bool Core0Pipeline::begin(QueueHandle_t outQ, CurrentSensor* cur, ArcFeatures* feat) {
  _q = outQ;
  _cur = cur;
  _feat = feat;

  if (!_q || !_cur || !_feat) return false;

  xTaskCreatePinnedToCore(
    Core0Pipeline::taskEntry,
    "Core0Sense",
    12288,
    this,
    3,
    &_task,
    0
  );

  return true;
}

void Core0Pipeline::setTimeProvider(uint64_t (*epochMsFn)()) {
  _epochMsFn = epochMsFn;
}

void Core0Pipeline::stop() {
  if (_task) {
    vTaskDelete(_task);
    _task = nullptr;
  }
}

void Core0Pipeline::taskEntry(void* arg) {
  ((Core0Pipeline*)arg)->taskLoop();
}

void Core0Pipeline::taskLoop() {
  ArcFeatOut out;
  
  // TRIPWIRE 1: Check if the task even launched
  Serial.println("\n[Core0] TASK HAS SUCCESSFULLY STARTED!");

  while (true) {
    FeatureFrame f;
    f.uptime_ms = millis();
    f.epoch_ms  = _epochMsFn ? _epochMsFn() : 0;

    float fs = 0.0f;
    const size_t got = _cur->capture(s_raw, N_SAMP, &fs);

    // --- MOVE THE DEBUG BLOCK OUTSIDE THE IF STATEMENT ---
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 1000) {
      lastPrint = millis();
      // Print 'got' to see if the capture is failing and returning 0
      Serial.printf("[SPI DEBUG] Captured: %d samples | RAW ADC[0]: %u\n", got, s_raw[0]);
    }
    // -----------------------------------------------------

    if (got > 0) {
      if (got == N_SAMP && fs > 20000.0f) {
        if (_feat->compute(s_raw, N_SAMP, fs, _cal, MAINS_F0_HZ, out)) {
          f.irms    = out.irms_a;
          f.thd_pct = out.thd_pct;
          f.entropy = out.entropy;
          f.zcv_ms  = out.zcv_ms;

          // vrms/temp/model_pred are filled on core1
          xQueueOverwrite(_q, &f);
        }
      }
    }

    vTaskDelay(1);
  }
}