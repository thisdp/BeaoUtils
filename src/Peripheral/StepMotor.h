#pragma once
#include "Peripheral.h"
#include "../STimer/STimer.h"

class StepMotor : public BasicIndustrialPeripheral{
protected:
  //状态位
  union{
    struct {
      uint16_t run:1;
      uint16_t alarmReset:1;    //复位报警
      uint16_t enable:1;
      uint16_t direction:1;
      uint16_t reserved:12;     //保留
    };
    uint16_t state;              // 通过这个变量访问整个状态
  };

  int8_t pinEnable;
  int8_t pinDirection;
  int8_t pinPulse;
  int8_t pinAlarm;
  int32_t stepAccumlator;

  bool pulseState;
  uint8_t edgeMode;
  uint8_t alarmStep;

public:
  static constexpr uint8_t PeriType = PeripheralType::StepMotor;

  static constexpr bool DirCW = false;  //顺时针转
  static constexpr bool DirCCW = true;  //逆时针转

  static constexpr uint8_t EdgeMode_Fall = 0;
  static constexpr uint8_t EdgeMode_Rise = 1;
  static constexpr uint8_t EdgeMode_Both = 2;
//报警解决器
  static constexpr uint16_t ASS_Disable = 3;
  static constexpr uint16_t ASS_WaitDisable = 4;
  static constexpr uint16_t ASS_Enable = 5;
  static constexpr uint16_t ASS_WaitEnable = 6;
  static constexpr uint16_t ASS_WaitAlarmRecover = 7;
  MSTimer alarmSolutionTimer;
  StepMotor(
      const char *periCustomName,
      int8_t pEnable = -1,
      int8_t pDirection = -1,
      int8_t pPulse = -1,
      int8_t pAlarm = -1
    ) :
      BasicIndustrialPeripheral(PeriType),
      pinEnable(pEnable),
      pinDirection(pDirection),
      pinPulse(pPulse),
      pinAlarm(pAlarm),
      pulseState(false),
      stepAccumlator(0),
      alarmStep(AlarmSolutionStep::Idle),
      edgeMode(EdgeMode_Rise),
      alarmSolutionTimer(2000)
  {
    periName = periCustomName;
    ioWrite = gDigitalWrite;
    ioRead = gDigitalRead;
    ioWrite(pinEnable,LOW);
    ioWrite(pinDirection,LOW);
  }
  bool setRun(bool state){
    run = state;
    return true;
  }
  void setEnabled(bool enState){
    enable = enState;
    ioWrite(pinEnable,!enState);  //低电平有效
    resetPulseState();
  }
  void setDirection(bool dir){
    direction = dir;
    ioWrite(pinDirection,dir);
  }
  inline bool flipPulse(){
    if(!run) return false;
    pulseState = !pulseState;
    ioWrite(pinPulse,pulseState);
    if(edgeMode == EdgeMode_Both){
      stepAccumlator = direction ? stepAccumlator+1 : stepAccumlator-1;
    }else{
      if(pulseState == edgeMode) stepAccumlator = direction ? stepAccumlator+1 : stepAccumlator-1;
    }
    return true;
  }
  inline bool getPulseState(){
    return pulseState;
  }
  inline bool getEdgeMode(){
    return edgeMode;
  }
  void resetStepAccumlator(){
    stepAccumlator = 0;
  }
  int32_t getStepAccumlator(){
    return stepAccumlator;
  }
  void resetPulseState(){
    if(edgeMode == EdgeMode_Both) return;
    pulseState = !edgeMode; //复位脉冲电平状态
  }
  bool getEnabled(){
    return enable;
  }
  bool getDirection(){
    return direction;
  }
  bool hasHardwareAlarm(){
    return pinAlarm != -1 ? ioRead(pinAlarm) : false;
  }
  bool isWaitingSolveAlarm(){
    return alarmStep == AlarmSolutionStep::Alarm;
  }
  bool isSolvingAlarm(){
    return alarmStep > AlarmSolutionStep::Alarm;
  }
  bool isAlarmSolutionIdle(){
    return alarmStep == AlarmSolutionStep::Idle;
  }
  void trySolveAlarm(){
    if(alarmStep != AlarmSolutionStep::Idle){
      alarmStep = AlarmSolutionStep::RequestSolve;
    }
  }
  void update(){
    if(alarmReset){ //尝试复位报警
      alarmReset = false;
      resetAlarm();
      trySolveAlarm();
    }
    bool alarm = hasHardwareAlarm();
    if(alarmStep == AlarmSolutionStep::Idle){ //报警解决器空闲
      if(alarm && getAlarm() != AlarmType::NoAlarm){  //第一次遇到报警
        alarmStep = AlarmSolutionStep::Alarm; //出现报警
      }
    }else{ //如果alarmStep表明非空闲，则尝试解决报警
      switch(alarmStep){
        case AlarmSolutionStep::RequestSolve:
          alarmStep = StepMotor::ASS_Disable;
        break;
        case StepMotor::ASS_Disable:
          alarmSolutionTimer.start();
          setEnabled(false);
          alarmStep = StepMotor::ASS_WaitDisable;
        break;
        case StepMotor::ASS_WaitDisable:
          if(alarmSolutionTimer.checkTimedOut()){
            alarmStep = StepMotor::ASS_Enable;
          }
        break;
        case StepMotor::ASS_Enable:
          alarmSolutionTimer.start();
          setEnabled(true);
          alarmStep = StepMotor::ASS_WaitEnable;
        break;
        case StepMotor::ASS_WaitEnable:
          if(alarmSolutionTimer.checkTimedOut()){
            alarmStep = StepMotor::ASS_WaitAlarmRecover;
          }
        break;
        case StepMotor::ASS_WaitAlarmRecover:
          alarmStep = AlarmSolutionStep::Idle;
          if(alarm){  //无法消除报警
            setAlarm(AlarmType::MotorHardwareAlarm);  //设置报警
          }else{
            if(getAlarm() != AlarmType::NoAlarm) resetAlarm(); //报警已经消除
          }
        break;
      }
    }
  }
};