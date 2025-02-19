#include "ProgramStepManager.h"

// 实现获取基本步骤名称的函数
const char* getBasicStepName(PSMStep step) {
    switch (step) {
        case PSMStep::PowerOn: return "上电";
        case PSMStep::NotInitialized: return "未初始化";
        case PSMStep::Pause: return "暂停";
        case PSMStep::Space: return "空白";
        case PSMStep::BreakPoint: return "断点";
        case PSMStep::Initialize: return "初始化";
        case PSMStep::Initialized: return "已初始化";
        case PSMStep::Stopping: return "停止中";
        case PSMStep::Stopped: return "已停止";
        case PSMStep::Loop: return "循环";
        default: return "未知";
    }
}

// 构造函数实现
ProgramStepManager::ProgramStepManager(const char* periCustomName)
    : BasicIndustrialPeripheral(PeripheralType::ProcessStep),
      run(false),
      step(0),
      lastTick(0),
      periName(periCustomName) {}

// 其他成员函数实现
void ProgramStepManager::stepTo(PSMStep main) {
    stepTo((uint16_t)main);
}

void ProgramStepManager::stepTo(uint16_t main) {
    if (step == main) return;
    uint32_t ms = millis();
    step = main;
    globalPDL.printfln("[%d][%s]流程 %d (+ %d ms)", ms, periName, (uint8_t)main, ms - lastTick);
    lastTick = ms;
}

bool ProgramStepManager::setRun(bool state) {
    run = state;  // 设置运行状态
    return true;
}

bool ProgramStepManager::isRunning() {
    return run;
}

const char* ProgramStepManager::getStepName() {
    return getBasicStepName((PSMStep)step);
}

uint16_t ProgramStepManager::getStepNumber() {
    if (step < (uint16_t)PSMStep::Loop) return step;
    if (!isRunning()) return (uint16_t)(PSMStep::Pause);
    return step;
}

uint16_t& ProgramStepManager::getStepRef() {
    return step;
}

uint16_t& ProgramStepManager::getReadWriteStateFlagRef() {
    return rwState;
}

uint16_t& ProgramStepManager::getReadOnlyStateFlagRef() {
    return rState;
}

void ProgramStepManager::alarmSolutionUpdate() {}