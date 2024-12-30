#include "SerialCommandHandler.h"
#include <stdlib.h>
bool SerialCommand::hasCommand(){
	return def != 0;
}
void SerialCommand::clear(){
	def = 0;
	blocks.clear();
}
bool SerialCommand::parseCommand(String cmdBuffer) {
	const char* s = cmdBuffer.c_str();
	uint32_t strLength = cmdBuffer.length();
	while (strLength) {
		//从末尾寻找最后一位有效参数
		char c = s[strLength - 1];
		if (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
			strLength--;
			continue;
		}
		break;
	}
	if (strLength == 0) return false; //不存在有效命令
	uint32_t readIndex = 0;
	uint32_t startIndex = 0;
	while (readIndex <= strLength) {
		if (readIndex < strLength) {
			if (s[readIndex] == ' ') {
				blocks.push_back(cmdBuffer.substring(startIndex, readIndex));
				while (s[readIndex] == ' ' || s[readIndex] == '\t') {
					readIndex++;
					if (readIndex == strLength) return true; //解析完成
				}
				startIndex = readIndex;
			}
			else {
				readIndex++;
			}
		} else {
			blocks.push_back(cmdBuffer.substring(startIndex, readIndex));
			return true;
		}
	}
	return false;
}
const char *SerialCommand::getCommandName(){
	if(!hasCommand()) return "";
	return def->cmd;
}
const char *SerialCommand::getCommandDescription(){
	if(!hasCommand()) return "";
	return def->description;
}

//----------------------------------------------------------------

SerialCommandHandler::SerialCommandHandler(Stream *parSerial, Stream *parDebugSerial){
	serial = parSerial;
	debugSerial = parDebugSerial?parDebugSerial:parSerial;
	maxCommandTimeout = SCP_COMMAND_TIMEOUT_MILLIS;
	commandCount = 0;
	isReadingCommand = false;
	commandReadingUpdateTick = 0;
	clearSerialCommand();
}

bool SerialCommandHandler::addCommand(const char *cmd, const char * parDescription, SCP_Callback parFunction){
	if (commandCount>=SCP_COMMAND_ARRAY_MAX_SIZE-1){
		errorMessage("Array of commands is full");
		return false;
	}
	commandsArray[commandCount].cmd = cmd;
	commandsArray[commandCount].description = parDescription;
	commandsArray[commandCount].cbFunction = parFunction;
	commandCount++;
	return true;
}

void SerialCommandHandler::errorMessage(const char * parErrorMessage){
	debugSerial->print("[SCP ERROR]");
	debugSerial->println(parErrorMessage);
}

void SerialCommandHandler::printCommandList(){
	debugSerial->print("\n-= Available commands =-\n");
	for (uint32_t i = 0; i <commandCount; i++){
		debugSerial->write('\'');
		debugSerial->print(commandsArray[i].cmd);
		debugSerial->print("' :\t");
		debugSerial->println(commandsArray[i].description);
	}
	debugSerial->println("-----------------------");
}

void SerialCommandHandler::showCurrentCommand(bool showDescription){
	if(!currentCommand.hasCommand()){
		debugSerial->println("No Serial Command");
		return;
	}
	debugSerial->print("Serial Command > ");
	debugSerial->println(currentCommand.getCommandName());
	if(showDescription) debugSerial->println(currentCommand.getCommandDescription());
}

void SerialCommandHandler::clearSerialCommand(){
	currentCommand.clear();
}

const char *SerialCommandHandler::getCurrentCommandName(){
	return currentCommand.getCommandName();
}

const char *SerialCommandHandler::getCurrentCommandDescription(){
	return currentCommand.getCommandDescription();
}

void SerialCommandHandler::update(){
	uint32_t ms = millis();
	while(serial->available()) {
		char newChar = serial->read();
		if(!isReadingCommand) currentCommand.clear();
		isReadingCommand = true;
		commandReadingUpdateTick = ms;
		commandReadBuffer += newChar;
		if(newChar == '\n'){
			if(currentCommand.parseCommand(commandReadBuffer)){
				if(processSerialCommand()){
					debugSerial->print("Serial Command > ");
					debugSerial->println(currentCommand.getCommandName());
					currentCommand.def->cbFunction(*this);
				}else{
					debugSerial->print("Unknown Serial Command > ");
					debugSerial->println(commandReadBuffer);
				}
			}else{
				debugSerial->println("Invalid Command");
			}
			commandReadBuffer = String("");
			isReadingCommand = false;
		}
	}
	if(isReadingCommand && ms-commandReadingUpdateTick >= maxCommandTimeout){
		commandReadBuffer = String("");
		isReadingCommand = false;
	}
}

bool SerialCommandHandler::processSerialCommand(){
	for(uint8_t i=0;i<commandCount;i++){
		if(currentCommand.blocks.at(0).equals(commandsArray[i].cmd)){	//查询匹配的命令
			currentCommand.def = &(commandsArray[i]);
			currentCommand.blocks.erase(currentCommand.blocks.begin());	//移除第一项（命令项）
			return true;
		}
	}
	return false;
}

bool SerialCommandHandler::getParameterInteger(int32_t index, int32_t &value){
	if(!currentCommand.hasCommand()) return false;
	if(index >= ((int32_t)currentCommand.blocks.size())) return false;
#if defined(ESP32)
	try {
#endif
	value = stol(currentCommand.blocks.at(index).c_str());
#if defined(ESP32)
	} catch (...) { return false; }
#endif
	return true;
}

bool SerialCommandHandler::getParameterUnsignedInteger(int32_t index, uint32_t &value){
	if(!currentCommand.hasCommand()) return false;
	if(index >= ((int32_t)currentCommand.blocks.size())) return false;
#if defined(ESP32)
	try {
#endif
	value = stoul(currentCommand.blocks.at(index).c_str());
#if defined(ESP32)
	} catch (...) { return false; }
#endif
	return true;
}


bool SerialCommandHandler::getParameterFloat(int32_t index, float &value){
	if(!currentCommand.hasCommand()) return false;
	if(index >= ((int32_t)currentCommand.blocks.size())) return false;
#if defined(ESP32)
	try {
#endif
	value = atof(currentCommand.blocks.at(index).c_str());
#if defined(ESP32)
	} catch (...) { return false; }
#endif
	return true;
}

bool SerialCommandHandler::getParameterChar(int32_t index, char &value){
	if(!currentCommand.hasCommand()) return false;
	if(index >= ((int32_t)currentCommand.blocks.size())) return false;
	value = currentCommand.blocks.at(index)[0];
	return true;
}

bool SerialCommandHandler::getParameterString(int32_t index, String &value){
	if(!currentCommand.hasCommand()) return false;
	if(index >= ((int32_t)currentCommand.blocks.size())) return false;
	value = currentCommand.blocks.at(index);
	return true;
}