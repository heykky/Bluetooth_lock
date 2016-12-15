
 /*
 ****************************************************************************************
 *
 * @file DA14580_examples.c
 *
 * @brief DA14580 Peripheral Examples for DA14580 SDK
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
#include <stdio.h>
#include "global_io.h"

#include "user_main.h"
#include "lis3dh.h"
#include "spi.h"
#include "spi_flash.h"
#include "uart.h"
#include "periph_setup.h"
#include "timer.h"
#include "lis3dh.h"
#include "button.h"

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;
int fputc(int ch, FILE * p_file)
{
    uart_send_byte((uint8_t)ch);
    return 0;
}

int user_init(void)
{

    SetWord16(CLK_AMBA_REG, 0x00);              // set clocks (hclk and pclk ) 16MHz
    SetWord16(SET_FREEZE_REG,FRZ_WDOG);         // stop watch dog
    SetBits16(SYS_CTRL_REG,PAD_LATCH_EN,1);     // open pads
    SetBits16(SYS_CTRL_REG,DEBUGGER_ENABLE,1);  // open debugger
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP,0);    // exit peripheral power down
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP));

    uart_initialization();// Initialize UART component
    GPIO_ConfigurePin( UART_GPIO_PORT, UART_TX_PIN, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( UART_GPIO_PORT, UART_RX_PIN, INPUT, PID_UART1_RX, false );

    lis3dh_initail();

    button_int_initail();

    GPIO_ConfigurePin(PWM0_PORT, PWM0_PIN, OUTPUT, PID_PWM0, true);//pwm
    GPIO_ConfigurePin(PWM4_PORT, PWM1_PIN, OUTPUT, PID_PWM4, true);//pwm

    timer_init();

    return 0;
}

