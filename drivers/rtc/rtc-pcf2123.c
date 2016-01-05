/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 */
#include <common.h>
#include <command.h>
#include <spi_flash.h>

#include <asm/io.h>
#include <spi.h>
#include <rtc.h>
#include <linux/time.h>


#define MAX_SPI_BYTES_RTC 		12     /* Maximum number of bytes we can handle */
#define TIME				4
#define DATE				4
#define RESET				2
#define YEAR				2
#define OS_FLAG				2
#define DATE_AND_TIME			8

#define MAX_HZ 				1000000
#define SPI_MODE			SPI_CS_HIGH
#if defined (CONFIG_MX6Q_SECO_Q7) || defined (CONFIG_MX6DL_SECO_Q7) || defined (CONFIG_MX6S_SECO_Q7)
#define CS				1 /* Aligned this value with Platform decode_cshold function! */
#define BUS				0
#elif defined (CONFIG_MX6Q_SECO_UQ7) || defined (CONFIG_MX6DL_SECO_UQ7) || defined (CONFIG_MX6S_SECO_UQ7) || defined (CONFIG_MX6Q_SECO_A62) || defined (CONFIG_MX6DL_SECO_A62) || defined (CONFIG_MX6S_SECO_A62)
#define CS                              0
#define BUS                             2
#elif defined (CONFIG_MX6Q_SECO_uSBC) || defined (CONFIG_MX6DL_SECO_uSBC) || defined (CONFIG_MX6S_SECO_uSBC)
#define CS                              0
#define BUS                             2
#endif

#define CMD_READ			0x03

#define PCF2123_REG_CTRL1       (0x00)  /* Control Register 1 */
#define PCF2123_REG_CTRL2       (0x01)  /* Control Register 2 */
#define PCF2123_REG_SC          (0x02)  /* datetime */
#define PCF2123_REG_MN          (0x03)
#define PCF2123_REG_HR          (0x04)
#define PCF2123_REG_DM          (0x05)
#define PCF2123_REG_DW          (0x06)
#define PCF2123_REG_MO          (0x07)
#define PCF2123_REG_YR          (0x08)

#define PCF2123_SUBADDR         (1 << 4)
#define PCF2123_WRITE           ((0 << 7) | PCF2123_SUBADDR)
#define PCF2123_READ            ((1 << 7) | PCF2123_SUBADDR)

unsigned char           time[TIME];
unsigned char 		date[DATE];
unsigned char		year[YEAR];
u64			reg_imx6;

int rtc_set(struct rtc_time *tm)
{
	struct spi_slave *spi;
        int     j,ret;
        unsigned char cmd[8],time[8];
	unsigned int CSh;

	/* Decode CS SPI chip select */
	CSh = decode_cshold(BUS,CS);	
	/* Setup spi device */
	spi = spi_setup_slave (BUS, CSh, MAX_HZ, SPI_MODE);
        if(!spi){
                printf("Error in setup slave!");
                return -1;
        }

        ret = spi_claim_bus(spi);
        if( ret < 0 ){
                printf("Error Claiming Bus!\n");
                printf("Release Bus and Claiming Again");
                spi_release_bus(spi);
                printf("\nClaiming Again");
                ret = spi_claim_bus(spi);
                if(!ret){
                        printf("\nError Claiming Bus!\n");
                        return ret;
                }
        }

        /*WRITE TIME OPERATION*/

	cmd[0] = PCF2123_WRITE | PCF2123_REG_SC;
	cmd[1] = tm->tm_sec;
	cmd[2] = tm->tm_min;
	cmd[3] = tm->tm_hour;
	cmd[4] = tm->tm_mday;
	cmd[5] = tm->tm_wday;
	cmd[6] = tm->tm_mon;
	cmd[7] = tm->tm_year;	

        for (j = 0; j < DATE_AND_TIME; j++){
        time[j] = 0x0;
        }

	if(spi_flash_cmd_write(spi, cmd, DATE_AND_TIME, time, sizeof(time)) ) {
		printf("Error with the SPI transaction.\n");
		return -1;
	}	

	spi_release_bus(spi);
        spi_free_slave(spi);

	return 0;
}

int rtc_get(struct rtc_time *tm){

 	struct spi_slave *spi;
        int     j,ret;
        unsigned char cmd[MAX_SPI_BYTES_RTC],time[7];
        int counter;
	unsigned int CSh;

	/* Decode CS SPI chip select */
        CSh = decode_cshold(BUS,CS);    
        /* Setup spi device */
	spi = spi_setup_slave (BUS, CSh, MAX_HZ, SPI_MODE);
	if(!spi){
		printf("Error in setup slave!");
		return -1;
	}
	
	ret = spi_claim_bus(spi);
	if( ret < 0 ){
		printf("Error Claiming Bus!\n");
        	printf("Release Bus and Claiming Again");
		spi_release_bus(spi);
		printf("\nClaiming Again");
		ret = spi_claim_bus(spi);
		if(!ret){
			printf("\nError Claiming Bus!\n");
			return ret;
		}
	}

	/* Set Direction */
	//spi_cs3_direction_output();

	/*READ TIME OPERATION*/	

        for (j = 0; j < TIME; j++){
        time[j] = 0x0;
        }		
	
	if(spi_flash_cmd(spi, PCF2123_READ | PCF2123_REG_SC, time, sizeof(time))){
		printf("Error with the SPI transaction.\n");
		return -1;
	}
	
	tm->tm_sec     = bcd2bin(time[0] & 0x7f);
        tm->tm_min     = bcd2bin(time[1] & 0x7f);
        tm->tm_hour    = bcd2bin(time[2] & 0x3f);

	/*READ DATE OPERATION*/

	for (j = 0; j < DATE; j++){
        date[j] = 0x0;
        }

	if(spi_flash_cmd(spi, PCF2123_READ | PCF2123_REG_DM, date, sizeof(date))){
                printf("Error with the SPI transaction.\n");
                return -1;
        }

	/*READ YEAR OPERATION*/

        for (j = 0; j < YEAR; j++){
        year[j] = 0x0;
        }

	if(spi_flash_cmd(spi, PCF2123_READ | PCF2123_REG_YR, year, sizeof(year))){
                printf("Error with the SPI transaction.\n");
                return -1;
        }

	spi_release_bus(spi);
        spi_free_slave(spi);

		tm->tm_mday    = bcd2bin(date[0] & 0x3f);
		tm->tm_wday    = date[1] & 0x07;
		tm->tm_mon     = bcd2bin(date[2] & 0x1f);
		tm->tm_year    = bcd2bin(year[0]);
		if (tm->tm_year < 38)
        	      	tm->tm_year += 2000;
		else
			tm->tm_year += 1900;
        	 tm->tm_yday = 0;
               	 tm->tm_isdst = 0;
	
	return 0;	


}
void rtc_reset(void){

	struct spi_slave *spi;
        int     j,ret;
        unsigned char cmd[RESET],reset[RESET];
	unsigned int CSh;
	
	/* Decode CS SPI chip select */
        CSh = decode_cshold(BUS,CS);    
        /* Setup spi device */	
	spi = spi_setup_slave (BUS, CSh, MAX_HZ, SPI_MODE);
	
	if(!spi){
		printf("Error in setup slave!");
		return;
	}
	
	ret = spi_claim_bus(spi);
	if( ret < 0 ){
		printf("Error Claiming Bus!\n");
        	printf("Release Bus and Claiming Again");
		spi_release_bus(spi);
		printf("\nClaiming Again");
		ret = spi_claim_bus(spi);
		if(!ret){
			printf("\nError Claiming Bus!\n");
			return;
		}
	}

	/*SOFTWARE RESET*/

	cmd[0] = PCF2123_WRITE | PCF2123_REG_CTRL1;
	cmd[1] = 0x58;	

        for (j = 0; j < RESET; j++){
        reset[j] = 0x0;
        }	

	if(spi_flash_cmd_write(spi, cmd, RESET, reset, sizeof(reset)) ) {
                printf("Error with the SPI transaction.\n");
                return -1;
        }	
	
	spi_release_bus(spi);
        spi_free_slave(spi);


}

int rtc_read_os_flag(int *os_flag){

 	struct spi_slave *spi;
        int     j,ret;
        unsigned char cmd[MAX_SPI_BYTES_RTC];
	unsigned char ctrl_os_flag[OS_FLAG];
	unsigned int CSh;
	
	/* Decode CS SPI chip select */
        CSh = decode_cshold(BUS,CS);    
        /* Setup spi device */
	spi = spi_setup_slave (BUS, CSh, MAX_HZ, SPI_MODE);
	if(!spi){
		printf("Error in setup slave!");
		return -1;
	}
	
	ret = spi_claim_bus(spi);
	if( ret < 0 ){
		printf("Error Claiming Bus!\n");
        	printf("Release Bus and Claiming Again");
		spi_release_bus(spi);
		printf("\nClaiming Again");
		ret = spi_claim_bus(spi);
		if(!ret){
			printf("\nError Claiming Bus!\n");
			return ret;
		}
	}


	/*READ OS FLAG */
        for (j = 0; j < OS_FLAG; j++){
        ctrl_os_flag[j] = 0x0;
        }

	if(spi_flash_cmd(spi, PCF2123_READ | PCF2123_REG_SC, ctrl_os_flag, sizeof(ctrl_os_flag))){
		printf("Error with the SPI transaction.\n");
                return -1;
	}
				
	spi_release_bus(spi);
        spi_free_slave(spi);
	*os_flag = (ctrl_os_flag[0] >> 7);	

	return 0;	



}
