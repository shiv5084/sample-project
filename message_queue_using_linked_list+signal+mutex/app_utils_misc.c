#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "app_message_queue.h"
#include "app_utils_misc.h"
#include "app_log.h"

/******************************
 Signal utility
 *****************************/
int app_utils_init_signal(TX_EVENT_FLAGS_GROUP** signal, const char* signalName)
{
    txm_module_object_allocate((void**)signal, sizeof(TX_EVENT_FLAGS_GROUP));
    
    return tx_event_flags_create(*signal, (char*)signalName);
}

int app_utils_deinit_signal(TX_EVENT_FLAGS_GROUP* signal)
{
    uint32_t ret = tx_event_flags_delete(signal);
    tmx_module_object_deallocate(signal);

    return ret;
}

int app_utils_wait_on_signal(TX_EVENT_FLAGS_GROUP* signal, TX_MUTEX* mutex)
{
    ULONG setSignal = 0;
    UINT old_threshold, dummy;
    TX_THREAD* thread;

    /*find the current thread and raise its preemption
    threshold so it does not get de-scheduled*/
    thread = tx_thread_identify();
    tx_thread_preemption_change(thread,0,&old_threshold);

    app_utils_mutex_put(mutex);
    int ret = tx_event_flags_get(signal,1,TX_OR_CLEAR,
                                      &setSignal,TX_WAIT_FOREVER);
    /*restore original preemption threshold and lock the mutex*/
    tx_thread_preemption_change(thread,old_threshold,&dummy);
    app_utils_mutex_get(mutex);  

    return ret;                                
}

void app_utils_set_signal(TX_EVENT_FLAGS_GROUP* signal, TX_MUTEX* mutex)
{
    app_utils_mutex_get(mutex);
    tx_event_flags_set(signal,1,TX_OR);
    app_utils_mutex_put(mutex);
}

/***********************************
Mutex utility
***********************************/
int app_utils_mutex_init(TX_MUTEX** mutex, char* mutexName)
{
    int ret = -1;

    if(NULL == mutex)
    {
        LOG_ERROR("invalid mutex object");
        return ret;
    }
    if(0 != txm_module_object_allocate((void**)mutex, sizeof(TX_MUTEX)))
    {
        LOG_ERROR("object allocation for mutex failed");
        return ret;
    }
    if(0 != tx_mutex_create(*mutex,mutexName,TX_NO_INHERIT))
    {
       LOG_DEBUG("mutex %s creation failed",mutexName); 
       tmx_module_object_deallocate(mutex);
       return ret;
    }

    return 0;
}

int app_utils_mutex_get(TX_MUTEX* mutex)
{
    if(NULL == mutex)
    {
        LOG_ERROR("invalid mutex object");
        return -1;
    }

    return tx_mutex_get(mutex,TX_WAIT_FOREVER);
}

int app_utils_mutex_put(TX_MUTEX* mutex)
{
    if(NULL == mutex)
    {
        LOG_ERROR("invalid mutex object");
        return -1;
    }

    return tx_mutex_put(mutex);
}

int app_utils_mutex_delete(TX_MUTEX* mutex)
{
     int ret = -1;
     if(NULL == mutex)
     {
         LOG_ERROR("invalid mutex object");
         return -1;
     }
     
     ret = tx_mutex_delete(mutex)
     tmx_module_object_deallocate(mutex);

     return ret;
}