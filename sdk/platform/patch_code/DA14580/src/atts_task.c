/**
 ****************************************************************************************
 *
 * @file atts_task.c
 *
 * @brief Attribute Server Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup ATTSTASK
 * @{
 ****************************************************************************************
 */
#include "rwip_config.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if (BLE_ATTS)
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
/*
 * MESSAGE HANDLERS
 * ***************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Handles data packet from L2CAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_ATTC).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int l2cc_pdu_recv_ind_handler(ke_msg_id_t const msgid, struct l2cc_pdu_recv_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    if(state != ATTS_FREE)
    {
        uint8_t  conidx               = KE_IDX_GET(dest_id);

        // wait for connection to be ready before doing anything
        if (ke_state_get(dest_id) == ATTS_CONNNECTED) return (KE_MSG_SAVED);

        // Indication confirmation event code.
        if (param->pdu.data.code == L2C_CODE_ATT_HDL_VAL_CFM)
        {
            // clear timeout of indication confirmation
            ke_timer_clear(ATTS_RTX_IND, dest_id);


            /* Indicate that command is correctly completed */
            atts_send_cmd_cmp(conidx, ATT_ERR_NO_ERROR);

            // return to IDLE mode
            ke_state_set(dest_id, ATTS_READY);
        }
        else
        {
            /* In indicate state, all request from peer device and from high layer are postponed until
             * Confirmation message is received or timeout is triggered
             */
//            if (ke_state_get(dest_id) == ATTS_INDICATE) return (KE_MSG_SAVED);

            // process the packet according to the attribute code
            switch (param->pdu.data.code)
            {
                case L2C_CODE_ATT_MTU_REQ:
                    atts_mtu_exc_resp(conidx, &(param->pdu.data.mtu_req));
                    break;

                case L2C_CODE_ATT_FIND_INFO_REQ:
                    atts_find_info_resp(conidx, &(param->pdu.data.find_info_req));
                    break;

                case L2C_CODE_ATT_FIND_BY_TYPE_REQ:
                    atts_find_by_type_resp(conidx, &(param->pdu.data.find_by_type_req));
                    break;

                case L2C_CODE_ATT_RD_BY_TYPE_REQ:
                    atts_read_by_type_resp(conidx, &(param->pdu.data.rd_by_type_req));
                    break;

                case L2C_CODE_ATT_RD_BY_GRP_TYPE_REQ:
                    atts_read_by_grp_type_resp(conidx, &(param->pdu.data.rd_by_grp_type_req));
                    break;

                case L2C_CODE_ATT_RD_REQ:
                    atts_read_resp(conidx, &(param->pdu.data.rd_req));
                    break;

                case L2C_CODE_ATT_RD_BLOB_REQ:
                    atts_read_blob_resp(conidx, &(param->pdu.data.rd_blob_req));
                    break;

                case L2C_CODE_ATT_RD_MULT_REQ:
                    atts_read_mult_resp(conidx, &(param->pdu.data.rd_mult_req));
                    break;

                case L2C_CODE_ATT_WR_CMD:
                    atts_write_no_resp(conidx, &(param->pdu.data.wr_cmd));
                    break;

                case L2C_CODE_ATT_WR_REQ:
                    atts_write_resp(conidx, &(param->pdu.data.wr_req));
                    break;

                case L2C_CODE_ATT_SIGN_WR_CMD:
                    atts_signed_write_resp(conidx, &(param->pdu.data.sign_wr_cmd));
                    break;

                case L2C_CODE_ATT_PREP_WR_REQ:
                    atts_prepare_write_resp(conidx, &(param->pdu.data.prep_wr_req));
                    break;

                case L2C_CODE_ATT_EXE_WR_REQ:
                    atts_execute_write_resp(conidx, &(param->pdu.data.exe_wr_req));
                    break;

                    // undefined code will be discarded
                default: break;
            }
        }
    }

    // message is consumed
    return (KE_MSG_CONSUMED);
}
#endif /* #if (BLE_ATTS */
/// @} ATTSTASK
