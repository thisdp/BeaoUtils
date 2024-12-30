#pragma once
#include <Arduino.h>
// CANTP协议定义
/*
如果设备为 断开连接 状态，则等待一段时间后主动发起 连接请求指令，并且进入 正在连接 状态
如果服务端发现总线上有 连接请求，往总线上发送 允许连接 数据包，
随后进入UniqueID仲裁，设备向服务端发送1字节的UniqueID长度、1字节随机生成的UniqueID、然后是N字节的UniqueID （STM32中UniqueID为12个字节，在ESP32中UniqueID是6字节的MAC地址）
在服务端开始处理连接请求途中，不会响应其他 连接请求指令
未通过仲裁的设备将会恢复 断开连接 状态，通过仲裁的设备，会向服务端发送 最大传输单元 和 CAN总线类型（CAN2.0B/CANFD），CANFD的速率协商会在建立连接之后处理
然后会向服务端发送一个 期待使用的CANID，
如果这个CANID不为0，则服务端会在列表中查找这个ID是否被占用，如果该ID被占用，则会向设备发送回一个随机分配的CANID，设备可以选择接受/不接收这个ID，不接受ID则会发送 断开连接指令，接受的话就会使用这个ID并且发送 连接指令
如果这个CANID为0，则服务器会先根据UniqueID在保存的设备列表里查找对应的CANID，如果未检索到相应的CANID则动态分配CANID
如果一切顺利，则建立连接，并发送 连接完成
*/
// 定义CAN总线标准或CAN FD
#define USECAN CANSTD
#if USECAN == CANSTD
#define CANFrameMaxLength 8
#elif USECAN == CANFD
#define CANFrameMaxLength 64
#endif

#define CANDataFrame 0
#define CANRemoteFrame 1

// 计算数据部分的最大长度
#define CANFrameDataLength ((uint8_t)(CANFrameMaxLength - sizeof(uint16_t)))
#define CANFrameDataLengthMin ((uint8_t)(8 - sizeof(uint16_t)))

enum CANTPDataPackType : uint8_t {    // 数据帧
    CANTP_CONNECTION_DONE = 0, // 连接成功
    CANTP_CONNECTION_LOST = 1, // 连接丢失
    CANTP_ASSIGNED_ID = 2,  // 分配到ID
    CANTP_CONNECT_ALLOW = 3,// 允许连接
    CANTP_SHORT_DATA = 4,   // 短数据包
    CANTP_DATA_HEAD = 5,    // 长数据包头
    CANTP_DATA = 6,         // 长数据包内容
};

enum CANTPRemotePackType : uint8_t {  // 遥控帧类型
    CANTP_HEARTBEAT = 0,       // 心跳包
    CANTP_DISCONNECT = 2,      // 设备断开连接请求
    CANTP_CONNECT = 3,         // 设备连接请求
    CANTP_EXPECTING_CAN_ID = 4,// 希望使用的CAN ID
    CANTP_CONFIGURE = 5,       // 设备配置请求 (MTU和CAN类型) -> 希望使用的CAN ID
    CANTP_UID_ARBITRATION = 6, // UniqueID仲裁请求 -> 设备配置请求 (类型等)
    CANTP_CONNECT_REQUEST = 7, // 加入网络请求 （优先级最低） ->UniqueID仲裁请求
};
// 每个设备连接的状态
enum class CANTPConnState : uint8_t {
    DISCONNECTED,         // 断开连接状态
    CONNECTED,            // 已连接状态
    ASKINGCONNECT,        // 询问加入网络许可
    CONNECTING,           // 正在加入网络
    ARBITRATING_UID,      // 正在进行UniqueID仲裁
    CONFIGURING,          // 正在配置设备
    EXPECTING_CAN_ID,     // 正在请求期待使用的CAN ID
};

enum CANType {
    CANType_CAN20B = 0,
    CANType_CANFD = 1,
};

class CANConfig {
public:
    uint8_t mtu : 4;
    uint8_t canType : 4;
};

/*CAN消息*/
#pragma pack(push)
#pragma pack(1)
class CANMessage {
public:
#if defined(ESP32)
    uint32_t isExtendPack : 1;     //IDE
    uint32_t isRemotePack : 1;     //RTR
    uint32_t reserved1 : 30;
    uint32_t identifier : 11;    //STDID
    uint32_t extIdentifier : 18;    //EXTID
    uint32_t reserved2 : 3;
    uint8_t dataLength;             //DLC
#else
    uint32_t identifier;    //STDID
    uint32_t extIdentifier; //EXTID
    uint32_t isExtendPack;  //IDE
    uint32_t isRemotePack;  //RTR
    uint32_t dataLength;    //DLC
#endif
    uint8_t data[CANFrameMaxLength];
#if defined(ESP32)

    inline void readFrom(twai_message_t *twaiMsg) { memcpy(this, twaiMsg, sizeof(CANMessage)); }
    inline void writeTo(twai_message_t* twaiMsg) { memcpy(twaiMsg, this, sizeof(CANMessage)); }
    inline void readHeadFrom(twai_message_t* readHead) { memcpy(this, readHead, sizeof(CANMessage) - sizeof(data)); }
    inline void writeHeadTo(twai_message_t* writeHead) { memcpy(writeHead, this, sizeof(CANMessage) - sizeof(data)); }
    inline void readDataFrom(uint8_t* readData, uint8_t size = CANFrameMaxLength) { memcpy(data, readData, size); }
    inline void writeDataTo(uint8_t* writeData, uint8_t size = CANFrameMaxLength) { memcpy(writeData, data, size); }
#else
    inline void readHeadFrom(CAN_TxHeaderTypeDef* readHead) { memcpy(this, readHead, sizeof(CANMessage) - sizeof(data)); }
    inline void readHeadFrom(CAN_RxHeaderTypeDef* readHead) { memcpy(this, readHead, sizeof(CANMessage) - sizeof(data)); }
    inline void writeHeadTo(CAN_TxHeaderTypeDef* writeHead) { memcpy(writeHead, this, sizeof(CANMessage) - sizeof(data)); }
    inline void writeHeadTo(CAN_RxHeaderTypeDef* writeHead) { memcpy(writeHead, this, sizeof(CANMessage) - sizeof(data)); }
    inline void readDataFrom(uint8_t* readData, uint8_t size = CANFrameMaxLength) { memcpy(data, readData, size); }
    inline void writeDataTo(uint8_t* writeData, uint8_t size = CANFrameMaxLength) { memcpy(writeData, data, size); }
#endif
    inline void clear() { memset(this, 0, sizeof(CANMessage)); };
    inline void setExtend(bool isExtend) { isExtendPack = isExtend; }
    inline void setRemote(bool isRemote) { isRemotePack = isRemote; }
    inline void setIdentifier(uint32_t id, uint32_t extID = 0) { identifier = id; extIdentifier = extID; }
    inline void setIdentifier(CANTPFrameID &id) { identifier = (id.packType<<8)+id.identifier; extIdentifier = id.extIdentifier; }
    inline void setExtIdentifier(uint32_t extID) { extIdentifier = extID; }
    inline void setDataLength(uint32_t dataLen) { dataLength = dataLen; }
    inline bool isExtend() { return isExtendPack != 0; }
    inline bool isRemote() { return isRemotePack != 0; }
    inline uint32_t getIdentifier() { return identifier; }
    inline uint32_t getExtIdentifier() { return extIdentifier; }
    inline uint32_t getCompleteIdentifier() { return (identifier << 11) + extIdentifier; }
    inline void getDataLength(uint32_t dataLen) { dataLength = dataLen; }
};
#pragma pack(pop)
/********************CANTP帧格式********************/
/*
完整传输: Head帧 > Data帧1 > Data帧2 > ... > Data帧N
Head帧:
    2Bytes 功能标识符
    2Bytes 起始地址
    2Bytes 数据总长度
    2Bytes 数据包总数
Data帧:
    2Bytes 数据包序号
    (mtuSize-2)Bytes 数据
*/
#pragma pack(push)
#pragma pack(1)
/*Packs*/
class CANTP_ShortPack { // 短数据包
private:
    uint16_t functionID;
    uint16_t startAddr;
    uint32_t data;
public:
    CANTP_ShortPack(uint16_t functionID = 0, uint16_t startAddr = 0, uint32_t data = 0) {
        this->functionID = functionID;
        this->startAddr = startAddr;
        this->data = data;
    }
    void setFunctionID(uint16_t functionID) { this->functionID = functionID; }
    uint16_t getFunctionID() const { return functionID; }
    void setStartAddress(uint16_t startAddr) { this->startAddr = startAddr; }
    uint16_t getStartAddress() const { return startAddr; }
    void setData(uint32_t data) { this->data = data; }
    uint32_t getData() const { return data; }
};

class CANTP_PackHead { // 长数据包头
private:
    uint16_t functionID;
    uint16_t startAddr;
    uint16_t dataLength;    //总数据长度
    uint16_t packCount;
public:
    CANTP_PackHead(uint16_t functionID = 0, uint16_t startAddr = 0, uint16_t dataLength = 0, uint16_t packCount = 0) {
        this->functionID = functionID;
        this->startAddr = startAddr;
        this->dataLength = dataLength;
        this->packCount = packCount;
    }
    void setFunctionID(uint16_t functionID) { this->functionID = functionID; }
    uint16_t getFunctionID() const { return functionID; }
    void setStartAddress(uint16_t startAddr) { this->startAddr = startAddr; }
    uint16_t getStartAddress() const { return startAddr; }
    void setDataLength(uint16_t dataLength) { this->dataLength = dataLength; }
    uint16_t getDataLength() const { return dataLength; }
    void setPackCount(uint16_t packCount) { this->packCount = packCount; }
    uint16_t getPackCount() const { return packCount; }
};

class CANTP_PackData {  // 长数据包数据
private:
    uint16_t packIndex;
    uint8_t data[CANFrameDataLength];   //硬件支持最大数据长度-2
public:
    CANTP_PackData(uint16_t packIndex = 0, uint8_t *data = nullptr) {
        this->packIndex = packIndex;
        if(data != nullptr) memcpy(this->data, data, CANFrameDataLength);
    }
    void setPackIndex(uint16_t packIndex) { this->packIndex = packIndex; }
    uint16_t getPackIndex() { return this->packIndex; }
    void setData(uint16_t *data, uint16_t size){
        memcpy(this->data, data, size);
    }
    uint8_t *getData(){
        return data;
    }
};
#pragma pack(pop)