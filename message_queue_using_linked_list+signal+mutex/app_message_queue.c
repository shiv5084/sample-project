#include <stdio.h>
#include <stdlib.h>

#include "app_message_queue.h"
#include "app_utils_linked_list.h"
#include "app_utils_misc.h"
#include "app_log.h"

typedef struct app_msg_q_s
{
    void* msg_list;                    /*Linked list to store information*/
    TX_MUTEX* list_mutex;              /*Mutex for exclusive access to message queue*/
    TX_EVENT_FLAGS_GROUP* list_signal; /*Signal to wait for message*/
}app_msg_q_t;

static app_msg_q_err_type_t _convert_linked_list_err_type(app_utils_list_err linked_list_val)
{
    switch( linked_list_val )
    {
        case LINKED_LIST_SUCCESS:
        return APP_MSG_Q_SUCCESS;
        case LINKED_LIST_INVALID_PARAMETER:
        return APP_MSG_Q_INVALID_PARAMETER;
        case LINKED_LIST_INVALID_HANDLE:
        return APP_MSG_Q_INVALID_HANDLE;
        case LINKED_LIST_UNAVAILABLE_RESOURCE:
        return APP_MSG_Q_UNAVAILABLE_RESOURCE;
        case LINKED_LIST_INSUFFICIENT_BUFFER:
        return APP_MSG_Q_INSUFFICIENT_BUFFER;
        case LINKED_LIST_FAILURE_GENERAL:
        return APP_MSG_Q_FAILURE_GENERAL;
        default:
        return APP_MSG_Q_FAILURE_GENERAL;
    }
}

app_msg_q_err_type_t app_msg_q_init(void** msg_q_data)
{
    app_msg_q_t* tmp_msg_q = NULL;
    int ret = 0;
     
    if(msg_q_data == NULL)
    {
        LOG_ERROR("invalid msg_q_data parameter");
        return APP_MSG_Q_INVALID_PARAMETER;
    }
    //tx_byte_allocate for tmp_msg_q of size app_msg_q_t
    //if tmp_msg_q == NULL or ret != 0 then unable to store space for msg_q
    #ifdef Tx_Thread
    if(0 != (ret = tx_byte_allocate(bytePoolLocation,tmp_msg_q,sizeof(app_msg_q_t),TX_NO_WAIT)))
    {
        LOG_ERROR("unable to allocate space for message queue %d",ret);
        return LINKED_LIST_FAILURE_GENERAL;
    }
    #else 
    if(NULL == (tmp_msg_q = (app_msg_q_t*)malloc(sizeof(app_msg_q_t))))
    {
       LOG_ERROR("unable to allocate space for message queue %d",tmp_msg_q);
       return LINKED_LIST_FAILURE_GENERAL; 
    }
    #endif 

    if(app_utils_list_init(&tmp_msg_q->msg_list) != 0)
    {
        LOG_ERROR("unable to initialize storage list");
        tx_byte_release(tmp_msg_q);
        return APP_MSG_Q_FAILURE_GENERAL;
    }

    if(app_utils_mutex_init(&tmp_msg_q->list_mutex, "list_mutex") != 0)
    {
        LOG_ERROR("unable to initialize list_mutex");
        app_utils_list_destroy(&tmp_msg_q->msg_list);
        tx_byte_release(tmp_msg_q);
        return APP_MSG_Q_FAILURE_GENERAL;
    }

    if(app_utils_init_signal(&tmp_msg_q->list_signal, "list_signal") != 0)
    {
        LOG_ERROR("unable to initialize list_signal");
        app_utils_list_destroy(&tmp_msg_q->msg_list);
        app_utils_mutex_delete(tmp_msg_q->list_mutex);
        tx_byte_release(tmp_msg_q);
        return APP_MSG_Q_FAILURE_GENERAL;
    }

    *msg_q_data = tmp_msg_q;

    return APP_MSG_Q_SUCCESS;
}

const void* app_msg_q_init2()
{
    void *p=NULL;
    if(APP_MSG_Q_SUCCESS != app_msg_q_init(&p))
    {
        p = NULL;
    }
    return p;
}

app_msg_q_err_type_t app_msg_q_destroy(void** msg_q_data)
{
    app_msg_q_t* p_msg_q;

    if(msg_q_data == NULL)
    {
        LOG_ERROR("invalid msg_q_data parameter");
        return APP_MSG_Q_INVALID_HANDLE;
    }

    p_msg_q = (app_msg_q_t*)*msg_q_data;

    app_utils_list_destroy(&p_msg_q->msg_list);
    app_utils_mutex_delete(p_msg_q->list_mutex);
    app_utils_deinit_signal(p_msg_q->list_signal);

    #ifdef Tx_Thread
    tx_byte_release(*msg_q_data);
    #else 
    free(*msg_q_data);
    #endif
    *msg_q_data=NULL;

    return APP_MSG_Q_SUCCESS;
}

app_msg_q_err_type_t app_msg_q_snd(void* msg_q_data, void* msg_obj, void (*dealloc)(void *))
{
    app_msg_q_t* p_msg_q;
    app_msg_q_err_type_t rv;

    if(msg_q_data == NULL)
    {
        LOG_ERROR("invalid msg_q_data parameter");
        return APP_MSG_Q_INVALID_HANDLE;
    }

    if(msg_obj == NULL)
    {
        LOG_ERROR("invalid msg_obj parameter");
        return APP_MSG_Q_INVALID_PARAMETER;
    }

    p_msg_q = (app_msg_q_t*)msg_q_data;

    app_utils_mutex_get(p_msg_q->list_mutex);
    rv = _convert_linked_list_err_type(app_utils_list_add(p_msg_q->msg_list,
    msg_obj, dealloc));
    app_utils_set_signal(p_msg_q->list_signal,p_msg_q->list_mutex);
    app_utils_mutex_put(p_msg_q->list_mutex);

    return rv;
}

app_msg_q_err_type_t app_msg_q_rcv(void* msg_q_data, void** msg_obj)
{
    app_msg_q_t* p_msg_q;
    app_msg_q_err_type_t rv;

    if(msg_q_data == NULL)
    {
        LOG_ERROR("invalid msg_q_data parameter");
        return APP_MSG_Q_INVALID_HANDLE;
    }

    if(msg_obj == NULL)
    {
        LOG_ERROR("invalid msg_obj parameter");
        return APP_MSG_Q_INVALID_PARAMETER;
    }

    p_msg_q = (app_msg_q_t*)msg_q_data;

    app_utils_mutex_get(p_msg_q->list_mutex);
    
    /*wait for data in the message queue*/
    while(app_utils_list_empty(p_msg_q->msg_list)) //block the caller till any data is available in list
    {
        app_utils_wait_on_signal(p_msg_q->list_signal, p_msg_q->list_mutex);
    }

    rv = _convert_linked_list_err_type(app_utils_list_remove(p_msg_q->msg_list,
    msg_obj));
    
    app_utils_mutex_put(p_msg_q->list_mutex);

    return rv;
}

app_msg_q_err_type_t app_msg_q_flush(void* msg_q_data)
{
    app_msg_q_t* p_msg_q;
    app_msg_q_err_type_t rv;

    if(msg_q_data == NULL)
    {
        LOG_ERROR("invalid msg_q_data");
        return APP_MSG_Q_INVALID_HANDLE;
    }

    p_msg_q = (app_msg_q_t*)msg_q_data;

    app_utils_mutex_get(p_msg_q->list_mutex);
    rv = _convert_linked_list_err_type(app_utils_list_flush(p_msg_q->msg_list));
    app_utils_mutex_put(p_msg_q->list_mutex);

    return rv;
}