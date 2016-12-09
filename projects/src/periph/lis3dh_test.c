#include "periph_setup.h"
#include "uart.h"
#include "spi.h"
#include "lis3dh_driver.h"
#include "lis3dh_test.h"

bool flag = false;

static status_t LIS3DH_clearINT()
{
    u8_t value=0;

    if(!LIS3DH_ReadReg(LIS3DH_INT1_SRC, &value))return MEMS_ERROR;
	if(value & 0x40)return MEMS_SUCCESS;

	return MEMS_ERROR;
}

void GPIO2_INT_Handler(void)
{
    flag = true;
    LIS3DH_clearINT();
}


 /**
 ****************************************************************************************
 * @brief SPI and SPI flash Initialization function
  *
 ****************************************************************************************
 */
status_t spi_lis3dh_peripheral_init(void)
{
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
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down.
 *        The Uart and SPi clocks are set.
 *
 ****************************************************************************************
 */
void lis3dh_periph_init(void)  // set i2c, spi, uart, uart2 serial clks
{
    // system init
	SetWord16(CLK_AMBA_REG, 0x00); 				// set clocks (hclk and pclk ) 16MHz
	SetWord16(SET_FREEZE_REG,FRZ_WDOG);			// stop watch dog
	SetBits16(SYS_CTRL_REG,PAD_LATCH_EN,1);  	// open pads
	SetBits16(SYS_CTRL_REG,DEBUGGER_ENABLE,1);  // open debugger
	SetBits16(PMU_CTRL_REG, PERIPH_SLEEP,0);  	// exit peripheral power down

	// Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP));
    SPI_Pad_t spi_LIS3DH_CS_Pad;
    spi_LIS3DH_CS_Pad.pin = SPI_CS_PIN;
    spi_LIS3DH_CS_Pad.port = SPI_GPIO_PORT;
    spi_init(&spi_LIS3DH_CS_Pad, SPI_MODE_16BIT, SPI_ROLE_MASTER, SPI_CLK_IDLE_POL_LOW, SPI_PHA_MODE_0, SPI_MINT_DISABLE, SPI_XTAL_DIV_8);
	lis3dh_set_pad_functions();

#ifdef UART_ENABLED
      uart_initialization();// Initialize UART component
#endif
}

/**
****************************************************************************************
* @brief set gpio port function mode
*
****************************************************************************************
*/
void lis3dh_set_pad_functions(void)
{
#ifdef UART_ENABLED
	 GPIO_ConfigurePin( UART_GPIO_PORT, UART_TX_PIN, OUTPUT, PID_UART1_TX, false );
     GPIO_ConfigurePin( UART_GPIO_PORT, UART_RX_PIN, INPUT, PID_UART1_RX, false );

#endif

#ifdef LED_ENABLED
	 GPIO_ConfigurePin(LED_GPIO_PORT, USER_LED_PIN, OUTPUT, PID_GPIO, false);
#endif


    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_SPI_EN, true );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_0, OUTPUT, PID_SPI_CLK, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_1, OUTPUT, PID_SPI_DO, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_3, INPUT, PID_SPI_DI, false );
    GPIO_ConfigurePin( GPIO_PORT_2, GPIO_PIN_4, INPUT_PULLDOWN, PID_GPIO, false );
    GPIO_EnableIRQ(GPIO_PORT_2,GPIO_PIN_4,GPIO2_IRQn,false,false,10);
    GPIO_RegisterCallback(GPIO2_IRQn, &GPIO2_INT_Handler);

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
    GPIO_ConfigurePin(PWM1_PORT, PWM1_PIN, OUTPUT, PID_PWM1, true);
#endif

#ifdef BUTTON_INT
	//GPIO_ConfigurePin(BUTTON_PORT, BUTTON_PIN, INPUT_PULLUP, PID_GPIO, true);
#endif

#ifdef SW_CURSOR
    GPIO_ConfigurePin(SW_CURSOR_PORT, SW_CURSOR_PIN, OUTPUT, PID_GPIO, true);
#endif
}

