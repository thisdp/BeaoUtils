#pragma once
#include "Arduino.h"
class TPSCounter{
public:
  uint32_t TPS;
  uint32_t counter;
  uint32_t lastTick;
  TPSCounter(): TPS(0), counter(0), lastTick(0) {}
  inline bool inc(){
    counter ++;
    if(millis() - lastTick >= 1000){
      TPS = counter;
      counter = 0;
      lastTick += 1000;
      return true;
    }
    return false;
  }
};