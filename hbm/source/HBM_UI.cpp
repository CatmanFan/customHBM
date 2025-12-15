#include "HBM.h"

// static void* HBM_Thread(void *arg);
// static lwp_t HBM_mainthread = LWP_THREAD_NULL;

struct HBM_CONFIG HBM_Settings;
Mtx HBM_GXmodelView2D;

static bool HBM_isAllowed = true;
static bool HBM_initialized = false;

static powercallback origPowerCallback;
static resetcallback origResetCallback;

static void* fb[2];
static int fb_i;

static HBMImage HBM_background;
static HBMImage HBM_noHomeIcon;

#define HBM_ELEMENT_COUNT 30
static HBMElement *HBM_allElements[HBM_ELEMENT_COUNT];
static HBMButtonMain HBM_wiiMenuButton, HBM_resetButton;
static HBMElement HBM_pointer1;
static HBMElement HBM_pointer2;
static HBMElement HBM_pointer3;
static HBMElement HBM_pointer4;

static void* HBM_backgroundSource = NULL;
static float HBM_backgroundOpacity = 0;

static f64 HBM_timeElapsed;
static f64 HBM_timeStopwatch;

static int HBM_exitType;
static float HBM_exitFade;

/******************************************************
 *                   TIME FUNCTIONS                   *
 ******************************************************/
static bool HBM_TimeWait(f64 value)
{
	if (HBM_timeStopwatch < 0)
		HBM_timeStopwatch = ((f64)ticks_to_millisecs(gettime()) / 1000.0F);

	HBM_timeElapsed = ((f64)ticks_to_millisecs(gettime()) / 1000.0F) - HBM_timeStopwatch;

	if (HBM_timeElapsed >= value && value >= 0)
	{
		HBM_timeElapsed = 0;
		HBM_timeStopwatch = -1;
		return true;
	}
	return false;
}


/******************************************************
 *                SCREENSHOT FUNCTIONS                *
 ******************************************************/

static bool HBM_InitScreenshot()
{
	if (!HBM_initialized) return false;

	// Init screenshot
	if (HBM_backgroundSource == NULL)
		HBM_backgroundSource = memalign(32, GX_GetTexBufferSize(HBM_Settings.Width, HBM_Settings.Height, GX_TF_RGBA8, GX_FALSE, 1));

	return true;
}

static void HBM_TakeScreenshot()
{
	if (!HBM_InitScreenshot()) return;

	GX_SetTexCopySrc(0, 0, HBM_Settings.Width, HBM_Settings.Height);
	GX_SetTexCopyDst(HBM_Settings.Width, HBM_Settings.Height, GX_TF_RGBA8, GX_FALSE);
	GX_CopyTex(HBM_backgroundSource, false);
	GX_PixModeSync();
	DCFlushRange(HBM_backgroundSource, GX_GetTexBufferSize(HBM_Settings.Width, HBM_Settings.Height, GX_TF_RGBA8, GX_FALSE, 1));

	HBM_background.LoadRaw(HBM_backgroundSource, HBM_Settings.Width, HBM_Settings.Height);
}

static void HBM_DeleteScreenshot()
{
	if (!HBM_initialized) return;

	if (HBM_backgroundSource != NULL)
	{
		free(MEM_K1_TO_K0(HBM_backgroundSource));
		HBM_background.LoadRaw(NULL, HBM_Settings.Width, HBM_Settings.Height);
	}
}

static void HBM_TriggerShutdown()
{
	HBM_ConsolePrintf("Exiting: 3 (Shutdown)");
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_BLOCKED) && HBM_exitType == 0)
		HBM_exitType = 3;
}

static void HBM_TriggerReset()
{
	HBM_ConsolePrintf("Exiting: 2 (Reset)");
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_BLOCKED) && HBM_exitType == 0)
		HBM_exitType = 2;
}

static void HBM_TriggerSystemMenu()
{
	HBM_ConsolePrintf("Exiting: 1 (System Menu)");
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_BLOCKED) && HBM_exitType == 0)
		HBM_exitType = 1;
}

static void HBM_HandleShutdown() { HBM_TriggerShutdown(); }
static void HBM_HandleWPADShutdown(s32 chan) { HBM_TriggerShutdown(); }
static void HBM_HandleReset(u32 chan, void* arg) { HBM_TriggerReset(); }

static void HBM_ResetStrings()
{
	HBM_wiiMenuButton.Text = (char *)HBM_gettextmsg("Wii Menu");
	HBM_resetButton.Text = (char *)HBM_gettextmsg("Reset");
}

void HBM_PlaySound(const void* pcm, size_t pcm_size)
{
	#ifdef HBM_ENABLE_SOUND

	s32 voice = ASND_GetFirstUnusedVoice();
	if (voice >= 0)
		ASND_SetVoice(voice, VOICE_STEREO_16BIT, 48000, 0, (void *)pcm, pcm_size, 128, 128, NULL);

	#endif
}

static void HBM_InitSound()
{
	#ifdef HBM_ENABLE_SOUND
		ASND_Init();
		ASND_Pause(0);
	#endif
}

/******************************************************
 *                MANAGEMENT FUNCTIONS                *
 ******************************************************/

void HBM_Init(int width, int height, bool widescreen)
{
	// LWP_CreateThread(&HBM_mainthread, HBM_Thread, NULL, NULL, 0, 70);
	HBM_initialized = true;

	HBM_Settings.Status = HBM_INACTIVE;
	HBM_Settings.Width = width;
	HBM_Settings.Height = height;
	HBM_Settings.ScaleX = (f32)width / 608;
	HBM_Settings.ScaleY = (f32)height / 480;
	HBM_Settings.Widescreen = widescreen;
	if (HBM_Settings.Widescreen)
		HBM_Settings.ScaleX *= (640.0F / 834.0F);

	// **********************
	// FORCE SET TO 1
	// HBM_Settings.ScaleX = HBM_Settings.ScaleY = 1;
	// **********************

	HBM_RomfsInit();
	HBM_InitSound();

	// Declare simple images
	fb[0] = VIDEO_GetNextFramebuffer();
	fb[1] = VIDEO_GetCurrentFramebuffer();
	// HBM_InitScreenshot();
	// HBM_background.LoadRaw(HBM_backgroundSource, width, height);
	HBM_background.LoadEFB(width, height);
	HBM_background.SetPosition(0, 0);
	HBM_background.Visible = true;
	// HBM_background.NoWidescreen = false;

	HBM_noHomeIcon.LoadPNG(&HBM_noHome_png, 52, 52);
	HBM_noHomeIcon.SetPosition(50, /*42*/54);
	HBM_noHomeIcon.Visible = true;

	// Declare elements
	/** HBM_wiiMenuButton **/
	{
		HBM_wiiMenuButton.SetPosition (34, 180); // 172, 168
		HBM_wiiMenuButton.Selected = HBM_TriggerSystemMenu;
		HBM_wiiMenuButton.Visible = true;
	}

	/** HBM_resetButton **/
	{
		HBM_resetButton.SetPosition (310, HBM_wiiMenuButton.Y);
		HBM_resetButton.Selected = HBM_TriggerReset;
		HBM_resetButton.Visible = true;
	}

	HBM_pointer1.Image.LoadPNG(&HBM_cursor1_png, 64, 114);
	HBM_pointer2.Image.LoadPNG(&HBM_cursor2_png, 64, 64);
	HBM_pointer3.Image.LoadPNG(&HBM_cursor3_png, 64, 64);
	HBM_pointer4.Image.LoadPNG(&HBM_cursor4_png, 64, 64);
	HBM_pointer1.Image.SetAnchorPoint(9, 27);
	HBM_pointer2.Image.SetAnchorPoint(9, 27);
	HBM_pointer3.Image.SetAnchorPoint(9, 27);
	HBM_pointer4.Image.SetAnchorPoint(9, 27);
	HBM_pointer1.Visible = HBM_pointer2.Visible = HBM_pointer3.Visible = HBM_pointer4.Visible = false;
	HBM_pointer1.Image.Scale = HBM_pointer2.Image.Scale = HBM_pointer3.Image.Scale = HBM_pointer4.Image.Scale = 0.92F;

	HBM_allElements[0] = &HBM_wiiMenuButton;
	HBM_allElements[1] = &HBM_resetButton;
	HBM_allElements[26] = &HBM_pointer4;
	HBM_allElements[27] = &HBM_pointer3;
	HBM_allElements[28] = &HBM_pointer2;
	HBM_allElements[29] = &HBM_pointer1;

	guMtxIdentity(HBM_GXmodelView2D);
	guMtxTransApply(HBM_GXmodelView2D, HBM_GXmodelView2D, 0.0F, 0.0F, -50.0F);
	GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);
	HBM_ConsoleInit();

	HBM_LoadLanguage(HBM_UI_LANGUAGE);
	HBM_Settings.Language = HBM_GetCurrentLanguage();
	HBM_ResetStrings();

	HBM_FontInit();
	HBM_FontReload(HBM_Settings.Language == CONF_LANG_KOREAN ? 1 : 0);

	HBM_timeElapsed = 0;
	HBM_timeStopwatch = -1;
	HBM_exitType = 0;
	HBM_exitFade = 0;
}

void HBM_Uninit()
{
	HBM_DeleteScreenshot();
	HBM_FontUninit();
	HBM_RomfsUninit();
}

void HBM_ToggleUsage(bool value)
{
	if (HBM_Settings.Status == HBM_INACTIVE)
		HBM_isAllowed = value;
}

/******************************************************
 *                 INTERNAL FUNCTIONS                 *
 ******************************************************/

void HBM_DrawBlackQuad(int x, int y, int width, int height, float percentage)
{
	f32 x2 = x + width * HBM_Settings.ScaleX, y2 = y + height * HBM_Settings.ScaleY;
	float f[4][2] = {{(f32)x, (f32)y}, {x2, (f32)y}, {x2, y2}, {(f32)x, y2}};

	GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
	for (int i = 0; i < 4; i++) {
	#ifdef HBM_USE_3D_RENDER
		GX_Position3f32(f[i][0], f[i][1], 0);
	#else
		GX_Position2f32(f[i][0], f[i][1]);
	#endif
	#ifdef HBM_DEBUG
		GX_Color4u8(0, 128, 0, lround((percentage > 1 ? 1 : percentage) * 255.0F));
	#else
		GX_Color4u8(0,  0,  0, lround((percentage > 1 ? 1 : percentage) * 255.0F));
	#endif
	}
	GX_End();
}

static void HBM_Draw()
{
	HBM_background.Draw();
	HBM_DrawBlackQuad(0, 0, HBM_Settings.Width, HBM_Settings.Height, HBM_backgroundOpacity);

	#ifdef HBM_USE_3D_RENDER
	for (int i = 0; i < HBM_ELEMENT_COUNT; i++) {
		if (HBM_allElements[i] != NULL)
			HBM_allElements[i]->Draw();
	}

	#ifdef HBM_DEBUG
	if (HBMPointers[0].Status > 0) {
		float x = HBMPointers[0].X - 1.0F,
			  x2 = HBMPointers[0].X + 1.0F,
			  y = HBMPointers[0].Y - 1.0F,
			  y2 = HBMPointers[0].Y + 1.0F;
		float f[4][2] = {{x, y}, {x2, y}, {x2, y2}, {x, y2}};

		GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
		for (int i = 0; i < 4; i++) {
			#ifdef HBM_USE_3D_RENDER
				GX_Position3f32(f[i][0], f[i][1], 0);
			#else
				GX_Position2f32(f[i][0], f[i][1]);
			#endif
			GX_Color4u8(255, 0, 0, 255);
		}
		GX_End();
	}
	#endif

	HBM_ConsoleDraw();

	if (HBM_exitFade > 0)
		HBM_DrawBlackQuad(0, 0, HBM_Settings.Width, HBM_Settings.Height, HBM_exitFade);
	#endif
}

static void HBM_AfterDraw()
{
	// Increment only if we are using more than one buffer
	if (fb[1] != NULL) fb_i ^= 1;

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(fb[fb_i], GX_TRUE);
	GX_DrawDone();

	if (fb[fb_i] != NULL) VIDEO_SetNextFramebuffer(fb[fb_i]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	// GX_InvalidateTexAll();

}

static void HBM_Update()
{
	HBM_PointerUpdate();

	// Visibility based on pointer's availability
	HBM_pointer1.Visible = HBMPointers[0].Status > 0;
	HBM_pointer2.Visible = HBMPointers[1].Status > 0;
	HBM_pointer3.Visible = HBMPointers[2].Status > 0;
	HBM_pointer4.Visible = HBMPointers[3].Status > 0;

	// Position based on pointer location
	HBM_pointer1.SetPosition(HBMPointers[0].X - 9 - 13, HBMPointers[0].Y - 27 - 5);
	HBM_pointer2.SetPosition(HBMPointers[1].X, HBMPointers[1].Y);
	HBM_pointer3.SetPosition(HBMPointers[2].X, HBMPointers[2].Y);
	HBM_pointer4.SetPosition(HBMPointers[3].X, HBMPointers[3].Y);

	// Rotation based on pitch
	HBM_pointer1.Image.Rotation = HBMPointers[0].Rotation;
	HBM_pointer2.Image.Rotation = HBMPointers[1].Rotation;
	HBM_pointer3.Image.Rotation = HBMPointers[2].Rotation;
	HBM_pointer4.Image.Rotation = HBMPointers[3].Rotation;

	if (HBM_exitType > 0)
	{
		/*********************************************************
		  Control menu exit after fading is completed
		**********************************************************/
		HBM_TimeWait(-1);
		if (HBM_exitFade > 1)
		{
			VIDEO_SetBlack(true);
			switch (HBM_exitType)
			{
				default:
				case 1:
					SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
					break;
				case 2:
					SYS_ResetSystem(SYS_HOTRESET, 0, 0);
					break;
				case 3:
					SYS_ResetSystem(SYS_POWEROFF, 0, 0);
					break;
			}
		}
		/*********************************************************
		  Fading animation when exiting to System Menu or power
		**********************************************************/
		else
		{
			if (HBM_Settings.Status == HBM_OPEN) {
				for (int i = 0; i < HBM_ELEMENT_COUNT; i++) {
					if (HBM_allElements[i] != NULL)
						HBM_allElements[i]->Update();
				}
			}

			HBM_exitFade = HBM_timeElapsed / 0.5F;

			#ifdef HBM_ENABLE_SOUND
			if (HBM_exitFade > 0.15F && HBM_exitType >= 3)
				ASND_Pause(1);
			#endif
		}
	}
	else
	{
		switch (HBM_Settings.Status)
		{
			/*********************************************************
			  Sliding animation
			  Do not accept any input during this time
			**********************************************************/
			case HBM_OPENING:
			case HBM_CLOSING:
				{
					float progress = HBM_Settings.Status == HBM_CLOSING ? 1 - (HBM_timeElapsed / 0.3333) : HBM_timeElapsed / 0.3333;

					if (HBM_TimeWait(0.3333)) {
						if (HBM_Settings.Status == HBM_OPENING)
							HBM_PlaySound(HBM_sfx_menuopen_pcm, HBM_sfx_menuopen_pcm_size);

						HBM_backgroundOpacity = 99.0F / 255.0F;
						HBM_wiiMenuButton.SetOpacity(1);
						HBM_resetButton.SetOpacity(1);

						progress = HBM_Settings.Status == HBM_CLOSING ? 0 : 1;
						HBM_Settings.Status = progress < 0.5 ? HBM_CLOSED : HBM_OPEN;
						HBM_ConsolePrintf(HBM_Settings.Status == HBM_CLOSED ? "HBM status: Closed" : "HBM status: Opened");
					} else {
						HBM_backgroundOpacity = (99.0F / 255.0F) * progress;
						HBM_wiiMenuButton.SetOpacity(progress);
						HBM_resetButton.SetOpacity(progress);
					}
				}
				break;

			/*********************************************************
			  Idle state
			**********************************************************/
			case HBM_OPEN:
				{
					for (int i = 0; i < WPAD_MAX_WIIMOTES; i++)
					{
						if (WPAD_ButtonsDown(i) & (WPAD_BUTTON_B)) {
							HBM_LoadLanguage(HBM_GetCurrentLanguage() + 1);
							HBM_Settings.Language = HBM_GetCurrentLanguage();
							HBM_ResetStrings();
							HBM_FontReload(HBM_Settings.Language == CONF_LANG_KOREAN ? 1 : 0);
						}
						if (WPAD_ButtonsDown(i) & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)
							&& HBM_Settings.InteractionLayer == 0)
							HBM_Hide();
					}

					for (int i = 0; i < HBM_ELEMENT_COUNT; i++) {
						if (HBM_allElements[i] != NULL)
							HBM_allElements[i]->Update();
					}
				}
				break;

			/*********************************************************
			  Closed state
			  Should automatically set to inactive
			**********************************************************/
			default:
			case HBM_CLOSED:
				HBM_timeElapsed = 0;
				HBM_Settings.Status = HBM_INACTIVE;
				return;
		}
	}
}

void HBM_Open()
{
	if (!HBM_initialized || (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_BLOCKED)) return;

	// Pre-show functions
	if (HBM_isAllowed)
	{
		// Init system power callbacks
		origPowerCallback = SYS_SetPowerCallback(HBM_HandleShutdown);
		origResetCallback = SYS_SetResetCallback(HBM_HandleReset);
		WPAD_SetPowerButtonCallback(HBM_HandleWPADShutdown);

		// Init video
		GX_SetScissorBoxOffset(0, 0);
		GX_SetScissor(0, 0, HBM_Settings.Width, HBM_Settings.Height);

		HBM_PointerInit();

		HBM_background.LoadEFB(HBM_Settings.Width, HBM_Settings.Height);
		// HBM_TakeScreenshot();
		HBM_ConsolePrintf("HBM status: Opening");

		// Set initial animation values
		HBM_timeElapsed = 0;
		HBM_timeStopwatch = -1;

		// Trigger loop
		HBM_Settings.Status = HBM_OPENING;
		while (HBM_Settings.Status != HBM_INACTIVE)
		{
			HBM_Update();
			HBM_ConsoleUpdate();

			/****************** DRAW ******************/
			HBM_Draw();
			HBM_AfterDraw();
		}

		HBM_background.Free();
		if (origPowerCallback != NULL) SYS_SetPowerCallback(origPowerCallback);
		if (origResetCallback != NULL) SYS_SetResetCallback(origResetCallback);
	}

	else
	{
		HBM_Settings.Status = HBM_BLOCKED;
	}
}

void HBM_Hide()
{
	if (!HBM_initialized || HBM_Settings.Status != HBM_OPEN || HBM_exitType > 0) return;

	HBM_ConsolePrintf("HBM status: Closing");
	HBM_PlaySound(HBM_sfx_menuclose_pcm, HBM_sfx_menuclose_pcm_size);
	HBM_Settings.Status = HBM_CLOSING;
}

void HBM_DrawNoHome()
{
	if (!HBM_initialized) return;

	if (HBM_Settings.Status == HBM_BLOCKED)
	{
		if (HBM_timeStopwatch < 0)
			HBM_timeStopwatch = ((f64)ticks_to_millisecs(gettime()) / 1000.0F);

		HBM_timeElapsed = ((f64)ticks_to_millisecs(gettime()) / 1000.0F) - HBM_timeStopwatch;

		HBM_noHomeIcon.A = HBM_timeElapsed < 0.233 ? lround(255.0F * (HBM_timeElapsed / 0.233))
							 : HBM_timeElapsed > 0.233 + 1.2 ? lround(255.0F * (1.0F - ((HBM_timeElapsed - 0.233 - 1.2) / 0.233)))
							 : 255;

		if (HBM_timeElapsed >= 1.6667)
		{
			HBM_timeStopwatch = -1;
			HBM_timeElapsed = 0;
			HBM_noHomeIcon.A = 0;
			HBM_Settings.Status = HBM_INACTIVE;
			return;
		}

		HBM_noHomeIcon.Draw();
	}
}

#undef HBM_ELEMENT_COUNT