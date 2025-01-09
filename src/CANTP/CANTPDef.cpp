#include "CANTPDef.h"
Logger globalCANTPLogger;

bool setCANTPDebugStream(Stream &stream){
    globalCANTPLogger.setStream(stream);
    return true;
}

CANConfig::CANConfig(uint8_t _mtu, uint8_t _canType) : mtu(_mtu), canType(_canType) {}
CANConfig::CANConfig() : mtu(0), canType(0) {}

void CANConfig::setConfig(uint8_t mtu, uint8_t canType) {
    this->mtu = mtu;
    this->canType = canType;
}

uint8_t CANConfig::getConfig() const {
    return config;
}

CANTPFrameID::CANTPFrameID() {}

CANTPFrameID::CANTPFrameID(uint8_t pType, uint8_t id, uint32_t extID) 
    : packType(pType), identifier(id), extIdentifier(extID), reserved(0) {}

void CANTPFrameID::setIdentifier(uint32_t standardID, uint32_t extensiveID) {
    stdID = standardID;
    extID = extensiveID;
}

uint32_t CANTPFrameID::getStdIdentifier(){
    return stdID;
}
uint32_t CANTPFrameID::getExtIdentifier(){
    return extID;
}
uint32_t CANTPFrameID::getIdentifier(){
    return frameIdentifier;
}

CANTPFrame::CANTPFrame(CANTPFrameID fID, CANTPFrameData fData, uint8_t length) 
    : id(fID), data(fData), dataLength(length) {}

void CANTPShortPack::setData(uint8_t *pData, uint8_t len) {
    memcpy(data, pData, len);
}

uint8_t* CANTPShortPack::getData() {
    return data;
}

CANTPPackHead::CANTPPackHead(uint16_t dataLength, uint16_t packCount) 
    : totalLength(dataLength), packCount(packCount) {}

void CANTPPackHead::setTotalLength(uint16_t length) {
    this->totalLength = length;
}

uint16_t CANTPPackHead::getTotalLength() const {
    return totalLength;
}

void CANTPPackHead::setPackCount(uint16_t packCount) {
    this->packCount = packCount;
}

uint16_t CANTPPackHead::getPackCount() const {
    return packCount;
}

CANTPPackData::CANTPPackData(uint16_t packIndex, uint8_t *data) 
    : packIndex(packIndex) {
    if(data != nullptr) memcpy(this->data, data, CANFrameDataLength);
}

void CANTPPackData::setPackIndex(uint16_t packIndex) {
    this->packIndex = packIndex;
}

uint16_t CANTPPackData::getPackIndex() {
    return this->packIndex;
}

void CANTPPackData::setData(uint8_t *data, uint16_t size){
    memcpy(this->data, data, size);
}

uint8_t* CANTPPackData::getData(){
    return data;
}