#pragma once
#include "TPSCounter/TPSCounter.h"
#include "STimer/STimer.h"
#include "FIFO/FIFO.h"
#include "GlobalLogger/GlobalLogger.h"
#include "SerialCommandHandler/SerialCommandHandler.h"
#include "MemoryController/MemoryController.h"
#include "AverageFilter/AverageFilter.h"
#include "ProgramStepManager/ProgramStepManager.h"
#ifdef Beao_Industry_Peripheral
#include "Peripheral/AlarmManager.h"
#include "Peripheral/AlarmDefinition.h"
#include "Peripheral/Peripheral.h"
#include "Peripheral/Cylinder.h"
#include "Peripheral/BLDCMotor.h"
#include "Peripheral/StepMotor.h"
#include "Peripheral/EncoderCounter.h"
#endif