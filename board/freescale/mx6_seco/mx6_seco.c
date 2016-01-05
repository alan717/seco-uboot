/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <micrel.h>
#include <asm/imx-common/mxc_i2c.h>
#include <i2c.h>
#if defined MX6Q
	#include <configs/mx6q_seco.h>
#elif defined MX6DL
	#include <configs/mx6dl_seco.h>
#elif defined MX6S
	 #include <configs/mx6solo_seco.h>
#endif



DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_47K_UP |                   \
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define I2C_PAD_CTRL    (PAD_CTL_PUS_100K_UP |                  \
        PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |   \
        PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define SPI_PAD_CTRL ( PAD_CTL_PUS_100K_UP |  \
	PAD_CTL_SPEED_MED |  PAD_CTL_DSE_40ohm  | PAD_CTL_HYS )

#define WDT_PAD_CTRL ( PAD_CTL_PUS_100K_UP |  \
        PAD_CTL_SPEED_MED |  PAD_CTL_DSE_40ohm  | PAD_CTL_HYS )

#define A62_SECO_CODE ( PAD_CTL_PUS_100K_UP |  \
        PAD_CTL_SPEED_MED |  PAD_CTL_DSE_40ohm  | PAD_CTL_HYS )

int dram_init(void)
{
	gd->ram_size = (phys_size_t)PHYS_SDRAM_SIZE;

	return 0;
}
// APX Watchdog
static iomux_v3_cfg_t const wdt_trigger_pads[] = {
        MX6_PAD_EIM_D25__GPIO_3_25   | MUX_PAD_CTRL(WDT_PAD_CTRL), //WDT_TRG
        MX6_PAD_KEY_ROW2__GPIO_4_11  | MUX_PAD_CTRL(WDT_PAD_CTRL), //WDT_EN
};
// UART 2 - Serial Debug Uart
static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
};
// UART 5 - Optional uart
static iomux_v3_cfg_t const uart5_pads[] = {
	MX6_PAD_KEY_COL1__UART5_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW1__UART5_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	//MX6_PAD_KEY_COL4__UART5_RTS | MUX_PAD_CTRL(UART_PAD_CTRL),
	//MX6_PAD_KEY_ROW4__UART5_CTS | MUX_PAD_CTRL(UART_PAD_CTRL),
};
//MMC
static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__USDHC3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__USDHC3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__USDHC3_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__USDHC3_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__USDHC3_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__USDHC3_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};
//On Board uSD
static iomux_v3_cfg_t const usdhc2_pads[] = {
        MX6_PAD_SD4_CLK__USDHC4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD4_CMD__USDHC4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD4_DAT0__USDHC4_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD4_DAT1__USDHC4_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD4_DAT2__USDHC4_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD4_DAT3__USDHC4_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

//EXT SD
static iomux_v3_cfg_t const usdhc1_pads[] = {
        MX6_PAD_SD1_CLK__USDHC1_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD1_CMD__USDHC1_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD1_DAT0__USDHC1_DAT0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD1_DAT1__USDHC1_DAT1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD1_DAT2__USDHC1_DAT2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX6_PAD_SD1_DAT3__USDHC1_DAT3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	MX6_PAD_EIM_A24__GPIO_5_4 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D19__GPIO_3_19,
};

static iomux_v3_cfg_t const lvds_pads[] = {
	MX6_PAD_GPIO_2__GPIO_1_2 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO_4__GPIO_1_4 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

// SECO_CODE [0-3]
static iomux_v3_cfg_t const a62_seco_code[] = {
        MX6_PAD_DISP0_DAT20__GPIO_5_14 | MUX_PAD_CTRL(A62_SECO_CODE),
        MX6_PAD_DISP0_DAT21__GPIO_5_15 | MUX_PAD_CTRL(A62_SECO_CODE),
	MX6_PAD_DISP0_DAT22__GPIO_5_16 | MUX_PAD_CTRL(A62_SECO_CODE),
	MX6_PAD_DISP0_DAT23__GPIO_5_17 | MUX_PAD_CTRL(A62_SECO_CODE),
};

int mx6_rgmii_rework(struct phy_device *phydev)
{
#ifdef CONFIG_KSZ9031_FIX_100MB
	/* 
	 * To speed up PHY auto negotiation process we disabled autonegotiation
	 * and fix physical speed to 100MBs.
	 */

	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, 0x2100);
#endif
      /* control data pad skew - devaddr = 0x02, register = 0x04 */
        ksz9031_phy_extended_write(phydev, 0x02, 
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW, 
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
      /* rx data pad skew - devaddr = 0x02, register = 0x05 */
        ksz9031_phy_extended_write(phydev, 0x02, 
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW, 
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
      /* tx data pad skew - devaddr = 0x02, register = 0x05 */
        ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
      /* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */

        ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x03FF);
	return 0;
}

static iomux_v3_cfg_t const enet_pads1[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__ENET_RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__ENET_RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__ENET_RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__ENET_RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__ENET_RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__ENET_RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/* RGMII reset */
	MX6_PAD_EIM_D23__GPIO_3_23		| MUX_PAD_CTRL(NO_PAD_CTRL | PAD_CTL_PUS_47K_UP),
	/* alimentazione ethernet*/
	//MX6_PAD_EIM_EB3__GPIO_2_31		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 32 - 1 - (MODE0) all */
	MX6_PAD_RGMII_RD0__GPIO_6_25		| MUX_PAD_CTRL(NO_PAD_CTRL | PAD_CTL_PUS_47K_UP),
	/* pin 31 - 1 - (MODE1) all */
	MX6_PAD_RGMII_RD1__GPIO_6_27		| MUX_PAD_CTRL(NO_PAD_CTRL | PAD_CTL_PUS_47K_UP),
	/* pin 28 - 1 - (MODE2) all */
	MX6_PAD_RGMII_RD2__GPIO_6_28		| MUX_PAD_CTRL(NO_PAD_CTRL | PAD_CTL_PUS_47K_UP),
	/* pin 27 - 1 - (MODE3) all */
	MX6_PAD_RGMII_RD3__GPIO_6_29		| MUX_PAD_CTRL(NO_PAD_CTRL | PAD_CTL_PUS_47K_UP),
	/* pin 33 - 1 - (CLK125_EN) 125Mhz clockout enabled */
	MX6_PAD_RGMII_RX_CTL__GPIO_6_24		| MUX_PAD_CTRL(NO_PAD_CTRL | PAD_CTL_PUS_47K_UP),
};

static iomux_v3_cfg_t const enet_pads2[] = {
	MX6_PAD_RGMII_RXC__ENET_RGMII_RXC 	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__ENET_RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__ENET_RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__ENET_RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__ENET_RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads1, ARRAY_SIZE(enet_pads1));
	udelay(20);
	//gpio_direction_output(IMX_GPIO_NR(2, 31), 1); /* Power on enet */

	gpio_direction_output(IMX_GPIO_NR(3, 23), 0); /* assert PHY rst */

	gpio_direction_output(IMX_GPIO_NR(6, 24), 1);
#ifdef CONFIG_KSZ9031_FIX_100MB
	gpio_direction_output(IMX_GPIO_NR(6, 25), 0); //RD0
	gpio_direction_output(IMX_GPIO_NR(6, 27), 0); //RD1
	gpio_direction_output(IMX_GPIO_NR(6, 28), 1); //RD2
	gpio_direction_output(IMX_GPIO_NR(6, 29), 1); //RD3
#else
	gpio_direction_output(IMX_GPIO_NR(6, 25), 1); //RD0
        gpio_direction_output(IMX_GPIO_NR(6, 27), 1); //RD1
        gpio_direction_output(IMX_GPIO_NR(6, 28), 1); //RD2
        gpio_direction_output(IMX_GPIO_NR(6, 29), 1); //RD3
#endif
	udelay(1000);

	gpio_set_value(IMX_GPIO_NR(3, 23), 1); /* deassert PHY rst */

	/* Need delay 100ms to exit from reset. */
	udelay(1000 * 100);

	gpio_free(IMX_GPIO_NR(6, 24));
	gpio_free(IMX_GPIO_NR(6, 25));
	gpio_free(IMX_GPIO_NR(6, 27));
	gpio_free(IMX_GPIO_NR(6, 28));
	gpio_free(IMX_GPIO_NR(6, 29));

	imx_iomux_v3_setup_multiple_pads(enet_pads2, ARRAY_SIZE(enet_pads2));
}

void enable_ethernet_rail(void)
{
	// Power on Ethernet 
	imx_iomux_v3_setup_pad(MX6_PAD_KEY_COL2__GPIO_4_10 | MUX_PAD_CTRL(ENET_PAD_CTRL));
	gpio_direction_output(IMX_GPIO_NR(4, 10), 1); 		
	gpio_set_value(IMX_GPIO_NR(4, 10), 1);
	
}

static void setup_iomux_uart(void)
{
#ifdef DEBUG_UART5
        imx_iomux_v3_setup_multiple_pads(uart5_pads, ARRAY_SIZE(uart5_pads));
#else
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
#endif
#ifdef CONFIG_ADDITIONAL_UART
	imx_iomux_v3_setup_multiple_pads(uart5_pads, ARRAY_SIZE(uart5_pads));	
#endif
}

void reset_usb_hub(void)
{
	// USB HUB CLOCK //
        imx_iomux_v3_setup_pad(MX6_PAD_NANDF_CS2__CCM_CLKO2 | MUX_PAD_CTRL(NO_PAD_CTRL));
	// Reset USB Hub //
	imx_iomux_v3_setup_pad(MX6_PAD_GPIO_17__GPIO_7_12 | MUX_PAD_CTRL(ENET_PAD_CTRL));
	gpio_direction_output(IMX_GPIO_NR(7, 12), 0);
	gpio_set_value(IMX_GPIO_NR(7, 12), 1);
	
}

static void setup_iomux_apx_watchdog(void)
{
        imx_iomux_v3_setup_multiple_pads(wdt_trigger_pads, ARRAY_SIZE(wdt_trigger_pads));
}

static struct fsl_esdhc_cfg usdhc_cfg[3] = {

	{USDHC3_BASE_ADDR},
	{USDHC4_BASE_ADDR},
	{USDHC1_BASE_ADDR},

};

int board_mmc_getcd(struct mmc *mmc)
{
	return 1; /* Always present */
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;
#if defined(A62)	
	enable_ethernet_rail();
#endif
	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;
	/* scan phy 4,5,6,7 */
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);

	if (!phydev) {
		free(bus);
		return 0;
	}
	printf("using phy at %d\n", phydev->addr);
	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret) {
		printf("FEC MXC: %s:failed\n", __func__);
		free(phydev);
		free(bus);
	}
#endif
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	//MMC
	/*
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg.max_bus_width = 4;
	*/
	//====================================
	s32 status = 0;
        u32 index = 0;

        /*
         * Following map is done:
         * (U-boot device node)    (Physical Port)
         * mmc0                    eMMC on Board 
         * mmc1                    Ext SD
         */
        for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM; ++index) {
                switch (index) {
                case 0:
                        imx_iomux_v3_setup_multiple_pads(
                                usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
                        usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
                        usdhc_cfg[0].max_bus_width = 4;
                        //gpio_direction_input(USDHC3_CD_GPIO);
                        break;
		case 1:
                        imx_iomux_v3_setup_multiple_pads(
                                usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
                        usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
                        usdhc_cfg[1].max_bus_width = 4;
                        //gpio_direction_input(USDHC3_CD_GPIO);
                        break;


		case 2:
                        imx_iomux_v3_setup_multiple_pads(
                                usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
                        usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
                        usdhc_cfg[2].max_bus_width = 4;
                        //gpio_direction_input(USDHC1_CD_GPIO);
                        break;

                default:
                        printf("Warning: you configured more USDHC controllers"
                               "(%d) then supported by the board (%d)\n",
                               index + 1, CONFIG_SYS_FSL_USDHC_NUM);
                        return status;
                }

                status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
        }

        return status;

}

#if defined(CONFIG_VIDEO_IPUV3)

struct display_info_t {
        int     bus;
        int     addr;
        int     pixfmt;
        int     (*detect)(struct display_info_t const *dev);
        void    (*enable)(struct display_info_t const *dev);
        struct  fb_videomode mode;
};

static int detect_hdmi(struct display_info_t const *dev)
{
        struct hdmi_regs *hdmi  = (struct hdmi_regs *)HDMI_ARB_BASE_ADDR;
        return readb(&hdmi->phy_stat0) & HDMI_DVI_STAT;
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
        imx_enable_hdmi_phy();
}

static int detect_lvds(struct display_info_t const *dev)
{
        return 0;
}

static void do_enable_lvds(struct display_info_t const *dev)
{
	imx_iomux_v3_setup_multiple_pads(lvds_pads, ARRAY_SIZE(lvds_pads));
	gpio_direction_output(IMX_GPIO_NR(1, 2), 1); /* LVDS power On */
	gpio_direction_output(IMX_GPIO_NR(1, 4), 1); /* LVDS backlight On */
        return;
}

static struct display_info_t const displays[] = {{
        .bus    = -1,
        .addr   = -1,
        .pixfmt = IPU_PIX_FMT_RGB666,
        .detect = detect_lvds,
        .enable = do_enable_lvds,
        .mode   = {
		// Rif. 800x480 Panel UMSH-8596MD-20T
		.name           = "LDB-WVGA",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
        .bus    = -1,
        .addr   = 0,
        .pixfmt = IPU_PIX_FMT_RGB24,
        .detect = detect_hdmi,
        .enable = do_enable_hdmi,
        .mode   = {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
        .bus    = -1,
        .addr   = -1,
        .pixfmt = IPU_PIX_FMT_RGB666,
        .detect = detect_lvds,
        .enable = do_enable_lvds,
        .mode   = {
		// Rif. Panel 1024x768 Panel UMSH-8596MD-15T
		.name           = "LDB-XGA",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
        .bus    = -1,
        .addr   = -1,
        .pixfmt = IPU_PIX_FMT_RGB666,
        .detect = detect_lvds,
        .enable = do_enable_lvds,
        .mode   = {
		// Rif. 1366x768 Panel CHIMEI M156B3-LA1
		.name           = "LDB-WXGA",
		.refresh        = 59,
		.xres           = 1368,
		.yres           = 768,
		.pixclock       = 13890,
		.left_margin    = 93,
		.right_margin   = 33,
		.upper_margin   = 22,
		.lower_margin   = 7,
		.hsync_len      = 40,
		.vsync_len      = 4,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, 
};

int board_video_skip(void)
{
        int i;
        int ret;
        char const *panel = getenv("panel");

        if (!panel) {
                for (i = 0; i < ARRAY_SIZE(displays); i++) {
                        struct display_info_t const *dev = displays+i;
                        if (dev->detect(dev)) {
                                panel = dev->mode.name;
                                printf("auto-detected panel %s\n", panel);
                                break;
                        }
                }
                if (!panel) {
                        panel = displays[0].mode.name;
                        printf("No panel detected: default to %s\n", panel);
                        i = 0;
                }
        } else {
                for (i = 0; i < ARRAY_SIZE(displays); i++) {
                        if (!strcmp(panel, displays[i].mode.name))
                                break;
                }
        }
        if (i < ARRAY_SIZE(displays)) {
                ret = ipuv3_fb_init(&displays[i].mode, 0,
                                    displays[i].pixfmt);
                if (!ret) {
                        displays[i].enable(displays+i);
                        printf("Display: %s (%ux%u)\n",
                               displays[i].mode.name,
                               displays[i].mode.xres,
                               displays[i].mode.yres);
                } else {
                        printf("LCD %s cannot be configured: %d\n",
                               displays[i].mode.name, ret);
                }
        } else {
                printf("unsupported panel %s\n", panel);
                ret = -EINVAL;
        }
        return (0 != ret);
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB0,IPU DI0 clocks */
	reg = __raw_readl(&mxc_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 |MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3<<MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
		|IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
		|IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
		|IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
		|IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
		|IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
		|IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
		|IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
		|IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK
			|IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
		| (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
		<<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}
#endif /* CONFIG_VIDEO_IPUV3 */
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

/* I2C1, MSP430 */
struct i2c_pads_info i2c_pad_info0 = {
        .scl = {
                .i2c_mode = MX6_PAD_EIM_D21__I2C1_SCL | PC,
                .gpio_mode = MX6_PAD_EIM_D21__GPIO_3_21 | PC,
                .gp = IMX_GPIO_NR(3, 21)
        },
        .sda = {
                .i2c_mode = MX6_PAD_EIM_D28__I2C1_SDA | PC,
                .gpio_mode = MX6_PAD_EIM_D28__GPIO_3_28 | PC,
                .gp = IMX_GPIO_NR(3, 28)
        }
};

#define SECO_CODE_1 IMX_GPIO_NR(5, 14)
#define SECO_CODE_2 IMX_GPIO_NR(5, 15)
#define SECO_CODE_3 IMX_GPIO_NR(5, 16)
#define SECO_CODE_4 IMX_GPIO_NR(5, 17)

int scode_value (int seco_code) {

	gpio_direction_input(seco_code);
	return gpio_get_value(seco_code);
}

static void get_seco_board_revision(void)
{

	int ret = 0;
	unsigned char boardrev[2];
#if defined(A62)
	imx_iomux_v3_setup_multiple_pads(a62_seco_code, ARRAY_SIZE(a62_seco_code));
	//A62 board codified as 0x01
	gd->bd->board_revision[0] = 0x01; 
	//A62 board revision
	if (scode_value(SECO_CODE_1) == 1 &&
	    scode_value(SECO_CODE_2) == 1 && 
	    scode_value(SECO_CODE_3) == 1 &&
	    scode_value(SECO_CODE_4) == 1) {	
        		gd->bd->board_revision[1] = 0x00; //REV A 
	} else if (scode_value(SECO_CODE_1) == 0 && 
		   scode_value(SECO_CODE_2) == 0 &&
		   scode_value(SECO_CODE_3) == 0 &&
		   scode_value(SECO_CODE_4) == 0) {
			gd->bd->board_revision[1] = 0x10; // REV B
	}
        gd->bd->board_revision[2] = 0x00;
        gd->bd->board_revision[3] = 0x00;
#else
	#if defined(CONFIG_I2C_MXC)
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE, &i2c_pad_info0);
	//setup_i2c(CONFIG_SYS_I2C_PORT);
        /* SECO BOARD REV */
        i2c_probe (0x40);
        if (i2c_read (0x40, 0x0E, 1, &boardrev, 2)) {
                printf ("Read Board Revision Failed\n");
        } else {
                gd->bd->board_revision[0] = boardrev[1];
                gd->bd->board_revision[1] = boardrev[0];
        }
        if (i2c_read (0x40, 0x0C, 1, &boardrev, 2)) {
                printf ("Read Board Revision Failed\n");
        } else {
                gd->bd->board_revision[2] = boardrev[1];
                gd->bd->board_revision[3] = boardrev[0];
        }
	#endif
#endif
	return;

}

#ifdef CONFIG_MXC_SPI
#ifdef QSEVEN
iomux_v3_cfg_t const ecspi1_pads[] = {
        /* SS1 */
        //MX6_PAD_EIM_D19__ECSPI1_SS1  | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D19__GPIO_3_19   | MUX_PAD_CTRL(SPI_PAD_CTRL), //CS0 BUS 0 -> eeprom
	MX6_PAD_EIM_A21__GPIO_2_17   | MUX_PAD_CTRL(SPI_PAD_CTRL), //CS1 BUS 0 -> pcf2123
        MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
};
#elif defined(UQSEVEN) || defined(A62)

iomux_v3_cfg_t const ecspi1_pads[] = {
        /* SS1 */
        MX6_PAD_EIM_D19__GPIO_3_19   | MUX_PAD_CTRL(SPI_PAD_CTRL), //CS0 BUS 0 -> eeprom
        MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
};

iomux_v3_cfg_t const ecspi3_pads[] = {
        /* SS3 */
        MX6_PAD_DISP0_DAT3__GPIO_4_24   | MUX_PAD_CTRL(SPI_PAD_CTRL), //CS0 BUS 3 -> pcf2123
        MX6_PAD_DISP0_DAT2__ECSPI3_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_DISP0_DAT1__ECSPI3_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_DISP0_DAT0__ECSPI3_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
};
#elif uSBC
	
iomux_v3_cfg_t const ecspi1_pads[] = {
        /* SS1 */
        MX6_PAD_EIM_D19__GPIO_3_19   | MUX_PAD_CTRL(SPI_PAD_CTRL), //CS0 BUS 0 -> eeprom
        MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
};

iomux_v3_cfg_t const ecspi3_pads[] = {
        /* SS3 */
        MX6_PAD_DISP0_DAT3__GPIO_4_24   | MUX_PAD_CTRL(SPI_PAD_CTRL), //CS0 BUS 3 -> pcf2123
        MX6_PAD_DISP0_DAT2__ECSPI3_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_DISP0_DAT1__ECSPI3_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
        MX6_PAD_DISP0_DAT0__ECSPI3_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
};

#endif


void setup_spi(void)
{
	u32 reg;
#ifdef QSEVEN
	/* Setup SS1 */
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads,
                                         ARRAY_SIZE(ecspi1_pads));
	/* Enable SPI1 clock*/
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR1);
	reg |= 0x30;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR1);
#elif defined(UQSEVEN) || defined(A62)
	/* Setup SS1 */
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads,
                                         ARRAY_SIZE(ecspi1_pads));
	
	/* Setup SS3 */
	imx_iomux_v3_setup_multiple_pads(ecspi3_pads,
                                         ARRAY_SIZE(ecspi3_pads));
	/* Enable SPI3 clock*/
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR1);
	reg |= 0x30;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR1);
#elif uSBC
	/* Setup SS1 */
        imx_iomux_v3_setup_multiple_pads(ecspi1_pads,
                                         ARRAY_SIZE(ecspi1_pads));

        /* Setup SS3 */
        imx_iomux_v3_setup_multiple_pads(ecspi3_pads,
                                         ARRAY_SIZE(ecspi3_pads));
        /* Enable SPI3 clock*/
        reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR1);
        reg |= 0x30;
        writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR1);
	
#endif 
	
}
#ifdef CONFIG_MXC_SPI_CSHOLD
/*
 * This function translate cs request in
 * a gpio request. This translation is necessary
 * for 'decode_cs' function in 'mxc_spi' to implement
 * the single burst SPI transaction.
 *
 */
int decode_cshold(int bus, int cs)
{
#ifdef QSEVEN
	switch (bus){
		case 0:
			switch (cs){
				case 0:
					return ( cs | IMX_GPIO_NR(3,19)<<8); // SPI eeprom 
					break;
				case 1:
					return ( cs | IMX_GPIO_NR(2,17)<<8); // SPI rtc low power
                                        break;
				default:
					return cs;
					break;
		
			}
			break;
		default:
			return cs;
			break;
	}
#elif defined(UQSEVEN) || defined(A62)
	switch (bus){
                case 0:
                        switch (cs){
                                case 0:
                                        return ( cs | IMX_GPIO_NR(3,19)<<8); // SPI eeprom 
                                        break;
                                default:
                                        return cs;
                                        break;

                        }
                        break;
		case 2:
			switch (cs){
                                case 0:
                                        return ( cs | IMX_GPIO_NR(4,24)<<8); // RTC pcf2123 
                                        break;
                                default:
                                        return cs;
                                        break;

                        }
                        break;	
                default:
                        return cs;
                        break;
        }
#elif uSBC
	switch (bus){
                case 0:
                        switch (cs){
                                case 0:
                                        return ( cs | IMX_GPIO_NR(3,19)<<8); // SPI eeprom 
                                        break;
                                default:
                                        return cs;
                                        break;

                        }
                        break;
                case 2:
                        switch (cs){
                                case 0:
                                        return ( cs | IMX_GPIO_NR(4,24)<<8); // RTC pcf2123 
                                        break;
                                default:
                                        return cs;
                                        break;

                        }
                        break;
                default:
                        return cs;
                        break;
        }

#endif 
}
#endif 
#endif

#ifdef CONFIG_CMD_SATA
int setup_sata(void)
{
        struct iomuxc_base_regs *const iomuxc_regs
                = (struct iomuxc_base_regs *) IOMUXC_BASE_ADDR;
        int ret = enable_sata_clock();
        if (ret)
                return ret;

        clrsetbits_le32(&iomuxc_regs->gpr[13],
                        IOMUXC_GPR13_SATA_MASK,
                        IOMUXC_GPR13_SATA_PHY_8_RXEQ_3P0DB
                        |IOMUXC_GPR13_SATA_PHY_7_SATA2M
                        |IOMUXC_GPR13_SATA_SPEED_3G
                        |(3<<IOMUXC_GPR13_SATA_PHY_6_SHIFT)
                        |IOMUXC_GPR13_SATA_SATA_PHY_5_SS_DISABLED
                        |IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_9_16
                        |IOMUXC_GPR13_SATA_PHY_3_TXBOOST_0P00_DB
                        |IOMUXC_GPR13_SATA_PHY_2_TX_1P104V
                        |IOMUXC_GPR13_SATA_PHY_1_SLOW);

        return 0;
}
#endif

#ifdef ENV_MMC
/* This is a switch beetwen usdhc controller.
 * The function get_mmc_env_devno() return
 * the usdhc boot controller and the switch/case 
 * find the right device number (devno) that is
 * the registering order of usdhc controller in
 * function 'int board_mmc_init(bd_t *bis)' in 
 * file mx6_seco.c 				*/
void mmc_environment_init(){
#ifdef CONFIG_DYNAMIC_MMC_DEVNO
#if defined QSEVEN
        switch (get_mmc_env_devno()) {
                case 0:
			if(board_is_booting_from_spi()){
				gd->mmc_env_devno = 0;
				/* Booting from SPI */
			}
			else {
				gd->mmc_env_devno = 2;
				/* Booting from Ext SD */
 			}			
                        break;
                case 2:
                        gd->mmc_env_devno = 0;
			/* Booting from eMMC */
                        break;
                case 3:
                        gd->mmc_env_devno = 1;
			/* Booting from uSD */
                        break;
                default:
                        gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
                        break;

        }
#elif defined UQSEVEN
        switch (get_mmc_env_devno()) {
		case 0:
			if(board_is_booting_from_spi()){
                                gd->mmc_env_devno = 2;
				/* Booting from SPI */
                        }
			else
				gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
			break;
                case 2:
                        gd->mmc_env_devno = 0;
			/* Booting from eMMC */
                        break;
                case 3:
                        gd->mmc_env_devno = 1;
			/* Booting from SD */
                        break;
                default:
                        gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
                        break;

        }
#elif defined uSBC
        switch (get_mmc_env_devno()) {
		case 0:
			if(board_is_booting_from_spi()){
                                gd->mmc_env_devno = 2;
				/* Booting from SPI */
                          }
                        else
                                gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
                        break;
                case 2:
                        gd->mmc_env_devno = 0;
			/* Booting from eMMC */
                        break;
                default:
                        gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
                        break;

        }
#elif defined A62
        switch (get_mmc_env_devno()) {
                case 0:
                        if(board_is_booting_from_spi()){
                                gd->mmc_env_devno = 2;
                                /* Booting from SPI */
                          }
                        else
                                gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
                        break;
                case 2:
                        gd->mmc_env_devno = 0;
                        /* Booting from eMMC */
                        break;
		case 3:
                        gd->mmc_env_devno = 1;
                        /* Booting from Onboard uSD */
                        break;
                default:
                        gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
                        break;

        }
#endif
#else
	gd->mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV; /*return eMMC devno*/
#endif
}
#endif
/*
 * Show device feature strings on current display
 * around uDOO Logo.
 */
/*
void show_boot_messages(void) 
{
	int i;
	ulong cycles = 0;
	int repeatable;
	char *plotmsg_cmd[2];
#if defined(CONFIG_MX6DL)
	char *boot_messages[7] = {
"SECO Board 2013",
"CPU Freescale i.MX6 DualLite 1GHz",
"dual ARMv7 Cortex-A9 core",
"1GB RAM DDR3",
"Vivante GC880 GPU",
};
#else
	char *boot_messages[7] = {
"SECO Board 2013",
"CPU Freescale i.MX6 Quad/Dual 1GHz",
"quad/dual ARMv7 Cortex-A9 core",
"1GB RAM DDR3",
"Vivante GC2000 / GC880",
};
#endif

	for (i=0; i<7; i++) {
		plotmsg_cmd[0] = "plotmsg";
		plotmsg_cmd[1] = boot_messages[i];
		cmd_process(0, 2, plotmsg_cmd, &repeatable, &cycles);
	}
}
*/


static void print_boot_device(void){

	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	uint bt_mem_ctl = (soc_sbmr & 0x000000F0) >> 4 ;
	uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;

	uint bt_usdhc_num = (soc_sbmr & 0x00001800) >> 11;
	uint bt_port_select = (soc_sbmr & 0x05000000) >> 24; 

        switch (bt_mem_ctl) {
        case 0x0:
                if (bt_mem_type)
                        printf("Boot: One Nand\n");
                else
                        printf("Boot: Weim Nor\n");
                break;
        case 0x2:
		printf("Boot:  Sata\n");
                break;
        case 0x3:
    
                if (bt_port_select == 0)  
                        printf("Boot:  Spi Nor\n");
                else
                        printf("Boot:  i2c\n");
                break;
   
        case 0x4:
#ifdef A62
                printf("Boot:  Onboard uSD\n");
		break;
#endif
        case 0x5:
  
                if(bt_usdhc_num == 0){
                   	 printf("Boot:  Ext SD\n");
			 break;
		}
                if(bt_usdhc_num == 3){
#ifdef QSEVEN 
                printf("Boot:  Onboard uSD\n");
#endif

#ifdef UQSEVEN 
                printf("Boot:  Ext SD\n");
#endif

#ifdef uSBC 
                printf("Boot:  eMMC\n");
#endif

		break;
		}
	case 0x6:
        case 0x7:
		printf("Boot:  eMMC\n");               
                break;
        case 0x8 ... 0xf:
                printf("Boot:  Nand\n");
                break;
        default:
                printf("Boot:  Unknown\n");
                break;
        }

}

int board_early_init_f(void)
{
#if defined(CONFIG_VIDEO_IPUV3)
        setup_display();
#endif
#if defined(CONFIG_APX_WATCHDOG)
	setup_iomux_apx_watchdog();
#endif
	setup_iomux_uart();
#if A62
	reset_usb_hub();
#endif
	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
#if defined(CONFIG_VIDEO_IPUV3)
//	show_boot_messages();
#endif
        return 1;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
        /* 4 bit bus width */
        {"sd2",  MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
        {"sd3",  MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
        /* 8 bit bus width */
        {"emmc", MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
        {NULL,   0},
};
#endif

int board_init(void)
{

	 u32 reg;
        /* address of boot parameters */
        gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
 	/* boot validate */
#ifdef QSEVEN

	#ifdef CONFIG_MX6Q
        imx_iomux_v3_setup_pad(MX6_PAD_SD4_DAT5__GPIO_2_13 | MUX_PAD_CTRL(NO_PAD_CTRL) );
	#elif defined CONFIG_MX6DL
        imx_iomux_v3_setup_pad(MX6_PAD_SD4_DAT5__GPIO_2_13 | MUX_PAD_CTRL(NO_PAD_CTRL) );
	#endif

	gpio_direction_output(IMX_GPIO_NR(2, 13), 0);

        /* Set Low */
	gpio_set_value(IMX_GPIO_NR(2, 13), 0);
    	udelay(1000);

        /* Set High */
	gpio_set_value(IMX_GPIO_NR(2, 13), 1);

#elif defined (UQSEVEN) || defined (uSBC)

	#ifdef CONFIG_MX6Q
        imx_iomux_v3_setup_pad(MX6_PAD_NANDF_D4__GPIO_2_4 | MUX_PAD_CTRL(NO_PAD_CTRL) );
        #elif defined CONFIG_MX6DL
        imx_iomux_v3_setup_pad(MX6_PAD_NANDF_D4__GPIO_2_4 | MUX_PAD_CTRL(NO_PAD_CTRL) );
        #endif

        gpio_direction_output(IMX_GPIO_NR(2, 4), 0);

        /* Set Low */
        gpio_set_value(IMX_GPIO_NR(2, 4), 0);
        udelay(1000);

        /* Set High */
        gpio_set_value(IMX_GPIO_NR(2, 4), 1);

#endif

	/* board id for linux */
#if defined(CONFIG_MX6Q_SECO_Q7) || defined(CONFIG_MX6DL_SECO_Q7) || defined(CONFIG_MX6S_SECO_Q7)
        gd->bd->bi_arch_number = MACH_TYPE_MX6_SECO_Q7;
#elif defined(CONFIG_MX6Q_SECO_UQ7) || defined(CONFIG_MX6DL_SECO_UQ7) || defined(CONFIG_MX6S_SECO_UQ7)
        gd->bd->bi_arch_number = MACH_TYPE_MX6_SECO_UQ7;
#elif defined(CONFIG_MX6Q_SECO_uSBC) || defined(CONFIG_MX6DL_SECO_uSBC) || defined(CONFIG_MX6S_SECO_uSBC)
        gd->bd->bi_arch_number = MACH_TYPE_MX6_SECO_uSBC;
#elif defined(CONFIG_MX6Q_SECO_A62) || defined(CONFIG_MX6DL_SECO_A62) || defined(CONFIG_MX6S_SECO_A62)
        gd->bd->bi_arch_number = MACH_TYPE_MX6_SECO_A62;
#else
        gd->bd->bi_arch_number = MACH_TYPE_MX6Q_SECO_Q7;
#endif
	
#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif
#ifdef ENV_MMC
	mmc_environment_init();
#endif
#ifdef CONFIG_MXC_SPI
        setup_spi();
#endif
	print_boot_device();

        return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
        add_board_boot_modes(board_boot_modes);
#endif
	get_seco_board_revision();

        return 0;
}
int checkboard(void)
{
#ifdef QSEVEN
	puts("Board: SECO Q7\n");
#elif UQSEVEN
	puts("Board: SECO uQ7\n");	
#elif uSBC
	puts("Board: SECO uSBC\n");
#elif A62
        puts("Board: SECO A62\n");
#endif
	return 0;
}
