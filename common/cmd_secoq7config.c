/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <stdio_dev.h>

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>
#include <watchdog.h>

#define DEF_memory "mem=1500M"
#define DEF_bootdev "mmc dev 1; ext2load mmc 1:1"
#define DEF_root "root=/dev/mmcblk1p1"
#define DEF_option "rootwait rw fixrtc rootflags=barrier=1"
#define DEF_setvideomode "setenv videomode video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24 fbmem=30M"
#define DEF_lvdsres1 ""
#define DEF_lvdsres2 ""
#define DEF_cpu_freq "arm_freq=792"

#define getDefValue(var) DEF_ ## var 

#define getEnvStr(env) ( getenv(# env) ? getenv(# env) : DEF_ ## env )

/* Legend
 * KS - Kernel Source
 * 	KS = { eMMC, uSD, SD, SPI, SATA, TFTP, USB }
 * FSS - File System Source
 * 	FSS = { eMMC, uSD, SD, SPI, SATA, NFS, USB }
*/

#define INDEX_SATA 4

const char  *k_device_label[] = { "eMMC onboard", "uSD onboard", "external SD", "SPI onboard", "SATA", "TFTP", "USB" };
const char *fs_device_label[] = { "eMMC onboard", "uSD onboard", "external SD", "SPI onboard", "SATA", "NFS", "USB" };

#ifdef QSEVEN
	// device num for u-boot, to load the kernel
#define K_EMMC_DEV_ID   "0"
#define K_EXT_SD_DEV_ID "2"
#define K_U_SD_DEV_ID   "1"
const short int KS[] = {1, 1, 1, 0, 1, 1, 1};
	// device num for kernel, to mount the FileSystem
#define EXT_SD_DEV_ID "1"
#define EMMC_DEV_ID   "0"
#define U_SD_DEV_ID   "2"
const short int FSS[] = {1, 1, 1, 0, 1, 1, 1};
#endif

#if defined UQSEVEN 
	// device num for u-boot, to load the kernel
#define K_EXT_SD_DEV_ID "1"
#define K_EMMC_DEV_ID   "0"
#define K_U_SD_DEV_ID   "-1"
const short int KS[] = {1, 0, 1, 0, 1, 1, 1};
	// device num for kernel, to mount the FileSystem
#define EXT_SD_DEV_ID "1"
#define EMMC_DEV_ID   "0"
#define U_SD_DEV_ID   "-1"
const short int FSS[] = {1, 0, 1, 0, 1, 1, 1};
#endif

#if defined (uSBC)  
        // device num for u-boot, to load the kernel
#define K_EMMC_DEV_ID   "0"
#define K_EXT_SD_DEV_ID "-1"
#define K_U_SD_DEV_ID   "-1"
const short int KS[] = {1, 0, 0, 0, 1, 1, 1};
        // device num for kernel, to mount the FileSystem
#define EMMC_DEV_ID   "0"
#define EXT_SD_DEV_ID "-1"
#define U_SD_DEV_ID   "-1"
const short int FSS[] = {1, 0, 0, 0, 1, 1, 1};
#endif

#if defined (A62)  
        // device num for u-boot, to load the kernel
#define K_EMMC_DEV_ID   "1"
#define K_EXT_SD_DEV_ID "-1"
#define K_U_SD_DEV_ID   "0"
const short int KS[] = {1, 1, 0, 0, 1, 1, 1};
        // device num for kernel, to mount the FileSystem
#define EMMC_DEV_ID   "0"
#define EXT_SD_DEV_ID "-1"
#define U_SD_DEV_ID   "1"
const short int FSS[] = {1, 1, 0, 0, 1, 1, 1};
#endif

int atoi (char *string);
int ctoi (char ch);
static char *getline(void);
char *do_ramsize (ulong min, ulong max);
void do_bootdev (char *bootdev, char *serverip_tftp , char *ipaddr_tftp, int *use_tftp);
void do_rootdev (char *root, char *option, 
		char *ip_local, char *ip_server, char *nfs_path, char *netmask, int *nfs_en, int *dhcp_set);
int do_videomenu (char *setvideomode);
void do_videoresmenu (char *lvdsres1, char *lvdsres2, int num_video);
void do_maxcpuspeed (char *cpu_freq);
void do_nfs_root (char *ip_local, char *ip_server, char *nfs_path, char *netmask, int *dhcp_set);
int do_secoq7config (cmd_tbl_t * cmd, int flag, int argc, char *argv[]);
void do_tftp_boot (char *serverip_tftp , char *ipaddr_tftp);


int atoi (char *string)
{
	int length;
	int retval = 0;
	int i;
	int sign = 1;

	length = strlen(string);
	for (i = 0; i < length; i++) {
		if (0 == i && string[0] == '-') {
			sign = -1;
			continue;
		}
		if (string[i] > '9' || string[i] < '0') {
			break;
		}
		retval *= 10;
		retval += string[i] - '0';
	}
	retval *= sign;
	return retval;
}

int ctoi (char ch) {
	int retval = 0;
	if (ch <= '9' && ch >= '0') {
		retval = ch - '0';
	}
	return retval;
}

static char *getline (void) {
	static char buffer[100];
	char c;
	size_t i;

	i = 0;
	while (1) {
		buffer[i] = '\0';
		while (!tstc()){
			WATCHDOG_RESET();
			continue;
		}

		c = getc();
		/* Convert to uppercase */
		//if (c >= 'a' && c <= 'z')
		//	c -= ('a' - 'A');

		switch (c) {
		case '\r':	/* Enter/Return key */
		case '\n':
			puts("\n");
			return buffer;

		case 0x03:	/* ^C - break */
			return NULL;

		case 0x5F:
		case 0x08:	/* ^H  - backspace */
		case 0x7F:	/* DEL - backspace */
			if (i) {
				puts("\b \b");
				i--;
			}
			break;

		default:
			/* Ignore control characters */
			if (c < 0x20)
				break;
			/* Queue up all other characters */
			buffer[i++] = c;
			printf("%c", c);
			break;
		}
	}
}



/* *********************************************************************
 * 			RAM size Selection 
 * *********************************************************************/
char *do_ramsize (ulong min, ulong max) {
	char *line;
	do {
		printf ("Chose the ram memory size to dedicate to the Kernel.\n");
		printf ("[min size: %luM - max size %luM]\n", min, max);
		printf ("> ");
		line = getline ();	
	}while ((ulong)atoi(line) < min || (ulong)atoi(line) > max);
	printf ("Will use %luM of RAM memory\n",(ulong)atoi(line));
	return line;
}



/* *********************************************************************
 * 			Kernel/FileSystem Source Selection 
 * *********************************************************************/
#define MIN_PARTITION_ID 1
#define MAX_PARTITION_ID 9

#ifdef QSEVEN
	#if defined(CONFIG_MX6Q)
	#define MAX_DEVICE 4
	#else
	#define MAX_DEVICE 3
	#endif
#elif defined UQSEVEN
	#if defined(CONFIG_MX6Q)
	#define MAX_DEVICE 3
	#else
	#define MAX_DEVICE 2
	#endif
#elif defined uSBC
	#if defined(CONFIG_MX6Q)
	#define MAX_DEVICE 2
	#else
	#define MAX_DEVICE 1
	#endif
#endif


void do_bootdev (char *bootdev, char *serverip_tftp , char *ipaddr_tftp, int *use_tftp) {
	char ch;
	char device[2];
	int has_sata = 0;
	int i, num_dev;
	int index_list[ARRAY_SIZE(KS)]; 
	int selection;
#ifdef CONFIG_MX6Q
	has_sata = 1;
#endif
	do {
		printf ("\nChose boot Device for Kernel.\n");
		num_dev = 0;
		for ( i=0; i<ARRAY_SIZE(KS) ; i++ ) {
			if (KS[i] == 1 && ((i == INDEX_SATA && has_sata == 1) || !(i == INDEX_SATA))) {
				printf ("%d) %s.\n", num_dev+1, k_device_label[i]);  
				index_list[num_dev] = i;
				num_dev ++;
			}
		}
		printf ("> ");
		ch = getc ();
		printf ("%d\n", ctoi(ch));
	} while ((ctoi(ch) < 1) || (ctoi(ch) > num_dev));
	selection = index_list[ctoi(ch)-1];
	*use_tftp = 0;
 	switch (selection) {
		case 0: // eMMC
			strcpy (device, K_EMMC_DEV_ID);
			break;
		case 1: // uSD
			strcpy (device, K_U_SD_DEV_ID);
			break;
		case 2:	// SD
			strcpy (device, K_EXT_SD_DEV_ID);
			break;
		case 3: // SPI
			break;
		case 4: // SATA
			strcpy (bootdev, "sata init; ext2load sata 0:");
			break;
		case 5: // TFTP
			do_tftp_boot (serverip_tftp, ipaddr_tftp);
			*use_tftp = 1;
			break;
		case 6: // USB
			strcpy (bootdev, "usb start; ext2load usb 0:");
			break;
		default:
			printf ("\nInvalid boot device. Valid option are from 1 to %d.\n\n", num_dev);
			strcpy (bootdev, "");
			break;
	}
	if (selection == 0 || selection == 1 || selection == 2) {
		strcpy (bootdev, "mmc dev ");
		strcat (bootdev, device);
		strcat (bootdev, "; ext2load mmc ");
		strcat (bootdev, device);
		strcat (bootdev, ":");
		do {
			printf ("Chose the partition of the selected device.\n");
			printf ("> ");
			ch = getc ();
			printf ("%c\n", ch);
		} while ((ulong)ctoi(ch) < MIN_PARTITION_ID || (ulong)ctoi(ch) > MAX_PARTITION_ID);
		strncat (bootdev, &ch, 1);
		printf ("\n\n");
	}
	if (selection == 4) {
                do {
                        printf ("Chose the partition of the selected device.\n");
                        printf ("> ");
                        ch = getc ();
                        printf ("%c\n", ch);
                } while ((ulong)ctoi(ch) < MIN_PARTITION_ID || (ulong)ctoi(ch) > MAX_PARTITION_ID);
                strncat (bootdev, &ch, 1);
                printf ("\n\n");
        }
	if (selection == 5) {}
	if (selection == 6) {
                do {
                        printf ("Chose the partition of the selected device.\n");
                        printf ("> ");
                        ch = getc ();
                        printf ("%c\n", ch);
                } while ((ulong)ctoi(ch) < MIN_PARTITION_ID || (ulong)ctoi(ch) > MAX_PARTITION_ID);
                strncat (bootdev, &ch, 1);
                printf ("\n\n");
        }
}
		

void do_rootdev (char *root, char *option, 
		char *ip_local, char *ip_server, char *nfs_path, char *netmask, int *nfs_en, int *dhcp_set) {
	char ch;
	char device[2];
	int has_sata = 0;
	int i, num_dev;
	int index_list[ARRAY_SIZE(FSS)]; 
	int selection;
#ifdef CONFIG_MX6Q
	has_sata = 1;
#endif
	do {
		printf ("\nChose boot Device for Kernel.\n");
		num_dev = 0;
		for ( i=0; i<ARRAY_SIZE(FSS) ; i++ ) {
			if (FSS[i] == 1 && ((i == INDEX_SATA && has_sata == 1) || !(i == INDEX_SATA))) {
				printf ("%d) %s.\n", num_dev+1, fs_device_label[i]);  
				index_list[num_dev] = i;
				num_dev ++;
			}
		}
		printf ("> ");
		ch = getc ();
		printf ("%d\n", ctoi(ch));
	} while ((ctoi(ch) < 1) || (ctoi(ch) > num_dev));
	selection = index_list[ctoi(ch)-1];
	*nfs_en = 0;
 	switch (selection) {
		case 0: // eMMC
			strcpy (device, EMMC_DEV_ID);
			break;
		case 1: // uSD
			strcpy (device, U_SD_DEV_ID);
			break;
		case 2:	// SD
			strcpy (device, EXT_SD_DEV_ID);
			break;
		case 3: // SPI
			break;
		case 4: // SATA
			break;
		case 5: // NFS
			do_nfs_root (ip_local, ip_server, nfs_path, netmask, dhcp_set);
			*nfs_en = 1;
			break;
		case 6: // USB
                        break;
		default:
			printf ("\nInvalid root device. Valid option are from 1 to %d.\n\n", num_dev);
			strcpy (root, "");
			strcpy (option, "");
			break;
	}
	if (selection == 0 || selection == 1 || selection == 2 || selection == 4 || selection == 6) {
		if (selection == 0 || selection == 1 || selection == 2) {
			strcpy (root, "root=/dev/mmcblk");
			strcat (root, device);
			strcat (root, "p");
		} else if (selection == 4 || selection == 6) {
			strcpy (root, "root=/dev/sda");
		}
		do {
			printf ("Chose the partition of the selected device.\n");
			printf ("> ");
			ch = getc ();
			printf ("%c\n", ch);
		} while ((ulong)ctoi(ch) < MIN_PARTITION_ID || (ulong)ctoi(ch) > MAX_PARTITION_ID);
		strncat (root, &ch, 1);
		printf ("\n\n");
	}
	if (selection == 5) {}
}


void do_tftp_boot (char *serverip_tftp , char *ipaddr_tftp) {
	char *line;

	do {
		printf ("\nInser the address ip of the tftp server\n");
		printf ("> ");
                line = getline ();
                printf ("%s\n", line);
        } while (0);
	strcpy (serverip_tftp, line);

	do {
		printf ("\nInser the address ip of this tftp client\n");
		printf ("> ");
                line = getline ();
                printf ("%s\n", line);
        } while (0);
	strcpy (ipaddr_tftp, line);
}


void do_nfs_root (char *ip_local, char *ip_server, char *nfs_path, char *netmask, int *dhcp_set) {
	char *line;
	char ch;
	do { 
		printf ("\nDo you want to use dynamic ip assignment (DHCP)? (y/n)\n");
		printf ("> ");
		ch = getc ();
	} while (ch != 'y' && ch != 'n');
	if (ch == 'y') {
		*dhcp_set = 1;
		printf ("\nYou have select to use dynamic ip\n"); 
	} else {
		*dhcp_set = 0;
		printf ("\nYou have select to use static ip\n"); 
	}

	do {
		printf ("Insert the address ip of the host machine\n");
		printf ("> ");
		line = getline ();
		printf ("%s\n", line);
	} while (0);
	strcpy (ip_server, line);

	do {
		printf ("Insert the nfs path of the host machine\n");
		printf ("> ");
		line = getline ();
		printf ("%s\n", line);
	} while (0);
	strcpy (nfs_path, line);

	if (*dhcp_set == 0) {
		do {
			printf ("Insert an address ip for this board\n");
			printf ("> ");
			line = getline ();
			printf ("%s\n", line);
		} while (0);
		strcpy (ip_local, line);

		do {
			printf ("Insert the netmask\n");
			printf ("> ");
			line = getline ();
			printf ("%s\n", line);
		} while (0);
		strcpy (netmask, line);
	}
}



/* *********************************************************************
 * 			Video Output Selection 
 * *********************************************************************/
int do_videomenu (char *setvideomode) {
	char ch;
	int nlvds = 1;
	do {
		printf ("Chose video output device for Seco i.MX6 board.\n");
		printf ("1) Primary LVDS Only.\n");
		printf ("2) HDMI Only.\n");
		printf ("3) LVDS + HDMI.\n");
		printf ("4) HDMI + LVDS.\n");
		printf ("5) LVDS + LVDS.\n");
		printf ("6) LVDS + LVDS + HDMI.\n");
		printf ("7) HDMI + LVDS + LVDS.\n");
		printf ("> ");
		ch = getc();
		printf ("%c\n", ch);
	} while ((ch < '1') || (ch > '7'));
	switch (ch) {
		case '1':
			strcpy (setvideomode, "setenv videomode video=mxcfb0:dev=ldb,${lvdsres1}");
			nlvds = 1;				
			break;
		case '2':
			strcpy (setvideomode, "setenv videomode video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24");
			nlvds = 0;
			break;
		case '3':
			strcpy (setvideomode, "setenv videomode video=mxcfb0:dev=ldb,${lvdsres1} video=mxcfb1:dev=hdmi,1920x1080M@60,if=RGB24");
			nlvds = 1;
			break;
		case '4':
			strcpy (setvideomode, "setenv videomode video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24 setenv video1 video=mxcfb1:dev=ldb,${lvdsres1}");
			nlvds = 1;
			break;
		case '5':
			strcpy (setvideomode, "setenv videomode video=mxcfb0:dev=ldb,${lvdsres1} video=mxcfb1:dev=ldb,${lvdsres2}");
			nlvds = 2;
			break;
		case '6':
			strcpy (setvideomode, "setenv videomode video=mxcfb0:dev=ldb,${lvdsres1} video=mxcfb1:dev=ldb,${lvdsres2} video=mxcfb2:dev=hdmi,1920x1080M@60,if=RGB24");
			nlvds = 2;
			break;
		case '7':
			strcpy (setvideomode, "setenv videomode video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24 video=mxcfb1:dev=ldb,${lvdsres1} video=mxcfb2:dev=ldb,${lvdsres2}");
			nlvds = 2;
			break;
		default:
			printf ("\nInvalid option. Valid choice are from 1 to 7.\n\n");
			strcpy (setvideomode, "");
			nlvds = 0;
			break;
	}
	return nlvds;
}

void do_videoresmenu (char *lvdsres1, char *lvdsres2, int num_video) {
	char ch = '0';
	int i;
	strcpy (lvdsres1, "");
	strcpy (lvdsres2, "");
	for (i = 0 ; i < num_video ; i++){
		printf ("Chose video output resolution for N°%x LVDS display on Seco i.MX6 board.\n", i+1);
		do {
			printf ("1) WVGA [800x480].\n");
			printf ("2) SVGA [800x600].\n");
			printf ("3) XGA  [1024x768].\n");
			printf ("4) WXGA [1368x768].\n");
			printf ("> ");
			ch = getc();
			printf ("%c\n", ch);
		} while ((ch < '1') || (ch > '4'));
		switch (ch) {
			case '1':
				if(i==0) strcpy (lvdsres1, "LDB-WVGA,if=RGB666");
				if(i==1) strcpy (lvdsres2, "LDB-WVGA,if=RGB666");
				break;
			case '2':
				if(i==0) strcpy (lvdsres1, "LDB-SVGA,if=RGB666");
				if(i==1) strcpy (lvdsres2, "LDB-SVGA,if=RGB666");
				break;
			case '3':
				if(i==0) strcpy (lvdsres1, "LDB-XGA,if=RGB666");
				if(i==1) strcpy (lvdsres2, "LDB-XGA,if=RGB666");
				break;
			case '4':
				if(i==0) strcpy (lvdsres1, "LDB-WXGA,if=RGB24");
				if(i==1) strcpy (lvdsres2, "LDB-WVGA,if=RGB24");
				break;
			default:
				printf ("\nInvalid option. Valid choice are from 1 to 4.\n\n");
				if(i==0) strcpy (lvdsres1, "");
				if(i==1) strcpy (lvdsres2, "");
				break;
		}
	}
}



/* *********************************************************************
 * 			CPU Speed Selection 
 * *********************************************************************/
void do_maxcpuspeed (char *cpu_freq) {
	char ch;
	do {
		printf ("Chose MAX cpu frequency [default = 792 MHz].\n");
		printf ("1) 396 MHz.\n"); 
		printf ("2) 792 MHz.\n");
		printf ("3) 996 MHz.\n");
		printf ("> ");
		ch = getc();
		printf ("%c\n", ch);
	} while ((ch < '1') || (ch > '3'));
	switch (ch) {
		case '1':
			strcpy(cpu_freq, "arm_freq=396");
			break;
		case '2':
			strcpy(cpu_freq, "arm_freq=792");
			break;
		case '3':
			strcpy(cpu_freq, "arm_freq=996");
			break;
		default:
			printf ("\nInvalid cpu speed frequency. Valid level are from 1 to 3.\n\n");
			strcpy(cpu_freq, "");
			break;
	}
	printf ("\n\n");

}


int do_secoq7config (cmd_tbl_t * cmd, int flag, int argc, char *argv[]) {
	char buf [50];	
	int nlvds = 0;
	ulong min_size = 512;
	ulong max_size;

	ulong size = PHYS_SDRAM_SIZE;
	max_size = size / (1 << 20); // get size in MB

	if (max_size > 2100)
		max_size = 3800;
	
	char *line;
	
	char bootdev[100]; 
	char root[100];
	char option[100];
	// TFTP
	char serverip_tftp[50];
	char clientip_tftp[50];
	
	char setvideomode[300];
	
	char lvdsres1[100];
	char lvdsres2[100];
	
	char cpu_freq[100];
	
	// NFS
	char ip_local[50];
	char ip_server[50];
	char nfs_path[300];
	char netmask[50];
	
	int valid_param = 0;
	int nfs_set = 0;
	int dhcp_set = 0;
	int tftp_set = 0;

	int set_kernel_boot = 0;
	int set_fs_boot = 0;
	
	printf ("Seco interactive configuration utility.\n");
		
	if (argc > 2)
		return cmd_usage(cmd);
	
	
	if (argc == 2 && strcmp(argv[1], "help") == 0) {
		return cmd_usage(cmd);
	}
	
	if (argc == 2) {
				
		 if (strcmp(argv[1], "default") == 0) {
		 	strcpy (buf, getDefValue(memory));
			strcpy (bootdev, getDefValue(bootdev));
			strcpy (root, getDefValue(root));
			strcpy (option, getDefValue(option));
			strcpy (setvideomode, getDefValue(setvideomode));
			strcpy (lvdsres1, getDefValue(lvdsres1));
			strcpy (lvdsres2, getDefValue(lvdsres2));
			strcpy (cpu_freq, getDefValue(cpu_freq));
			printf ("Default configuration setted\n");
			valid_param = 1;
		}  else {
			strcpy (buf, getEnvStr(memory));
			strcpy (bootdev, getEnvStr(bootdev));
			strcpy (root, getEnvStr(root));
			strcpy (setvideomode, getEnvStr(setvideomode));
			strcpy (lvdsres1, getEnvStr(lvdsres1));
			strcpy (lvdsres2, getEnvStr(lvdsres2));
			strcpy (cpu_freq, getEnvStr(cpu_freq));
		}
			
		if (strcmp(argv[1], "bootdev") == 0) {
			/* set Kernel source */
			do_bootdev (bootdev, serverip_tftp, clientip_tftp, &tftp_set);
			set_kernel_boot = 1;
			valid_param = 1;	
		} 
			
		if (strcmp(argv[1], "rootdev") == 0) {
			/* set File System source */
			do_rootdev (root, option, ip_local, ip_server, nfs_path, netmask, &nfs_set, &dhcp_set);
			strcpy (option, getDefValue(option));
			set_fs_boot = 1;
			valid_param = 1;	
		} 
	
		if (strcmp(argv[1], "memsize") == 0) {
			/* set memory size for the kernel */
			line = do_ramsize (min_size, max_size);
			sprintf (buf, "mem=%sM", line);
			setenv ("memory", buf);	
			valid_param = 2;
		} 
	
		if (strcmp(argv[1], "video") == 0) {
			nlvds = do_videomenu (setvideomode);
			/* set lvds resolution */
			do_videoresmenu (lvdsres1, lvdsres2, nlvds);
			setenv ("setvideomode", setvideomode);
			setenv ("lvdsres1", lvdsres1);
			setenv ("lvdsres2", lvdsres2);
			valid_param = 2;
		}
	
		if (strcmp(argv[1], "cpufreq") == 0) {
			/* set cpu frequency */
			do_maxcpuspeed (cpu_freq);
			setenv ("cpu_freq", cpu_freq);
			valid_param = 2;
		}
		
		if (valid_param == 0) {
			printf ("\nERROR: argument not valid!\n");
			return cmd_usage(cmd);
		}

		if (valid_param == 2) {
			// we do not update other boot parameters
			goto save_param;
		}
	}
	
	if (argc == 1) {
		/* set memory size for the kernel */
		line = do_ramsize (min_size, max_size);
		sprintf (buf, "mem=%sM", line);
		/* set boot device and file system source device */
		do_bootdev (bootdev, serverip_tftp, clientip_tftp, &tftp_set);
		set_kernel_boot = 1;
		do_rootdev (root, option, ip_local, ip_server, nfs_path, netmask, &nfs_set, &dhcp_set);
		set_fs_boot = 1;
		strcpy (option, getDefValue(option));
		nlvds = do_videomenu (setvideomode);
		
		/* set lvds resolution */
		do_videoresmenu (lvdsres1, lvdsres2, nlvds);
		
		/* set cpu frequency */
		do_maxcpuspeed (cpu_freq);
		
	}
	

	if (set_kernel_boot) {
		// KERNEL BOOT
		if (tftp_set == 1) {
			setenv ("loadaddr", "0x18000000");
			setenv ("use_tftp", "1");
			setenv ("serverip", serverip_tftp);
			setenv ("ipaddr", clientip_tftp);
		} else {
			setenv ("loadaddr", "0x10800000");
			setenv ("use_tftp", "0");
			setenv ("serverip", "");
			setenv ("ipaddr", "");
		}

		setenv ("setbootdev", "if test \"${use_tftp}\" = \"0\"; then setenv boot_dev ${bootdev} 10800000 /boot/uImage; else setenv boot_dev tftpboot 0x18000000 uImage; fi");
	}

	if (set_fs_boot) {	
		// FILE SYSTEM BOOT
		if (nfs_set == 0) {
			setenv ("memory", buf);	
			setenv ("bootdev", bootdev);
			setenv ("root", root);
			setenv ("option", option);
			setenv ("setvideomode", setvideomode);
			setenv ("lvdsres1", lvdsres1);
			setenv ("lvdsres2", lvdsres2);
			setenv ("cpu_freq", cpu_freq);
			setenv ("run_from_nfs", "0");
		} else {
			setenv ("ip_local", ip_local);
			setenv ("ip_server", ip_server);
			setenv ("nfs_path", nfs_path);
			setenv ("netmask", netmask);
			setenv ("run_from_nfs", "1");
		}
		if (dhcp_set == 1)
			setenv ("use_dhcp", "1");
		else
			setenv ("use_dhcp", "0");

		setenv ("set_ipconf_no_dhcp", "setenv ip \"${ip_local}:::${netmask}::eth0:off\"");
		setenv ("set_ipconf_dhcp", "setenv ip \":::::eth0:dhcp\"");
		setenv ("set_ip", "if test \"${use_dhcp}\" = \"0\"; then run set_ipconf_no_dhcp; else run set_ipconf_dhcp; fi");
		setenv ("set_nfsroot", "setenv nfsroot \"${ip_server}:${nfs_path}\"");
#ifdef DEBUG_UART5
		setenv ("setbootargs_nfs", "setenv bootargs console=ttymxc4,115200 root=/dev/nfs nfsroot=${nfsroot} nolock,wsize=4096,rsize=4096 ip=${ip} ${memory} ${cpu_freq} ${videomode}");
#else
		setenv ("setbootargs_nfs", "setenv bootargs console=ttymxc1,115200 root=/dev/nfs nfsroot=${nfsroot} nolock,wsize=4096,rsize=4096 ip=${ip} ${memory} ${cpu_freq} ${videomode}");
#endif
	}

#ifdef DEBUG_UART5
	setenv ("setbootargs", "setenv bootargs console=ttymxc4,115200 ${root} ${option} ${memory} ${cpu_freq} ${videomode}");
#else
	setenv ("setbootargs", "setenv bootargs console=ttymxc1,115200 ${root} ${option} ${memory} ${cpu_freq} ${videomode}");
#endif
	setenv ("bootcmd", "run setvideomode; if test \"${run_from_nfs}\" = \"0\"; then run setbootargs; else run set_ip; run set_nfsroot; run setbootargs_nfs; fi; run setbootdev; run boot_dev; bootm ${loadaddr}");
	
save_param:
	saveenv();
	printf ("\n");
	return 0;
}


/***************************************************/

U_BOOT_CMD(
	secoq7config, 3, 1,	do_secoq7config,
	"Interactive setup for seco q7 configuration.",
	"           - set whole environment\n"
	"secoq7config [default] - use default configuration\n"
	"secoq7config [bootdev] - set only boot device (Kernel source) and use remaining predefined parameters \n"
	"secoq7config [rootdev] - set only root device (File System source) and use remaining predefined parameters \n"
	"secoq7config [memsize] - set only RAM memory to use and use remaining predefined parameters \n"
	"secoq7config [video]   - set only video settings and use remaining predefined parameters \n"
	"secoq7config [cpufreq] - set only CPU's frequency and use remaining predefined parameters \n"
);
