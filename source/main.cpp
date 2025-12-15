#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "HBM.h"

//! Use a predefined graphics library. Uncomment only ONE.
// #define GFX_MODE 1	// 0: Libwiisprite
					// 1: GRRLIB
#ifdef HBM_USE_3D_RENDER
#define GFX_MODE 1
#else
#define GFX_MODE 0
#endif

#if GFX_MODE == 1
	/* **********************************
	   **            GRRLIB            **
	   ********************************** */

	#include <grrlib.h>

	#define SPRITE_CREATE(NAME)			GRRLIB_texImg* NAME;
	#define SPRITE_LOAD(NAME, FILE)		NAME = GRRLIB_LoadTexturePNG(FILE);
	#define SPRITE_UNLOAD(NAME)			GRRLIB_FreeTexture(NAME);

	/* ********************************** */
#else
	/* **********************************
	   **         LIBWIISPRITE         **
	   ********************************** */

	// libwiisprite uses wsp as it's namespace
	#include <wiisprite.h>
	using namespace wsp;

	#define SPRITE_CREATE(NAME)			Image NAME ## _img; \
										Sprite NAME;
	#define SPRITE_LOAD(NAME, FILE)		NAME ## _img.LoadImage(FILE); \
										NAME.SetImage(&NAME ## _img);
	#define SPRITE_UNLOAD(NAME)			{ ; }

	// Sprite layout
	GameWindow gwd;
	LayerManager manager(3);
	Quad quad;

	/* ********************************** */
#endif

#include "HBM_cursor1_png.h"
#include "HBM_reference_png.h"
SPRITE_CREATE(cursor)
SPRITE_CREATE(HBM_reference)

static int color = 0;

static void initVideo()
{
	#if GFX_MODE == 1
	/* **********************************
	   **            GRRLIB            **
	   ********************************** */

	// Initialise the Graphics & Video subsystem
	GRRLIB_Init();

	SPRITE_LOAD(HBM_reference, HBM_reference_png);
	SPRITE_LOAD(cursor, HBM_cursor1_png);

	/* ********************************** */
	#else
	/* **********************************
	   **         LIBWIISPRITE         **
	   ********************************** */

	// Create the game window and initalise the VIDEO subsystem
	gwd.InitVideo();

	SPRITE_LOAD(HBM_reference, HBM_reference_png);
	SPRITE_LOAD(cursor, HBM_cursor1_png);

	manager.Append(&HBM_reference);
	manager.Append(&cursor);

	/* ********************************** */
	#endif
}

#include <freetype/ftimage.h>
#include "times_ttf.h"
#include "HBM_RGFX_Font_png.h"

#if GFX_MODE == 1
	GRRLIB_ttfFont *font = NULL;
#else
	Sprite txt_spr;
#endif

static void drawText(int width, int height, int x, int y, int size, const char *string, ...)
{
	char buffer[1024];
    va_list argp;
    va_start(argp, string);
    vsnprintf(buffer, sizeof(buffer), string, argp);
    va_end(argp);

	#if GFX_MODE == 1
	/* **********************************
	   **            GRRLIB            **
	   ********************************** */

	if (font == NULL) font = GRRLIB_LoadTTF(times_ttf, times_ttf_size);
	GRRLIB_PrintfTTF(x, y, font, buffer, size, 0xFFFFFFFF);

	/* ********************************** */
	#else
	/* **********************************
	   **         LIBWIISPRITE         **
	   ********************************** */

	ftImage txt(width, height);

	txt.setFont(times_ttf, times_ttf_size);
	txt.setSize(size);
	txt.setColor(255,255,255);
	txt.printf(buffer);
	txt.flush();

	txt_spr.SetImage(&txt);
	txt_spr.SetPosition(x,y);
	txt_spr.Draw();

	/* ********************************** */
	#endif
}

static void drawGfx()
{
	#if GFX_MODE == 1
		/* **********************************
		   **            GRRLIB            **
		   ********************************** */
		if (color == 0) GRRLIB_FillScreen(RGBA(0,  0,  128,255));
		if (color == 1) GRRLIB_FillScreen(RGBA(255,0,  0,  255));
		if (color == 2) GRRLIB_FillScreen(RGBA(0,  0,  255,255));
		if (color == 3) GRRLIB_FillScreen(RGBA(0,  128,0,  255));
		if (color == 4) GRRLIB_FillScreen(RGBA(0,  0,  0,  255));
		if (color == 5) GRRLIB_FillScreen(RGBA(255,255,255,255));

		GRRLIB_DrawImg(0, 0, HBM_reference, 0, 1, 1, 0xFFFFFFFF);

		GRRLIB_Rectangle(0, 0, 52, 52, RGBA(255,255,255,255), 1);
		GRRLIB_Rectangle(1, 1, 50, 50, RGBA(0,0,0,255), 1);

	/* ********************************** */
	#else
		/* **********************************
		   **         LIBWIISPRITE         **
		   ********************************** */

		if (color == 0) gwd.SetBackground((GXColor){ 0, 0, 128, 255 });
		if (color == 1) gwd.SetBackground((GXColor){ 255, 0, 0, 255 });
		if (color == 2) gwd.SetBackground((GXColor){ 0, 0, 255, 255 });
		if (color == 3) gwd.SetBackground((GXColor){ 0, 128, 0, 255 });
		if (color == 4) gwd.SetBackground((GXColor){ 0, 0, 0, 255 });
		if (color == 5) gwd.SetBackground((GXColor){ 255, 255, 255, 255 });

		HBM_reference.SetImage(&HBM_reference_img);
		HBM_reference.SetPosition(0, 0);
		HBM_reference.Draw();

		quad.SetPosition(0, 0);
		quad.SetWidth(52);
		quad.SetHeight(52);
		quad.SetBorder(false);
		quad.SetFillColor((GXColor){255, 255, 255, 255});
		quad.Draw();

		quad.SetPosition(1, 1);
		quad.SetWidth(50);
		quad.SetHeight(50);
		quad.SetBorder(false);
		quad.SetFillColor((GXColor){0, 0, 0, 255});
		quad.Draw();

		/* ********************************** */
	#endif
}

struct ir_t IR0, IR1, IR2, IR3;

static void updateCursor()
{
	WPAD_IR(WPAD_CHAN_0, &IR0);
	// WPAD_IR(WPAD_CHAN_1, &IR1);
	// WPAD_IR(WPAD_CHAN_2, &IR2);
	// WPAD_IR(WPAD_CHAN_3, &IR3);

	if (IR0.valid)
	{
	#if GFX_MODE == 1
	/* **********************************
	   **            GRRLIB            **
	   ********************************** */

		GRRLIB_DrawImg(IR0.sx-200, IR0.sy-250, cursor, IR0.angle, 1, 1, 0xFFFFFFFF);

	/* ********************************** */
	#else
	/* **********************************
	   **         LIBWIISPRITE         **
	   ********************************** */

		// Give our sprite the positions and the angle.
		cursor.SetImage(&cursor_img);
		cursor.SetPosition(IR0.sx-200, IR0.sy-250); // We use these constants to translate the position correctly to the screen
		cursor.Move(-((f32)cursor.GetWidth()/2), -((f32)cursor.GetHeight()/2)); // And these to make our image appear at the center of this position.
		cursor.SetRotation(IR0.angle/2); // Set angle/2 to translate correctly
		cursor.Draw();

	/* ********************************** */
	#endif
	}
}

static void endVideo()
{
    SPRITE_UNLOAD(cursor);
    SPRITE_UNLOAD(HBM_reference);

	#if GFX_MODE == 1
	GRRLIB_Exit();
	#else // Libwiisprite
	gwd.StopVideo();
	#endif
}

static int fadeType = 0;
static int fadeValue = 0;

static void drawFade()
{
	switch (fadeType)
	{
		default:
			break;

		case 1:
		case 2:
			#if GFX_MODE == 1
			/* **********************************
			   **            GRRLIB            **
			   ********************************** */

			GRRLIB_Rectangle(-100, -100, 800, 800, RGBA(0,0,0,fadeValue), 1);

			/* ********************************** */
			#else
			/* **********************************
			   **         LIBWIISPRITE         **
			   ********************************** */

			quad.SetPosition(-100, -100);
			quad.SetWidth(800);
			quad.SetHeight(800);
			quad.SetBorder(false);
			quad.SetFillColor((GXColor){0, 0, 0, fadeValue});
			quad.Draw();

			/* ********************************** */
			#endif
			break;
	}
}

static void beginFade(bool in)
{
	if (in)
	{
		fadeType = 1;
		fadeValue = 0xFF;
	}
	else
	{
		fadeType = 2;
		fadeValue = 0;
	}
}

static int controlFade()
{
	#if GFX_MODE == 1
		#define FADE_INCREMENT 12
	#else
		#define FADE_INCREMENT 24
	#endif

	int outcome = -1;

	switch (fadeType)
	{
		case 0:
		default:
			break;

		case 1: // Fade in
			fadeValue = fadeValue - FADE_INCREMENT;
			if (fadeValue < 0) fadeValue = 0;
			break;

		case 2: // Fade out
			fadeValue = fadeValue + FADE_INCREMENT;
			if (fadeValue > 0xFF) fadeValue = 0xFF;
			break;
	}

	if ((fadeType == 1 && fadeValue <= 0) || (fadeType == 2 && fadeValue >= 0xFF))
	{
		outcome = fadeType;
		fadeType = 0;
	}

	return outcome;

	#undef FADE_INCREMENT
}

static int HWButton = -1;

static void resetRequest(u32 chan, void* arg)
{
	beginFade(false);
	HWButton = SYS_HOTRESET;
}
static void powerRequest()
{
	beginFade(false);
	HWButton = SYS_POWEROFF;
}
static void wpadPowerRequest(s32 chan)
{
	beginFade(false);
	HWButton = SYS_POWEROFF;
}

static void crash()
{
	// TLB exception on load/instruction fetch
	long e1 = *(long *)1;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	// Initialise the controllers
	PAD_Init();
	WPAD_Init();
	ASND_Init();

	SYS_SetPowerCallback(powerRequest);
	SYS_SetResetCallback(resetRequest);
	WPAD_SetPowerButtonCallback(wpadPowerRequest);

	initVideo();
	beginFade(true);

	// Initialise IR
	#if GFX_MODE == 1
	WPAD_SetVRes(WPAD_CHAN_0, rmode->viWidth, rmode->viHeight);
	HBM_Init(rmode->viWidth, rmode->viHeight, CONF_GetAspectRatio() == CONF_ASPECT_16_9);
	#else
	WPAD_SetVRes(WPAD_CHAN_0, gwd.GetWidth(), gwd.GetHeight());
	HBM_Init(gwd.GetWidth(), gwd.GetHeight(), CONF_GetAspectRatio() == CONF_ASPECT_16_9);
	#endif
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	HBM_ToggleUsage(true);

	while(controlFade() != 2)
	{
		// If [HOME] was pressed on the first Wiimote, break out of the loop
		if (WPAD_ButtonsDown(0) & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME))
		{
			if (fadeType == 0)
			{
				HBM_Open();

				#if GFX_MODE == 1
				WPAD_SetVRes(WPAD_CHAN_ALL, rmode->viWidth, rmode->viHeight);
				#else
				WPAD_SetVRes(WPAD_CHAN_ALL, gwd.GetWidth(), gwd.GetHeight());
				#endif
				WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
				WPAD_SetPowerButtonCallback(wpadPowerRequest);
			}
		}

		// If [POWER] or [RESET] was pressed, break out of the loop
		// if (HWButton != -1) break;

		PAD_ScanPads();
		WPAD_ScanPads(); // Scan the Wiimotes

		// ---------------------------------------------------------------------

		if ((WPAD_ButtonsDown(0) & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A))
		 || (PAD_ButtonsDown(0)  & (PAD_BUTTON_A)))
		{
			color = (color + 1) % 6;
		}

		if ((WPAD_ButtonsDown(0) & (WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS))
		 || (PAD_ButtonsDown(0)  & (PAD_BUTTON_START)))
		{
			beginFade(false);
		}

		if ((WPAD_ButtonsDown(0) & (WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS))
		 || (PAD_ButtonsDown(0)  & (PAD_TRIGGER_Z)))
		{
			crash();
		}

		// ---------------------------------------------------------------------
		// Place your drawing code here
		// ---------------------------------------------------------------------
		drawGfx();

		#if GFX_MODE == 1
			drawText(640, 100, 60, 120, 100, "hi");
			drawText(640, 480, 80, 200, 15, "Press HOME to test function");
			drawText(640, 480, 80, 220, 15, "Press A to change background color");
			drawText(640, 480, 80, 240, 15, "Press - to crash");
			drawText(640, 480, 80, 270, 15, "Window size: %d x %d", rmode->viWidth, rmode->viHeight);
			drawText(640, 480, 80, 310, 15, "System language: %d", CONF_GetLanguage());
		#endif

		updateCursor();
		drawFade();
		HBM_DrawNoHome();

		// Render the frame buffer to the TV
		#if GFX_MODE == 1
			GRRLIB_Render();
		#else
			gwd.Flush();
		#endif
	}

	VIDEO_SetBlack(true);
	// HBM_Uninit();
	if (HWButton != -1) { SYS_ResetSystem(HWButton, 0, 0); }
	endVideo();  // This will free any resources used by the video library
	exit(0);
}
