/*
 * (C) Copyright 2010-2013 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <errno.h>
#include <sata.h>

char *env_name_spec = "SATA";

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr;
#endif /* ENV_IS_EMBEDDED */

extern int sata_curr_device;

/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec(int index)
{
	return *((uchar *)(gd->env_addr + index));
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV

inline int write_env(block_dev_desc_t *sata, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = sata->block_write(sata_curr_device, blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

int saveenv(void)
{
	block_dev_desc_t *sata = NULL;
	env_t	env_new;
	ssize_t	len;
	char *res;

	if (sata_curr_device == -1) {
		if (sata_initialize())
			return 1;
		sata_curr_device = CONFIG_SATA_ENV_DEV;
	}

	if (sata_curr_device >= CONFIG_SYS_SATA_MAX_DEVICE) {
		printf("Unknown SATA(%d) device for environment!\n",
			sata_curr_device);
		return 1;
	}

	sata = sata_get_dev(sata_curr_device);

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);

	printf("Writing to SATA(%d)...", sata_curr_device);
	if (write_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, &env_new)) {
		puts("failed\n");
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

inline int read_env(block_dev_desc_t *sata, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = sata->block_read(sata_curr_device, blk_start,
					blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	block_dev_desc_t *sata = NULL;
	char buf[CONFIG_ENV_SIZE];
	int ret;

	if (sata_curr_device == -1) {
		if (sata_initialize())
			return;
		sata_curr_device = CONFIG_SATA_ENV_DEV;
	}

	if (sata_curr_device >= CONFIG_SYS_SATA_MAX_DEVICE) {
		printf("Unknown SATA(%d) device for environment!\n",
			sata_curr_device);
		return;
	}
	sata = sata_get_dev(sata_curr_device);

	if (read_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, buf))
		return use_default();

	ret = env_import(buf, 1);
	if (ret)
		gd->env_valid = 1;

#endif
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	puts("*** Warning - bad CRC or MMC, using default environment\n\n");
	set_default_env(NULL);
}
#endif

