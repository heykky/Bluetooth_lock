#include "co_endian.h"
#include "lld.h"
#include "llc_util.h"

__INLINE uint16_t my_lld_evt_con_count_get(struct lld_evt_tag *evt)
{
    return (evt->counter - evt->missed_cnt);
}


void my_llc_con_update_req_ind(uint16_t conhdl, struct llcp_con_up_req const *param)
{
    struct llc_env_tag *llc_env_ptr = llc_env[conhdl];
    uint16_t instant = co_btohs(param->instant);
    uint32_t drift = llc_env[conhdl]->evt->drift_base;
    uint32_t latency = param->latency + 1;
    uint32_t winsize = drift * latency;

    // Check if instant has passed
    if(((uint16_t)((instant - my_lld_evt_con_count_get(llc_env[conhdl]->evt)) % 65536) < 32767)
       &&(instant != my_lld_evt_con_count_get(llc_env[conhdl]->evt)))
    {
        // Instant has not passed, so program the LLD to schedule the parameter update
        lld_con_update_ind(llc_env_ptr->evt, param);

        llc_env_ptr->n_sup_to = co_btohs(param->timeout) + 1 + ((winsize / 10000) + 1);//lld_evt_sup_to_compensate(llc_env[conhdl]->evt);

        // Wait for instant
        ke_state_set(KE_BUILD_ID(TASK_LLC, conhdl), LLC_CON_UPD_WAIT_INSTANT);
    }
    else
    {
        // Connection is considered lost, the Link Layer does not send any further packets
        llc_util_dicon_procedure(conhdl, CO_ERROR_INSTANT_PASSED);
    }
}

void my_llc_ch_map_req_ind (uint16_t conhdl, struct llcp_channel_map_req const *param)
{
    uint16_t instant = co_btohs(param->instant);

    // checks if instant is passed.
    if(((uint16_t)((instant - my_lld_evt_con_count_get(llc_env[conhdl]->evt)) % 65536) < 32767)
       &&(instant != my_lld_evt_con_count_get(llc_env[conhdl]->evt)))
    {
        // Instant has not passed, so program the LLD to schedule the parameter update
        lld_ch_map_ind(llc_env[conhdl]->evt, instant);

        // Copy the new channel map from the PDU parameters
        llc_env[conhdl]->n_ch_map = param->ch_map;

        // Wait for instant
        ke_state_set(KE_BUILD_ID(TASK_LLC, conhdl), LLC_MAP_UPD_WAIT_INSTANT);
    }
    else
    {
        // Connection is considered lost, the Link Layer does not send any further packets
        llc_util_dicon_procedure(conhdl,CO_ERROR_INSTANT_PASSED);
    }
}
