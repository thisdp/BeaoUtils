#pragma once
#include "STimer.h"
STimerBase::STimerBase(uint32_t tDuration) : duration(tDuration), activated(false), startTime(0){}
bool STimerBase::start(){
    startTime = getCurrentTime();
    activated = true;
    return true;
}
bool STimerBase::startIfNotActivated(){
    if(activated) return true;
    startTime = getCurrentTime();
    activated = true;
    return true;
}
bool STimerBase::startIfNotRunning(){
    if(activated && !checkTimedOut()) return true;
    startTime = getCurrentTime();
    activated = true;
    return true;
}
bool STimerBase::restart(){
    restartTimes++;
    return start();
}
void STimerBase::clearRestartTimes(){
    restartTimes = 0;
}
bool STimerBase::stop(){
    activated = false;
    return true;
}
bool STimerBase::isActivated(){
    return activated;
}
uint32_t STimerBase::getPassedTime(){
    return getCurrentTime()-startTime;
}
uint32_t STimerBase::getStartTime(){
    return startTime;
}
void STimerBase::setDuration(uint32_t tDuration){
    duration = tDuration;
}
bool STimerBase::checkTimedOut(){
    return (getPassedTime() >= duration);
}

//STimer
STimer::STimer(uint32_t tDuration) : STimerBase(tDuration) {}
uint32_t STimer::getCurrentTime(){
    return millis()/1000;
}

//MSTimer
MSTimer::MSTimer(uint32_t tDuration) : STimerBase(tDuration) {}
uint32_t MSTimer::getCurrentTime(){
    return millis();
}

//USTimer
USTimer::USTimer(uint32_t tDuration) : STimerBase(tDuration) {}
uint32_t USTimer::getCurrentTime(){
    return micros();
}