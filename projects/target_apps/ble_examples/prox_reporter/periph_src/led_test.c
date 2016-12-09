 
 /*
 ****************************************************************************************
 *
 * @file led_test.c
 *
 * @brief LED test functions
 *
 *
 ****************************************************************************************
 */
 #include "periph_setup.h"
 #include "gpio.h"

 
 /**
 ****************************************************************************************
 * @brief SPI and SPI flash test function
  * 
 ****************************************************************************************
 */
void led_test(void)
{
	int ret;
	ret = GPIO_GetPinStatus(LED_GPIO_PORT, USER_LED_PIN);
	
	if(ret)
		GPIO_SetInactive(LED_GPIO_PORT, USER_LED_PIN);
	else
		GPIO_SetActive(LED_GPIO_PORT, USER_LED_PIN);
}
