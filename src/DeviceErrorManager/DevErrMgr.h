#include "../FIFO/FIFO.h"

#pragma pack(push)
#pragma pack(1)
class DeviceErrorContext{
public:
  DeviceErrorContext(): ID(0), Type(0), Content(0){}
  DeviceErrorContext(uint16_t pID, uint16_t pType, uint16_t pContent): ID(pID), Type(pType), Content(pContent){}
  uint16_t ID;
  uint16_t Type;
  uint16_t Content;     //报警
};
#pragma pack(pop)

template<typename SIZE>
class DeviceErrorManager : public FIFO<DeviceErrorContext,SIZE>{
public:
};