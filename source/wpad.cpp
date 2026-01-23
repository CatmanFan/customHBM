#include "hbm.h"
#include "hbm/extern.h"

#define HBM_CC_MIN_MAGNITUDE 0.2f

struct HBMPointer HBMPointers[HBM_MAX_POINTERS];

/** Elements to be drawn to UI **/
HBMRemoteDataSprite HBM_remoteData[4];

static void HBM_PointerSetStatus(int i, enum HBM_POINTER_STATUS status) {
	if (HBMPointers[i].Status != status) {
		HBMPointers[i].Status = status;
		switch (status) {
			default:
			case HBM_POINTER_INACTIVE:
				if (CONF_GetPadMotorMode() != 0) {
					WPAD_Rumble(i, 0);
					HBMPointers[i].RumbleEnd = 0;
					HBMPointers[i].RumbleTicks = 0;
				}
				HBM_ConsolePrintf("WPAD %d set to inactive", i);
				break;

			case HBM_POINTER_IR:
				HBM_ConsolePrintf("WPAD %d set to IR", i);
				break;

			case HBM_POINTER_CC:
				HBM_ConsolePrintf("WPAD %d set to CC", i);
				break;
		}
	}
}

static bool HBM_PointerCheckIR(int i)
{
	return i < WPAD_MAX_IR_DOTS ? HBMPointers[i].Data->ir.valid : false;
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

static void HBM_PointerCheckConnection(int i, bool first = false) {
	// Init IR if unavailable before scanning
	if (!HBMPointers[i].IRCapable && !HBM_remoteData[i].Disconnected) {
		WPAD_SetVRes		(i, HBM_Settings.Width, HBM_Settings.Height);
		WPAD_SetDataFormat	(i, WPAD_FMT_BTNS_ACC_IR);
		HBMPointers[i].IRCapable = true;
	}

	WPADData* data = WPAD_Data(i);
	if (data == NULL && HBMPointers[i].Data != NULL) goto Disconnect;
	if (data != NULL) {
		if (data->err == WPAD_ERR_NO_CONTROLLER) goto Disconnect;

		if (data->data_present > 0) goto Connect;
		else goto Disconnect;
	}
	goto Link;

	Connect:
	if (i < 4 && HBM_remoteData[i].Disconnected) {
		HBM_ConsolePrintf("WPAD %d is connected (battery: %d, err: %d)", i, data->battery_level, data->err);

		if (first) {
			HBM_remoteData[i].Disconnected = false;
		} else {
			HBM_remoteData[i].Flash(false);
			HBM_PointerRumble(i, 0.5, 1);
			switch (i) {
				default:
				case 0:
					HBM_SOUND(HBM_sfx_sync1_pcm, true);
					break;
				case 1:
					HBM_SOUND(HBM_sfx_sync2_pcm, true);
					break;
				case 2:
					HBM_SOUND(HBM_sfx_sync3_pcm, true);
					break;
				case 3:
					HBM_SOUND(HBM_sfx_sync4_pcm, true);
					break;
			}
		}
	}
	goto Link;

	Disconnect:
	if (i < 4 && !HBM_remoteData[i].Disconnected) {
		HBM_ConsolePrintf("WPAD %d is disconnected", i);

		HBMPointers[i].IRCapable = false;
		HBM_remoteData[i].Flash(true);
	}
	goto Link;

	Link:
	if (data != NULL) {
		// Change battery level image
		if (HBM_remoteData[i].Disconnected) HBM_remoteData[i].SetBatteryImage(0);
		else {
			if (data->battery_level < 2)		HBM_remoteData[i].SetBatteryImage(1);
			else if (data->battery_level < 48)	HBM_remoteData[i].SetBatteryImage(2);
			else if (data->battery_level < 64)	HBM_remoteData[i].SetBatteryImage(3);
			else if (data->battery_level < 81)	HBM_remoteData[i].SetBatteryImage(4);
			else								HBM_remoteData[i].SetBatteryImage(5);
		}
	}

	HBMPointers[i].Data = data;
}

void HBM_PointerInit()
{
    WPAD_ScanPads();

	for (int i = 0; i < HBM_MAX_POINTERS; i++) {
		HBMPointers[i].IRCapable = false;
		HBM_remoteData[i].SetBatteryImage(0);
		HBM_remoteData[i].Disconnected = true;

		HBM_PointerCheckConnection(i, true);

		if (!HBM_PointerCheckIR(i))
			HBM_PointerSetStatus(i, HBM_POINTER_INACTIVE);
	}
}

void HBM_PointerRumble(int i, f64 time, int type)
{
	if (CONF_GetPadMotorMode() != 0 && HBMPointers[i].RumbleEnd <= 0) {
		HBMPointers[i].RumbleEnd = ((f64)HBM_GETTIME / 1000.0F) + time;
		HBMPointers[i].RumbleType = type;
	}
}

void HBM_PointerUpdate()
{
	// PAD_ScanPads();
    WPAD_ScanPads();

	for (int i = 0; i < HBM_MAX_POINTERS; i++)
	{
		HBM_PointerCheckConnection(i);

		if (HBMPointers[i].Data) {
			switch (HBMPointers[i].Status) {
				case HBM_POINTER_INACTIVE:
					// Activate Classic Controller joystick
					if (HBM_PointerCheckCC(i)) {
						HBMPointers[i].X = (HBM_Settings.Widescreen ? HBM_WIDTH * HBM_WIDESCREEN_RATIO : HBM_WIDTH) / 2;
						HBMPointers[i].Y = HBM_HEIGHT / 2;
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
						HBMPointers[i].X = HBMPointers[i].Data->ir.x / HBM_Settings.ScaleX;
						HBMPointers[i].Y = HBMPointers[i].Data->ir.y / HBM_Settings.ScaleY;
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

			if (CONF_GetPadMotorMode() != 0 && HBMPointers[i].RumbleEnd > 0) {
				// Soft rumble
				if (HBMPointers[i].RumbleType == 0) {
					HBMPointers[i].RumbleTicks = (HBMPointers[i].RumbleTicks + 1) % 3;
					WPAD_Rumble(i, HBMPointers[i].RumbleTicks == 1 ? 0 : 1);
				}

				// Full rumble
				if (HBMPointers[i].RumbleType == 1) {
					WPAD_Rumble(i, 1);
				}

				// Stop rumbling if reached past allocated duration
				if ((f64)HBM_GETTIME / 1000.0F >= HBMPointers[i].RumbleEnd) {
					WPAD_Rumble(i, 0);
					HBMPointers[i].RumbleEnd = 0;
					HBMPointers[i].RumbleTicks = 0;
				}
			}
		} else {
			HBM_PointerSetStatus(i, HBM_POINTER_INACTIVE);
		}
	}
}

#undef HBM_CC_MIN_MAGNITUDE