#include "AlarmSolution.h"
#include "../Peripheral/AlarmDefinition.h"

// 构造函数初始化成员变量
AlarmSolution::AlarmSolution(uint8_t argMaxContinuousSolvingTime, uint32_t argMinAllowAlarmDuration) :
  inputAlarm(false),  // 输入报警信号
  wasAlarmed(false),  // 是否有报警
  waitReset(false),   // 是否等待复位
  wasResolved(false), // 是否已解决
  step(AlarmSolutionStep::Idle),  // 当前步骤为无操作
  solvingTimes(0),    // 解决次数初始化为0
  lastHappenedTick(0),  // 上次发生时间戳初始化为0
  maxContinuousSolvingTime(argMaxContinuousSolvingTime),  // 最大连续解决次数
  minAllowAlarmDuration(argMinAllowAlarmDuration) {}  // 最小允许的报警间隔

// 检查是否有报警
bool AlarmSolution::hasAlarm() {
  return wasAlarmed;
}

// 检查是否已解决
bool AlarmSolution::isResolved() {
  return wasResolved;
}

// 设置报警
bool AlarmSolution::setAlarm() {
  wasResolved = false;  // 标记为未解决
  if (maxContinuousSolvingTime != 0 && solvingTimes >= maxContinuousSolvingTime) {  // 如果超过最大连续解决次数
    waitReset = true;  // 等待复位
    onAlarmSolveFailed();  // 调用报警处理失败逻辑
    return false;  // 不再处理报警
  }
  solvingTimes++;  // 增加解决次数
  wasAlarmed = true;  // 标记为有报警
  step = AlarmSolutionStep::Alarm;  // 设置当前步骤为存在报警
  debugLogger.println("[报警处理]出现报警");  // 如果设置报警失败，打印不再处理报警信息
  onAlarm();  // 调用报警触发程序
  return true;  // 返回成功
}

// 检查是否等待复位
bool AlarmSolution::isWaitingReset() {
  return waitReset;
}

// 解决报警
void AlarmSolution::resolveAlarm() {
  lastHappenedTick = millis();  // 记录重置完报警后的时间
  wasAlarmed = false;  // 清除报警标记
  wasResolved = true;  // 标记为已解决
  waitReset = false;  // 清除等待复位标记
  step = AlarmSolutionStep::Idle;  // 设置当前步骤为无操作
}

// 强制复位报警
void AlarmSolution::resetAlarm() {
  lastHappenedTick = millis();  // 记录重置完报警后的时间
  wasAlarmed = false;  // 清除报警标记
  waitReset = false;  // 清除等待复位标记
  solvingTimes = 0;  // 复位解决次数
}

void AlarmSolution::setDebugStream(Stream &dbStream){
  debugLogger.setStream(dbStream);
}

// 更新报警状态
bool AlarmSolution::update(bool argInputAlarm) {
  if (step == AlarmSolutionStep::Idle) {  // 如果当前步骤是无操作
    if (solvingTimes != 0 && millis() - lastHappenedTick >= minAllowAlarmDuration) {  // 如果解决次数不为0且距离上次报警时间超过最小允许间隔
      debugLogger.println("[报警处理]记录复位");  // 打印报警记录复位信息
      solvingTimes = 0;  // 复位解决次数
      return true;  // 返回成功
    }
  }
  inputAlarm = argInputAlarm;  // 更新输入报警信号
  if (inputAlarm && !hasAlarm()) {  // 如果有新的报警信号且当前没有报警
    if (!isWaitingReset()) {  // 如果不处于等待复位状态
      if (!setAlarm()) {  // 尝试设置报警
        debugLogger.println("[报警处理]不再处理报警");  // 如果设置报警失败，打印不再处理报警信息
        return false;  // 返回失败
      }
    } else {
      return false;  // 如果处于等待复位状态，返回失败
    }
  }
  return true;  // 返回成功
}

// 更新解决方案尝试
void AlarmSolution::updateSolutionAttempt(void *arg) {
  onSolutionStepLoop(arg);  // 调用用户自定义的报警处理程序
}