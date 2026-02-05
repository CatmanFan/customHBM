#include "hbm.h"
#include "hbm/extern.h"

#define HBM_DIALOGBUTTON_NORMAL_SCALE 0.9F

HBMDialogButton::HBMDialogButton() : HBMButton::HBMButton() {
	// Load images
	this->Image.LoadPNG(&HBM_dialogButton_png, 240, 68);
	this->Image.SetAnchorPoint(119.5, 33);
	this->SetHitbox(0, 0, 239, 66);
	this->ShadowOpacity = 0;

	// Load mask
	this->Mask.LoadPNG(&HBM_dialogButton_mask_png, 240, 68);
	this->Mask.SetAnchorPoint(119.5, 33);
	this->MaskOpacity = 0;

	this->Sound = 0;
	this->Selected = NULL;
	this->Blocked = false;
	this->TimerStop();
}

void HBMDialogButton::Draw() {
	HBMElement::Draw();

	// Draw text
	if (this->Image.A > 0 && this->Visible) {
		HBM_DrawText
		(
			/* text */		this->Text,
			/* X */			this->X + (HBM_FontType() == 2 ? 118 : 119) - (this->AltAppearance ? 1 : 0),
			/* Y */			this->Y + 30,
			/* size */		(HBM_FontType() == 2 ? 27 : 26),
			/* scaleX */	this->AltAppearance ? this->Scale * 0.98 : this->Scale,
			/* scaleY */	this->Scale,
			/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
			/* serif */		false,
			/* color */		this->AltAppearance ? 101 : 64,
							this->AltAppearance ? 101 : 64,
							this->AltAppearance ? 101 : 64,
							this->AltAppearance ? 255 : 240,
			/* maxWidth */	200,
			/* maxHeight */	32
		);
	}

	// Draw mask
	if (this->MaskOpacity > 0) {
		this->Mask.ScaleX = this->Image.ScaleX;
		this->Mask.ScaleY = this->Image.ScaleY;
		this->Mask.A = lround(255 * this->MaskOpacity);
		this->Mask.Draw(this->X + this->Image.GetX(), this->Y + this->Image.GetY());
	}
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
			if (!this->TimerPassed(0.100))
				this->Scale = (1 - this->TimerProgressEase(0.100)) + (HBM_DIALOGBUTTON_NORMAL_SCALE * this->TimerProgressEase(0.100));
			else {
				this->Scale = HBM_DIALOGBUTTON_NORMAL_SCALE;
				this->TimerStop();
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
				this->Status = 0;
			}
			break;
	}

	/*******************
	 * Status control
	 *******************/
	u8 chan = this->HitboxStatus(this->Status != 2 && (HBM_Settings.Stage & HBM_STAGE_DIALOG) == HBM_STAGE_DIALOG);
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

				HBM_Settings.Stage |= HBM_STAGE_BLOCKED;
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

#undef HBM_DIALOGBUTTON_NORMAL_SCALE