#include "app_log.h"

void app_format_log_printf(char* buf_ptr, int buf_size, char* fmt)
{
    int len = 0;
    va_list arg_list;

    if(buf_ptr == NULL || buf_size < 0)
    {
        return;
    }

    va_start(arg_list,fmt);
    len = vsnprintf((char*)buf_ptr, (size_t)buf_size,(char*)fmt,arg_list);
    va_end(arg_list);

    buff_ptr[buf_size] = 0;  

    return;  
}