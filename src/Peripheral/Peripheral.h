#pragma once
#include <Arduino.h>
#include "../Logger/Logger.h"
#include "PeripheralType.h"
#include "AlarmDefinition.h"

uint16_t globalPeripheralCount;
Logger globalPDL;  //global Peripheral Debug Logger

class BasicIndustrialPeripheral;
typedef void(*BasicIndustrialPeripheralCallBack)(BasicIndustrialPeripheral *peri, uint16_t oldAlarm);
#if defined(ESP32)
typedef void(*IODigitalWrite)(uint8_t pin, uint8_t state);
typedef int(*IODigitalRead)(uint8_t pin);
#else
typedef void(*IODigitalWrite)(uint32_t pin, uint32_t state);
typedef int(*IODigitalRead)(uint32_t pin);
#endif
IODigitalWrite gDigitalWrite = digitalWrite;  //默认使用的digitalWrite
IODigitalRead gDigitalRead = digitalRead;     //默认使用的digitalRead

class BasicIndustrialPeripheral{
protected:
  uint16_t periID;       //ID
  uint16_t periType;     //类型
  uint16_t alarmState;   //报警
  const char* periName;  //名称
  IODigitalWrite ioWrite;
  IODigitalRead ioRead;
  bool updateNotify;     //更新通知
  bool debugOn;          //通用debug模式开关
  void onInternalAlarm(uint16_t oldAlarm){
    if(alarmState == AlarmType::NoAlarm){
      onAlarmReset();
      return;
    }
    globalPDL.printfln("[报警][ID:%d][类型:%d]%s: %s",periID,periType,periName,getAlarmDetail(alarmState));
    if(onAlarm) onAlarm(this,oldAlarm);
  }
public:
  static bool useDigitalWrite(IODigitalWrite hookedDigitalWrite){
    gDigitalWrite = hookedDigitalWrite;
    return true;
  }
  static bool useDigitalRead(IODigitalRead hookedDigitalRead){
    gDigitalRead = hookedDigitalRead;
    return true;
  }

  static bool useDigitalIO(IODigitalRead hookedDigitalRead,IODigitalWrite hookedDigitalWrite){
    gDigitalWrite = hookedDigitalWrite;
    gDigitalRead = hookedDigitalRead;
    return true;
  }

  static bool setGlobalPeripheralDebugStream(Stream &stream){ //设置全局调试输出流
    globalPDL.setStream(stream);
    return true;
  };
  BasicIndustrialPeripheral(uint16_t type) :
    periID(globalPeripheralCount++),
    periType(type),
    alarmState(false),
    periName("基础外设"),
    updateNotify(false),
    ioWrite(gDigitalWrite),
    ioRead(gDigitalRead)
  {}
  uint16_t &getAlarmRef(){ return alarmState; }
  const char* getName(){ return periName; }
  uint16_t getID(){ return periID; }
  uint16_t getAlarm(){ return alarmState; }
  void setAlarm(uint16_t alarm){
    if(alarmState != alarm){
      uint16_t oldAlarm = alarmState;
      alarmState = alarm;
      onInternalAlarm(oldAlarm);
    }
    setNotify(true);
  }
  void resetAlarm() { setAlarm(AlarmType::NoAlarm); }
  virtual void onAlarmReset(){ }
  void setNotify(bool state){
    updateNotify = state;
  }
  bool getNotify(){
    return updateNotify;
  }
  const char* getAlarmDetail(uint16_t alarm){
    return getBasicAlarmDetail(periType,alarm);
  }
  //回调函数
  BasicIndustrialPeripheralCallBack onAlarm;

  void setDebugEnabled(bool state){
    debugOn = state;
  }
  bool isDebugEnabled(){
    return debugOn;
  }
  void toggleDebugMode(){
    debugOn = !debugOn;
  }
  virtual bool setRun(bool state) = 0;
};