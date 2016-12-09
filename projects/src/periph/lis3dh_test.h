/**
 ****************************************************************************************
 *
 * @file lis3dh_test.h
 *
 * @brief lis3dh test header file.
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
#ifndef _LIS3DH_TEST_H_
#define _LIS3DH_TEST_H_
#include "lis3dh_driver.h"

status_t spi_lis3dh_peripheral_init(void);
void lis3dh_set_pad_functions(void);// set gpio port function mode
void lis3dh_periph_init(void) ;

#endif
