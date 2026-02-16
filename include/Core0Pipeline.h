#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "SmartPlugConfig.h"
#include "SmartPlugTypes.h"
#include "ArcFeatures.h"

class CurrentSensor;

class Core0Pipeline {
public:
  bool begin(QueueHandle_t outQ, CurrentSensor* cur, ArcFeatures* feat);
  void setTimeProvider(uint64_t (*epochMsFn)());
  void setCalib(const CurrentCalib& cal) { _cal = cal; }
  void stop();

private:
  static void taskEntry(void* arg);
  void taskLoop();

  TaskHandle_t _task = nullptr;
  QueueHandle_t _q = nullptr;

  CurrentSensor* _cur = nullptr;
  ArcFeatures* _feat = nullptr;

  CurrentCalib _cal{};

  uint64_t (*_epochMsFn)() = nullptr;
};