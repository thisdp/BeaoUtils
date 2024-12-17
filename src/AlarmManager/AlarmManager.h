#include "../Logger/Logger.h"
#include "AlarmDefinition.h"
#include "Arduino.h"
#include <vector>
#pragma pack(push)
#pragma pack(1)

template<uint32_t MaxAlarmNum>
class AlarmManager {
public:
    // 构造函数
    AlarmManager() : alarmCount(0), alarmContents(MaxAlarmNum, false) {}
    // 指定报警索引进行注册
    bool registerAlarmAt(uint32_t uIndex) {
        if (uIndex >= MaxAlarmNum){
          debugLogger.println("@registerAlarmAt, 报警索引超出范围");
          return false; // 如果索引超出范围或已被占用，则返回false
        }
        if(isRegistered(uIndex)){
          debugLogger.println("@registerAlarmAt, 报警索引已被占用");
          return false; // 如果索引超出范围或已被占用，则返回false
        }
        alarmContents[uIndex] = true; // 设置为已注册
        alarmCount ++;
        return true;
    }
    // 自动分配下一个可用的报警索引，并通过引用参数返回分配的ID
    bool registerAlarm(uint32_t &uIndex) {
        for (uint32_t i = 0; i < MaxAlarmNum; ++i) {
            if (!isRegistered(i)) { // 找到第一个空闲位置
                uIndex = i; // 将找到的空闲索引赋值给引用参数
                alarmContents[i] = true; // 设置为已注册
                alarmCount ++;
                return true; // 返回成功
            }
        }
        debugLogger.println("@registerAlarm, 报警列表已满");
        return false; // 如果没有空闲位置，则返回失败
    }
    // 获取指定索引的报警内容
    uint16_t getAlarmContent(uint32_t index) {
        if (index >= MaxAlarmNum){
          debugLogger.println("@getAlarmContent, 报警索引超出范围");
          return false;
        }
        return alarms[index];
    }
    // 设置指定索引的报警内容
    bool setAlarmContent(uint32_t index, uint16_t content) {
        if (index >= MaxAlarmNum){
          debugLogger.println("@setAlarmContent, 报警索引超出范围");
          return false;
        }
        alarms[index] = content;
    }
    void setDebugStream(Stream &dbStream){
      debugLogger.setStream(dbStream);
    }
private:
    uint16_t alarms[MaxAlarmNum]; // 报警内容数组
    uint32_t alarmCount;
    std::vector<bool> alarmContents; // 报警内容数组，使用bool来表示是否注册
    Logger debugLogger;
    // 检查指定索引是否已经被注册
    bool isRegistered(uint32_t index) {
        if (index >= MaxAlarmNum){
          debugLogger.println("@setAlarmContent, 报警索引超出范围");
          return false;
        }
        return alarmContents[index];
    }

};


#pragma pack(pop)