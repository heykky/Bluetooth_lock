/**
 ****************************************************************************************
 *
 * @file ke_task.c
 *
 * @brief This file contains the implementation of the kernel task management.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup TASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"       // stack configuration

#include <stddef.h>            // standard definition
#include <stdint.h>            // standard integer
#include <stdbool.h>           // standard boolean
#include <string.h>            // memcpy defintion

#include "ke_config.h"         // kernel configuration
#include "ke_task.h"           // kernel task
#include "ke_env.h"            // kernel environment
#include "ke_queue.h"          // kernel queue
#include "ke_event.h"          // kernel event
#include "ke_mem.h"            // kernel memory

#include "arch.h"


ke_msg_func_t ke_task_handler_get(ke_msg_id_t const msg_id, ke_task_id_t const task_id);

/*
 * STRUCTURES DEFINTIONS
 ****************************************************************************************
 */
#define KE_USER_TASK_SIZE 5

/// KE TASK element structure
struct ke_task_elem
{
    uint8_t   type;
    struct ke_task_desc const * p_desc;
};

/// KE TASK environment structure
struct ke_task_env_tag
{
    uint8_t task_cnt;
    struct ke_task_elem task_list[23];
};

extern struct ke_task_env_tag ke_task_env;

/*
 * DEFINES
 ****************************************************************************************
 */



/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */



/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/// <PATCH> double free patch
/// Used memory block delimiter structure (size must be word multiple)
struct mblock_used
{
    /// Size of the current used block (including delimiter)
    uint16_t size;
    /// Used to check if memory block has been corrupted or not
    uint16_t corrupt_check;
};
#define KE_NOT_BUFF_CORRUPTED  0x8338
/// </PATCH> double free patch


/**
 ****************************************************************************************
 * @brief Scheduler entry point.
 *
 * This function is the scheduler of messages. It tries to get a message
 * from the sent queue, then try to get the appropriate message handler
 * function (from the current state, or the default one). This function
 * is called, then the message is saved or freed.
 ****************************************************************************************
 */
static void ke_task_schedule(void)
{
    // Process one message at a time to ensure that events having higher priority are
    // handled in time
    do
    {
        int msg_status;
        struct ke_msg *msg;
        // Get a message from the queue
        GLOBAL_INT_DISABLE();
        msg = (struct ke_msg*) ke_queue_pop(&ke_env.queue_sent);
        GLOBAL_INT_RESTORE();
        if (msg == NULL) break;

        /// <PATCH> double free patch
        // point to the block descriptor (before user memory so decrement)
        if((((struct mblock_used *)msg) - 1)->corrupt_check != KE_NOT_BUFF_CORRUPTED) break;
        /// </PATCH> double free patch

        // Retrieve a pointer to the task instance data
        ke_msg_func_t func = ke_task_handler_get(msg->id, msg->dest_id);

        // sanity check
        ASSERT_WARN(func != NULL);

        // Call the message handler
        if (func != NULL)
        {
            msg_status = func(msg->id, ke_msg2param(msg), msg->dest_id, msg->src_id);
        }
        else
        {
            msg_status = KE_MSG_CONSUMED;
        }

        switch (msg_status)
        {
        case KE_MSG_CONSUMED:
            // Free the message
            ke_msg_free(msg);
            break;

        case KE_MSG_NO_FREE:
            break;

        case KE_MSG_SAVED:
            // The message has been saved
            // Insert it at the end of the save queue
            ke_queue_push(&ke_env.queue_saved, (struct co_list_hdr*) msg);
            break;

        default:
            ASSERT_ERR(0);
            break;
        } // switch case
    } while(0);

    // Verify if we can clear the event bit
    GLOBAL_INT_DISABLE();
    if (co_list_is_empty(&ke_env.queue_sent))
        ke_event_clear(KE_EVENT_KE_MESSAGE);
    GLOBAL_INT_RESTORE();
}


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void ke_task_init_func(void)
{
    memset(&ke_task_env, 0, sizeof(ke_task_env));

    // Register message event
    ke_event_callback_set(KE_EVENT_KE_MESSAGE, &ke_task_schedule);
}

