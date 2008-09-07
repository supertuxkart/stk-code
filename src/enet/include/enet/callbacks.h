/** 
 @file  callbacks.h
 @brief ENet callbacks
*/
#ifndef __ENET_CALLBACKS_H__
#define __ENET_CALLBACKS_H__

#include <stdlib.h>

typedef struct
{
    void * (ENET_CALLBACK * malloc) (size_t size);
    void (ENET_CALLBACK * free) (void * memory);
    int (ENET_CALLBACK * rand) (void);
} ENetCallbacks;

/** @defgroup callbacks ENet internal callbacks
    @{
    @ingroup private
*/
extern void * enet_malloc (size_t);
extern void   enet_free (void *);
extern int    enet_rand (void);

/** @} */

#endif /* __ENET_CALLBACKS_H__ */

