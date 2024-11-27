#pragma once
#include <Stream.h>
#define log_print(...) LOG.print(__VA_ARGS__)
#ifdef LOG
    bool _logLock = false;

    const size_t _log_printf(const char *format, ...) {
        if(LOG == 0) return 0;
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
        LOG.print(buffer);
        if (buffer != temp) free(buffer);
        _logLock = false;
        return len;
    }

    const size_t _log_printf_lock(const char *format, ...){
        if(LOG == 0) return 0;
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
        while(_logLock);
        _logLock = true;
        LOG.print(buffer);
        _logLock = false;
        if (buffer != temp) free(buffer);
        return len;
    }

    #define dblog(s,...) _log_printf("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dblogln(s,...) _log_printf("<DB>" s "</DB>\n", ##__VA_ARGS__)
    #define dbloglk(s,...) _log_printf_lock("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dbloglnlk(s,...) _log_printf_lock("<DB>" s "</DB>\n", ##__VA_ARGS__)
#endif