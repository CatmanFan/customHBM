#include "HBM.h"
#include "HBM_Extern.h"

#define HBM_BUTTONMAIN_HOVERED_SCALE 1.033F

HBMButtonMain::HBMButtonMain() : HBMElement::HBMElement() {
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

	this->Selected = NULL;
	this->TimeSnapshot = 0;
}

void HBMButtonMain::Draw() {
	HBMElement::Draw();

	// Draw mask
	if (this->MaskOpacity > 0) {
		this->Mask.Scale = this->Image.Scale;
		this->Mask.A = lround(255 * this->MaskOpacity);
		this->Mask.Draw(this->X + this->Image.GetX(), this->Y + this->Image.GetY());
	}

	// Draw text
	if (this->Image.A > 0 && this->Visible) {
		HBM_DrawText
		(
			/* text */	this->Text,
			/* X */		this->X + (this->Image.GetWidth() - HBM_MeasureText(this->Text, 26 * this->Scale, false, false)) / 2,
			/* Y */		this->Y + (this->Image.GetHeight() - HBM_MeasureText(this->Text, 26 * this->Scale, false, true)) / 2,
			/* size */	26 * this->Scale,
			/* serif */	false,
			/* color */	32, 46, 101, this->Image.A
		);
	}
}

void HBMButtonMain::Update() {
	int chan = -1;

	for (int i = 0; i < 4 /* WPAD_MAX_WIIMOTES */; i++) {
		if (this->HitboxTouched(i)) {
			chan = i;
			break;
		}
	}

	if (chan >= 0 && this->Status != 2 && HBM_Settings.InteractionLayer == 0) {
		if ((HBMPointers[chan].Status == HBM_POINTER_CC && WPAD_ButtonsDown(chan) & WPAD_CLASSIC_BUTTON_A)
		 || (HBMPointers[chan].Status == HBM_POINTER_IR && WPAD_ButtonsDown(chan) & WPAD_BUTTON_A)) {
			if (this->Status != 2) {
				HBM_PlaySound(HBM_sfx_select_pcm, HBM_sfx_select_pcm_size);
				HBM_BUTTON_TIME_RESET
				this->Status = 2;
			}
		} else {
			if (this->Status < 1) {
				HBM_PlaySound(HBM_sfx_hover_pcm, HBM_sfx_hover_pcm_size);
				HBM_BUTTON_TIME_RESET
				this->Status = 1;
			}
		}
	}

	// Set to default state and animation if any pointer is out of focus
	if (chan < 0 && this->Status == 1) {
		HBM_BUTTON_TIME_RESET
		this->Status = 0;
	}

	switch (this->Status) {
		// Inactive state
		default:
		case 0:
			if (HBM_BUTTON_TIME_WAITING(0.0833))
				this->Scale = (HBM_BUTTONMAIN_HOVERED_SCALE * (1 - HBM_BUTTON_TIME_PROGRESS(0.0833))) + (1 * HBM_BUTTON_TIME_PROGRESS(0.0833));
			else {
				this->Scale = 1;
				HBM_BUTTON_TIME_CLEAR
			}
			break;

		// Hovered state
		case 1:
			if (HBM_BUTTON_TIME_WAITING(0.0833))
				this->Scale = (1 * (1 - HBM_BUTTON_TIME_PROGRESS(0.0833))) + (HBM_BUTTONMAIN_HOVERED_SCALE * HBM_BUTTON_TIME_PROGRESS(0.0833));
			else {
				this->Scale = HBM_BUTTONMAIN_HOVERED_SCALE;
				HBM_BUTTON_TIME_CLEAR
			}
			break;

		// Selected state
		case 2:
			this->Scale = HBM_BUTTONMAIN_HOVERED_SCALE;
			HBM_Settings.InteractionLayer = 1;

			if (HBM_BUTTON_TIME_WAITING(0.050))
				this->MaskOpacity = 1 * HBM_BUTTON_TIME_PROGRESS(0.050);
			else if (HBM_BUTTON_TIME_WAITING(0.1667))
				this->MaskOpacity = 1;
			else if (HBM_BUTTON_TIME_WAITING(0.250))
				this->MaskOpacity = 1 * (1 - HBM_BUTTON_TIME_PROGRESS_PARTIAL(0.1667, 0.25));

			if (!HBM_BUTTON_TIME_WAITING(0.2667) && this->Substatus == 0) {
				if (this->Selected != NULL) this->Selected();
				this->Substatus = 1;
			}

			if (!HBM_BUTTON_TIME_WAITING(0.6667)) {
				HBM_BUTTON_TIME_RESET
				HBM_Settings.InteractionLayer = 0;
				this->Substatus = 0;
				this->Status = 0;
			}
			break;
	}
}

f64 HBMButtonMain::GetTime() {
	return ((f64)ticks_to_millisecs(gettime()) / 1000.0F);
}

u8 HBMButtonMain::GetStatus() {
	return this->Status;
}

#undef HBM_BUTTONMAIN_HOVERED_SCALE