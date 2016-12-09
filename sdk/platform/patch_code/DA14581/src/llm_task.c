/**
 ****************************************************************************************
 *
 * @file llm_task.c
 *
 * @brief LLM task source file
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup LLMTASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "rwip_config.h"

#include "co_bt.h"
#include "co_error.h"
#include "co_list.h"
#include "co_buf.h"
#include "co_utils.h"
#include "co_math.h"
#include "co_endian.h"
#include "ke_msg.h"
#include "ke_timer.h"
#include "ke_event.h"
#include "llm.h"
#include "llm_task.h"
#include "llm_util.h"
#include "co_version.h"
#include "llc.h"
#include "llc_task.h"
#include "llc_util.h"
#include "lld.h"
#include "lld_util.h"
#include "lld_evt.h"
#include "rwip.h"
#include "dbg.h"
#include "reg_ble_em_wpb.h"
#include "reg_ble_em_wpv.h"

/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles the command HCI read local legacy supported features.
 * The handler processes the command by checking the local legacy supported features set
 * in the controller and sends back the dedicated command complete event with the status
 * and the features.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int llm_rd_local_supp_feats_cmd_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // structure type for the complete command event
    struct llm_rd_local_supp_feats_cmd_complete *event;
    // allocate the complete event message
    event = KE_MSG_ALLOC(LLM_RD_LOCAL_SUPP_FEATS_CMP_EVT, src_id, TASK_LLM,
            llm_rd_local_supp_feats_cmd_complete);

    // get the local features
    event->feats.feats[0]= BT_FEATURES_BYTE0;
    event->feats.feats[1]= BT_FEATURES_BYTE1;
    event->feats.feats[2]= BT_FEATURES_BYTE2;
    event->feats.feats[3]= BT_FEATURES_BYTE3;
    event->feats.feats[4]= 0x60;//BT_FEATURES_BYTE4;
    event->feats.feats[5]= BT_FEATURES_BYTE5;
    event->feats.feats[6]= BT_FEATURES_BYTE6;
    event->feats.feats[7]= BT_FEATURES_BYTE7;

    // update the status
    event->status = CO_ERROR_NO_ERROR;
    // send the message
    ke_msg_send(event);

    return (KE_MSG_CONSUMED);
}


/// @} LLMTASK
