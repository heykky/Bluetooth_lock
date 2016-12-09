/**
 ****************************************************************************************
 *
 * @file llc.c
 *
 * @brief Patched functions for memroy leak issue of DA14580. Not applicable in DA14581
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef __DA14581__
#include "rwip_config.h"
#include "llc_task.h"
#include "lld.h"
#include "llc.h"
#include "lld_evt.h"
#include "ke_task.h"
#include "ke_mem.h"

// KE TASK element structure
struct ke_task_elem
{
    uint8_t   type;
    struct ke_task_desc * p_desc;
};

/// KE TASK environment structure
struct ke_task_env_tag
{
    uint8_t task_cnt;
    struct ke_task_elem task_list[];
};
    
extern volatile struct ke_task_env_tag ke_task_env;

/// Used memory block delimiter structure (size must be word multiple)
struct mblock_used
{
    /// Size of the current used block (including delimiter)
    uint16_t size;
    /// Used to check if memory block has been corrupted or not
    uint16_t corrupt_check;
};
#define KE_NOT_BUFF_CORRUPTED  0x8338

void lld_evt_int_extract(struct lld_evt_tag *evt);

int lld_stop_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int lld_data_ind_handler(ke_msg_id_t const msgid,
        struct lld_data_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int llc_le_con_update_cmd_handler(ke_msg_id_t const msgid,
        struct llc_le_con_update_cmd const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int llcp_con_up_req_handler(ke_msg_id_t const msgid,
                        struct llcp_con_up_req const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);

struct lld_evt_tag *alt_evts[BLE_CONNECTION_MAX_USER] __attribute__((section("retention_mem_area0"), zero_init));
extern struct llc_env_tag* llc_env[];
extern const struct ke_msg_handler llc_default_state[];

static int my_llc_le_con_update_cmd_handler(ke_msg_id_t const msgid,
        struct llc_le_con_update_cmd  const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    int ret = llc_le_con_update_cmd_handler(msgid, param, dest_id, src_id);            
    
    if(llc_env[param->conhdl]->evt)
        alt_evts[param->conhdl] = llc_env[param->conhdl]->evt->alt_evt;
    
    return ret;
}

int my_llcp_con_up_req_handler(ke_msg_id_t const msgid,
                        struct llcp_con_up_req const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id)
{
    int ret = llcp_con_up_req_handler(msgid, param, dest_id, src_id);
    uint16_t conhdl = KE_IDX_GET(dest_id);

    if(llc_env[conhdl]->evt)
        alt_evts[conhdl] = llc_env[conhdl]->evt->alt_evt;

    return ret;
}

static int
my_lld_stop_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint16_t conhdl = KE_IDX_GET(dest_id);
    struct lld_evt_tag * evt = llc_env[conhdl]->evt;
    struct lld_evt_tag * alt_evt = NULL;

    if(evt)
    {
        alt_evt = evt->alt_evt;
    }
    else
    {
        alt_evt = alt_evts[conhdl];
        alt_evts[conhdl] = NULL;
    }
    
    // Check if event has an alternative event
    if(alt_evt != NULL)
    {
        struct mblock_used *bfreed = ((struct mblock_used *)alt_evt) - 1;
        if(bfreed->corrupt_check == KE_NOT_BUFF_CORRUPTED
            && bfreed->size == (sizeof(struct lld_evt_tag) + sizeof(struct mblock_used))
            && alt_evt->conhdl == conhdl
            && alt_evt->interval != 0)
        {
        #if (BLE_CENTRAL || BLE_OBSERVER)
        // extract the alternative event interval
        if (alt_evt->int_list != NULL)
            lld_evt_int_extract(alt_evt);
        #endif // (BLE_CENTRAL || BLE_OBSERVER)
        // free alternative event
        ke_free(alt_evt);
        }
    }        
    
    return lld_stop_ind_handler(msgid, param, dest_id, src_id);
}


// declarations of ROM handlers
     //llc_le_con_update_cmd_handler has been defined earlier
     extern int llc_le_rd_chnl_map_cmd_handler (ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_le_rd_rem_used_feats_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_le_start_enc_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_flush_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_disconnect_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_rd_rssi_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_rd_tx_pow_lvl_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_rd_rem_info_ver_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_le_ltk_req_reply_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_le_ltk_req_neg_reply_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_data_req_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_data_ind_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_llcp_tx_cfm_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     //lld_data_ind_handler has been defined earlier
     extern int llm_le_enc_cmp_evt_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_set_host_ch_class_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llm_gen_chnl_cls_cmd_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_link_sup_to_ind_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_llcp_rsp_to_ind_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_version_ind_send_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_unknown_rsp_send_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llc_llcp_unknown_ind_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_terminate_ind_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_feats_req_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_feats_rsp_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_vers_ind_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     //llcp_con_up_req_handler has been defined earlier
     extern int llcp_channel_map_req_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_enc_req_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_enc_rsp_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_start_enc_req_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_start_enc_rsp_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_pause_enc_req_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_pause_enc_rsp_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_reject_ind_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     extern int llcp_unknown_rsp_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
     // lld_stop_ind_handler has been defined earlier


/// Specifies the default message handlers  - overrides ROM handlers
static const struct ke_msg_handler my_llc_default_state[] =
{
    {LLC_LE_UPDATE_CON_REQ, (ke_msg_func_t)my_llc_le_con_update_cmd_handler}, //llc_le_con_update_cmd_handler},
    {LLC_LE_RD_CHNL_MAP_CMD, (ke_msg_func_t)llc_le_rd_chnl_map_cmd_handler},
    {LLC_LE_RD_REM_USED_FEATS_CMD, (ke_msg_func_t)llc_le_rd_rem_used_feats_cmd_handler},
    {LLC_LE_START_ENC_CMD, (ke_msg_func_t)llc_le_start_enc_cmd_handler},
    {LLC_FLUSH_CMD,(ke_msg_func_t)llc_flush_cmd_handler},
    {LLC_DISCONNECT_CMD,(ke_msg_func_t)llc_disconnect_cmd_handler},
    {LLC_RD_RSSI_CMD, (ke_msg_func_t)llc_rd_rssi_cmd_handler},
    {LLC_RD_TX_PW_LVL_CMD, (ke_msg_func_t)llc_rd_tx_pow_lvl_cmd_handler},
    {LLC_RD_REM_VER_INFO_CMD, (ke_msg_func_t)llc_rd_rem_info_ver_cmd_handler},
    {LLC_LE_LTK_REQ_RPLY_CMD, (ke_msg_func_t)llc_le_ltk_req_reply_cmd_handler},
    {LLC_LE_LTK_REQ_NEG_RPLY_CMD, (ke_msg_func_t)llc_le_ltk_req_neg_reply_cmd_handler},
    // data sent to the peer
    {LLC_LE_DATA_REQ, (ke_msg_func_t)llc_data_req_handler},
    // data received from the peer
    {LLC_LE_DATA_IND, (ke_msg_func_t)llc_data_ind_handler},
    {LLC_LLCP_TX_CFM, (ke_msg_func_t)llc_llcp_tx_cfm_handler},
    // data received from the peer
    {LLD_DATA_IND, (ke_msg_func_t)lld_data_ind_handler},
    // data has been encrypted by the AES
    {LLM_LE_ENC_CMP_EVT, (ke_msg_func_t)llm_le_enc_cmp_evt_handler},
    #if (BLE_CENTRAL)
    // request from the llm to send the new channel map
    {LLM_LE_SET_HOST_CHNL_CLASSIF_REQ_IND, (ke_msg_func_t)llc_set_host_ch_class_cmd_handler},
    #if (BLE_CHNL_ASSESS)
    // Channel Assessment - Update the channel classification
    {LLM_GEN_CHNL_CLS_CMD, (ke_msg_func_t)llm_gen_chnl_cls_cmd_handler},
    #endif //(BLE_CHNL_ASSESS)
    #endif // BLE_CENTRAL
    // Time out and termination TO
    {LLC_LE_LINK_SUP_TO, (ke_msg_func_t)llc_link_sup_to_ind_handler},
    // Response Timeout
    {LLC_LLCP_RSP_TO, (ke_msg_func_t)llc_llcp_rsp_to_ind_handler},
    // A LL_VERSION_IND has to be transmitted
    {LLC_VERSION_IND_SEND, (ke_msg_func_t)llc_version_ind_send_handler},
    // A LL_UNKNOWN_RSP has to be transmitted
    {LLC_UNKNOWN_RSP_SEND, (ke_msg_func_t)llc_unknown_rsp_send_handler},
    // Unknown LLCP packet received
    {LLC_LLCP_UNKNOWN_IND, (ke_msg_func_t)llc_llcp_unknown_ind_handler},
    // LLCP Terminate Indication
    {LLCP_TERMINATE_IND, (ke_msg_func_t)llcp_terminate_ind_handler},
    // LLCP Feature Request
    {LLCP_FEATURE_REQ, (ke_msg_func_t)llcp_feats_req_handler},
    // LLCP Feature Response
    {LLCP_FEATURE_RSP, (ke_msg_func_t)llcp_feats_rsp_handler},
    // LLCP Version Indication
    {LLCP_VERSION_IND, (ke_msg_func_t)llcp_vers_ind_handler},
    // LLCP Connection Parameter Update request
    {LLCP_CONNECTION_UPDATE_REQ, (ke_msg_func_t)my_llcp_con_up_req_handler}, // llcp_con_up_req_handler
    // LLCP Channel Map Update request
    {LLCP_CHANNEL_MAP_REQ, (ke_msg_func_t)llcp_channel_map_req_handler},
    // LLCP Encryption request
    {LLCP_ENC_REQ, (ke_msg_func_t)llcp_enc_req_handler},
    // LLCP Encryption response
    {LLCP_ENC_RSP, (ke_msg_func_t)llcp_enc_rsp_handler},
    // LLCP Start Encryption request
    {LLCP_START_ENC_REQ, (ke_msg_func_t)llcp_start_enc_req_handler},
    // LLCP Start Encryption response
    {LLCP_START_ENC_RSP, (ke_msg_func_t)llcp_start_enc_rsp_handler},
    // LLCP Pause Encryption request
    {LLCP_PAUSE_ENC_REQ, (ke_msg_func_t)llcp_pause_enc_req_handler},
    // LLCP Pause Encryption response
    {LLCP_PAUSE_ENC_RSP, (ke_msg_func_t)llcp_pause_enc_rsp_handler},
    // LLCP Reject Indication
    {LLCP_REJECT_IND, (ke_msg_func_t)llcp_reject_ind_handler},
    // LLCP Unknown response
    {LLCP_UNKNOWN_RSP, (ke_msg_func_t)llcp_unknown_rsp_handler},
    // save the indication if not in the good state
    {LLD_STOP_IND,(ke_msg_func_t)lld_stop_ind_handler}
};


/// STOPPING State handlers definition.
static const struct ke_msg_handler my_llc_stopping[] =
{
    /// indication from the LLD
    {LLD_STOP_IND,(ke_msg_func_t)my_lld_stop_ind_handler},
    {LLD_DATA_IND, (ke_msg_func_t)lld_data_ind_handler}
};

/// Specifies the message handler structure for every input state
const struct ke_state_handler my_llc_state_handler[LLC_STATE_MAX] =
{
    /// FREE State message handlers.
    [LLC_FREE] = KE_STATE_HANDLER_NONE,
    /// CONNECTED state message handlers.
    [LLC_CONNECTED] = KE_STATE_HANDLER_NONE,
    /// DISCONNECTING state message handlers.
    [LLC_DISC] =  KE_STATE_HANDLER_NONE,
    /// WAIT ACKNOW state message handlers.
    [LLCP_WAIT_ACK] =  KE_STATE_HANDLER_NONE,
    /// STOPPING state message handlers.
    [LLC_STOPPING] = KE_STATE_HANDLER(my_llc_stopping)
};

const struct ke_state_handler my_llc_default_handler = KE_STATE_HANDLER(my_llc_default_state);

/// LLC task descriptor
const struct ke_task_desc TASK_DESC_LLC = {my_llc_state_handler, &my_llc_default_handler, llc_state, LLC_STATE_MAX, LLC_IDX_MAX};


void patch_llc_task(void)
{
    uint8_t hdl;
    volatile struct ke_task_elem * curr_list = ke_task_env.task_list;
    uint8_t curr_nb = ke_task_env.task_cnt;

    // Search task handle
    for(hdl=0 ; hdl < curr_nb; hdl++)
    {
        if(curr_list[hdl].type == TASK_LLC)
        {
            ke_task_env.task_list[hdl].p_desc = (struct ke_task_desc *) &TASK_DESC_LLC;
            //return;   let it search for all connections
        }
    }
}


#endif

