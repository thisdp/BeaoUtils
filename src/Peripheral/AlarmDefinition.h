#pragma once
#include "PeripheralType.h"
#include <stdint.h>

namespace AlarmType{
    enum{
      NoAlarm = 0
    };
    enum{  //气缸
      CylinderGoMoveButNoMove = 1,	  //气缸去动点但动点信号未出现
      CylinderGoMoveButHasHome = 2,		//气缸去动点但原点信号未消失
      CylinderGoHomeButNoHome = 3,		//气缸去原点但原点信号未出现
      CylinderGoHomeButHasMove = 4,		//气缸去原点但动点信号未消失
      ManualAutoStateNoMatch = 5,		  //手自动状态不匹配
    };
    enum{ //通信
      CommunicationNoSignal = 1,  //通信故障
    };
    enum{
      VacuumValveSuckTimedOut	= 1,		  //真空吸取超时
      VacuumValveReleaseTimedOut = 2,		//真空释放超时
      VacuumValveSuckDropped = 3,				//真空吸取掉落
    };
    enum{
      MotorHardwareAlarm = 1,			//通用电机硬件报警
      MotorSoftwareAlarm = 2,			//通用电机软件报警
    };
    enum{
      EncoderHardwareAlarm = 1,			  //编码器硬件故障
      EncoderHardwareAPhaseAlarm = 2,	//编码器硬件A相故障
      EncoderHardwareBPhaseAlarm = 3,	//编码器硬件B相故障
    };
};

static const char* getBasicAlarmDetail(uint16_t periType, uint16_t alarm){
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

namespace AlarmSolutionStep{
	constexpr uint16_t Idle = 0;
	constexpr uint16_t RequestSolve = 1;
};

namespace AlarmSolutionType{
  constexpr uint8_t NoSolution = 0; //直接报警
  constexpr uint8_t AutoSolve = 1;  //自动解决
  constexpr uint8_t ProgramSolve = 2; //程序解决
};