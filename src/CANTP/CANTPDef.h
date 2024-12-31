#pragma once
#include <Arduino.h>
#include <vector>
#include <cstring>
#include "../Logger/Logger.h"
Logger globalCANTPLogger;  //global CANTP Debug Logger
bool setCANTPDebugStream(Stream &stream){ //设置全局调试输出流
    globalCANTPLogger.setStream(stream);
    return true;
};

using namespace std;
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

/*
CANTP数据包组成:
    CANTPFrame ID   (对应Identifier段)
    CANTPFrame Data (对应Data段)
通信属性:
    MTU: 最大报文长度 (由所使用的CAN协议决定，也可以手动指定，但是必须低于所使用的CAN支持的最大长度，CAN2.0B为8字节，CANFD为64字节)
    CANType: CAN/CANFD
*/
// CAN帧标识符及包类型
class CANTPFrameID {
public:
    uint32_t extIdentifier  : 18;
    uint32_t identifier     : 8;
    uint32_t packType       : 3; // 使用3位表示包类型
    uint32_t reserved       : 3;
    CANTPFrameID();
    CANTPFrameID(uint8_t id, uint8_t pType, uint32_t extID = 0);
};
CANTPFrameID::CANTPFrameID() {}
CANTPFrameID::CANTPFrameID(uint8_t pType, uint8_t id, uint32_t extID) : packType(pType), identifier(id), extIdentifier(extID) {}

// CAN帧数据部分
class CANTPFrameData {
public:
    uint8_t totalFrames;  // 总包数
    uint8_t frameNumber;  // 当前包编号
    uint8_t data[CANFrameDataLength]; // 数据部分，数据包长度为MTU
};

class CANTPFrame {
public:
    CANTPFrame(CANTPFrameID fID, CANTPFrameData fData, uint8_t length);
    CANTPFrameID id;
    CANTPFrameData data;
    uint8_t dataLength;
};
CANTPFrame::CANTPFrame(CANTPFrameID fID, CANTPFrameData fData, uint8_t length) : id(fID), data(fData), dataLength(length) {}


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
    inline uint8_t *getData() { return data; }
    inline void clear() { memset(this, 0, sizeof(CANMessage)); };
#if defined(ESP32)
    inline void setExtend(bool isExtend) { isExtendPack = isExtend; }
    inline void setRemote(bool isRemote) { isRemotePack = isRemote; }
#else
    inline void setExtend(bool isExtend) { isExtendPack = isExtend?CAN_ID_EXT:CAN_ID_STD; }
    inline void setRemote(bool isRemote) { isRemotePack = isRemote?CAN_RTR_REMOTE:CAN_RTR_DATA; }
#endif
    inline void setIdentifier(uint32_t id, uint32_t extID = 0) { identifier = id; extIdentifier = extID; }
    inline void setIdentifier(CANTPFrameID &id) { identifier = (id.packType<<8)+id.identifier; extIdentifier = id.extIdentifier; }
    inline void setExtIdentifier(uint32_t extID) { extIdentifier = extID; }
    inline void setDataLength(uint32_t dataLen) { dataLength = dataLen; }
    inline bool isExtend() { return isExtendPack != 0; }
    inline bool isRemote() { return isRemotePack != 0; }
    inline uint32_t getIdentifier() { return identifier; }
    inline uint32_t getExtIdentifier() { return extIdentifier; }
    inline uint32_t getCompleteIdentifier() { return (identifier << 11) + extIdentifier; }
    inline uint8_t getDataLength() { return dataLength; }
    CANMessage &operator=(CANMessage& other){
        if (this == &other) return *this; // 防止自我赋值
        memcpy(this, &other, sizeof(CANMessage));
        return *this;
    }
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
    uint8_t data[CANFrameMaxLength - 1];
public:
    void setData(uint8_t *pData, uint8_t len) { memcpy(data, pData, len); }
    uint8_t *getData() { return data; }
};

class CANTP_PackHead { // 长数据包头
private:
    uint16_t totalLength;    //总数据长度
    uint16_t packCount;
public:
    CANTP_PackHead(uint16_t dataLength = 0, uint16_t packCount = 0) {
        this->totalLength = dataLength;
        this->packCount = packCount;
    }
    void setTotalLength(uint16_t length) { this->totalLength = length; }
    uint16_t getTotalLength() const { return totalLength; }
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
    void setData(uint8_t *data, uint16_t size){
        memcpy(this->data, data, size);
    }
    uint8_t *getData(){
        return data;
    }
};

class CANTP_MessageCodec {  //数据包编解码器
protected:
    // 接收缓存
    uint16_t rxCount;
    uint16_t rxDataSize;
    vector<uint8_t> rxData;
    // 发送缓存
    uint16_t txCounter;     //发送计数器
    uint16_t txDataSize;    //数据长度
    uint8_t *txData;        //数据地址

    uint16_t mtuSize;   //MTU大小
    uint8_t deviceID;   //设备ID
public:
    CANTP_MessageCodec(){
        txData = nullptr;
        txDataSize = 0;
        txCounter = 0;
        rxData.reserve(16);  //16个字节
    }
    void setMTUSize(uint16_t mtu){
        mtuSize = mtu;
    }
    void setDeviceID(uint8_t devID){
        deviceID = devID;
    }
    void readRXMessage(CANMessage &rxMsg){
        if(rxMsg.isRemote()) return;   //只解析数据帧
        uint32_t canIdentifier = rxMsg.getCompleteIdentifier();
        uint8_t packType = ((CANTPFrameID*)&canIdentifier)->packType;
        uint8_t packIdentifier = ((CANTPFrameID*)&canIdentifier)->identifier;
        switch(packType){   //数据包解码
            case CANTP_SHORT_DATA:
                //onReceived
                break;
            case CANTP_DATA_HEAD:{
                if(rxMsg.getDataLength() != sizeof(CANTP_PackHead)) return; //长数据包头长度错误
                CANTP_PackHead *head = (CANTP_PackHead*)rxMsg.getData();    //强制转换
                rxDataSize = head->getTotalLength();    //数据总长度
                rxCount = head->getPackCount();         //数据包数量
                rxData.clear();
                break;
            }
            case CANTP_DATA:{
                if(rxMsg.getDataLength() != sizeof(CANTP_PackData)) return; //长数据包头长度错误
                CANTP_PackData *data = (CANTP_PackData*)rxMsg.getData();    //强制转换
                for(uint8_t i = 0; i < rxMsg.getDataLength(); i++){ //遍历数据
                    rxData.push_back(data->getData()[i]);
                }
                if(data->getPackIndex() == rxCount-1){
                    //onReceived
                }
                break;
            }
            default:
                break;
        }
    }
    bool getNextTXMessage(CANMessage &txMsg){
        if(txDataSize == 0 || txData == 0) return false;    //无效数据
        if(txDataSize <= mtuSize){  //使用短数据包传输
            CANTPFrameID id(deviceID,CANTP_SHORT_DATA,0);
            txMsg.setDataLength(txDataSize);
            txMsg.readDataFrom(txData,txDataSize);
            txMsg.setRemote(false);
            txMsg.setExtend(false);
            txMsg.setIdentifier(id);
            txData = 0;
            txDataSize = 0;
        }else{  //使用长数据包传输
            if(txCounter == 0){ //如果计数器=0，则发送头帧
                CANTPFrameID id(deviceID,CANTP_DATA_HEAD,0);
                txMsg.setDataLength(sizeof(CANTP_PackHead));
                CANTP_PackHead *head = (CANTP_PackHead*)txMsg.getData();
                head->setPackCount(txDataSize / (mtuSize-2)+1);
                head->setTotalLength(txDataSize);
                txMsg.setRemote(false);
                txMsg.setExtend(false);
                txMsg.setIdentifier(id);
            }else{
                CANTPFrameID id(deviceID,CANTP_DATA,0);
                txMsg.setDataLength(sizeof(CANTP_PackHead));
                CANTP_PackData *data = (CANTP_PackData*)txMsg.getData();
                uint16_t packIndex = txCounter-1;
                uint16_t dataStartIndex = packIndex*(mtuSize-2);
                uint16_t dataEndIndex = txCounter*(mtuSize-2);
                dataEndIndex = dataEndIndex >= txDataSize ? txDataSize : dataEndIndex;
                data->setPackIndex(txCounter-1);
                data->setData(&txData[dataStartIndex],dataEndIndex-dataStartIndex);  //复制数据
                txMsg.setRemote(false);
                txMsg.setExtend(false);
                txMsg.setIdentifier(id);
                if(dataEndIndex == txDataSize){ //完成
                    txData = 0;
                    txDataSize = 0;
                    txCounter = 0;
                    return false;
                }
            }
            txCounter ++;
        }
        return true;
    }
    void startTransmission(uint8_t *data, uint16_t size){
        if(size == 0 || data == 0) return;
        txDataSize = size;
        txData = data;
        txCounter = 0;
    }
};


#pragma pack(pop)