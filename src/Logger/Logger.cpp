#include <Stream.h>
#include "Logger.h"

Logger::Logger(){
    _logStream = 0;
    _logLock = false;
}

Logger::Logger(Stream &stream){
    setStream(stream);
    _logLock = false;
}
void Logger::setStream(Stream &stream){
    _logStream = &stream;
}

size_t const Logger::printf(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    _logStream->print(buffer);
    if (buffer != temp) free(buffer);
    return len;
}


size_t const Logger::printfln(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    _logStream->println(buffer);
    if (buffer != temp) free(buffer);
    return len;
}

size_t const Logger::printf_lock(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    while(_logLock);
    _logLock = true;
    _logStream->print(buffer);
    _logLock = false;
    if (buffer != temp) free(buffer);
    return len;
}

size_t const Logger::printfln_lock(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    while(_logLock);
    _logLock = true;
    _logStream->println(buffer);
    _logLock = false;
    if (buffer != temp) free(buffer);
    return len;
}

void const Logger::print(const char *str){
    if(!_logStream) return;
    _logStream->print(str);
}

void const Logger::println(const char *str){
    if(!_logStream) return;
    _logStream->println(str);
}

/*Logger Keeper*/
LoggerKeeper::LoggerKeeper(uint16_t size) : _loggerFIFO(size), onLog(0) {}
LoggerKeeper::LoggerKeeper(Stream &stream, uint16_t size): Logger(stream), _loggerFIFO(size), onLog(0) {}

void LoggerKeeper::setLoggerKeepSize(uint16_t size){
    _loggerFIFO.resize(size);
}

int LoggerKeeper::available(){
    return _loggerFIFO.length();
}

String LoggerKeeper::read(){
    if(_loggerFIFO.length() == 0) return "";
    String *str;
    if(!_loggerFIFO.dequeue(str)) return "";
    return *str;
}

String LoggerKeeper::peek(uint32_t index){
    if(_loggerFIFO.length() == 0) return "";
    String *str;
    if(!_loggerFIFO.peek(str,index)) return "";
    return *str;
}

size_t const LoggerKeeper::printf(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    _logStream->print(buffer);
    String str(buffer);
    _loggerFIFO.enqueue(str);
    if (onLog != 0) onLog(this,buffer);
    if (buffer != temp) free(buffer);
    return len;
}


size_t const LoggerKeeper::printfln(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    _logStream->println(buffer);
    String str(buffer);
    _loggerFIFO.enqueue(str);
    if (onLog != 0) onLog(this,buffer);
    if (buffer != temp) free(buffer);
    return len;
}

size_t const LoggerKeeper::printf_lock(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    while(_logLock);
    _logLock = true;
    _logStream->print(buffer);
    String str(buffer);
    _loggerFIFO.enqueue(str);
    if (onLog != 0) onLog(this,buffer);
    _logLock = false;
    if (buffer != temp) free(buffer);
    return len;
}

size_t const LoggerKeeper::printfln_lock(const char *format, ...){
    if(!_logStream) return 0;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) return 0;
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    while(_logLock);
    _logLock = true;
    _logStream->println(buffer);
    String str(buffer);
    _loggerFIFO.enqueue(str);
    if (onLog != 0) onLog(this,buffer);
    _logLock = false;
    if (buffer != temp) free(buffer);
    return len;
}

void const LoggerKeeper::print(const char *str){
    if(!_logStream) return;
    _logStream->print(str);
    String bStr(str);
    _loggerFIFO.enqueue(bStr);
    if (onLog != 0) onLog(this,str);
}

void const LoggerKeeper::println(const char *str){
    if(!_logStream) return;
    _logStream->println(str);
    String bStr(str);
    _loggerFIFO.enqueue(bStr);
    if (onLog != 0) onLog(this,str);
}