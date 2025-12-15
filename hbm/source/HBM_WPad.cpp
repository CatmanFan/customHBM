#include "HBM.h"
#include "HBM_Extern.h"

#define HBM_CC_MIN_MAGNITUDE 0.2f

struct HBMPointer HBMPointers[4]; // WPAD_MAX_WIIMOTES

static void HBM_PointerSetStatus(int i, enum HBM_POINTER_STATUS status) {
	if (HBMPointers[i].Status != status) {
		HBMPointers[i].Status = status;
		switch (status) {
			default:
			case HBM_POINTER_INACTIVE:
				HBM_ConsolePrintf("Pointer %d status: N/A", i);
				break;

			case HBM_POINTER_IR:
				HBM_ConsolePrintf("Pointer %d status: IR", i);
				break;

			case HBM_POINTER_CC:
				HBM_ConsolePrintf("Pointer %d status: Classic Controller", i);
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

void HBM_PointerInit()
{
	// Init IR
	WPAD_SetVRes(WPAD_CHAN_ALL, HBM_Settings.Width, HBM_Settings.Height);
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
						HBMPointers[i].X += (HBMPointers[i].Data->exp.classic.ljs.mag > HBM_CC_MIN_MAGNITUDE ? HBMPointers[i].Data->exp.classic.ljs.mag : 0) * cos(HBMPointers[i].Data->exp.classic.ljs.ang);
						HBMPointers[i].Y += (HBMPointers[i].Data->exp.classic.ljs.mag > HBM_CC_MIN_MAGNITUDE ? HBMPointers[i].Data->exp.classic.ljs.mag : 0) * sin(HBMPointers[i].Data->exp.classic.ljs.ang);
						// HBM_ConsolePrintf2("Status: %d, Cursor pos: %.1f %.1f", HBMPointers[i].Status, HBMPointers[i].X, HBMPointers[i].Y);
					}
					break;
			}
		} else {
			HBM_PointerSetStatus(i, HBM_POINTER_INACTIVE);
		}
	}

	// WPAD_ButtonsDown(WPAD_CHAN_0);
}

#undef HBM_CC_MIN_MAGNITUDE