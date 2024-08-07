#pragma once
#include <Stream.h>
#define log_print(...) LOG.print(__VA_ARGS__)
__attribute__((weak)) Stream *LOG;
bool _logLock = false;

const size_t _log_printf(const char *format, ...) {
    if(LOG == 0) return;
    while(_logLock);
    logLock = true;
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = (char *) malloc(len+1);
        if (!buffer) {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    LOG->print(buffer);
    if (buffer != temp) free(buffer);
    logLock = false;
    return len;
}

#define dblog(s,...) _log_printf("<DB>" s "</DB>", ##__VA_ARGS__)
#define dblogln(s,...) _log_printf("<DB>" s "</DB>\n", ##__VA_ARGS__)