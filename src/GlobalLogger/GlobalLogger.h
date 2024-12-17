#pragma once
#include <Stream.h>
#include "../Logger/Logger.h"
#define log_print(...) LOG.print(__VA_ARGS__)
#ifdef LOG
    Logger dbLogger(LOG);
    #define dblog(s,...) dbLogger.printf("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dblogln(s,...) dbLogger.printf("<DB>" s "</DB>\n", ##__VA_ARGS__)
    #define dbloglk(s,...) dbLogger.printf_lock("<DB>" s "</DB>", ##__VA_ARGS__)
    #define dbloglnlk(s,...) dbLogger.printf_lock("<DB>" s "</DB>\n", ##__VA_ARGS__)
#endif