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

// /*
//  * FUNCTION DEFINITIONS
//  ****************************************************************************************
//  */

// #if (BLE_CENTRAL)
// /**
//  ****************************************************************************************
//  * @brief Handles pairing request from GAP.
//  *        GAP block received the bonding request from the application and sends SMPC
//  *        the pairing information to be used in pairing procedure.
//  *
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpc_pairing_cmd_handler(ke_msg_id_t const msgid,
//                                     struct smpc_pairing_cmd *param,
//                                     ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Command Status
//     uint8_t status      = SMP_ERROR_NO_ERROR;
//     // Message Status
//     uint8_t msg_status  = KE_MSG_CONSUMED;
//     // State
//     ke_state_t state    = ke_state_get(dest_id);
//     // The command can be handled
//     uint8_t idx         = KE_IDX_GET(dest_id);

//     do
//     {
//         ASSERT_ERR(state < SMPC_STATE_MAX);

//         /*-------------------------------------------------
//          * Check the current state of the task
//          *-------------------------------------------------*/
//         if (state == SMPC_FREE)
//         {
//             status = SMP_ERROR_REQ_DISALLOWED;
//             break;
//         }
//         // else state is SMPC_IDLE or SMPC_BUSY, the command can be handled.

//         /* ------------------------------------------------
//          * Check SMP Timeout timer state
//          * ------------------------------------------------*/
//         if (SMPC_IS_FLAG_SET(idx, SMPC_TIMER_TIMEOUT_BLOCKED_FLAG))
//         {
//             /*
//              * Once a timeout has occurred, no security procedure can be initiated until a new
//              * physical link has been established.
//              */
//             status = SMP_ERROR_TIMEOUT;
//             break;
//         }

//         /* ------------------------------------------------
//          * Check Repeated Attempts Timer state
//          * ------------------------------------------------*/
//         if (SMPC_IS_FLAG_SET(idx, SMPC_TIMER_REP_ATT_FLAG))
//         {
//             status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                               SMP_ERROR_REPEATED_ATTEMPTS);
//             break;
//         }

//         /* ------------------------------------------------
//          * Check provided parameters
//          * ------------------------------------------------*/
//         if ((param->operation != SMPC_PAIRING) || (!smpc_check_pairing_feat(&param->pairing_feat)))
//         {
//             status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                               SMP_ERROR_INVALID_PARAM);
//             break;
//         }

//         /* ------------------------------------------------
//          * Check Encryption Key Size
//          * ------------------------------------------------*/
//         if ((param->pairing_feat.key_size < SMPC_MIN_ENC_SIZE_LEN) ||
//             (param->pairing_feat.key_size > SMPC_MAX_ENC_SIZE_LEN))
//         {
//             status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                               SMP_ERROR_INVALID_PARAM);
//             break;
//         }
//         // State is either SMPC_IDLE or SMPC_BUSY
//         if (state == SMPC_IDLE)
//         {
//             ASSERT_ERR(smpc_env[idx]->pair_info == NULL);

//             // Allocate memory for the pairing information structure
//             smpc_env[idx]->pair_info = ke_malloc(sizeof(struct smpc_pair_info), KE_MEM_ENV);

//             // Check if memory has been allocated
//             if (smpc_env[idx]->pair_info != NULL)
//             {
//                 struct smpc_pair_info *pair_info = (struct smpc_pair_info *)smpc_env[idx]->pair_info;

//                 memset(pair_info, 0x00, sizeof(struct smpc_pair_info));

//                 // Keep the command message until the end of the request => State is now SMPC_BUSY
//                 smpc_init_operation(idx, (void *)param);

//                 // Copy the pairing features
//                 memcpy(&pair_info->pair_req_feat, &param->pairing_feat, sizeof(struct gapc_pairing));

//                 // If device is not bondable, useless to ask for keys
//                 if ((pair_info->pair_req_feat.auth & GAP_AUTH_BOND) != GAP_AUTH_BOND)
//                 {
//                     pair_info->pair_req_feat.ikey_dist = 0x00;
//                     pair_info->pair_req_feat.rkey_dist = 0x00;
//                 }

//                 // Send the pairing request PDU to the peer device
//                 smpc_pdu_send(idx, L2C_CODE_PAIRING_REQUEST, &pair_info->pair_req_feat);
//                 // Waiting for the pairing response
//                 smpc_env[idx]->state = SMPC_PAIRING_RSP_WAIT;

//                 // Inform the kernel that this message cannot be unallocated
//                 msg_status = KE_MSG_NO_FREE;
//             }
//             else
//             {
//                 status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                                   SMP_ERROR_UNSPECIFIED_REASON);
//             }
//         }
//         else if (state != SMPC_FREE)
//         {
//             // Another operation is in progress (Encryption, Signature, ...)
//             msg_status = KE_MSG_SAVED;
//         }
//         else
//         {
//             ASSERT_ERR(0);
//         }
//     } while (0);

//     // Inform the source task if an error has occurred
//     if (status != SMP_ERROR_NO_ERROR)
//     {
//         // The Repeated attempt timer won't be started because the pairing procedure has not begun
//         smpc_pairing_end(idx, ROLE_MASTER, status, false);
//     }

//     return msg_status;
// }
// #endif //(BLE_CENTRAL)

/**
 ****************************************************************************************
 * @brief Handles Host response containing pairing information.
 *        Depending on device role, this response is obtained at different times
 *        during the procedure.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] rsp       Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                    struct smpc_pairing_cfm *param,
                                    ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    // Message Status
    uint8_t msg_status  = KE_MSG_CONSUMED;
    // Status
    uint8_t status      = SMP_ERROR_NO_ERROR;
    // Index
    uint8_t idx         = KE_IDX_GET(dest_id);
    // Role
    uint8_t role        = gapc_get_role(idx);

    if (ke_state_get(dest_id) != SMPC_FREE)
    {
    switch (param->request)
    {
        // TK is received from the HL, can occurred as master or slave
        case (GAPC_TK_EXCH):
        {
            bool gen_rand_nb = false;

            if ((smpc_env[idx]->state == SMPC_PAIRING_TK_WAIT)
                || (smpc_env[idx]->state == SMPC_PAIRING_TK_WAIT_CONF_RCV))
            {
                // If key is null, pairing fails
                if (param->accept)
                {
                    ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

                    // Keep TK
                    memcpy(&smpc_env[idx]->pair_info->key.key[0], &param->data.tk.key[0], KEY_LEN);

                    if (role == ROLE_MASTER)
                    {
                        #if (BLE_CENTRAL)
                        // Verify the state
                        if (smpc_env[idx]->state == SMPC_PAIRING_TK_WAIT)
                        {
                            gen_rand_nb = true;
                        }
                        // Else drop the message, the message has been sent without any reason.
                        #endif //(BLE_CENTRAL)
                    }
                    else    // role == ROLE_SLAVE
                    {
                        #if (BLE_PERIPHERAL)
                        if (smpc_env[idx]->state == SMPC_PAIRING_TK_WAIT)
                        {
                            // Wait for the master device confirm value
                            smpc_env[idx]->state = SMPC_PAIRING_WAIT_CONFIRM;
                        }
                        else if (smpc_env[idx]->state == SMPC_PAIRING_TK_WAIT_CONF_RCV)
                        {
                            gen_rand_nb = true;
                        }
                        // Else drop the message, the message has been sent without any reason.
                        #endif //(BLE_PERIPHERAL)
                    }

                    if (gen_rand_nb)
                    {
                        // Generate Random Number
                        smpc_generate_rand(idx, SMPC_PAIRING_GEN_RAND_P1);
                    }
                }
                else
                {
                    // TK could not be retrieved in the Host
                    status = SMP_ERROR_PASSKEY_ENTRY_FAILED;
                }
            } // if ((smpc_env[idx]->state == SMPC_PAIRING_TK_WAIT) || (smpc_env[idx]->state == SMPC_PAIRING_TK_WAIT_CONF_RCV))
        } break;

        // LTK is received from the HL
        case (GAPC_LTK_EXCH):
        {
            // Verify the state
            if (smpc_env[idx]->state == SMPC_PAIRING_LTK_WAIT)
            {
                ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

                // Counter
                uint8_t counter;
                // Master ID Information
                struct smpc_mst_id_info mst_id_info;

                ASSERT_ERR(gapc_get_enc_keysize(idx) <= KEY_LEN);

                // Mask the MSB to respect key size (Key is stored LSB->MSB)
                for (counter = gapc_get_enc_keysize(idx); counter < KEY_LEN; counter++)
                {
                    param->data.ltk.ltk.key[counter] = 0x00;
                }

                // Send MST_ID PDU with EDIV and RandNb
                mst_id_info.ediv = param->data.ltk.ediv;
                memcpy(&(mst_id_info.randnb[0]), &(param->data.ltk.randnb.nb[0]), RAND_NB_LEN);

                // Decide the next step in TKDP: Distribute IRK, CSRK or end;
                smpc_tkdp_send_continue(idx, role, &(param->data.ltk.ltk.key[0]), &mst_id_info);
            }
            // Else drop the message
        } break;

        case (GAPC_CSRK_EXCH):
        {
            // Verify the state
            if (smpc_env[idx]->state == SMPC_PAIRING_CSRK_WAIT)
            {
                ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

                // Send the PDU to the peer device
                    smpc_pdu_send(idx, L2C_CODE_SIGNING_INFORMATION, (void *)&param->data.csrk.key[0]);

                if (role == ROLE_MASTER)
                {
                    // Pairing is over
                        smpc_pairing_end(idx, role, SMP_ERROR_NO_ERROR, false);
                }
                else    // role == ROLE_SLAVE
                {
                    // Master will now send its keys (if needed)
                    smpc_tkdp_rcp_start(idx, role);
                }
            }
        } break;

        // The slave device provides its pairing features
        case (GAPC_PAIRING_RSP):
        {
            #if (BLE_PERIPHERAL)
            // Only the slave device provides its pairing features in a SMPC_PAIRING_CFM message
            if (role == ROLE_SLAVE)
            {
                // State
                ke_state_t state = ke_state_get(dest_id);

                if (state == SMPC_IDLE)
                {
                    if (smpc_env[idx]->state == SMPC_PAIRING_FEAT_WAIT)
                        {
                            if (param->accept)
                    {
                        ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

                        // Set state to busy
                        ke_state_set(dest_id, SMPC_BUSY);

                        // Store the pairing features of the slave
                        memcpy(&smpc_env[idx]->pair_info->pair_rsp_feat,
                               &param->data.pairing_feat, sizeof(struct gapc_pairing));

                        do
                        {
                            /*
                             * If the slave device has not found OOB data for the master while the master has
                             * OOB data, the pairing will failed with a OOB Not Available error status
                             */
                            if ((param->data.pairing_feat.oob == 0) &&
                                (smpc_env[idx]->pair_info->pair_req_feat.oob == 1))
                            {
                                // OOB status mismatch
                                status = SMP_ERROR_OOB_NOT_AVAILABLE;
                                break;
                            }

                            if (!smpc_check_max_key_size(idx))
                            {
                                // Resultant encryption key size is below the minimal accepted value
                                status = SMP_ERROR_ENC_KEY_SIZE;
                                break;
                            }

                            if (!smpc_check_key_distrib(idx, param->data.pairing_feat.sec_req))
                            {
                                // The resultant key distribution doesn't match with the provided parameters
                                status = SMP_ERROR_UNSPECIFIED_REASON;
                                break;
                            }

                            // Select security properties and STK generation type
                            smpc_get_key_sec_prop(idx);

                            // Check if the required security mode can be reached
                            if (!smpc_is_sec_mode_reached(idx, ROLE_SLAVE))
                            {
                                /*
                                 * The pairing procedure cannot be performed as authentication requirements cannot
                                 * be met due to IO capabilities of one or both devices.
                                 */
                                status = SMP_ERROR_AUTH_REQ;
                                break;
                            }

//                            if (smpc_env[idx]->pair_info->pair_method == SMPC_METH_JW)
//                            {
//                                // Force the authentication requirements to NMITM
//                                smpc_env[idx]->pair_info->pair_rsp_feat.auth &= ~GAP_AUTH_MITM;
//                            }

                            // Adjust the pairing features according on master's
                            // If device is not bondable, useless to ask for keys
                            if ((smpc_env[idx]->pair_info->pair_rsp_feat.auth == GAP_AUTH_REQ_NO_MITM_NO_BOND) ||
                                (smpc_env[idx]->pair_info->pair_rsp_feat.auth == GAP_AUTH_REQ_MITM_NO_BOND))
                            {
                                smpc_env[idx]->pair_info->pair_rsp_feat.ikey_dist    = 0x00;
                                smpc_env[idx]->pair_info->pair_rsp_feat.rkey_dist    = 0x00;
                            }
                            else
                            {
                                smpc_env[idx]->pair_info->pair_rsp_feat.ikey_dist
                                        &= smpc_env[idx]->pair_info->pair_req_feat.ikey_dist;
                                smpc_env[idx]->pair_info->pair_rsp_feat.rkey_dist
                                        &= smpc_env[idx]->pair_info->pair_req_feat.rkey_dist;
                            }

                            if (status == SMP_ERROR_NO_ERROR)
                            {
                                // Can send the Pairing Response PDU
                                        smpc_pdu_send(idx, L2C_CODE_PAIRING_RESPONSE,
                                                      &smpc_env[idx]->pair_info->pair_rsp_feat);

                                if (smpc_env[idx]->pair_info->pair_method == SMPC_METH_JW)
                                {
                                    smpc_env[idx]->state = SMPC_PAIRING_WAIT_CONFIRM;
                                }
                                else
                                {
                                    // Send a TK request to the HL
                                    smpc_send_pairing_req_ind(idx, GAPC_TK_EXCH);
                                }
                            }
                        } while (0);
                            }
                            else
                            {
                                // Pairing will fail due to application error
                                status = SMP_ERROR_UNSPECIFIED_REASON;
                            }
                    }
                    // Else drop the message
                }
                    else if (state != SMPC_FREE)
                {
                    msg_status = KE_MSG_SAVED;
                }
                // Else drop the message
            }
            // Else drop the message
            #endif //(BLE_PERIPHERAL)
        } break;

        default: break; // Drop the message
    }

    if (status != SMP_ERROR_NO_ERROR)
    {
        // Send the Pairing Failed PDU to the peer device
            smpc_pdu_send(idx, L2C_CODE_PAIRING_FAILED, (void *)&status);

        // Apply the error mask to know who triggered the error
        status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK, status);

        // Inform the HL that an error has occurred.
            smpc_pairing_end(idx, role, status, true);
    }
    }

    return msg_status;
}

// #if (BLE_PERIPHERAL)
// /**
//  ****************************************************************************************
//  * @brief Handles request of sending Security Request message to peer device.
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpc_security_cmd_handler(ke_msg_id_t const msgid,
//                                      struct smpc_security_cmd *param,
//                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Command Status
//     uint8_t status      = SMP_ERROR_NO_ERROR;
//     // Message Status
//     uint8_t msg_status  = KE_MSG_CONSUMED;
//     // State
//     ke_state_t state    = ke_state_get(dest_id);
//     // The command can be handled
//     uint8_t idx         = KE_IDX_GET(dest_id);

//     do
//     {
//         ASSERT_ERR(state < SMPC_STATE_MAX);

//         /*-------------------------------------------------
//          * Check the current state of the task
//          *-------------------------------------------------*/
//         if (state == SMPC_FREE)
//         {
//             status = SMP_ERROR_REQ_DISALLOWED;
//             break;
//         }
//         // else state is SMPC_IDLE or SMPC_BUSY, the command can be handled.

//         /* ------------------------------------------------
//          * Check SMP Timeout timer state
//          * ------------------------------------------------*/
//         if (SMPC_IS_FLAG_SET(idx, SMPC_TIMER_TIMEOUT_BLOCKED_FLAG))
//         {
//             /*
//              * Once a timeout has occurred, no security procedure can be initiated until a new
//              * physical link has been established.
//              */
//             status = SMP_ERROR_TIMEOUT;
//             break;
//         }

//         /* ------------------------------------------------
//          * Check Repeated Attempts Timer state
//          * ------------------------------------------------*/
//         if (SMPC_IS_FLAG_SET(idx, SMPC_TIMER_REP_ATT_FLAG))
//         {
//             status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                               SMP_ERROR_REPEATED_ATTEMPTS);
//             break;
//         }

//         /* ------------------------------------------------
//          * Check provided parameters
//          * ------------------------------------------------*/
//         if (param->operation != SMPC_SECURITY)
//         {
//             status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                               SMP_ERROR_INVALID_PARAM);
//             break;
//         }

//         // State is either SMPC_IDLE or SMPC_BUSY
//         if (state == SMPC_IDLE)
//         {
//             // Send the pairing request PDU to the peer device
//             smpc_pdu_send(idx, L2C_CODE_SECURITY_REQUEST, (void *)&param->auth);

//             // Keep the command message until the end of the request => State is now SMPC_BUSY
//             smpc_init_operation(idx, (void *)param);

//             // Inform the kernel that this message cannot be unallocated
//             msg_status = KE_MSG_NO_FREE;
//         }
//         else if (state != SMPC_FREE)
//         {
//             msg_status = KE_MSG_SAVED;
//         }
//         else
//         {
//             ASSERT_ERR(0);
//         }
//     } while (0);
//     // else an error has occurred and the status value has been updated.

//     // Inform the source task if an error has occurred
//     if (status != SMP_ERROR_NO_ERROR)
//     {
//         smpc_send_cmp_evt(idx, KE_BUILD_ID(TASK_GAPC, idx), SMPC_SECURITY, status);
//     }

//     return msg_status;
// }
// #endif //(BLE_PERIPHERAL)

// #if (BLE_CENTRAL)
// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpc_start_enc_cmd_handler(ke_msg_id_t const msgid,
//                                       struct smpc_start_enc_cmd *param,
//                                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Command Status
//     uint8_t status      = SMP_ERROR_NO_ERROR;
//     // Message Status
//     uint8_t msg_status  = KE_MSG_CONSUMED;

//     // State
//     ke_state_t state    = ke_state_get(dest_id);
//     // The command can be handled
//     uint8_t idx = KE_IDX_GET(dest_id);

//     // Check if the command can be handled
//     if (param->operation == SMPC_START_ENC)
//     {
//         // State is either SMPC_IDLE or SMPC_BUSY
//         if (state == SMPC_IDLE)
//         {
//             // Keep the command message until the end of the request
//             smpc_init_operation(idx, (void *)param);

//             // Inform the kernel that this message cannot be unallocated
//             msg_status = KE_MSG_NO_FREE;

//             smpc_env[idx]->state = SMPC_START_ENC_LTK;

//             // Send start encryption to LL
//             smpc_send_start_enc_cmd(idx, SMPC_USE_LTK,
//                                     &param->ltk.ltk.key[0], &param->ltk.randnb.nb[0], param->ltk.ediv);
//         }
//         else if (state != SMPC_FREE)
//         {
//             msg_status = KE_MSG_SAVED;
//         }
//         else
//         {
//             status = SMP_ERROR_REQ_DISALLOWED;
//         }
//     }
//     else
//     {
//         status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                           SMP_ERROR_INVALID_PARAM);
//     }

//     // Inform the source task if an error has occurred
//     if (status != SMP_ERROR_NO_ERROR)
//     {
//         smpc_send_cmp_evt(idx, KE_BUILD_ID(TASK_GAPC, idx), SMPC_START_ENC, status);
//     }

//     return msg_status;
// }
// #endif //(BLE_CENTRAL)

// #if (BLE_PERIPHERAL)
// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpc_start_enc_cfm_handler(ke_msg_id_t const msgid,
//                                       struct smpc_start_enc_cfm const *param,
//                                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Message Status
//     uint8_t msg_status  = KE_MSG_CONSUMED;
//     // Retrieve the connection index
//     uint8_t idx = KE_IDX_GET(dest_id);

//     if (ke_state_get(dest_id) != SMPC_FREE)
//     {
//     // Check if a LTK has been found
//     if (param->accept)
//     {
//         // State
//         ke_state_t state    = ke_state_get(dest_id);

//         if (state == SMPC_IDLE)
//         {
//             // Set state to busy
//             ke_state_set(dest_id, SMPC_BUSY);

//             // Update the procedure state value
//             smpc_env[idx]->state = SMPC_START_ENC_LTK;

//             // Send the LTK to the controller so that it can start the encryption
//             smpc_send_ltk_req_rsp(idx, true, (uint8_t *)&param->ltk.key[0]);
//         }
//             else if (state != SMPC_FREE)
//         {
//             msg_status = KE_MSG_SAVED;
//         }
//         // else drop the packet
//     }
//     else
//     {
//         smpc_send_ltk_req_rsp(idx, false, NULL);
//     }
//     }

//     return msg_status;
// }
// #endif //(BLE_PERIPHERAL)

// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpc_sign_cmd_handler(ke_msg_id_t const msgid,
//                                  struct smpc_sign_cmd *param,
//                                  ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Command Status
//     uint8_t status      = SMP_ERROR_NO_ERROR;
//     // Message Status
//     uint8_t msg_status  = KE_MSG_CONSUMED;
//     // State
//     ke_state_t state    = ke_state_get(dest_id);
//     // Index
//     uint8_t idx         = KE_IDX_GET(dest_id);

//     ASSERT_ERR((KE_TYPE_GET(src_id) == TASK_GATTC) || (KE_TYPE_GET(src_id) == TASK_ATTS));

//     // Check if the command can be handled
//     if (param->operation == SMPC_SIGN)
//     {
//         // State is either SMPC_IDLE or SMPC_BUSY
//         if (state == SMPC_IDLE)
//         {
//             // Check the provided flag
//             if ((param->flag == SMPC_SIGN_GEN) || (param->flag == SMPC_SIGN_VERIF))
//             {
//                 ASSERT_ERR(smpc_env[idx]->sign_info == NULL);

//                 // Allocate memory for the signature information structure
//                 smpc_env[idx]->sign_info = ke_malloc(sizeof(struct smpc_sign_info), KE_MEM_ENV);

//                 // Check if memory has been allocated
//                 if (smpc_env[idx]->sign_info != NULL)
//                 {
//                     //Source of the CSRK
//                     uint8_t csrk_src = GAPC_INFO_LOCAL;

//                     // Set C0 value = 0[0:127]
//                     memset(&(smpc_env[idx]->sign_info->cn1[0]), 0x00, KEY_LEN);

//                     if (param->flag == SMPC_SIGN_GEN)
//                     {
//                         /*
//                          * **************************
//                          * * DATA PDU * SIGNCOUNTER *
//                          * **************************
//                          * LSB -----------------> MSB
//                          */

//                         uint32_t local_sign_counter = gapc_get_sign_counter(idx, GAPC_INFO_LOCAL);


//                         // Put the SignCounter in the message to sign
//                         co_write32p(&param->msg[param->byte_len - SMPC_SIGN_COUNTER_LEN], local_sign_counter);

//                         // Block counter value is the number of 128-bits block in the Data PDU
//                         smpc_env[idx]->sign_info->block_nb   = (uint16_t)((((param->byte_len << 1) - 1) / (KEY_LEN << 1)) + 1);
//                         // Set the message offset
//                         smpc_env[idx]->sign_info->msg_offset = param->byte_len;
//                     }
//                     else if (param->flag == SMPC_SIGN_VERIF)
//                     {
//                         /*
//                          * ******************************************
//                          * * DATA PDU * SIGNCOUNTER * MAC (8 bytes) *
//                          * ******************************************
//                          * LSB ---------------------------------> MSB
//                          */

//                         ASSERT_ERR(param->byte_len > SMPC_SIGN_LEN);

//                         // Peer SignCounter
//                         uint32_t peer_sign_counter;
//                         // Received SignCounter
//                         uint32_t rcv_sign_counter;

//                         co_write32(&rcv_sign_counter, (uint32_t)param->msg[param->byte_len - SMPC_SIGN_LEN]);

//                         // Stored SignCounter
//                         peer_sign_counter = gapc_get_sign_counter(idx, GAPC_INFO_PEER);

//                         // Check signCounter
//                         if (rcv_sign_counter >= peer_sign_counter)
//                         {
//                             // Block counter value is the number of 128-bits block in the Data PDU - MAC not used
//                             smpc_env[idx]->sign_info->block_nb
//                                 = (uint16_t)(((((param->byte_len - SMPC_SIGN_MAC_LEN) << 1) - 1) / (KEY_LEN << 1)) + 1);
//                             // Set the message offset
//                             smpc_env[idx]->sign_info->msg_offset = param->byte_len - SMPC_SIGN_MAC_LEN;

//                             csrk_src = GAPC_INFO_PEER;
//                         }
//                         else
//                         {
//                             status = SMP_ERROR_SIGN_VERIF_FAIL;
//                         }
//                     }

//                     if (status == SMP_ERROR_NO_ERROR)
//                     {
//                         // Keep the command message until the end of the request
//                         smpc_init_operation(idx, (void *)param);

//                         smpc_env[idx]->sign_info->requester = src_id;

//                         // Inform the kernel that this message cannot be unallocated
//                         msg_status = KE_MSG_NO_FREE;


//                         // Size of PDU to sign is lower than or equal to 128bits
//                         if (smpc_env[idx]->sign_info->block_nb == 1)
//                         {
//                             /*
//                              * ---------------------------------------------------
//                              * GENERATE L ----------------------------------------
//                              *----------------------------------------------------
//                              */
//                             smpc_generate_l(idx, csrk_src);
//                         }
//                         else    // Size of PDU to sign is upper than 128bits
//                         {
//                             ASSERT_ERR(smpc_env[idx]->sign_info->msg_offset > KEY_LEN);
//                             /*
//                              * ---------------------------------------------------
//                              * GENERATE C1 ---------------------------------------
//                              *----------------------------------------------------
//                              */
//                             smpc_generate_ci(idx, csrk_src,
//                                              &smpc_env[idx]->sign_info->cn1[0],
//                                              &param->msg[smpc_env[idx]->sign_info->msg_offset - KEY_LEN]);
//                         }
//                     }
//                 }
//                 else
//                 {
//                     status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                                       SMP_ERROR_UNSPECIFIED_REASON);
//                 }
//             }
//             else
//             {
//                 status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                                   SMP_ERROR_INVALID_PARAM);
//             }
//         }
//         else if (state != SMPC_FREE)
//         {
//             msg_status = KE_MSG_SAVED;
//         }
//         else
//         {
//             status = SMP_ERROR_REQ_DISALLOWED;
//         }
//     }
//     else
//     {
//         status = SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK,
//                                           SMP_ERROR_INVALID_PARAM);
//     }

//     // Inform the source task if an error has occurred
//     if (status != SMP_ERROR_NO_ERROR)
//     {
//         smpc_send_cmp_evt(idx, src_id, SMPC_SIGN, status);
//     }

//     return msg_status;
// }

// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpc_timeout_timer_ind_handler(ke_msg_id_t const msgid,
//                                           void const *param,
//                                           ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Recover connection index
//     uint8_t idx    = KE_IDX_GET(dest_id);

//     if (ke_state_get(dest_id) != SMPC_FREE)
//     {
//     // Disable the timer state in the environment
//     SMPC_TIMER_UNSET_FLAG(idx, SMPC_TIMER_TIMEOUT_FLAG);
//     // No more SM procedure may occur
//     SMPC_TIMER_SET_FLAG(idx, SMPC_TIMER_TIMEOUT_BLOCKED_FLAG);

//     // Inform the HL that an error has occurred.
//         smpc_pairing_end(idx, gapc_get_role(idx), SMP_ERROR_TIMEOUT, false);
//     }

//     return (KE_MSG_CONSUMED);
// }

// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpc_rep_attempts_timer_handler(ke_msg_id_t const msgid,
//                                            void const *param,
//                                            ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     if (ke_state_get(dest_id) != SMPC_FREE)
// {
//     // Recover connection index
//     uint8_t idx = KE_IDX_GET(dest_id);

//         ASSERT_ERR(SMPC_IS_FLAG_SET(idx, SMPC_TIMER_REP_ATT_FLAG));
//     // Reset the timer value
//     smpc_env[idx]->rep_att_timer_val = SMPC_REP_ATTEMPTS_TIMER_DEF_VAL;

//     // Update the timer flag
//     SMPC_TIMER_UNSET_FLAG(idx, SMPC_TIMER_REP_ATT_FLAG);
//     }

//     //message is consumed
//     return (KE_MSG_CONSUMED);
// }

// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpm_use_enc_block_ind_handler(ke_msg_id_t const msgid,
//                                           struct gapm_use_enc_block_ind *param,
//                                           ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Index
//     uint8_t idx     = KE_IDX_GET(dest_id);
//     // Role
//     uint8_t role    = gapc_get_role(idx);

//     if (ke_state_get(dest_id) != SMPC_FREE)
//     {
//     switch (smpc_env[idx]->state)
//     {
//         // Signature Procedure
//         case (SMPC_SIGN_L_GEN):
//         case (SMPC_SIGN_Ci_GEN):
//         {
//             // Retrieve the command request
//             struct smpc_sign_cmd *sign_cmd   = (struct smpc_sign_cmd *)smpc_env[idx]->operation;

//             ASSERT_ERR(sign_cmd  != NULL);
//             ASSERT_ERR(smpc_env[idx]->sign_info != NULL);
//             ASSERT_ERR(sign_cmd->operation == SMPC_SIGN);

//             // CSRK Source
//             uint8_t csrk_src;

//             // Get the source of the CSRK
//             if (sign_cmd->flag == SMPC_SIGN_GEN)
//             {
//                 csrk_src = GAPC_INFO_LOCAL;
//             }
//             else        // SMPC_SIGN_VERIF
//             {
//                 csrk_src = GAPC_INFO_PEER;
//             }

//             // L has been generated => Mn => Cn
//             if (smpc_env[idx]->state == SMPC_SIGN_L_GEN)
//             {
//                 // Subkey (K1 or K2)
//                 uint8_t subkey[KEY_LEN];
//                 // Mn value
//                 uint8_t mn[KEY_LEN];
//                     ASSERT_ERR(smpc_env[idx]->sign_info->msg_offset <= KEY_LEN);

//                 /*
//                  ******************************************************************
//                  * COMPUTE Mn
//                  * Computing method is different following remaining size.
//                  ******************************************************************
//                  */

//                     if (smpc_env[idx]->sign_info->msg_offset == KEY_LEN)     // Remaining block is complete
//                 {
//                     // Generate K1
//                     smpc_calc_subkeys(false, (uint8_t *)&param->result[0], &subkey[0]);

//                     // Compute Mn = K1 XOR Mn'
//                         smpc_xor(&mn[0], &subkey[0], &sign_cmd->msg[0]);
//                 }
//                     else if (smpc_env[idx]->sign_info->msg_offset < KEY_LEN)     // Remaining block is not complete
//                 {
//                     // Remaining PDU is shorter than 128 - needs padding
//                     memset(&mn[0], 0x00, KEY_LEN);
//                         memcpy(&mn[KEY_LEN - smpc_env[idx]->sign_info->msg_offset],
//                                &sign_cmd->msg[0], smpc_env[idx]->sign_info->msg_offset);
//                         mn[KEY_LEN - smpc_env[idx]->sign_info->msg_offset - 1] = 0x80;

//                     // Generate K2
//                     smpc_calc_subkeys(true, (uint8_t *)&param->result[0], &subkey[0]);

//                     // Compute Mn = K2 XOR (Mn' || padding)
//                     smpc_xor(&mn[0], &subkey[0], &mn[0]);
//                 }

//                 /*
//                  ******************************************************************
//                  * COMPUTE Cn
//                  ******************************************************************
//                  */

//                 smpc_generate_ci(idx, csrk_src, &smpc_env[idx]->sign_info->cn1[0], &mn[0]);
//             }
//             else if (smpc_env[idx]->state == SMPC_SIGN_Ci_GEN)
//             {
//                 switch (smpc_env[idx]->sign_info->block_nb)
//                 {
//                     case (0):   // Last Ci has been computed => MAC
//                     {
//                         uint8_t status = SMP_ERROR_NO_ERROR;

//                         // Inform the HL that one of the sign counter has been updated
//                         struct smpc_sign_counter_ind *sign_counter_ind = KE_MSG_ALLOC(SMPC_SIGN_COUNTER_IND,
//                                                                                       KE_BUILD_ID(TASK_GAPC, idx),
//                                                                                       KE_BUILD_ID(TASK_SMPC, idx),
//                                                                                       smpc_sign_counter_ind);

//                         if (sign_cmd->flag == SMPC_SIGN_GEN)
//                         {
//                             // Signed Message length
//                             uint16_t length = sign_cmd->byte_len + SMPC_SIGN_MAC_LEN;

//                             // Send the signed data to the requester
//                             struct smpc_sign_ind *ind = KE_MSG_ALLOC_DYN(SMPC_SIGN_IND,
//                                                                          smpc_env[idx]->sign_info->requester,
//                                                                          KE_BUILD_ID(TASK_SMPC, idx),
//                                                                              smpc_sign_ind,
//                                                                              length);

//                             ind->type      = sign_cmd->flag;
//                             // Data PDU length (Bytes)
//                             ind->byte_len  = length;

//                             // Original Message
//                             memcpy(&ind->signed_msg[0], &sign_cmd->msg[0], sign_cmd->byte_len);

//                                 // Append MAC - MAC = MSB64(result)
//                                 memcpy(&(ind->signed_msg[sign_cmd->byte_len]),
//                                        &(param->result[KEY_LEN - SMPC_SIGN_MAC_LEN]), SMPC_SIGN_MAC_LEN);

//                             ke_msg_send(ind);

//                             // Set the sign counter values
//                             co_write32(&sign_counter_ind->local_sign_counter,
//                                        (uint32_t)sign_cmd->msg[sign_cmd->byte_len - SMPC_SIGN_COUNTER_LEN]);
//                                 // Update the signCounter value in the GAP
//                                 sign_counter_ind->local_sign_counter++;
//                             sign_counter_ind->peer_sign_counter = gapc_get_sign_counter(idx, GAPC_INFO_PEER);
//                         }
//                         else        // SMPC_SIGN_VERIF
//                         {
//                                 // Compare generated MAC and received MAC (MAC = MSB64(Cn))
//                                 if (!memcmp(&param->result[KEY_LEN - SMPC_SIGN_MAC_LEN],
//                                             &sign_cmd->msg[sign_cmd->byte_len- SMPC_SIGN_MAC_LEN], SMPC_SIGN_MAC_LEN))
//                             {

//                                 // The signature is approved
//                                 struct smpc_sign_ind *ind = KE_MSG_ALLOC_DYN(SMPC_SIGN_IND,
//                                                                              smpc_env[idx]->sign_info->requester,
//                                                                              KE_BUILD_ID(TASK_SMPC, idx),
//                                                                              smpc_sign_ind, sign_cmd->byte_len);

//                                 ind->type      = sign_cmd->flag;
//                                 // Data PDU length (Bytes)
//                                 ind->byte_len  = sign_cmd->byte_len;
//                                 // Data PDU + SignCounter + MAC (LSB->MSB)
//                                 memcpy(&ind->signed_msg[0], &sign_cmd->msg[0], sign_cmd->byte_len);

//                                 ke_msg_send(ind);

//                                 // Set the sign counter values
//                                 co_write32(&sign_counter_ind->peer_sign_counter,
//                                            (uint32_t)sign_cmd->msg[sign_cmd->byte_len - SMPC_SIGN_LEN]);
//                                     // Update the signCounter value in the GAP
//                                     sign_counter_ind->peer_sign_counter++;
//                                 sign_counter_ind->local_sign_counter = gapc_get_sign_counter(idx, GAPC_INFO_LOCAL);
//                             }
//                             else
//                             {
//                                 // The signature is not approved.
//                                 status = SMP_ERROR_SIGN_VERIF_FAIL;
//                             }
//                         }

//                         if (status == SMP_ERROR_NO_ERROR)
//                         {
//                             ke_msg_send(sign_counter_ind);
//                         }
//                         else
//                         {
//                             // Don't send the message
//                             ke_msg_free(ke_param2msg(sign_counter_ind));
//                         }

//                         // Send CMP_EVT to the requester
//                         smpc_send_cmp_evt(idx, smpc_env[idx]->sign_info->requester, SMPC_SIGN, status);
//                     } break;

//                     case (1):   // The last Ci will be computed but before that we need subkeys
//                     {
//                         // Keep the last computed Ci value
//                         memcpy(&smpc_env[idx]->sign_info->cn1[0], &param->result[0], KEY_LEN);

//                         /*
//                          * ---------------------------------------------------
//                          * GENERATE L ----------------------------------------
//                          *----------------------------------------------------
//                          */

//                         smpc_generate_l(idx, csrk_src);
//                     } break;

//                     default:    // Next Ci to compute
//                     {
//                             ASSERT_ERR(smpc_env[idx]->sign_info->msg_offset > KEY_LEN);
//                         /*
//                          * ---------------------------------------------------
//                          * GENERATE Ci ---------------------------------------
//                          *----------------------------------------------------
//                          */

//                         smpc_generate_ci(idx, csrk_src, (uint8_t *)&param->result[0],
//                                                &sign_cmd->msg[smpc_env[idx]->sign_info->msg_offset - KEY_LEN]);
//                     }
//                 }
//             }
//         } break;

//         // E1 value used in the Confirm Value generation
//         case (SMPC_PAIRING_CFM_P1):
//         {
//             ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

//             smpc_env[idx]->state = SMPC_PAIRING_CFM_P2;

//             smpc_generate_cfm(idx, role, &param->result[0]);
//         } break;

//         case (SMPC_PAIRING_REM_CFM_P1):
//         {
//             ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

//             smpc_env[idx]->state = SMPC_PAIRING_REM_CFM_P2;

//             smpc_generate_cfm(idx, role, &param->result[0]);
//         } break;

//         case (SMPC_PAIRING_CFM_P2):
//         {
//             /*
//              * Confirm value has been generated, next step is to send it to the peer device
//              */

//             ASSERT_ERR(smpc_env[idx]->pair_info != NULL);


//             // In any role, the confirm value is sent right after calculation
//                 smpc_pdu_send(idx, L2C_CODE_PAIRING_CONFIRM, (void *)&param->result[0]);

//             /*
//              * The slave device shall send its own confirm value upon reception of the master
//              * confirm value.
//              */
//             if (role == ROLE_MASTER)
//             {
//                 #if (BLE_CENTRAL)
//                 smpc_env[idx]->state = SMPC_PAIRING_WAIT_CONFIRM;
//                 #endif //(BLE_CENTRAL)
//             }
//             else
//             {
//                 #if (BLE_PERIPHERAL)
//                 smpc_env[idx]->state = SMPC_PAIRING_WAIT_RAND;
//                 #endif //(BLE_PERIPHERAL)
//             }
//         } break;

//         case (SMPC_PAIRING_REM_CFM_P2):
//         {
//             // Status
//             uint8_t status = SMP_ERROR_NO_ERROR;

//             ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

//             /*
//              * ********************************************
//              * COMPARE CONFIRM VALUES
//              * ********************************************
//              */

//                 if (!memcmp(&(smpc_env[idx]->pair_info->conf_value[0]), &(param->result[0]), KEY_LEN))
//             {
//                 if (role == ROLE_SLAVE)
//                 {
//                     #if (BLE_PERIPHERAL)
//                     // If slave, this calculation is made prior to sending the Rand value
//                         smpc_pdu_send(idx, L2C_CODE_PAIRING_RANDOM, (void *)&smpc_env[idx]->pair_info->rand[0]);

//                     // Set the state
//                     smpc_env[idx]->state = SMPC_START_ENC_STK;
//                     #endif //(BLE_PERIPHERAL)
//                 }
//                 else    // role = ROLE_MASTER
//                 {
//                     #if (BLE_CENTRAL)
//                     smpc_generate_stk(idx, ROLE_MASTER);
//                     #endif //(BLE_CENTRAL)
//                 }
//             }
//             else
//             {
//                 status = SMP_ERROR_CONF_VAL_FAILED;

//                     smpc_pdu_send(idx, L2C_CODE_PAIRING_FAILED, &status);

//                     // Start the Repeated Attempts timer
//                     smpc_pairing_end(idx, role,
//                                      SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK, status), true);
//             }
//         } break;

//         case (SMPC_PAIRING_GEN_STK):
//         {
//             // Counter
//             uint8_t counter;

//             ASSERT_ERR(smpc_env[idx]->pair_info != NULL);


//             // Mask the right bytes to respect key size
//             for (counter = gapc_get_enc_keysize(idx); counter < KEY_LEN; counter++)
//             {
//                     // The result is the LTK
//                     param->result[counter] = 0x00;
//             }

//             smpc_env[idx]->state = SMPC_START_ENC_STK;

//             // If Master, start encryption using STK
//             if (role == ROLE_MASTER)
//             {
//                 #if (BLE_CENTRAL)
//                 // Send a Start Encryption Request using the STK
//                     smpc_send_start_enc_cmd(idx, SMPC_USE_STK, &param->result[0], NULL, 0);
//                 #endif //(BLE_CENTRAL)
//             }
//             // If slave, STK was calculated for LTK_REQ_REPLY
//             else
//             {
//                 #if (BLE_PERIPHERAL)
//                     smpc_send_ltk_req_rsp(idx, true, &param->result[0]);
//                 #endif //(BLE_PERIPHERAL)
//             }
//         } break;

//         default:
//         {
//             ASSERT_ERR(0);
//         } break;
//         }
//     }

//     return (KE_MSG_CONSUMED);
// }

// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpm_gen_rand_nb_ind_handler(ke_msg_id_t const msgid,
//                                         struct gapm_gen_rand_nb_ind const *param,
//                                         ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Index
//     uint8_t idx     = KE_IDX_GET(dest_id);

//     if (ke_state_get(dest_id) != SMPC_FREE)
//     {
//     switch (smpc_env[idx]->state)
//     {
//         // Signature Procedure
//         case (SMPC_PAIRING_GEN_RAND_P1):
//         {
//             ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

//             memcpy(&smpc_env[idx]->pair_info->rand[0], &param->randnb.nb[0], RAND_NB_LEN);

//             smpc_generate_rand(idx, SMPC_PAIRING_GEN_RAND_P2);
//         } break;

//         case (SMPC_PAIRING_GEN_RAND_P2):
//         {
//             ASSERT_ERR(smpc_env[idx]->pair_info != NULL);

//             memcpy(&smpc_env[idx]->pair_info->rand[RAND_NB_LEN], &param->randnb.nb[0], RAND_NB_LEN);

//             smpc_env[idx]->state = SMPC_PAIRING_CFM_P1;

//             // Random value has been generated => start confirm value generation
//             smpc_generate_e1(idx, gapc_get_role(idx), true);
//         } break;

//         default:
//         {
//             ASSERT_ERR(0);
//         } break;
//     }
//     }

//     return KE_MSG_CONSUMED;
// }

// /**
//  ****************************************************************************************
//  * @brief Handles Encryption Start request from HL (when LTK is known).
//  * @param[in] msgid     Id of the message received.
//  * @param[in] req       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int smpm_cmp_evt_handler(ke_msg_id_t const msgid,
//                                 struct smpm_cmp_evt const *param,
//                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     if (ke_state_get(dest_id) != SMPC_FREE)
//     {
//     if (param->status != SMP_ERROR_NO_ERROR)
//     {
//         // Index
//         uint8_t idx         = KE_IDX_GET(dest_id);
//         // Destination
//         ke_task_id_t dest   = 0;
//         // Operation Code
//         uint8_t operation;

//         ASSERT_ERR(smpc_env[idx]->operation != NULL);

//         operation = ((struct smp_cmd *)(smpc_env[idx]->operation))->operation;

//         if (operation == SMPC_SIGN)
//         {
//             ASSERT_ERR(smpc_env[idx]->sign_info != NULL);

//             dest = smpc_env[idx]->sign_info->requester;
//         }
//         else if (operation == SMPC_PAIRING)
//         {
//             dest = KE_BUILD_ID(TASK_GAPC, idx);
//         }
//         else
//         {
//             ASSERT_ERR(0);
//         }

//          smpc_send_cmp_evt(idx, dest, operation, param->status);
//     }
//     }

//     return KE_MSG_CONSUMED;
// }

// /**
//  ****************************************************************************************
//  * @brief Handles the LLc encryption change event.
//  *        This event with Ok status will symbolize the end of the link encryption session
//  *        for a link that was not encrypted to start with.
//  *        It can arrive in two cases
//  *         - the un-encrypted link is encrypted with STK after pairing
//  *         - the un-encrypted link is encrypted with LTK using existing LTK, wo pairing
//  * Both slave and master receive this event, after which, if status OK and no TKDP,
//  * they send the host an ENC_STARTED_IND.
//  *
//  * @param[in] msgid     Id of the message received.
//  * @param[in] event     Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int llc_enc_chg_evt_handler(ke_msg_id_t const msgid,
//                                    struct llc_enc_change_evt const *param,
//                                    ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     uint8_t conidx = gapc_get_conidx(param->conhdl);

//     if (conidx != GAP_INVALID_CONIDX)
//     {
//         smpc_handle_enc_change_evt(conidx, gapc_get_role(conidx), param->status);
//     }

//     return (KE_MSG_CONSUMED);
// }

// /**
//  ****************************************************************************************
//  * @brief Handles the LLC encryption key refresh event.
//  *        This event with Ok status will symbolize the end of the link encryption session
//  *        with a new key . This can arrive in two cases:
//  *         - the pairing and encryption using STK was done on an already encrypted link
//  *         - the link encrypted under STK is re-encrypted using LTK.
//  *        Any different status will be signaled to the Host as error in encrypting the link.
//  * Both slave and master receive this event, after which, if encryption successful,
//  * they send the host an ENC_STARTED_IND.
//  *
//  * @param[in] msgid     Id of the message received.
//  * @param[in] event     Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance.
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int llc_enc_key_refr_evt_handler(ke_msg_id_t const msgid,
//                                         struct llc_enc_key_ref_cmp_evt const *param,
//                                         ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     uint8_t conidx = gapc_get_conidx(param->conhdl);

//     if (conidx != GAP_INVALID_CONIDX)
//     {
//         smpc_handle_enc_change_evt(conidx, gapc_get_role(conidx), param->status);
//     }

//     return (KE_MSG_CONSUMED);
// }

// #if (BLE_PERIPHERAL)
// /**
//  ****************************************************************************************
//  * @brief Handles long term key request from link layer.
//  * Link layer checks if host has long term key corresponding to the given EDIV and Rand.
//  * Only a slave can receive this event,
//  * It is due to session encryption procedure started by Master.
//  *
//  * @param[in] msgid     Id of the message received.
//  * @param[in] event     Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance (TASK_SMP).
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int llc_le_ltk_req_evt_handler(ke_msg_id_t const msgid,
//                                       struct llc_le_ltk_req const *param,
//                                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Recover connection index
//     uint8_t idx = gapc_get_conidx(param->conhdl);

//     // Flag used to differentiate between LTK and STK requests (depends on rand is 0 or not)
//     bool ltk_req = false;
//     // Counter
//     uint8_t counter;

//     if (idx != GAP_INVALID_CONIDX)
//     {
//         // If rand is 0, then start STK calculation
//         for (counter = 0; ((counter < RAND_NB_LEN) && !ltk_req); counter++)
//         {
//             if (param->rand.nb[counter] != 0x00)
//             {
//                 ltk_req = true;
//             }
//         }

//         // Master started encryption with LTK
//         if (ltk_req)
//         {
//             // Indicate the host that a LTK is required
//             struct smpc_start_enc_req_ind  *ind = KE_MSG_ALLOC(SMPC_START_ENC_REQ_IND,
//                                                                KE_BUILD_ID(TASK_GAPC, idx), dest_id,
//                                                                smpc_start_enc_req_ind);

//             // Set EDIV
//             ind->ediv = param->ediv;
//             // Set Random Value
//             memcpy(&ind->randnb, &param->rand, sizeof(struct rand_nb));

//             ke_msg_send(ind);
//         }
//         // Master started encryption with STK
//         else
//         {
//             // Calculate STK
//             smpc_generate_stk(idx, ROLE_SLAVE);
//         }
//     }

//     //message is consumed
//     return (KE_MSG_CONSUMED);
// }
// #endif //(BLE_PERIPHERAL)

// #if (BLE_CENTRAL)
// /**
//  ****************************************************************************************
//  * @brief Handles common command status events from LLC.
//  *
//  * @param[in] msgid     Id of the message received.
//  * @param[in] event     Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance (TASK_SMP).
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int llc_le_start_enc_stat_evt_handler(ke_msg_id_t const msgid,
//                                                struct llc_event_common_cmd_status const *event,
//                                                ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     // Connection Index
//     uint8_t idx = KE_IDX_GET(dest_id);

//     if (ke_state_get(dest_id) != SMPC_FREE)
//     {
//     /*
//      * The LLC_LE_START_ENC_STAT_EVT is sent by the LL upon reception of the LLC_LE_START_ENC_CMD
//      */
//     if (event->status != CO_ERROR_NO_ERROR)
//     {
//         // Encryption has not be started due to an error in LL
//         ASSERT_ERR(smpc_env[KE_IDX_GET(dest_id)]->operation != NULL);
//         ASSERT_ERR(((struct smp_cmd *)(smpc_env[KE_IDX_GET(dest_id)]->operation))->operation == SMPC_START_ENC);

//         smpc_send_cmp_evt(idx, KE_BUILD_ID(TASK_GAPC, idx), SMPC_START_ENC, SMP_ERROR_LL_ERROR);
//     }
//     }

//     return (KE_MSG_CONSUMED);
// }
// #endif //(BLE_CENTRAL)

// /**
//  ****************************************************************************************
//  * @brief Handles data packet from link layer.
//  * This will take the raw L2CAP data and process the information inside the packet.
//  * If TO happens during pairing procedure and Host receives the error IND, the
//  * link will be disconnected, so any PDU arriving after should not even arrive due to lack
//  * of L2CAP channel bearer.
//  *
//  * @param[in] msgid     Id of the message received.
//  * @param[in] ind       Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance (TASK_SMP).
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int l2cc_pdu_recv_ind_handler(ke_msg_id_t const msgid,
//                                         struct l2cc_pdu_recv_ind *param,
//                                         ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     int msg_status = KE_MSG_CONSUMED;
//     uint8_t state = ke_state_get(dest_id);

//     // In encrypt state, postpone message process
//     if (state == SMPC_ENCRYPT)
//     {
//         msg_status = KE_MSG_SAVED;
//     }
//     // Allow PDU reception in any state but FREE
//     else if (state != SMPC_FREE)
//     {
//         if(param->status == L2C_ERR_NO_ERROR)
//         {
//             // Decide next action depending on received PDU
//             smpc_pdu_recv(KE_IDX_GET(dest_id), &(param->pdu));
//         }
//         else if(param->status == L2C_ERR_INVALID_PDU)
//         {
//             uint8_t conidx = KE_IDX_GET(dest_id);

//             // Allocate the message for LLC task to inform that an invalid PDU has been received
//             struct l2cc_pdu_send_req *err_msg = KE_MSG_ALLOC(L2CC_PDU_SEND_REQ,
//                                                              KE_BUILD_ID(TASK_L2CC, conidx),
//                                                              KE_BUILD_ID(TASK_SMPC, conidx),
//                                                              l2cc_pdu_send_req);

//             err_msg->pdu.chan_id   = L2C_CID_SECURITY;
//             err_msg->pdu.data.code = L2C_CODE_PAIRING_FAILED;
//             // invalid op code or PDU
//             err_msg->pdu.data.pairing_failed.reason = SMP_ERROR_CMD_NOT_SUPPORTED;

//             // Send message to L2CAP
//             ke_msg_send(err_msg);
//         }
//     }
//     // else drop the packet

//     return msg_status;
// }

// /**
//  ****************************************************************************************
//  * @brief Handles data send response from L2CAP.
//  *  this is used to know when an indication has been correctly sent
//  *
//  * @param[in] msgid     Id of the message received.
//  * @param[in] param     Pointer to the parameters of the message.
//  * @param[in] dest_id   ID of the receiving task instance (TASK_ATTS).
//  * @param[in] src_id    ID of the sending task instance.
//  *
//  * @return If the message was consumed or not.
//  ****************************************************************************************
//  */
// static int l2cc_data_send_rsp_handler(ke_msg_id_t const msgid,
//                                       struct l2cc_data_send_rsp *param,
//                                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
// {
//     #if (BLE_PERIPHERAL)
//     // Recover connection index
//     uint8_t idx = KE_IDX_GET(dest_id);

//     if (ke_state_get(dest_id) != SMPC_FREE)
//     {
//         // Security request message sent, inform GAP that security request procedure is finished.
//         if (ke_state_get(dest_id) != SMPC_FREE)
//         {
//             if (smpc_env[idx]->operation != NULL)
//             {
//                 if (((struct smp_cmd *)(smpc_env[idx]->operation))->operation == SMPC_SECURITY)
//                 {
//                     // Security request performed
//                     smpc_send_cmp_evt(idx, KE_BUILD_ID(TASK_GAPC, idx), SMPC_SECURITY, SMP_ERROR_NO_ERROR);
//                 }
//             }
//         }
//     }
//     #endif //(BLE_PERIPHERAL)

//     return (KE_MSG_CONSUMED);
// }

// /*
//  * TASK DESCRIPTOR DEFINITIONS
//  ****************************************************************************************
//  */
// /// Specifies the default message handlers
// const struct ke_msg_handler smpc_default_state[] =
// {
//     #if (BLE_CENTRAL)
//     {SMPC_PAIRING_CMD,                (ke_msg_func_t)smpc_pairing_cmd_handler},
//     {SMPC_START_ENC_CMD,              (ke_msg_func_t)smpc_start_enc_cmd_handler},

//     {LLC_LE_START_ENC_STAT_EVT,       (ke_msg_func_t)llc_le_start_enc_stat_evt_handler},
//     #endif //(BLE_CENTRAL)

//     #if (BLE_PERIPHERAL)
//     {SMPC_SECURITY_CMD,               (ke_msg_func_t)smpc_security_cmd_handler},
//     {SMPC_START_ENC_CFM,              (ke_msg_func_t)smpc_start_enc_cfm_handler},

//     {LLC_LE_LTK_REQ_EVT,              (ke_msg_func_t)llc_le_ltk_req_evt_handler},
//     {LLC_LE_LTK_REQ_RPLY_CMP_EVT,     (ke_msg_func_t)ke_msg_discard},
//     {LLC_LE_LTK_REQ_NEG_RPLY_CMP_EVT, (ke_msg_func_t)ke_msg_discard},
//     #endif //(BLE_PERIPHERAL)

//     {SMPC_PAIRING_CFM,                (ke_msg_func_t)smpc_pairing_cfm_handler},

//     {SMPC_SIGN_CMD,                   (ke_msg_func_t)smpc_sign_cmd_handler},

//     {SMPC_TIMEOUT_TIMER_IND,          (ke_msg_func_t)smpc_timeout_timer_ind_handler},
//     {SMPC_REP_ATTEMPTS_TIMER_IND,     (ke_msg_func_t)smpc_rep_attempts_timer_handler},

//     {SMPM_USE_ENC_BLOCK_IND,          (ke_msg_func_t)smpm_use_enc_block_ind_handler},
//     {SMPM_GEN_RAND_NB_IND,            (ke_msg_func_t)smpm_gen_rand_nb_ind_handler},
//     {SMPM_CMP_EVT,                    (ke_msg_func_t)smpm_cmp_evt_handler},

//     {L2CC_PDU_RECV_IND,               (ke_msg_func_t)l2cc_pdu_recv_ind_handler},
//     {L2CC_DATA_SEND_RSP,              (ke_msg_func_t)l2cc_data_send_rsp_handler},

//     {LLC_ENC_CHANGE_EVT,              (ke_msg_func_t)llc_enc_chg_evt_handler},
//     {LLC_ENC_KEY_REFRESH_CMP_EVT,     (ke_msg_func_t)llc_enc_key_refr_evt_handler},

// };


// /// Specifies the message handler structure for every input state.
// const struct ke_state_handler smpc_state_handler[SMPC_STATE_MAX] =
// {
//     /// FREE state
//     [SMPC_FREE]           = KE_STATE_HANDLER_NONE,
//     /// IDLE state
//     [SMPC_IDLE]           = KE_STATE_HANDLER_NONE,
//     /// BUSY state
//     [SMPC_BUSY]           = KE_STATE_HANDLER_NONE,
// };

// /// Message handlers that are common to all states.
// const struct ke_state_handler smpc_default_handler = KE_STATE_HANDLER(smpc_default_state);

// /// Defines the place holder for the states of all the task instances.
// ke_state_t smpc_state[SMPC_IDX_MAX] __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY

#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

#endif //(RW_BLE_USE_CRYPT)

/// @} SMPC_TASK
