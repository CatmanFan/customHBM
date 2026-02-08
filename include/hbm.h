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
 * Sets the HOME Menu aspect ratio, otherwise it will either default to the system's config or a forced ratio as specified in config.mk.
 *
 * @param value 0 - (standard, 4/3);
				1 - (widescreen, 16/9)
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
	HBM_LANG_JAPANESE = 0,
	HBM_LANG_ENGLISH,
	HBM_LANG_FRENCH,
	HBM_LANG_GERMAN,
	HBM_LANG_SPANISH,
	HBM_LANG_ITALIAN,
	HBM_LANG_DUTCH,
	HBM_LANG_PT_PORTUGUESE,
	HBM_LANG_BR_PORTUGUESE,
	HBM_LANG_RUSSIAN,
	HBM_LANG_DANISH,
	HBM_LANG_FINNISH,
	HBM_LANG_GREEK,
	HBM_LANG_SWEDISH,
	HBM_LANG_NORWEGIAN, // Bokmål
	HBM_LANG_TURKISH,
	HBM_LANG_POLISH,
	// HBM_LANG_CZECH,
	HBM_LANG_UKRAINIAN,
	HBM_LANG_WELSH,
	HBM_LANG_CATALAN,
	HBM_LANG_GALICIAN,
	HBM_LANG_BASQUE,
	HBM_LANG_KALAALLISUT,
	HBM_LANG_SIMP_CHINESE,
	HBM_LANG_TRAD_CHINESE,
	HBM_LANG_KOREAN,
	HBM_LANG_OKINAWAN,
	HBM_LANG_COUNT, // Total number of language entries, do not touch!

	HBM_LANG_TAMAZIGHT_KAB,
	HBM_LANG_TAMAZIGHT_ZGH,
};

bool HBM_SetLanguage(enum HBM_LANG value);

/**
 * Determines whether to show the "any unsaved data will be lost" message. By default this is disabled (see below).
 *
 * @param value 0 - disabled;
 *				1 - disabled for "Wii Menu", enabled for "Reset";
 *				2 - enabled for both
 **/
void HBM_SetUnsaved(int value);

#endif