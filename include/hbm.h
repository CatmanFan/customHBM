#ifndef __HBM_library__
#define __HBM_library__

/***********************************************
 *             HBM PUBLIC FUNCTIONS            *
 ***********************************************/

/**
 * Initializes Home Menu library..
 * Please use the dimensions of the game/homebrew. The screen will be clipped otherwise.
 *
 * Original GX attributes used by the target homebrew.
 * The HBM engine renders using GX_MODULATE & GX_DIRECT, and this is required to restore the original attributes
 * when closing the menu (or when drawing the No Home icon over the original render), and in order to avoid crashes or freezing.
 *
 * To disable graphics entirely, set either host_TEVSTAGE0 or host_TEX0 to -1.
 *
 * 				host_TEVSTAGE0	host_TEX0
 *				***************************************
 * LWS			GX_MODULATE		GX_DIRECT
 * GRRLIB	  	GX_PASSCLR		GX_NONE
 * Libwiigui	GX_PASSCLR		GX_NONE
 * Libgui		GX_PASSCLR		GX_DIRECT
 **/
void HBM_Init(int width, int height, int host_TEVSTAGE0, int host_TEX0);

/**
 * Releases resources used by Home Menu library.
 **/
void HBM_Uninit();

/**
 * Determines whether the No Home icon shows up instead of the menu.
 **/
void HBM_ToggleUsage(bool value);

/**
 * Draws the No Home icon on the screen. Should be called before flushing framebuffer.
 **/
void HBM_DrawNoHome();

/**
 * Takes the main screenshot to be used as the background.
 * This is automatically called at HBM_Menu() but can be called before if the background
 * screenshot should be made earlier.
 **/
void HBM_TakeScreenshot();

/**
 * Opens the HOME Menu UI.
 **/
void HBM_Menu();

/**
 * Closes the HOME Menu UI.
 **/
void HBM_HideMenu();

/**
 * Checks if the Home Menu has been successfully inited.
 * This can be useful if it has failed to init and you need a way to exit the homebrew (e.g. "if (HBM_IsAvailable()) HBM_Menu()").
 **/
bool HBM_IsAvailable();

/**
 * Determines whether to force an aspect ratio by default. If left uncommented, it will be detected from the system's config.
 * The aspect ratio can also be changed inline using HBM_SetWidescreen() (see below).
 *
 * @param value Can be either 0 (standard, 4/3) or 1 (widescreen, 16/9).
 **/
void HBM_SetWidescreen(bool value);

void HBM_SetBeforeShowMenu(void (*func)());		// called just before starting menu-display animation
void HBM_SetAfterShowMenu(void (*func)());		// called just after finishing menu-display animation
void HBM_SetBeforeDraw(void (*func)());			// called at the very beginning of the menu loop
void HBM_SetAfterDraw(void (*func)());			// called at the very end of the menu loop
void HBM_SetBeforeHideMenu(void (*func)());		// called just before starting menu-hide animation
void HBM_SetAfterHideMenu(void (*func)());		// called just after finishing menu-hide animation
void HBM_SetBeforeExit(void (*func)());			// called just before exiting to Wii system menu or HBC
void HBM_SetMyReset(void (*func)());			// called at "reset"

/**
 * Default UI language setting.
 * This can also be changed inline using HBM_SetLanguage() (see below).
 *
 * @param value The language setting, available options are defined in enum HBM_LANG.
 **/
enum HBM_LANG {
	HBM_LANG_SYSTEM = -1, // Uses Wii's system language
	HBM_LANG_CATALAN,
	HBM_LANG_WELSH,
	HBM_LANG_DANISH,
	HBM_LANG_GERMAN,
	HBM_LANG_GREEK,
	HBM_LANG_ENGLISH,
	HBM_LANG_SPANISH,
	HBM_LANG_BASQUE,
	HBM_LANG_FRENCH,
	HBM_LANG_GALICIAN,
	HBM_LANG_ITALIAN,
	HBM_LANG_KALAALLISUT,
	HBM_LANG_DUTCH,
	HBM_LANG_NORWEGIAN, // Bokmål
	HBM_LANG_POLISH,
	HBM_LANG_PT_PORTUGUESE,
	HBM_LANG_BR_PORTUGUESE,
	HBM_LANG_FINNISH,
	HBM_LANG_SWEDISH,
	HBM_LANG_TURKISH,
	HBM_LANG_RUSSIAN,
	HBM_LANG_UKRAINIAN,
	HBM_LANG_JAPANESE,
	HBM_LANG_OKINAWAN,
	HBM_LANG_SIMP_CHINESE,
	HBM_LANG_TRAD_CHINESE,
	HBM_LANG_KOREAN,

	/* HBM_LANG_JAPANESE = 0,
	HBM_LANG_ENGLISH,
	HBM_LANG_GERMAN,
	HBM_LANG_FRENCH,
	HBM_LANG_SPANISH,
	HBM_LANG_ITALIAN,
	HBM_LANG_DUTCH,
	HBM_LANG_SIMP_CHINESE,
	HBM_LANG_TRAD_CHINESE,
	HBM_LANG_KOREAN,
	HBM_LANG_PT_PORTUGUESE,
	HBM_LANG_BR_PORTUGUESE,
	HBM_LANG_RUSSIAN,
	HBM_LANG_UKRAINIAN,
	HBM_LANG_POLISH,
	HBM_LANG_SWEDISH,
	HBM_LANG_DANISH,
	// HBM_LANG_KALAALLISUT,
	// HBM_LANG_FINNISH,
	// HBM_LANG_NORWEGIAN, // Bokmål
	HBM_LANG_GREEK,
	HBM_LANG_TURKISH,
	HBM_LANG_CATALAN,
	HBM_LANG_WELSH,
	HBM_LANG_OKINAWAN, */
	HBM_LANG_COUNT, // Total number of language entries, do not touch!

	HBM_LANG_TAMAZIGHT_KAB,
	HBM_LANG_TAMAZIGHT_ZGH,
};

bool HBM_SetLanguage(enum HBM_LANG value);

/**
 * Determines whether to show the unsaved data message.
 * This can also be changed inline using HBM_SetUnsaved() (see below).
 *
 * @param value Can be set to 0 (disabled), 1 (enabled for "Reset" only), or 2 (enabled for "Wii Menu" and "Reset").
 **/
void HBM_SetUnsaved(int value);

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

// Texture and vertex attributes for rendering
// ******************************
#define HBM_GX_VTXFMT   GX_VTXFMT1 /* max: 7 */

// #define HBM_GX_COLOR1A1

// Other settings
// ******************************
#define HBM_WIDTH					608
#define HBM_HEIGHT					480
#define HBM_MAX_POINTERS			4 /* max: 4 */
#define HBM_WIDESCREEN_RATIO		(832.0/608.0)
#define HBM_EASEINOUT(x)			(x * x * (3.0f - 2.0f * x))
#define HBM_MAX_GLYPHS_SANSSERIF	64
#define HBM_MAX_GLYPHS_SERIF		48
#define HBM_ELEMENT_COUNT			20
#define HBM_VOLUME					164 /* max: 256 */

// Use u32 instead of u64 ticks_to_millisecs(gettime())
#define HBM_GETTIME ((u32)(gettick())/(u32)(TB_TIMER_CLOCK))

// Structs, enums
// ******************************
enum HBM_STATUS {
	HBM_INACTIVE,		// Idle state
	HBM_NOHOME,			// Used for displaying NoHome icon
	HBM_OPENING,		// Used at opening slide animation
	HBM_OPEN,			// Open state
	HBM_CLOSING,		// Used at closing slide animation
	HBM_CLOSED			// Used after closing slide animation, signals to switch back to HBM_INACTIVE
};

enum HBM_STAGE {
	HBM_STAGE_MAIN		= 0,
	HBM_STAGE_WPAD		= 1 << 1,
	HBM_STAGE_BLOCKED	= 1 << 2,
	HBM_STAGE_DIALOG	= 1 << 3,
};

enum HBM_TEXTALIGNH {
	HBM_TEXT_LEFT,
	HBM_TEXT_CENTER,
	HBM_TEXT_RIGHT,
};

enum HBM_TEXTALIGNV {
	HBM_TEXT_TOP,
	HBM_TEXT_MIDDLE,
	HBM_TEXT_BOTTOM
};

struct HBM_CONFIG {
	enum HBM_STATUS Status;
	int Stage;

	float ScaleX;
	float ScaleY;
	int Width;
	int Height;

	int Host_TEX0;
	int Host_TEVSTAGE0;

	bool Widescreen;
	bool ShowManualButton;
	enum HBM_LANG Language;
	int Unsaved;
};

struct HBM_EXITTRANSITION {
	int Type;
	float Fade;
};

// Filelist
// ******************************
// Images
extern const uint8_t HBM_cursor1_png[];
extern const uint8_t HBM_cursor2_png[];
extern const uint8_t HBM_cursor3_png[];
extern const uint8_t HBM_cursor4_png[];
extern const uint8_t HBM_cursor_shadow_png[];
extern const uint8_t HBM_dialogBG_png[];
extern const uint8_t HBM_dialogButton_mask_png[];
extern const uint8_t HBM_dialogButton_png[];
extern const uint8_t HBM_header_png[];
extern const uint8_t HBM_headerHighlighted_png[];
extern const uint8_t HBM_mainButton_mask_png[];
extern const uint8_t HBM_mainButton_png[];
extern const uint8_t HBM_noHome_png[];
extern const uint8_t HBM_remote_png[];
extern const uint8_t HBM_remoteDataBG_png[];
extern const uint8_t HBM_remoteBattery_0_png[];
extern const uint8_t HBM_remoteBattery_1_png[];
extern const uint8_t HBM_remoteBattery_2_png[];
extern const uint8_t HBM_remoteBattery_3_png[];
extern const uint8_t HBM_remoteBattery_4_png[];
extern const uint8_t HBM_topHeaderButton_png[];

// Sounds
extern const uint8_t HBM_sfx_cancel_pcm[];
extern const uint8_t HBM_sfx_cancel_pcm_end[];
extern const uint8_t HBM_sfx_confirm_pcm[];
extern const uint8_t HBM_sfx_confirm_pcm_end[];
extern const uint8_t HBM_sfx_dialog_pcm[];
extern const uint8_t HBM_sfx_dialog_pcm_end[];
extern const uint8_t HBM_sfx_hover_pcm[];
extern const uint8_t HBM_sfx_hover_pcm_end[];
extern const uint8_t HBM_sfx_menuclose_pcm[];
extern const uint8_t HBM_sfx_menuclose_pcm_end[];
extern const uint8_t HBM_sfx_menuopen_pcm[];
extern const uint8_t HBM_sfx_menuopen_pcm_end[];
extern const uint8_t HBM_sfx_select_pcm[];
extern const uint8_t HBM_sfx_select_pcm_end[];
extern const uint8_t HBM_sfx_sync1_pcm[];
extern const uint8_t HBM_sfx_sync1_pcm_end[];
extern const uint8_t HBM_sfx_sync2_pcm[];
extern const uint8_t HBM_sfx_sync2_pcm_end[];
extern const uint8_t HBM_sfx_sync3_pcm[];
extern const uint8_t HBM_sfx_sync3_pcm_end[];
extern const uint8_t HBM_sfx_sync4_pcm[];
extern const uint8_t HBM_sfx_sync4_pcm_end[];
extern const uint8_t HBM_sfx_syncend_pcm[];
extern const uint8_t HBM_sfx_syncend_pcm_end[];

// Classes
// ******************************
#include "hbm/classes/HBMImage.h"
#include "hbm/classes/HBMElement.h"
#include "hbm/classes/HBMHeader.h"
#include "hbm/classes/HBMButton.h"
#include "hbm/classes/HBMButtonMain.h"
#include "hbm/classes/HBMDialogButton.h"
#include "hbm/classes/HBMDialog.h"
#include "hbm/classes/HBMRemoteDataSprite.h"
#include "hbm/classes/HBMPointerImage.h"

#include "hbm/console.h"
#include "hbm/font.h"
#include "hbm/wpad.h"

#endif