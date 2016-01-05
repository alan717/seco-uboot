/*
 * apx_watchdog.c - driver for i.MX6 Seco Apx Watchdog
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/arch/imx-regs.h>
//--------gpio support ---------
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>

#define SECO_APX_WDT_TRIGGER	IMX_GPIO_NR(3, 25)
#define SECO_APX_WDT_ENABLE	IMX_GPIO_NR(4, 11)

#ifdef CONFIG_APX_WATCHDOG
void apx_watchdog_reset(void)
{
	//APX Watchdog - Refresh watchdog -// EIM_D25 - WDT_TRG
        gpio_set_value(SECO_APX_WDT_TRIGGER, 1);
        gpio_set_value(SECO_APX_WDT_TRIGGER, 0);
}

void apx_watchdog_init(void)
{
	//APX Watchdog - Initialization - // KEY_ROW2 -  WDT_EN
	gpio_direction_output(SECO_APX_WDT_ENABLE, 1);
	//EIM_D25 - WDT_TRG
	gpio_direction_output(SECO_APX_WDT_TRIGGER, 1);
        gpio_set_value(SECO_APX_WDT_TRIGGER, 0);
}
#endif
