/**
****************************************************************************************
*
* @file user_empty_peripheral_template.h
*
* @brief Empty peripheral template project header file.
*
* Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
*
* <bluetooth.support@diasemi.com> and contributors.
*
****************************************************************************************
*/

#ifndef USER_EMPTY_PERIPHERAL_TEMPLATE_H_
#define USER_EMPTY_PERIPHERAL_TEMPLATE_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions
#include "smpc_task.h"                 // error code definitions


/****************************************************************************
Add here supported profiles' application header files.
i.e.
#if (BLE_DIS_SERVER)
#include "app_dis.h"
#include "app_dis_task.h"
#endif
*****************************************************************************/
#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions
#include "smpc_task.h"                 // error code definitions
#include "arch_wdg.h"

#include "app_callback.h"
#include "app_default_handlers.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_PARAM_UPDATE_REQUEST_TO         (1000)   // 500*10ms = 5sec, The maximum allowed value is 30000 = 300 sec

/* Advertising data update timer */
#define APP_ADV_DATA_UPDATE_TO              (1000)  // 1000*10ms = 10sec

/* Manufacturer specific data constants */
#define APP_AD_MSD_COMPANY_ID       (0xABCD)
#define APP_AD_MSD_COMPANY_ID_LEN   (2)
#define APP_AD_MSD_DATA_LEN         (sizeof(uint16_t))

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param);

void user_on_disconnect( struct gapc_disconnect_ind const *param );

/// @} APP

#endif //USER_EMPTY_PERIPHERAL_TEMPLATE_H_
