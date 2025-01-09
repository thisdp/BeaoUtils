#include "CANTPClient.h"

// 构造函数
CANTPClient::CANTPClient(HardwareCAN &can) b
    : hwCAN(&can),
      reconnectTimer(1000),
      askConnectTimeOutTimer(500),
      clientSyncTimer(1000),
      serverSyncTimeOutTimer(2500),
      connectingTimeOutTimer(5000),
      receiveTimes(0),
      hbTimes(0) {}

// 初始化方法
void CANTPClient::begin() {
    connection = CANTPConnState::DISCONNECTED;
    deviceID = 0;
    canConf.canType = CANType_CAN20B;
    canConf.mtu = 0; // 8 Bytes
    uniqueIDFragmentAppendLength = 0;
    reconnectTimer.start();
}

// 处理接收到的消息
void CANTPClient::onMessageReceived() {
    receiveTimes++;
    uint32_t canIdentifier = rxMsg.getCompleteIdentifier();
    uint8_t packType = ((CANTPFrameID*)&canIdentifier)->packType;
    uint8_t packIdentifier = ((CANTPFrameID*)&canIdentifier)->identifier;

    if (rxMsg.isRemote()) { // 远程帧
        switch (packType) {
        case CANTP_HEARTBEAT_SERVER: // 当接收到服务端心跳包
            serverSyncTimeOutTimer.start(); // 重置服务端同步定时器
            break;
        }
    } else { // 数据帧
        switch (packType) {
        case CANTP_CONNECT_ALLOW: {
            setConnectionState(CANTPConnState::CONNECTING);
            for (uint8_t i = 0; i < uniqueID.getLength(); i++) {
                txMsg.clear();
                CANTPFrameID frameIdentifier;
                frameIdentifier.setPackType(CANTP_UID_ARBITRATION);
                frameIdentifier.setDeviceID(uniqueID.getRaw(i));
                txMsg.setStdIdentifier(frameIdentifier.getStdIdentifier());
                txMsg.setDataLength(0);
                txMsg.setExtend(false);
                txMsg.setRemote(true);
                hwCAN->send(txMsg);
            }

            CANConfig conf(0, CANType_CAN20B);
            txMsg.clear();
            CANTPFrameID frameIdentifier(CANTP_CONFIGURE, conf.getConfig());
            txMsg.setStdIdentifier(frameIdentifier.getStdIdentifier());
            txMsg.setDataLength(0);
            txMsg.setExtend(false);
            txMsg.setRemote(true);
            hwCAN->send(txMsg);

            txMsg.clear();
            frameIdentifier.setPackType(CANTP_EXPECTING_CAN_ID);
            frameIdentifier.setDeviceID(deviceID);
            txMsg.setStdIdentifier(frameIdentifier.getStdIdentifier());
            txMsg.setDataLength(0);
            txMsg.setExtend(false);
            txMsg.setRemote(true);
            hwCAN->send(txMsg);
            break;
        }
        case CANTP_ASSIGNED_ID: {
            if (getConnectionState() != CANTPConnState::CONNECTING) return;
            if (packIdentifier == 0 || (deviceID != 0 && packIdentifier != deviceID && !reassignIfIDConflict)) {
                txMsg.clear();
                CANTPFrameID frameIdentifier(CANTP_DISCONNECT, packIdentifier);
                txMsg.setStdIdentifier(frameIdentifier.getStdIdentifier());
                txMsg.setDataLength(0);
                txMsg.setExtend(false);
                txMsg.setRemote(true);
                hwCAN->send(txMsg);
                setConnectionState(CANTPConnState::DISCONNECTED);
                return;
            }
            setDeviceID(packIdentifier);
            txMsg.clear();
            CANTPFrameID frameIdentifier(CANTP_CONNECT, packIdentifier);
            txMsg.setStdIdentifier(frameIdentifier.getStdIdentifier());
            txMsg.setDataLength(0);
            txMsg.setExtend(false);
            txMsg.setRemote(true);
            hwCAN->send(txMsg);
            break;
        }
        case CANTP_CONNECTION_DONE:
            if (getConnectionState() != CANTPConnState::CONNECTING) return;
            setConnectionState(CANTPConnState::CONNECTED);
            break;
        case CANTP_CONNECTION_LOST:
            setConnectionState(CANTPConnState::DISCONNECTED);
            break;
        case CANTP_SHORT_DATA:
        case CANTP_DATA_HEAD:
        case CANTP_DATA:
            // Implement handling for short data packets, data headers, and data content as necessary.
            break;
        }
    }
}

// 尝试加入总线
void CANTPClient::tryToJoinBus() {
    setConnectionState(CANTPConnState::ASKINGCONNECT);
    askConnectTimeOutTimer.start();
    txMsg.clear();
    CANTPFrameID frameIdentifier(CANTP_CONNECT_REQUEST, deviceID);
    txMsg.setStdIdentifier(frameIdentifier.getStdIdentifier());
    txMsg.setDataLength(0);
    txMsg.setExtend(false);
    txMsg.setRemote(true);
    hwCAN->send(txMsg);
}

// 内部连接状态改变时触发
void CANTPClient::onInternalConnectionStateChange() {
    if (getConnectionState() == CANTPConnState::DISCONNECTED) {
        reconnectTimer.start();
    } else if (getConnectionState() == CANTPConnState::CONNECTING) {
        connectingTimeOutTimer.start();
    }
}

// 更新方法
void CANTPClient::update() {
    hwCAN->update();
    while (hwCAN->available()) {
        hwCAN->receive(rxMsg);
        onMessageReceived();
    }

    switch (connection) {
    case CANTPConnState::DISCONNECTED:
        if (reconnectTimer.checkTimedOut()) {
            reconnectTimer.stop();
            tryToJoinBus();
        }
        break;
    case CANTPConnState::ASKINGCONNECT:
        if (askConnectTimeOutTimer.checkTimedOut()) {
            setConnectionState(CANTPConnState::DISCONNECTED);
            askConnectTimeOutTimer.stop();
        }
        break;
    case CANTPConnState::CONNECTING:
        if (connectingTimeOutTimer.checkTimedOut()) {
            setConnectionState(CANTPConnState::DISCONNECTED);
            connectingTimeOutTimer.stop();
        }
        break;
    case CANTPConnState::CONNECTED:
        if (hwCAN->isTXEmpty()) {
            hbTimes++;
            if (clientSyncTimer.checkTimedOut()) {
                clientSyncTimer.start();
                txMsg.clear();
                CANTPFrameID frameIdentifier(CANTP_HEARTBEAT_CLIENT, deviceID);
                txMsg.setStdIdentifier(frameIdentifier.getStdIdentifier());
                txMsg.setDataLength(0);
                txMsg.setExtend(false);
                txMsg.setRemote(true);
                hwCAN->send(txMsg);
            }
            if (serverSyncTimeOutTimer.checkTimedOut()) {
                setConnectionState(CANTPConnState::DISCONNECTED);
            }
        }
        break;
    }
}

// 发送数据帧
void CANTPClient::send(uint8_t packType, uint8_t* data, uint8_t length) {
    // Implementation of sending a data frame with the specified type and payload.
}

void CANTPClient::onReceived(){
    
}