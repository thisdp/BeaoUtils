#pragma once
#include "CANTPDef.h"
#include "../FIFO/FIFO.h"
#include "../STimer/STimer.h"
#include <cstdint>
#include <vector>
#include <queue>
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <Arduino.h>
using namespace std;

/****************************/
class HardwareCAN { //STM32 CAN硬件实现
private:
#if defined(ESP32)
    
#else
    CAN_HandleTypeDef* can;
#endif
    DynamicFIFO<CANMessage> rxMessage;
    DynamicFIFO<CANMessage> txMessage;
    CANMessage rxTempMessage;
    CANMessage txTempMessage;
public:

#if defined(ESP32)

#else
    HardwareCAN(CAN_HandleTypeDef &argCan) : can(&argCan), rxMessage(16), txMessage(16) {}
    inline CAN_HandleTypeDef* getCAN() { return can; }
    inline uint8_t getPendingTXMessages() {
        return HAL_CAN_GetTxMailboxesFreeLevel(can);
    }
#endif
    int available() {
        return rxMessage.length();
    }
    int availableForWrite() {
        return txMessage.emptyLength();
    }
    bool isTXEmpty() {
        return txMessage.length() == 0 && getPendingTXMessages() == 0;
    }
    bool send(uint32_t identifier, uint8_t* data, uint8_t length, bool isRemote = false, bool isExtend = false, uint32_t extIdentifier = 0){ // 发送数据帧/远程帧/扩展帧
        if (!availableForWrite()) return false;
        txTempMessage.setIdentifier(identifier, extIdentifier);
        txTempMessage.setExtend(isExtend);
        txTempMessage.setRemote(isRemote);
        txTempMessage.setDataLength(length);
        txTempMessage.readDataFrom(data,length);
        txMessage.enqueue(txTempMessage);
        return true;
    }
    bool send(CANMessage &msg) {
        if (!availableForWrite()) return false;
        txMessage.enqueue(msg);
        return true;
    }
    inline void doReceive() {
#if defined(ESP32)

#else
        if (getPendingTXMessages()) {   //如果RX的FIFO有数据
            if (!rxMessage.isFull()) {  // 如果接收FIFO没有满
                CAN_RxHeaderTypeDef rxHead;
                HAL_CAN_GetRxMessage(can, 1, &rxHead, rxTempMessage.data);  //读取数据
                rxTempMessage.readHeadFrom(&rxHead);
                rxMessage.enqueue(rxTempMessage);   //扔进FIFO
            }
        }
#endif
    }
    inline void doSend() {
#if defined(ESP32)

#else
        if (HAL_CAN_GetTxMailboxesFreeLevel(can)) {    // 如果发送位空闲
            if (!txMessage.isEmpty()) {   // 如果有需要发送的数据
                CAN_TxHeaderTypeDef txHead;
                CANMessage* txMsg;
                txMessage.dequeue(txMsg);
                txMsg->writeHeadTo(&txHead);
                uint32_t txUsingMailBox;
                HAL_CAN_AddTxMessage(can, &txHead, txMsg->data, &txUsingMailBox);
            }
        }
#endif
    }
    void update() {
        doReceive();
        doSend();
    }
};

/*CANTP的UniqueID组成
1Byte:  UniqueID总长度N
1Byte:  0~255随机数
NBytes: UniqueID数据
*/
class CANTPUniqueID {
public:
    CANTPUniqueID(uint8_t len = 0) {
        memset(this, 0, sizeof(CANTPUniqueID));
        uniqueIDLength = len; //UniqueID数据总长度
    }
    CANTPUniqueID(uint8_t* uniqueID, uint8_t len){
        uniqueIDLength = len;
        setData(uniqueID);
        uniqueIDRandom = 0;
    }
    void setLength(uint8_t len) { uniqueIDLength = len;}
    uint8_t getLength() { return uniqueIDLength; }
    void setData(uint8_t* uID) { memcpy(uniqueIDData, uID, uniqueIDLength-2); }
    uint8_t *getData() { return uniqueIDData; }
    uint8_t getDataLength() { return (uniqueIDLength < 2)? 0 : uniqueIDLength - 2; }
    void setRaw(uint8_t index, uint8_t raw) { uniqueID[index] = raw; }
    uint8_t getRaw(uint8_t index) { return (index < uniqueIDLength) ? uniqueID[index] : 0; }
    void clear() { memset(this, 0, sizeof(CANTPUniqueID)); }
    uint8_t getRandomID() { return uniqueIDRandom; }
    void setRandomID(uint8_t randID) { uniqueIDRandom = randID; }
    bool operator==(const CANTPUniqueID& other) const { // Random ID仅用作CAN总线仲裁，不用作UniqueID比较
        return uniqueIDLength == other.uniqueIDLength && (memcmp(uniqueIDData, other.uniqueIDData, uniqueIDLength-2) == 0);
    }
    bool operator!=(const CANTPUniqueID& other) const { return !(*this == other); }
private:
    union {
        struct {
            uint8_t uniqueIDLength;     // 总长度
            uint8_t uniqueIDRandom;     // 随机数
            uint8_t uniqueIDData[30];   // UniqueID数据内容
        };
        uint8_t uniqueID[32]; // 存储UniqueID，最大支持32字节
    };
    //STM32为12位
};
class CANTPServer;
class CANTPServerDevice : public CANTP_MessageCodec {   //服务端存储的 设备客户端
private:
    char deviceName[64];
    CANTPUniqueID uniqueID;     // 存储UniqueID
    CANConfig canConf;          // CAN配置

    uint8_t uniqueIDFragmentAppendLength;
    CANTPConnState connection;  // 设备连接状态
    CANTPServer* server;        // 依赖的服务端（用于数据发送/接收)
    MSTimer syncTimer;
public:
    CANTPServerDevice(CANTPServer *attachedServer) : server(attachedServer), syncTimer(2000) {}
    CANTPServerDevice& operator=(CANTPServerDevice& other) {
        if (this == &other) return *this; // 防止自我赋值
        memcpy(this, &other, sizeof(CANTPServerDevice));
        return *this;
    }
    CANTPServerDevice(const CANTPServerDevice& other) {
        memcpy(this, &other, sizeof(CANTPServerDevice));
    }
    void begin() {
        connection = CANTPConnState::DISCONNECTED;
        deviceID = 0;
        canConf.canType = CANType_CAN20B;
        canConf.mtu = 0;    //8 Bytes
        mtuSize = 1<<(canConf.mtu+3);
        uniqueIDFragmentAppendLength = 0;
        memset(deviceName, 0, sizeof(deviceName));
    }
    void setCANConfig(CANConfig aCANType) { canConf = aCANType; mtuSize = 1<<(canConf.mtu+3); }
    CANConfig getCANConfig() { return canConf; }
    void setConnectionState(CANTPConnState newState) { connection = newState; }
    CANTPConnState getConnectionState() { return connection; }
    void addUniqueIDFragment(uint8_t uidFrag) { uniqueID.setRaw(uniqueIDFragmentAppendLength++,uidFrag); }
    CANTPUniqueID &getUniqueID() { return uniqueID; }
    void setUniqueID(CANTPUniqueID &uid) { uniqueID = uid; }
    void setDeviceID(uint8_t id) { deviceID = id; }
    uint8_t getDeviceID() const { return deviceID; }
    void resetSyncTimer() { syncTimer.start(); }
    void setDeviceName(const char* devName) {
        // 确保我们不会超出 deviceName 的边界
        size_t len = strlen(devName);
        if (len >= sizeof(deviceName)) len = sizeof(deviceName) - 1; // 我们需要空间给字符串结束符 '\0'
        strncpy(deviceName, devName, len);
        deviceName[len] = '\0'; // 确保字符串正确终止
    }
    char *getDeviceName(){
        return deviceName;
    }
    bool isDesynced() { return syncTimer.checkTimedOut(); }
    //发送相关
};

/******************************************************客户端*************************************************************/
class CANTPClient : public CANTP_MessageCodec {
private:
    char deviceName[64];
    CANTPUniqueID uniqueID;     // 存储UniqueID
    CANConfig canConf;          // CAN配置

    bool reassignIfIDConflict;
    MSTimer reconnectTimer;
    MSTimer askConnectTimeOutTimer;
    MSTimer syncTimer;
    uint8_t uniqueIDFragmentAppendLength;
    CANTPConnState connection;  // 设备连接状态
    HardwareCAN* hwCAN;
    CANMessage rxMsg;
    CANMessage txMsg;
    CANTP_MessageCodec codec;
public:
    CANTPClient(HardwareCAN &can) : hwCAN(&can), reconnectTimer(200), askConnectTimeOutTimer(100), syncTimer(1000){}
    void begin() {
        connection = CANTPConnState::DISCONNECTED;
        deviceID = 0;
        canConf.canType = CANType_CAN20B;
        canConf.mtu = 0;    //8 Bytes
        uniqueIDFragmentAppendLength = 0;
        reconnectTimer.start();

    }
    void onMessageReceived() {
        uint32_t canIdentifier = rxMsg.getCompleteIdentifier();
        uint8_t packType = ((CANTPFrameID*)&canIdentifier)->packType;
        uint8_t packIdentifier = ((CANTPFrameID*)&canIdentifier)->identifier;
        if (rxMsg.isRemote()) {    //远程帧
            switch (packType) {}
        } else {    //数据帧
            switch (packIdentifier) {
            case CANTP_CONNECT_ALLOW:  // 当接收到允许连接回包
                setConnectionState(CANTPConnState::CONNECTING); // 正在连接
                for (uint8_t i = 0; i < uniqueID.getLength(); i++) {   // 如果接收到，则开始发送UniqueID
                    txMsg.clear();
                    CANTPFrameID frameIdentifier(CANTP_UID_ARBITRATION, uniqueID.getRaw(i));
                    txMsg.setIdentifier(frameIdentifier);
                    txMsg.setExtend(false);
                    txMsg.setRemote(true);
                    hwCAN->send(txMsg); //发送
                }
                // 如果没有接收到的话，不会进入下一步，等待超时后重新进入Disconnected状态
                // 如果仲裁失败，则退出竞争，间歇性发送JOIN REQUEST
            break;
            case CANTP_ASSIGNED_ID:{ // 当接收到分配ID的回包
                if (deviceID != 0) {    // 如果指定了ID
                    if (packIdentifier != deviceID && !reassignIfIDConflict) {  // 如果选择拒绝重新分配ID
                        txMsg.clear();
                        CANTPFrameID frameIdentifier(CANTP_DISCONNECT, packIdentifier);
                        txMsg.setIdentifier(frameIdentifier);
                        txMsg.setExtend(false);
                        txMsg.setRemote(true);
                        hwCAN->send(txMsg); //发送Disconnect
                        setConnectionState(CANTPConnState::DISCONNECTED);
                        return;
                    }
                }
                setDeviceID(packIdentifier);  //写入ID
                txMsg.clear();
                CANTPFrameID frameIdentifier(CANTP_CONNECT, packIdentifier);
                txMsg.setIdentifier(frameIdentifier);
                txMsg.setExtend(false);
                txMsg.setRemote(true);
                hwCAN->send(txMsg); //发送Connect
            }
            break;
            case CANTP_CONNECTION_DONE:    // 连接成功
                setConnectionState(CANTPConnState::CONNECTED); // 随后服务器只能通过CANID访问本机
            break;
            case CANTP_CONNECTION_LOST:    // 连接丢失
                setConnectionState(CANTPConnState::DISCONNECTED);
            break;
            case CANTP_SHORT_DATA:  //短数据包
            break;
            case CANTP_DATA_HEAD:   //数据包头
            break;
            case CANTP_DATA:    //数据内容
            break;
            }
        }
    }
    void tryToJoinBus() {
        setConnectionState(CANTPConnState::ASKINGCONNECT); // 询问连接
        askConnectTimeOutTimer.start();
        txMsg.clear();
        CANTPFrameID frameIdentifier(CANTP_CONNECT_REQUEST, deviceID);  //预使用指定ID
        txMsg.setIdentifier(frameIdentifier);
        txMsg.setExtend(false);
        txMsg.setRemote(true);
        hwCAN->send(txMsg); //发送Connect Request
    }
    void onInternalConnectionStateChange() {
        if (getConnectionState() == CANTPConnState::DISCONNECTED) {
            reconnectTimer.start();
        }
    }
    void update() {
        hwCAN->update();
        switch (connection) {
        case CANTPConnState::DISCONNECTED:  //如果已断开连接
            if (reconnectTimer.checkTimedOut()) {
                reconnectTimer.stop();  //停止计时
                tryToJoinBus(); //尝试加入总线
            }
            break;
        case CANTPConnState::ASKINGCONNECT:    //如果正在询问连接
            if (askConnectTimeOutTimer.checkTimedOut()) {
                setConnectionState(CANTPConnState::DISCONNECTED);   //连接超时，复位状态
                askConnectTimeOutTimer.stop();  //停止计时
            }
            break;
        case CANTPConnState::CONNECTED:
            if (!hwCAN->isTXEmpty()) {
                if (syncTimer.checkTimedOut()) {    //每隔一定时间，如果没有任何数据包发送至服务端，则需要发送一个心跳包
                    syncTimer.start();
                    txMsg.clear();
                    CANTPFrameID frameIdentifier(CANTP_HEARTBEAT, deviceID);
                    txMsg.setIdentifier(frameIdentifier);
                    txMsg.setExtend(false);
                    txMsg.setRemote(true);
                    hwCAN->send(txMsg); //发送Heart Beat
                }
            }
            break;
        }
    }
    
// 发送数据帧
    void send(uint8_t packType, uint8_t* data, uint8_t length) {
    }
    void send() {   //发送数据
        syncTimer.start();  //清除Sync定时器
    }
    void setCANConfig(CANConfig aCANType) { canConf = aCANType; mtuSize = 1<<(canConf.mtu+3); codec.setMTUSize(mtuSize); }
    CANConfig getCANConfig() { return canConf; }
    void setConnectionState(CANTPConnState newState) { connection = newState; onInternalConnectionStateChange(); }
    CANTPConnState getConnectionState() { return connection; }
    void addUniqueIDFragment(uint8_t uidFrag) { uniqueID.setRaw(uniqueIDFragmentAppendLength++,uidFrag); }
    void setUniqueID(CANTPUniqueID &uid) { uniqueID = uid; }
    CANTPUniqueID& getUniqueID() { return uniqueID; }
    void setDeviceID(uint8_t id) { deviceID = id; codec.setDeviceID(deviceID); }
    uint8_t getDeviceID() const { return deviceID; }
    void setDeviceName(const char* devName) {
        // 确保我们不会超出 deviceName 的边界
        size_t len = strlen(devName);
        if (len >= sizeof(deviceName)) len = sizeof(deviceName) - 1; // 我们需要空间给字符串结束符 '\0'
        strncpy(deviceName, devName, len);
        deviceName[len] = '\0'; // 确保字符串正确终止
    }
};

/******************************服务端****************************/
class CANTPServer { //服务端
private:
    bool isNewDeviceConnecting;
    uint32_t newDeviceConnectingTick;
    unordered_map<uint8_t, CANTPServerDevice>deviceList;
#define ConnectingDevice deviceList.at(0)
    uint8_t staticDevicesCount;
    HardwareCAN *hwCAN;
    CANMessage rxMsg;
    CANMessage txMsg;
    
public:
    CANTPServer(HardwareCAN& canPhy, uint8_t staticDevices){  //创建固定数量的设备列表
        staticDevicesCount = staticDevices;
        hwCAN = &canPhy;
        deviceList.emplace(0, this);    //正在连接的设备
        for (uint8_t i = 1; i <= staticDevicesCount; i++) deviceList.emplace(i, this);
    }
    void loadDeviceList() {

    }
    void loadDevice(uint8_t devID) {

    }
    void saveDeviceList() {

    }
    void saveDevice(uint8_t devID) {

    }
    void onDeviceConnected(CANTPServerDevice &dev) {
        uint8_t devID = dev.getDeviceID();
        if (devID == 0) return; //devID不可能为0
        if (deviceList.count(devID) == 0) return;   //该处不可能没有devID
        deviceList.at(devID) = dev;    //复制
        deviceList.at(devID).setConnectionState(CANTPConnState::CONNECTED);    //标记为在线
        if (devID <= staticDevicesCount) saveDevice(devID);
    }
    void onDeviceDisconnected(CANTPServerDevice &dev) {
        uint8_t devID = dev.getDeviceID();
        if (devID == 0) return; //devID不可能为0
        if (deviceList.count(devID) == 0) return;   //该处不可能没有devID
        deviceList.at(devID).setConnectionState(CANTPConnState::DISCONNECTED);    //标记为离线
    }
    uint8_t searchProperSpace(uint8_t expectedID = 0) {  // 寻找合适的空间
        if (expectedID != 0) {  //如果指定了ID，则ID优先
            if(deviceList.count(expectedID) == 0){  //如果该处没有分配设备
                deviceList.emplace(expectedID, this);   //分配ID
                return expectedID;
            }
            if (deviceList.at(expectedID).getConnectionState() == CANTPConnState::DISCONNECTED) {    //如果设备已断开
                return expectedID;
            } else {    //如果设备已经连接
                if (deviceList.at(expectedID).getUniqueID() == ConnectingDevice.getUniqueID()) {   //如果UID一样，则可能是掉线了，重连
                    return expectedID;
                } //如果UID不一样，则可能是地址冲突，重新在动态设备区域内分配ID
            }
        } else {    //如果未指定ID，则根据 UniqueID，优先搜索 staticDevice
            for (uint8_t i = 1; i <= staticDevicesCount; i++) { // 先扫描静态分配UniqueID一致的设备
                if(deviceList.count(expectedID) == 0){  //如果该处没有分配设备
                    deviceList.emplace(expectedID, this);   //分配ID
                    return expectedID;
                }
                if (deviceList.at(i).getUniqueID() == ConnectingDevice.getUniqueID())    //如果找到已保存的ID
                    return i;
            }
        }
        //上述条件都不满足，则分配动态ID
        for (size_t i = staticDevicesCount+1; i <= 255; i++) {    // 扫描动态区域内空闲ID
            if (deviceList.count(i) == 0|| deviceList.at(i).getConnectionState() == CANTPConnState::DISCONNECTED) {   //如果该处没有创建对象或该处设备空闲
                return i;
            }
        }
        return 0;  //没有空闲的位置
    }
    inline bool isNewDeviceConnectTimedOut() { return isNewDeviceConnecting && (millis() - newDeviceConnectingTick >= 2000); }
    inline void resetNewDeviceConnectTick() { newDeviceConnectingTick = millis(); }
    void onMessageReceived() {
        uint32_t canIdentifier = rxMsg.getCompleteIdentifier();
        uint8_t packType = ((CANTPFrameID*)&canIdentifier)->packType;
        uint8_t packIdentifier = ((CANTPFrameID*)&canIdentifier)->identifier;
        if (rxMsg.isRemote()) {    //远程帧
            switch (packType) {
            case CANTP_CONNECT_REQUEST:    // 当有设备请求连接
                if (!isNewDeviceConnecting || isNewDeviceConnectTimedOut()) { //如果没有设备在连接，或上一个连接已经超时
                    isNewDeviceConnecting = true;   //开始处理新连接
                    ConnectingDevice.begin();    // 立刻对正在连接的设备初始化
                    ConnectingDevice.setConnectionState(CANTPConnState::CONNECTING);
                    txMsg.clear();
                    CANTPFrameID frameIdentifier(CANTP_CONNECT_ALLOW,0);
                    txMsg.setIdentifier(frameIdentifier);
                    txMsg.setExtend(false); //标准帧
                    txMsg.setRemote(false); //数据帧
                    hwCAN->send(txMsg); //发送接受连接回包
                    resetNewDeviceConnectTick();
                } //否则不回复接受设备连接请求
                break;
            case CANTP_UID_ARBITRATION: // 开始通过设备UniqueID仲裁
                if (isNewDeviceConnecting && !isNewDeviceConnectTimedOut()) { //如果设备正在连接，并且没有超时
                    ConnectingDevice.addUniqueIDFragment(packIdentifier);   //记录Unique片段
                    ConnectingDevice.setConnectionState(CANTPConnState::ARBITRATING_UID);
                    resetNewDeviceConnectTick();
                }
                break;
            case CANTP_CONFIGURE:        // 接收设备的配置
                if (isNewDeviceConnecting && !isNewDeviceConnectTimedOut()) { //如果设备正在连接，并且没有超时
                    ConnectingDevice.setCANConfig(*(CANConfig*)&(packIdentifier));
                    ConnectingDevice.setConnectionState(CANTPConnState::CONFIGURING);
                    resetNewDeviceConnectTick();
                }
                break;
            case CANTP_EXPECTING_CAN_ID:   // 尝试使用期待的CAN ID
                if (isNewDeviceConnecting && !isNewDeviceConnectTimedOut()) { //如果设备正在连接，并且没有超时
                    uint8_t id = searchProperSpace(packIdentifier); //0表示未分配到地址，如果期待的ID不等于分配到的ID，则表示ID已占用
                    ConnectingDevice.setConnectionState(CANTPConnState::EXPECTING_CAN_ID);
                    ConnectingDevice.setDeviceID(id);
                    txMsg.clear();
                    CANTPFrameID frameIdentifier(CANTP_ASSIGNED_ID, id);
                    txMsg.setIdentifier(frameIdentifier);
                    txMsg.setExtend(false); //标准帧
                    txMsg.setRemote(false); //数据帧
                    hwCAN->send(txMsg); //发送回获取到的CAN ID
                }
                break;
            case CANTP_CONNECT:       // 设备连接
                if (isNewDeviceConnecting && !isNewDeviceConnectTimedOut()) { //如果设备正在连接，并且没有超时
                    onDeviceConnected(ConnectingDevice);
                    isNewDeviceConnecting = false;  //连接完成
                    txMsg.clear();
                    CANTPFrameID frameIdentifier(CANTP_CONNECTION_DONE, ConnectingDevice.getDeviceID());
                    txMsg.setIdentifier(frameIdentifier);
                    txMsg.setExtend(false); //标准帧
                    txMsg.setRemote(false); //数据帧
                    hwCAN->send(txMsg); //向指定设备发送Connect Done
                }
                break;
            case CANTP_DISCONNECT:   // 设备断开连接
                if (isNewDeviceConnecting && !isNewDeviceConnectTimedOut()) { //如果设备正在连接，并且没有超时
                    ConnectingDevice.setConnectionState(CANTPConnState::DISCONNECTED);
                    isNewDeviceConnecting = false;  //连接完成
                }
            case CANTP_HEARTBEAT:       // 心跳包
                if (deviceList.count(packIdentifier) != 0 && deviceList.at(packIdentifier).getConnectionState() == CANTPConnState::CONNECTED) {    //仅在线有效
                    deviceList.at(packIdentifier).resetSyncTimer();
                }
                break;
            }
        } else {    //数据帧
            switch (packType) {
            case CANTP_SHORT_DATA:  //短数据包
                break;
            case CANTP_DATA_HEAD:   //数据头
                if (deviceList.count(packIdentifier) != 0){
                    if (deviceList.at(packIdentifier).getConnectionState() == CANTPConnState::CONNECTED) {    //仅在线有效
                        deviceList.at(packIdentifier).resetSyncTimer();    //只要收到数据包就复位
                    } else {    //如果不在线
                        txMsg.clear();
                        CANTPFrameID frameIdentifier(CANTP_CONNECTION_LOST, packIdentifier);
                        txMsg.setIdentifier(frameIdentifier);
                        txMsg.setExtend(false); //标准帧
                        txMsg.setRemote(false); //数据帧
                        hwCAN->send(txMsg); //向指定设备发送Connect Lost
                    }
                }
                break;
            case CANTP_DATA:    //数据本体
                if (deviceList.count(packIdentifier) != 0) {
                    if (deviceList.at(packIdentifier).getConnectionState() == CANTPConnState::CONNECTED) {    //仅在线有效
                        deviceList.at(packIdentifier).resetSyncTimer();    //只要收到数据包就复位
                    } else {    //如果接收到的数据来自于离线的设备
                        txMsg.clear();
                        CANTPFrameID frameIdentifier(CANTP_CONNECTION_LOST, packIdentifier);
                        txMsg.setIdentifier(frameIdentifier);
                        txMsg.setExtend(false); //标准帧
                        txMsg.setRemote(false); //数据帧
                        hwCAN->send(txMsg); //向指定设备发送Connect Lost
                    }
                }
                break;
            }
        }
    }
    void update() {   //处理消息
        hwCAN->update();
        /*for (auto& kv : deviceList) {
            if (kv.first != 0) {
                if (kv.second.getConnectionState() == CANTPConnState::CONNECTED) {  //如果已连接
                    if (kv.second.isDesynced()) {   //如果设备丢失连接过久
                        kv.second.setConnectionState(CANTPConnState::DISCONNECTED); //标记为断开
                    }
                }
            }
        }*/
    }
};