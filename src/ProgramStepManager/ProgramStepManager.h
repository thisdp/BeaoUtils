#pragma once
#include <Arduino.h>
#include "../Logger/Logger.h"
#include "../AlarmManager/AlarmDefinition.h"

Logger globalPSDL;  //global Program Step Debug Logger

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

class ProgramStepManager{
public:
    const char *name;
    uint16_t run;
    uint16_t step;
    uint16_t alarmState;
    bool updateNotify;     //更新通知
    bool debugOn;          //通用debug模式开关
    uint32_t lastTick;
    ProgramStepManager(const char* n) : 
        name(name),
        lastTick(0),
        run(false),
        alarmState(false),
        updateNotify(false)
    {
        stepTo(PSMStep::Stopped);
    }
    void setRun(bool state){
        run = state;
    }
    bool isRunning(){
        return run != 0;
    }
    uint16_t getAlarm(){ return alarmState; }
    void setAlarm(uint16_t alarm){
        alarmState = alarm;
        setNotify(true);
    }
    void resetAlarm() { setAlarm(AlarmType::NoAlarm); }
    void setNotify(bool state){
        updateNotify = state;
    }
    bool getNotify(){
        return updateNotify;
    }
    void stepTo(PSMStep main){
        stepTo((uint16_t)main);
    }
    void stepTo(uint16_t main){
        uint32_t ms = millis();
        step = main;
        globalPSDL.printfln("[%d][%s] State -> %d (%d ms)",ms,name,(uint8_t)main,ms-lastTick);
        lastTick = ms;
    }
    uint16_t getStepNumber(){
        if(step < (uint16_t)PSMStep::Loop) return step;
        if(!isRunning()) return (uint16_t)(PSMStep::Pause);
        return step;
    }
};