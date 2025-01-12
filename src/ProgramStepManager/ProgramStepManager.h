#pragma once
#include <Arduino.h>
#include "../Logger/Logger.h"
#include "../Peripheral/AlarmDefinition.h"
#include "../Peripheral/Peripheral.h"

/*
	PowerOn = 0,	//0号流程上电
    NotInitialized, //上电后未初始化
	Pause,			//暂停流程
	Space,			//空白流程，用于分割
	BreakPoint,		//断点流程，用于暂停程序
    
	Initialize,
    Initialized,
	Stopping,
	Stopped,
	Loop = 20,
*/
#define PSMDefaultSteps \
    PowerOn = 0,\
    NotInitialized,\
	Pause,\
	Space,\
	BreakPoint,\
	Initialize,\
    Initialized,\
	Stopping,\
	Stopped,\
	Loop = 20

enum class PSMStep : uint16_t{
    PSMDefaultSteps,
};

const char *getBasicStepName(PSMStep step){
    switch(step){
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

class ProgramStepManager : public BasicIndustrialPeripheral{
public:
    //状态位
    union{
        struct {
            uint16_t alarmReset:1;    //复位报警
            uint16_t reserved1:15;    //保留
        };
        uint16_t rwState;              //外部可读写状态
    };
    union{
        struct {
            uint16_t run:1;           //运行中
            uint16_t reserved:15;
        };
        uint16_t rState;  //外部只读状态
    };
    uint16_t step;
    uint32_t lastTick;
public:
  uint16_t &getReadWriteStateFlagRef(){
    return rwState;
  }
  uint16_t &getReadOnlyStateFlagRef(){
    return rState;
  }
    ProgramStepManager(const char* periCustomName) : 
        BasicIndustrialPeripheral(PeripheralType::ProcessStep),
        lastTick(0),
        run(false),
        step(0)
    {
        periName = periCustomName;
    }
    void stepTo(PSMStep main){
        stepTo((uint16_t)main);
    }
    void stepTo(uint16_t main){
        if(step == main) return;
        uint32_t ms = millis();
        step = main;
        globalPDL.printfln("[%d][%s]流程 %d (+ %d ms)",ms,periName,(uint8_t)main,ms-lastTick);
        lastTick = ms;
    }
    bool setRun(bool state){
        run = state;  //设置运行状态
        return true;
    }
    bool isRunning(){
        return run;
    }
    const char* getStepName(){
        return getBasicStepName((PSMStep)step);
    }
    uint16_t getStepNumber(){
        if(step < (uint16_t)PSMStep::Loop) return step;
        if(!isRunning()) return (uint16_t)(PSMStep::Pause);
        return step;
    }
    uint16_t &getStepRef(){
        return step;
    }
    void alarmSolutionUpdate(){}
};