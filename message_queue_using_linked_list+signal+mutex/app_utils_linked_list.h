#ifndef _LINKED_LIST_UTILS_H_
#define _LINKED_LIST_UTILS_H_

#include <stdbool.h>

typedef enum
{
  LINKED_LIST_SUCCESS,
  LINKED_LIST_FAILURE_GENERAL,
  LINKED_LIST_INVALID_PARAMETER,
  LINKED_LIST_INVALID_HANDLE,
  LINKED_LIST_UNAVAILABLE_RESOURCE,
  LINKED_LIST_INSUFFICIENT_BUFFER,
}app_utils_list_err;

app_utils_list_err app_utils_list_init(void** list_data);
app_utils_list_err app_utils_list_destroy(void** list_data);
app_utils_list_err app_utils_list_add(void* list_data, void *data_obj,void (*dealloc)(void *));
app_utils_list_err app_utils_list_remove(void *list_data,void ** data_obj);
app_utils_list_err app_utils_list_flush(void *list);
app_utils_list_err app_utils_list_search(void *list_data, void** searched_data_out,
  bool (*equal)(void *data_to_be_searched_input, void* data_to_be_matched_and_fetched_while_operation),
  void *data_to_be_searched_input, bool remove_if_found);   
int app_utils_list_empty(void *list_data);

#endif 