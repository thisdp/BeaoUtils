#pragma once
#include "Peripheral.h"
#include "../STimer/STimer.h"

class BLDCMotor : public BasicIndustrialPeripheral{
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

  IODigitalWrite ioWrite;  //默认使用的digitalWrite
  IODigitalRead ioRead;   //默认使用的digitalRead

  
  int8_t pinEnable;
  int8_t pinDirection;
  int8_t pinSV;
  int8_t pinSpeed;
  int8_t pinAlarm;

  uint8_t alarmStep;


public:
  uint16_t &getStateFlagRef(){
    return state;
  }
  static constexpr uint8_t PeriType = PeripheralType::BLDCMotor;

  static constexpr bool DirCW = false;  //顺时针转
  static constexpr bool DirCCW = true;  //逆时针转
//报警解决器
  static constexpr uint16_t ASS_Disable = 3;
  static constexpr uint16_t ASS_WaitDisable = 4;
  static constexpr uint16_t ASS_Enable = 5;
  static constexpr uint16_t ASS_WaitEnable = 6;
  static constexpr uint16_t ASS_WaitAlarmRecover = 7;
  MSTimer alarmSolutionTimer;
  BLDCMotor(
      const char *periCustomName,
      int8_t pEnable = -1,
      int8_t pDirection = -1,
      int8_t pSV = -1,
      int8_t pSpeed = -1,
      int8_t pAlarm = -1
    ) :
      BasicIndustrialPeripheral(PeriType),
      pinEnable(pEnable),
      pinDirection(pDirection),
      pinSV(pSV),
      pinSpeed(pSpeed),
      pinAlarm(pAlarm),
      alarmStep(AlarmSolutionStep::Idle),
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
    if(run){  //仅开启的时候才使能
      ioWrite(pinEnable,enable);
    }else{
      ioWrite(pinEnable,LOW);
    }
    return true;
  }
  void setEnabled(bool enState){
    enable = enState;
    ioWrite(pinEnable,enState);
  }
  void setDirection(bool dir){
    direction = dir;
    ioWrite(pinDirection,dir);
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
          alarmStep = BLDCMotor::ASS_Disable;
        break;
        case BLDCMotor::ASS_Disable:
          alarmSolutionTimer.start();
          setEnabled(false);
          alarmStep = BLDCMotor::ASS_WaitDisable;
        break;
        case BLDCMotor::ASS_WaitDisable:
          if(alarmSolutionTimer.checkTimedOut()){
            alarmStep = BLDCMotor::ASS_Enable;
          }
        break;
        case BLDCMotor::ASS_Enable:
          alarmSolutionTimer.start();
          setEnabled(true);
          alarmStep = BLDCMotor::ASS_WaitEnable;
        break;
        case BLDCMotor::ASS_WaitEnable:
          if(alarmSolutionTimer.checkTimedOut()){
            alarmStep = BLDCMotor::ASS_WaitAlarmRecover;
          }
        break;
        case BLDCMotor::ASS_WaitAlarmRecover:
          alarmStep = AlarmSolutionStep::Idle;
          if(alarm){  //无法消除报警
            setAlarm(AlarmType::MotorHardwareAlarm);  //设置报警
          }else{
            if(getAlarm() != AlarmType::NoAlarm) resetAlarm(); //报警已经消除
          }
        break;
      }
    }
    if(run){
      if(pinSpeed != -1){ //转速反馈
        //ioRead(pinSpeed);
      }
    }
  }
};