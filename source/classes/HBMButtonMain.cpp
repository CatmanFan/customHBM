#include "hbm.h"
#include "hbm/extern.h"

#define HBM_BUTTONMAIN_HOVERED_SCALE 1.033F

HBMButtonMain::HBMButtonMain() : HBMButton::HBMButton() {
	// Load images
	this->Image.LoadPNG(&HBM_mainButton_png, 264, 104);
	this->SetHitbox(10, 10, 264 - 10, 104 - 10);

	// Load mask
	this->Mask.LoadPNG(&HBM_mainButton_mask_png, 264, 104);
	this->MaskOpacity = 0;

	// Load shadow
	this->Shadow.LoadRaw(this->Image.GetImage(), this->Image.GetWidth(), this->Image.GetHeight());
	this->Shadow.R = this->Shadow.G = this->Shadow.B = 0;
	this->ShadowOpacity = 0.58;
	this->ShadowX = this->ShadowY = 5;

	// Set text color
	this->TextColor = 0x202E65FF;
	this->TextSize = 26;
	this->TextOverMask = true;

	this->Selected = NULL;
	this->TimeSnapshot = 0;
	this->Blocked = false;
}

void HBMButtonMain::Update() {
	int chan = -1;

	for (int i = 0; i < 4 /* WPAD_MAX_WIIMOTES */; i++) {
		if (this->HitboxTouched(i)) {
			chan = i;
			break;
		}
	}

	if (chan >= 0 && this->Status != 2 && HBM_Settings.InteractionLayer == HBM_INTERACTION_MAIN && !this->Blocked) {
		if ((HBMPointers[chan].Status == HBM_POINTER_CC && WPAD_ButtonsDown(chan) & WPAD_CLASSIC_BUTTON_A)
		 || (HBMPointers[chan].Status == HBM_POINTER_IR && WPAD_ButtonsDown(chan) & WPAD_BUTTON_A)) {
			if (this->Status != 2) {
				HBMPointers[chan].Rumble = false;
				HBM_PlaySound(HBM_sfx_select_pcm, HBM_sfx_select_pcm_size);
				HBM_BUTTON_TIME_RESET
				HBM_Settings.InteractionLayer = HBM_INTERACTION_BUTTON;
				this->Status = 2;
			}
		} else {
			if (this->Status < 1) {
				HBMPointers[chan].Rumble = true;
				HBM_PlaySound(HBM_sfx_hover_pcm, HBM_sfx_hover_pcm_size);
				HBM_BUTTON_TIME_RESET
				this->Status = 1;
			}
		}
	}

	// Set to default state and animation if any pointer is out of focus
	if (chan < 0 && this->Status == 1) {
		for (int i = 0; i < 4 /* WPAD_MAX_WIIMOTES */; i++)
			HBMPointers[i].Rumble = false;

		HBM_BUTTON_TIME_RESET
		this->Status = 0;
	}

	switch (this->Status) {
		// Inactive state
		default:
		case 0:
			if (HBM_BUTTON_TIME_WAITING(0.0833))
				this->Scale = (HBM_BUTTONMAIN_HOVERED_SCALE * (1 - HBM_BUTTON_TIME_PROGRESS(0.0833))) + HBM_BUTTON_TIME_PROGRESS(0.0833);
			else {
				this->Scale = 1;
				HBM_BUTTON_TIME_CLEAR
			}
			break;

		// Hovered state
		case 1:
			// Stop rumbling
			if (!HBM_BUTTON_TIME_WAITING(0.075))
				for (int i = 0; i < 4 /* WPAD_MAX_WIIMOTES */; i++)
					HBMPointers[i].Rumble = false;

			if (HBM_BUTTON_TIME_WAITING(0.0833)) {
				this->Scale = (1 - HBM_BUTTON_TIME_PROGRESS(0.0833)) + (HBM_BUTTONMAIN_HOVERED_SCALE * HBM_BUTTON_TIME_PROGRESS(0.0833));
			} else {
				this->Scale = HBM_BUTTONMAIN_HOVERED_SCALE;
				HBM_BUTTON_TIME_CLEAR
			}
			break;

		// Selected state
		case 2:
			this->Scale = HBM_BUTTONMAIN_HOVERED_SCALE;

			if (HBM_BUTTON_TIME_WAITING(0.050))
				this->MaskOpacity = 1 * HBM_BUTTON_TIME_PROGRESS(0.050);
			else if (HBM_BUTTON_TIME_WAITING(0.1667))
				this->MaskOpacity = 1;
			else if (HBM_BUTTON_TIME_WAITING(0.2667))
				this->MaskOpacity = 1 * (1 - HBM_BUTTON_TIME_PROGRESS_PARTIAL(0.1667, 0.2667));

			if (!HBM_BUTTON_TIME_WAITING(0.2667) && this->Substatus == 0) {
				this->MaskOpacity = 0;
				if (this->Selected != NULL) this->Selected();
				this->Substatus = 1;
			}

			if (!HBM_BUTTON_TIME_WAITING(0.6667)) {
				HBM_BUTTON_TIME_RESET
				if (HBM_Settings.InteractionLayer == HBM_INTERACTION_BUTTON)
					HBM_Settings.InteractionLayer = HBM_INTERACTION_MAIN;
				this->Substatus = 0;
				this->Status = 0;
			}
			break;
	}
}

#undef HBM_BUTTONMAIN_HOVERED_SCALE