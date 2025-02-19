#include "Peripheral.h"

// 全局变量初始化
uint16_t globalPeripheralCount = 0;
Logger globalPDL;

// 默认使用的 IO 函数
IODigitalWrite gDigitalWrite = digitalWrite;
IODigitalRead gDigitalRead = digitalRead;

void BasicIndustrialPeripheral::onInternalAlarm(uint16_t oldAlarm) {
    if (alarmState == AlarmType::NoAlarm) {
        onAlarmReset();
        return;
    }
    globalPDL.printfln("[报警][ID:%d][类型:%d]%s: %s (%d)", periID, periType, periName, getAlarmDetail(alarmState), alarmState);
    if (onAlarm) onAlarm(this, oldAlarm);
}

bool BasicIndustrialPeripheral::useDigitalWrite(IODigitalWrite hookedDigitalWrite) {
    gDigitalWrite = hookedDigitalWrite;
    return true;
}

bool BasicIndustrialPeripheral::useDigitalRead(IODigitalRead hookedDigitalRead) {
    gDigitalRead = hookedDigitalRead;
    return true;
}

bool BasicIndustrialPeripheral::useDigitalIO(IODigitalRead hookedDigitalRead, IODigitalWrite hookedDigitalWrite) {
    gDigitalWrite = hookedDigitalWrite;
    gDigitalRead = hookedDigitalRead;
    return true;
}

bool BasicIndustrialPeripheral::setGlobalPeripheralDebugStream(Stream &stream) {
    globalPDL.setStream(stream);
    return true;
}

BasicIndustrialPeripheral::BasicIndustrialPeripheral(uint16_t type)
    : periID(globalPeripheralCount++),
      periType(type),
      alarmState(false),
      periName("基础外设"),
      alarmSolutionStep(AlarmSolutionStep::Idle),
      alarmSolutionActionType(AlarmSolutionType::NoSolution),
      ioWrite(gDigitalWrite),
      ioRead(gDigitalRead),
      updateNotify(false),
      debugOn(false) {}

uint16_t& BasicIndustrialPeripheral::getAlarmRef() {
    return alarmState;
}

const char* BasicIndustrialPeripheral::getName() {
    return periName;
}

uint16_t BasicIndustrialPeripheral::getID() {
    return periID;
}

uint16_t BasicIndustrialPeripheral::getAlarm() {
    return alarmState;
}

void BasicIndustrialPeripheral::setAlarm(uint16_t alarm) {
    if (alarmState != alarm) {
        uint16_t oldAlarm = alarmState;
        alarmState = alarm;
        onInternalAlarm(oldAlarm);
    }
    setNotify(true);
}

void BasicIndustrialPeripheral::resetAlarm() {
    setAlarm(AlarmType::NoAlarm);
}

void BasicIndustrialPeripheral::onAlarmReset() {
    // 纯虚函数默认实现为空
}

void BasicIndustrialPeripheral::setNotify(bool state) {
    updateNotify = state;
}

bool BasicIndustrialPeripheral::getNotify() {
    return updateNotify;
}

const char* BasicIndustrialPeripheral::getAlarmDetail(uint16_t alarm) {
    return getBasicAlarmDetail(periType, alarm);
}

void BasicIndustrialPeripheral::setDebugEnabled(bool state) {
    debugOn = state;
}

bool BasicIndustrialPeripheral::isDebugEnabled() {
    return debugOn;
}

void BasicIndustrialPeripheral::toggleDebugMode() {
    debugOn = !debugOn;
}

void BasicIndustrialPeripheral::alarmSolutionSetActionType(uint8_t type) {
    alarmSolutionActionType = type;
}

bool BasicIndustrialPeripheral::alarmIsSolving() {
    return alarmSolutionStep != AlarmSolutionStep::Idle;
}

void BasicIndustrialPeripheral::alarmTryToSolve() {
    alarmSolutionStep = AlarmSolutionStep::RequestSolve;
}