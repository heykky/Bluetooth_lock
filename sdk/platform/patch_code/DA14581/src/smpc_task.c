/**
 ****************************************************************************************
 *
 * @file smpc_task.c
 *
 * @brief SMPC Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup SMPC_TASK
 * @ingroup SMPC
 * @{
 ****************************************************************************************
 */

#include "smpc_task.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (RW_BLE_USE_CRYPT)

#if (BLE_CENTRAL || BLE_PERIPHERAL)

#include "co_math.h"
#include "co_error.h"
#include "co_endian.h"
#include "co_utils.h"

#include <string.h>

#include "ke_timer.h"
#include "gap.h"
#include "gapc.h"
#include "gapc_task.h"

#include "l2cc_task.h"
#include "l2cm.h"

#include "smpm_task.h"
#include "smpc_util.h"
#include "smpc.h"

#include "ke_mem.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

// Declaration of ROM function
extern int smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                    struct smpc_pairing_cfm *param,
                                    ke_task_id_t const dest_id, ke_task_id_t const src_id);



int PATCHED_581_smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                     struct smpc_pairing_cfm *param,
                                     ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    // Index
    uint8_t idx = KE_IDX_GET(dest_id);

    if ((ke_state_get(dest_id) != SMPC_FREE) && (smpc_env[idx]->state != SMPC_STATE_RESERVED))
    {
        // dispatch to original 581 ROM handler
        return smpc_pairing_cfm_handler(msgid,param,dest_id,src_id);
    }

    return KE_MSG_CONSUMED;
}


#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

#endif //(RW_BLE_USE_CRYPT)

/// @} SMPC_TASK
