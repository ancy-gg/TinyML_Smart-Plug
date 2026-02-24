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

  while (true) {
    FeatureFrame f;
    f.uptime_ms = millis();
    f.epoch_ms  = _epochMsFn ? _epochMsFn() : 0;

    float fs = 0.0f;
    const size_t got = _cur->capture(s_raw, N_SAMP, &fs);

    if (got > 0) {
      // ---> ADD THIS DEBUG LINE <---
      Serial.printf("RAW ADC[0]: %u | RAW ADC[1]: %u\n", s_raw[0], s_raw[1]);
    }
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

    vTaskDelay(1);
  }
}