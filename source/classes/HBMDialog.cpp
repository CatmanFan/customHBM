#include "hbm.h"
#include "hbm/extern.h"

#define HBM_DIALOG_TRANSITION_TIME 0.390F
#define HBM_DIALOG_TRANSITION_POSITION -600
#define HBM_DIALOG_TRANSITION_PROGRESS (this->Status == 2 ? 1 - this->Transition : this->Transition)

#define HBM_DIALOG_X (HBM_Settings.Widescreen ? 161 : 48)
#define HBM_DIALOG_Y 62

HBMDialog::HBMDialog() {
	this->Image.LoadPNG(&HBM_dialogBG_png, 512, 356);
	this->SetHitbox(0, 0, 0, 0);
	this->ShadowOpacity = 0;
	this->Button2.Visible = this->Button1.Visible = this->Visible = true;

	this->Active = false;
	this->Status = 0;
}

HBMDialog::~HBMDialog() {
	this->Image.Free();
}

void HBMDialog::Draw() {
	if (HBM_Settings.InteractionLayer == HBM_INTERACTION_DIALOG || HBM_Settings.InteractionLayer == HBM_INTERACTION_DIALOGBUTTON) {
		HBM_DrawBlackQuad(0, 0, HBM_Settings.Width, HBM_Settings.Height, this->Status > 0 ? 0.7333F * HBM_EASEINOUT(HBM_DIALOG_TRANSITION_PROGRESS) : 0.7333F, true);
		HBMElement::Draw();
		HBM_DrawText
		(
			/* text */		this->Text,
			/* X */			this->X + 256,
			/* Y */			this->Y + 136,
			/* size */		31,
			/* scaleX */	1.0,
			/* scaleY */	1.0,
			/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
			/* serif */		true,
			/* color */		100, 100, 100, 255,
							this->Image.GetWidth()
		);
		this->Button2.Draw();
		this->Button1.Draw();
	}
}

void HBMDialog::Update() {
	// Correct positions
	if (this->X != HBM_DIALOG_X) {
		this->X = HBM_DIALOG_X;
		this->Button1.SetPosition (HBM_DIALOG_X + 15, HBM_DIALOG_Y + 264);
		this->Button2.SetPosition (HBM_DIALOG_X + 258, HBM_DIALOG_Y + 264);
	}

	if (HBM_Settings.InteractionLayer == HBM_INTERACTION_DIALOG || HBM_Settings.InteractionLayer == HBM_INTERACTION_DIALOGBUTTON) {
		switch (this->Status) {
			case 0:
			default:
				if (this->Button1.Cancelling || this->Button2.Cancelling)
					this->Hide();
				break;

			case 1:
			case 2:
				this->Transition = (((f64)ticks_to_millisecs(gettime()) / 1000.0F) - this->TransitionStart) / HBM_DIALOG_TRANSITION_TIME;

				if (this->Transition >= 1.0F) {
					this->Y = HBM_DIALOG_Y;
					this->Button2.Y = this->Button1.Y = HBM_DIALOG_Y + 264;

					if (this->Status == 2) {
						this->Button1.Cancelling = false;
						this->Button2.Cancelling = false;

						// Turn off dialog
						this->Active = false;
						HBM_Settings.InteractionLayer = HBM_INTERACTION_MAIN;
					}

					this->Button2.Blocked = this->Button1.Blocked = false;
					this->TransitionStart = 0;
					this->Transition = 0;

					this->Status = 0;
				} else {
					this->Y = HBM_DIALOG_Y + (HBM_DIALOG_TRANSITION_POSITION * (1.0F - HBM_EASEINOUT(HBM_DIALOG_TRANSITION_PROGRESS)));
					this->Button2.Y = this->Button1.Y = HBM_DIALOG_Y + 264 + (HBM_DIALOG_TRANSITION_POSITION * (1.0F - HBM_EASEINOUT(HBM_DIALOG_TRANSITION_PROGRESS)));
				}
				break;
		}
	}

	this->Button1.Update();
	this->Button2.Update();
}

void HBMDialog::Show() {
	if (!this->Active) {
		HBM_ConsolePrintf("Opening dialog");

		this->X = HBM_DIALOG_X;
		this->Y = HBM_DIALOG_Y;
		this->Button1.SetPosition (HBM_DIALOG_X + 15, HBM_DIALOG_Y + 264);
		this->Button2.SetPosition (HBM_DIALOG_X + 258, HBM_DIALOG_Y + 264);

		this->Button1.Selected = Confirm;
		this->Button1.Cancel = false;
		this->Button2.Cancel = true;

		this->Button2.Blocked = this->Button1.Blocked = true;
		this->TransitionStart = ((f64)ticks_to_millisecs(gettime()) / 1000.0F);
		this->Transition = 0;

		HBM_Settings.InteractionLayer = HBM_INTERACTION_DIALOG;
		this->Status = 1;
		this->Active = true;
	}
}

void HBMDialog::Hide() {
	if (this->Active && this->Status != 2) {
		HBM_ConsolePrintf("Closing dialog");

		this->Button2.Blocked = this->Button1.Blocked = true;
		this->TransitionStart = ((f64)ticks_to_millisecs(gettime()) / 1000.0F);
		this->Transition = 0;

		this->Status = 2;
	}
}

void HBMDialog::UpdateText(char *Text, char *Button1, char *Button2) {
	this->Text = Text;
	this->Button1.Text = Button1;
	this->Button2.Text = Button2;
}

#undef HBM_DIALOG_TRANSITION_TIME
#undef HBM_DIALOG_TRANSITION_POSITION
#undef HBM_DIALOG_TRANSITION_PROGRESS

#undef HBM_DIALOG_X
#undef HBM_DIALOG_Y