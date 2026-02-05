#include "hbm.h"
#include "hbm/extern.h"

#define HBM_DIALOG_TRANSITION_TIME 0.390F
#define HBM_DIALOG_TRANSITION_SLIDE_OUT -520
#define HBM_DIALOG_TRANSITION_SLIDE_IN (this->SlideFromTop ? HBM_DIALOG_TRANSITION_SLIDE_OUT : 520)
#define HBM_DIALOG_TRANSITION_PROGRESS (this->Status == 2 ? MAX(0, 1 - (this->GetTimePassed() / HBM_DIALOG_TRANSITION_TIME)) \
										: MIN(1, (this->GetTimePassed() / HBM_DIALOG_TRANSITION_TIME)))

#define HBM_DIALOG_X (HBM_Settings.Widescreen ? 161 : 48)
#define HBM_DIALOG_Y 62

HBMDialog::HBMDialog() {
	this->Image.LoadPNG(&HBM_dialogBG_png, 522, 366);
	this->Image.SetAnchorPoint(255.5, 177.5);
	this->SetHitbox(0, 0, 0, 0);
	this->ShadowOpacity = 0;
	this->Visible = true;
	this->Reset();
}

HBMDialog::~HBMDialog() {
	this->Image.Free();
}

void HBMDialog::Draw() {
	if ((HBM_Settings.Stage & HBM_STAGE_DIALOG) == HBM_STAGE_DIALOG) {
		HBM_DrawQuad(0, 0, HBM_WIDTH, HBM_HEIGHT, 0, this->Status > 0 ? 0.7333F * HBM_EASEINOUT(HBM_DIALOG_TRANSITION_PROGRESS) : 0.7333F, true);
		HBMElement::Draw();
		for (int i = 0; i < 1; i++) {
			HBM_DrawText
			(
				/* text */		this->Text,
				/* X */			this->X + 256,
				/* Y */			this->Y + (this->AltAppearance ? 134 : 132),
				/* size */		HBM_FontType() >= 1 ? 27 : -1,
				/* scaleX */	this->AltAppearance ? 0.82 : 1.02,
				/* scaleY */	this->AltAppearance ? 0.9 : 1.02,
				/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
				/* serif */		true,
				/* color */		101, 101, 101, 240,
				/* maxWidth */	510
			);
		}
		this->Button2.Draw();
		this->Button1.Draw();
	}
}

void HBMDialog::Update() {
	// Correct positions
	if (this->X != HBM_DIALOG_X) {
		this->X = HBM_DIALOG_X;
		this->Button1.X = HBM_DIALOG_X + (this->Button2.Visible ? 15 : 137);
		this->Button2.X = HBM_DIALOG_X + 260;
	}

	if (this->AltAppearance != this->Button1.AltAppearance) {
		this->Button1.AltAppearance = this->AltAppearance;
		this->Button2.AltAppearance = this->AltAppearance;
	}

	if ((HBM_Settings.Stage & HBM_STAGE_DIALOG) == HBM_STAGE_DIALOG) {
		this->TimerUpdate();
		switch (this->Status) {
			case 0:
			default:
				if (this->Button1.CloseDialog || this->Button2.CloseDialog)
					this->Hide();
				break;

			case 1:
			case 2:
				if (!this->TimerPassed(HBM_DIALOG_TRANSITION_TIME)) {
					this->Y = HBM_DIALOG_Y + ((this->Status == 2 ? HBM_DIALOG_TRANSITION_SLIDE_OUT : HBM_DIALOG_TRANSITION_SLIDE_IN)
										   * (1.0F - HBM_EASEINOUT(HBM_DIALOG_TRANSITION_PROGRESS)));
					this->Button2.Y = this->Button1.Y = HBM_DIALOG_Y + (this->AltAppearance ? 266 : 264)
									+ ((this->Status == 2 ? HBM_DIALOG_TRANSITION_SLIDE_OUT : HBM_DIALOG_TRANSITION_SLIDE_IN)
									* (1.0F - HBM_EASEINOUT(HBM_DIALOG_TRANSITION_PROGRESS)));

					// Set button status
					this->Button2.Blocked = this->Button1.Blocked = true;
				} else {
					// Set final positions of elements
					this->Y = HBM_DIALOG_Y;
					this->Button2.Y = this->Button1.Y = HBM_DIALOG_Y + (this->AltAppearance ? 266 : 264);

					// Set button status
					this->Button2.CloseDialog = this->Button1.CloseDialog = false;
					this->Button1.Blocked = !this->Button1.Visible;
					this->Button2.Blocked = !this->Button2.Visible;

					// Turn off dialog
					if (this->Status == 2) {
						this->X = 0;
						HBM_Settings.Stage &= ~HBM_STAGE_DIALOG;
					}

					HBM_Settings.Stage &= ~HBM_STAGE_BLOCKED;
					this->TimerStop();
					this->Status = 0;
				}
				break;
		}
	}

	this->Button1.Update();
	this->Button2.Update();
}

void HBMDialog::Show() {
	if ((HBM_Settings.Stage & HBM_STAGE_DIALOG) != HBM_STAGE_DIALOG) {
		HBM_ConsolePrintf("Opening dialog");

		this->X = HBM_DIALOG_X;
		this->Y = HBM_DIALOG_Y;

		if (Confirm != NULL) {
			this->Button1.Selected = Confirm;
			this->Button2.Selected = NULL;
			this->Button1.Sound = 2; // Confirm sound
			this->Button2.Sound = 1; // Cancel sound
		} else {
			HBM_SOUND(HBM_sfx_dialog_pcm, false);
			this->Button1.Selected = NULL;
			this->Button2.Selected = NULL;
			this->Button1.Sound = 0; // Normal sound
			this->Button2.Sound = 0; // Normal sound
		}

		HBM_Settings.Stage |= (HBM_STAGE_DIALOG | HBM_STAGE_BLOCKED);
		this->TimerStart();
		this->Status = 1;
	}
}

void HBMDialog::Hide() {
	if ((HBM_Settings.Stage & HBM_STAGE_DIALOG) == HBM_STAGE_DIALOG) {
		HBM_ConsolePrintf("Closing dialog");

		this->TimerStart();
		this->Status = 2;
	}
}

void HBMDialog::Reset() {
	this->Text = NULL;
	this->Button1.Text = NULL;
	this->Button2.Text = NULL;
	this->Button1.Visible = false;
	this->Button2.Visible = false;

	HBM_Settings.Stage &= ~HBM_STAGE_DIALOG;
	this->Status = 0;
}

void HBMDialog::UpdateText(const char* Text) {
	this->Text = Text;

	this->Button1.Visible = false;
	this->Button2.Visible = false;
}

void HBMDialog::UpdateText(const char* Text, const char* Button1) {
	this->Text = Text;
	this->Button1.Text = Button1;

	this->Button1.X = HBM_DIALOG_X + 137;
	this->Button1.Visible = true;
	this->Button2.Visible = false;
}

void HBMDialog::UpdateText(const char* Text, const char* Button1, const char* Button2) {
	this->Text = Text;
	this->Button1.Text = Button1;
	this->Button2.Text = Button2;

	this->Button1.X = HBM_DIALOG_X + 15;
	this->Button1.Visible = true;
	this->Button2.Visible = true;
}

#undef HBM_DIALOG_TRANSITION_TIME
#undef HBM_DIALOG_TRANSITION_SLIDE_OUT
#undef HBM_DIALOG_TRANSITION_SLIDE_IN
#undef HBM_DIALOG_TRANSITION_PROGRESS

#undef HBM_DIALOG_X
#undef HBM_DIALOG_Y