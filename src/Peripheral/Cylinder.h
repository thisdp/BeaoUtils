#pragma once
#include "Peripheral.h"
#include "../STimer/STimer.h"

class Cylinder : public BasicIndustrialPeripheral{
protected:
  //状态位
  union{
    struct {
      uint16_t alarmReset:1;    //复位报警
      uint16_t manualState:1;   //手动状态
      uint16_t reserved1:14;    //保留
    };
    uint16_t rwState;              //外部可读写状态
  };
  union{
    struct {
      uint16_t run:1;           //运行中
      uint16_t currentState:1;  //当前状态
      uint16_t forceManual:1;   //强制手动
      uint16_t autoState:1;     //自动状态
      uint16_t atHome:1;        //气缸在原点
      uint16_t atMove:1;        //气缸在动点
      uint16_t reserved2:10;      //保留
    };
    uint16_t rState;  //外部只读状态
  };

  bool lastState;     //上个状态
  bool manualStateBefore; //用于同步

  //uint16_t goHomeDelay;     //去原点延时
  //uint16_t goMoveDelay;     //去动点延时
  //uint16_t goHomeTimeOut;   //去原点超时
  //uint16_t goMoveTimeOut;   //去动点超时

  int8_t pinCylinder; //输出
  int8_t pinHome;     //输入，原点
  int8_t pinMove;     //输入，动点
  bool homeState;
  bool moveState;
  //定时器
  MSTimer goHomeDelayTimer;
  MSTimer goMoveDelayTimer;
  MSTimer goHomeTimeOutTimer;
  MSTimer goMoveTimeOutTimer;

  bool forceSuccessGoHomeTimeout; //去原点超时强制成功
  bool forceSuccessGoMoveTimeout; //去动点超时强制成功


public:
  uint16_t &getReadWriteStateFlagRef(){
    return rwState;
  }
  uint16_t &getReadOnlyStateFlagRef(){
    return rState;
  }
  static constexpr uint8_t PeriType = PeripheralType::Cylinder;
  Cylinder(
      const char *periCustomName,
      int8_t execPin,
      int8_t inputHome = -1, int8_t inputMove = -1,
      uint32_t timeGoHomeDelay = 0, uint32_t timeGoMoveDelay = 0,
      uint32_t timeGoHomeTimeOut = 3000, uint32_t timeGoMoveTimeOut = 3000,
      bool forceSuccessWhenGoHomeTimeout = false, bool forceSuccessWhenGoMoveTimeout = false
    ) :
      BasicIndustrialPeripheral(PeriType),

      alarmReset(false),
      manualState(false),
      reserved1(0),

      run(false),
      currentState(false),
      forceManual(false),
      autoState(false),
      atHome(false),
      atMove(false),
      reserved2(0),

      lastState(false),
      manualStateBefore(false),
      
      pinCylinder(execPin),
      pinHome(inputHome),
      pinMove(inputMove),

      homeState(false),
      moveState(false),

      goHomeDelayTimer(timeGoHomeDelay),
      goMoveDelayTimer(timeGoMoveDelay),
      goHomeTimeOutTimer(timeGoHomeTimeOut),
      goMoveTimeOutTimer(timeGoMoveTimeOut),
      forceSuccessGoHomeTimeout(forceSuccessWhenGoHomeTimeout),
      forceSuccessGoMoveTimeout(forceSuccessWhenGoMoveTimeout)
  {
    periName = periCustomName;
    setNotify(true);
  }
  bool getManualState(){
    return manualState;
  }
  bool getAutoState(){
    return autoState;
  }
  bool getState(){  //最终状态
    if(forceManual){  //如果强制手动
      return manualState; //返回手动状态
    }else{
      return autoState; //否则返回自动状态
    }
  }
  bool setManualState(bool state){
    if(run) return false; //如果是自动模式，则设置手动无效
    manualState = state;
    manualStateBefore = state;
    forceManual = true;
    setNotify(true);
    return true;
  }
  void setAutoState(bool state){ 
    autoState = state;
    if(run) manualState = state;  //如果正在运行 则同步手动状态
    setNotify(true);
  }
  bool setRun(bool state){
    run = state;  //设置运行状态
    if(forceManual){  //如果强制手动
      if(manualState != autoState){ //查看状态是否匹配
        setAlarm(AlarmType::ManualAutoStateNoMatch);  //不匹配就报警
        return false;
      }else{  //如果匹配
        forceManual = false;  //复位强制手动
        setAutoState(autoState);  //重新设置一次自动状态
      }
    }
    return true;
  }
  bool isRunning(){ return run; }
  void setForceSuccessGoHomeTimeout(bool state){ forceSuccessGoHomeTimeout = state; }
  void setForceSuccessGoMoveTimeout(bool state){ forceSuccessGoMoveTimeout = state; }
  inline void goHome(){ setAutoState(false); }
  inline void goMove(){ setAutoState(true); }
  inline bool isAtHome(){ return atHome; }
  inline bool isAtMove(){ return atMove; }
  inline bool isAtHomePhysically(){
    return !currentState && (homeState);
  }
  inline bool isAtMovePhysically(){
    return currentState && (moveState);
  }
  void stopAllTimers(){
    goMoveTimeOutTimer.stop();
    goHomeTimeOutTimer.stop();
    goMoveDelayTimer.stop();
    goHomeDelayTimer.stop();
  }
  void alarmSolutionUpdate(){}
  void update(){
    if(alarmReset){ //尝试复位报警
      alarmReset = false;
      resetAlarm();
    }
    if(getAlarm() == AlarmType::NoAlarm){ //如果没有报警，则进行判定
      homeState = pinHome != -1 ? ioRead(pinHome) : !currentState;  //如果没有原点，强制到达
      moveState = pinMove != -1 ? ioRead(pinMove) : currentState;  //如果没有动点, 强制到达
      currentState = getState();
      if(currentState != lastState){
        setNotify(true);
        ioWrite(pinCylinder, currentState);
        lastState = currentState;
      }
      if(currentState){ //去动点
        if(homeState || (!moveState && !(forceSuccessGoMoveTimeout && atMove))){  //如果仍然在原点，或者没有去动点（如果打开了强制到达动点，并且已经到达动点，则无视moveState）
          atMove = false;
          goMoveTimeOutTimer.startIfNotActivated();
          if(goMoveTimeOutTimer.checkTimedOut()){ //如果超时
            goMoveTimeOutTimer.stop();
            if(homeState && !forceSuccessGoMoveTimeout){  //如果在原点，并且没有启用强制到达
              setAlarm(AlarmType::CylinderGoMoveButHasHome);  //报警
            }else if(!moveState){ //如果没有到达动点
              if(forceSuccessGoMoveTimeout){ //如果启用强制到达
                atMove = true;  //强制到达动点
                stopAllTimers();
              }else{
                setAlarm(AlarmType::CylinderGoMoveButNoMove); //报警
              }
            }
          }
        }
        if(!atMove){  //如果未在动点
          if(!homeState && moveState){  //进行动点到达判定
            goMoveTimeOutTimer.stop();
            goMoveDelayTimer.startIfNotActivated();
            if(goMoveDelayTimer.checkTimedOut()){
              goMoveDelayTimer.stop();
              atMove = true;  //到达动点
              stopAllTimers();
              setNotify(true);
            }
          }
        }
      }else{  //去原点
        if(moveState || (!homeState && !(forceSuccessGoHomeTimeout && atHome))){  //如果仍然在动点，或者没有去原点（如果打开了强制到达原点，并且已经到达原点，则无视homeState）
          atHome = false;
          goHomeTimeOutTimer.startIfNotActivated();
          if(goHomeTimeOutTimer.checkTimedOut()){ //如果超时
            goHomeTimeOutTimer.stop();
            if(moveState && !forceSuccessGoHomeTimeout){  //如果在动点，并且没有启用强制到达
              setAlarm(AlarmType::CylinderGoHomeButHasMove);  //报警
            }else if(!homeState){ //如果没有到达原点
              if(forceSuccessGoHomeTimeout){ //如果启用强制到达
                atHome = true;  //强制到达原点
                stopAllTimers();
              }else{
                setAlarm(AlarmType::CylinderGoHomeButNoHome); //报警
              }
            }
          }
        }
        if(!atHome){  //如果未在原点
          if(!moveState && homeState){  //进行原点到达判定
            goHomeTimeOutTimer.stop();
            goHomeDelayTimer.startIfNotActivated();
            if(goHomeDelayTimer.checkTimedOut()){
              goHomeDelayTimer.stop();
              atHome = true;  //到达原点
              stopAllTimers();
              setNotify(true);
            }
          }
        }
      }
      if(!run){
        if(manualState != manualStateBefore){
          setManualState(manualState);
        }
      }
      manualStateBefore = manualState;
    }
  }
};