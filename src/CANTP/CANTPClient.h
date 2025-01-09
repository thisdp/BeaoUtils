#pragma once
#include <cstdint>
#include "CANTPMsgCodec.h" // Assuming this is the header for CANTPMsgCodec
#include "CANTPDevice.h"
#include "HardwareCAN.h"        // Assuming this is the header for HardwareCAN
#include "CANTPUniqueID.h"
#include "../STimer/STimer.h"            // Assuming this is the header for MSTimer

class CANTPClient : public CANTPMsgCodec {
private:
    char deviceName[64];
    CANTPUniqueID uniqueID;
    CANConfig canConf;

    bool reassignIfIDConflict;
    MSTimer reconnectTimer;
    MSTimer askConnectTimeOutTimer;
    MSTimer connectingTimeOutTimer;
    MSTimer clientSyncTimer;
    MSTimer serverSyncTimeOutTimer;
    uint8_t uniqueIDFragmentAppendLength;
    CANTPConnState connection;
    HardwareCAN* hwCAN;
    CANMessage rxMsg;
    CANMessage txMsg;
    CANTPDevice device;

public:
    uint32_t receiveTimes;
    uint32_t hbTimes;
    
    CANTPClient(HardwareCAN &can);

    void begin();
    void onMessageReceived();
    void tryToJoinBus();
    void onInternalConnectionStateChange();
    void update();
    void send(uint8_t packType, uint8_t* data, uint8_t length);
    void onReceived();
    inline void send() { clientSyncTimer.start(); }
    inline void setCANConfig(CANConfig aCANType) { canConf = aCANType; mtuSize = 1<<(canConf.mtu+3); setMTUSize(mtuSize); }
    inline CANConfig getCANConfig() { return canConf; }
    inline void setConnectionState(CANTPConnState newState) { connection = newState; onInternalConnectionStateChange(); }
    inline CANTPConnState getConnectionState() { return connection; }
    inline void addUniqueIDFragment(uint8_t uidFrag) { uniqueID.setRaw(uniqueIDFragmentAppendLength++,uidFrag); }
    inline void setUniqueID(CANTPUniqueID &uid) { uniqueID = uid; }
    inline CANTPUniqueID& getUniqueID() { return uniqueID; }
    inline uint8_t getDeviceID() const { return deviceID; }
    inline void setDeviceName(const char* devName) {
        size_t len = strlen(devName);
        if (len >= sizeof(deviceName)) len = sizeof(deviceName) - 1;
        strncpy(deviceName, devName, len);
        deviceName[len] = '\0';
    }
};