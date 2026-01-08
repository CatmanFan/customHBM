#include "hbm.h"

// static void* HBM_Thread(void *arg);
// static lwp_t HBM_mainthread = LWP_THREAD_NULL;

Mtx HBM_GXmodelView2D;
struct HBM_CONFIG HBM_Settings;

static bool HBM_isAllowed = true;
static bool HBM_initialized = false;

static powercallback origPowerCallback;
static resetcallback origResetCallback;

static void* fb[2];
static int fb_i;

static void *HBM_backgroundBuffer;
static HBMImage HBM_background;
static float HBM_backgroundOpacity = 0;

#define HBM_ELEMENT_COUNT 30
static HBMElement *HBM_allElements[HBM_ELEMENT_COUNT];
static HBMButtonMain HBM_wiiMenuButton, HBM_resetButton;
static HBMDialog HBM_dialog;
static HBMPointerImage HBM_pointer1, HBM_pointer2, HBM_pointer3, HBM_pointer4;

static HBMImage HBM_noHomeIcon;

static f64 HBM_timeElapsed;
static f64 HBM_timeStopwatch;

static int HBM_exitType;
static float HBM_exitFade;

/** Declare at main **/
void HBM_HideMenu();

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
 *                   POWER FUNCTIONS                  *
 ******************************************************/

static void HBM_TriggerShutdown()
{
	HBM_dialog.Block(true);
	HBM_ConsolePrintf("Exiting: 3 (Shutdown)");
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_BLOCKED) && HBM_exitType == 0)
		HBM_exitType = 3;
}

static void HBM_TriggerReset()
{
	HBM_dialog.Block(true);
	HBM_ConsolePrintf("Exiting: 2 (Reset)");
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_BLOCKED) && HBM_exitType == 0)
		HBM_exitType = 2;
}

static void HBM_TriggerSystemMenu()
{
	HBM_dialog.Block(true);
	HBM_ConsolePrintf("Exiting: 1 (System Menu)");
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_BLOCKED) && HBM_exitType == 0)
		HBM_exitType = 1;
}

static void HBM_PromptReset()
{
	HBM_dialog.Confirm = HBM_TriggerReset;
	HBM_dialog.UpdateText(
		(char *)HBM_gettextmsg(HBM_Settings.Unsaved > 0 ? "Reset the software? (Anything not saved will be lost.)" : "Reset the software?"),
		(char *)HBM_gettextmsg("Yes"),
		(char *)HBM_gettextmsg("No")
	);
	HBM_dialog.Show();
}

static void HBM_PromptSystemMenu()
{
	HBM_dialog.Confirm = HBM_TriggerSystemMenu;
	HBM_dialog.UpdateText(
		(char *)HBM_gettextmsg(HBM_Settings.Unsaved > 1 ? "Return to the Wii Menu? (Anything not saved will be lost.)" : "Return to the Wii Menu?"),
		(char *)HBM_gettextmsg("Yes"),
		(char *)HBM_gettextmsg("No")
	);
	HBM_dialog.Show();
}

static void HBM_HandleShutdown() { HBM_TriggerShutdown(); }
static void HBM_HandleWPADShutdown(s32 chan) { HBM_TriggerShutdown(); }
static void HBM_HandleReset(u32 chan, void* arg) { HBM_TriggerReset(); }

static void HBM_SoundInit()
{
	#if (HBM_SOUND_OUTPUT == 1)
		ASND_Init();
		ASND_Pause(0);
	#endif
}

static void HBM_SoundStop()
{
	#if (HBM_SOUND_OUTPUT == 1)
		ASND_Pause(1);
	#endif
}

void HBM_PlaySound(const void* pcm, size_t pcm_size)
{
	#if (HBM_SOUND_OUTPUT == 1)
		s32 voice = ASND_GetFirstUnusedVoice();
		if (voice >= 0)
			ASND_SetVoice(voice, VOICE_STEREO_16BIT, 48000, 0, (void *)pcm, pcm_size, 128, 128, NULL);
	#endif
}

/******************************************************
 *                HBM SETTINGS CONTROL                *
 ******************************************************/

static void HBM_elementPositions() {
	HBM_wiiMenuButton.SetPosition (HBM_Settings.Widescreen ? 94 : 34, 180); // 172, 168
	HBM_resetButton.SetPosition (HBM_Settings.Widescreen ? 470 : 310, HBM_wiiMenuButton.Y);
}

void HBM_SetWidescreen(bool value) {
	HBM_Settings.Widescreen = value;
	HBM_Settings.ScaleX = (f32)HBM_Settings.Width / 640;
	HBM_Settings.ScaleY = 1 /* 480 / (f32)HBM_Settings.Height */;
	// **********************
	// FORCE SET TO 1
	// HBM_Settings.ScaleX = HBM_Settings.ScaleY = 1;
	// **********************
	if (value)
		HBM_Settings.ScaleX /= HBM_WIDESCREEN_RATIO;

	if (HBM_initialized)
		HBM_elementPositions();
}

void HBM_SetLanguage(int value) {
	if (!HBM_initialized) {
		if (value != HBM_Settings.Language)
			HBM_Settings.Language = value;
	} else {
		HBM_LoadLanguage(value);
		HBM_Settings.Language = HBM_GetCurrentLanguage();

		HBM_wiiMenuButton.Text = (char *)HBM_gettextmsg("Wii Menu");
		HBM_resetButton.Text = (char *)HBM_gettextmsg("Reset");
		HBM_dialog.UpdateText(
			(char *)HBM_gettextmsg(HBM_dialog.Confirm == HBM_TriggerReset ? HBM_Settings.Unsaved > 0 ? "Reset the software? (Anything not saved will be lost.)" : "Reset the software?"
																		  : HBM_Settings.Unsaved > 1 ? "Return to the Wii Menu? (Anything not saved will be lost.)" : "Return to the Wii Menu?"),
			(char *)HBM_gettextmsg("Yes"),
			(char *)HBM_gettextmsg("No")
		);
	}
}

void HBM_SetUnsaved(int value) {
	if (value != HBM_Settings.Unsaved)
		HBM_Settings.Unsaved = value;
}

/******************************************************
 *                MANAGEMENT FUNCTIONS                *
 ******************************************************/

void HBM_Init(int width, int height, int host_TEVSTAGE0, int host_TEX0)
{
	if (HBM_initialized) return;

	// LWP_CreateThread(&HBM_mainthread, HBM_Thread, NULL, NULL, 0, 70);
	HBM_initialized = true;

	HBM_Settings.Status = HBM_INACTIVE;
	HBM_Settings.InteractionLayer = HBM_INTERACTION_MAIN;
	HBM_Settings.ScaleY = HBM_Settings.ScaleX = 0;

	HBM_Settings.Width = width;
	HBM_Settings.Height = height == 576 ? 480 : height;

	#ifdef HBM_FORCE_ASPECT_RATIO
		#if (HBM_FORCE_ASPECT_RATIO == 1)
			HBM_SetWidescreen(true);
		#else
			HBM_SetWidescreen(false);
		#endif
	#else
		HBM_SetWidescreen(CONF_GetAspectRatio() == CONF_ASPECT_16_9);
	#endif
	#ifdef HBM_LANGUAGE
		HBM_Settings.Language = HBM_LANGUAGE;
	#else
		HBM_Settings.Language = -1;
	#endif
	#ifdef HBM_UNSAVED
		HBM_Settings.Unsaved = HBM_UNSAVED;
	#else
		HBM_Settings.Unsaved = 0;
	#endif

	// Setup target vertex descriptor
	GX_SetVtxAttrFmt(HBM_GX_VTXFMT, GX_VA_POS, GX_POS_XY, GX_F32, 0); // Positions given in 2 f32's (f32 x, f32 y)
	GX_SetVtxAttrFmt(HBM_GX_VTXFMT, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0); // Texture coordinates given in 2 f32's
	GX_SetVtxAttrFmt(HBM_GX_VTXFMT, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	// GX compatibility mode
	HBM_Settings.Host_TEX0 = host_TEX0;
	HBM_Settings.Host_TEVSTAGE0 = host_TEVSTAGE0;

	// Setup 2D model view
	guMtxIdentity(HBM_GXmodelView2D);
	guMtxTransApply(HBM_GXmodelView2D, HBM_GXmodelView2D, 0.0F, 0.0F, -50.0F);
	GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);

	// Declare simple images
	HBM_backgroundBuffer = memalign(32, GX_GetTexBufferSize(width, height, GX_TF_RGBA8, GX_FALSE, 1));
	HBM_background.LoadRaw(HBM_backgroundBuffer, width, height);
	HBM_background.SetPosition(0, 0);
	HBM_background.Visible = true;
	HBM_background.FixedSize = true;

	HBM_noHomeIcon.LoadPNG(&HBM_noHome_png, 52, 52);
	HBM_noHomeIcon.SetPosition(50, /*42*/54);
	HBM_noHomeIcon.Visible = true;

	// Declare elements
	HBM_elementPositions();
	/** HBM_wiiMenuButton **/
	HBM_wiiMenuButton.Selected = HBM_PromptSystemMenu;
	HBM_wiiMenuButton.Visible = true;

	/** HBM_resetButton **/
	HBM_resetButton.Selected = HBM_PromptReset;
	HBM_resetButton.Visible = true;

	/** Pointers **/
	HBM_pointer1.Load(1, NULL);
	HBM_pointer2.Load(2, &HBM_pointer1);
	HBM_pointer3.Load(3, &HBM_pointer1);
	HBM_pointer4.Load(4, &HBM_pointer1);

	/** Elements list **/
	HBM_allElements[0] = &HBM_wiiMenuButton;
	HBM_allElements[1] = &HBM_resetButton;
	HBM_allElements[25] = &HBM_dialog;
	HBM_allElements[26] = &HBM_pointer4;
	HBM_allElements[27] = &HBM_pointer3;
	HBM_allElements[28] = &HBM_pointer2;
	HBM_allElements[29] = &HBM_pointer1;

	// Initialize the remainder of the libraries
	HBM_SoundInit();
	HBM_ConsoleInit();
	HBM_RomfsInit();
	HBM_FontInit();
	HBM_SetLanguage(HBM_Settings.Language);

	HBM_timeElapsed = 0;
	HBM_timeStopwatch = -1;
	HBM_exitType = 0;
	HBM_exitFade = 0;
}

void HBM_Uninit()
{
	if (!HBM_initialized) return;

	HBM_FontUninit();
	HBM_RomfsUninit();

	if (HBM_backgroundBuffer != NULL) {
		free(MEM_K1_TO_K0(HBM_backgroundBuffer));
		HBM_backgroundBuffer = NULL;
	}

	HBM_initialized = false;
}

void HBM_ToggleUsage(bool value)
{
	if (HBM_Settings.Status == HBM_INACTIVE)
		HBM_isAllowed = value;
}

/******************************************************
 *                 INTERNAL FUNCTIONS                 *
 ******************************************************/

void HBM_DrawBlackQuad(int x, int y, int width, int height, float percentage, bool noWidescreen)
{
	f32 x1 = (f32)x * (noWidescreen && HBM_Settings.Widescreen ? HBM_Settings.ScaleX * HBM_WIDESCREEN_RATIO : HBM_Settings.ScaleX);
	f32 y1 = (f32)y * HBM_Settings.ScaleY;
	f32 x2 = (f32)width * (noWidescreen && HBM_Settings.Widescreen ? HBM_Settings.ScaleX * HBM_WIDESCREEN_RATIO : HBM_Settings.ScaleX);
	f32 y2 = (f32)height * HBM_Settings.ScaleY;

	float f[4][2] = {{x1, y1}, {x1 + x2, y1}, {x1 + x2, y1 + y2}, {x1, y1 + y2}};

	// Turn off texturing, otherwise crash (taken from HomeMenu.c)
	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

	GX_Begin(GX_TRIANGLEFAN, HBM_GX_VTXFMT, 4);
	for (int i = 0; i < 4; i++) {
		GX_Position2f32(f[i][0], f[i][1]);
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
	/** Failsafe **/
	if (HBM_Settings.Host_TEVSTAGE0 < 0 || HBM_Settings.Host_TEX0 < 0) return;

	// Background
	HBM_background.Draw();
	HBM_DrawBlackQuad(0, 0, HBM_Settings.Width, HBM_Settings.Height, HBM_backgroundOpacity, true);

	// Elements
	for (int i = 0; i < HBM_ELEMENT_COUNT; i++) {
		if (HBM_allElements[i] != NULL)
			HBM_allElements[i]->Draw();
	}

	// Pointer square
	#ifdef HBM_DEBUG
	if (HBMPointers[0].Status > 0) {
		float x = (HBMPointers[0].X - 1.0F) * HBM_Settings.ScaleX,
			  x2 = (HBMPointers[0].X + 1.0F) * HBM_Settings.ScaleX,
			  y = (HBMPointers[0].Y - 1.0F) * HBM_Settings.ScaleY,
			  y2 = (HBMPointers[0].Y + 1.0F) * HBM_Settings.ScaleY;
		float f[4][2] = {{x, y}, {x2, y}, {x2, y2}, {x, y2}};

		GX_Begin(GX_TRIANGLEFAN, HBM_GX_VTXFMT, 4);
		for (int i = 0; i < 4; i++) {
			GX_Position2f32(f[i][0], f[i][1]);
			GX_Color4u8(255, 0, 0, 255);
		}
		GX_End();
	}
	#endif

	// Console
	HBM_ConsoleDraw();

	// Fade
	if (HBM_exitFade > 0)
		HBM_DrawBlackQuad(0, 0, HBM_Settings.Width, HBM_Settings.Height, HBM_exitFade, true);
}

static void HBM_AfterDraw()
{
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(fb[fb_i],GX_TRUE);
	GX_DrawDone();

	// Increment only if we are using more than one buffer
	if (fb[1] != NULL) fb_i ^= 1;
	if (fb[fb_i] != NULL) VIDEO_SetNextFramebuffer(fb[fb_i]);

	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
}

static void HBM_Update()
{
	if (HBM_Settings.Status != HBM_CLOSING && HBM_Settings.Status != HBM_CLOSED) {
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
	}

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

			if (HBM_exitType < 3) {
				for (int i = 0; i < MAX_SND_VOICES; i++) {
					s32 vol = lround((float)MAX_VOLUME * (1.0F - (HBM_timeElapsed / 0.5F)));
					ASND_ChangeVolumeVoice(i, vol, vol);
				}
			} else {
				if (HBM_exitFade > 0.15F)
					HBM_SoundStop();
			}
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
						HBM_backgroundOpacity = (99.0F / 255.0F) * HBM_EASEINOUT(progress);
						HBM_wiiMenuButton.SetOpacity(HBM_EASEINOUT(progress));
						HBM_resetButton.SetOpacity(HBM_EASEINOUT(progress));
					}
				}
				break;

			/*********************************************************
			  Idle state
			**********************************************************/
			case HBM_OPEN:
				{
					HBM_ConsolePrintf2("Resolution: %d x %d", HBM_Settings.Width, HBM_Settings.Height);

					if (WPAD_ButtonsHeld(0) & (WPAD_BUTTON_PLUS))
						HBM_Settings.ScaleX += 0.002F;
					else if (WPAD_ButtonsHeld(0) & (WPAD_BUTTON_MINUS))
						HBM_Settings.ScaleX -= 0.002F;
					else if (WPAD_ButtonsDown(0) & (WPAD_BUTTON_1))
						HBM_SetWidescreen(HBM_Settings.Widescreen ? false : true);
					else if (WPAD_ButtonsDown(0) & (WPAD_BUTTON_B))
						HBM_SetLanguage(HBM_GetCurrentLanguage() + 1);

					for (int i = 0; i < WPAD_MAX_WIIMOTES; i++)
					{
						if (WPAD_ButtonsDown(i) & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)
							&& HBM_Settings.InteractionLayer == HBM_INTERACTION_MAIN)
							HBM_HideMenu();
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

void HBM_Menu()
{
	if (!HBM_initialized || HBM_Settings.Status != HBM_INACTIVE) return;

	// Pre-show functions
	if (HBM_isAllowed)
	{
		HBM_ConsolePrintf("HBM status: Opening");

		// Init video
		fb[0] = VIDEO_GetCurrentFramebuffer();
		fb[1] = VIDEO_GetNextFramebuffer();

		// Init GX
		if (HBM_Settings.Host_TEVSTAGE0 >= 0 && HBM_Settings.Host_TEX0 >= 0) {
			GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
			GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
		}
		// GX_SetScissorBoxOffset(0, 0);
		// GX_SetScissor(0, 0, HBM_Settings.Width, HBM_Settings.Height);

		// take a screenshot to be used as our background
		GX_SetTexCopySrc(0, 0, HBM_Settings.Width, HBM_Settings.Height);
		GX_SetTexCopyDst(HBM_Settings.Width, HBM_Settings.Height, GX_TF_RGBA8, GX_FALSE);
		GX_CopyTex(HBM_backgroundBuffer, GX_FALSE);
		GX_PixModeSync();
		DCFlushRange(HBM_backgroundBuffer, GX_GetTexBufferSize(HBM_Settings.Width, HBM_Settings.Height, GX_TF_RGBA8, GX_FALSE, 1));
		HBM_background.LoadRaw(HBM_backgroundBuffer, HBM_Settings.Width, HBM_Settings.Height);

		// Init system power callbacks
		origPowerCallback = SYS_SetPowerCallback(HBM_HandleShutdown);
		origResetCallback = SYS_SetResetCallback(HBM_HandleReset);
		WPAD_SetPowerButtonCallback(HBM_HandleWPADShutdown);

		// Set initial animation values
		HBM_timeElapsed = 0;
		HBM_timeStopwatch = -1;

		// Main loop
		HBM_PointerInit();
		HBM_Settings.Status = HBM_OPENING;
		while (HBM_Settings.Status != HBM_INACTIVE)
		{
			HBM_Update();
			HBM_ConsoleUpdate();

			/****************** DRAW ******************/
			HBM_Draw();
			HBM_AfterDraw();
		}

		// Exiting ...
		// HBM_background.Free();

		// Restore old GX mode
		if (HBM_Settings.Host_TEVSTAGE0 >= 0 && HBM_Settings.Host_TEX0 >= 0) {
			GX_SetTevOp(GX_TEVSTAGE0, HBM_Settings.Host_TEVSTAGE0);
			GX_SetVtxDesc(GX_VA_TEX0, HBM_Settings.Host_TEX0);
		}

		// Restore old callbacks
		if (origPowerCallback != NULL) SYS_SetPowerCallback(origPowerCallback);
		if (origResetCallback != NULL) SYS_SetResetCallback(origResetCallback);
	}

	else
	{
		HBM_Settings.Status = HBM_BLOCKED;
	}
}

void HBM_HideMenu()
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