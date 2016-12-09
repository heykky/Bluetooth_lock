
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

#include "DA14580_examples.h"
#include "spi.h"
#include "spi_flash.h"
#include "i2c_eeprom.h"
#include "battery.h"
#include "uart.h"
#include "periph_setup.h"
#include "pwm_test.h"
#include "lis3dh_test.h"

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;
extern bool flag;
 /**
 ****************************************************************************************
 * @brief Battery Level Indication  example function
  *
 ****************************************************************************************
*/
void batt_test(void){

        printf_string("\n\r\n\r****************");
        printf_string("\n\r* BATTERY TEST *\n\r");
        printf_string("****************\n\r");

#if BATT_CR2032
        printf_string("\n\rBattery type: CR2032");
        printf_string("\n\rCurrent battery level (max.0xFF%): 0x");
		printf_byte(battery_get_lvl(BATT_CR2032));
        printf_string("% left");
#else
        printf_string("\n\rBattery type unknown");
#endif

}



/**
 ****************************************************************************************
 * @brief print menu function
  *
 ****************************************************************************************
*/
void print_menu(void)
{
		printf_string("\n\r====================================\n\r");
		printf_string("= DA14580 Peripheral Examples Menu =\n\r");
		printf_string("\n\r====================================\n\r");
		printf_string("u. UART Print String Example\n\r");
#ifdef LED_ENABLED
		printf_string("l. user LED Example\n\r");
#else
    printf_string("l. (disabled in this build) user LED Example\n\r");
#endif //LED_ENABLED

#ifdef SPI_LIS3DH
		printf_string("a. Accelerometer LIS3DH Example\n\r");
#else
    printf_string("a. (disabled in this build) Accelerometer LIS3DH Example\n\r");
#endif //SPI_LIS3DH

#ifdef SPI_ENABLED
		printf_string("f. SPI Flash Memory Example\n\r");
#else
    	printf_string("f. (disabled in this build) SPI Flash Memory Example\n\r");
#endif //SPI_ENABLED
#ifdef EEPROM_ENABLED
		printf_string("e. I2C EEPROM Example\n\r");
#else
		printf_string("e. (disabled in this build) I2C EEPROM Example\n\r");
#endif //EEPROM_ENABLED
#ifdef QUADEC_ENABLED
		printf_string("q. Quadrature Encoder Example\n\r");
#else
  		printf_string("q. (disabled in this build) Quadrature (Rotary Encoder) Example\n\r");
#endif //QUADEC_ENABLED

#ifdef BUZZER_ENABLED
		printf_string("t. Timer0 (PWM0, PWM1) Example\n\r");
		printf_string("p. Timer2 (PWM2, PWM3, PWM4) Example\n\r");
#else
 		printf_string("t. (disabled in this build) Timer0 (PWM0, PWM1) Example\n\r");
		printf_string("p. (disabled in this build) Timer2 (PWM2, PWM3, PWM4) Example\n\r");
#endif //BUZZER_ENABLED

        printf_string("b. Battery Example\n\r");
		printf_string("x. Exit\n\r\n\r");
		print_input();
}

/**
 ****************************************************************************************
 * @brief print input function
  *
 ****************************************************************************************
*/
void print_input(void)
{
		printf_string("\n\rMake a choice : ");
}

/**
 ****************************************************************************************
 * @briefend of test option
  *
 ****************************************************************************************
*/
void endtest_bridge(short int *idx)
{
	short int index2 = 0;
	char bchoice;
	printf_string("\n\r\n\rPress m for Peripheral Examples Menu or x to exit : ");
	while(1)
    {
		if (index2 == 1) break;
		bchoice = uart_receive_byte();
		switch (bchoice)
        {
			case 'm' : print_menu(); index2 = 1; break;
			case 'x' : (*idx)=1; index2 = 1; break;
			default :  break;
		};
	};
}

 /**
 ****************************************************************************************
 * @briefend exit test
  *
 ****************************************************************************************
*/
void exit_test(void)
{
	printf_string("\n\r  End of tests \n\r");
}


int lid3dh_main (void)
{
	lis3dh_periph_init();
    spi_lis3dh_peripheral_init();
    return 0;
}

void Buzzer_test()
{
    if(flag == true)
    {
        flag = false;
		timer0_test();
        printf_string("\n\r End of buzzer_test\n\r");
    }
}

int fputc(int ch, FILE * p_file)
{
    uart_send_byte((uint8_t)ch);
    return 0;
}



