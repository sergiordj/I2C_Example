/* Copyright 2017, Sergio De Jesús
 * All rights reserved.
 *
 * Using:
 * I2C driver for DS3231 RTC for ESP8266.
 * Copyright 2015 Richard A Burton
 *
 */
 
/** @brief This is a DS3231 RTC Example.
 */


/*==================[inclusions]=============================================*/

#include "../../I2C_Example/inc/main.h"
#include "board.h"
#include "ciaaI2C.h"
#include "ciaaUART.h"
#include "time.h"

/*==================[macros and definitions]=================================*/
#define DS3231_ADDR 0x68

#define DS3231_ADDR_TIME    0x00
#define DS3231_ADDR_ALARM1  0x07
#define DS3231_ADDR_ALARM2  0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS  0x0f
#define DS3231_ADDR_AGING   0x10
#define DS3231_ADDR_TEMP    0x11

#define DS3231_SET     0
#define DS3231_CLEAR   1
#define DS3231_REPLACE 2

#define DS3231_12HOUR_FLAG 0x40
#define DS3231_12HOUR_MASK 0x1f
#define DS3231_PM_FLAG     0x20
#define DS3231_MONTH_MASK  0x1f
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

/** @brief delay function
 * @param t desired milliseconds to wait
 */
static void pausems(uint32_t t);

/*==================[internal data definition]===============================*/

/** @brief used for delay counter */
static uint32_t pausems_count;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
// convert normal decimal to binary coded decimal
static uint8_t decToBcd(uint8_t dec)
{
	return(((dec / 10) * 16) + (dec % 10));
}

// convert binary coded decimal to normal decimal
static uint8_t bcdToDec(uint8_t bcd) {
  return(((bcd / 16) * 10) + (bcd % 16));
}

// set the time on the rtc
// timezone agnostic, pass whatever you like
// I suggest using GMT and applying timezone and DST when read back
// returns true to indicate success
bool ds3231_setTime(struct tm *time) {

	uint8_t RTC_Time[8];

	// start register
	RTC_Time[0] = DS3231_ADDR_TIME;
	// time/date data
	RTC_Time[1] = decToBcd(time->tm_sec);
	RTC_Time[2] = decToBcd(time->tm_min);
	RTC_Time[3] = decToBcd(time->tm_hour);
	RTC_Time[4] = decToBcd(time->tm_wday + 1);
	RTC_Time[5] = decToBcd(time->tm_mday);
	RTC_Time[6] = decToBcd(time->tm_mon + 1);
	RTC_Time[7] = decToBcd(time->tm_year - 100);

	return ciaaI2CWrite(DS3231_ADDR, RTC_Time, sizeof(RTC_Time));

}

// get the temperature as an integer (rounded down)
// returns true to indicate success
bool ds3231_getTempInteger(int8_t *temp) {
	uint8_t RTC_Temp[1];
	RTC_Temp[0] = DS3231_ADDR_TEMP;
	// get just the integer part of the temp
	if (ciaaI2CWrite(DS3231_ADDR, RTC_Temp, 1) && ciaaI2CRead(DS3231_ADDR, RTC_Temp, 1)) {
		*temp = (signed)RTC_Temp[0];
		return true;
	}
	return false;
}

// get the time from the rtc, populates a supplied tm struct
// returns true to indicate success
bool ds3231_getTime(struct tm *time) {

	int loop;
	uint8_t RTC_Hour[7];

	// start register address
	RTC_Hour[0] = DS3231_ADDR_TIME;
	if (!ciaaI2CWrite(DS3231_ADDR, RTC_Hour, 1)) {
		return false;
	}

	// read time
	if (!ciaaI2CRead(DS3231_ADDR, RTC_Hour, 7)) {
		return false;
	}

	// convert to unix time structure
	time->tm_sec = bcdToDec(RTC_Hour[0]);
	time->tm_min = bcdToDec(RTC_Hour[1]);
	if (RTC_Hour[2] & DS3231_12HOUR_FLAG) {
		// 12h
		time->tm_hour = bcdToDec(RTC_Hour[2] & DS3231_12HOUR_MASK);
		// pm?
		if (RTC_Hour[2] & DS3231_PM_FLAG) time->tm_hour += 12;
	} else {
		// 24h
		time->tm_hour = bcdToDec(RTC_Hour[2]);
	}
	time->tm_wday = bcdToDec(RTC_Hour[3]) - 1;
	time->tm_mday = bcdToDec(RTC_Hour[4]);
	time->tm_mon  = bcdToDec(RTC_Hour[5] & DS3231_MONTH_MASK) - 1;
	time->tm_year = bcdToDec(RTC_Hour[6]) + 100;
	time->tm_isdst = 0;

	return true;

}

static void initHardware(void)
{
	Board_Init();
	ciaaUARTInit();
	ciaaI2CInit();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);

	/*
	//Set RTC Hour
	struct tm Initial_Hour;

	Initial_Hour.tm_hour = 12;
	Initial_Hour.tm_min	 = 58;
	Initial_Hour.tm_sec  = 00;
	Initial_Hour.tm_mday = 15;
	Initial_Hour.tm_mon  = 6-1;
	Initial_Hour.tm_year = 2017-1900;

	ds3231_setTime(&Initial_Hour);
	*/

}

static void pausems(uint32_t t)
{
	pausems_count = t;
	while (pausems_count != 0) {
		__WFI();
	}
}

/*==================[external functions definition]==========================*/

void SysTick_Handler(void)
{
	if(pausems_count > 0) pausems_count--;
}

int main(void)
{
	initHardware();

	while (1)
	{
		uint8_t temp[3] = "\0\0\0";
		uint8_t hour[3], minutes[3], seconds[3]  = "\0\0\0";
		struct tm Current_time;

		//Get hour and print it
		if (ds3231_getTime(&Current_time)){
			hour[1]=(Current_time.tm_hour%10)+48;
			hour[0]=(Current_time.tm_hour/10)+48;
			minutes[1]=(Current_time.tm_min%10)+48;
			minutes[0]=(Current_time.tm_min/10)+48;
			seconds[1]=(Current_time.tm_sec%10)+48;
			seconds[0]=(Current_time.tm_sec/10)+48;
			dbgPrint("Son las: ");
			dbgPrint(hour);
			dbgPrint(":");
			dbgPrint(minutes);
			dbgPrint(":");
			dbgPrint(seconds);
			dbgPrint("\n");
		}
		else {
			dbgPrint("Error de lectura\n");
		}

		//Get temperature and print it
		if (ds3231_getTempInteger(&temp)){
			temp[1]=(temp[0]%10)+48;
			temp[0]=(temp[0]/10)+48;
			dbgPrint("Hay: ");
			dbgPrint(temp);
			dbgPrint(" grados centigrados\n\n");
		}
		else {
			dbgPrint("Error de lectura\n");
		}

		pausems(5000);
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
