#pragma once
#include <Arduino.h>
#include "../Logger/Logger.h"
#include "PeripheralType.h"
#include "../AlarmManager/AlarmDefinition.h"

uint16_t globalPeripheralCount;
Logger globalPDL;  //global Peripheral Debug Logger

class BasicIndustrialPeripheral;
typedef void(*BasicIndustrialPeripheralCallBack)(BasicIndustrialPeripheral *peri);
typedef void(*IODigitalWrite)(uint8_t pin, uint8_t state);
typedef int(*IODigitalRead)(uint8_t pin);

IODigitalWrite gDigitalWrite = digitalWrite;  //默认使用的digitalWrite
IODigitalRead gDigitalRead = digitalRead;     //默认使用的digitalRead

class BasicIndustrialPeripheral{
protected:
  uint16_t periID;       //ID
  uint16_t periType;     //类型
  uint16_t alarmState;   //报警
  bool updateNotify;     //更新通知
  bool debugOn;          //通用debug模式开关
  const char* periName;  //名称
  IODigitalWrite ioWrite = gDigitalWrite;
  IODigitalRead ioRead = gDigitalRead;
  void onInternalAlarm(){
    globalPDL.printf("[报警][ID:%d][类型:%d]%s: %s",periID,periType,periName,getAlarmDetail(alarmState));
    if(onAlarm) onAlarm(this);
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

  static void setGlobalPeripheralDebugStream(Stream &stream){ //设置全局调试输出流
    globalPDL.setStream(stream);
  };
  BasicIndustrialPeripheral(uint16_t type) :
    periID(globalPeripheralCount++),
    periType(type),
    alarmState(false),
    periName("基础外设"),
    updateNotify(false)
  {
    ioWrite = gDigitalWrite;
    ioRead = gDigitalRead;
  }
  const char* getName(){ return periName; }
  uint16_t getID(){ return periID; }
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
  static const char* getAlarmDetail(uint16_t alarm){
    switch(alarm){
      case AlarmType::NoAlarm:
        return "无错误";
      case AlarmType::CylinderGoMoveButNoMove:
        return "气缸去动点，但动点信号未出现";
      case AlarmType::CylinderGoMoveButHasHome:
        return "气缸去动点，但原点信号未消失";
      case AlarmType::CylinderGoHomeButNoHome:
        return "气缸去原点，但原点信号未出现";
      case AlarmType::CylinderGoHomeButHasMove:
        return "气缸去原点，但动点信号未消失";
      case AlarmType::VacuumValveSuckTimedOut:
        return "真空吸取超时";
      case AlarmType::VacuumValveReleaseTimedOut:
        return "真空释放超时";
      case AlarmType::VacuumValveSuckDropped:
        return "真空吸取掉落";
      case AlarmType::MotorHardwareAlarm:
        return "电机硬件报警";
      case AlarmType::MotorSoftwareAlarm:
        return "电机软件报警";
      case AlarmType::EncoderHardwareAlarm:
        return "编码器硬件故障";
      case AlarmType::EncoderHardwareAPhaseAlarm:
        return "编码器硬件A相故障";
      case AlarmType::EncoderHardwareBPhaseAlarm:
        return "编码器硬件B相故障";
      default:
        return "未知故障";
    }
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