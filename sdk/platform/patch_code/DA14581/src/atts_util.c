/**
 ****************************************************************************************
 *
 * @file atts_util.c
 *
 * @brief Attribute server utility
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */


#include "rwip_config.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "co_math.h"
#include "co_error.h"
#include "ke_task.h"
#include "atts_task.h"
#include "l2cc_task.h"
#include "smpc_task.h"
#include "atts.h"
#include "atts_util.h"
#include "gatt.h"
#include "gattc_task.h"
#include "gap.h"
#include "gapc.h"
#include "gattc.h"
#include "attm_db.h"
/*
 * MESSAGE HANDLERS
 * ***************************************************************************************
 */

/* function starts */

uint8_t atts_read_resp_patch(uint8_t conidx, struct l2cc_att_rd_req* req)
{
    uint8_t status = ATT_ERR_NO_ERROR;
    struct attm_elmt * attm_elmt = NULL;
    uint16_t value_len = 0;
    struct l2cc_att_rd_rsp* rd_rsp;

    /* retrieve attribute +  check permission */
    status = atts_get_att_chk_perm(conidx, ATT_READ_ACCESS, req->handle, &attm_elmt);

    if (status == ATT_ERR_NO_ERROR)
    {
        // Ensure that the length is not higher than negociated MTU (1 = Code length)
        value_len = co_min(attm_elmt->length, gattc_get_mtu(conidx)-1);
    }

    rd_rsp = ATTS_ALLOCATE_PDU(conidx, L2C_CODE_ATT_RD_RSP, 
                               l2cc_att_rd_rsp, value_len);

    if (status == ATT_ERR_NO_ERROR)
    {
        // Prepare the PDU structure
        rd_rsp->value_len = value_len;

        // Copy the attribute data into the response buffer
        memcpy(&rd_rsp->value[0], &(attm_elmt->value[0]), rd_rsp->value_len);

        /* construct and send PDU */
        atts_send_pdu(rd_rsp);
        
        
        struct attm_svc_db * svc = attmdb_get_service(req->handle);

        if(svc != NULL)
        {
            // send write indication to set task anyway
            struct gattc_read_cmd_ind *rd_ind = KE_MSG_ALLOC(GATTC_READ_CMD_IND,
                    svc->task_id, KE_BUILD_ID(TASK_GATTC, conidx),
                    gattc_read_cmd_ind);

            // Fill the message parameters
            rd_ind->handle  = req->handle;
            
            // And send the message
            ke_msg_send(rd_ind);
        }

    }
    else
    {
        /* read request not allowed */
        atts_send_error(rd_rsp, req->code, req->handle, status);
    }

    return ATT_ERR_NO_ERROR;
}

/// @} ATTSTASK
