#ifndef _APP_UTILS_MISC_H_
#define _APP_UTILS_MISC_H_

#include <stdint.h>

/*********************************
 wait and signal utility
 ********************************/
int app_utils_init_signal(TX_EVENT_FLAGS_GROUP** signal, const char* signalName);
int app_utils_deinit_signal(TX_EVENT_FLAGS_GROUP* signal);
int app_utils_wait_on_signal(TX_EVENT_FLAGS_GROUP* signal, TX_MUTEX* mutex);
void app_utils_set_signal(TX_EVENT_FLAGS_GROUP* signal, TX_MUTEX* mutex);

/*********************************
 Mutex Utility
 ********************************/
int app_utils_mutex_init(TX_MUTEX** mutex, char* mutexName);
int app_utils_mutex_get(TX_MUTEX* mutex);
int app_utils_mutex_put(TX_MUTEX* mutex);
int app_utils_mutex_delete(TX_MUTEX* mutex);

#endif 