#ifndef __HBM__WPAD__
#define __HBM__WPAD__

enum HBM_POINTER_STATUS {
	HBM_POINTER_INACTIVE = 0,
	HBM_POINTER_IR,
	HBM_POINTER_CC,
};

struct HBMPointer {
	WPADData* Data;
	enum HBM_POINTER_STATUS Status;
	bool IRCapable;

	float X;
	float Y;
	float Rotation;

	int RumbleType; // Soft or full
	int RumbleTicks; // Used to control vibration intensity
	f64 RumbleEnd; // Current time + duration allocated to rumble
};

extern struct HBMPointer HBMPointers[];

void HBM_PointerInit();
void HBM_PointerUpdate();
void HBM_PointerRumble(int i, f64 time, int type);

#endif