#pragma once
#include <Arduino.h>
#include <string>
#include <vector>
using namespace std;
#define SCP_COMMAND_ARRAY_MAX_SIZE 64
#define SCP_COMMAND_TIMEOUT_MILLIS 500

class SerialCommandHandler;
using SCP_Callback = function<void(SerialCommandHandler &)>;
class SerialCommandDefinition{	
public:
	const char* cmd;
	const char* description;
	SCP_Callback cbFunction;
};

class SerialCommand{
public:
	SerialCommandDefinition *def;
	vector<String> blocks;
	bool hasCommand();
	void clear();
	bool parseCommand(String cmdBuffer);
	const char *getCommandName();
	const char *getCommandDescription();
};

class SerialCommandHandler{
public:
	SerialCommandHandler(Stream *parSerial, Stream *parDebugSerial = 0);
	bool addCommand(const char *parChar, const char * parDescription, SCP_Callback cbFunction);
	void printCommandList();
	void showCurrentCommand(bool showCurrentCommand = false);
	void errorMessage(const char * parErrorMessage);
	
	bool processSerialCommand();	
	void clearSerialCommand();
	void update();
	
	bool getParameterInteger(int32_t index, int32_t &value);
	bool getParameterUnsignedInteger(int32_t index, uint32_t &value);
	bool getParameterFloat(int32_t index, float &value);
	bool getParameterString(int32_t index, String &value);
	bool getParameterChar(int32_t index, char &value);
	const char *getCurrentCommandName();
	const char *getCurrentCommandDescription();
	SerialCommand currentCommand;
private:
	Stream *serial;
	Stream *debugSerial;
	SerialCommandDefinition commandsArray[SCP_COMMAND_ARRAY_MAX_SIZE];
	uint32_t commandCount;

	bool isReadingCommand;
	uint32_t commandReadingUpdateTick;
	String commandReadBuffer;
	uint32_t maxCommandTimeout;
};
