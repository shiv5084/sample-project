#ifndef _APP_LOG_H_
#define _APP_LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_LOG_PRINT_SIZE 512

void app_format_log_printf(char* buf_ptr, int buf_size, char* fmt);

#define APP_LOG_PRINTF(lvl,...) \
{ \
    char app_log_buff[MAX_LOG_PRINT_SIZE]; \
    app_format_log_printf(app_log_buff, MAX_LOG_PRINT_SIZE, _VA_ARGS_); \
}

#define LOG_FATAL(...)  APP_LOG_PRINTF(MSG_LEGACY_FATAL, _VA_ARGS_)
#define LOG_ERROR(...)  APP_LOG_PRINTF(MSG_LEGACY_FATAL, _VA_ARGS_)
#define LOG_DEBUG(...)  APP_LOG_PRINTF(MSG_LEGACY_FATAL, _VA_ARGS_)
#define LOG_INFO(...)   APP_LOG_PRINTF(MSG_LEGACY_FATAL, _VA_ARGS_)

#endif 