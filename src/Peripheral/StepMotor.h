#pragma once
#include "Peripheral.h"
#include "../STimer/STimer.h"

class StepMotor : public BasicIndustrialPeripheral{
protected:
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
      uint16_t enable:1;        //使能
      uint16_t direction:1;     //方向
      uint16_t reserved2:13;    //保留
    };
    uint16_t rState;  //外部只读状态
  };

  int8_t pinEnable;
  int8_t pinDirection;
  int8_t pinPulse;
  int8_t pinAlarm;
  volatile int32_t stepAccumlator;

  bool busyState;
  bool pulseState;
  uint8_t edgeMode;

public:
  static constexpr uint8_t PeriType = PeripheralType::StepMotor;

  static constexpr bool DirCW = false;  //顺时针转
  static constexpr bool DirCCW = true;  //逆时针转

  static constexpr uint8_t EdgeMode_Fall = 0;
  static constexpr uint8_t EdgeMode_Rise = 1;
  static constexpr uint8_t EdgeMode_Both = 2;
//报警解决器
  static constexpr uint16_t ASS_Disable = 10;
  static constexpr uint16_t ASS_WaitDisable = 11;
  static constexpr uint16_t ASS_Enable = 12;
  static constexpr uint16_t ASS_WaitEnable = 13;
  static constexpr uint16_t ASS_WaitAlarmRecover = 14;
  MSTimer alarmSolutionTimer;
  uint16_t &getReadWriteStateFlagRef(){
    return rwState;
  }
  uint16_t &getReadOnlyStateFlagRef(){
    return rState;
  }
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
      edgeMode(EdgeMode_Rise),
      alarmSolutionTimer(2000)
  {
    periName = periCustomName;
    ioWrite = gDigitalWrite;
    ioRead = gDigitalRead;
    setEnabled(false);
    setDirection(false);
  }
  bool setRun(bool state){
    run = state;
    return true;
  }
  bool isRunning(){ return run; }
  void setEnabled(bool enState){
    enable = enState;
    ioWrite(pinEnable,enState);  //高电平有效
    resetPulseState();
  }
  bool isEnabled(){
    return enable;
  }
  void setDirection(bool dir){
    direction = dir;
    ioWrite(pinDirection,dir);
  }
  bool getDirection(){
    return direction;
  }
  bool isBusy(){
    return busyState;
  }
  void setBusy(bool state){
    busyState = state;
  }
  inline bool flipPulse(){
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
  bool hasHardwareAlarm(){
    return pinAlarm != -1 ? ioRead(pinAlarm) : false;
  }
  void alarmSolutionUpdate(){
    bool alarm = hasHardwareAlarm();
    if(alarmSolutionStep == AlarmSolutionStep::Idle){ //如果报警解决器空闲
      if(alarm && getAlarm() == AlarmType::NoAlarm){  //第一次遇到报警
        globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 遇到报警，使用%d方案",periID,periType,periName,alarmSolutionActionType);
        if(alarmSolutionActionType == AlarmSolutionType::NoSolution){ //如果未设定则直接报警
          setAlarm(AlarmType::MotorHardwareAlarm);  //设置报警
        }else if(alarmSolutionActionType == AlarmSolutionType::AutoSolve){  //尝试自动解决
          alarmTryToSolve();  //尝试解决
          globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 自动尝试解决",periID,periType,periName);
        }else if(alarmSolutionActionType == AlarmSolutionType::ProgramSolve){
          globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 外部处理",periID,periType,periName);
          // 程序解决，不处理报警
        }
      }
    }
    switch(alarmSolutionStep){
      case AlarmSolutionStep::RequestSolve:
        alarmSolutionStep = StepMotor::ASS_Disable;
        globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 开始解决报警",periID,periType,periName);
      break;
      case StepMotor::ASS_Disable:
        alarmSolutionTimer.start();
        setEnabled(false);
        alarmSolutionStep = StepMotor::ASS_WaitDisable;
        globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 关闭使能",periID,periType,periName);
      break;
      case StepMotor::ASS_WaitDisable:
        if(alarmSolutionTimer.checkTimedOut()){
          alarmSolutionStep = StepMotor::ASS_Enable;
          globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 延时",periID,periType,periName);
        }
      break;
      case StepMotor::ASS_Enable:
        alarmSolutionTimer.start();
        setEnabled(true);
        alarmSolutionStep = StepMotor::ASS_WaitEnable;
        globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 打开使能",periID,periType,periName);
      break;
      case StepMotor::ASS_WaitEnable:
        if(alarmSolutionTimer.checkTimedOut()){
          alarmSolutionStep = StepMotor::ASS_WaitAlarmRecover;
          globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 等待恢复",periID,periType,periName);
        }
      break;
      case StepMotor::ASS_WaitAlarmRecover:
        alarmSolutionStep = AlarmSolutionStep::Idle;
        if(alarm){  //无法消除报警
          setAlarm(AlarmType::MotorHardwareAlarm);  //设置报警
          globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 报警未消除，上报",periID,periType,periName);
        }else{
          if(getAlarm() != AlarmType::NoAlarm) resetAlarm(); //报警已经消除
          globalPDL.printfln("[报警解决器][ID:%d][类型:%d]%s: 报警已消除",periID,periType,periName);
        }
      break;
    }
  }
  void update(){
    if(alarmReset){ //尝试复位报警
      alarmReset = false;
      resetAlarm();
      alarmTryToSolve();
    }
    alarmSolutionUpdate();
  }
};