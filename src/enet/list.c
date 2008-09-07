/** 
 @file list.c
 @brief ENet linked list functions
*/
#define ENET_BUILDING_LIB 1
#include "enet/list.h"

/** 
    @defgroup list ENet linked list utility functions
    @ingroup private
    @{
*/
void
enet_list_clear (ENetList * list)
{
   list -> sentinel.next = & list -> sentinel;
   list -> sentinel.previous = & list -> sentinel;
}

ENetListIterator
enet_list_insert (ENetListIterator position, void * data)
{
   ENetListIterator result = (ENetListIterator) data;

   result -> previous = position -> previous;
   result -> next = position;

   result -> previous -> next = result;
   position -> previous = result;

   return result;
}

void *
enet_list_remove (ENetListIterator position)
{
   position -> previous -> next = position -> next;
   position -> next -> previous = position -> previous;

   return position;
}

size_t
enet_list_size (ENetList * list)
{
   size_t size = 0;
   ENetListIterator position;

   for (position = enet_list_begin (list);
        position != enet_list_end (list);
        position = enet_list_next (position))
     ++ size;
   
   return size;
}

/** @} */
