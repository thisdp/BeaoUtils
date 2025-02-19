#pragma once
#include <Stream.h>
#include <Arduino.h>
#include "../FIFO/FIFO.h"
#include <string>
class Logger{
public:
    Logger();
    Logger(Stream &stream);
    void setStream(Stream &stream);
    size_t printf(const char *format, ...);
    size_t printfln(const char *format, ...);
    size_t printf_lock(const char *format, ...);
    size_t printfln_lock(const char *format, ...);
    void print(const char *str);
    void println(const char *str);
protected:
    Stream *_logStream;
    bool _logLock;
};

class LoggerKeeper;
typedef void(*LoggerKeeperOnLog)(LoggerKeeper *logger,const char *str);
class LoggerKeeper : public Logger{
public:
    LoggerKeeper(uint16_t size);
    LoggerKeeper(Stream &stream,uint16_t size);
    void setLoggerKeepSize(uint16_t size);
    size_t printf(const char *format, ...);
    size_t printfln(const char *format, ...);
    size_t printf_lock(const char *format, ...);
    size_t printfln_lock(const char *format, ...);
    void print(const char *str);
    void println(const char *str);
    int available();
    String read();
    String peek(uint32_t index);
    LoggerKeeperOnLog onLog;
protected:
    DynamicFIFO<String> _loggerFIFO;
};