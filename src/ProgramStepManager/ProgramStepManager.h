#pragma once

#include <Arduino.h>
#include "../Logger/Logger.h"
#include "../Peripheral/AlarmDefinition.h"
#include "../Peripheral/Peripheral.h"

// 定义默认步骤宏
#define PSMDefaultSteps \
    PowerOn = 0, \
    NotInitialized, \
	Pause, \
	Space, \
	BreakPoint, \
	Initialize, \
    Initialized, \
	Stopping, \
	Stopped, \
	Loop = 20

// 枚举类定义
enum class PSMStep : uint16_t {
    PSMDefaultSteps,
};

// 获取基本步骤名称的函数声明
const char* getBasicStepName(PSMStep step);

// 类声明
class ProgramStepManager : public BasicIndustrialPeripheral {
public:
    // 状态位
    union {
        struct {
            uint16_t alarmReset:1;    // 复位报警
            uint16_t reserved1:15;    // 保留
        };
        uint16_t rwState;              // 外部可读写状态
    };

    union {
        struct {
            uint16_t run:1;           // 运行中
            uint16_t reserved:15;
        };
        uint16_t rState;  // 外部只读状态
    };

    uint16_t step;
    uint32_t lastTick;

public:
    ProgramStepManager(const char* periCustomName);
    void stepTo(PSMStep main);
    void stepTo(uint16_t main);
    bool setRun(bool state);
    bool isRunning();
    const char* getStepName();
    uint16_t getStepNumber();
    uint16_t& getStepRef();
    uint16_t& getReadWriteStateFlagRef();
    uint16_t& getReadOnlyStateFlagRef();
    void alarmSolutionUpdate();

private:
    const char* periName;
};