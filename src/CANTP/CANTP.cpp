#include "CANTP.h"
#include <iostream>
#include <cstring>

// 清空接收缓冲区
void CANTPBuffer::clear() {
    data.clear();
    receivedFrames = 0;
    byteCount = 0;  // 清空时重置字节计数
    currentMsgId = 0;  // 重置消息ID
}

// 初始化接收缓冲区
bool CANTPBuffer::init(CANTPFrame& frame) {
    if (receivedFrames > 0 && frame.id.identifier != currentMsgId) return false; // 如果已经开始接收另一个消息则返回false
    clear();
    totalFrames = frame.data.totalFrames;
    currentMsgId = frame.id.identifier;  // 设置当前消息ID
    for (size_t i = 0; i < frame.dataLength; ++i) {
        data.push_back(frame.data.data[i]);
        byteCount++;  // 更新字节计数
    }
    receivedFrames++;
    return true;
}

// 添加帧到接收缓冲区
bool CANTPBuffer::append(CANTPFrame& frame) {
    if (frame.id.identifier != currentMsgId) return false;  // 检查消息ID是否匹配
    for (size_t i = 0; i < frame.dataLength; ++i) {
        data.push_back(frame.data.data[i]);
        byteCount++;  // 更新字节计数
    }
    receivedFrames++;
    return true;
}

// 打印缓冲区的内容
void CANTPBuffer::print() {
    std::cout << "RX Frame:" << "Total Frames: " << (int)totalFrames
        << ", Received Frames: " << (int)receivedFrames
        << ", Byte Count: " << (int)byteCount << std::endl;
    std::cout << "  Data: ";
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << " " << std::hex << (int)data[i] << " ";
    }
    std::cout << std::dec << std::endl;  // 切换回十进制
}



// 发送单个帧
void CANTP::sendDataFrame(CANTPFrame& frame) {
    // 模拟发送帧，实际中应替换为真实发送逻辑
    std::cout << "TX Frame: PackType=" << (int)frame.id.packType
        << ", Identifier=" << (int)frame.id.identifier
        << ", TotalFrames=" << (int)frame.data.totalFrames
        << ", FrameNumber=" << (int)frame.data.frameNumber
        << ", DataLength=" << (int)frame.dataLength
        << std::endl;
    twai_message_t twaimsg;
    twaimsg.extd = 0;
    twaimsg.identifier = *((uint16_t*)&(frame.id));
    twaimsg.data_length_code = frame.dataLength;
    twaimsg.rtr = 0;
    *((uint64_t*)twaimsg.data) = *((uint64_t*)&(frame.data));
    if (fifo != 0)
        fifo->push(twaimsg);
}

// 接收单个帧
bool CANTP::receiveDataFrame(CANTPFrame& frame) {
    if (frame.data.frameNumber == 0) {
        if (!rcvBuffer.init(frame)) return false;  // 初始化失败
    }
    else {
        if (!rcvBuffer.append(frame)) return false;  // 附加失败
    }

    // 检查是否所有帧都已接收到
    return (rcvBuffer.receivedFrames == rcvBuffer.totalFrames);
}

// 接收单个帧
bool CANTP::receiveRemoteFrame(CANTPFrameID& frameID) {
    handleRemoteFrame(frameID);
    return (rcvBuffer.receivedFrames == rcvBuffer.totalFrames);
}