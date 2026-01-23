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
	this->TextWidth = 240;
	this->TextColor = 0x202E65FF;
	this->TextSize = 26;
	this->TextOverMask = true;

	this->Selected = NULL;
	this->Blocked = false;
	this->TimerStop();
}

void HBMButtonMain::UpdateScale() {
	if (this->Scale < HBM_BUTTONMAIN_HOVERED_SCALE) {
		this->Timer2Start();
		if (!this->Timer2Passed(0.0833)) {
			this->Scale = (1 - this->Timer2Progress(0.0833)) + (HBM_BUTTONMAIN_HOVERED_SCALE * this->Timer2Progress(0.0833));
		} else {
			this->Scale = HBM_BUTTONMAIN_HOVERED_SCALE;
			this->Timer2Stop();
			if (this->Status == 1) { this->TimerStop(); }
		}
	}
}

void HBMButtonMain::Update() {
	/*******************
	 * Animation
	 *******************/
	this->TimerUpdate();
	switch (this->Status) {
		// Inactive state
		default:
		case 0:
			if (!this->TimerPassed(0.0833))
				this->Scale = (HBM_BUTTONMAIN_HOVERED_SCALE * (1 - this->TimerProgress(0.0833))) + this->TimerProgress(0.0833);
			else {
				this->Scale = 1;
				this->TimerStop();
			}
			break;

		// Hovered state
		case 1:
			UpdateScale();
			break;

		// Selected state
		case 2:
			UpdateScale();

			if (!this->TimerPassed(0.050))
				this->MaskOpacity = this->TimerProgress(0.050);
			else if (!this->TimerPassed(0.1667))
				this->MaskOpacity = 1;
			else if (!this->TimerPassed(0.2667))
				this->MaskOpacity = 1 - this->TimerProgress(0.1667, 0.2667);
			else if (!this->TimerPassed(0.6667)) {
				if (this->Substatus == 0) {
					this->MaskOpacity = 0;
					if (this->Selected != NULL) this->Selected();
					this->Substatus = 1;
				}
			}
			else {
				if (HBM_Settings.InteractionLayer == HBM_INTERACTION_BLOCKED)
					HBM_Settings.InteractionLayer = HBM_INTERACTION_MAIN;
				this->Substatus = 0;
				this->Status = 1;
				this->TimerStop();
			}
			break;
	}

	/*******************
	 * Status control
	 *******************/
	u8 chan = this->HitboxStatus(this->Status != 2 && HBM_Settings.InteractionLayer == HBM_INTERACTION_MAIN);
	switch ((chan >> 4) & 0x0F) {
		case 0: // Hovering
			if (this->Status != 1) {
				HBM_PointerRumble((chan & 0x0F), 0.08, 0);
				HBM_SOUND(HBM_sfx_hover_pcm, false);
				this->TimerStart();
				this->Status = 1;
			}
			break;

		case 1: // Pressed
			if (this->Status != 2) {
				HBM_SOUND(HBM_sfx_select_pcm, false);
				HBM_Settings.InteractionLayer = HBM_INTERACTION_BLOCKED;
				this->TimerStart();
				this->Status = 2;
			}
			break;

		default: // Lost focus, set to default state
			if (this->Status == 1) {
				this->TimerStart();
				this->Status = 0;
			}
			break;
	}
}

#undef HBM_BUTTONMAIN_HOVERED_SCALE