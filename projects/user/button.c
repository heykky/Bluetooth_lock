/*
 * button.c
 *
 *  Created on: 2016/12/12
 *      Author: lc
 */
#include "button.h"
#include "periph_setup.h"
#include "uart.h"
#include "gpio.h"

static void GPIO1_INT_Handler(void)
{
    printf_string("\r\nbutton happened\r\n");
}

void button_int_initail(void)
{
    GPIO_ConfigurePin(BUTTON_PORT, BUTTON_PIN, INPUT_PULLUP, PID_GPIO, true);
    GPIO_EnableIRQ(BUTTON_PORT,BUTTON_PIN,GPIO1_IRQn,false,true,10);
    GPIO_RegisterCallback(GPIO1_IRQn, &GPIO1_INT_Handler);
}
