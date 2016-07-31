#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <ogcsys.h>

#include "tools.h"

extern void __exception_setreload(int);

int main(int argc, char* argv[])
{
	__exception_setreload(10);
	
	int i, ret;
	
	Init_Console();
	printf("\x1b[%u;%um", 37, false);
	
	PAD_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	
	printheadline();
	
	ret = ahbprot_menu();
	if (ret < 0)
	{
		ret = ios_selectionmenu(236);
		if (ret > 0)
		{
			printf("\t- Reloading to IOS%d... ", ret);
			WPAD_Shutdown();
			IOS_ReloadIOS(ret);
			sleep(2);
			PAD_Init();
			WPAD_Init();
			WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
			printf("done.\n\n");
		} else
		if (ret == 0)
		{
			printf("\t- Proceeding without IOS reload...\n\n");
		} else {
			Reboot();
		}
	}
	
	resetscreen();
	printheadline();
	
	/* Initialize NAND FS */
	ISFS_Initialize();
	
	/* Do our stuff */
	char path[ISFS_MAXPATH];
	char *filenames[10] = { "misc.bin", "nwc24dl.bin", "nwc24fl.bin", "nwc24fls.bin", "nwc24msg.cbk", "nwc24msg.cfg", \
							"mbox/wc24recv.ctl", "mbox/wc24recv.mbx", "mbox/wc24send.ctl", "mbox/wc24send.mbx" };
	
	for (i = 0; i < 10; i++)
	{
		snprintf(path, MAX_CHARACTERS(path), "/shared2/wc24/%s", filenames[i]);
		
		printf("%u. Deleting \"%s\"... ", i + 1, path);
		ret = ISFS_Delete(path);
		if (ret < 0)
		{
			switch (ret)
			{
				case -1:
				case -102:
					printf("\n\t- Error: Permission denied!");
					break;
				case -2:
				case -105:
					printf("\n\t- Error: File exists!");
					break;
				case -4:
				case -101:
					printf("\n\t- Error: Invalid argument!");
					break;
				case -6:
				case -106:
					printf("\n\t- Error: File not found!");
					break;
				case -8:
				case -118:
					printf("\n\t- Error: Resource busy!");
					break;
				case -12:
				case -103:
				case -114:
					printf("\n\t- Error: NAND ECC failure!");
					break;
				case -22:
					printf("\n\t- Error: Memory allocation failed during request!");
					break;
				case -107:
				case -109:
					printf("\n\t- Error: Too many files open!");
					break;
				case -108:
					printf("\n\t- Error: Memory full!");
					break;
				case -110:
					printf("\n\t- Error: Path name is too long!");
					break;
				case -111:
					printf("\n\t- Error: File already open!");
					break;
				case -115:
					printf("\n\t- Error: Directory not empty!");
					break;
				case -116:
					printf("\n\t- Error: Max directory depth exceeded!");
					break;
				default:
					printf("\n\t- FATAL / UNKNOWN ERROR!!!");
			}
			
			printf(" (ret = %d)\n\n", ret);
			
			if (ret <= -119) break;
		} else {
			printf("OK!\n\n");
		}
		
		usleep(2000000); // 2 seconds
	}
	
	/* Unmount NAND FS and exit */
	ISFS_Deinitialize();
	Reboot();
	
	return 0;
}
