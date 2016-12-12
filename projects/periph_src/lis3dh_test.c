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
    if( !LIS3DH_ReadReg(LIS3DH_INT1_SRC, &value) ) 
    	return MEMS_ERROR;

	if(value & 0x40)
		return MEMS_SUCCESS;

	return MEMS_ERROR;
	
}

 /**
 ****************************************************************************************
 * @brief SPI and SPI flash Initialization function
  * 
 ****************************************************************************************
 */
status_t spi_lis3dh_peripheral_init(void)
{
	/*default configuration:
		50Hz
		normal mode
		x y z axes enable
		2G fullscale
		temperature disable
	*/
	/*
			LIS3DH_SetODR(LIS3DH_ODR_100Hz);	//ODR
			LIS3DH_SetMode(LIS3DH_NORMAL);		//Normal mode
			LIS3DH_SetFullScale(LIS3DH_FULLSCALE_2);	//2G fullscale
			LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);//enable X Y Z.
	*/
	u8_t value=0;
	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, 0x5f) ) //100Hz, low power mode.
    	return MEMS_ERROR;
			
	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG2, 0x09) ) //config HP filter to INT1
    	return MEMS_ERROR;
			
	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG3, 0x40) ) //enable AOI 1
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, 0x00) ) //+- 2g 
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, 0x08) ) //enable lack INT1
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_INT1_THS, 0x7F) ) //set threshold
    	return MEMS_ERROR;

	if( !LIS3DH_ReadReg(LIS3DH_REFERENCE_REG, &value) ) //read reference,began compare
    	return MEMS_ERROR;

	if( !LIS3DH_WriteReg(LIS3DH_INT1_CFG, 0x2A) ) //enable X Y Z high event INT
    	return MEMS_ERROR;
    return MEMS_SUCCESS;
}
/**
 ****************************************************************************************
 * @brief SPI and SPI flash test function
  * 
 ****************************************************************************************
 */
void lis3dh_test(void)
{/*
    printf_string("\n\r\n\r***************");
	printf_string("\n\r* LIS3DH TEST *\n\r");
	printf_string("***************\n\r");
	uint8_t ret = 0;
	AxesRaw_t	accel_data;

	LIS3DH_GetWHO_AM_I(&who_am_i);
	*/
	spi_lis3dh_peripheral_init();

/*
	if(MEMS_SUCCESS == LIS3DH_GetWHO_AM_I(&who_am_i))
		printf_string("LIS3DH_WHO_AM_I successed!\n\r");
	else
		printf_string("LIS3DH_WHO_AM_I failure!\n\r");
	*/
    
    //while (1)
    {
        /*if(LIS3DH_GetINT())
        {
            printf("INT dectected\n\r");
        }
    
        else if(0x33 != who_am_i)
        {
			//printf("LIS3DH_WHO_AM_I is 0x%x!\n\r", who_am_i);
			//printf("Enter infitini loop, press RESET button to go back.\n\r");
			//printf("Show 10 times accelerometer raw data.\n\r");
			printf("\n\r");
			
			ret = LIS3DH_GetAccAxesRaw(&accel_data);
				if(ret == MEMS_ERROR)
					printf("ERROR:LIS3DH_GetAccAxesRaw return is MEMS_ERROR");
			printf("Accelerometer raw data is:\n\r");
			printf("X-Axes is %d\n\r", accel_data.AXIS_X);
			printf("Y-Axes is %d\n\r", accel_data.AXIS_Y);
			printf("Z-Axes is %d\n\r", accel_data.AXIS_Z);
			printf("\n\r");
            */
/*
			LIS3DH_GetTempRaw(&tempareture);
			printf("Tempareture is %d degree.\n\r", tempareture);
			printf("\n\r");
*/			
        }
}

#endif

