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
	float X;
	float Y;
	float Rotation;
};

extern struct HBMPointer HBMPointers[];

void HBM_PointerInit();
void HBM_PointerUpdate();

#endif