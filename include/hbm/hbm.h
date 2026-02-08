#ifndef __HBM_FULLINCLUDE__
#define __HBM_FULLINCLUDE__

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

// Texture and vertex attributes for rendering (max: 7)
// ******************************
#define HBM_GX_VTXFMT   GX_VTXFMT1
#define HBM_GX_VTXFMT2  GX_VTXFMT2

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
	HBM_CLOSED,			// Used after closing slide animation, signals to switch back to HBM_INACTIVE
	HBM_EXITED			// Used to signify that the Wii Menu/Loader/Reset button has been pressed
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
	int Language;
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
#ifdef __cplusplus
#include "hbm/classes/HBMImage.h"
#include "hbm/classes/HBMElement.h"
#include "hbm/classes/HBMButton.h"
#include "hbm/classes/HBMButtonMain.h"
#include "hbm/classes/HBMDialogButton.h"
#include "hbm/classes/HBMDialog.h"
#include "hbm/classes/HBMHeader.h"
#include "hbm/classes/HBMPointerImage.h"
#include "hbm/classes/HBMRemoteDataSprite.h"
extern "C" {
#else
typedef struct HBMImage HBMImage;
typedef struct HBMElement HBMElement;
typedef struct HBMButton HBMButton;
typedef struct HBMButtonMain HBMButtonMain;
typedef struct HBMDialogButton HBMDialogButton;
typedef struct HBMDialog HBMDialog;
typedef struct HBMHeader HBMHeader;
typedef struct HBMPointerImage HBMPointerImage;
typedef struct HBMRemoteDataSprite HBMRemoteDataSprite;
#endif

HBMElement *HBM_HBMElement_create();
void HBM_HBMElement_destroy(HBMElement *c);
void HBM_HBMElement_update(HBMElement *c);
void HBM_HBMElement_draw(HBMElement *c);

HBMImage *HBM_HBMImage_create();
void HBM_HBMImage_destroy(HBMImage *c);
void HBM_HBMImage_draw(HBMImage *c);

HBMHeader *HBM_HBMHeader_create();
void HBM_HBMHeader_destroy(HBMHeader *c);
void HBM_HBMHeader_draw(HBMHeader *c);

HBMButtonMain *HBM_HBMButtonMain_create();
void HBM_HBMButtonMain_destroy(HBMButtonMain *c);
void HBM_HBMButtonMain_draw(HBMButtonMain *c);

HBMDialogButton *HBM_HBMDialogButton_create();
void HBM_HBMDialogButton_destroy(HBMDialogButton *c);
void HBM_HBMDialogButton_draw(HBMDialogButton *c);

HBMDialog *HBM_HBMDialog_create();
void HBM_HBMDialog_destroy(HBMDialog *c);
void HBM_HBMDialog_draw(HBMDialog *c);

HBMPointerImage *HBM_HBMPointerImage_create();
void HBM_HBMPointerImage_destroy(HBMPointerImage *c);
void HBM_HBMPointerImage_draw(HBMPointerImage *c);

HBMRemoteDataSprite *HBM_HBMRemoteDataSprite_create();
void HBM_HBMRemoteDataSprite_destroy(HBMRemoteDataSprite *c);
void HBM_HBMRemoteDataSprite_draw(HBMRemoteDataSprite *c, int X, int Y);

#ifdef __cplusplus
}
#endif

#include "hbm/console.h"
#include "hbm/font.h"
#include "hbm/wpad.h"

#endif