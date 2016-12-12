/**
 ****************************************************************************************
 *
 * @file lld.c
 *
 * @brief Definition of the functions used by the logical link driver
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup LLD
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "co_utils.h"
#include "co_endian.h"
#include "ke_event.h"
#include "llm.h"
#include "llm_util.h"
#include "llc_cntl.h"
#include "lld.h"
#include "lld_evt.h"
#include "lld_data.h"

#if (DEEP_SLEEP)
    #if (RW_BLE_SUPPORT)
    #include "lld_sleep.h"
    #endif //RW_BLE_SUPPORT
#endif //DEEP_SLEEP

#if (RW_BLE_WLAN_COEX)
#include "lld_wlcoex.h"         // 802.11 coexistence definitions
    #if (RW_BLE_WLAN_COEX_TEST)
    #include "mwsgen.h"                // MWS generator definitions
    #endif //RW_BT_WLAN_COEX_TEST
#endif // RW_BLE_WLAN_COEX

#include "reg_blecore.h"
#include "reg_ble_em_wpb.h"
#include "reg_ble_em_wpv.h"
#include "reg_ble_em_cs.h"
#include "reg_ble_em_et.h"
#include "rwip.h"

#if (NVDS_SUPPORT)
#include "nvds.h"         // NVDS definitions
#endif // NVDS_SUPPORT

extern uint32_t arch_adv_int;

struct lld_evt_tag *lld_adv_start_patch(struct advertising_pdu_params *adv_par,
                                  struct co_buf_tx_node *adv_pdu,
                                  struct co_buf_tx_node *scan_rsp_pdu,
                                  uint8_t adv_pwr)
{
    uint8_t restart_pol;
    uint32_t adv_int;

    // Check the advertising type to put the correct restart policy (directed advertising
    // is programmed only one)
    if (adv_par->type == LLM_ADV_CONN_DIR)
    {
        restart_pol = LLD_NO_RESTART;
    }
    else
    {
        restart_pol = LLD_ADV_RESTART;
    }
    
    adv_int = arch_adv_int;
    
    
    if (adv_int == 0)
    {
        if (adv_par->type == LLM_ADV_NONCONN_UNDIR)
        {
                adv_int = ARCH_ADV_INT_NONCONNECTABLE;
        }
        else if (adv_par->type == LLM_ADV_CONN_DIR)
        {
                adv_int = ARCH_ADV_INT_DIRECT;
        }
        else
        {
                adv_int = ARCH_ADV_INT_UNDIRECT;  
        }
            
    }

    // Create an event to handle the advertising
    struct lld_evt_tag *evt = lld_evt_adv_create(LLD_ADV_HDL,
                                                 adv_par->intervalmin,
                                                 adv_par->intervalmax,
                                                 restart_pol);

    // Update the control structure according to the parameters
    ble_cntl_set(LLD_ADV_HDL, LLD_ADVERTISER);
    ble_hopcntl_set(LLD_ADV_HDL, BLE_FH_EN_BIT | 39);
    ble_crcinit1_set(LLD_ADV_HDL, (adv_par->filterpolicy <<BLE_FILTER_POLICY_LSB) | 0x55);
    ble_rxwincntl_set(LLD_ADV_HDL, BLE_RXWIDE_BIT | evt->duration);
    ble_maxevtime_set(LLD_ADV_HDL, 2400);
    ble_txrxcntl_set(LLD_ADV_HDL, (LLD_RX_IRQ_THRES << BLE_RXTHR_LSB) | adv_pwr );
    ble_fcntoffset_set(LLD_ADV_HDL,0);
    ble_timgencntl_set(150);
    // Set the advertising channel map
    ble_advchmap_set(adv_par->channelmap);

    // Set advertising timing register
    ble_advtim_set(adv_int);

    #if (RW_DM_SUPPORT)
    // Set the priority properties
    lld_prio_init(LLD_ADV_HDL, BLE_ADV_PRIO_INC, BLE_ADV_PRIO_DEF);
    #endif // RW_DM_SUPPORT

    #if (RW_BLE_WLAN_COEX)
    if(adv_par->type == LLM_ADV_NONCONN_UNDIR)
    {
        ble_pti_setf(LLD_ADV_HDL,BLEMPRIO_NCONADV);
        #if (RW_BLE_WLAN_COEX_TEST)
        if(lld_wlcoex_enable)
        {
            ble_dnabort_setf(LLD_ADV_HDL,0);
            ble_rxbsy_en_setf(LLD_ADV_HDL,0);
            ble_txbsy_en_setf(LLD_ADV_HDL,0);
        }
        if(lld_wlcoex_scenario & BLE_WLCOEX_TST_NCONADV_PASSC)
        {
            mwsgen_start();
        }
        #endif // RW_BLE_WLAN_COEX_TEST
    }
    else // connectable advertising
    {
        ble_pti_setf(LLD_ADV_HDL,BLEMPRIO_CONADV);
        if(lld_wlcoex_enable)
        {
            ble_dnabort_setf(LLD_ADV_HDL,1);
            ble_rxbsy_en_setf(LLD_ADV_HDL,1);
            ble_txbsy_en_setf(LLD_ADV_HDL,0);
        }
        #if (RW_BLE_WLAN_COEX_TEST)
        if(lld_wlcoex_scenario & BLE_WLCOEX_TST_CONADV_ACTSC)
        {
            mwsgen_start();
        }
        #endif // RW_BLE_WLAN_COEX_TEST
    }
    #endif //RW_BLE_WLAN_COEX
    // Update the TX Power in the event
    evt->tx_pwr = adv_pwr;

    // Chain the advertising data into the control structure
    lld_data_tx_push(evt, adv_pdu);

    // Chain the scan response data into the control structure
    if (scan_rsp_pdu != NULL)
        lld_data_tx_push(evt, scan_rsp_pdu);

    // Loop the advertising data
    lld_data_tx_loop(evt);

    return (evt);
}