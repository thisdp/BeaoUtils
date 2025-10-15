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

const char* getBasicAlarmDetail(uint16_t periType, uint16_t alarm);

namespace AlarmSolutionStep{
	constexpr uint16_t Idle = 0;
	constexpr uint16_t RequestSolve = 1;
};

namespace AlarmSolutionType{
  constexpr uint8_t NoSolution = 0; //直接报警
  constexpr uint8_t AutoSolve = 1;  //自动解决
  constexpr uint8_t ProgramSolve = 2; //程序解决
};