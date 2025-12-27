#ifndef __HBM_library__
#define __HBM_library__

/**
 * Initializes Home Menu library..
 * Please use the dimensions of the game/homebrew. The screen will be clipped otherwise.
 */
void HBM_Init(int width, int height);

/**
 * Releases resources used by Home Menu library.
 */
void HBM_Uninit();

/**
 * Determines whether the No Home icon shows up instead of the menu.
 */
void HBM_ToggleUsage(bool value);

/**
 * Draws the No Home icon on the screen. Should be called before flushing framebuffer.
 */
void HBM_DrawNoHome();

/**
 * Opens the HOME Menu UI.
 */
void HBM_Menu();

/**
 * Closes the HOME Menu UI.
 */
void HBM_HideMenu();

/***********************************************
 *              HBM CONFIGURATION              *
 ***********************************************/

/**
 * Determines whether to force an aspect ratio by default. If left uncommented, it will be detected from the system's config.
 * The aspect ratio can also be changed inline using HBM_SetWidescreen() (see below).
 *
 * 0: Standard (4:3)
 * 1: Widescreen (16:9)
 **/
#define HBM_FORCE_ASPECT_RATIO 0
void HBM_SetWidescreen(bool value);

/**
 * Default UI language setting.
 * This can also be changed inline using HBM_SetLanguage() (see below).
 *
 * -1: System (auto)		7:  Chinese	(Simplified)	15: Danish
 * 0:  Japanese				8:  Chinese (Traditional)	16: Swedish
 * 1:  English				9:  Korean					17: Turkish
 * 2:  German				10: Portuguese				18: Catalan
 * 3:  French				11: Portuguese (Brazil)
 * 4:  Spanish				12: Russian
 * 5:  Italian				13: Ukrainian
 * 6:  Dutch				14: Polish
 **/
#define HBM_LANGUAGE -1
void HBM_SetLanguage(int value);

/**
 * Enables sound output.
 *
 * 0: Disabled
 * 1: ASNDLib
 **/
#define HBM_SOUND_OUTPUT 1

/**
 * Choose method of drawing to the screen.
 *
 * None: Disabled
 * 0: Libwiisprite
 * 1: GRRLIB
 * 2: GX
 **/
#define HBM_DRAW_METHOD 1

/**
 * Verbose & debug options
 **/
#define HBM_VERBOSE
#define HBM_DEBUG

/*************************************************************************/
/*************************************************************************
  /!\ DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING /!\
**************************************************************************/
/*************************************************************************/

// Global libraries
// ******************************
#include <gccore.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <asndlib.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>

// Utilities
// ******************************
#include <pngu.h>
#include "hbm/i18n.h"
#include "hbm/romfs.h"

// Structs
// ******************************
enum HBM_STATUS {
	HBM_INACTIVE,
	HBM_BLOCKED,
	HBM_OPENING,
	HBM_OPEN,
	HBM_CLOSING,
	HBM_CLOSED
};

enum HBM_INTERACTIONLAYER {
	HBM_INTERACTION_MAIN,
	HBM_INTERACTION_BUTTON,
	HBM_INTERACTION_DIALOG,
	HBM_INTERACTION_DIALOGBUTTON,
	HBM_INTERACTION_WPAD
};

enum HBM_TEXTALIGNH {
	HBM_TEXT_LEFT,
	HBM_TEXT_CENTER,
	HBM_TEXT_RIGHT,
};

enum HBM_TEXTALIGNV {
	HBM_TEXT_TOP,
	HBM_TEXT_MIDDLE
};

struct HBM_CONFIG {
	enum HBM_STATUS Status;
	enum HBM_INTERACTIONLAYER InteractionLayer;

	float ScaleX;
	float ScaleY;
	int Width;
	int Height;

	bool Widescreen;
	int Language;
};

// Filelist
// ******************************

// Images
#include "HBM_cursor1_png.h"
#include "HBM_cursor2_png.h"
#include "HBM_cursor3_png.h"
#include "HBM_cursor4_png.h"
#include "HBM_dialogBG_png.h"
#include "HBM_dialogButton_mask_png.h"
#include "HBM_dialogButton_png.h"
#include "HBM_mainButton_mask_png.h"
#include "HBM_mainButton_png.h"
#include "HBM_noHome_png.h"

// Sounds
#include "HBM_sfx_cancel_pcm.h"
#include "HBM_sfx_confirm_pcm.h"
#include "HBM_sfx_hover_pcm.h"
#include "HBM_sfx_menuclose_pcm.h"
#include "HBM_sfx_menuopen_pcm.h"
#include "HBM_sfx_select_pcm.h"

// Fonts
// HBM_BINARY_DECLARE(nintendo_NTLGDB_001_ttf, ???)
// HBM_BINARY_DECLARE(nintendo_NTLG-DB_002_ttf, ???)
// HBM_BINARY_DECLARE(UtrilloProGrecoStd_ttf, ???)

// Classes
// ******************************
#include "hbm/classes/HBMImage.h"
#include "hbm/classes/HBMElement.h"
#include "hbm/classes/HBMButton.h"
#include "hbm/classes/HBMButtonMain.h"
#include "hbm/classes/HBMDialogButton.h"
#include "hbm/classes/HBMDialog.h"

#include "hbm/console.h"
#include "hbm/font.h"
#include "hbm/wpad.h"

#define HBM_WIDESCREEN_RATIO (852.0F/640.0F)
#define HBM_EASEINOUT(x) (x * x * (3.0f - 2.0f * x))
#define HBM_MAXGLYPHS 256

#endif