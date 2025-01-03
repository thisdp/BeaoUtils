#pragma once
#include <Stream.h>
#include "../Logger/Logger.h"
#define log_print(...) LOG.print(__VA_ARGS__)
#if defined(LOG)
    Logger dbLogger(LOG);
    #define dblog(s,...) dbLogger.printf("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dblogln(s,...) dbLogger.printf("<DB>" s "</DB>\n", ##__VA_ARGS__)
    #define dbloglk(s,...) dbLogger.printf_lock("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dbloglnlk(s,...) dbLogger.printf_lock("<DB>" s "</DB>\n", ##__VA_ARGS__)
#elif defined(LOGKEEPER)
    #ifdef LOGKEEPER_SIZE
    LoggerKeeper dbLoggerKeeper(LOGKEEPER,LOGKEEPER_SIZE);
    #else
    LoggerKeeper dbLoggerKeeper(LOGKEEPER,100);
    #endif
    #define dblog(s,...) dbLoggerKeeper.printf("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dblogln(s,...) dbLoggerKeeper.printf("<DB>" s "</DB>\n", ##__VA_ARGS__)
    #define dbloglk(s,...) dbLoggerKeeper.printf_lock("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dbloglnlk(s,...) dbLoggerKeeper.printf_lock("<DB>" s "</DB>\n", ##__VA_ARGS__)
#endif