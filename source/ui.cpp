#include "hbm.h"

// static void* HBM_Thread(void *arg);
// static lwp_t HBM_mainthread = LWP_THREAD_NULL;

Mtx HBM_GXmodelView2D;
struct HBM_CONFIG HBM_Settings;
struct HBM_EXITTRANSITION HBM_ExitTransition;

static bool HBM_isAllowed = true;
static bool HBM_initialized = false;

static powercallback origPowerCallback;
static resetcallback origResetCallback;

// Callbacks:
static void (*HBM_BeforeShowMenu)();	// called just before starting menu-display animation
static void (*HBM_AfterShowMenu)();		// called just after finishing menu-display animation
static void (*HBM_BeforeDraw)();		// called at the very beginning of the menu loop
static void (*HBM_AfterDraw)();			// called at the very end of the menu loop
static void (*HBM_BeforeHideMenu)();	// called just before starting menu-hide animation
static void (*HBM_AfterHideMenu)();		// called just after finishing menu-hide animation
static void (*HBM_BeforeExit)();		// called just before exiting to Wii system menu or HBC
static void (*HBM_MyReset)();			// called at "reset"

// Callback Setters:
void HBM_SetBeforeShowMenu(void (*func)()) { HBM_BeforeShowMenu = func; }
void HBM_SetAfterShowMenu(void (*func)()) { HBM_AfterShowMenu = func; }
void HBM_SetBeforeDraw(void (*func)()) { HBM_BeforeDraw = func; }
void HBM_SetAfterDraw(void (*func)()) { HBM_AfterDraw = func; }
void HBM_SetBeforeHideMenu(void (*func)()) { HBM_BeforeHideMenu = func; }
void HBM_SetAfterHideMenu(void (*func)()) { HBM_AfterHideMenu = func; }
void HBM_SetBeforeExit(void (*func)()) { HBM_BeforeExit = func; }
void HBM_SetMyReset(void (*func)()) { HBM_MyReset = func; }

static void* fb[2];
static int fb_i;

static HBMElement *HBM_allElements[HBM_ELEMENT_COUNT];
static HBMButtonMain HBM_wiiMenuButton, HBM_resetButton, HBM_manualButton;
static HBMDialog HBM_dialog;
static HBMPointerImage HBM_pointer1, HBM_pointer2, HBM_pointer3, HBM_pointer4;
static HBMHeader HBM_topHeader, HBM_bottomHeader;
static HBMImage HBM_topHeaderButton, HBM_remote, HBM_remoteDataBG;
extern HBMRemoteDataSprite HBM_remoteData[4];
static const char* HBM_topHeaderButtonText;

static HBMImage HBM_noHomeIcon;

/** Declare at main **/
void HBM_HideMenu();

/******************************************************
 *                SCREENSHOT FUNCTIONS                *
 ******************************************************/

static HBMImage HBM_background;
static u8 * HBM_backgroundScreenshot = NULL;
static float HBM_backgroundOpacity = 0;
static bool HBM_backgroundCreated = false;

void HBM_TakeScreenshot() {
	if (!HBM_backgroundCreated) {
		HBM_backgroundScreenshot = (u8 *)memalign(32, 320 * 240 * 4);

		if (HBM_backgroundScreenshot) {
			GX_SetTexCopySrc(0, 0, 320, 240);
			GX_SetTexCopyDst(320, 240, GX_TF_RGBA8, GX_FALSE);
			DCInvalidateRange(HBM_backgroundScreenshot, 320 * 240 * 4);
			GX_CopyTex(HBM_backgroundScreenshot, GX_FALSE);
			GX_PixModeSync();

			HBM_background.LoadRaw(HBM_backgroundScreenshot, 320, 240);
			HBM_backgroundCreated = true;
		}
	}
}

static void HBM_FreeScreenshot() {
	if (HBM_backgroundCreated) {
		HBM_background.Free();
		free(HBM_backgroundScreenshot);
		HBM_backgroundScreenshot = NULL;
		HBM_backgroundCreated = false;
	}
}

/******************************************************
 *                   TIME FUNCTIONS                   *
 ******************************************************/
static f64 HBM_timeElapsed;
static f64 HBM_timeStopwatch;

static f64 HBM_aboutTime;

static void HBM_TimeClear()
{
	HBM_timeStopwatch = -1;
	HBM_timeElapsed = 0;
	HBM_aboutTime = 0;
}

static void HBM_TimeUpdate()
{
	if (HBM_timeStopwatch < 0)
		HBM_timeStopwatch = ((f64)HBM_GETTIME / 1000.0F);

	HBM_timeElapsed = ((f64)HBM_GETTIME / 1000.0F) - HBM_timeStopwatch;

	while (HBM_timeElapsed < -1) {
		HBM_ConsolePrintf("ui timer is negative!! resetting");
		HBM_ConsolePrintf("(timer: %.5f)", HBM_timeElapsed);
		HBM_ConsolePrintf("(timer snap: %.5f)", HBM_timeStopwatch);
		HBM_timeStopwatch = ((f64)HBM_GETTIME / 1000.0F);
		HBM_timeElapsed = ((f64)HBM_GETTIME / 1000.0F) - HBM_timeStopwatch;
	}
}

/******************************************************
 *                  GLOBAL FUNCTIONS                  *
 ******************************************************/
void HBM_DrawQuad(int x, int y, int width, int height, u8 shade, float alpha, bool noWidescreen)
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
			GX_Color4u8(shade > 0 ? shade : 0, shade > 0 ? shade : 128, shade > 0 ? shade : 0, lround((alpha > 1 ? 1 : alpha) * 255.0F));
		#else
			GX_Color4u8(shade, shade, shade, lround((alpha > 1 ? 1 : alpha) * 255.0F));
		#endif
	}
	GX_End();
}

static void HBM_SoundSetVolume(float value)
{
	#if (HBM_SOUND_OUTPUT == 1)
	for (int i = 0; i < MAX_SND_VOICES; i++) {
		s32 vol = lround((float)HBM_VOLUME * value);
		ASND_ChangeVolumeVoice(i, vol, vol);
	}
	#endif
}

static void HBM_SoundStop()
{
	#if (HBM_SOUND_OUTPUT == 1)
		for (int i = 0; i < MAX_SND_VOICES; i++)
			ASND_StopVoice(i);
	#endif
}

static void HBM_SoundInit()
{
	#if (HBM_SOUND_OUTPUT == 1)
		ASND_Init();
		ASND_Pause(0);
	#endif
}

static void HBM_SoundUninit()
{
	#if (HBM_SOUND_OUTPUT == 1)
		ASND_Pause(1);
	#endif
}

void HBM_PlaySound(const void* pcm, const void* pcm_end, bool lowRate)
{
	#if (HBM_SOUND_OUTPUT == 1)
		s32 voice = ASND_GetFirstUnusedVoice();
		if (voice >= 0)
			ASND_SetVoice(voice, VOICE_STEREO_16BIT, lowRate ? 32000 : 48000, 0, (void *)pcm, (size_t)pcm_end - (size_t)pcm, HBM_VOLUME, HBM_VOLUME, NULL);
	#endif
}

/******************************************************
 *                    ERROR SCREEN                    *
 ******************************************************/
#ifdef HBM_USE_ERROR_SCREEN

#include <cstring>
#include <stdarg.h>

static char HBM_ErrorText[256];
static int HBM_ErrorStatus = 0;

void HBM_Error(const char* text, ...)
{
	va_list argp;
	va_start(argp, text);
	vsnprintf(HBM_ErrorText, 256, text, argp);
	va_end(argp);

	HBM_TimeClear();
	HBM_ErrorStatus = 1;
}

static void HBM_ErrorUpdate()
{
	if (HBM_ErrorStatus <= 0) return;
	if (HBM_ErrorStatus == 1) {
		// Wait for a single frame before stopping sound
		HBM_SoundUninit();
		HBM_ErrorStatus = 2;
		return;
	}

	if (HBM_ErrorStatus < 4)
		HBM_TimeUpdate();

	// Tells the text to fade in
	if (HBM_timeElapsed >= 0.4 && HBM_ErrorStatus < 3) {
		extern void HBM_FontSetForError();
		HBM_FontSetForError();

		// Reset timer
		HBM_TimeClear();

		HBM_ErrorStatus = 3;
	}

	// Stops the timer and remains fixed until poweroff
	if (HBM_timeElapsed >= 0.333 && HBM_ErrorStatus == 3) {
		HBM_ErrorStatus = 4;
		HBM_TimeClear();
	}
}

static void HBM_ErrorDraw()
{
	if (HBM_ErrorStatus <= 1) return;

	// Draw black fade
	if (HBM_ErrorStatus == 2)
		HBM_DrawQuad(0, 0, HBM_WIDTH, HBM_HEIGHT, 0, HBM_timeElapsed < 0.350 ? HBM_timeElapsed / 0.350 : 1, true);

	// Draw text
	if (HBM_ErrorStatus >= 3) {
		for (int i = 0, max = 2; i < max; i++) {
			HBM_DrawText
			(
				/* text */		HBM_ErrorText,
				/* X */			(HBM_Settings.Widescreen ? HBM_WIDTH * HBM_WIDESCREEN_RATIO : HBM_WIDTH) / 2 - 0.33,
				/* Y */			238,
				/* size */		22,
				/* scaleX */	0.931,
				/* scaleY */	1.015,
				/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
				/* serif */		false,
				/* color */		220, 220, 220, 255,
				/* maxWidth */	520 /* 456 */
			);
		}

		// Draw black fade
		if (HBM_ErrorStatus == 3)
			HBM_DrawQuad(0, 0, HBM_WIDTH, HBM_HEIGHT, 0, HBM_timeElapsed < 0.333 ? 1 - (HBM_timeElapsed / 0.333) : 0, true);
	}
}

#endif // HBM_USE_ERROR_SCREEN

/******************************************************
 *                   POWER FUNCTIONS                  *
 ******************************************************/

static void HBM_TriggerShutdown()
{
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_NOHOME) && HBM_ExitTransition.Type == 0) {
		HBM_ConsolePrintf("Exiting: 3 (Shutdown)");
		HBM_Settings.Stage |= HBM_STAGE_BLOCKED;
		HBM_ExitTransition.Type = 3;
	}
}

static void HBM_TriggerReset()
{
#ifdef HBM_USE_ERROR_SCREEN
	if (HBM_ErrorStatus > 0) return;
#endif

	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_NOHOME) && HBM_ExitTransition.Type == 0) {
		HBM_ConsolePrintf("Exiting: 2 (Reset)");
		HBM_Settings.Stage |= HBM_STAGE_BLOCKED;
		HBM_ExitTransition.Type = 2;
	}
}

static void HBM_TriggerSystemMenu()
{
	if (HBM_initialized && (HBM_Settings.Status != HBM_INACTIVE && HBM_Settings.Status != HBM_NOHOME) && HBM_ExitTransition.Type == 0) {
		HBM_ConsolePrintf("Exiting: 1 (System Menu)");
		HBM_Settings.Stage |= HBM_STAGE_BLOCKED;
		HBM_ExitTransition.Type = 1;
	}
}

static void HBM_HandleShutdown() { HBM_TriggerShutdown(); }
static void HBM_HandleWPADShutdown(s32 chan) { HBM_TriggerShutdown(); }
static void HBM_HandleReset(u32 chan, void* arg) { HBM_TriggerReset(); }

/******************************************************
 *                      DIALOGS                       *
 ******************************************************/

static void HBM_PromptReset()
{
	HBM_dialog.Confirm = HBM_TriggerReset;
	HBM_dialog.SlideFromTop = true;
	HBM_dialog.AltAppearance = false;
	HBM_dialog.UpdateText(
		HBM_gettextmsg(HBM_MyReset == NULL ? "This will quit the game. Proceed? (Anything not saved will be lost.)" :
					   HBM_Settings.Unsaved > 0 ? "Reset the software? (Anything not saved will be lost.)" : "Reset the software?"),
		HBM_gettextmsg("Yes"),
		HBM_gettextmsg("No")
	);
	HBM_dialog.Show();
}

static void HBM_PromptSystemMenu()
{
	HBM_dialog.Confirm = HBM_TriggerSystemMenu;
	HBM_dialog.SlideFromTop = true;
	HBM_dialog.AltAppearance = false;
	HBM_dialog.UpdateText(
		HBM_gettextmsg(HBM_Settings.Unsaved > 1 ? "Return to the Wii Menu? (Anything not saved will be lost.)" : "Return to the Wii Menu?"),
		HBM_gettextmsg("Yes"),
		HBM_gettextmsg("No")
	);
	HBM_dialog.Show();
}

static void HBM_PromptUnimplemented()
{
#ifdef HBM_USE_ERROR_SCREEN
	HBM_Error(HBM_gettextmsg("This feature is currently unavailable."));
#else
	HBM_dialog.Confirm = NULL;
	HBM_dialog.SlideFromTop = false;
	HBM_dialog.AltAppearance = true;
	HBM_dialog.UpdateText(
		HBM_gettextmsg("This feature is currently unavailable."),
		// "Miiがとうろくされていません。\nMii",
		HBM_gettextmsg("OK")
	);
	HBM_dialog.Show();
#endif
}

static void HBM_PromptAbout()
{
	HBM_dialog.Confirm = NULL;
	HBM_dialog.SlideFromTop = false;
	HBM_dialog.AltAppearance = true;
	HBM_dialog.UpdateText(
		"libhbm 1.0 (by Mr. Lechkar)\n\nTranslations by Mr. Lechkar, TVGZone,\nAngel333119, CaXaP, Wiicat\nand Graj Po Polsku",
		HBM_gettextmsg("OK")
	);
	HBM_dialog.Show();
}

/******************************************************
 *                HBM ELEMENTS CONTROL                *
 ******************************************************/

static void HBM_elementPositions() {
	HBM_wiiMenuButton.SetPosition (HBM_Settings.Widescreen ? 94 : 34, HBM_Settings.ShowManualButton ? 110 : 180);
	HBM_resetButton.SetPosition (HBM_Settings.Widescreen ? 471 : 310, HBM_wiiMenuButton.Y);
	HBM_manualButton.SetPosition (HBM_Settings.Widescreen ? 283 : 172, 234);

	HBM_topHeader.TextX = 28.7;
	HBM_topHeader.TextY = 67.28;
	HBM_bottomHeader.TextX = 324.25 + (HBM_Settings.Widescreen ? 118.75 : 0);
	HBM_bottomHeader.TextY = 34.8;
}

static void HBM_elementLoad() {
	HBM_topHeaderButton.LoadPNG(&HBM_topHeaderButton_png, 184, 48);
	HBM_remote.LoadPNG(&HBM_remote_png, 76, 300);
	HBM_remoteDataBG.LoadPNG(&HBM_remoteDataBG_png, 436, 32);
	HBM_bottomHeader.WiiRemote = &HBM_remote;
}

static void HBM_elementFree() {
	HBM_FreeScreenshot();

	HBM_topHeaderButton.Free();
	HBM_bottomHeader.WiiRemote = NULL;
	HBM_remote.Free();
	HBM_remoteDataBG.Free();
}

static void HBM_elementSetup() {
	/** HBM_background **/
	HBM_background.SetPosition(0, 0);
	HBM_background.R = 255;
	HBM_background.G = 255;
	HBM_background.B = 255;
	HBM_background.A = 255;
	HBM_background.Visible = true;
	// HBM_background.FixedSize = true;

	/** HBM_wiiMenuButton **/
	HBM_wiiMenuButton.Selected = HBM_PromptSystemMenu;
	HBM_wiiMenuButton.Visible = true;

	/** HBM_resetButton **/
	HBM_resetButton.Selected = HBM_PromptReset;
	HBM_resetButton.Visible = true;

	/** HBM_manualButton **/
	HBM_manualButton.Selected = HBM_PromptUnimplemented;
	HBM_manualButton.Visible = true;

	/** HBM_topHeader **/
	HBM_topHeader.AfterSelected = HBM_HideMenu;
	HBM_topHeader.Visible = true;
	HBM_topHeader.Inverted = true;
	HBM_topHeader.TextRectWidth = 368 /* 380 */;
	HBM_topHeader.TextRectHeight = 40;
	HBM_topHeader.TextCentered = false;
	HBM_topHeader.TextSize = 36;
	HBM_topHeader.TextOpacity = 1;

	/** HBM_bottomHeader **/
	HBM_bottomHeader.Selected = HBM_PromptUnimplemented;
	HBM_bottomHeader.Visible = true;
	HBM_bottomHeader.Inverted = false;
	HBM_bottomHeader.TextRectWidth = 420;
	HBM_bottomHeader.TextRectHeight = 32;
	HBM_bottomHeader.TextCentered = true;
	HBM_bottomHeader.TextSize = 25.75;
	HBM_bottomHeader.TextOpacity = 1;

	HBM_topHeaderButton.Visible = true;
	HBM_remote.Visible = true;
	HBM_remoteDataBG.Visible = true;

	/** Pointers and dialog **/
	HBM_pointer1.Load(1, NULL);
	HBM_pointer2.Load(2, &HBM_pointer1);
	HBM_pointer3.Load(3, &HBM_pointer1);
	HBM_pointer4.Load(4, &HBM_pointer1);
	HBM_dialog.Reset();

	/** Elements list **/
	HBM_allElements[0] = &HBM_wiiMenuButton;
	HBM_allElements[1] = &HBM_resetButton;
	HBM_allElements[2] = HBM_Settings.ShowManualButton ? &HBM_manualButton : NULL;
	HBM_allElements[HBM_ELEMENT_COUNT - 2] = &HBM_topHeader;
	HBM_allElements[HBM_ELEMENT_COUNT - 1] = &HBM_bottomHeader;
}

static void HBM_SlideAnimation(float progress)
{
	HBM_backgroundOpacity = (99.0F / 255.0F) * HBM_EASEINOUT(progress);
	HBM_topHeader.Y = -10 - (90 * (1 - HBM_EASEINOUT(progress)));
	HBM_bottomHeader.Y = 376 + (115 * (1 - HBM_EASEINOUT(progress)));
	HBM_wiiMenuButton.SetOpacity(HBM_EASEINOUT(progress));
	HBM_resetButton.SetOpacity(HBM_EASEINOUT(progress));
	HBM_manualButton.SetOpacity(HBM_EASEINOUT(progress));
}

/******************************************************
 *                HBM SETTINGS CONTROL                *
 ******************************************************/

void HBM_SetWidescreen(bool value) {
	HBM_Settings.Widescreen = value;
	HBM_Settings.ScaleX = (HBM_Settings.Width - HBM_WIDTH) / 2.0 / HBM_WIDTH + 1.0;
	HBM_Settings.ScaleY = (HBM_Settings.Height - HBM_HEIGHT) / 2.0 / HBM_HEIGHT + 1.0;

	// Adjustment for 576i (PAL)
	if (VIDEO_GetCurrentTvMode() == VI_PAL)
		HBM_Settings.ScaleY *= 0.916667;

	if (value)
		HBM_Settings.ScaleX /= HBM_WIDESCREEN_RATIO;

	if (HBM_initialized)
		HBM_elementPositions();
}

bool HBM_SetLanguage(enum HBM_LANG value) {
	if (!HBM_initialized) {
		if (value != HBM_Settings.Language)
			HBM_Settings.Language = value;
	} else {
		if (!HBM_LoadLanguage(value))
			return false;

		HBM_Settings.Language = (enum HBM_LANG)HBM_GetCurrentLanguage();

		// Update strings
		// **********************
		HBM_topHeader.Text = HBM_gettextmsg("HOME Menu");
		HBM_topHeaderButtonText = HBM_gettextmsg("Close");
		HBM_remoteData[0].Text = HBM_gettextmsg("P1");
		HBM_remoteData[1].Text = HBM_gettextmsg("P2");
		HBM_remoteData[2].Text = HBM_gettextmsg("P3");
		HBM_remoteData[3].Text = HBM_gettextmsg("P4");
		HBM_bottomHeader.Text = HBM_gettextmsg("Wii Remote Settings");

		HBM_wiiMenuButton.Text = HBM_gettextmsg("Wii Menu");
		HBM_resetButton.Text = HBM_MyReset == NULL ? HBM_gettextmsg("Loader") : HBM_gettextmsg("Reset");
		HBM_manualButton.Text = HBM_gettextmsg("Operations Guide");

		HBM_dialog.UpdateText(
			HBM_gettextmsg(HBM_dialog.Confirm == HBM_TriggerReset ? (HBM_MyReset == NULL ? "This will quit the game. Proceed? (Anything not saved will be lost.)" :
																	HBM_Settings.Unsaved > 0 ? "Reset the software? (Anything not saved will be lost.)" : "Reset the software?")
																  : HBM_Settings.Unsaved > 1 ? "Return to the Wii Menu? (Anything not saved will be lost.)" : "Return to the Wii Menu?"),
			HBM_gettextmsg("Yes"),
			HBM_gettextmsg("No")
		);
	}

	return true;
}

void HBM_SetUnsaved(int value) {
	if (value != HBM_Settings.Unsaved)
		HBM_Settings.Unsaved = value;
}

/******************************************************
 *                MANAGEMENT FUNCTIONS                *
 ******************************************************/

static void HBM_clearValues()
{
	// Set initial animation values
	HBM_timeElapsed = 0;
	HBM_timeStopwatch = -1;
	HBM_ExitTransition.Fade = 0;
	HBM_ExitTransition.Type = 0;
}

void HBM_Init(int width, int height, int host_TEVSTAGE0, int host_TEX0)
{
	if (HBM_initialized) return;

	// LWP_CreateThread(&HBM_mainthread, HBM_Thread, NULL, NULL, 0, 70);
	HBM_Settings.Status = HBM_INACTIVE;

	HBM_Settings.ScaleY = HBM_Settings.ScaleX = 1;
	HBM_Settings.Width = width;
	HBM_Settings.Height = height;

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
		HBM_Settings.Language = HBM_LANG_SYSTEM;
	#endif
	HBM_Settings.Unsaved = 0;

	// Setup target vertex descriptor
	GX_SetVtxAttrFmt(HBM_GX_VTXFMT, GX_VA_POS, GX_POS_XY, GX_F32, 0); // Positions given in 2 f32's (f32 x, f32 y)
	GX_SetVtxAttrFmt(HBM_GX_VTXFMT, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0); // Texture coordinates given in 2 f32's
	GX_SetVtxAttrFmt(HBM_GX_VTXFMT, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	// GX compatibility mode
	HBM_Settings.Host_TEX0 = host_TEX0;
	HBM_Settings.Host_TEVSTAGE0 = host_TEVSTAGE0;

	HBM_initialized = true;

	// Setup 2D model view
	guMtxIdentity(HBM_GXmodelView2D);
	guMtxTransApply(HBM_GXmodelView2D, HBM_GXmodelView2D, 0.0F, 0.0F, -50.0F);
	GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);

	// Initialize the remainder of the libraries
	HBM_SoundInit();
	HBM_ConsoleInit();
	#ifdef HBM_ENABLE_ROMFS
	HBM_RomfsInit();
	#endif
}

void HBM_Uninit()
{
	if (!HBM_initialized) return;

	HBM_FreeScreenshot();

	HBM_FontUninit();
	#ifdef HBM_ENABLE_ROMFS
	HBM_RomfsUninit();
	#endif

	HBM_initialized = false;
}

void HBM_ToggleUsage(bool value) {
	if (HBM_Settings.Status == HBM_INACTIVE)
		HBM_isAllowed = value;
}

bool HBM_IsAvailable() {
	return HBM_initialized;
}

/******************************************************
 *                 INTERNAL FUNCTIONS                 *
 ******************************************************/

static void HBM_Draw()
{
	/** Failsafes **/
	if (HBM_Settings.Host_TEVSTAGE0 < 0 || HBM_Settings.Host_TEX0 < 0) return;

#ifdef HBM_USE_ERROR_SCREEN
	// Error screen
	if (HBM_ErrorStatus > 2) {
		HBM_ErrorDraw();
	}
	else {
#endif

	// Background
	if (HBM_backgroundCreated) {
		HBM_background.Draw();
		// HBM_DrawQuad(0, 0, HBM_WIDTH, HBM_HEIGHT, 0, HBM_backgroundOpacity, true);
	}

	// Elements
	for (int i = 0; i < HBM_ELEMENT_COUNT; i++) {
		if (HBM_allElements[i] != NULL && HBM_allElements[i]->Visible)
			HBM_allElements[i]->Draw();
	}

	HBM_remoteDataBG.Draw(130 + (HBM_Settings.Widescreen ? 118.75 : 0), HBM_bottomHeader.Y - 15);
	for (int i = 0; i < HBM_MAX_POINTERS; i++) {
		HBM_remoteData[i].Draw(190 + (HBM_Settings.Widescreen ? 118.75 : 0) + (107 * i), HBM_bottomHeader.Y - 8);
	}

	HBM_topHeaderButton.Draw((HBM_Settings.Widescreen ? HBM_WIDTH * HBM_WIDESCREEN_RATIO : HBM_WIDTH) - 195, HBM_topHeader.Y + 49);
	HBM_DrawText
	(
		/* text */		HBM_topHeaderButtonText,
		/* X */			(HBM_Settings.Widescreen ? HBM_WIDTH * HBM_WIDESCREEN_RATIO : HBM_WIDTH) - 195 + 105.525,
		/* Y */			HBM_topHeader.Y + 49 + 19.6,
		/* size */		28.533,
		/* scaleX */	1,
		/* scaleY */	1,
		/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
		/* serif */		false,
		/* color */		80, 80, 80, 255,
		/* maxWidth */	108
	);

	// Foreground objects
	HBM_dialog.Draw();
	HBM_pointer4.Draw();
	HBM_pointer3.Draw();
	HBM_pointer2.Draw();
	HBM_pointer1.Draw();

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

#ifdef HBM_USE_ERROR_SCREEN
	// Fade to error screen
	if (HBM_ErrorStatus > 1) HBM_ErrorDraw();
	}
#endif

	// Fade to black
	if (HBM_ExitTransition.Fade > 0)
		HBM_DrawQuad(0, 0, HBM_WIDTH, HBM_HEIGHT, 0, HBM_ExitTransition.Fade, true);
}

static void HBM_Vsync()
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
#ifdef HBM_USE_ERROR_SCREEN
	if (HBM_Settings.Status != HBM_CLOSING && HBM_Settings.Status != HBM_CLOSED && HBM_ErrorStatus <= 0) {
#else
	if (HBM_Settings.Status != HBM_CLOSING && HBM_Settings.Status != HBM_CLOSED) {
#endif
		HBM_PointerUpdate();

		// Visibility based on pointer's availability
		HBM_pointer1.Visible = HBMPointers[0].Status > 0;
		HBM_pointer2.Visible = HBMPointers[1].Status > 0;
		HBM_pointer3.Visible = HBMPointers[2].Status > 0;
		HBM_pointer4.Visible = HBMPointers[3].Status > 0;

		// Position based on pointer location
		HBM_pointer1.SetPosition(HBMPointers[0].X, HBMPointers[0].Y);
		HBM_pointer2.SetPosition(HBMPointers[1].X, HBMPointers[1].Y);
		HBM_pointer3.SetPosition(HBMPointers[2].X, HBMPointers[2].Y);
		HBM_pointer4.SetPosition(HBMPointers[3].X, HBMPointers[3].Y);

		// Rotation based on pitch
		HBM_pointer1.Image.Rotation = HBMPointers[0].Rotation;
		HBM_pointer2.Image.Rotation = HBMPointers[1].Rotation;
		HBM_pointer3.Image.Rotation = HBMPointers[2].Rotation;
		HBM_pointer4.Image.Rotation = HBMPointers[3].Rotation;
	}

	if (HBM_ExitTransition.Type > 0)
	{
		/*********************************************************
		  Control menu exit after fading is completed
		**********************************************************/
		HBM_TimeUpdate();
		if (HBM_ExitTransition.Fade >= 1)
		{
			HBM_Settings.Status = HBM_INACTIVE;

			switch (HBM_ExitTransition.Type)
			{
				default:
				case 1:
					VIDEO_SetBlack(true);
					if (HBM_BeforeExit != NULL) HBM_BeforeExit();
					HBM_Uninit();
					SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
					break;

				case 2:
					if (HBM_MyReset == NULL) {
						VIDEO_SetBlack(true);
						if (HBM_BeforeExit != NULL) HBM_BeforeExit();
						HBM_Uninit();
						exit(0);
						break;
					} else {
						HBM_MyReset();
						break;
					}

				case 3:
					VIDEO_SetBlack(true);
					HBM_Uninit();
					SYS_ResetSystem(SYS_POWEROFF, 0, 0);
					break;
			}

			return;
		}
		/*********************************************************
		  Fading animation when exiting to System Menu or power
		**********************************************************/
		else
		{
#ifdef HBM_USE_ERROR_SCREEN
			if (HBM_Settings.Status == HBM_OPEN && HBM_ErrorStatus <= 0) {
#else
			if (HBM_Settings.Status == HBM_OPEN) {
#endif
				for (int i = 0; i < HBM_ELEMENT_COUNT; i++) {
					if (HBM_allElements[i] != NULL)
						HBM_allElements[i]->Update();
				}

				HBM_dialog.Update();
				HBM_pointer4.Update();
				HBM_pointer3.Update();
				HBM_pointer2.Update();
				HBM_pointer1.Update();
			}

			HBM_ExitTransition.Fade = HBM_timeElapsed / (HBM_ExitTransition.Type == 3 ? 0.333 : 0.5);

			// if (HBM_ExitTransition.Type < 3) {
				HBM_SoundSetVolume(1 - (HBM_timeElapsed / (HBM_ExitTransition.Type == 3 ? 0.333 : 0.5)));
			// } else {
				// if (HBM_ExitTransition.Fade > 0.15F)
					// HBM_SoundUninit();
			// }
		}
	}
	else
	{
#ifdef HBM_USE_ERROR_SCREEN
		/*********************************************************
		  Error screen
		**********************************************************/
		if (HBM_ErrorStatus > 0) {
			HBM_ErrorUpdate();
			return;
		}
#endif

		switch (HBM_Settings.Status)
		{
			/*********************************************************
			  Sliding animation
			  Do not accept any input during this time
			**********************************************************/
			case HBM_OPENING:
			case HBM_CLOSING:
				if (HBM_Settings.Stage == HBM_STAGE_MAIN)
				{
					// HBM_ConsolePrintf2("progress: %.3f, Top Y: %d, Bottom Y: %d", HBM_timeElapsed / 0.3333, HBM_topHeader.Y, HBM_bottomHeader.Y);
					HBM_TimeUpdate();
					if (HBM_timeElapsed >= 0.3333) {
						if (HBM_Settings.Status == HBM_OPENING)
							HBM_PlaySound(HBM_sfx_menuopen_pcm, HBM_sfx_menuopen_pcm_end, false);

						HBM_SlideAnimation(HBM_Settings.Status == HBM_CLOSING ? 0 : 1);
						HBM_Settings.Status = HBM_Settings.Status == HBM_CLOSING ? HBM_CLOSED : HBM_OPEN;
						HBM_ConsolePrintf(HBM_Settings.Status == HBM_CLOSED ? "HBM status: Closed" : "HBM status: Opened");
						HBM_TimeClear();
					} else {
						HBM_SlideAnimation(HBM_Settings.Status == HBM_CLOSING ? 1 - (HBM_timeElapsed / 0.3333) : HBM_timeElapsed / 0.3333);
						if (HBM_Settings.Status == HBM_CLOSING) HBM_SoundSetVolume(1 - (HBM_timeElapsed / 0.3333));
					}
				}
				else {
					HBM_Settings.Status = HBM_Settings.Status == HBM_CLOSING ? HBM_CLOSED : HBM_OPEN;
				}
				break;

			/*********************************************************
			  Idle state
			**********************************************************/
			case HBM_OPEN:
				{
					#ifdef HBM_VERBOSE
					{
						static u8 frameCount = 0;
						static u32 lastTime;
						static u8 FPS = 0;
						const u32 currentTime = ticks_to_millisecs(gettime());

						frameCount++;
						if(currentTime - lastTime > 1000) {
							lastTime = currentTime;
							FPS = frameCount;
							frameCount = 0;
						}
						// HBM_ConsolePrintf2("FPS: %2d - %d x %d, scaled %.3f x %.3f", FPS, HBM_Settings.Width, HBM_Settings.Height, HBM_Settings.ScaleX, HBM_Settings.ScaleY);
						// HBM_ConsolePrintf2("FPS: %2d, stages: %4x, time: %.2f", FPS, (int)HBM_Settings.Stage, HBM_timeElapsed);
						HBM_ConsolePrintf2("FPS: %2d", FPS);
					}
					#endif

					if (!(HBM_Settings.Stage & HBM_STAGE_BLOCKED)) {
						// Hold for five seconds to show About prompt
						if ((WPAD_ButtonsHeld(0) & WPAD_BUTTON_1) && (WPAD_ButtonsHeld(0) & WPAD_BUTTON_2)) {
							if (HBM_aboutTime <= 0) {
								HBM_aboutTime = ((f64)HBM_GETTIME / 1000.0F);
							} else if (((f64)HBM_GETTIME / 1000.0F) >= HBM_aboutTime + 5.0) {
								HBM_aboutTime = 0;
								HBM_PromptAbout();
							}
						} else if (HBM_aboutTime > 0) {
							HBM_aboutTime = 0;
						}

						if (WPAD_ButtonsDown(0) & WPAD_BUTTON_1)
							HBM_SetWidescreen(HBM_Settings.Widescreen ? false : true);
						else if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
							HBM_SetLanguage((enum HBM_LANG)(HBM_GetCurrentLanguage() + 1));
						else if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_PLUS && WPAD_ButtonsHeld(0) & WPAD_BUTTON_2)
							HBM_Settings.ScaleY += 0.001;
						else if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_MINUS && WPAD_ButtonsHeld(0) & WPAD_BUTTON_2)
							HBM_Settings.ScaleY -= 0.001;
						else if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_PLUS)
							HBM_Settings.ScaleX += 0.001;
						else if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_MINUS)
							HBM_Settings.ScaleX -= 0.001;
#ifdef HBM_USE_ERROR_SCREEN
						else if ((WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_FULL_L) || (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_FULL_R))
							HBM_Error("You done goofed");
#endif
					}

					for (int i = 0; i < HBM_MAX_POINTERS; i++)
					{
						if (WPAD_ButtonsDown(i) & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)) {
							switch (HBM_Settings.Stage) {
								default:
									break;
								case HBM_STAGE_MAIN:
									HBM_topHeader.Call();
									break;
								case HBM_STAGE_WPAD:
									HBM_bottomHeader.Call();
									break;
							}
						}
					}

					for (int i = 0; i < HBM_ELEMENT_COUNT; i++) {
						if (HBM_allElements[i] != NULL)
							HBM_allElements[i]->Update();
					}

					HBM_dialog.Update();
					HBM_pointer4.Update();
					HBM_pointer3.Update();
					HBM_pointer2.Update();
					HBM_pointer1.Update();
				}
				break;

			/*********************************************************
			  Closed state
			  Should automatically set to inactive
			**********************************************************/
			default:
			case HBM_CLOSED:
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
		if (HBM_BeforeShowMenu != NULL) HBM_BeforeShowMenu();

		// Declare elements and values
		HBM_clearValues();
		HBM_elementLoad();
		HBM_elementSetup();
		HBM_elementPositions();
		HBM_SetLanguage(HBM_Settings.Language);

		// Init console
		HBM_ConsoleClear();
		HBM_ConsolePrintf("HBM status: Opening");

		// Init video
		fb[0] = VIDEO_GetCurrentFramebuffer();
		fb[1] = VIDEO_GetNextFramebuffer();
		HBM_TakeScreenshot();

		// Init GX
		GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
		GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);

		if (HBM_Settings.Host_TEVSTAGE0 >= 0 && HBM_Settings.Host_TEX0 >= 0) {
			GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
			GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
		}
		// GX_SetScissorBoxOffset(0, 0);
		// GX_SetScissor(0, 0, HBM_Settings.Width, HBM_Settings.Height);

		// Init system power callbacks
		origPowerCallback = SYS_SetPowerCallback(HBM_HandleShutdown);
		origResetCallback = SYS_SetResetCallback(HBM_HandleReset);
		WPAD_SetPowerButtonCallback(HBM_HandleWPADShutdown);

		HBM_PointerInit();
		HBM_Settings.Stage = HBM_STAGE_MAIN;
		HBM_Settings.Status = HBM_OPENING;

		// Main loop
		do
		{
			HBM_Update();
			HBM_ConsoleUpdate();

			if (HBM_BeforeDraw != NULL) HBM_BeforeDraw();

			HBM_Draw();
			HBM_Vsync();

			if (HBM_AfterDraw != NULL) HBM_AfterDraw();
		} while (HBM_Settings.Status != HBM_INACTIVE);

		if (HBM_AfterHideMenu != NULL) HBM_AfterHideMenu();

		// Exiting ...
		HBM_elementFree();

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
		HBM_noHomeIcon.LoadPNG(&HBM_noHome_png, 52, 52);
		HBM_noHomeIcon.SetPosition(50, /*42*/54);
		HBM_noHomeIcon.Visible = true;

		HBM_Settings.Status = HBM_NOHOME;
	}
}

void HBM_HideMenu()
{
	if (!HBM_initialized || HBM_Settings.Status != HBM_OPEN || HBM_ExitTransition.Type > 0) return;

	if (HBM_BeforeHideMenu != NULL) HBM_BeforeHideMenu();

	HBM_ConsolePrintf("HBM status: Closing");
	HBM_Settings.Status = HBM_CLOSING;
}

void HBM_DrawNoHome()
{
	if (!HBM_initialized || HBM_Settings.Host_TEVSTAGE0 < 0 || HBM_Settings.Host_TEX0 < 0) return;

	if (HBM_Settings.Status == HBM_NOHOME)
	{
		if (HBM_timeStopwatch < 0)
			HBM_timeStopwatch = ((f64)HBM_GETTIME / 1000.0F);

		HBM_timeElapsed = ((f64)HBM_GETTIME / 1000.0F) - HBM_timeStopwatch;

		HBM_noHomeIcon.A = HBM_timeElapsed < 0.233 ? lround(255.0F * (HBM_timeElapsed / 0.233))
							 : HBM_timeElapsed > 0.233 + 1.2 ? lround(255.0F * (1.0F - ((HBM_timeElapsed - 0.233 - 1.2) / 0.233)))
							 : 255;

		if (HBM_timeElapsed >= 1.6667)
		{
			HBM_TimeClear();
			HBM_noHomeIcon.A = 0;
			HBM_Settings.Status = HBM_INACTIVE;
			HBM_noHomeIcon.Free();
			return;
		}

		GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
		GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

		HBM_noHomeIcon.Draw();

		GX_SetTevOp(GX_TEVSTAGE0, HBM_Settings.Host_TEVSTAGE0);
		GX_SetVtxDesc(GX_VA_TEX0, HBM_Settings.Host_TEX0);
	}
}

#undef HBM_ELEMENT_COUNT