#pragma once
#include <stdint.h>

namespace AlarmType{
    enum{
		NoAlarm = 0,
		ManualAutoStateNoMatch = 10,		//手自动状态不匹配
    CommunicationError = 15,  //通信故障
			
		CylinderGoMoveButNoMove = 20,	//气缸去动点但动点信号未出现
		CylinderGoMoveButHasHome,		//气缸去动点但原点信号未消失
		CylinderGoHomeButNoHome,		//气缸去原点但原点信号未出现
		CylinderGoHomeButHasMove,		//气缸去原点但动点信号未消失
			
		VacuumValveSuckTimedOut	= 30,		//真空吸取超时
		VacuumValveReleaseTimedOut,			//真空释放超时
		VacuumValveSuckDropped,				//真空吸取掉落
			
		MotorHardwareAlarm = 40,			//通用电机硬件报警
		MotorSoftwareAlarm = 41,			//通用电机软件报警
		
		EncoderHardwareAlarm = 45,			//编码器硬件故障
		EncoderHardwareAPhaseAlarm = 46,	//编码器硬件A相故障
		EncoderHardwareBPhaseAlarm = 47,	//编码器硬件B相故障


    };
};

static const char* getBasicAlarmDetail(uint16_t alarm){
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
      case AlarmType::CommunicationError:
        return "通信故障";
      default:
        return "未知故障";
    }
  }

namespace AlarmSolutionStep{
	constexpr uint16_t Idle = 0;
	constexpr uint16_t Alarm = 1;
	constexpr uint16_t RequestSolve = 2;
};