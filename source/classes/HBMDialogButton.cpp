#include "hbm.h"
#include "hbm/extern.h"

#define HBM_DIALOGBUTTON_NORMAL_SCALE 0.93F

HBMDialogButton::HBMDialogButton() : HBMButton::HBMButton() {
	// Load images
	this->Image.LoadPNG(&HBM_dialogButton_png, 240, 68);
	this->SetHitbox(0, 0, 239, 66);
	this->ShadowOpacity = 0;

	// Load mask
	this->Mask.LoadPNG(&HBM_dialogButton_mask_png, 240, 68);
	this->MaskOpacity = 0;

	// Set text color
	this->TextWidth = 200;
	this->TextColor = 0x464646FF;
	this->TextSize = 26;
	this->TextOverMask = false;

	this->Sound = 0;
	this->Selected = NULL;
	this->Blocked = false;
	this->TimerStop();
}

void HBMDialogButton::Update() {
	/*******************
	 * Animation
	 *******************/
	this->TimerUpdate();
	switch (this->Status) {
		// Inactive state
		default:
		case 0:
			if (this->Substatus == 0) {
				if (!this->TimerPassed(0.100))
					this->Scale = (1 - this->TimerProgressEase(0.100)) + (HBM_DIALOGBUTTON_NORMAL_SCALE * this->TimerProgressEase(0.100));
				else {
					this->Scale = HBM_DIALOGBUTTON_NORMAL_SCALE;
					this->TimerStop();
				}
			} else {
				this->Substatus = 0;
			}
			break;

		// Hovered state
		case 1:
			if (!this->TimerPassed(0.100)) {
				this->Scale = (HBM_DIALOGBUTTON_NORMAL_SCALE * (1 - this->TimerProgressEase(0.100))) + this->TimerProgressEase(0.100);
			} else {
				this->Scale = 1;
				this->TimerStop();
			}
			break;

		// Selected state
		case 2:
			this->Scale = 1;

			if (!this->TimerPassed(0.033))
				this->Scale = (1 - this->TimerProgress(0.033)) + (HBM_DIALOGBUTTON_NORMAL_SCALE * this->TimerProgress(0.033));
			else if (!this->TimerPassed(0.083))
			{
				// this->Scale = HBM_DIALOGBUTTON_NORMAL_SCALE;
				this->Scale = (HBM_DIALOGBUTTON_NORMAL_SCALE * (1 - this->TimerProgress(0.033, 0.133))) + this->TimerProgress(0.033, 0.133);
				this->MaskOpacity = this->TimerProgress(0.033, 0.133);
			}
			else if (!this->TimerPassed(0.133))
			{
				this->Scale = 1;
				this->MaskOpacity = 1;
			}
			else if (!this->TimerPassed(0.333))
			{
				// this->Scale = HBM_DIALOGBUTTON_NORMAL_SCALE;
				this->Scale = (1 - this->TimerProgressEase(0.133, 0.333)) + (HBM_DIALOGBUTTON_NORMAL_SCALE * this->TimerProgressEase(0.133, 0.333));
				this->MaskOpacity = 1 - this->TimerProgressEase(0.133, 0.333);
			}
			else
			{
				this->Scale = HBM_DIALOGBUTTON_NORMAL_SCALE;
				this->MaskOpacity = 0;

				if (this->Selected != NULL)
					this->Selected();
				else
					CloseDialog = true;

				this->TimerStop();
				// if (HBM_Settings.InteractionLayer == HBM_INTERACTION_BLOCKED_DIALOG)
					// HBM_Settings.InteractionLayer = HBM_INTERACTION_DIALOG;
				this->Status = 1;
			}
			break;
	}

	/*******************
	 * Status control
	 *******************/
	u8 chan = this->HitboxStatus(this->Status != 2 && !this->Blocked && HBM_Settings.InteractionLayer == HBM_INTERACTION_DIALOG);
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
				switch (this->Sound) {
					default:
						HBM_SOUND(HBM_sfx_select_pcm, false);
						break;
					case 1:
						HBM_SOUND(HBM_sfx_cancel_pcm, false);
						break;
					case 2:
						HBM_SOUND(HBM_sfx_confirm_pcm, false);
						break;
				}
				this->TimerStart();
				HBM_Settings.InteractionLayer = HBM_INTERACTION_BLOCKED_DIALOG;
				this->Substatus = 1;
				this->Status = 2;
			}
			break;

		default: // Lost focus, set to default state
			if (this->Status == 1) {
				if (this->Substatus == 0) {
					this->TimerStart();
				}
				this->Status = 0;
			}
			break;
	}
}

#undef HBM_DIALOGBUTTON_NORMAL_SCALE