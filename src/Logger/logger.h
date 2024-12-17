#pragma once
#include <Stream.h>
#include <Arduino.h>
class Logger{
public:
    Logger();
    Logger(Stream &stream);
    void setStream(Stream &stream);
    const size_t printf(const char *format, ...);
    const size_t printfln(const char *format, ...);
    const size_t printf_lock(const char *format, ...);
    const size_t printfln_lock(const char *format, ...);
    const void print(const char *str);
    const void println(const char *str);
private:
    Stream *_logStream;
    bool _logLock;
};