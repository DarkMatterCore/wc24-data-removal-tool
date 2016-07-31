#include <gccore.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "tools.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void Reboot()
{
	if (*(u32*)0x80001800) exit(0);
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

/* Big thanks to JoostinOnline for the new controller code */
u32 DetectInput(u8 DownOrHeld)
{
	u32 pressed = 0;
	u32 gcpressed = 0;
	VIDEO_WaitVSync();
	
	// WiiMote (and Classic Controller) take precedence over the GCN controller to save time
	if (WPAD_ScanPads() > WPAD_ERR_NONE) // Scan the Wii remotes.  If there any problems, skip checking buttons
	{
		if (DownOrHeld == DI_BUTTONS_DOWN)
		{
			pressed = WPAD_ButtonsDown(0) | WPAD_ButtonsDown(1) | WPAD_ButtonsDown(2) | WPAD_ButtonsDown(3); // Store pressed buttons
		} else {
			pressed = WPAD_ButtonsHeld(0) | WPAD_ButtonsHeld(1) | WPAD_ButtonsHeld(2) | WPAD_ButtonsHeld(3); // Store held buttons
		}
		
		// Convert to WiiMote values
		if (pressed & WPAD_CLASSIC_BUTTON_ZR) pressed |= WPAD_BUTTON_PLUS;
		if (pressed & WPAD_CLASSIC_BUTTON_ZL) pressed |= WPAD_BUTTON_MINUS;
		
		if (pressed & WPAD_CLASSIC_BUTTON_PLUS) pressed |= WPAD_BUTTON_PLUS;
		if (pressed & WPAD_CLASSIC_BUTTON_MINUS) pressed |= WPAD_BUTTON_MINUS;
		
		if (pressed & WPAD_CLASSIC_BUTTON_A) pressed |= WPAD_BUTTON_A;
		if (pressed & WPAD_CLASSIC_BUTTON_B) pressed |= WPAD_BUTTON_B;
		if (pressed & WPAD_CLASSIC_BUTTON_X) pressed |= WPAD_BUTTON_2;
		if (pressed & WPAD_CLASSIC_BUTTON_Y) pressed |= WPAD_BUTTON_1;
		if (pressed & WPAD_CLASSIC_BUTTON_HOME) pressed |= WPAD_BUTTON_HOME;
		
		if (pressed & WPAD_CLASSIC_BUTTON_UP) pressed |= WPAD_BUTTON_UP;
		if (pressed & WPAD_CLASSIC_BUTTON_DOWN) pressed |= WPAD_BUTTON_DOWN;
		if (pressed & WPAD_CLASSIC_BUTTON_LEFT) pressed |= WPAD_BUTTON_LEFT;
		if (pressed & WPAD_CLASSIC_BUTTON_RIGHT) pressed |= WPAD_BUTTON_RIGHT;
	}
	
	// Return WiiMote / Classic Controller values
	if (pressed) return pressed;
	
	// No buttons on the WiiMote or Classic Controller were pressed
	if (PAD_ScanPads() > PAD_ERR_NONE)
	{
		if (DownOrHeld == DI_BUTTONS_DOWN)
		{
			gcpressed = PAD_ButtonsDown(0) | PAD_ButtonsDown(1) | PAD_ButtonsDown(2) | PAD_ButtonsDown(3); // Store pressed buttons
		} else {
			gcpressed = PAD_ButtonsHeld(0) | PAD_ButtonsHeld(1) | PAD_ButtonsHeld(2) | PAD_ButtonsHeld(3); // Store held buttons
		}
		
		// Convert to WiiMote values
		if (gcpressed & PAD_TRIGGER_R) pressed |= WPAD_BUTTON_PLUS;
		if (gcpressed & PAD_TRIGGER_L) pressed |= WPAD_BUTTON_MINUS;
		if (gcpressed & PAD_BUTTON_A) pressed |= WPAD_BUTTON_A;
		if (gcpressed & PAD_BUTTON_B) pressed |= WPAD_BUTTON_B;
		if (gcpressed & PAD_BUTTON_X) pressed |= WPAD_BUTTON_2;
		if (gcpressed & PAD_BUTTON_Y) pressed |= WPAD_BUTTON_1;
		if (gcpressed & PAD_BUTTON_MENU) pressed |= WPAD_BUTTON_HOME;
		if (gcpressed & PAD_BUTTON_UP) pressed |= WPAD_BUTTON_UP;
		if (gcpressed & PAD_BUTTON_DOWN) pressed |= WPAD_BUTTON_DOWN;
		if (gcpressed & PAD_BUTTON_LEFT) pressed |= WPAD_BUTTON_LEFT;
		if (gcpressed & PAD_BUTTON_RIGHT) pressed |= WPAD_BUTTON_RIGHT;
	}
	
	return pressed;
}

void Init_Console()
{
	// Initialise the video system
	VIDEO_Init();
	
	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	
	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);
	
	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);
	
	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	// Set console parameters
    int x = 24, y = 32, w, h;
    w = rmode->fbWidth - (32);
    h = rmode->xfbHeight - (48);

    // Initialize the console - CON_InitEx works after VIDEO_ calls
	CON_InitEx(rmode, x, y, w, h);

	// Clear the garbage around the edges of the console
    VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
}

void printheadline()
{
	int rows, cols;
	CON_GetMetrics(&cols, &rows);
	
	printf("WC24 Data Removal Tool.");
	
	char buf[64];
	sprintf(buf, "IOS%u (v%u)", IOS_GetVersion(), IOS_GetRevision());
	printf("\x1B[%d;%dH", 0, cols-strlen(buf)-1);
	printf(buf);
	
	printf("\nOriginal idea by Krafter. Made by DarkMatterCore.\n\n");
}

int ahbprot_menu()
{
	s32 ret;
	u32 pressed;

	/* HW_AHBPROT check */
	if (AHBPROT_DISABLED)
	{
		printf("Hardware protection is disabled!\n");
		printf("Current IOS: %u.\n\n", IOS_GetVersion());
		
		printf("Press A button to use full hardware access.\n");
		printf("Press B button to reload to another IOS.\n");
		printf("Press HOME or Start to exit.\n\n");
		
		for(;;)
		{
			pressed = DetectInput(DI_BUTTONS_DOWN);
			
			/* A button */
			if (pressed & WPAD_BUTTON_A) break;
			
			/* B button */
			if (pressed & WPAD_BUTTON_B)
			{
				resetscreen();
				printheadline();
				return -1;
			}
			
			/* HOME/Start button */
			if (pressed & WPAD_BUTTON_HOME)
			{
				printf("Exiting...");
				Reboot();
			}
		}
		
		printf("Initializing IOS patches... ");
		ret = IosPatch_RUNTIME(true, false, true, false);
		if (ret < 0)
		{
			/* This is a very, very weird error */
			
			printf("ERROR!\n\n");
			printf("\tUnable to load the initial patches. Maybe the loaded IOS isn't\n");
			printf("\tvulnerable for an unknown reason.\n");
			sleep(4);
			printf("\tThis error is very uncommon. I already checked if the HW_AHBPROT\n");
			printf("\tprotection was disabled. You should report this to me as soon as\n");
			printf("\tyou can.\n");
			sleep(4);
			printf("\tI'll let you reload to another IOS instead of kicking you out\n");
			printf("\tto the loader...");
			sleep(4);
			
			resetscreen();
			printheadline();
			
			return -1;
		}
		
		printf("OK!\n\n");
	} else {
		return -1;
	}
	
	return 0;
}

void set_highlight(bool highlight)
{
	if (highlight)
	{
		printf("\x1b[%u;%um", 47, false);
		printf("\x1b[%u;%um", 30, false);
	} else {
		printf("\x1b[%u;%um", 37, false);
		printf("\x1b[%u;%um", 40, false);
	}
}

s32 __u8Cmp(const void *a, const void *b)
{
	return *(u8 *)a-*(u8 *)b;
}

u8 *get_ioslist(u32 *cnt)
{
	u64 *buf = 0;
	s32 i, res;
	u32 tcnt = 0, icnt;
	u8 *ioses = NULL;
	
	// Get stored IOS versions.
	res = ES_GetNumTitles(&tcnt);
	if (res < 0)
	{
		printf("\t- ES_GetNumTitles: Error! (result = %d).\n", res);
		return 0;
	}
	
	buf = memalign(32, sizeof(u64) * tcnt);
	if (!buf) 
	{
		printf("\t- Error allocating memory buffer!\n");
		return 0;
	}
	
	res = ES_GetTitles(buf, tcnt);
	if (res < 0)
	{
		printf("\t- ES_GetTitles: Error! (result = %d).\n", res);
		free(buf);
		return 0;
	}

	icnt = 0;
	for(i = 0; i < tcnt; i++)
	{
		if(*((u32 *)(&(buf[i]))) == 1 && (u32)buf[i] > 2 && (u32)buf[i] < 0x100)
		{
			icnt++;
			ioses = (u8 *)realloc(ioses, sizeof(u8) * icnt);
			ioses[icnt - 1] = (u8)buf[i];
		}
	}

	ioses = (u8 *)malloc(sizeof(u8) * icnt);
	if (!ioses)
	{
		printf("\t- Error allocating IOS memory buffer!\n");
		free(buf);
		return 0;
	}
	
	icnt = 0;
	
	for (i = 0; i < tcnt; i++)
	{
		if(*((u32 *)(&(buf[i]))) == 1 && (u32)buf[i] > 2 && (u32)buf[i] < 0x100)
		{
			icnt++;
			ioses[icnt - 1] = (u8)buf[i];
		}
	}
	
	free(buf);
	qsort(ioses, icnt, 1, __u8Cmp);

	*cnt = icnt;
	return ioses;
}

int ios_selectionmenu(int default_ios)
{
	u32 pressed;
	int i, selection = 0;
	u32 ioscount;
	
	u8 *list = get_ioslist(&ioscount);
	if (list == 0) return -1;
	
	for (i = 0; i < ioscount; i++)
	{
		/* Default to default_ios if found, else the loaded IOS */
		
		if (list[i] == default_ios)
		{
			selection = i;
			break;
		}
		
		if (list[i] == IOS_GetVersion())
		{
			selection = i;
		}
	}
	
	while (true)
	{
		printf("\x1B[%d;%dH", 5, 0);	// move console cursor to y/x
		printf("Select the IOS version to use:       \b\b\b\b\b\b");
		
		set_highlight(true);
		printf("IOS%u", list[selection]);
		set_highlight(false);
		
		printf("\n\nPress LEFT/RIGHT to change IOS version.");
		printf("\nPress A button to load the selected IOS.");
		printf("\nPress B to continue without IOS Reload.");
		printf("\nPress HOME or Start to exit.\n\n");
		
		pressed = DetectInput(DI_BUTTONS_DOWN);
		
		if (pressed & WPAD_BUTTON_LEFT)
		{	
			if (selection > 0)
			{
				selection--;
			} else {
				selection = ioscount - 1;
			}
		}
		
		if (pressed & WPAD_BUTTON_RIGHT)
		{
			if (selection < ioscount -1)
			{
				selection++;
			} else {
				selection = 0;
			}
		}
		
		if (pressed & WPAD_BUTTON_A) break;
		
		if (pressed & WPAD_BUTTON_B) return 0;
		
		if (pressed & WPAD_BUTTON_HOME)
		{
			printf("Exiting...");
			free(list);
			Reboot();
		}
	}
	
	selection = list[selection];
	free(list);
	
	return selection;
}
