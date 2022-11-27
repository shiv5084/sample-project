#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "app_utils_linked_list.h"
#include "app_log.h"

static TX_BYTE_POOL* bytePoolLocation = NULL;

typedef struct list_element
{
    struct list_element* next;
    struct list_element* prev;
    void *data_ptr;
    void (*dealloc_func)(void*);    
}list_element;

typedef struct list_state
{
    list_element* p_head;
    list_element* p_tail;
}list_state;

app_utils_list_err app_utils_list_init(void **list_data)
{
  int ret=-1;
  list_state *tmp_list;

  if(list_data == NULL)
  {
      LOG_ERROR("invalid list parameter");
      return LINKED_LIST_INVALID_PARAMETER;
  }

  //tx_byte_allocate for tmp_list of size list_state//or malloc
 // tmp_list = (list_state*)malloc(sizeof(list_state));
 #ifdef Tx_Thread 
 if(0 != (ret = tx_byte_allocate(bytePoolLocation,tmp_list,sizeof(list_state),TX_NO_WAIT)))
 {
     LOG_ERROR("unable to allocate space for list %d",ret);
     return LINKED_LIST_FAILURE_GENERAL;
 }
 #else
 if(NULL == (tmp_list = (app_msg_q_t*)malloc(sizeof(list_state))))
 {
    LOG_ERROR("unable to allocate space for message queue %d",tmp_list);
    return LINKED_LIST_FAILURE_GENERAL; 
 }
#endif 


  tmp_list->p_head = NULL;
  tmp_list->p_tail = NULL;

  *list_data = tmp_list;

  return LINKED_LIST_SUCCESS;
}

app_utils_list_err app_utils_list_destroy(void **list_data)
{
    list_state *p_list;
    if(list_data == NULL)
    {
        LOG_ERROR("invalid list parameter");
        return LINKED_LIST_INVALID_HANDLE;
    }

    p_list = (list_state*)*list_data; 

    app_utils_list_flush(p_list);

    #ifdef Tx_Thread
    tx_byte_release(*list_data);
    #else
    free(*list_data);
    #endif 
    list_data = NULL;

    return LINKED_LIST_SUCCESS;
}

app_utils_list_err app_utils_list_add(void *list_data, void *data_obj, void (*dealloc)(void *))
{
    int ret = -1;
    list_state* p_list;
    list_element* elem;
    list_element* tmp; 

    if(list_data == NULL)
    {
        LOG_ERROR("invalid list parameter");
        return LINKED_LIST_INVALID_HANDLE; 
    }

    if(data_obj == NULL)
    {
        LOG_ERROR("invalid input parameter");
        return LINKED_LIST_INVALID_PARAMETER;
    }
    //tx_byte_allocate for elem of size list_element
    #ifdef Tx_Thread
    if(0 != (ret = tx_byte_allocate(bytePoolLocation,elem,sizeof(list_element),TX_NO_WAIT)))
    {
        LOG_ERROR("memory allocation failed err %d",ret);
        return LINKED_LIST_FAILURE_GENERAL;
    }
    #else
    if(NULL == (elem = (app_msg_q_t*)malloc(sizeof(list_element))))
    {
        LOG_ERROR("unable to allocate space for message queue %d",elem);
        return LINKED_LIST_FAILURE_GENERAL; 
    }
    #endif 

    p_list = (list_state*)list_data;

    /*copy data to newly created element */
    elem->next = NULL;
    elem->prev = NULL;
    elem->data_ptr = data_obj;
    elem->dealloc_func = dealloc;

    /* replace head element*/
    tmp = p_list->p_head;
    p_list->p_head = elem;
     /* point next to the previous head element*/
    p_list->p_head->next = tmp;

    if(tmp != NULL)
    {
        tmp->prev = p_list->p_head;
    }
    else
    {
        p_list->p_head = p_list->p_head;
    }

    return LINKED_LIST_SUCCESS;
}
 
 /*it is callers responsibility to deallocate data_obj */
 app_utils_list_err app_utils_list_remove(void *list_data, void** data_obj)
 {
     list_state* p_list;
     list_element* tmp;

     if(list_data == NULL)
     {
         LOG_ERROR("invalid list parameter");
         return LINKED_LIST_INVALID_HANDLE;
     }

     if(data_obj == NULL)
     {
         LOG_ERROR("invalid input parameter");
         return LINKED_LIST_INVALID_PARAMETER;
     }

     p_list = (list_state*)list_data;

     if(p_list->p_tail == NULL)
     {
         return LINKED_LIST_UNAVAILABLE_RESOURCE;
     }

     tmp = p_list->p_tail; 
     /*replace tail element */
     p_list->p_tail = tmp->prev;

     if(tmp != NULL)
     {
         p_list->p_tail->next = NULL; 
     }
     else
     {
         p_list->p_head = p_list->p_tail;
     }
     
     /*copy data to output parameter, callers responsibility to deallocate data_obj*/
     *data_obj = tmp->data_ptr;

     /*free allocated list element*/
     #ifdef Tx_Thread
     tx_byte_release(tmp);
     #else
     free(tmp);
     #endif 

     return LINKED_LIST_SUCCESS;
 }

 int app_utils_list_empty(void *list_data)
 {
     if(list_data == NULL)
     {
         LOG_ERROR("invalid list parameter");
         return (int)LINKED_LIST_INVALID_HANDLE;
     }
     else
     {
         list_state* p_list = (list_state*)list_data;
         return p_list->p_head == NULL ? 1 : 0;
     }
 }

 app_utils_list_err app_utils_list_flush(void *list_data)
 {
     list_state* p_list;

     if(list_data == NULL)
     {
         LOG_ERROR("invalid list parameter");
         return LINKED_LIST_INVALID_HANDLE;
     }

     p_list = (list_state*)list_data;
     /*remove all dynamically allocated elements */
     while(p_list->p_head != NULL)
     {
         list_element* tmp = p_list->p_head->next;

         if(p_list->p_head->dealloc_func != NULL)
         {
             p_list->p_head->dealloc_func(p_list->p_head->data_ptr);
         }

         #ifdef Tx_Thread
         tx_byte_release(p_list->p_head)
         #else 
         free(p_list->p_head);
         #endif

         p_list->p_head = tmp;
     }

     p_list->p_tail = NULL;

     return LINKED_LIST_SUCCESS;
 }

 app_utils_list_err app_utils_list_search(void *list_data, void** searched_data_out,
  bool (*equal)(void *data_to_be_searched_input, void* data_to_be_matched_and_fetched_while_operation),
  void *data_to_be_searched_input, bool remove_if_found)
  {
      list_state* p_list;
      list_element* tmp;

      if(list_data == NULL || NULL == equal)
      {
          LOG_ERROR("invalid list parameter list_data %p equal %p",
                    list_data,equal);
          return LINKED_LIST_INVALID_HANDLE;
      }

       p_list = (list_state*)list_data;

       if(p_list->p_tail == NULL) //to check if any data exist in list
       {
           return LINKED_LIST_UNAVAILABLE_RESOURCE;
       }

      tmp = p_list->p_head;

      if(searched_data_out != NULL)
       {
           *searched_data_out = NULL;
       }

       while(NULL != tmp)
       {
           if((*equal)(data_to_be_searched_input,tmp->data_ptr))
           {
               if(searched_data_out != NULL)
               {
                   *searched_data_out = tmp->data_ptr;
               }

               if(remove_if_found)
               {
                   if(NULL == tmp->prev)//if searched data is 1st element 
                   {
                       p_list->p_head = tmp->next;
                   }
                   else
                   {
                       tmp->prev->next = tmp->next;//if searched is in between element
                   }

                   if(NULL == tmp->next)//if searched data is last element
                   {
                       p_list->p_tail = tmp->prev;
                   }
                   else
                   {
                       tmp->next->prev = tmp->prev;//if searched data is in between element
                   }

                   tmp->next = tmp->prev = NULL;
                /*deallocate data if it is not copied out & caller has provided
                a dealloc function pointer*/
                   if(NULL == searched_data_out && NULL != tmp->dealloc_func)
                   {
                       tmp->dealloc_func(tmp->data_ptr);
                   }
                   #ifdef Tx_Thread
                   tx_byte_release(tmp);
                   #else 
                   free(tmp);
                   #endif 
               }
               tmp = NULL;
           }
           else
           {
               tmp = tmp->next;
           }
       }
    return LINKED_LIST_SUCCESS;
  }