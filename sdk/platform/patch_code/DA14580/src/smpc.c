/**
 ****************************************************************************************
 *
 * @file smpc.c
 *
 * @brief SMP Controller implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup SMPC
 * @ingroup SMP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "smp_common.h"

#if (RW_BLE_USE_CRYPT)

#if (BLE_CENTRAL || BLE_PERIPHERAL)

#include <string.h>

#include "ke_mem.h"
#include "ke_timer.h"           // Kernel Timer Functions

#include "gap.h"
#include "gapc.h"
#include "gapm_util.h"

#include "smpm.h"
#include "smpm_task.h"
#include "smpc.h"
#include "smpc_util.h"

#include "l2cc_task.h"

#include "co_utils.h"
#include "co_error.h"
#include "arch.h"

/*
 * PRIVATE VARIABLE DEFINITIONS
 ****************************************************************************************
 */

// /// SMPC task descriptor
// static const struct ke_task_desc TASK_DESC_SMPC = {smpc_state_handler, &smpc_default_handler,
//                                                    smpc_state, SMPC_STATE_MAX,
//                                                    SMPC_IDX_MAX};


// /// SMPC table for pairing method used for different IO capabilities, 1st idx = R, 2nd = I
// static const uint8_t smpc_pair_method[GAP_IO_CAP_LAST][GAP_IO_CAP_LAST] =
// {
//     [GAP_IO_CAP_DISPLAY_ONLY][GAP_IO_CAP_DISPLAY_ONLY]             = SMPC_METH_JW,
//     [GAP_IO_CAP_DISPLAY_ONLY][GAP_IO_CAP_DISPLAY_YES_NO]           = SMPC_METH_JW,
//     [GAP_IO_CAP_DISPLAY_ONLY][GAP_IO_CAP_KB_ONLY]                  = SMPC_METH_PK,
//     [GAP_IO_CAP_DISPLAY_ONLY][GAP_IO_CAP_NO_INPUT_NO_OUTPUT]       = SMPC_METH_JW,
//     [GAP_IO_CAP_DISPLAY_ONLY][GAP_IO_CAP_KB_DISPLAY]               = SMPC_METH_PK,

//     [GAP_IO_CAP_DISPLAY_YES_NO][GAP_IO_CAP_DISPLAY_ONLY]           = SMPC_METH_JW,
//     [GAP_IO_CAP_DISPLAY_YES_NO][GAP_IO_CAP_DISPLAY_YES_NO]         = SMPC_METH_JW,
//     [GAP_IO_CAP_DISPLAY_YES_NO][GAP_IO_CAP_KB_ONLY]                = SMPC_METH_PK,
//     [GAP_IO_CAP_DISPLAY_YES_NO][GAP_IO_CAP_NO_INPUT_NO_OUTPUT]     = SMPC_METH_JW,
//     [GAP_IO_CAP_DISPLAY_YES_NO][GAP_IO_CAP_KB_DISPLAY]             = SMPC_METH_PK,

//     [GAP_IO_CAP_KB_ONLY][GAP_IO_CAP_DISPLAY_ONLY]                  = SMPC_METH_PK,
//     [GAP_IO_CAP_KB_ONLY][GAP_IO_CAP_DISPLAY_YES_NO]                = SMPC_METH_PK,
//     [GAP_IO_CAP_KB_ONLY][GAP_IO_CAP_KB_ONLY]                       = SMPC_METH_PK,
//     [GAP_IO_CAP_KB_ONLY][GAP_IO_CAP_NO_INPUT_NO_OUTPUT]            = SMPC_METH_JW,
//     [GAP_IO_CAP_KB_ONLY][GAP_IO_CAP_KB_DISPLAY]                    = SMPC_METH_PK,

//     [GAP_IO_CAP_NO_INPUT_NO_OUTPUT][GAP_IO_CAP_DISPLAY_ONLY]       = SMPC_METH_JW,
//     [GAP_IO_CAP_NO_INPUT_NO_OUTPUT][GAP_IO_CAP_DISPLAY_YES_NO]     = SMPC_METH_JW,
//     [GAP_IO_CAP_NO_INPUT_NO_OUTPUT][GAP_IO_CAP_KB_ONLY]            = SMPC_METH_JW,
//     [GAP_IO_CAP_NO_INPUT_NO_OUTPUT][GAP_IO_CAP_NO_INPUT_NO_OUTPUT] = SMPC_METH_JW,
//     [GAP_IO_CAP_NO_INPUT_NO_OUTPUT][GAP_IO_CAP_KB_DISPLAY]         = SMPC_METH_JW,

//     [GAP_IO_CAP_KB_DISPLAY][GAP_IO_CAP_DISPLAY_ONLY]               = SMPC_METH_PK,
//     [GAP_IO_CAP_KB_DISPLAY][GAP_IO_CAP_DISPLAY_YES_NO]             = SMPC_METH_PK,
//     [GAP_IO_CAP_KB_DISPLAY][GAP_IO_CAP_KB_ONLY]                    = SMPC_METH_PK,
//     [GAP_IO_CAP_KB_DISPLAY][GAP_IO_CAP_NO_INPUT_NO_OUTPUT]         = SMPC_METH_JW,
//     [GAP_IO_CAP_KB_DISPLAY][GAP_IO_CAP_KB_DISPLAY]                 = SMPC_METH_PK,
// };

// /*
//  * GLOBAL VARIABLE DEFINITIONS
//  ****************************************************************************************
//  */

// /// SMPC tasks
// struct smpc_env_tag *smpc_env[SMPC_IDX_MAX]  __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY

// /*
//  * FUNCTION DEFINITIONS
//  ****************************************************************************************
//  */
// /* FUNCTION ADDED BY WIK, 
//  * Saving here the smpc vars during power down state
//  * Note: used only by smpc_regs_push() / smpc_regs_pop()
//  * The reason for not putting smpc_env_tag completely in 
//  * retention memory is because the size is 484 (1 connection)
//  * and we need only to store 43 bytes
//  */
// struct s_smpc_reg_save {
//    
//     uint16_t conhdl;    
//     ke_task_id_t srcid;             //2 bytes  
//     uint8_t role;
//     uint8_t secmode;
//     uint8_t bonded;
//     uint8_t enc_enabled;
//     uint16_t rep_attempts_intv;
//     struct bd_addr laddr;           //6 bytes
//     uint8_t laddr_type;    
//     struct bd_addr raddr;           //6 bytes 
//     uint8_t raddr_type;
// //    struct smp_pair_pdu pair_req;   //6 bytes
// //    struct smp_pair_pdu pair_rsp;   //6 bytes 
//     uint8_t key_sec_prop;
//     struct bd_addr res_addr;        //6 bytes
// }; 

// //@WIK, removed next line because this was only for ES1
// //volatile struct s_smpc_reg_save  smpc_reg_save[BLE_CONNECTION_MAX] __attribute__((section("exchange_mem_case1")));


// /*********************************************************************************
//  *** smpc_regs_push()
//  ***/
// void smpc_regs_push(void)
// {
// //vm temporary
// #if 0	
//     for(uint8 i=0;i< jump_table_struct[nb_links_user];i++)
//     {   
//         smpc_reg_save[i].conhdl             = smpc_env[i]->conhdl;
//         smpc_reg_save[i].srcid              = smpc_env[i].srcid;
// //        smpc_reg_save[i].role               = smpc_env[i].role;
//         smpc_reg_save[i].secmode            = smpc_env[i].secmode;
//         smpc_reg_save[i].bonded             = smpc_env[i].bonded;
//         smpc_reg_save[i].enc_enabled        = smpc_env[i].enc_enabled;
//         smpc_reg_save[i].rep_attempts_intv  = smpc_env[i].rep_attempts_intv;
// //        smpc_reg_save[i].laddr_type         = smpc_env[i].laddr_type;
// //        smpc_reg_save[i].raddr_type         = smpc_env[i].raddr_type;
//         smpc_reg_save[i].key_sec_prop       = smpc_env[i].key_sec_prop;

// //        memcpy((void*)&smpc_reg_save[i].laddr,    (const void*)&smpc_env[i].laddr,    6);
// //        memcpy((void*)&smpc_reg_save[i].raddr,    (const void*)&smpc_env[i].raddr,    6);
//         memcpy((void*)&smpc_reg_save[i].res_addr, (const void*)&smpc_env[i].res_addr, 6);
//         memcpy((void*)&smpc_reg_save[i].pair_req, (const void*)&smpc_env_lpus[i].pair_req, 6);
//         memcpy((void*)&smpc_reg_save[i].pair_rsp, (const void*)&smpc_env_lpus[i].pair_rsp, 6);  
//     }
// #endif		
// }

// /*********************************************************************************
//  *** smpc_regs_pop()
//  *** Caution: make it inline to avoid usage of the stuck (i.e. the possibly corrupted SysRAM)!
//  ***/
// void smpc_regs_pop(void)
// {
// //vm temporary
// #if 0	

//     for(uint8 i=0;i< jump_table_struct[nb_links_user];i++)
//     {
//         smpc_env[i].conhdl                  = smpc_reg_save[i].conhdl;
//         smpc_env[i].srcid                   = smpc_reg_save[i].srcid;
// //        smpc_env[i].role                    = smpc_reg_save[i].role;
//         smpc_env[i].secmode                 = smpc_reg_save[i].secmode;
//         smpc_env[i].bonded                  = smpc_reg_save[i].bonded;
//         smpc_env[i].enc_enabled             = smpc_reg_save[i].enc_enabled;
//         smpc_env[i].rep_attempts_intv       = smpc_reg_save[i].rep_attempts_intv;
// //        smpc_env[i].laddr_type              = smpc_reg_save[i].laddr_type;
// //        smpc_env[i].raddr_type              = smpc_reg_save[i].raddr_type;
//         smpc_env[i].key_sec_prop            = smpc_reg_save[i].key_sec_prop;

// //        memcpy((void*)&smpc_env[i].laddr,    (const void*)&smpc_reg_save[i].laddr,    6);
// //        memcpy((void*)&smpc_env[i].raddr,    (const void*)&smpc_reg_save[i].raddr,    6);
//         memcpy((void*)&smpc_env[i].res_addr, (const void*)&smpc_reg_save[i].res_addr, 6);
//         memcpy((void*)&smpc_env_lpus[i].pair_req, (const void*)&smpc_reg_save[i].pair_req, 6);
//         memcpy((void*)&smpc_env_lpus[i].pair_rsp, (const void*)&smpc_reg_save[i].pair_rsp, 6);   
//     }
// #endif		
// }



// void smpc_init(bool reset)
// {
//     // Index
//     uint8_t conidx;

//     if(!reset)
//     {
//         // Create SMPC task
//         ke_task_create(TASK_SMPC, &TASK_DESC_SMPC);

//         // Initialize SMPC environment variable
//         for (conidx = 0; conidx < jump_table_struct[nb_links_user]; conidx++)
//         {
//             smpc_env[conidx] = NULL;
//         }
//     }

//     for (conidx = 0; conidx < jump_table_struct[nb_links_user]; conidx++)
//     {
//         // Clean states
//         smpc_cleanup(conidx);
//     }
// }

// void smpc_create(uint8_t conidx)
// {
//     // Allocate environment variable for connection
//     smpc_env[conidx] = (struct smpc_env_tag *)ke_malloc(sizeof(struct smpc_env_tag), KE_MEM_ENV);

//     ASSERT_ERR(smpc_env[conidx] != NULL);

//     // Clean-up structure
//     memset(smpc_env[conidx], 0, sizeof(struct smpc_env_tag));

// 	// Set the default value of the Repeated Attempt timer
//     smpc_env[conidx]->rep_att_timer_val = SMPC_REP_ATTEMPTS_TIMER_DEF_VAL;
//     // Put current task in connection ready state.
//     ke_state_set(KE_BUILD_ID(TASK_SMPC, conidx), SMPC_IDLE);
// }

// void smpc_cleanup(uint8_t conidx)
// {
//     // Cleanup environment variable for connection
//     if(smpc_env[conidx] != NULL)
//     {
//         if (smpc_env[conidx]->operation != NULL)
//         {
//             ke_free(ke_param2msg(smpc_env[conidx]->operation));
//         }
//         // Release both information structures if needed.
//         if (smpc_env[conidx]->pair_info != NULL)
//         {
//             ke_free(smpc_env[conidx]->pair_info);
//         }

//         if (smpc_env[conidx]->sign_info != NULL)
//         {
//             ke_free(smpc_env[conidx]->sign_info);
//         }

//         // Clear the timeout timer
//         smpc_clear_timeout_timer(conidx);
//         // Clear the repeated attempt timer
//         if (SMPC_IS_FLAG_SET(conidx, SMPC_TIMER_REP_ATT_FLAG))
//         {
//             ke_timer_clear(SMPC_REP_ATTEMPTS_TIMER_IND,
//                            KE_BUILD_ID(TASK_SMPC, conidx));
//         }
//         ke_free(smpc_env[conidx]);
//         smpc_env[conidx] = NULL;
//     }

//     // Set state to free state
//     ke_state_set(KE_BUILD_ID(TASK_SMPC, conidx), SMPC_FREE);
// }

// void smpc_init_operation(uint8_t conidx, void* operation)
// {
//     // Store operation
//     smpc_env[conidx]->operation = operation;

//     // Set state to busy
//     ke_state_set(KE_BUILD_ID(TASK_SMPC, conidx), SMPC_BUSY);
// }

// void smpc_send_cmp_evt(uint8_t conidx, ke_task_id_t cmd_src_id, uint8_t operation, uint8_t status)
// {
//     ke_task_id_t smpc_task_id = KE_BUILD_ID(TASK_SMPC, conidx);

//     // Release the command message
//     if (smpc_env[conidx]->operation != NULL)
//     {
//         ke_msg_free(ke_param2msg(smpc_env[conidx]->operation));
//         smpc_env[conidx]->operation = NULL;
//     }

//     // Release the pairing information structure
//     if (smpc_env[conidx]->pair_info != NULL)
//     {
//         ke_free(smpc_env[conidx]->pair_info);
//         smpc_env[conidx]->pair_info = NULL;
//     }

//     // Release the signature information structure
//     if (smpc_env[conidx]->sign_info != NULL)
//     {
//         ke_free(smpc_env[conidx]->sign_info);
//         smpc_env[conidx]->sign_info = NULL;
//     }

//     // Stop the timeout timer if needed
//     smpc_clear_timeout_timer(conidx);

//     // Reset the internal state
//     smpc_env[conidx]->state = SMPC_STATE_RESERVED;

//     // Come back to IDLE state
//     ke_state_set(smpc_task_id, SMPC_IDLE);

//     struct smpc_cmp_evt *cmp_evt = KE_MSG_ALLOC(SMPC_CMP_EVT,
//                                                 cmd_src_id, smpc_task_id,
//                                                 smpc_cmp_evt);

//     cmp_evt->operation = operation;
//     cmp_evt->status    = status;

//     ke_msg_send(cmp_evt);
// }

// void smpc_send_use_enc_block_cmd(uint8_t conidx, uint8_t *operand_1, uint8_t *operand_2)
// {
//     // Send an encryption request to the SMPM task
//     struct gapm_use_enc_block_cmd *cmd = KE_MSG_ALLOC(SMPM_USE_ENC_BLOCK_CMD,
//                                                       TASK_SMPM, KE_BUILD_ID(TASK_SMPC, conidx),
//                                                       gapm_use_enc_block_cmd);

//     cmd->operation = SMPM_OP_USE_ENC_BLOCK;
//     // Set operand_1 value
//     memcpy(&cmd->operand_1[0], operand_1, KEY_LEN);
//     // Set operand_2 value
//     memcpy(&cmd->operand_2[0], operand_2, KEY_LEN);

//     ke_msg_send(cmd);
// }

// void smpc_send_start_enc_cmd(uint8_t idx, uint8_t key_type, uint8_t *key, uint8_t *randnb, uint16_t ediv)
// {
//     struct llc_le_start_enc_cmd *cmd = KE_MSG_ALLOC(LLC_LE_START_ENC_CMD,
//                                                     EMB_LLC_TASK, KE_BUILD_ID(TASK_SMPC, idx),
//                                                     llc_le_start_enc_cmd);

//     cmd->conhdl = gapc_get_conhdl(idx);

//     if (key_type == SMPC_USE_STK)
//     {
//         // Set EDIV value to 0
//         cmd->enc_div = 0;
//         // Set Random Number value to 0
//         memset(&cmd->nb.nb[0], 0x00, RAND_NB_LEN);
//     }
//     else    // SMPC_USE_LTK
//     {
//         // Set EDIV value
//         cmd->enc_div = ediv;
//         // Set Random Number
//         memcpy(&cmd->nb.nb[0], randnb, RAND_NB_LEN);
//     }

//     // Copy the key
//     memcpy(&cmd->ltk.ltk[0], key, KEY_LEN);

//     ke_msg_send(cmd);
//     // Set state to encrypting
//     ke_state_set(KE_BUILD_ID(TASK_SMPC, idx), SMPC_ENCRYPT);
// }

// void smpc_send_ltk_req_rsp(uint8_t idx, bool found, uint8_t *key)
// {
//     uint16_t conhdl = gapc_get_conhdl(idx);

//     if (found)
//     {
//         // Reply that the encryption key has been found
//         struct llc_le_ltk_req_rply_cmd *cmd = KE_MSG_ALLOC(LLC_LE_LTK_REQ_RPLY_CMD,
//                                                            EMB_LLC_TASK, KE_BUILD_ID(TASK_SMPC, idx),
//                                                            llc_le_ltk_req_rply_cmd);

//         cmd->conhdl = conhdl;

//         // Copy the found key
//         memcpy(&cmd->ltk.ltk[0], key, KEY_LEN);

//         ke_msg_send(cmd);
//         // Set state to encrypting
//         ke_state_set(KE_BUILD_ID(TASK_SMPC, idx), SMPC_ENCRYPT);
//     }
//     else
//     {
//         // Reply that the encryption key has not been found
//         struct llc_le_ltk_req_neg_rply_cmd *neg_cmd = KE_MSG_ALLOC(LLC_LE_LTK_REQ_NEG_RPLY_CMD,
//                                                                    EMB_LLC_TASK, KE_BUILD_ID(TASK_SMPC, idx),
//                                                                    llc_le_ltk_req_neg_rply_cmd);

//         neg_cmd->conhdl = conhdl;

//         ke_msg_send(neg_cmd);
//     }
// }

void smpc_send_pairing_req_ind(uint8_t conidx, uint8_t req_type)
{
    struct smpc_pairing_req_ind *req_ind = KE_MSG_ALLOC(SMPC_PAIRING_REQ_IND,
                                                        KE_BUILD_ID(TASK_GAPC, conidx), KE_BUILD_ID(TASK_SMPC, conidx),
                                                        smpc_pairing_req_ind);

    req_ind->request = req_type;

    switch (req_type)
    {
        case (GAPC_PAIRING_REQ):
        {
             req_ind->data.auth_req = (smpc_env[conidx]->pair_info->pair_req_feat.auth) & GAP_AUTH_REQ_MITM_BOND;

            // Update the internal state
            smpc_env[conidx]->state = SMPC_PAIRING_FEAT_WAIT;
        } break;

        case (GAPC_LTK_EXCH):
        {
            // Provide the key size
            req_ind->data.key_size = gapc_get_enc_keysize(conidx);
        } break;

        case (GAPC_CSRK_EXCH):
        {
            // Update the internal state
            smpc_env[conidx]->state = SMPC_PAIRING_CSRK_WAIT;
        } break;

        case (GAPC_TK_EXCH):
        {
            ASSERT_ERR(smpc_env[conidx]->pair_info->pair_method != SMPC_METH_JW);

            // Both have OOB -> The length of the TK shall be 16 bytes
            if ((smpc_env[conidx]->pair_info->pair_req_feat.oob == GAP_OOB_AUTH_DATA_PRESENT) &&
                (smpc_env[conidx]->pair_info->pair_rsp_feat.oob == GAP_OOB_AUTH_DATA_PRESENT))
            {

                req_ind->data.tk_type = GAP_TK_OOB;
            }
            else
            {
                /*
                 * If we are here, the method is Passkey Entry
                 * Specification Vol 3, Part H, 2.3.5.1: Selecting STK Generation Method
                 */
                if (gapc_get_role(conidx) == ROLE_MASTER)
                {
                    if (((smpc_env[conidx]->pair_info->pair_rsp_feat.iocap == GAP_IO_CAP_KB_ONLY) ||
                         (smpc_env[conidx]->pair_info->pair_rsp_feat.iocap == GAP_IO_CAP_KB_DISPLAY)) &&
                        (smpc_env[conidx]->pair_info->pair_req_feat.iocap != GAP_IO_CAP_KB_ONLY))
                    {
                        // The application shall display the PIN Code
                        req_ind->data.tk_type = GAP_TK_DISPLAY;
                    }
                    else
                    {
                        // The use shall enter the key using the keyboard
                        req_ind->data.tk_type = GAP_TK_KEY_ENTRY;
                    }
                }
                else    // role == ROLE_SLAVE
                {
                    if ((smpc_env[conidx]->pair_info->pair_rsp_feat.iocap == GAP_IO_CAP_DISPLAY_ONLY)   ||
                        (smpc_env[conidx]->pair_info->pair_rsp_feat.iocap == GAP_IO_CAP_DISPLAY_YES_NO) ||
                        ((smpc_env[conidx]->pair_info->pair_rsp_feat.iocap == GAP_IO_CAP_KB_DISPLAY) &&
                         (smpc_env[conidx]->pair_info->pair_req_feat.iocap == GAP_IO_CAP_KB_ONLY) ))
                    {
                        // The application shall display the PIN Code
                        req_ind->data.tk_type = GAP_TK_DISPLAY;
                    }
                    else
                    {
                        // The use shall enter the key using the keyboard
                        req_ind->data.tk_type = GAP_TK_KEY_ENTRY;
                    }
                }
            }

            smpc_env[conidx]->state = SMPC_PAIRING_TK_WAIT;
        } break;

        default:
        {
            ASSERT_ERR(0);
        } break;
    }

    ke_msg_send(req_ind);
}

// void smpc_send_pairing_ind(uint8_t conidx, uint8_t ind_type, void *value)
// {
//     struct smpc_pairing_ind *ind = KE_MSG_ALLOC(SMPC_PAIRING_IND,
//                                                 KE_BUILD_ID(TASK_GAPC, conidx), KE_BUILD_ID(TASK_SMPC, conidx),
//                                                 smpc_pairing_ind);

//     ind->info = ind_type;

//     switch (ind_type)
//     {
//         case (GAPC_PAIRING_FAILED):
//         {
//             ind->data.reason = *(uint8_t *)value;
//         } break;

//         case (GAPC_PAIRING_SUCCEED):
//         {
//             ind->data.auth = smpc_env[conidx]->pair_info->auth;
//             // update link authorization level
//             gapc_auth_set(conidx, ind->data.auth);
//             // informs that link is now encrypted
//             gapc_link_encrypted(conidx);
//         } break;

//         case (GAPC_LTK_EXCH):
//         {
//             memcpy(&ind->data.ltk, value, sizeof(struct gapc_ltk));
//         } break;

//         case (GAPC_IRK_EXCH):
//         {
//             memcpy(&ind->data.irk, value, sizeof(struct gapc_irk));
//         } break;

//         case (GAPC_CSRK_EXCH):
//         {
//             memcpy(&ind->data.csrk.key[0], value, KEY_LEN);
//         } break;

//         default:
//         {
//             ASSERT_ERR(0);
//         } break;
//     }

//     ke_msg_send(ind);
// }

bool smpc_check_pairing_feat(struct gapc_pairing *pair_feat)
{
    // Returned status
    bool status = true;

    // Check IO Capabilities value
    if (pair_feat->iocap > GAP_IO_CAP_KB_DISPLAY)
    {
        status = false;
    }
    // Check Out Of Band status
    else if (((pair_feat->oob) != GAP_OOB_AUTH_DATA_NOT_PRESENT) &&
             ((pair_feat->oob) != GAP_OOB_AUTH_DATA_PRESENT) )
    {
        status = false;
    }
    // Check Key Distribution
    else if ((pair_feat->ikey_dist > GAP_KDIST_LAST) ||
             (pair_feat->rkey_dist > GAP_KDIST_LAST))
    {
        status = false;
    }

    return status;
}

// uint8_t smpc_check_repeated_attempts(uint8_t conidx)
// {
//     // Returned status
//     uint8_t status = SMPC_REP_ATTEMPTS_NO_ERROR;

//     if (SMPC_IS_FLAG_SET(conidx, SMPC_TIMER_REP_ATT_FLAG))
//     {
//         // Check if an attack has already been detected
//         if (smpc_env[conidx]->rep_att_timer_val != SMPC_REP_ATTEMPTS_TIMER_MAX_VAL)
//         {
//             // The timer value shall be increased exponentially if a repeated attempt occurs
//             smpc_env[conidx]->rep_att_timer_val *= SMPC_REP_ATTEMPTS_TIMER_MULT;

//             // Check if the timer value is upper than the max limit
//             if (smpc_env[conidx]->rep_att_timer_val >= SMPC_REP_ATTEMPTS_TIMER_MAX_VAL)
//             {
//                 smpc_env[conidx]->rep_att_timer_val = SMPC_REP_ATTEMPTS_TIMER_MAX_VAL;

//                 // Inform the HL that an attack by repeated attempts has been detected
//                 ke_msg_send_basic(SMPC_REP_ATTEMPTS_ATTACK_IND,
//                                   KE_BUILD_ID(TASK_GAPC, conidx), KE_BUILD_ID(TASK_SMPC, conidx));
//             }

//             status = SMPC_REP_ATTEMPT;
//         }
//         else
//         {
//             // New attack attempt, the pairing request PDU will be dropped
//             status = SMPC_REP_ATTEMPTS_ATTACK;
//         }
//         // Restart the timer
//         smpc_launch_rep_att_timer(conidx);
//     }
//     // else status is SMP_ERROR_NO_ERROR

//     return status;
// }

// bool smpc_check_max_key_size(uint8_t conidx)
// {
//     // Returned status
//     bool status = false;
//     // Negociated key size (min value between both sent key sizes)
//     uint8_t size;

//     // The lower size shall be kept as key size.
//     if (smpc_env[conidx]->pair_info->pair_req_feat.key_size
//                 < smpc_env[conidx]->pair_info->pair_rsp_feat.key_size)
//     {
//         size = smpc_env[conidx]->pair_info->pair_req_feat.key_size;
//     }
//     else
//     {
//         size = smpc_env[conidx]->pair_info->pair_rsp_feat.key_size;
//     }

//     // If the key size is below the negociated size, reject it
//     if (size >= SMPC_MIN_ENC_SIZE_LEN)
//     {
//         status = true;

//         gapc_set_enc_keysize(conidx, size);
//     }

//     return status;
// }

// bool smpc_check_key_distrib(uint8_t conidx, uint8_t sec_level)
// {
//     // Returned status
//     bool status = true;
//     // Keys distributed by the initiator
//     uint8_t i_keys    = smpc_env[conidx]->pair_info->pair_req_feat.ikey_dist
//                             & smpc_env[conidx]->pair_info->pair_rsp_feat.ikey_dist;
//     // Keys distributed by the responder
//     uint8_t r_keys    = smpc_env[conidx]->pair_info->pair_req_feat.rkey_dist
//                             & smpc_env[conidx]->pair_info->pair_rsp_feat.rkey_dist;

//     // If both device are bondable check that at least one key is distributed
//     if (((smpc_env[conidx]->pair_info->pair_req_feat.auth & GAP_AUTH_BOND) == GAP_AUTH_BOND) &&
//         ((smpc_env[conidx]->pair_info->pair_rsp_feat.auth & GAP_AUTH_BOND) == GAP_AUTH_BOND))
//     {
//         if ((i_keys == GAP_KDIST_NONE) && (r_keys == GAP_KDIST_NONE))
//         {
//             status = false;
//         }
//     }

//     // If a security mode 1 is required, check if a LTK is distributed
//     if ((sec_level == GAP_SEC1_NOAUTH_PAIR_ENC) || (sec_level == GAP_SEC1_AUTH_PAIR_ENC))
//     {
//         if (!(((i_keys & GAP_KDIST_ENCKEY) == GAP_KDIST_ENCKEY) ||
//               ((r_keys & GAP_KDIST_ENCKEY) == GAP_KDIST_ENCKEY)))
//         {
//             status = false;
//         }
//     }

//     // If a security mode 2 is required, check if a CSRK is distributed
//     if ((sec_level == GAP_SEC2_NOAUTH_DATA_SGN) || (sec_level == GAP_SEC2_AUTH_DATA_SGN))
//     {
//         if (!(((i_keys & GAP_KDIST_SIGNKEY) == GAP_KDIST_SIGNKEY) ||
//               ((r_keys & GAP_KDIST_SIGNKEY) == GAP_KDIST_SIGNKEY)))
//         {
//             status = false;
//         }
//     }

//     return status;
// }

// void smpc_xor(uint8_t *result, uint8_t *operand_1, uint8_t *operand_2)
// {
//     // Counter
//     uint8_t counter;

//     // result = operand_1 XOR operand_2
//     for (counter = 0; counter < KEY_LEN; counter++)
//     {
//         *(result + counter) = (*(operand_1 + counter))^(*(operand_2 + counter));
//     }
// }

// void smpc_generate_l(uint8_t conidx, uint8_t src)
// {
//     /**
//      * L = AES_128(CSRK, 0[0:127])
//      */

//     struct gap_sec_key csrk;

//     // Get the CSRK in the GAP
//     memcpy(&csrk.key[0], gapc_get_csrk(conidx, src), KEY_LEN);

//     // Set the current state of the procedure
//     smpc_env[conidx]->state = SMPC_SIGN_L_GEN;

//     // Send an encryption request to the SMPM task
//     struct gapm_use_enc_block_cmd *cmd = KE_MSG_ALLOC(SMPM_USE_ENC_BLOCK_CMD,
//                                                       TASK_SMPM, KE_BUILD_ID(TASK_SMPC, conidx),
//                                                       gapm_use_enc_block_cmd);

//     cmd->operation = SMPM_OP_USE_ENC_BLOCK;
//     // operand_1 is the CSRK
//     memcpy(&cmd->operand_1[0], &csrk.key[0], KEY_LEN);
//     // Set operand_2 value
//     memset(&cmd->operand_2[0], 0x00, KEY_LEN);

//     ke_msg_send(cmd);
// }

// void smpc_generate_ci(uint8_t conidx, uint8_t src, uint8_t *ci1, uint8_t *mi)
// {
//     /**
//      * Ci = AES_128(CSRK, Ci-1 XOR Mi)
//      */

//     // CSRK
//     struct gap_sec_key csrk;
//     // Signature Information
//     struct smpc_sign_info *sign_info = ((struct smpc_sign_info *)(smpc_env[conidx]->sign_info));

//     // Get the CSRK in the GAP
//     memcpy(&csrk.key[0], gapc_get_csrk(conidx, src), KEY_LEN);

//     // Set the current state of the procedure
//     smpc_env[conidx]->state = SMPC_SIGN_Ci_GEN;

//     struct gapm_use_enc_block_cmd *cmd = KE_MSG_ALLOC(SMPM_USE_ENC_BLOCK_CMD,
//                                                       TASK_SMPM, KE_BUILD_ID(TASK_SMPC, conidx),
//                                                       gapm_use_enc_block_cmd);

//     cmd->operation = SMPM_OP_USE_ENC_BLOCK;
//     // operand_1 is the CSRK
//     memcpy(&cmd->operand_1[0], &csrk.key[0], KEY_LEN);
//     // Set operand_2 value = Ci-1 XOR Mi
//     smpc_xor(&cmd->operand_2[0], ci1, mi);

//     ke_msg_send(cmd);

//     // Update number of block to analyze
//     sign_info->block_nb--;
//     // Update message offset
//     if (sign_info->msg_offset < KEY_LEN)
//     {
//         sign_info->msg_offset = 0;
//     }
//     else
//     {
//         sign_info->msg_offset -= KEY_LEN;
//     }
// }

// void smpc_generate_rand(uint8_t conidx, uint8_t state)
// {
//     smpc_env[conidx]->state = state;

//     struct gapm_gen_rand_nb_cmd *cmd = KE_MSG_ALLOC(SMPM_GEN_RAND_NB_CMD,
//                                                     TASK_SMPM, KE_BUILD_ID(TASK_SMPC, conidx),
//                                                     gapm_gen_rand_nb_cmd);

//     cmd->operation = SMPM_OP_GEN_RAND_NB;

//     ke_msg_send(cmd);
// }

// void smpc_generate_e1(uint8_t conidx, uint8_t role, bool local)
// {
//     /**
//      * Calculation of e1 is the first step of the confirm value generation
//      *  e1 = AES_128(RAND, p1)  with:
//      *      p1 = PRES || PREQ || 0 || RAT || 0 || IAT
//      *          PRES = Pairing Response Command
//      *          PREQ = Pairing Request Command
//      *          RAT  = Responding device address type
//      *          IAT  = Initiating Device Address Type
//      */

//     // P1 value (LSB->MSB)
//     uint8_t p1[KEY_LEN];
//     // Offset
//     uint8_t offset = 0;

//     memset(&p1[0], 0x00, KEY_LEN);

//     /*
//      * Initiating Device Address Type
//      * Responding Device Address Type
//      */
//     if (role == ROLE_MASTER)
//     {
//         p1[offset]     = gapc_get_bdaddr(conidx, GAPC_INFO_LOCAL)->addr_type;
//         p1[offset + 1] = gapc_get_bdaddr(conidx, GAPC_INFO_PEER)->addr_type;
//     }
//     else    // role == ROLE_SLAVE
//     {
//         p1[offset]     = gapc_get_bdaddr(conidx, GAPC_INFO_PEER)->addr_type;
//         p1[offset + 1] = gapc_get_bdaddr(conidx, GAPC_INFO_LOCAL)->addr_type;
//     }

//     offset += 2;

//     /*
//      * Pairing Request Command
//      */
//     p1[offset] = L2C_CODE_PAIRING_REQUEST;
//     offset++;
//     memcpy(&p1[offset], &smpc_env[conidx]->pair_info->pair_req_feat, SMPC_CODE_PAIRING_REQ_RESP_LEN - 1);
//     offset += (SMPC_CODE_PAIRING_REQ_RESP_LEN - 1);

//     /*
//      * Pairing Response Command
//      */
//     p1[offset] = L2C_CODE_PAIRING_RESPONSE;
//     offset++;
//     memcpy(&p1[offset], &smpc_env[conidx]->pair_info->pair_rsp_feat, SMPC_CODE_PAIRING_REQ_RESP_LEN - 1);

//     struct gapm_use_enc_block_cmd *cmd = KE_MSG_ALLOC(SMPM_USE_ENC_BLOCK_CMD,
//                                                       TASK_SMPM, KE_BUILD_ID(TASK_SMPC, conidx),
//                                                       gapm_use_enc_block_cmd);

//     cmd->operation = SMPM_OP_USE_ENC_BLOCK;

//     // Operand_1 is the TK
//     memcpy(&cmd->operand_1[0], &smpc_env[conidx]->pair_info->key.key[0], KEY_LEN);

//     // The used random value depends on the confirm value type (Rand values stored LSB->MSB)
//     if (local)
//     {
//         // Set operand_2 value = R XOR P1
//         smpc_xor(&cmd->operand_2[0], &smpc_env[conidx]->pair_info->rand[0], &p1[0]);
//     }
//     else    // Remote
//     {
//         // Set operand_2 value = R XOR P1
//         smpc_xor(&cmd->operand_2[0], &smpc_env[conidx]->pair_info->rem_rand[0], &p1[0]);
//     }


//     ke_msg_send(cmd);
// }

// void smpc_generate_cfm(uint8_t conidx, uint8_t role, uint8_t *e1)
// {
//     /**
//      *  cfm = AES_128(e1, p2)  with:
//      *      p2 = 0[0:4] || IA || RA
//      *          RA  = Responding device address
//      *          IA  = Initiating Device address
//      */

//     // P2
//     uint8_t p2[CFM_LEN];

//     memset(&p2[0], 0x00, KEY_LEN);

//     /*
//      * Responding Device Address
//      * Initiating Device Address+
//      */
//     if (role == ROLE_MASTER)
//     {
//         memcpy(&p2[0], &(gapc_get_bdaddr(conidx, GAPC_INFO_PEER)->addr.addr), BD_ADDR_LEN);
//         memcpy(&p2[BD_ADDR_LEN], &(gapc_get_bdaddr(conidx, GAPC_INFO_LOCAL)->addr.addr), BD_ADDR_LEN);
//     }
//     else    // role == ROLE_SLAVE
//     {
//         memcpy(&p2[0], &(gapc_get_bdaddr(conidx, GAPC_INFO_LOCAL)->addr.addr), BD_ADDR_LEN);
//         memcpy(&p2[BD_ADDR_LEN], &(gapc_get_bdaddr(conidx, GAPC_INFO_PEER)->addr.addr), BD_ADDR_LEN);
//     }

//     struct gapm_use_enc_block_cmd *cmd = KE_MSG_ALLOC(SMPM_USE_ENC_BLOCK_CMD,
//                                                       TASK_SMPM, KE_BUILD_ID(TASK_SMPC, conidx),
//                                                       gapm_use_enc_block_cmd);

//     cmd->operation = SMPM_OP_USE_ENC_BLOCK;

//     // Operand_1 is the TK
//     memcpy(&cmd->operand_1[0], &smpc_env[conidx]->pair_info->key.key[0], KEY_LEN);

//     // Set operand_2 value = E1 XOR P2
//     smpc_xor(&cmd->operand_2[0], e1, &p2[0]);


//     ke_msg_send(cmd);
// }

// void smpc_generate_stk(uint8_t conidx, uint8_t role)
// {
//     /**
//      * ********************************************
//      * CALCULATE STK
//      *      STK = AES_128(TK, r) with:
//      *            r = LSB64(Srand) || LSB64(Mrand)
//      * ********************************************
//      */

//     uint8_t r[KEY_LEN];

//         if (role == ROLE_MASTER)
//         {
//         memcpy(&(r[0]), &(smpc_env[conidx]->pair_info->rand[0]), (RAND_VAL_LEN/2));
//         memcpy(&(r[RAND_VAL_LEN/2]), &(smpc_env[conidx]->pair_info->rem_rand[0]), (RAND_VAL_LEN/2));
//         }
//         else    // role == ROLE_SLAVE
//         {
//         memcpy(&(r[0]), &(smpc_env[conidx]->pair_info->rem_rand[0]), (RAND_VAL_LEN/2));
//         memcpy(&(r[RAND_VAL_LEN/2]), &(smpc_env[conidx]->pair_info->rand[0]), (RAND_VAL_LEN/2));
//     }

//     // Set the state
//     smpc_env[conidx]->state = SMPC_PAIRING_GEN_STK;

//     // Call the AES 128 block
//     smpc_send_use_enc_block_cmd(conidx, &smpc_env[conidx]->pair_info->key.key[0], &r[0]);
// }

// void smpc_calc_subkeys(bool gen_k2, uint8_t *l_value, uint8_t *subkey)
// {
//     // NOTE: All arrays have format: [0:15] = LSB to MSB

//     // Counter
//     uint8_t counter;
//     // MSB
//     uint8_t msb;

//     // Rb = 0*120 1000 0111
//     uint8_t rb[16] = {0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//     /**
//      * -------------------------------------------------------------------
//      * K1 CALCULATION
//      * If MSB(L) = 0, K1 = (L << 1), else K1 = (L << 1) XOR Rb
//      * -------------------------------------------------------------------
//      */

//     // L<<1
//     for (counter = (KEY_LEN - 1); counter >= 1; counter--)
//     {
//         *(subkey + counter) = (*(l_value + counter) << 1) | (*(l_value + counter - 1) >> 7);
//     }

//     // Last one
//     *(subkey) = (*(l_value) << 1);

//     // Check MSBit(L) value
//     if ((*(l_value +  KEY_LEN - 1) & 0x80) == 0x80)
//     {
//         smpc_xor(subkey, subkey, &rb[0]);
//     }

//     if (gen_k2)
//     {
//         /**
//          * -------------------------------------------------------------------
//          * K2 CALCULATION
//          * If MSB(K2) = 0, K2 = (K1 << 1), else K2 = (K1 << 1) XOR Rb
//          * -------------------------------------------------------------------
//          */
//         msb = *(subkey +  KEY_LEN - 1);

//         // K1<<1
//         for (counter = (KEY_LEN - 1); counter >= 1; counter--)
//         {
//             *(subkey + counter) = (*(subkey + counter) << 1) | (*(subkey + counter - 1) >> 7);
//         }

//         // Last one
//         *(subkey) = (*(subkey) << 1);

//         // Check MSB(K1) value
//         if ((msb & 0x80) == 0x80)
//         {
//             smpc_xor(subkey, subkey, &rb[0]);
//         }
//     }
// }

// void smpc_tkdp_send_start(uint8_t conidx, uint8_t role)
// {
//     // Use generic variable for the master or slave key distribution
//     uint8_t keys;
//     // Indicate if the section is over
//     bool send_over = true;

//     if (role == ROLE_MASTER)
//     {
//         keys = smpc_env[conidx]->pair_info->pair_req_feat.ikey_dist
//                         & smpc_env[conidx]->pair_info->pair_rsp_feat.ikey_dist;
//     }
//     else
//     {
//         keys = smpc_env[conidx]->pair_info->pair_req_feat.rkey_dist
//                         & smpc_env[conidx]->pair_info->pair_rsp_feat.rkey_dist;
//     }

//     if ((keys & GAP_KDIST_ENCKEY) == GAP_KDIST_ENCKEY)
//     {
//         // LTK need to be retrieved from the HL
//         smpc_send_pairing_req_ind(conidx, GAPC_LTK_EXCH);

//         // Update the internal state
//         smpc_env[conidx]->state = SMPC_PAIRING_LTK_WAIT;

//         send_over = false;
//     }
//     else
//     {
//         // IRK will be distributed
//         if ((keys & GAP_KDIST_IDKEY) == GAP_KDIST_IDKEY)
//         {
//             // Send the IRK to the peer device
//             smpc_pdu_send(conidx, L2C_CODE_IDENTITY_INFORMATION, (void *)gapm_get_irk());
//             // Send the BD Address to the peer device
//             smpc_pdu_send(conidx, L2C_CODE_IDENTITY_ADDRESS_INFORMATION, (void *)gapm_get_bdaddr());
//         }

//         // CSRK will be distributed
//         if ((keys & GAP_KDIST_SIGNKEY) == GAP_KDIST_SIGNKEY)
//         {
//             send_over = false;

//             // Ask for the CSRK
//             smpc_send_pairing_req_ind(conidx, GAPC_CSRK_EXCH);
//         }
//     }

//     if (send_over)
//     {
//         if (role == ROLE_MASTER)
//         {
//             // Pairing is over
//             smpc_pairing_end(conidx, role, SMP_ERROR_NO_ERROR, false);
//         }
//         else    // role == ROLE_SLAVE
//         {
//             // Master will now send its keys (if needed)
//             smpc_tkdp_rcp_start(conidx, role);
//         }
//     }
// }

// void smpc_tkdp_send_continue(uint8_t conidx, uint8_t role, uint8_t *ltk, struct smpc_mst_id_info *mst_id_info)
// {
//     ASSERT_ERR(smpc_env[conidx]->state == SMPC_PAIRING_LTK_WAIT);

//     // Keys distribution
//     uint8_t keys;

//     // If we are here the LTK has been received from the HL
//     if (role == ROLE_MASTER)
//     {
//         keys = smpc_env[conidx]->pair_info->pair_req_feat.ikey_dist
//                         & smpc_env[conidx]->pair_info->pair_rsp_feat.ikey_dist;
//     }
//     else
//     {
//         keys = smpc_env[conidx]->pair_info->pair_req_feat.rkey_dist
//                         & smpc_env[conidx]->pair_info->pair_rsp_feat.rkey_dist;
//     }

//     // Send the LTK to the peer device
//     smpc_pdu_send(conidx, L2C_CODE_ENCRYPTION_INFORMATION, (void *)ltk);
//     // Send the BD Address to the peer device
//     smpc_pdu_send(conidx, L2C_CODE_MASTER_IDENTIFICATION, (void *)mst_id_info);

//     // IRK will be distributed
//     if ((keys & GAP_KDIST_IDKEY) == GAP_KDIST_IDKEY)
//     {
//         // Send the IRK to the peer device
//         smpc_pdu_send(conidx, L2C_CODE_IDENTITY_INFORMATION, (void *)gapm_get_irk());
//         // Send the BD Address to the peer device
//         smpc_pdu_send(conidx, L2C_CODE_IDENTITY_ADDRESS_INFORMATION, (void *)gapm_get_bdaddr());
//     }

//     // CSRK will be distributed
//     if ((keys & GAP_KDIST_SIGNKEY) == GAP_KDIST_SIGNKEY)
//     {
//         // Ask for the CSRK
//         smpc_send_pairing_req_ind(conidx, GAPC_CSRK_EXCH);
//     }
//     else
//     {
//         if (role == ROLE_MASTER)
//         {
//             // Pairing is over
//             smpc_pairing_end(conidx, role, SMP_ERROR_NO_ERROR, false);
//         }
//         else    // role == ROLE_SLAVE
//         {
//             // Master will now send its keys (if needed)
//             smpc_tkdp_rcp_start(conidx, role);
//         }
//     }
// }

// void smpc_tkdp_rcp_start(uint8_t conidx, uint8_t role)
// {
//     // Use generic variable for the master or slave key distribution
//     uint8_t keys;

//     if (role == ROLE_MASTER)
//     {
//         keys = smpc_env[conidx]->pair_info->pair_req_feat.rkey_dist
//                         & smpc_env[conidx]->pair_info->pair_rsp_feat.rkey_dist;
//     }
//     else
//     {
//         keys = smpc_env[conidx]->pair_info->pair_req_feat.ikey_dist
//                         & smpc_env[conidx]->pair_info->pair_rsp_feat.ikey_dist;
//     }

//     if (keys == GAP_KDIST_NONE)
//     {
//         /*
//          * The next step of the Transport Keys Distribution is selected in the same way as if
//          * we had received a CSRK.
//          */

//         smpc_env[conidx]->state = SMPC_PAIRING_REM_CSRK_WAIT;

//         smpc_tkdp_rcp_continue(conidx, role);
//     }
//     else
//     {
//         if ((keys & GAP_KDIST_ENCKEY) == GAP_KDIST_ENCKEY)
//         {
//             smpc_env[conidx]->state = SMPC_PAIRING_REM_LTK_WAIT;
//         }
//         else if ((keys & GAP_KDIST_IDKEY) == GAP_KDIST_IDKEY)
//         {
//             smpc_env[conidx]->state = SMPC_PAIRING_REM_IRK_WAIT;
//         }
//         else if ((keys & GAP_KDIST_SIGNKEY) == GAP_KDIST_SIGNKEY)
//         {
//             smpc_env[conidx]->state = SMPC_PAIRING_REM_CSRK_WAIT;
//         }
//     }
// }

// void smpc_tkdp_rcp_continue(uint8_t conidx, uint8_t role)
// {
//     // Indicate if the peer device still has keys to distribute
//     bool more_key = true;
//     // Peer device keys distribution
//     uint8_t key_dist;

//     if (role == ROLE_MASTER)
//     {
//         key_dist = smpc_env[conidx]->pair_info->pair_req_feat.rkey_dist
//                             & smpc_env[conidx]->pair_info->pair_rsp_feat.rkey_dist;
//     }
//     else    // role == ROLE_SLAVE
//     {
//         key_dist = smpc_env[conidx]->pair_info->pair_req_feat.ikey_dist
//                             & smpc_env[conidx]->pair_info->pair_rsp_feat.ikey_dist;
//     }


//     switch (smpc_env[conidx]->state)
//     {
//         // LTK shall always be followed by the EDIV and the Rand values
//         case (SMPC_PAIRING_REM_LTK_WAIT):
//         {
//             // Wait for the Master ID PDU
//             smpc_env[conidx]->state = SMPC_PAIRING_REM_MST_ID_WAIT;
//         } break;

//         // IRK shall always be followed by the BD Address
//         case (SMPC_PAIRING_REM_IRK_WAIT):
//         {
//             // Wait for the peer device BD Address
//             smpc_env[conidx]->state = SMPC_PAIRING_REM_BD_ADDR_WAIT;
//         } break;

//         case (SMPC_PAIRING_REM_MST_ID_WAIT):
//         {
//             // The peer shall distribute its IRK
//             if ((key_dist & GAP_KDIST_IDKEY) == GAP_KDIST_IDKEY)
//             {
//                 smpc_env[conidx]->state = SMPC_PAIRING_REM_IRK_WAIT;;
//             }
//             else if ((key_dist & GAP_KDIST_SIGNKEY) == GAP_KDIST_SIGNKEY)
//             {
//                 smpc_env[conidx]->state = SMPC_PAIRING_REM_CSRK_WAIT;
//             }
//             else
//             {
//                 more_key = false;

//             }
//         } break;

//         case (SMPC_PAIRING_REM_BD_ADDR_WAIT):
//         {
//             if ((key_dist & GAP_KDIST_SIGNKEY) == GAP_KDIST_SIGNKEY)
//             {
//                 smpc_env[conidx]->state = SMPC_PAIRING_REM_CSRK_WAIT;
//             }
//             else
//             {
//                 more_key = false;
//             }
//         } break;

//         case (SMPC_PAIRING_REM_CSRK_WAIT):
//         {
//             more_key = false;
//         } break;

//         default:
//         {
//             ASSERT_ERR(0);
//         } break;
//     }

//     if (!more_key)
//     {
//         if (role == ROLE_MASTER)
//         {
//             // Slave device has sent all his keys, send his own keys
//             smpc_tkdp_send_start(conidx, ROLE_MASTER);
//         }
//         else    // role = ROLE_SLAVE
//         {
//             // All keys have been exchanged, pairing is over
//             smpc_pairing_end(conidx, role, SMP_ERROR_NO_ERROR, false);
//         }
//     }
// }

// void smpc_pairing_end(uint8_t conidx, uint8_t role, uint8_t status, bool start_ra_timer)
// {
//     if(status == GAP_ERR_NO_ERROR)
//     {
//         smpc_send_pairing_ind(conidx, GAPC_PAIRING_SUCCEED, (void *)NULL);
//     }
//     else
//     {
//         smpc_send_pairing_ind(conidx, GAPC_PAIRING_FAILED, (void *)&status);
//         if (start_ra_timer)
//         {
//             smpc_launch_rep_att_timer(conidx);
//         }
//     }

//     #if (BLE_CENTRAL)
//     // The complete event status to the HL.
//     if (role == ROLE_MASTER)
//     {
//         smpc_send_cmp_evt(conidx, KE_BUILD_ID(TASK_GAPC, conidx), SMPC_PAIRING, GAP_ERR_NO_ERROR);
//     }
//     #endif //(BLE_CENTRAL)

//     if (smpc_env[conidx]->pair_info != NULL)
//     {
//         // Release the memory allocated for the Pairing Information structure.
//         ke_free(smpc_env[conidx]->pair_info);
//         smpc_env[conidx]->pair_info = NULL;
//     }

//     // Stop the timeout timer if needed
//     smpc_clear_timeout_timer(conidx);

//     // Reset the internal state
//     smpc_env[conidx]->state = SMPC_STATE_RESERVED;

//     // Come back to IDLE state
//     ke_state_set(KE_BUILD_ID(TASK_SMPC, conidx), SMPC_IDLE);
// }

// void smpc_clear_timeout_timer(uint8_t conidx)
// {
//     // Test if the Timeout Timer already on, then clear it
//     if (SMPC_IS_FLAG_SET(conidx, SMPC_TIMER_TIMEOUT_FLAG))
//     {
//         ke_timer_clear(SMPC_TIMEOUT_TIMER_IND, KE_BUILD_ID(TASK_SMPC, conidx));
//         SMPC_TIMER_UNSET_FLAG(conidx, SMPC_TIMER_TIMEOUT_FLAG);
//     }
// }

// void smpc_launch_rep_att_timer(uint8_t conidx)
// {
//     // Test if the timer is already running
//     if (SMPC_IS_FLAG_SET(conidx, SMPC_TIMER_REP_ATT_FLAG))
//     {
//         // Stop the timer
//         ke_timer_clear(SMPC_REP_ATTEMPTS_TIMER_IND, KE_BUILD_ID(TASK_SMPC, conidx));
//     }

//     // Start the timer
//     ke_timer_set(SMPC_REP_ATTEMPTS_TIMER_IND,
//                  KE_BUILD_ID(TASK_SMPC, conidx),
//                  smpc_env[conidx]->rep_att_timer_val);

//     // Set the status in the environment
//     SMPC_TIMER_SET_FLAG(conidx, SMPC_TIMER_REP_ATT_FLAG);
// }

// void smpc_get_key_sec_prop(uint8_t conidx)
// {
//     // Check if the TK will be OOB data
//     if ((smpc_env[conidx]->pair_info->pair_req_feat.oob == GAP_OOB_AUTH_DATA_PRESENT) &&
//         (smpc_env[conidx]->pair_info->pair_rsp_feat.oob == GAP_OOB_AUTH_DATA_PRESENT))
//     {
//         // Will have to get the TK from host
//         smpc_env[conidx]->pair_info->pair_method  = SMPC_METH_OOB;
//     }
//     // Both have no MITM set in authentication requirements
//     else if (((smpc_env[conidx]->pair_info->pair_req_feat.auth & GAP_AUTH_MITM) == 0x00) &&
//              ((smpc_env[conidx]->pair_info->pair_rsp_feat.auth & GAP_AUTH_MITM) == 0x00))
//     {
//         // Will have to use TK = 0, no need to ask Host
//         smpc_env[conidx]->pair_info->pair_method  = SMPC_METH_JW;
//     }
//     // In function of IOs, the PASSKEY ENTRY or JW methods will be used
//     else
//     {
//         smpc_env[conidx]->pair_info->pair_method
//                     = smpc_pair_method[smpc_env[conidx]->pair_info->pair_rsp_feat.iocap]
//                                       [smpc_env[conidx]->pair_info->pair_req_feat.iocap];
//     }

//     // Security properties of the STK and all distributed keys
//     switch (smpc_env[conidx]->pair_info->pair_method)
//     {
//         case (SMPC_METH_OOB):
//         case (SMPC_METH_PK):
//         {
//            // All distributed keys will have these properties
//             smpc_env[conidx]->pair_info->auth = GAP_AUTH_MITM;
//         } break;

//         case (SMPC_METH_JW):
//         {
//             // All distributed keys will have these properties
//             smpc_env[conidx]->pair_info->auth = GAP_AUTH_NONE;
//         } break;

//         default:
//         {
//             ASSERT_ERR(0);
//         } break;
//     }

//     // Check if both devices are bondable
//     if (((smpc_env[conidx]->pair_info->pair_req_feat.auth & GAP_AUTH_BOND) == GAP_AUTH_BOND) &&
//         ((smpc_env[conidx]->pair_info->pair_rsp_feat.auth & GAP_AUTH_BOND) == GAP_AUTH_BOND))
//     {
//         smpc_env[conidx]->pair_info->auth |= GAP_AUTH_BOND;
//     }
// }

// bool smpc_is_sec_mode_reached(uint8_t conidx, uint8_t role)
// {
//     // Returned status
//     bool status = true;
//     // Requested Security Mode
//     uint8_t secmode;

//     // Retrieve the requested security level
//     if (role == ROLE_MASTER)
//     {
//         secmode = smpc_env[conidx]->pair_info->pair_req_feat.sec_req;
//     }
//     else // role == ROLE_SLAVE
//     {
//         secmode = smpc_env[conidx]->pair_info->pair_rsp_feat.sec_req;
//     }

//     // The mode is not reached
//     if (smpc_env[conidx]->pair_info->pair_method == SMPC_METH_JW)
//     {
//         if ((secmode == GAP_SEC1_AUTH_PAIR_ENC) || (secmode == GAP_SEC2_AUTH_DATA_SGN))
//         {
//             status = false;
//         }
//     }

//     return status;
// }

// void smpc_handle_enc_change_evt(uint8_t conidx, uint8_t role, uint8_t status)
// {
//     uint8_t int_status = SMP_ERROR_NO_ERROR;

//     switch (smpc_env[conidx]->state)
//     {
//         case (SMPC_START_ENC_LTK):
//         {
//             // Set state to busy
//             ke_state_set(KE_BUILD_ID(TASK_SMPC, conidx), SMPC_BUSY);
//             if (status == CO_ERROR_PIN_MISSING)
//             {
//                 // The peer device cannot find the keys to start encryption
//                 int_status = SMP_ERROR_ENC_KEY_MISSING;
//             }
//             else if (status == CO_ERROR_UNSUPPORTED)
//             {
//                 // The peer device doesn't support encryption
//                 int_status = SMP_ERROR_ENC_NOT_SUPPORTED;
//             }
//             else if (status == CO_ERROR_LMP_RSP_TIMEOUT)
//             {
//                 // The encryption didn't because a timeout has occurred
//                 int_status = SMP_ERROR_ENC_TIMEOUT;
//             }
//             else if (status == CO_ERROR_CON_TIMEOUT)
//             {
//                 // The encryption didn't because a connection timeout has occurred
//                 int_status = SMP_ERROR_LL_ERROR;
//             }
//             else
//             {
//                 ASSERT_INFO(status == CO_ERROR_NO_ERROR, status, conidx);
//             }

//             if (int_status == SMP_ERROR_NO_ERROR)
//             {
//                 // Reset the internal state
//                 smpc_env[conidx]->state = SMPC_STATE_RESERVED;

//                 // Come back to IDLE state
//                 ke_state_set(KE_BUILD_ID(TASK_SMPC, conidx), SMPC_IDLE);

//                 ke_msg_send_basic(SMPC_START_ENC_IND,
//                                   KE_BUILD_ID(TASK_GAPC, conidx), KE_BUILD_ID(TASK_SMPC, conidx));
//                 // informs that link is now encrypted
//                 gapc_link_encrypted(conidx);
//             }

//             if (role == ROLE_MASTER)
//             {
//                 ASSERT_ERR(smpc_env[conidx]->operation != NULL);
//                 ASSERT_ERR(((struct smp_cmd *)(smpc_env[conidx]->operation))->operation == SMPC_START_ENC);

//                 // Send the status of the procedure to the HL
//                 smpc_send_cmp_evt(conidx, KE_BUILD_ID(TASK_GAPC, conidx), SMPC_START_ENC, int_status);
//             }
//         } break;

//         case (SMPC_START_ENC_STK):
//         {
//             // Set state to busy
//             ke_state_set(KE_BUILD_ID(TASK_SMPC, conidx), SMPC_BUSY);
//             ASSERT_ERR(smpc_env[conidx]->pair_info != NULL);

//             if (int_status == SMP_ERROR_NO_ERROR)
//             {
//                 if(role == ROLE_MASTER)
//                 {
//                     ASSERT_ERR(smpc_env[conidx]->operation != NULL);
//                     ASSERT_ERR(((struct smp_cmd *)(smpc_env[conidx]->operation))->operation == SMPC_PAIRING);

//                     /*
//                      * Phase 2 of the pairing is now over, start Transport Keys Distribution.
//                      * The master will received the slave's keys.
//                      */
//                     smpc_tkdp_rcp_start(conidx, ROLE_MASTER);
//                 }
//                 else
//                 {
//                     /*
//                      * Phase 2 of the pairing is now over, start Transport Keys Distribution.
//                      * The slave begins to send its keys.
//                      */
//                     smpc_tkdp_send_start(conidx, ROLE_SLAVE);
//                 }
//             }
//             else
//             {
//                 // Inform the HL that the pairing failed
//                 smpc_pairing_end(conidx, role, SMP_ERROR_LL_ERROR, false);
//             }
//         } break;

//         default:
//         {
//             // Do nothing, this event can be triggered by LL during disconnection,
//             // just ignore it.
//         } break;
//     }
// }

// void smpc_pdu_send(uint8_t conidx, uint8_t cmd_code, void *value)
// {
//     // Test if the Timeout Timer already on, then clear it
//     if (SMPC_IS_FLAG_SET(conidx, SMPC_TIMER_TIMEOUT_FLAG))
//     {
//         ke_timer_clear(SMPC_TIMEOUT_TIMER_IND, KE_BUILD_ID(TASK_SMPC, conidx));
//         SMPC_TIMER_UNSET_FLAG(conidx, SMPC_TIMER_TIMEOUT_FLAG);
//     }

//     // Allocate the message for LLC task
//     struct l2cc_pdu_send_req *send_msg = KE_MSG_ALLOC(L2CC_PDU_SEND_REQ,
//                                                       KE_BUILD_ID(TASK_L2CC, conidx), KE_BUILD_ID(TASK_SMPC, conidx),
//                                                       l2cc_pdu_send_req);

//     send_msg->pdu.chan_id   = L2C_CID_SECURITY;
//     send_msg->pdu.data.code = cmd_code;
//     smpc_construct_pdu[cmd_code](&(send_msg->pdu), value);

//     // Send message to L2CAP
//     ke_msg_send(send_msg);

//     // Start the Timeout Timer (SEC_REQ and PAIRING_FAILED don't need an answer)
//     if ((cmd_code != L2C_CODE_SECURITY_REQUEST) &&
//         (cmd_code != L2C_CODE_PAIRING_FAILED))
//     {
//         ke_timer_set(SMPC_TIMEOUT_TIMER_IND, KE_BUILD_ID(TASK_SMPC, conidx), SMPC_TIMEOUT_TIMER_DURATION);
//         SMPC_TIMER_SET_FLAG(conidx, SMPC_TIMER_TIMEOUT_FLAG);
//     }
// }

// void smpc_pdu_recv(uint8_t conidx, struct l2cc_pdu *pdu)
// {
//     // Check PDU parameters
//     uint8_t status = smpc_check_param(pdu);

//     if (status == SMP_ERROR_NO_ERROR)
//     {
//         // Depending on PDU, do what is specified by protocol
//         (smpc_recv_pdu[pdu->data.code])(conidx, pdu);
//     }
//     // The data packet is bad, send status to GAP.
//     else
//     {
//         // If a packet is received with a reserved code it shall be ignored.
//         if (status != SMP_ERROR_CMD_NOT_SUPPORTED)
//         {
//             // Send PDU fail to peer (status is SMP_ERROR_INVALID_PARAM)
//             smpc_pdu_send(conidx, L2C_CODE_PAIRING_FAILED, (void *)&status);

//             /*
//              * Check if upper layers need to be informed about the error:
//              *      => No error sent if code is PAIRING_REQUEST or SECURITY_REQUEST
//              */
//             if ((pdu->data.code != L2C_CODE_PAIRING_REQUEST) &&
//                 (pdu->data.code != L2C_CODE_SECURITY_REQUEST))
//             {
//                 // Inform HL about the pairing failed - PDU are received only during Pairing
//                 smpc_pairing_end(conidx, gapc_get_role(conidx),
//                                  SMP_GEN_PAIR_FAIL_REASON(SMP_PAIR_FAIL_REASON_MASK, status), true);
//             }
//         }
//     }
// }

 #endif //(BLE_CENTRAL || BLE_PERIPHERAL)

#endif //(RW_BLE_USE_CRYPT)

/// @} SMPC
