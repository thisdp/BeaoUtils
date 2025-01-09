#include "HardwareCAN.h"

uint8_t CANLengthCode[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
/***********************************CAN数据缓存区 */
CANMessageBuffer::CANMessageBuffer()
    : data(nullptr), bufferLength(0), usingLength(0) {}

CANMessageBuffer::~CANMessageBuffer() {
    if (data) delete[] data;
}

void CANMessageBuffer::resize(uint16_t size) {
    if (data) delete[] data;
    data = new uint8_t[size];
    bufferLength = size;
    usingLength = 0;
}

void CANMessageBuffer::requestSpace(uint16_t size) {
    if (data == nullptr || bufferLength != size)
        resize(size);
}

void CANMessageBuffer::setData(uint8_t* argData, uint16_t argLength) {
    requestSpace(argLength);
    memcpy(data, argData, argLength);
    usingLength = argLength;
}

uint8_t* CANMessageBuffer::getData() {
    return data;
}

void CANMessageBuffer::clear() {
    if (data) memset(data, 0, bufferLength);
    usingLength = 0;
}

uint16_t CANMessageBuffer::getLength() {
    if (!data) return 0;
    if (usingLength > bufferLength) return bufferLength;
    return usingLength;
}
/************************CAN Message实现 */
uint16_t CANMessage::lengthCodeToLength(uint8_t dlc, uint16_t maxSize){
    if(dlc > sizeof(CANLengthCode)/sizeof(CANLengthCode[0])) return maxSize;
    return CANLengthCode[dlc];
}
uint16_t CANMessage::lengthToLengthCode(uint16_t length, uint16_t maxSize){
    if(length <= 8) return length;
    //if(length <= 32) return (((length-8)+3)/4)+8;
    if(length <= 32) return (length-5)/4+8;
    if(length <= 48) return 48;
    return maxSize;
}
/************************Hardware CAN实现 */
// 构造函数实现
#if defined(ESP32)
HardwareCAN::HardwareCAN(TWAICAN &argCan) : can(&argCan), rxMessage(32), txMessage(32), isInited(false) {}
#else
HardwareCAN::HardwareCAN(CAN_HandleTypeDef &argCan) : can(&argCan), rxMessage(32), txMessage(32), isInited(false) {}
#endif

void HardwareCAN::begin(uint16_t maxLength, uint32_t baud, uint32_t baudData){
    rxTempMessage.data.requestSpace(maxLength);
    txTempMessage.data.requestSpace(maxLength);
    baudrate = baud;
    baudrateData = baudData;
}

bool HardwareCAN::receive(CANMessage &msg) {
    if (rxMessage.isEmpty()) return false;
    rxMessage.dequeue(msg);
    return true;
}

bool HardwareCAN::send(uint32_t identifier, uint8_t* data, uint8_t length, bool isRemote, bool isExtend, uint32_t extIdentifier) {
    if (!availableForWrite()) return false;
    txTempMessage.setStdIdentifier(identifier);
    txTempMessage.setExtIdentifier(extIdentifier);
    txTempMessage.setExtend(isExtend);
    txTempMessage.setRemote(isRemote);
    txTempMessage.setDataLength(length);
    txTempMessage.readDataFrom(data, length);
    txMessage.enqueue(txTempMessage);
    return true;
}

bool HardwareCAN::send(CANMessage &msg) {
    if (!availableForWrite()) return false;
    txMessage.enqueue(msg);
    return true;
}


void HardwareCAN::doReceive() {
#if defined(ESP32)
// ESP32-specific implementation if any
#else
    if (HAL_CAN_GetRxFifoFillLevel(can, CAN_RX_FIFO0) > 0) { // 检查是否有消息
        if (!rxMessage.isFull()) {
            CAN_RxHeaderTypeDef rxHead;
            HAL_CAN_GetRxMessage(can, CAN_RX_FIFO0, &rxHead, rxTempMessage.getData());
            rxTempMessage.readHeadFrom(&rxHead);
            rxMessage.enqueue(rxTempMessage);
        }
    }
#endif
}
void HardwareCAN::doSend() {
#if defined(ESP32)
// ESP32-specific implementation if any
#else
    if (HAL_CAN_GetTxMailboxesFreeLevel(can) > 0) { // 检查是否有空闲邮箱
        if (!txMessage.isEmpty()) {
            CANMessage* txMsg;
            if (txMessage.dequeue(txMsg)) {
                CAN_TxHeaderTypeDef txHead;
                memset(&txHead, 0, sizeof(txHead));
                txMsg->writeHeadTo(&txHead);
                uint32_t txUsingMailBox;
                HAL_CAN_AddTxMessage(can, &txHead, txMsg->getData(), &txUsingMailBox);
            }
        }
    }
#endif
}

void HardwareCAN::update() {

    doReceive();
    doSend();
}