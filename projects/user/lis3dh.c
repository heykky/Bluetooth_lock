/*
 * peripherals.c
 *
 *  Created on: 2016/12/12
 *      Author: lc
 */
#include <core_cm0.h>

#include "global_io.h"
#include "uart.h"
#include "gpio.h"
#include "lis3dh.h"
#include "periph_setup.h"
#include "i2c_eeprom.h"
#include "spi.h"

void lis3dh_periph_init(void)  // set i2c, spi, uart, uart2 serial clks
{
	SetWord16(CLK_AMBA_REG, 0x00); 				// set clocks (hclk and pclk ) 16MHz
	SetWord16(SET_FREEZE_REG,FRZ_WDOG);			// stop watch dog
	SetBits16(SYS_CTRL_REG,PAD_LATCH_EN,1);  	// open pads
	SetBits16(SYS_CTRL_REG,DEBUGGER_ENABLE,1);  // open debugger
	SetBits16(PMU_CTRL_REG, PERIPH_SLEEP,0);  	// exit peripheral power down
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP));

    uart_initialization();// Initialize UART component

#ifdef SPI_LIS3DH
    SPI_Pad_t spi_LIS3DH_CS_Pad;
    spi_LIS3DH_CS_Pad.pin = SPI_CS_PIN;
    spi_LIS3DH_CS_Pad.port = SPI_GPIO_PORT;
    spi_init(&spi_LIS3DH_CS_Pad, \
            SPI_MODE_16BIT, \
			SPI_ROLE_MASTER, \
			SPI_CLK_IDLE_POL_LOW, \
			SPI_PHA_MODE_0, \
			SPI_MINT_DISABLE, \
			SPI_XTAL_DIV_8);
#endif

	lis3dh_set_pad_functions();//Init pads

}

void lis3dh_set_pad_functions(void)
{
#ifdef UART_ENABLED
	 GPIO_ConfigurePin( UART_GPIO_PORT, UART_TX_PIN, OUTPUT, PID_UART1_TX, false );
     GPIO_ConfigurePin( UART_GPIO_PORT, UART_RX_PIN, INPUT, PID_UART1_RX, false );

#endif

#ifdef LED_ENABLED
	 GPIO_ConfigurePin(LED_GPIO_PORT, USER_LED_PIN, OUTPUT, PID_GPIO, false);
#endif

#ifdef SPI_LIS3DH
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_0, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_1, OUTPUT, PID_SPI_DO, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_3, INPUT, PID_SPI_DI, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_4, INPUT_PULLDOWN, PID_GPIO, false );
    GPIO_EnableIRQ(GPIO_PORT_2,GPIO_PIN_4,GPIO2_IRQn,false,false,10);
#endif // SPI_LIS3DH

#ifdef SPI_ENABLED
   GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_CS_PIN, OUTPUT, PID_SPI_EN, true );
   GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_CLK_PIN, OUTPUT, PID_SPI_CLK, false );
   GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_DO_PIN, OUTPUT, PID_SPI_DO, false );
   GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_DI_PIN, INPUT, PID_SPI_DI, false );
#endif

#ifdef EEPROM_ENABLED
   GPIO_ConfigurePin(I2C_GPIO_PORT, I2C_SCL_PIN, INPUT, PID_I2C_SCL, false);
   GPIO_ConfigurePin(I2C_GPIO_PORT, I2C_SDA_PIN, INPUT, PID_I2C_SDA,false);
#endif

#ifdef QUADEC_ENABLED
    GPIO_ConfigurePin( QUADRATURE_ENCODER_CHX_A_PORT, QUADRATURE_ENCODER_CHX_A_PIN, INPUT_PULLUP, PID_GPIO, true);
    GPIO_ConfigurePin( QUADRATURE_ENCODER_CHX_B_PORT, QUADRATURE_ENCODER_CHX_B_PIN, INPUT_PULLUP, PID_GPIO, true);
    GPIO_ConfigurePin( WKUP_TEST_BUTTON_1_PORT, WKUP_TEST_BUTTON_1_PIN, INPUT_PULLUP, PID_GPIO, true);
    GPIO_ConfigurePin( WKUP_TEST_BUTTON_2_PORT, WKUP_TEST_BUTTON_2_PIN, INPUT_PULLUP, PID_GPIO, true);
#endif

#ifdef BUZZER_ENABLED
    GPIO_ConfigurePin(PWM0_PORT, PWM0_PIN, OUTPUT, PID_PWM0, true);
    GPIO_ConfigurePin(PWM4_PORT, PWM1_PIN, OUTPUT, PID_PWM4, true);
#endif

#ifdef BUTTON_INT
	//GPIO_ConfigurePin(BUTTON_PORT, BUTTON_PIN, INPUT_PULLUP, PID_GPIO, true);
#endif

#ifdef SW_CURSOR
    GPIO_ConfigurePin(SW_CURSOR_PORT, SW_CURSOR_PIN, OUTPUT, PID_GPIO, true);
#endif
}

