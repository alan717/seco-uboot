/*
 * Copyright (C) 2013 Seco S.r.l
 *
 * Configuration settings for the Seco Boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>
#include <asm/sizes.h>

#define CONFIG_MX6
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

//===== CONFIG MACH TYPE AND BOARD ======//
#ifdef QSEVEN
	#define CONFIG_MX6DL_SECO_Q7
	#define CONFIG_MACH_TYPE	MACH_TYPE_MX6_SECO_Q7
#endif
#ifdef UQSEVEN
	#define CONFIG_MX6DL_SECO_UQ7
	#define CONFIG_MACH_TYPE 	MACH_TYPE_MX6_SECO_UQ7
#endif
#ifdef uSBC
	#define CONFIG_MX6DL_SECO_uSBC
	#define CONFIG_MACH_TYPE 	MACH_TYPE_MX6_SECO_uSBC
#endif
#ifdef A62
        #define CONFIG_MX6DL_SECO_A62
        #define CONFIG_MACH_TYPE        MACH_TYPE_MX6_SECO_A62
	/* APX WATCHDOG */
	#define CONFIG_APX_WATCHDOG
	#define CONFIG_HW_WATCHDOG
#endif

#ifdef EXTRA_UART
#define CONFIG_ADDITIONAL_UART
#endif
//===== CONFIG_DDR_SIZE ======//
/* CONFIG_DDR_SIZE:
                0 ->  512M,  bus size 32, active CS = 1 (256Mx2);
                1 ->  1Giga, bus size 32, active CS = 1 (512Mx2);
                2 ->  1Giga, bus size 64, active CS = 1 (256Mx4);
                3 ->  2Giga, bus size 64, active CS = 1 (512Mx4);
                4 ->  4Giga, bus size 64, active CS = 2 (512Mx8);
*/

/* CONFIG_DDR_TYPE:     
                0 ->  DDR3
                1 ->  LDDR3
*/
#define CONFIG_DDR_SIZE DDR_SIZE_CONF

#define CONFIG_DDR_TYPE DDR_TYPE_CONF

#if(CONFIG_DDR_SIZE == 0)
        #define CONFIG_DDR_32BIT
#elif(CONFIG_DDR_SIZE > 0)
        #define CONFIG_DDR_64BIT
#endif
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_BOARD_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_MXC_GPIO

#define CONFIG_MXC_UART
#ifdef DEBUG_UART5
        #define CONFIG_MXC_UART_BASE            UART5_BASE
#else
        #define CONFIG_MXC_UART_BASE            UART2_BASE
#endif

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200
/*
 * I2C Configs
 */
#define CONFIG_CMD_I2C
#ifdef CONFIG_CMD_I2C
        #define CONFIG_HARD_I2C         1
        #define CONFIG_I2C_MXC          1
        #define CONFIG_SYS_I2C_BASE             I2C1_BASE_ADDR
        #define CONFIG_SYS_I2C_SPEED            100000
        #define CONFIG_SYS_I2C_SLAVE            0x8
#endif

/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_IMLS

#define CONFIG_CMD_BMODE
#define CONFIG_CMD_SETEXPR

#define CONFIG_BOOTDELAY		3

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 500 * SZ_1M)
#define CONFIG_LOADADDR			0x12000000
#define CONFIG_SYS_TEXT_BASE		0x17800000

/* MMC Configuration */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#if defined QSEVEN
	#define CONFIG_SYS_FSL_USDHC_NUM	3
#elif defined UQSEVEN
	#define CONFIG_SYS_FSL_USDHC_NUM        2
#elif defined uSBC
	#define CONFIG_SYS_FSL_USDHC_NUM        1
#elif defined A62
        #define CONFIG_SYS_FSL_USDHC_NUM        2
#endif
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

/*
 * SPI Configs
 */
#define CONFIG_CMD_SF
#ifdef CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SST
#define CONFIG_MXC_SPI_CSHOLD
#define CONFIG_MXC_SPI
#define CONFIG_SF_DEFAULT_BUS  0
#define CONFIG_SF_DEFAULT_CS   (0|(IMX_GPIO_NR(3, 19)<<8))
#define CONFIG_SF_DEFAULT_SPEED 2500000
#define CONFIG_SF_DEFAULT_MODE (SPI_MODE_0)
/*
 * RTC Low Power
 */
#ifdef RTC_LOW_POWER
        #define CONFIG_CMD_RTC
        #define CONFIG_RTC_PCF2123
        #define CONFIG_RTC_SNVS
#endif
#endif

/* Ethernet Configuration */
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		6
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL
#define CONFIG_KSZ9031_FIX_100MB

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORT     1
#define CONFIG_MXC_USB_PORTSC   (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS    0

/* Framebuffer */
//#define CONFIG_VIDEO
//#define CONFIG_VIDEO_IPUV3
//#define CONFIG_CFB_CONSOLE
//#define CONFIG_VGA_AS_SINGLE_DEVICE
//#define CONFIG_SYS_CONSOLE_IS_IN_ENV
//#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
//#define CONFIG_VIDEO_BMP_RLE8
//#define CONFIG_SPLASH_SCREEN
//#define CONFIG_SPLASH_SCREEN_ALIGN
//#define CONFIG_BMP_16BPP
//#define CONFIG_VIDEO_LOGO
//#define CONFIG_VIDEO_BMP_LOGO
//#define CONFIG_IPUV3_CLK 260000000
//#define CONFIG_IMX_HDMI

#if defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#define CONFIG_DEFAULT_FDT_FILE		"imx6dl-seco.dtb"
#elif defined(CONFIG_MX6Q)
#define CONFIG_DEFAULT_FDT_FILE		"imx6q-seco.dtb"
#endif

#define CONFIG_CMD_SECOQ7CONFIG

#define CONFIG_EXTRA_ENV_SETTINGS                                       \
                "netdev=eth0\0"                                         \
                "ethprime=FEC0\0"                                       \
                "netdev=eth0\0"                                         \
                "ethprime=FEC0\0"                                       \
                "uboot=u-boot.bin\0"                    \
		"ethaddr=00:11:22:33:44:55\0"		\
                "kernel=uImage\0"                               \
                "nfsroot=/opt/eldk/arm\0"                               \
		"splashpos=325,300\0" 					\
                "ip_local=10.0.0.5::10.0.0.1:255.255.255.0::eth0:off\0" \
                "serverip=10.0.0.1\0"                                  \
		"ipaddr=10.0.0.5\0"                                  \
                "nfs_path=/targetfs \0"                                 \
                "memory=mem=" ENV_SDRAM_SIZE "\0"			\
                "bootdev=mmc dev 0; ext2load mmc 0:1\0"                 \
                "root=root=/dev/mmcblk0p1\0"                            \
                "option=rootwait rw fixrtc rootflags=barrier=1\0"       \
                "setvideomode=setenv videomode video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24 setenv video1 video=mxcfb1:dev=ldb,${lvdsres1}\0"  \
                "lvdsres1=LDB-WXGA,if=RGB666\0"                          \
                "cpu_freq=arm_freq=996\0"                               \
                "setbootargs=setenv bootargs console=ttymxc1,115200 ${root} ${option} ${memory} ${cpu_freq} ${videomode}\0"     \
                "setbootargs_nfs=setenv bootargs console=ttymxc1,115200 root=/dev/nfs  nfsroot=${serverip}:${nfs_path} nolock,wsize=4096,rsize=4096  ip=:::::eth0:dhcp  ${memory} ${cpu_freq} ${videomode}\0" \
                "setbootdev=setenv boot_dev ${bootdev} 10800000 /boot/uImage\0"                 \
                "bootcmd=run setvideomode; run setbootargs_nfs; tftpboot 0x18000000 /targetfs/boot/${kernel}; bootm 0x18000000\0"                     \
                "stdin=serial\0"                                        \
                "stdout=serial\0"                                       \
                "stderr=serial\0"


/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
//============= CONSOLE PROMPT ==========================================//
#ifdef QSEVEN
	#define CONFIG_SYS_PROMPT	       "MX6DL SECO_Q7 U-Boot > "
#elif UQSEVEN
	#define CONFIG_SYS_PROMPT              "MX6DL SECO_uQ7 U-Boot > "
#elif uSBC
	#define CONFIG_SYS_PROMPT              "MX6DL SECO_uSBC U-Boot > "
#elif A62
        #define CONFIG_SYS_PROMPT              "MX6DL SECO_A62 U-Boot > "
#endif
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	       16
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

#define CONFIG_CMDLINE_EDITING

/*=======*/
/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS     1
#define PHYS_SDRAM      		 MMDC0_ARB_BASE_ADDR

#if (CONFIG_DDR_SIZE == 0)
#define PHYS_SDRAM_SIZE       (512u * 1024 * 1024)
#define ENV_SDRAM_SIZE       "512M"
#endif

#if (CONFIG_DDR_SIZE == 1 || CONFIG_DDR_SIZE == 2)
#define PHYS_SDRAM_SIZE       (1u * 1024 * 1024 * 1024)
#define ENV_SDRAM_SIZE       "1024M"
#endif

#if (CONFIG_DDR_SIZE == 3)
#define PHYS_SDRAM_SIZE       (2u * 1024 * 1024 * 1024)
#define ENV_SDRAM_SIZE       "2048M"
#endif

#if (CONFIG_DDR_SIZE == 4)
#define PHYS_SDRAM_SIZE       (4u * 1024 * 1024 * 1024 - 256u * 1024 * 1024)
#define ENV_SDRAM_SIZE       "3800M"
#endif

#define CONFIG_SYS_SDRAM_BASE           PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR        IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE        IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
        (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
        (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SIZE                 (8 * 1024)

#ifdef ENV_MMC
	#define CONFIG_ENV_IS_IN_MMC
	#define CONFIG_ENV_OFFSET               (6 * 128 * 1024)  // 0x000C0000
	#define CONFIG_SYS_MMC_ENV_DEV          0
	#define CONFIG_DYNAMIC_MMC_DEVNO
#elif defined(ENV_SPI)
	#define CONFIG_ENV_IS_IN_SPI_FLASH	1
	#define CONFIG_ENV_OFFSET               (504 * 1024)
	#define CONFIG_ENV_SECT_SIZE            (8 * 1024)
	#define CONFIG_ENV_SPI_BUS              CONFIG_SF_DEFAULT_BUS
	#define CONFIG_ENV_SPI_CS               CONFIG_SF_DEFAULT_CS
	#define CONFIG_ENV_SPI_MODE             CONFIG_SF_DEFAULT_MODE
	#define CONFIG_ENV_SPI_MAX_HZ           CONFIG_SF_DEFAULT_SPEED
#endif

#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTZ

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

#endif			       /* __CONFIG_H * */
