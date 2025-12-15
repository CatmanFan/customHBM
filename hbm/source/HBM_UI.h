#ifndef __HBM__HBMUI__
#define __HBM__HBMUI__

enum HBM_STATUS {
	HBM_INACTIVE,
	HBM_BLOCKED,
	HBM_OPENING,
	HBM_OPEN,
	HBM_CLOSING,
	HBM_CLOSED
};

struct HBM_CONFIG {
	enum HBM_STATUS Status;
	float ScaleX;
	float ScaleY;
	int Width;
	int Height;
	int Language;
	bool Widescreen;
	int InteractionLayer;
};

/**
 * Initializes Home Menu library..
 * Please use the dimensions of the game/homebrew. The screen will be clipped otherwise.
 * "use3D" should be set to TRUE if using GRRLIB, otherwise FALSE.
 */
void HBM_Init(int width, int height, bool widescreen);

void HBM_Uninit();

/**
 * Determines whether the No Home icon shows up instead of the menu.
 */
void HBM_ToggleUsage(bool value);

void HBM_Open();
void HBM_Hide();
void HBM_DrawNoHome();

#endif