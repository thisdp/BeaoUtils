#pragma once
#include <Arduino.h>
namespace PeripheralType{
    constexpr uint16_t BasicPeripheral = 0; //基础外设
    constexpr uint16_t Cylinder = 1;        //气缸
    constexpr uint16_t VacuumValve = 2;     //真空阀
    constexpr uint16_t BLDCMotor = 3;       //直流无刷电机
    constexpr uint16_t StepMotor = 4;       //步进电机
    constexpr uint16_t ServoMotor = 5;      //伺服电机
    constexpr uint16_t EncoderCounter = 6;  //编码器
    constexpr uint16_t ProcessStep = 20;    //流程步管理器
    constexpr uint16_t Communication = 60;  //通信外设
};