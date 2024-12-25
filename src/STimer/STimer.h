#pragma once
#include <Arduino.h>
class STimerBase{
public:
  STimerBase(uint32_t tDuration = 0);
  bool start();
  bool startIfNotActivated();
  bool startIfNotRunning();
  bool restart();
  void clearRestartTimes();
  bool stop();
  bool isActivated();
  uint32_t getStartTime();
  uint32_t getPassedTime();
  void setDuration(uint32_t tDuration);
  bool checkTimedOut();
  virtual uint32_t getCurrentTime() = 0;
protected:
  uint32_t duration;
  uint32_t startTime;
  uint32_t restartTimes;
  bool activated;
};

class STimer : public STimerBase{
public:
  STimer(uint32_t tDuration = 0);
  uint32_t getCurrentTime();
};

class MSTimer : public STimerBase{
public:
  MSTimer(uint32_t tDuration = 0);
  uint32_t getCurrentTime();
};

class USTimer : public STimerBase{
public:
  USTimer(uint32_t tDuration = 0);
  uint32_t getCurrentTime();
};