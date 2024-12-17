#pragma once
#include <stdint.h>  // 用于 uint16_t, uint8_t, uint32_t
#include <Arduino.h> // 用于 millis, Serial (如果使用 Arduino 框架)
#include "../Logger/Logger.h"


// 报警解决方案类（这是一个抽象类，需要派生）
class AlarmSolution {
public:

  AlarmSolution(uint8_t argMaxContinuousSolvingTime = 3, uint32_t argMinAllowAlarmDuration = 60 * 1000);  // 构造函数
  bool hasAlarm();  // 检查是否有报警
  bool isResolved();  // 检查是否已解决
  bool setAlarm();  // 设置报警
  bool isWaitingReset();  // 检查是否等待复位
  void resolveAlarm();  // 解决报警
  void resetAlarm();  // 强制复位报警
  bool update(bool argInputAlarm);  // 更新报警状态
  void updateSolutionAttempt(void *arg = NULL);  // 更新解决方案尝试
  void setDebugStream(Stream &dbStream);
  // 成员变量
  bool wasResolved;            // 是否已解决
  bool inputAlarm;             // 输入报警信号
  bool wasAlarmed;             // 是否有报警
  bool waitReset;              // 是否等待复位
  uint8_t solvingTimes;        // 解决次数
  uint16_t step;               // 当前步骤
  uint32_t lastHappenedTick;   // 上次发生时间戳
  uint32_t minAllowAlarmDuration;  // 最小允许的报警间隔
  uint8_t maxContinuousSolvingTime;  // 最大连续解决次数
protected:
  virtual void onAlarm() = 0;  // 用户自定义的报警触发程序
  virtual void onSolutionStepLoop(void *arg = NULL) = 0;  // 用户自定义的报警处理程序
  virtual void onAlarmSolveFailed(void *arg = NULL) = 0;  // 用户自定义的报警处理失败逻辑
private:
  Logger debugLogger;
};