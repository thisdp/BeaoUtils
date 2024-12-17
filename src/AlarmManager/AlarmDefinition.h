#pragma once
#include <stdint.h>

namespace AlarmType{
    enum{
		NoAlarm = 0,
		ManualAutoStateNoMatch = 10,		//手自动状态不匹配
			
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

namespace AlarmSolutionStep{
	constexpr uint16_t Idle = 0;
	constexpr uint16_t Alarm = 1;
	constexpr uint16_t RequestSolve = 2;
};