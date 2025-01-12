#include "Peripheral.h"
#include "../STimer/STimer.h"

class EncoderCounter : public BasicIndustrialPeripheral{
protected:
  //状态位
  union{
    struct {
      uint16_t alarmReset:1;    //复位报警
      uint16_t reserved1:15;    //保留
    };
    uint16_t rwState;              //外部可读写状态
  };
  union{
    struct {
      uint16_t run:1;           //运行中
      uint16_t recording:1;     //记录中
      uint16_t reserved2:14;    //保留
    };
    uint16_t rState;  //外部只读状态
  };

  int8_t pinENA;
  int8_t pinENB;

  uint32_t count[2];
  bool lastState[2];
  
  MSTimer responseTimer;    //反应检测定时器，如果长时间未检测到信号变化，则表明编码器故障
  int8_t encoderTiming;
  uint32_t lastUpdateTick;

public:
  static constexpr uint8_t PeriType = PeripheralType::EncoderCounter;
  static constexpr uint8_t ENA = 0;
  static constexpr uint8_t ENB = 1;
  static constexpr int8_t encoderTimingTore = 5;
  uint16_t &getReadWriteStateFlagRef(){
    return rwState;
  }
  uint16_t &getReadOnlyStateFlagRef(){
    return rState;
  }
  EncoderCounter(const char *periCustomName, uint8_t rA, uint8_t rB) :
    BasicIndustrialPeripheral(PeriType),
    pinENA(rA),
    pinENB(rB),
    encoderTiming(0),
    responseTimer(2000)
  {
    periName = periCustomName,
    clear();
  }
  bool setRun(bool state){
    run = state;
    responseTimer.start();
    return true;
  }
  bool isRunning(){ return run; }
  void start(){
    responseTimer.start();
    clear();
    recording = true;
    lastState[ENA] = ioRead(pinENA);
    lastState[ENB] = ioRead(pinENB);
  }
  
  void stop(){
    recording = false;
  }

  void clear(){
    count[ENA] = 0;
    count[ENB] = 0;
    encoderTiming = 0;
  }
  uint32_t getCount(bool dir){
    return count[dir];
  }
  uint32_t getLastUpdateTick(){
    return lastUpdateTick;
  }
  void alarmSolutionUpdate(){}
  void update(){
    if(alarmReset){
        alarmReset = false;
        resetAlarm();
    }
    if(recording && run){
      bool ENAState = ioRead(pinENA);
      bool ENBState = ioRead(pinENB);

      if(lastState[ENA] != ENAState){
          encoderTiming = encoderTiming+1;
          lastState[ENA] = ENAState;
          //编码器计数
          if(ENAState){
              count[ENBState] ++;
          }else{
              count[!ENBState] ++;
          }
          if(debugOn) globalPDL.printfln("[DBENC]%s->B:%d, F:%d",periName,count[0],count[1]);
          responseTimer.start();
          lastUpdateTick = millis();
      }
      if(lastState[ENB] != ENBState){
          encoderTiming = encoderTiming-1;
          lastState[ENB] = ENBState;
          responseTimer.start();
          lastUpdateTick = millis();
      }
      //检测编码器故障
      if(getAlarm() == AlarmType::NoAlarm){
          if(encoderTiming > encoderTimingTore || encoderTiming < -encoderTimingTore){    //编码器误码故障
              if(encoderTiming > encoderTimingTore){  //仅A相运动，B相无动作
                  setAlarm(AlarmType::EncoderHardwareBPhaseAlarm);
              }else if(encoderTiming < -encoderTimingTore){  //仅A相运动，B相无动作
                  setAlarm(AlarmType::EncoderHardwareAPhaseAlarm);
              }
          }
          if(responseTimer.checkTimedOut()){
              dblogln("开始时间 %d %d %d",responseTimer.getStartTime(),responseTimer.getCurrentTime(),responseTimer.getPassedTime());
              setAlarm(AlarmType::EncoderHardwareAlarm);  //无信号故障
          }
      }
    }
  }
};