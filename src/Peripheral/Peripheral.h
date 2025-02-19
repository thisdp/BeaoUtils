#pragma once

#include <Arduino.h>
#include "../Logger/Logger.h"
#include "PeripheralType.h"
#include "AlarmDefinition.h"

#if defined(ESP32)
typedef void(*IODigitalWrite)(uint8_t pin, uint8_t state);
typedef int(*IODigitalRead)(uint8_t pin);
#else
typedef void(*IODigitalWrite)(uint32_t pin, uint32_t state);
typedef int(*IODigitalRead)(uint32_t pin);
#endif
// 声明全局变量
extern uint16_t globalPeripheralCount;
extern Logger globalPDL;  // Global Peripheral Debug Logger
extern IODigitalWrite gDigitalWrite;
extern IODigitalRead gDigitalRead;

class BasicIndustrialPeripheral;

// 回调函数类型定义
typedef void(*BasicIndustrialPeripheralCallBack)(BasicIndustrialPeripheral *peri, uint16_t oldAlarm);

// 类声明
class BasicIndustrialPeripheral {
protected:
    uint16_t periID;       // ID
    uint16_t periType;     // 类型
    uint16_t alarmState;   // 报警
    const char* periName;  // 名称
    uint8_t alarmSolutionStep;   // 报警解决器步骤
    uint8_t alarmSolutionActionType;  // 报警解决器动作类型
    IODigitalWrite ioWrite;
    IODigitalRead ioRead;
    bool updateNotify;     // 更新通知
    bool debugOn;          // 通用debug模式开关

    void onInternalAlarm(uint16_t oldAlarm);

public:
    static bool useDigitalWrite(IODigitalWrite hookedDigitalWrite);
    static bool useDigitalRead(IODigitalRead hookedDigitalRead);
    static bool useDigitalIO(IODigitalRead hookedDigitalRead, IODigitalWrite hookedDigitalWrite);
    static bool setGlobalPeripheralDebugStream(Stream &stream); // 设置全局调试输出流

    BasicIndustrialPeripheral(uint16_t type);

    uint16_t& getAlarmRef();
    const char* getName();
    uint16_t getID();
    uint16_t getAlarm();
    void setAlarm(uint16_t alarm);
    void resetAlarm();
    virtual void onAlarmReset();
    void setNotify(bool state);
    bool getNotify();
    const char* getAlarmDetail(uint16_t alarm);

    // 回调函数
    BasicIndustrialPeripheralCallBack onAlarm;

    void setDebugEnabled(bool state);
    bool isDebugEnabled();
    void toggleDebugMode();

    virtual bool setRun(bool state) = 0;
    virtual bool isRunning() = 0;

    // 报警解决器
    void alarmSolutionSetActionType(uint8_t type);
    bool alarmIsSolving();
    void alarmTryToSolve();
    virtual void alarmSolutionUpdate() = 0;
};