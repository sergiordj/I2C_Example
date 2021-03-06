/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: i2c_master.c
 *
 * Description: i2c master API
 *
 * Modification history:
 *     2014/3/12, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"

#include "driver/i2c_master.h"
#include "user_config.h"

#ifndef I2C_MASTER_SDA_GPIO
#define I2C_MASTER_SDA_GPIO 4
#endif

#ifndef I2C_MASTER_SCL_GPIO
#define I2C_MASTER_SCL_GPIO 5
#endif

#ifndef I2C_MASTER_HALF_CYCLE
#define I2C_MASTER_HALF_CYCLE 5
#elif I2C_MASTER_HALF_CYCLE < 3
#define I2C_MASTER_HALF_CYCLE 3
#endif

#if I2C_MASTER_SDA_GPIO == I2C_MASTER_SCL_GPIO
#error "I2C_MASTER_SDA_GPIO can't be equal to I2C_MASTER_SCL_GPIO"
#endif

#if I2C_MASTER_SDA_GPIO == 0
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO0_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO0
#elif I2C_MASTER_SDA_GPIO == 1
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_U0TXD_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO1
#elif I2C_MASTER_SDA_GPIO == 2
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO2_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO2
#elif I2C_MASTER_SDA_GPIO == 3
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_U0RXD_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO3
#elif I2C_MASTER_SDA_GPIO == 4
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO4_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO4
#define I2C_MASTER_SDA_BIT BIT4
#elif I2C_MASTER_SDA_GPIO == 5
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO5_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO5
#define I2C_MASTER_SDA_BIT BIT5
#elif I2C_MASTER_SDA_GPIO == 9
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_SD_DATA2_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO9
#elif I2C_MASTER_SDA_GPIO == 10
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_SD_DATA3_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO10
#elif I2C_MASTER_SDA_GPIO == 12
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_MTDI_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO12
#elif I2C_MASTER_SDA_GPIO == 13
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_MTCK_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO13
#elif I2C_MASTER_SDA_GPIO == 14
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_MTMS_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO14
#elif I2C_MASTER_SDA_GPIO == 15
#define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_MTDO_U
#define I2C_MASTER_SDA_FUNC FUNC_GPIO15
#else 
#error "I2C_MASTER_SDA_GPIO illegal value"
#endif
      
#if I2C_MASTER_SCL_GPIO == 0
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO0_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO0
#elif I2C_MASTER_SCL_GPIO == 1
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_U0TXD_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO1
#elif I2C_MASTER_SCL_GPIO == 2
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO2_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO2
#elif I2C_MASTER_SCL_GPIO == 3
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_U0RXD_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO3
#elif I2C_MASTER_SCL_GPIO == 4
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO4_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO4
#define I2C_MASTER_SCL_BIT BIT4
#elif I2C_MASTER_SCL_GPIO == 5
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO5_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO5
#define I2C_MASTER_SCL_BIT BIT5
#elif I2C_MASTER_SCL_GPIO == 9
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_SD_DATA2_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO9
#elif I2C_MASTER_SCL_GPIO == 10
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_SD_DATA3_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO10
#elif I2C_MASTER_SCL_GPIO == 12
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_MTDI_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO12
#elif I2C_MASTER_SCL_GPIO == 13
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_MTCK_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO13
#elif I2C_MASTER_SCL_GPIO == 14
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_MTMS_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO14
#elif I2C_MASTER_SCL_GPIO == 15
#define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_MTDO_U
#define I2C_MASTER_SCL_FUNC FUNC_GPIO15
#else 
#error "I2C_MASTER_SCL_GPIO illegal value"
#endif


LOCAL uint8_t m_nLastSDA;
LOCAL uint8_t m_nLastSCL;

/******************************************************************************
 * FunctionName : i2c_master_setDC
 * Description  : Internal used function -
 *                    set i2c SDA and SCL bit value for half clk cycle
 * Parameters   : uint8_t SDA
 *                uint8_t SCL
 * Returns      : NONE
*******************************************************************************/
LOCAL uint8_t ICACHE_FLASH_ATTR
i2c_master_setDC(uint8_t SDA, uint8_t SCL)
{
    SDA	&= 0x01;
    SCL	&= 0x01;
    m_nLastSDA = SDA;
    m_nLastSCL = SCL;

   if ((0 == SDA) && (0 == SCL)) {
        gpio_output_set(0, I2C_MASTER_SDA_BIT, I2C_MASTER_SDA_BIT, 0);
        gpio_output_set(0, I2C_MASTER_SCL_BIT, I2C_MASTER_SCL_BIT, 0);
        return true;
    } else if ((0 == SDA) && (1 == SCL)) {
		gpio_output_set(0, I2C_MASTER_SDA_BIT, I2C_MASTER_SDA_BIT, 0);
        gpio_output_set(0, 0, 0, I2C_MASTER_SCL_BIT);
        if ( GPIO_INPUT_GET(I2C_MASTER_SCL_GPIO) ){
			return true;
		} else return false;
    } else if ((1 == SDA) && (0 == SCL)) {
		gpio_output_set(0, 0, 0, I2C_MASTER_SDA_BIT);
        gpio_output_set(0, I2C_MASTER_SCL_BIT, I2C_MASTER_SCL_BIT, 0);
        if ( GPIO_INPUT_GET(I2C_MASTER_SDA_GPIO) ){
			return true;
		} else return false;
    } else {
		gpio_output_set(0, 0, 0, I2C_MASTER_SDA_BIT);
		gpio_output_set(0, 0, 0, I2C_MASTER_SCL_BIT);
		if ( GPIO_INPUT_GET(I2C_MASTER_SDA_GPIO) && GPIO_INPUT_GET(I2C_MASTER_SCL_GPIO)){
			return true;
		} else return false;
    }
}

/******************************************************************************
 * FunctionName : i2c_master_getDC
 * Description  : Internal used function -
 *                    get i2c SDA bit value
 * Parameters   : NONE
 * Returns      : uint8_t - SDA bit value
*******************************************************************************/
LOCAL uint8_t ICACHE_FLASH_ATTR
i2c_master_getDC(void)
{
    uint8_t sda_out;
    sda_out = GPIO_INPUT_GET(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO));
    return sda_out;
}

/******************************************************************************
 * FunctionName : i2c_master_init
 * Description  : initilize I2C bus to enable i2c operations
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_init(void)
{
    uint8_t i;

    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);

    // when SCL = 0, toggle SDA to clear up
    i2c_master_setDC(0, 0) ;
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);
    i2c_master_setDC(1, 0) ;
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);

    // set data_cnt to max value
    for (i = 0; i < 28; i++) {
        i2c_master_setDC(1, 0);
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 0
        i2c_master_setDC(1, 1);
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 1
    }

    // reset all
    i2c_master_stop();
    return;
}

/******************************************************************************
 * FunctionName : i2c_master_gpio_init
 * Description  : config SDA and SCL gpio to open-drain output mode,
 *                mux and gpio num defined in i2c_master.h
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_gpio_init(void)
{
    ETS_GPIO_INTR_DISABLE() ;
//    ETS_INTR_LOCK();

    PIN_FUNC_SELECT(I2C_MASTER_SDA_MUX, I2C_MASTER_SDA_FUNC);
    PIN_FUNC_SELECT(I2C_MASTER_SCL_MUX, I2C_MASTER_SCL_FUNC);

    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << I2C_MASTER_SDA_GPIO));
    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << I2C_MASTER_SCL_GPIO));

    gpio_output_set(0, 0, 0, I2C_MASTER_SDA_BIT); //Set SDA in "1"
	gpio_output_set(0, 0, 0, I2C_MASTER_SCL_BIT); //Set SCL in "1"
	
    ETS_GPIO_INTR_ENABLE() ;
//    ETS_INTR_UNLOCK();

    i2c_master_init();
}

/******************************************************************************
 * FunctionName : i2c_master_start
 * Description  : set i2c to send state
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
uint8_t ICACHE_FLASH_ATTR
i2c_master_start(void)
{
	if ( GPIO_INPUT_GET(I2C_MASTER_SDA_GPIO) && GPIO_INPUT_GET(I2C_MASTER_SCL_GPIO) ){
		if ( false == i2c_master_setDC(1, m_nLastSCL) ){
			i2c_master_stop();
			return 0;
		}
		i2c_master_wait(I2C_MASTER_HALF_CYCLE);
		if ( false == i2c_master_setDC(1, 1) ){
			i2c_master_stop();
			return 0;
		}
		i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 1
		if ( false == i2c_master_setDC(0, 1) ){
			i2c_master_stop();
			return 0;
		}
		i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 0, scl 1
		return 1;
	}
	else return 0;
}

/******************************************************************************
 * FunctionName : i2c_master_stop
 * Description  : set i2c to stop sending state
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_stop(void)
{
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);

    i2c_master_setDC(0, m_nLastSCL);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 0
    i2c_master_setDC(0, 1);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 0, scl 1
    i2c_master_setDC(1, 1);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 1
}

/******************************************************************************
 * FunctionName : i2c_master_setAck
 * Description  : set ack to i2c bus as level value
 * Parameters   : uint8_t level - 0 or 1
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_setAck(uint8_t level)
{
    i2c_master_setDC(m_nLastSDA, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);
    i2c_master_setDC(level, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda level, scl 0
    i2c_master_setDC(level, 1);
    i2c_master_wait(8);	// sda level, scl 1
    i2c_master_setDC(level, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda level, scl 0
    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);
}

/******************************************************************************
 * FunctionName : i2c_master_getAck
 * Description  : confirm if peer send ack
 * Parameters   : NONE
 * Returns      : uint8_t - ack value, 0 or 1
*******************************************************************************/
uint8_t ICACHE_FLASH_ATTR
i2c_master_getAck(void)
{
    uint8_t retVal;
    i2c_master_setDC(m_nLastSDA, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);
    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);
    i2c_master_setDC(1, 1);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);

    retVal = i2c_master_getDC();
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);
    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);

    return retVal;
}

/******************************************************************************
* FunctionName : i2c_master_checkAck
* Description  : get dev response
* Parameters   : NONE
* Returns      : true : get ack ; false : get nack
*******************************************************************************/
bool ICACHE_FLASH_ATTR
i2c_master_checkAck(void)
{
    if(i2c_master_getAck()){
        return FALSE;
    }else{
        return TRUE;
    }
}

/******************************************************************************
* FunctionName : i2c_master_send_ack
* Description  : response ack
* Parameters   : NONE
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_send_ack(void)
{
    i2c_master_setAck(0x0);
}
/******************************************************************************
* FunctionName : i2c_master_send_nack
* Description  : response nack
* Parameters   : NONE
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_send_nack(void)
{
    i2c_master_setAck(0x1);
}

/******************************************************************************
 * FunctionName : i2c_master_readByte
 * Description  : read Byte from i2c bus
 * Parameters   : NONE
 * Returns      : uint8_t - readed value
*******************************************************************************/
uint8_t ICACHE_FLASH_ATTR
i2c_master_readByte(void)
{
    uint8_t retVal = 0;
    uint8_t k, i;

    i2c_master_wait(I2C_MASTER_HALF_CYCLE);
    i2c_master_setDC(m_nLastSDA, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 0

    for (i = 0; i < 8; i++) {
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);
        i2c_master_setDC(1, 0);
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 0
        false == i2c_master_setDC(1, 1);
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 1

        k = i2c_master_getDC();
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);

        if (i == 7) {
          i2c_master_wait(I2C_MASTER_HALF_CYCLE-2);   ////
        }

        k <<= (7 - i);
        retVal |= k;
    }

    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);	// sda 1, scl 0

    return retVal;
}

/******************************************************************************
 * FunctionName : i2c_master_writeByte
 * Description  : write wrdata value(one byte) into i2c
 * Parameters   : uint8_t wrdata - write value
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_writeByte(uint8_t wrdata)
{
    uint8_t dat;
    sint8 i;

    i2c_master_wait(I2C_MASTER_HALF_CYCLE);

    if ( false == i2c_master_setDC(m_nLastSDA, 0) ){
		i2c_master_stop;
		return;
	}
    i2c_master_wait(I2C_MASTER_HALF_CYCLE);

    for (i = 7; i >= 0; i--) {
        dat = wrdata >> i;
        if ( false == i2c_master_setDC(dat, 0) ){
			i2c_master_stop;
			return;
		}
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);
        if ( false == i2c_master_setDC(dat, 1) ){
			i2c_master_stop;
			return;
		}
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);

        if (i == 0) {
          i2c_master_wait(I2C_MASTER_HALF_CYCLE-2);   ////
        }

        if ( false == i2c_master_setDC(dat, 0) ){
			i2c_master_stop;
			return;
		}
        i2c_master_wait(I2C_MASTER_HALF_CYCLE);
    }
}

