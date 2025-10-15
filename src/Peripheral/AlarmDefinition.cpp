#include "AlarmDefinition.h"
const char* getBasicAlarmDetail(uint16_t periType, uint16_t alarm){
  if(alarm == AlarmType::NoAlarm) return "无错误";
  switch(periType){
    case PeripheralType::Cylinder:{
      switch(alarm){
        case AlarmType::CylinderGoMoveButNoMove:
          return "气缸去动点，但动点信号未出现";
        case AlarmType::CylinderGoMoveButHasHome:
          return "气缸去动点，但原点信号未消失";
        case AlarmType::CylinderGoHomeButNoHome:
          return "气缸去原点，但原点信号未出现";
        case AlarmType::CylinderGoHomeButHasMove:
          return "气缸去原点，但动点信号未消失";
      }
    }
    break;
    case PeripheralType::VacuumValve:{
      switch(alarm){
        case AlarmType::VacuumValveSuckTimedOut:
          return "真空吸取超时";
        case AlarmType::VacuumValveReleaseTimedOut:
          return "真空释放超时";
        case AlarmType::VacuumValveSuckDropped:
          return "真空吸取掉落";
      }
    }
    break;
    case PeripheralType::BLDCMotor:
    case PeripheralType::StepMotor:
    case PeripheralType::ServoMotor:{
      switch(alarm){
        case AlarmType::MotorHardwareAlarm:
          return "电机硬件报警";
        case AlarmType::MotorSoftwareAlarm:
          return "电机软件报警";
      }
    }
    break;
    case PeripheralType::EncoderCounter:{
      switch(alarm){
        case AlarmType::EncoderHardwareAlarm:
          return "编码器硬件故障";
        case AlarmType::EncoderHardwareAPhaseAlarm:
          return "编码器硬件A相故障";
        case AlarmType::EncoderHardwareBPhaseAlarm:
          return "编码器硬件B相故障";
     }
    }
    break;
    case PeripheralType::Communication:{
      switch(alarm){
        case AlarmType::CommunicationNoSignal:
          return "通信无信号故障";
      }
    }
    break;
  }
  return "自定义故障";
}