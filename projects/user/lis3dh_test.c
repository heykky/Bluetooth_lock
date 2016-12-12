#if SPI_LIS3DH

#include "periph_setup.h"
#include "uart.h"
#include "spi.h"
#include "lis3dh_driver.h"

u8_t LIS3DH_ReadReg(u8_t, u8_t* );
u8_t LIS3DH_WriteReg(u8_t, u8_t );

status_t LIS3DH_GetINT()
{
    u8_t value=0;
    if( !LIS3DH_ReadReg(LIS3DH_INT1_SRC, &value) )return MEMS_ERROR;

	if(value & 0x40)return MEMS_SUCCESS;

	return MEMS_ERROR;
}

 /**
 ****************************************************************************************
 * @brief SPI and SPI flash Initialization function
  *
 ****************************************************************************************
 */
status_t spi_lis3dh_init(void)
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

	if( !LIS3DH_WriteReg(LIS3DH_INT1_THS, 0x10) ) //set threshold
    	return MEMS_ERROR;

	if( !LIS3DH_ReadReg(LIS3DH_REFERENCE_REG, &value) ) //read reference,began compare
    	return MEMS_ERROR;

    return MEMS_SUCCESS;
}

void lis3dh_test(void)
{
    spi_lis3dh_init();
}

#endif

