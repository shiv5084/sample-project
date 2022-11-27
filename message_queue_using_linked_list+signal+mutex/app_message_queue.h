#ifndef _APP_MSG_Q_H
#define _APP_MSG_Q_H
#include <stdlib.h>

typedef enum app_msg_q_err_type_s
{
  APP_MSG_Q_SUCCESS,
  APP_MSG_Q_FAILURE_GENERAL,
  APP_MSG_Q_INVALID_PARAMETER,
  APP_MSG_Q_INVALID_HANDLE,
  APP_MSG_Q_UNAVAILABLE_RESOURCE,
  APP_MSG_Q_INSUFFICIENT_BUFFER,
}app_msg_q_err_type_t;

app_msg_q_err_type_t app_msg_q_init(void** msg_q_data);
const void* app_msg_q_init2(void);
app_msg_q_err_type_t app_msg_q_destroy(void** msg_q_data);
app_msg_q_err_type_t app_msg_q_snd(void* msg_q_data,void *msg_obj,void (*dealloc)(void *));
app_msg_q_err_type_t app_msg_q_rcv(void* msg_q_data,void **msg_obj);
app_msg_q_err_type_t app_msg_q_flush(void* msg_q_data);

#endif 

