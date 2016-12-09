/**
 ****************************************************************************************
 *
 * @file gapm_util.c
 *
 * @brief Generic Access Profile Manager Tool Box Implementation
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 * $Rev$
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup GAPM_UTIL
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "gap.h"
#include "gapm.h"
#include "gapm_task.h"
#include "gapm_util.h"

#include "gapc_task.h"

#include "ke_mem.h"

#include "attm_db.h"
#include "attm.h"

#include "smpm.h"
#include "smpm_task.h"

#include "llm_task.h"
///*
// * TYPE DEFINITIONS
// ****************************************************************************************
// */



///*
// * DEFINES
// ****************************************************************************************
// */

///*
// * MACROS
// ****************************************************************************************
// */


///*
// * GLOBAL VARIABLE DEFINITIONS
// ****************************************************************************************
// */

///*
// * LOCAL FUNCTIONS DEFINITIONS
// ****************************************************************************************
// */

//static void gapm_update_address_op_state(struct gapm_air_operation* op, uint8_t state)
//{
//    // execute current operation state.
//    switch(state)
//    {
//        case GAPM_OP_ADDR_GEN: // Address generation state
//        {
//            // Set generated random address
//            GAPM_SET_OP_STATE(*op, GAPM_OP_ADDR_SET);
//        }
//        break;

//        case GAPM_OP_ADDR_SET: // Set device random address
//        {
//            // next step, go to wait state
//            GAPM_SET_OP_STATE(*op, GAPM_OP_START);
//        }
//        break;
//        default:
//        {
//            // clear address renewal field
//            GAPM_CLEAR_OP_FIELD(*op, ADDR_RENEW);

//            if(op->addr_src == GAPM_PUBLIC_ADDR)
//            {
//                // next step, go to start operation state
//                GAPM_SET_OP_STATE(*op, GAPM_OP_START);
//                break;
//            }
//            // Use provided address
//            else if(!((op->addr_src >= GAPM_GEN_STATIC_RND_ADDR)
//                    && (op->addr_src <= GAPM_GEN_NON_RSLV_ADDR)))
//            {
//                // Set provided random address
//                GAPM_SET_OP_STATE(*op, GAPM_OP_ADDR_SET);
//            }
//            else
//            {
//                // Generate address
//                GAPM_SET_OP_STATE(*op, GAPM_OP_ADDR_GEN);
//            }
//        }
//        break;
//    }
//}

///**
// ****************************************************************************************
// * @brief Set a random address operation.
// *
// * This function is used by air operations to modify random address in lower layer before
// * starting any air operations.
// *
// *  - If a public address is configured, operation state is set to @ref GAPM_OP_WAIT.
// *
// *  - If a random address should be generated, @ref SMPM_GEN_RAND_ADDR_CMD will be sent to
// *    @ref TASK_SMPM to request random address generation.
// *
// *  - When random address is generated, or provided by operation parameters, address is
// *    set to lower layers.
// *
// * @param op Operation parameters
// *
// ****************************************************************************************
// */
//static void gapm_set_address_op(struct gapm_air_operation* op)
//{
//    // execute current operation state.
//    switch(GAPM_GET_OP_STATE(*op))
//    {
//        case GAPM_OP_ADDR_GEN: // Address generation state
//        {
//            struct smpm_gen_rand_addr_cmd * req = KE_MSG_ALLOC(SMPM_GEN_RAND_ADDR_CMD,
//                    TASK_SMPM, TASK_GAPM, smpm_gen_rand_addr_cmd);

//            req->operation = GAPM_GEN_RAND_ADDR;

//            // address type to generate
//            switch(op->addr_src)
//            {
//                // Static random address
//                case GAPM_GEN_STATIC_RND_ADDR:  req->addr_type = GAP_STATIC_ADDR; break;
//                // Private resolvable address
//                case GAPM_GEN_RSLV_ADDR:        req->addr_type = GAP_RSLV_ADDR; break;
//                // Private non resolvable address
//                case GAPM_GEN_NON_RSLV_ADDR:    req->addr_type = GAP_NON_RSLV_ADDR; break;
//                // cannot append
//                default: break;
//            }

//            // send request
//            ke_msg_send(req);
//        }
//        break;

//        case GAPM_OP_ADDR_SET: // Set device random address
//        {
//            // update lower layer random address.
//            struct llm_le_set_rand_addr_cmd *rand_addr =
//                        KE_MSG_ALLOC(LLM_LE_SET_RAND_ADDR_CMD, EMB_LLM_TASK, TASK_GAPM,
//                                llm_le_set_rand_addr_cmd);

//            /* random address is in operation address (put during address generation
//             * or provided by host application
//             */
//            memcpy(&(rand_addr->rand_addr), &(op->addr), sizeof(struct bd_addr));

//            // send command
//            ke_msg_send(rand_addr);

//            // start resolvable address timer to regenerate address if timeout occurs.
//            if(op->addr_src)
//            {
//                // minimum duration: GAP_TMR_PRIV_ADDR_INT
//                ke_timer_set(GAPM_ADDR_RENEW_TO_IND, TASK_GAPM,
//                        ((op->renew_dur < GAP_TMR_PRIV_ADDR_INT)
//                                ? GAP_TMR_PRIV_ADDR_INT: op->renew_dur));
//            }
//        }
//        break;
//        default:
//        {
//            // should never happen
//            ASSERT_ERR(0);
//        }
//        break;
//    }
//}


///*
// * EXPORTED FUNCTIONS DEFINITIONS
// ****************************************************************************************
// */

//#if (BLE_PERIPHERAL || BLE_BROADCASTER)

///**
// ****************************************************************************************
// * @brief Verify if advertising data type is unique
// *
// * @param[in] adv_type  Type of advertising data
// *
// * @return True if unique, False else
// ****************************************************************************************
// */
static bool gapm_is_advtype_unique(uint8_t type)
{
    // advertising type check which shall be unique
    switch(type)
    {
        case GAP_AD_TYPE_MORE_16_BIT_UUID:           case GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID:
        case GAP_AD_TYPE_MORE_32_BIT_UUID:           case GAP_AD_TYPE_COMPLETE_LIST_32_BIT_UUID:
        case GAP_AD_TYPE_MORE_128_BIT_UUID:          case GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID:
        case GAP_AD_TYPE_SHORTENED_NAME:             case GAP_AD_TYPE_COMPLETE_NAME:
        case GAP_AD_TYPE_APPEARANCE:                 case GAP_AD_TYPE_ADV_INTV:
        case GAP_AD_TYPE_PUB_TGT_ADDR:               case GAP_AD_TYPE_RAND_TGT_ADDR:
        case GAP_AD_TYPE_LE_BT_ADDR:                 case GAP_AD_TYPE_LE_ROLE:
        case GAP_AD_TYPE_FLAGS:

        return true;

        default: return false;
    }
}


///**
// ****************************************************************************************
// * @brief Perform an advertising data sanity check
// *
// * @param[in] adv_data           Advertising data
// * @param[in] adv_data_len       Advertising data length
// * @param[in] scan_rsp_data      Scan response data
// * @param[in] scan_rsp_data_len  Scan response data length
// *
// * @return GAP_ERR_NO_ERROR if valid, GAP_ERR_ADV_DATA_INVALID if not valid
// ****************************************************************************************
// */
static uint8_t gapm_adv_sanity(uint8_t *adv_data, uint8_t adv_data_len,
                               uint8_t *scan_rsp_data, uint8_t scan_rsp_data_len)
{
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t data_type_cursor = 0;

    // check for duplicate information in advertising or scan response data.
    uint8_t dup_filter[(ADV_DATA_LEN * 2) / 3];
    dup_filter[0] = GAP_AD_TYPE_FLAGS;
    uint8_t dup_filt_cursor = 1;

    while((data_type_cursor < 2) && (status == GAP_ERR_NO_ERROR))
    {
        uint8_t cursor = 0;
        uint8_t* data;
        uint8_t length;
        uint8_t max_length;

        // check adv_data
        if(data_type_cursor == 0)
        {
            data =       adv_data;
            length =     adv_data_len;
            max_length = ADV_DATA_LEN-3;
        }
        // check scan_rsp_data
        else
        {
            data =       scan_rsp_data;
            length =     scan_rsp_data_len;
            max_length = SCAN_RSP_DATA_LEN;
        }

        if(length > max_length)
        {
            status = GAP_ERR_INVALID_PARAM;
        }

        // parse advertising data to find ad_type
        while ((cursor < length) && (status == GAP_ERR_NO_ERROR))
        {
            uint8_t ad_type = data[cursor+1];

            // check if it's AD Type which shall be unique
            if (gapm_is_advtype_unique(ad_type))
            {
                int8_t i;
                // check if there is no duplicate data in advertising.
                for(i = dup_filt_cursor-1 ; i >= 0 ; i--)
                {
                    // check if ad type duplicated
                    if(dup_filter[i] == ad_type)
                    {
                        break;
                    }
                }

                if(i >= 0)
                {
                    // data duplicated
                    status = GAP_ERR_ADV_DATA_INVALID;
                }
                else
                {
                    // put advertising data into filter.
                    dup_filter[dup_filt_cursor] = ad_type;
                    dup_filt_cursor++;
                }
            }

            /* go to next advertising info */
            cursor += data[cursor] + 1;
        }

        // check if total advertising length is valid with advertising data info
        if(cursor != length)
        {
            status = GAP_ERR_ADV_DATA_INVALID;
        }

        // check next data
        data_type_cursor++;
    }
    return status;
}

//uint8_t gapm_adv_op_sanity(struct gapm_start_advertise_cmd *adv)
uint8_t patched_gapm_adv_op_sanity(struct gapm_start_advertise_cmd *adv)
{
    uint8_t status = GAP_ERR_NO_ERROR;

    do
    {
        uint8_t supp_role = ((adv->op.code != GAPM_ADV_NON_CONN)
                ? GAP_PERIPHERAL_SLV: GAP_BROADCASTER_ADV);

        // can advertise only if there's no ongoing connection
        if((gapm_env.connections > 0)
                && (adv->op.code != GAPM_ADV_NON_CONN)
                && (adv->info.host.mode != GAP_BROADCASTER_MODE))
        {
            status = GAP_ERR_COMMAND_DISALLOWED;
            break;
        }

        // check if this operation supported by current role.
        if(!GAPM_IS_ROLE_SUPPORTED(supp_role))
        {
            // role not supported
            status = GAP_ERR_NOT_SUPPORTED;
            break;
        }

        //check operation
        switch(adv->op.code)
        {
            #if (BLE_PERIPHERAL)
            // only advertising is allowed
            case GAPM_ADV_UNDIRECT:
            {
                if(adv->info.host.mode >= GAP_BROADCASTER_MODE)
                {
                    status = GAP_ERR_INVALID_PARAM;
                    break;
                }
            }
            // no break
            #endif // (BLE_PERIPHERAL)
            case GAPM_ADV_NON_CONN:
            {
                // ADV mode sanity check
                if(adv->info.host.mode > GAP_BROADCASTER_MODE)
                {
                    status = GAP_ERR_INVALID_PARAM;
                    break;
                }

                // Filter policy sanity check
                if(adv->info.host.adv_filt_policy >= ADV_ALLOW_SCAN_END)
                {
                    status = GAP_ERR_INVALID_PARAM;
                    break;
                }

                // privacy check ...
                #if (BLE_PERIPHERAL)
                if(gapm_env.role == GAP_PERIPHERAL_SLV)
                {
                    // if privacy enable, only resolvable random address is allowed
                    if((gapm_get_privacy_flag() == GAP_ENABLE)
                            && (adv->op.addr_src != GAPM_GEN_RSLV_ADDR)
                            // TODO [FBE] when privacy is enabled we should used generated random address
                            // but some flexibility added in order to go deeper in some privacy PTS tests
                            // that perform unexpected operation. this flexibility will be removed when
                            // PTS tests will be corrected
                            && (adv->op.addr_src != GAPM_PUBLIC_ADDR))
                    {

                        status = GAP_ERR_PRIVACY_CFG_PB;
                        break;
                    }
                    // if privacy is disabled, only public and static random address are allowed
                    else if((gapm_get_privacy_flag() == GAP_DISABLE)
                            && (adv->op.addr_src > GAPM_GEN_STATIC_RND_ADDR))
                    {
                        status = GAP_ERR_PRIVACY_CFG_PB;
                        break;
                    }
                }
                else
                #endif // (BLE_PERIPHERAL)
                {
                    // While a device is in the Broadcaster, Observer or Central role
                    // the device shall not support the general or limited discoverable mode
                    if((adv->info.host.mode == GAP_GEN_DISCOVERABLE)
                            || (adv->info.host.mode == GAP_LIM_DISCOVERABLE))
                    {
                        status = GAP_ERR_NOT_SUPPORTED;
                        break;
                    }

                    // broadcaster can have no attribute database an in this role
                    // privacy attribute not available. just check that address is
                    // valid for advertising data.
                    if(adv->op.addr_src > GAPM_GEN_RSLV_ADDR)
                    {
                        status = GAP_ERR_PRIVACY_CFG_PB;
                        break;
                    }
                }

                // perform sanity check of advertising data and scan response data.
                status = gapm_adv_sanity(adv->info.host.adv_data,
                                         adv->info.host.adv_data_len,
                                         adv->info.host.scan_rsp_data,
                                         adv->info.host.scan_rsp_data_len);
            }
            break;

            #if (BLE_PERIPHERAL)
            case GAPM_ADV_DIRECT:
            {
                // privacy check ...

                // if privacy enabled
                // Reconnection address can be used, check that reconnection
                // address attribute can be modified
                if((gapm_get_privacy_flag() == GAP_ENABLE)
                    && (adv->op.addr_src == GAPM_PROVIDED_RECON_ADDR)
                    && (!gapm_set_recon_addr(&(adv->op.addr))))
                {
                    status = GAP_ERR_PRIVACY_CFG_PB;
                    break;
                }
                // else only public and static random address are allowed
                else if ((gapm_get_privacy_flag() == GAP_DISABLE)
                        && (adv->op.addr_src > GAPM_GEN_STATIC_RND_ADDR))
                {
                    status = GAP_ERR_PRIVACY_CFG_PB;
                    break;
                }
            }
            break;
            #endif // (BLE_PERIPHERAL)

            default:
            {
                /* send command complete event with error */
                status = GAP_ERR_INVALID_PARAM;
            }
            break;
        };
    } while(0);

    return status;
}

///**
// ****************************************************************************************
// * @brief Manage state of advertising operation
// *
// * @param[in|out] adv  Advertising operation message pointer
// * @param[in] state    Status event that trigger modification of operation state
// ****************************************************************************************
// */
//static void gapm_update_adv_op_state(struct gapm_start_advertise_cmd *adv, uint8_t state)
//{
//    // Update operation states with state
//    switch(state)
//    {
//        // Initialize operation
//        case GAPM_OP_INIT:
//        {
//            // First set scanning parameters
//            GAPM_SET_OP_STATE(adv->op, GAPM_OP_SET_PARAMS);
//        }
//        break;

//        // entry point of scan operation execution.
//        case GAPM_OP_SET_PARAMS:
//        {
//            if(adv->op.code != GAPM_ADV_DIRECT)
//            {
//                // change operation state to set adv data
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_SET_ADV_DATA);
//            }
//            else
//            {
//                // let address management to update state
//                gapm_update_address_op_state(&(adv->op), state);
//            }
//        }
//        break;

//        // Set operation advertise data
//        case GAPM_OP_SET_ADV_DATA:
//        {
//            // change operation state to set scan data
//            GAPM_SET_OP_STATE(adv->op, GAPM_OP_SET_SCAN_RSP_DATA);
//        }
//        break;
//        // Set operation scan response data
//        case GAPM_OP_SET_SCAN_RSP_DATA:
//        {
//            // let address management to update state
//            gapm_update_address_op_state(&(adv->op), state);
//        }
//        break;

//        // address management
//        case GAPM_OP_ADDR_GEN:
//        case GAPM_OP_ADDR_SET:
//        {
//            // let address management to update state
//            gapm_update_address_op_state(&(adv->op), state);
//        }
//        break;

//        // no break
//        case GAPM_OP_START:
//        {
//            if((GAPM_IS_OP_FIELD_SET(adv->op, ADDR_RENEW))
//                    || (GAPM_IS_OP_FIELD_SET(adv->op, CANCELED))
//                    || (GAPM_IS_OP_FIELD_SET(adv->op, TIMEOUT)))
//            {
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_STOP);
//            }
//            else
//            {
//                // next step is wait state
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_WAIT);
//            }
//        }
//        break;
//        case GAPM_OP_STOP:
//        {
//            if((GAPM_IS_OP_FIELD_SET(adv->op, CANCELED))
//                || (GAPM_IS_OP_FIELD_SET(adv->op, TIMEOUT)))
//            {
//                // operation is finished
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_FINISH);
//            }
//            else if(GAPM_IS_OP_FIELD_SET(adv->op, ADDR_RENEW))
//            {
//                // let address management to update state
//                gapm_update_address_op_state(&(adv->op), state);
//            }
//            else
//            {
//                // else return to start state
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_START);
//            }
//        }
//        break;
//        // Operation is in canceled state and shall be terminated.
//        case GAPM_OP_CANCEL:
//        {
//            // Cancel operation
//            GAPM_SET_OP_FIELD(adv->op, CANCELED);

//            if(GAPM_GET_OP_STATE(adv->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_STOP);
//            }
//        }
//        break;
//        // Operation is in canceled state and shall be terminated.
//        case GAPM_OP_TIMEOUT:
//        {
//            // Cancel operation
//            GAPM_SET_OP_FIELD(adv->op, TIMEOUT);

//            if(GAPM_GET_OP_STATE(adv->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_STOP);
//            }
//        }
//        break;
//        // Operation timeout and already terminated.
//        case GAPM_OP_TERM_TIMEOUT:
//        {
//            // operation timeout
//            GAPM_SET_OP_FIELD(adv->op, TIMEOUT);
//            // nothing more to do.
//            GAPM_SET_OP_STATE(adv->op, GAPM_OP_FINISH);
//        }
//        break;

//        // Renew address generation
//        case GAPM_OP_ADDR_RENEW:
//        {
//            // set Connecting state
//            GAPM_SET_OP_FIELD(adv->op, ADDR_RENEW);

//            // if waiting state
//            if(GAPM_GET_OP_STATE(adv->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(adv->op, GAPM_OP_STOP);
//            }
//        }
//        break;
//        case GAPM_OP_CONNECT:
//        {
//            // operation is finished
//            GAPM_SET_OP_STATE(adv->op, GAPM_OP_FINISH);
//        }
//        break;
//        default:
//        {
//            // error state, trigger an error message.
//            GAPM_SET_OP_STATE(adv->op, GAPM_OP_ERROR);
//        }
//        break;
//    }
//}

//void gapm_execute_adv_op(struct gapm_start_advertise_cmd *adv)
//{
//    // clear message in kernel queue
//    GAPM_CLEAR_OP_FIELD(adv->op, QUEUED);

//    // execute current operation state.
//    switch(GAPM_GET_OP_STATE(adv->op))
//    {
//        // entry point of advertising operation execution.
//        case GAPM_OP_SET_PARAMS:
//        {
//            // First, Advertising parameters shall be set in lower layers.
//            struct llm_le_set_adv_param_cmd *adv_par =
//                    KE_MSG_ALLOC(LLM_LE_SET_ADV_PARAM_CMD, EMB_LLM_TASK, TASK_GAPM, llm_le_set_adv_param_cmd);


//            // set own address type
//            adv_par->own_addr_type = ((adv->op.addr_src == GAPM_PUBLIC_ADDR) ?
//                                       ADDR_PUBLIC:
//                                       ADDR_RAND);

//            // set provided parameters
//            adv_par->adv_chnl_map  = adv->channel_map;
//            adv_par->adv_intv_max  = adv->intv_max;
//            adv_par->adv_intv_min  = adv->intv_min;

//            switch(adv->op.code)
//            {
//                case GAPM_ADV_NON_CONN:
//                {
//                    if(adv->info.host.scan_rsp_data_len == 0)
//                    {
//                        // Advertising without scan response data (ADV_NON_CONN_IND)
//                        adv_par->adv_type = ADV_NONCONN_UNDIR;
//                    }
//                    else
//                    {
//                        // Advertising with scan response (ADV_SCAN_IND)
//                        adv_par->adv_type = ADV_DISC_UNDIR;
//                    }
//                    // set filter policy
//                    adv_par->adv_filt_policy = adv->info.host.adv_filt_policy;
//                    // set direct address to default value
//                    adv_par->direct_addr_type = ADDR_RAND;
//                    memset(&adv_par->direct_addr, 0, BD_ADDR_LEN);
//                }
//                break;
//                case GAPM_ADV_UNDIRECT:
//                {
//                    // Advertising connectable (ADV_IND)
//                    adv_par->adv_type = ADV_CONN_UNDIR;
//                    // set filter policy
//                    adv_par->adv_filt_policy = adv->info.host.adv_filt_policy;

//                    // set direct address to default value
//                    adv_par->direct_addr_type = ADDR_RAND;
//                    memset(&adv_par->direct_addr, 0, BD_ADDR_LEN);
//                }
//                break;
//                case GAPM_ADV_DIRECT:
//                {
//                    // Direct connectable advertising (ADV_DIRECT_IND)
//                    adv_par->adv_type = ADV_CONN_DIR;

//                    // Set advertising policy TODO [FBE] do we put initiator address in WL? YES
//                    adv_par->adv_filt_policy  = ADV_ALLOW_SCAN_ANY_CON_ANY;

//                    /* Privacy is enabled, reconnection address is visible
//                     * then we can use reconnection address
//                     */
//                    if((adv->op.addr_src == GAPM_PROVIDED_RECON_ADDR))
//                    {
//                        // set reconnection address
//                        adv_par->direct_addr_type = ADDR_RAND;
//                        memcpy(&adv_par->direct_addr, &adv->op.addr, BD_ADDR_LEN);
//                    }
//                    else
//                    {
//                        // set initiator address
//                        adv_par->direct_addr_type = adv->info.direct.addr_type;
//                        memcpy(&adv_par->direct_addr, &adv->info.direct.addr, BD_ADDR_LEN);
//                    }
//                }
//                break;
//                default: ASSERT_ERR(0); break; // not allowed
//            }

//            /* send the message */
//            ke_msg_send(adv_par);
//        }
//        break;
//        case GAPM_OP_SET_ADV_DATA:
//        {
//            // Set advertising data
//            struct llm_le_set_adv_data_cmd *adv_data =
//                    KE_MSG_ALLOC(LLM_LE_SET_ADV_DATA_CMD, EMB_LLM_TASK, TASK_GAPM,
//                            llm_le_set_adv_data_cmd);

//            // Set data lengh (with added AD_TYPE Flag)
//            adv_data->adv_data_len = adv->info.host.adv_data_len + 3;
//            // copy provided advertising data
//            memcpy(&(adv_data->data.data[3]), adv->info.host.adv_data,  ADV_DATA_LEN-3);
//            // Set ad type flags.
//            adv_data->data.data[0] = 2; // Length of ad type flags
//            adv_data->data.data[1] = GAP_AD_TYPE_FLAGS;
//            adv_data->data.data[2] = GAP_BR_EDR_NOT_SUPPORTED;

//            // set mode in ad_type
//            switch(adv->info.host.mode)
//            {
//                // General discoverable mode
//                case GAP_GEN_DISCOVERABLE:
//                {
//                    adv_data->data.data[2] |= GAP_LE_GEN_DISCOVERABLE_FLG;
//                }
//                break;
//                // Limited discoverable mode
//                case GAP_LIM_DISCOVERABLE:
//                {
//                    adv_data->data.data[2] |= GAP_LE_LIM_DISCOVERABLE_FLG;
//                }
//                break;
//                default: break; // do nothing
//            }

//            /* send the data */
//            ke_msg_send(adv_data);
//        }
//        break;
//        case GAPM_OP_SET_SCAN_RSP_DATA:
//        {
//            struct llm_le_set_scan_rsp_data_cmd *scan_resp =
//                    KE_MSG_ALLOC(LLM_LE_SET_SCAN_RSP_DATA_CMD, EMB_LLM_TASK, TASK_GAPM,
//                            llm_le_set_scan_rsp_data_cmd);
//            // retrieve scan response data length
//            scan_resp->scan_rsp_data_len =adv->info.host.scan_rsp_data_len;
//            // copy provided scan response data
//            memcpy(&(scan_resp->data), adv->info.host.scan_rsp_data, SCAN_RSP_DATA_LEN);

//            ke_msg_send(scan_resp);
//        }
//        break;
//        // address management
//        case GAPM_OP_ADDR_GEN:
//        case GAPM_OP_ADDR_SET:
//        {
//            // Use address management toolbox to set address
//            gapm_set_address_op(&adv->op);
//        }
//        break;
//        case GAPM_OP_START: // Start advertising
//        {
//            // start a timer in limited discoverable mode
//            if(adv->info.host.mode== GAP_LIM_DISCOVERABLE)
//            {
//                ke_timer_set(GAPM_LIM_DISC_TO_IND, TASK_GAPM, GAP_TMR_LIM_ADV_TIMEOUT);
//            }

//            // start advertising mode
//            gapm_set_adv_mode(true);
//        }
//        break;

//        case GAPM_OP_STOP: // Stop
//        {
//            gapm_set_adv_mode(false);
//        }
//        break;

//        default:
//        {
//            // error state, trigger an error message.
//            GAPM_SET_OP_STATE(adv->op, GAPM_OP_ERROR);
//        }
//        break;
//    }

//}
//#endif // (BLE_PERIPHERAL || BLE_BROADCASTER)


//#if (BLE_OBSERVER || BLE_CENTRAL)


//uint8_t gapm_scan_op_sanity(struct gapm_start_scan_cmd *scan)
//{
//    uint8_t status = GAP_ERR_NO_ERROR;

//    do
//    {
//        // check if this operation supported by current role.
//        if(!GAPM_IS_ROLE_SUPPORTED(GAP_OBSERVER_SCA))
//        {
//            // role not supported
//            status = GAP_ERR_NOT_SUPPORTED;
//            break;
//        }

//        //check operation
//        switch(scan->op.code)
//        {
//            // only active and passive scan are allowed
//            case GAPM_SCAN_ACTIVE:
//            case GAPM_SCAN_PASSIVE:
//            {
//                // sanity check address (reconnection address supported only for ADV_DIRECT)
//                if(scan->op.addr_src > GAPM_GEN_NON_RSLV_ADDR)
//                {
//                    status = GAP_ERR_INVALID_PARAM;
//                    break;
//                }

//                // Scan mode sanity check
//                if(scan->mode >= GAP_INVALID_MODE)
//                {
//                    status = GAP_ERR_INVALID_PARAM;
//                    break;
//                }

//                // Filter policy sanity check
//                if(scan->filt_policy >= SCAN_ALLOW_ADV_END)
//                {
//                    status = GAP_ERR_INVALID_PARAM;
//                    break;
//                }

//                // Filter duplicate sanity check
//                if(scan->filter_duplic >= SCAN_FILT_DUPLIC_END)
//                {
//                    status = GAP_ERR_INVALID_PARAM;
//                    break;
//                }
//            }
//            break;

//            default:
//            {
//                /* send command complete event with error */
//                status = GAP_ERR_INVALID_PARAM;
//            }
//            break;
//        }
//    } while(0);

//    return status;
//}

///**
// ****************************************************************************************
// * @brief Manage state of scanning operation
// *
// * @param[in|out] scan  Scanning operation message pointer
// * @param[in] state     Status event that trigger modification of operation state
// ****************************************************************************************
// */
//static void gapm_update_scan_op_state(struct gapm_start_scan_cmd *scan, uint8_t state)
//{
//    // Update operation states with state
//    switch(state)
//    {
//        // Initialize operation
//        case GAPM_OP_INIT:
//        {
//            // First set scanning parameters
//            GAPM_SET_OP_STATE(scan->op, GAPM_OP_SET_PARAMS);
//        }
//        break;

//        // entry point of scan operation execution.
//        case GAPM_OP_SET_PARAMS:
//        {
//            // let address management to update state
//            gapm_update_address_op_state(&(scan->op), state);
//        }
//        break;
//        // address management
//        case GAPM_OP_ADDR_GEN:
//        case GAPM_OP_ADDR_SET:
//        {
//            // let address management to update state
//            gapm_update_address_op_state(&(scan->op), state);
//        }
//        break;

//        // no break
//        case GAPM_OP_START:
//        {
//            if((GAPM_IS_OP_FIELD_SET(scan->op, ADDR_RENEW))
//                    || (GAPM_IS_OP_FIELD_SET(scan->op, CANCELED))
//                    || (GAPM_IS_OP_FIELD_SET(scan->op, TIMEOUT)))
//            {
//                GAPM_SET_OP_STATE(scan->op, GAPM_OP_STOP);
//            }
//            else
//            {
//                // next step is wait state
//                GAPM_SET_OP_STATE(scan->op, GAPM_OP_WAIT);
//            }
//        }
//        break;
//        case GAPM_OP_STOP:
//        {
//            if((GAPM_IS_OP_FIELD_SET(scan->op, CANCELED))
//                || (GAPM_IS_OP_FIELD_SET(scan->op, TIMEOUT)))
//            {
//                // operation is finished
//                GAPM_SET_OP_STATE(scan->op, GAPM_OP_FINISH);
//            }
//            else if(GAPM_IS_OP_FIELD_SET(scan->op, ADDR_RENEW))
//            {
//                // let address management to update state
//                gapm_update_address_op_state(&(scan->op), state);
//            }
//            else
//            {
//                // else return to start state
//                GAPM_SET_OP_STATE(scan->op, GAPM_OP_START);
//            }
//        }
//        break;
//        // Operation is in canceled state and shall be terminated.
//        case GAPM_OP_CANCEL:
//        {
//            // Cancel operation
//            GAPM_SET_OP_FIELD(scan->op, CANCELED);

//            if(GAPM_GET_OP_STATE(scan->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(scan->op, GAPM_OP_STOP);
//            }
//        }
//        break;
//        // Operation is in canceled state and shall be terminated.
//        case GAPM_OP_TIMEOUT:
//        {
//            // Cancel operation
//            GAPM_SET_OP_FIELD(scan->op, TIMEOUT);

//            if(GAPM_GET_OP_STATE(scan->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(scan->op, GAPM_OP_STOP);
//            }
//        }
//        break;
//        // Renew address generation
//        case GAPM_OP_ADDR_RENEW:
//        {
//            // set Connecting state
//            GAPM_SET_OP_FIELD(scan->op, ADDR_RENEW);

//            // if waiting state
//            if(GAPM_GET_OP_STATE(scan->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(scan->op, GAPM_OP_STOP);
//            }
//        }
//        break;
//        default:
//        {
//            // error state, trigger an error message.
//            GAPM_SET_OP_STATE(scan->op, GAPM_OP_ERROR);
//        }
//        break;
//    }
//}

//void gapm_execute_scan_op(struct gapm_start_scan_cmd *scan)
//{
//    // clear message in kernel queue
//    GAPM_CLEAR_OP_FIELD(scan->op, QUEUED);

//    // execute current operation state.
//    switch(GAPM_GET_OP_STATE(scan->op))
//    {
//        // entry point of scan operation execution.
//        case GAPM_OP_SET_PARAMS:
//        {
//            struct llm_le_set_scan_param_cmd *scan_param =
//                    KE_MSG_ALLOC(LLM_LE_SET_SCAN_PARAM_CMD, EMB_LLM_TASK, TASK_GAPM,
//                            llm_le_set_scan_param_cmd);

//            /* fill up the parameters */
//            scan_param->own_addr_type = ((scan->op.addr_src == GAPM_PUBLIC_ADDR)
//                    ? (ADDR_PUBLIC) : (ADDR_RAND));

//            scan_param->scan_filt_policy = scan->filt_policy;
//            scan_param->scan_intv = scan->interval;
//            scan_param->scan_window = scan->window;
//            scan_param->scan_type = ((scan->op.code == GAPM_SCAN_ACTIVE)
//                    ? (SCAN_ACTIVE) : (SCAN_PASSIVE));

//            /* send the message */
//            ke_msg_send(scan_param);
//        }
//        break;
//        // address management
//        case GAPM_OP_ADDR_GEN:
//        case GAPM_OP_ADDR_SET:
//        {
//            // Use address management toolbox to set address
//            gapm_set_address_op(&scan->op);
//        }
//        break;
//        case GAPM_OP_START: // start scanning
//        {
//            switch (scan->mode) {
//                case GAP_GEN_DISCOVERY:
//                {
//                    // start a timer in general discovery mode
//                    ke_timer_set(GAPM_SCAN_TO_IND, TASK_GAPM, GAP_TMR_GEN_DISC_SCAN);
//                }
//                break;
//                case GAP_LIM_DISCOVERY:
//                {
//                    // start a timer in limited discovery mode
//                    ke_timer_set(GAPM_SCAN_TO_IND, TASK_GAPM, GAP_TMR_LIM_DISC_SCAN);
//                }
//                break;
//                default: /* Nothing to do */ break;
//            }

//            // start scanning mode
//            gapm_set_scan_mode(true, scan->filter_duplic);
//        }
//        break;
//        case GAPM_OP_STOP: // wait state
//        {
//            // stop scanning mode
//            gapm_set_scan_mode(false, scan->filter_duplic);
//        }
//        break;
//        default:
//        {
//            // error state, trigger an error message.
//            GAPM_SET_OP_STATE(scan->op, GAPM_OP_ERROR);
//        }
//        break;
//    }

//}
//#endif // (BLE_OBSERVER || BLE_CENTRAL)


//#if (BLE_CENTRAL)


//uint8_t gapm_connect_op_sanity(struct gapm_start_connection_cmd *connect)
//{
//    uint8_t status = GAP_ERR_NO_ERROR;

//    do
//    {
//        // Not possible to establish a connection if max number of connection already established
//        if(gapm_env.connections >= BLE_CONNECTION_MAX)
//        {
//            status = GAP_ERR_COMMAND_DISALLOWED;
//            break;
//        }

//        // check if this operation supported by current role.
//        if(!GAPM_IS_ROLE_SUPPORTED(GAP_CENTRAL_MST))
//        {
//            // role not supported
//            status = GAP_ERR_NOT_SUPPORTED;
//            break;
//        }

//        // sanity check address (reconnection address supported only for ADV_DIRECT)
//        if(connect->op.addr_src > GAPM_GEN_NON_RSLV_ADDR)
//        {
//            status = GAP_ERR_INVALID_PARAM;
//            break;
//        }

//        // Check that some peer devices are set.
//        if(connect->nb_peers == 0)
//        {
//            status = GAP_ERR_INVALID_PARAM;
//            break;
//        }

//        //check operation
//        switch(connect->op.code)
//        {
//            // only active and passive scan are allowed
//            case GAPM_CONNECTION_AUTO:
//            case GAPM_CONNECTION_DIRECT:
//            case GAPM_CONNECTION_SELECTIVE:
//            case GAPM_CONNECTION_NAME_REQUEST:
//            {
//                // Nothing to check
//            }
//            break;

//            default:
//            {
//                /* send command complete event with error */
//                status = GAP_ERR_INVALID_PARAM;
//            }
//            break;
//        }
//    } while(0);

//    return status;
//}


///**
// ****************************************************************************************
// * @brief Manage state of connection establishment operation
// *
// * @param[in|out] scan  connection operation message pointer
// * @param[in] state     Status event that trigger modification of operation state
// ****************************************************************************************
// */
//static void gapm_update_connect_op_state(struct gapm_start_connection_cmd *connect, uint8_t state)
//{
//    // Update operation states with state
//    switch(state)
//    {
//        // Initialize operation
//        case GAPM_OP_INIT:
//        {
//            //check operation
//            switch(connect->op.code)
//            {
//                // only active and passive scan are allowed
//                case GAPM_CONNECTION_AUTO:
//                case GAPM_CONNECTION_SELECTIVE:
//                {
//                    // First clear current white list
//                    GAPM_SET_OP_STATE(connect->op, GAPM_OP_CLEAR_WL);
//                }
//                break;
//                case GAPM_CONNECTION_DIRECT:
//                case GAPM_CONNECTION_NAME_REQUEST:
//                {
//                    // let address management to update state
//                    gapm_update_address_op_state(&(connect->op), state);
//                    // start a connection
//                    GAPM_SET_OP_FIELD(connect->op, CONNECTING);
//                }
//                break;
//                default: /* nothing to do */ break;
//            } /* end of switch */
//        }
//        break;

//        // request to clear white list.
//        case GAPM_OP_CLEAR_WL:
//        {
//            // next step is set white list
//            GAPM_SET_OP_STATE(connect->op, GAPM_OP_SET_WL);
//        }
//        break;

//        // set white list
//        case GAPM_OP_SET_WL:
//        {
//            // next step is set white list
//            GAPM_SET_OP_STATE(connect->op, GAPM_OP_SET_PARAMS);
//        }
//        break;

//        // entry point of scan operation execution.
//        case GAPM_OP_SET_PARAMS:
//        {
//            // let address management to update state
//            gapm_update_address_op_state(&(connect->op), state);
//        }
//        break;
//        // address management
//        case GAPM_OP_ADDR_GEN:
//        case GAPM_OP_ADDR_SET:
//        {
//            // let address management to update state
//            gapm_update_address_op_state(&(connect->op), state);
//        }
//        break;

//        // no break
//        case GAPM_OP_START:
//        {
//            if((GAPM_IS_OP_FIELD_SET(connect->op, ADDR_RENEW))
//                    || (GAPM_IS_OP_FIELD_SET(connect->op, CANCELED))
//                    || ((GAPM_IS_OP_FIELD_SET(connect->op, CONNECTING))
//                            && (GAPM_IS_OP_FIELD_SET(connect->op, SCANNING))))
//            {
//                // first stop current mode
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_STOP);
//            }
//            else
//            {
//                // next step is wait state
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_WAIT);
//            }
//        }
//        break;
//        case GAPM_OP_CONNECT:
//        {
//            if(connect->op.code == GAPM_CONNECTION_NAME_REQUEST)
//            {
//                // next step is name request
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_NAME_REQ);
//            }
//            else
//            {
//                // operation is finished
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_FINISH);
//            }
//        }
//        break;
//        case GAPM_OP_STOP:
//        {
//            if(GAPM_IS_OP_FIELD_SET(connect->op, CANCELED))
//            {
//                // operation is finished
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_FINISH);
//            }
//            else if(GAPM_IS_OP_FIELD_SET(connect->op, ADDR_RENEW))
//            {
//                // let address management to update state
//                gapm_update_address_op_state(&(connect->op), state);
//            }
//            else
//            {
//                // else return to start state
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_START);
//            }
//        }
//        break;
//        // Perform peer device name request
//        case GAPM_OP_NAME_REQ:
//        {
//            // next step is disconnect
//            GAPM_SET_OP_STATE(connect->op, GAPM_OP_DISCONNECT);
//        }
//        break;

//        // Perform peer device disconnection (after name request)
//        case GAPM_OP_DISCONNECT:
//        {
//            GAPM_SET_OP_STATE(connect->op, GAPM_OP_FINISH);
//        }
//        break;

//        // Operation is in canceled state and shall be terminated.
//        case GAPM_OP_CANCEL:
//        {
//            // Cancel operation
//            GAPM_SET_OP_FIELD(connect->op, CANCELED);

//            if(GAPM_GET_OP_STATE(connect->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_STOP);
//            }
//            else if(GAPM_GET_OP_STATE(connect->op) == GAPM_OP_NAME_REQ)
//            {
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_DISCONNECT);
//            }
//        }
//        break;
//        // Renew address generation
//        case GAPM_OP_ADDR_RENEW:
//        {
//            // set Connecting state
//            GAPM_SET_OP_FIELD(connect->op, ADDR_RENEW);

//            // if waiting state
//            if(GAPM_GET_OP_STATE(connect->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_STOP);
//            }
//        }
//        break;

//        // Start connection request
//        case GAPM_OP_CONNECT_REQ:
//        {
//            // set Connecting state
//            GAPM_SET_OP_FIELD(connect->op, CONNECTING);
//            // if waiting state
//            if(GAPM_GET_OP_STATE(connect->op) == GAPM_OP_WAIT)
//            {
//                GAPM_SET_OP_STATE(connect->op, GAPM_OP_STOP);
//            }
//        }
//        break;

//        default:
//        {
//            // error state, trigger an error message.
//            GAPM_SET_OP_STATE(connect->op, GAPM_OP_ERROR);
//        }
//        break;
//    }
//}


//void gapm_execute_connect_op(struct gapm_start_connection_cmd *connect)
//{
//    // clear message in kernel queue
//    GAPM_CLEAR_OP_FIELD(connect->op, QUEUED);

//    // execute current operation state.
//    switch(GAPM_GET_OP_STATE(connect->op))
//    {
//        // request to clear white list.
//        case GAPM_OP_CLEAR_WL:
//        {
//            struct gapm_white_list_mgt_cmd* clear_wl = KE_MSG_ALLOC(GAPM_WHITE_LIST_MGT_CMD,
//                    TASK_GAPM, TASK_GAPM, gapm_white_list_mgt_cmd);

//            // fill parameters
//            clear_wl->operation = GAPM_CLEAR_WLIST;

//            ke_msg_send(clear_wl);
//        }
//        break;

//        // set white list
//        case GAPM_OP_SET_WL:
//        {
//            struct gapm_white_list_mgt_cmd* set_wl = KE_MSG_ALLOC_DYN(GAPM_WHITE_LIST_MGT_CMD,
//                    TASK_GAPM, TASK_GAPM, gapm_white_list_mgt_cmd,
//                    connect->nb_peers * sizeof(struct gap_bdaddr));

//            // fill parameters
//            set_wl->operation = GAPM_ADD_DEV_IN_WLIST;
//            set_wl->nb = connect->nb_peers;
//            memcpy(set_wl->devices, connect->peers,
//                    connect->nb_peers * sizeof(struct gap_bdaddr));
//            ke_msg_send(set_wl);
//        }
//        break;

//        // entry point of scan operation execution.
//        case GAPM_OP_SET_PARAMS:
//        {
//            struct llm_le_set_scan_param_cmd *scan_param =
//                    KE_MSG_ALLOC(LLM_LE_SET_SCAN_PARAM_CMD, EMB_LLM_TASK, TASK_GAPM,
//                            llm_le_set_scan_param_cmd);

//            /* fill up the parameters */
//            scan_param->own_addr_type = ((connect->op.addr_src == GAPM_PUBLIC_ADDR)
//                    ? (ADDR_PUBLIC) : (ADDR_RAND));

//            scan_param->scan_filt_policy = SCAN_ALLOW_ADV_WLST;
//            scan_param->scan_intv = connect->scan_interval;
//            scan_param->scan_window = connect->scan_window;
//            scan_param->scan_type = SCAN_PASSIVE;

//            /* send the message */
//            ke_msg_send(scan_param);
//        }
//        break;
//        // address management
//        case GAPM_OP_ADDR_GEN:
//        case GAPM_OP_ADDR_SET:
//        {
//            // Use address management toolbox to set address
//            gapm_set_address_op(&connect->op);
//        }
//        break;
//        // no break
//        case GAPM_OP_START:
//        {
//            // should we initiate a connection ?
//            if(GAPM_IS_OP_FIELD_SET(connect->op, CONNECTING))
//            {
//                // initiate a connection
//                struct llm_le_create_con_cmd *conn_par = KE_MSG_ALLOC(LLM_LE_CREATE_CON_CMD,
//                        EMB_LLM_TASK, TASK_GAPM, llm_le_create_con_cmd);

//                // fill up the parameters for connection
//                conn_par->init_filt_policy = INIT_FILT_IGNORE_WLST;
//                conn_par->scan_intv        = connect->scan_interval;
//                conn_par->scan_window      = connect->scan_window;
//                conn_par->con_intv_max     = connect->con_intv_max;
//                conn_par->con_intv_min     = connect->con_intv_min;
//                conn_par->ce_len_min       = connect->ce_len_min;
//                conn_par->ce_len_max       = connect->ce_len_max;
//                conn_par->con_latency      = connect->con_latency;
//                conn_par->superv_to        = connect->superv_to;
//                conn_par->peer_addr_type   = connect->peers[0].addr_type;
//                conn_par->own_addr_type    = ((connect->op.addr_src == GAPM_PUBLIC_ADDR)
//                        ? (ADDR_PUBLIC) : (ADDR_RAND));
//                memcpy(&(conn_par->peer_addr), &(connect->peers[0].addr), BD_ADDR_LEN);

//                /* send connection request */
//                ke_msg_send(conn_par);
//            }
//            else
//            {
//                // start scanning operation
//                gapm_set_scan_mode(true, true);
//                GAPM_SET_OP_FIELD(connect->op, SCANNING);
//            }
//        }
//        break;
//        case GAPM_OP_STOP: // Stop state
//        {
//            // next step is address generation
//            GAPM_SET_OP_STATE(connect->op, GAPM_OP_ADDR_GEN);

//            // should we stop scanning
//            if(GAPM_IS_OP_FIELD_SET(connect->op, SCANNING))
//            {
//                // stop scanning operation
//                gapm_set_scan_mode(false, true);
//                GAPM_CLEAR_OP_FIELD(connect->op, SCANNING);
//            }
//            else
//            {
//                // cancel connection request
//                ke_msg_send_basic(LLM_LE_CREATE_CON_CANCEL_CMD, EMB_LLM_TASK, TASK_GAPM);
//            }
//        }
//        break;

//        // Perform peer device name request
//        case GAPM_OP_NAME_REQ:
//        {
//            // connection index has been put in addr_src
//            struct gapc_get_info_cmd* info_cmd = KE_MSG_ALLOC(GAPC_GET_INFO_CMD,
//                    KE_BUILD_ID(TASK_GAPC, connect->op.addr_src), TASK_GAPM,
//                    gapc_get_info_cmd);

//            // request peer device name.
//            info_cmd->operation = GAPC_GET_PEER_NAME;

//            // send command
//            ke_msg_send(info_cmd);
//        }
//        break;

//        // Perform peer device disconnection (after name request)
//        case  GAPM_OP_DISCONNECT:
//        {
//            // connection index has been put in addr_src
//            struct gapc_disconnect_cmd* disconnect_cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
//                    KE_BUILD_ID(TASK_GAPC, connect->op.addr_src), TASK_GAPM,
//                    gapc_disconnect_cmd);

//            // request peer device name.
//            disconnect_cmd->operation = GAPC_DISCONNECT;
//            disconnect_cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;

//            // send command
//            ke_msg_send(disconnect_cmd);
//        }
//        break;

//        default:
//        {
//            // error state, trigger an error message.
//            GAPM_SET_OP_STATE(connect->op, GAPM_OP_ERROR);
//        }
//        break;

//    }
//}
//#endif // (BLE_CENTRAL)

//uint8_t  gapm_get_role(void)
//{
//    // return current role.
//    return gapm_env.role;
//}


//#if (BLE_PERIPHERAL)
//uint8_t gapm_get_privacy_flag(void)
//{
//    struct attm_elmt * attm_elmt = NULL;
//    uint8_t priv_flag = GAP_DISABLE;

//    attm_elmt = attmdb_get_attribute(GAPM_GET_ATT_HANDLE(GAP_IDX_PRIVY_FLAG));
//    if(attm_elmt != NULL)
//    {
//        // ensure that the element is correctly decoded
//        // does not rely only with GAP_IDX_PRIVY_FLAG
//        if (attm_elmt->uuid == ATT_CHAR_PRIVACY_FLAG)
//        {
//            priv_flag = attm_elmt->value[0];
//        }
//    }
//    // limit return to enable or disable
//    return ((priv_flag) ? GAP_ENABLE:GAP_DISABLE);
//}

///* function starts */
//struct bd_addr gapm_get_recon_addr(void)
//{
//    struct bd_addr recon_addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
//    struct attm_elmt * attm_elmt = NULL;

//    attm_elmt = attmdb_get_attribute(GAPM_GET_ATT_HANDLE(GAP_IDX_RECON_ADDR));

//    /* retrieve reconnection address */
//    if(attm_elmt != NULL)
//    {
//        // ensure that the element is correctly decoded
//        // does not rely only with GAP_IDX_RECON_ADDR
//        if (attm_elmt->uuid == ATT_CHAR_RECONNECTION_ADDR)
//            memcpy(&recon_addr, &(attm_elmt->value[0]), sizeof(struct bd_addr));
//    }

//    return recon_addr;
//}

///* function starts */
//void gapm_set_privacy_flag(uint8_t priv_flag)
//{
//    struct attm_elmt * attm_elmt = NULL;
//    attm_elmt = attmdb_get_attribute(GAPM_GET_ATT_HANDLE(GAP_IDX_PRIVY_FLAG));

//    if(attm_elmt != NULL)
//    {
//        // ensure that the element is correctly decoded
//        // does not rely only with GAP_IDX_PRIVY_FLAG
//        if (attm_elmt->uuid == ATT_CHAR_PRIVACY_FLAG)
//        {
//            attm_elmt->value[0] = priv_flag;
//            attm_elmt->length = 1;
//        }
//    }
//}

///* function starts */
//bool gapm_set_recon_addr(struct bd_addr* recon_addr)
//{
//    bool ret = false;

//    struct attm_elmt * attm_elmt = NULL;
//    attm_elmt = attmdb_get_attribute(GAPM_GET_ATT_HANDLE(GAP_IDX_RECON_ADDR));

//    /* modify reconnection address */
//    if(attm_elmt != NULL)
//    {
//        // ensure that the element is correctly decoded
//        // does not rely only with GAP_IDX_RECON_ADDR
//        // and check permission visibility
//        if ((attm_elmt->uuid == ATT_CHAR_RECONNECTION_ADDR)
//            && (PERM_IS_SET(attm_elmt->perm, HIDE, DISABLE)))
//        {
//            memcpy(&(attm_elmt->value[0]), recon_addr,  sizeof(struct bd_addr));
//            attm_elmt->length = sizeof(struct bd_addr);
//            ret = true;
//        }
//    }

//    return ret;
//}
//#endif // BLE_PERIPHERAL


//#if (BLE_BROADCASTER || BLE_PERIPHERAL)
//void gapm_set_adv_mode(uint8_t en_flag)
//{
//    struct llm_le_set_adv_en_cmd *adv_en =
//            KE_MSG_ALLOC(LLM_LE_SET_ADV_EN_CMD, EMB_LLM_TASK, TASK_GAPM,
//                    llm_le_set_adv_en_cmd);

//    adv_en->adv_en = en_flag;

//    /* send the message */
//    ke_msg_send(adv_en);
//}
//#endif // (BLE_BROADCASTER || BLE_PERIPHERAL)


//#if (BLE_OBSERVER || BLE_CENTRAL)
//void gapm_set_scan_mode(uint8_t en_flag, uint8_t filter_duplic_en)
//{
//    struct llm_le_set_scan_en_cmd *scan_en =
//            KE_MSG_ALLOC(LLM_LE_SET_SCAN_EN_CMD, EMB_LLM_TASK, TASK_GAPM,
//                    llm_le_set_scan_en_cmd);

//    scan_en->scan_en = en_flag;
//    scan_en->filter_duplic_en = filter_duplic_en;

//    /* send the message */
//    ke_msg_send(scan_en);
//}


//uint8_t gapm_get_ad_type_flag(uint8_t *data, uint8_t length)
//{
//    uint8_t cursor = 0;
//    uint8_t add_flag = 0;

//    // parse advertising data to find ad_type
//    while (cursor < length)
//    {
//        // check if it's AD Type flag data
//        if (data[cursor+1] == GAP_AD_TYPE_FLAGS)
//        {
//            /* move to the value of the AD flag */
//            add_flag = data[cursor+2];
//            break;
//        }

//        /* go to next advertising info */
//        cursor += data[cursor] + 1;
//    }

//    return add_flag;
//}

//void gapm_add_to_filter(struct bd_addr * addr, uint8_t addr_type)
//{
//    struct bd_addr default_addr = {{0,0,0,0,0,0}};
//    uint8_t cursor = 0;

//    // allocate scan filtered device list if needed.
//    if(gapm_env.scan_filter == NULL)
//    {
//        gapm_env.scan_filter =
//                (struct gap_bdaddr*) ke_malloc(sizeof(struct gap_bdaddr) * GAPM_SCAN_FILTER_SIZE, KE_MEM_KE_MSG);

//        memset(gapm_env.scan_filter, 0, sizeof(struct gap_bdaddr) * GAPM_SCAN_FILTER_SIZE);
//    }

//    // find first available space in array
//    while((memcmp(&(gapm_env.scan_filter[cursor].addr), &default_addr,
//            sizeof(struct bd_addr)) != 0)
//           && (cursor <  GAPM_SCAN_FILTER_SIZE))
//    {
//        cursor++;
//    }

//    // copy provided device address in array
//    if (cursor < GAPM_SCAN_FILTER_SIZE)
//    {
//        memcpy(&(gapm_env.scan_filter[cursor].addr), addr,
//                sizeof(struct bd_addr));
//        gapm_env.scan_filter[cursor].addr_type = addr_type;
//    }
//    // else don't put device into filter.
//}

//bool gapm_is_filtered(struct bd_addr * addr, uint8_t addr_type)
//{
//    bool ret = true;
//    uint8_t cursor = 0;

//    // check that filter array exists
//    if(gapm_env.scan_filter != NULL)
//    {
//        // Find provided address in filter
//        while(((memcmp(&(gapm_env.scan_filter[cursor].addr), addr,
//                sizeof(struct bd_addr)) != 0 )
//                || (gapm_env.scan_filter[cursor].addr_type != addr_type))
//                && (cursor <  GAPM_SCAN_FILTER_SIZE))
//        {
//            cursor++;
//        }
//        // copy provided device address in array
//        if (cursor < GAPM_SCAN_FILTER_SIZE)
//        {
//            ret = false;
//            // remove device from filter.
//            memset(&(gapm_env.scan_filter[cursor].addr), 0,
//                    sizeof(struct bd_addr));
//        }
//    }
//    return ret;
//}

//#endif // (BLE_OBSERVER || BLE_CENTRAL)


//void gapm_update_air_op_state(struct gapm_air_operation *op, uint8_t state)
//{
//    // sanity check
//    ASSERT_WARN(op != NULL);
//    if(op != NULL)
//    {
//        uint8_t prev_state = GAPM_GET_OP_STATE(*op);
//        if(state != GAPM_OP_ERROR)
//        {
//            // Update state only for specific event if message is in kernel queue
//            if(!GAPM_IS_OP_FIELD_SET(*op, QUEUED)
//                    || (state == GAPM_OP_CANCEL) || (state == GAPM_OP_TIMEOUT)
//                    || (state == GAPM_OP_ADDR_RENEW) || (state == GAPM_OP_CONNECT_REQ))
//            {
//                switch(op->code)
//                {
//                    #if (BLE_PERIPHERAL || BLE_BROADCASTER)
//                    case GAPM_ADV_NON_CONN:
//                    case GAPM_ADV_UNDIRECT:
//                    case GAPM_ADV_DIRECT:
//                    {
//                        gapm_update_adv_op_state((struct gapm_start_advertise_cmd*)op , state);
//                    }
//                    break;
//                    #endif // (BLE_PERIPHERAL || BLE_BROADCASTER)

//                    #if (BLE_OBSERVER || BLE_CENTRAL)
//                    case GAPM_SCAN_ACTIVE:
//                    case GAPM_SCAN_PASSIVE:
//                    {
//                        gapm_update_scan_op_state((struct gapm_start_scan_cmd*)op , state);
//                    }
//                    break;
//                    #endif // (BLE_OBSERVER || BLE_CENTRAL)

//                    #if (BLE_CENTRAL)
//                    case GAPM_CONNECTION_DIRECT:
//                    case GAPM_CONNECTION_AUTO:
//                    case GAPM_CONNECTION_SELECTIVE:
//                    case GAPM_CONNECTION_NAME_REQUEST:
//                    {
//                        gapm_update_connect_op_state((struct gapm_start_connection_cmd*)op , state);
//                    }
//                    break;
//                    #endif // (BLE_CENTRAL)

//                    default:
//                    {
//                        // Error unexpected
//                        state = GAPM_OP_ERROR;
//                    }
//                    break;
//                }
//            }
//        }

//        if(!GAPM_IS_OP_FIELD_SET(*op, QUEUED))
//        {
//            if((GAPM_GET_OP_STATE(*op) == GAPM_OP_FINISH) || (state == GAPM_OP_ERROR))
//            {
//                uint8_t status = GAP_ERR_NO_ERROR;
//                // error
//                if((GAPM_GET_OP_STATE(*op) == GAPM_OP_ERROR) || (state == GAPM_OP_ERROR))
//                {
//                    status = GAP_ERR_PROTOCOL_PROBLEM;
//                }
//                // cancel
//                else if(GAPM_IS_OP_FIELD_SET(*op, CANCELED))
//                {
//                    status = GAP_ERR_CANCELED;
//                }
//                // timeout
//                else if(GAPM_IS_OP_FIELD_SET(*op, TIMEOUT))
//                {
//                    status = GAP_ERR_TIMEOUT;
//                }
//                // else succeed

//                // operation is finished
//                gapm_send_complete_evt(GAPM_AIR_OP, status);
//            }

//            else if((prev_state != GAPM_GET_OP_STATE(*op))
//                        && (state != GAPM_OP_INIT)
//                        && (GAPM_GET_OP_STATE(*op) != GAPM_OP_WAIT))
//            {
//                // Inform that operation is pushed into kernel queue
//                GAPM_SET_OP_FIELD(*op, QUEUED);
//                // continue execution
//                ke_msg_forward(op, TASK_GAPM, TASK_GAPM);
//            }
//        }
//        // an error occurs, send it later.
//        else if(state == GAPM_OP_ERROR)
//        {
//            // keep that an error occurs.
//            GAPM_SET_OP_STATE(*op, GAPM_OP_ERROR);
//        }
//        // else do nothing
//    }
//}


//struct gap_sec_key* gapm_get_irk(void)
//{
//    return &(gapm_env.irk);
//}


//struct bd_addr* gapm_get_bdaddr(void)
//{
//    return &(gapm_env.addr);
//}

///// @} GAPM_UTIL
