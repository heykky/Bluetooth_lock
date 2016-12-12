/*
 * lis3dh.c
 *
 *  Created on: 2016/12/12
 *      Author: lc
 */
#include <core_cm0.h>
#include "global_io.h"
#include "gpio.h"
#include "uart.h"
#include "spi.h"

#include "periph_setup.h"

#include "lis3dh.h"
#include "timer.h"

static status_t spi_lis3dh_init(void)
{
	u8_t value=0;
	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, 0x5f) ) //100Hz, low power mode.
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG2, 0x09) ) //config HP filter to INT1
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG3, LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE) ) //enable AOI 1
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, 0x00) ) //+- 2g
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, 0x08) ) //enable lack INT1
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_INT1_CFG, LIS3DH_INT1_MOVE|LIS3DH_INT1_ZHIE_ENABLE|LIS3DH_INT1_YHIE_ENABLE|LIS3DH_INT1_XHIE_ENABLE)) //enable move int
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_INT1_THS, 0x40) ) //set threshold
    	return MEMS_ERROR;

	if( !LIS3DH_ReadReg(LIS3DH_REFERENCE_REG, &value) ) //read reference,began compare
    	return MEMS_ERROR;

    return MEMS_SUCCESS;
}

status_t LIS3DH_clearINT(void)
{
    u8_t value=0;
    if( !LIS3DH_ReadReg(LIS3DH_INT1_SRC, &value) )return MEMS_ERROR;
	if(value & 0x40)return MEMS_SUCCESS;
	return MEMS_ERROR;
}

static void GPIO2_INT_Handler(void)
{
    printf_string("\r\nint happened\r\n");
    timer0_pwm_alarm();
    LIS3DH_clearINT();
}

void lis3dh_initail(void)
{

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

    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_0, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_1, OUTPUT, PID_SPI_DO, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_3, INPUT, PID_SPI_DI, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_4, INPUT_PULLDOWN, PID_GPIO, false );
    GPIO_EnableIRQ(GPIO_PORT_2,GPIO_PIN_4,GPIO2_IRQn,false,false,10);
    GPIO_RegisterCallback(GPIO2_IRQn, &GPIO2_INT_Handler);

    GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_CS_PIN, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_CLK_PIN, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_DO_PIN, OUTPUT, PID_SPI_DO, false );
    GPIO_ConfigurePin( SPI_GPIO_PORT, SPI_DI_PIN, INPUT, PID_SPI_DI, false );

    spi_lis3dh_init();
}

