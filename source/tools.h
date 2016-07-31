#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <sys/unistd.h>
#include <wiiuse/wpad.h>
#include <runtimeiospatch.h>
#include <malloc.h>

// Values for DetectInput
#define DI_BUTTONS_DOWN		0
#define DI_BUTTONS_HELD		1

#define resetscreen() printf("\x1b[2J")

#define MAX_CHARACTERS(x) ((sizeof((x))) / (sizeof((x)[0]))) // Returns the number of elements in an array

void Reboot();
u32 DetectInput(u8 DownOrHeld);
void Init_Console();
void printheadline();
int ahbprot_menu();
int ios_selectionmenu(int default_ios);

#endif /* __TOOLS_H__ */
