#include "hbm.h"
#include "hbm/extern.h"

#define HBM_CC_MIN_MAGNITUDE 0.2f

struct HBMPointer HBMPointers[4]; // WPAD_MAX_WIIMOTES

static void HBM_PointerSetStatus(int i, enum HBM_POINTER_STATUS status) {
	if (HBMPointers[i].Status != status) {
		HBMPointers[i].Status = status;
		switch (status) {
			default:
			case HBM_POINTER_INACTIVE:
				if (HBMPointers[i].Rumble) {
					WPAD_Rumble(i, 0);
					HBMPointers[i].RumbleTicks = 0;
					HBMPointers[i].Rumble = false;
				}
				// HBM_ConsolePrintf("Pointer %d status: N/A", i);
				break;

			case HBM_POINTER_IR:
				// HBM_ConsolePrintf("Pointer %d status: IR", i);
				break;

			case HBM_POINTER_CC:
				// HBM_ConsolePrintf("Pointer %d status: Classic Controller", i);
				break;
		}
	}
}

static bool HBM_PointerCheckIR(int i)
{
	return HBMPointers[i].Data->ir.valid;
}

static bool HBM_PointerCheckCC(int i)
{
	return HBMPointers[i].Data->exp.type == WPAD_EXP_CLASSIC
		   && ((HBMPointers[i].Data->exp.classic.btns != 0)
			   || (HBMPointers[i].Data->exp.classic.ljs.mag > HBM_CC_MIN_MAGNITUDE)
			   || (HBMPointers[i].Data->exp.classic.rjs.mag > HBM_CC_MIN_MAGNITUDE));
}

/****************************************************************************
 * WPAD_Stick
 *
 * Get X/Y value from Wii Joystick (classic, nunchuk) input
 * https://github.com/dborth/vbagx/blob/master/source/gui/gui_trigger.cpp
 ***************************************************************************/

static s8 HBM_WPADStick(u8 stick, int axis, struct joystick_t* js)
{
	if (js) {
		if (js->mag < HBM_CC_MIN_MAGNITUDE) return 0;

		int pos;
		int min;
		int max;
		int center;

		if(axis == 1) {
			pos = js->pos.y;
			min = js->min.y;
			max = js->max.y;
			center = js->center.y;
		}
		else {
			pos = js->pos.x;
			min = js->min.x;
			max = js->max.x;
			center = js->center.x;
		}

		// some 3rd party controllers return invalid analog sticks calibration data
		if ((min >= center) || (max <= center)) {
			// force default calibration settings
			min = 0;
			max = stick ? 32 : 64;
			center = stick ? 16 : 32;
		}

		if (pos > max) return 127;
		if (pos < min) return -128;

		pos -= center;

		if (pos > 0) {
			return (s8)(127.0 * ((float)pos / (float)(max - center)));
		}
		else {
			return (s8)(128.0 * ((float)pos / (float)(center - min)));
		}
	}

	return 0;
}

void HBM_PointerInit()
{
	// Init IR
	WPAD_SetVRes(WPAD_CHAN_ALL, lround(HBM_Settings.Width / HBM_Settings.ScaleX), HBM_Settings.Height);
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_2, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_3, WPAD_FMT_BTNS_ACC_IR);

    WPAD_ScanPads();

	for (int i = 0; i < 4; i++) // WPAD_MAX_WIIMOTES
	{
		HBMPointers[i].Data = WPAD_Data(i);
		if (!HBM_PointerCheckIR(i))
			HBM_PointerSetStatus(i, HBM_POINTER_INACTIVE);
	}
}

void HBM_PointerUpdate()
{
	// PAD_ScanPads();
    WPAD_ScanPads();

	for (int i = 0; i < 4; i++) // WPAD_MAX_WIIMOTES
	{
		HBMPointers[i].Data = WPAD_Data(i);

		if (HBMPointers[i].Data) {
			switch (HBMPointers[i].Status) {
				case HBM_POINTER_INACTIVE:
					// Activate Classic Controller joystick
					if (HBM_PointerCheckCC(i)) {
						HBMPointers[i].X = HBM_Settings.Width / 2;
						HBMPointers[i].Y = HBM_Settings.Height / 2;
						HBMPointers[i].Rotation = -20.0F;
						HBM_PointerSetStatus(i, HBM_POINTER_CC);
					}

					// Activate IR
					if (HBM_PointerCheckIR(i))
						HBM_PointerSetStatus(i, HBM_POINTER_IR);
					break;

				case HBM_POINTER_IR:
					// Deactivate if IR is not detected
					if (!HBM_PointerCheckIR(i)) 
						HBM_PointerSetStatus(i, HBM_POINTER_INACTIVE);

					// Control using IR
					else {
						HBMPointers[i].X = HBMPointers[i].Data->ir.x;
						HBMPointers[i].Y = HBMPointers[i].Data->ir.y;
						HBMPointers[i].Rotation = HBMPointers[i].Data->orient.roll;
						// HBM_ConsolePrintf2("Status: %d, Cursor pos: %.1f %.1f", HBMPointers[i].Status, HBMPointers[i].X, HBMPointers[i].Y);
					}
					break;

				case HBM_POINTER_CC:
					// Deactivate if CC is not detected
					if (HBMPointers[i].Data->exp.type != WPAD_EXP_CLASSIC)
						HBM_PointerSetStatus(i, HBM_POINTER_INACTIVE);

					// Deactivate if IR is detected
					else if (HBM_PointerCheckIR(i))
						HBM_PointerSetStatus(i, HBM_POINTER_IR);

					// Control using Classic Controller joystick
					else {
						HBMPointers[i].X += lround(HBM_WPADStick(0, 0, &HBMPointers[i].Data->exp.classic.ljs) * 0.133F);
						HBMPointers[i].Y -= lround(HBM_WPADStick(0, 1, &HBMPointers[i].Data->exp.classic.ljs) * 0.133F);
						// HBM_ConsolePrintf2("Status: %d, Cursor pos: %.1f %.1f", HBMPointers[i].Status, HBMPointers[i].X, HBMPointers[i].Y);
					}
					break;
			}
		} else {
			HBM_PointerSetStatus(i, HBM_POINTER_INACTIVE);
		}

		if (HBMPointers[i].Rumble) {
			if (CONF_GetPadMotorMode() == 0) {
				HBMPointers[i].Rumble = false;
				HBMPointers[i].RumbleTicks = 0;
			} else {
				// Soft rumble
				if (HBMPointers[i].RumbleStatus == 0) {
					HBMPointers[i].RumbleTicks = (HBMPointers[i].RumbleTicks + 1) % 3;
					WPAD_Rumble(i, HBMPointers[i].RumbleTicks == 1 ? 0 : 1);
				}

				// Full rumble
				if (HBMPointers[i].RumbleStatus == 1) {
					WPAD_Rumble(i, 1);
				}
			}
		} else {
			HBMPointers[i].RumbleTicks = 0;
			WPAD_Rumble(i, 0);
		}
	}
}

#undef HBM_CC_MIN_MAGNITUDE