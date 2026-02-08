#include "hbm/hbm.h"
#include "hbm/extern.h"

HBMElement::HBMElement() {
	// Load images
	// this->Image.LoadPNG(&..., 64, 64);
	// this->SetHitbox(0, 0, 64, 64);

	// Load shadow
	// this->Shadow.LoadRaw(this->Image.GetImage(), this->Image.GetWidth(), this->Image.GetHeight());
	// this->Shadow.R = this->Shadow.G = this->Shadow.B = 0;
	// this->ShadowOpacity = 0.58;
	// this->ShadowX = this->ShadowY = 5;

	// this->TimerStop();
}

HBMElement::~HBMElement() {
	// Unload from memory
	this->Image.Free();
	this->Shadow.Free();
}

void HBMElement::Draw() {
	this->Shadow.Visible = this->Image.Visible = this->Visible;
	this->Shadow.ScaleX = this->Image.ScaleX = this->Scale;
	this->Shadow.ScaleY = this->Image.ScaleY = this->Scale;

	if (this->Visible) {
		if (this->ShadowOpacity > 0) {
			this->Shadow.Color.R = 0;
			this->Shadow.Color.G = 0;
			this->Shadow.Color.B = 0;
			this->Shadow.Color.A = lround(this->Image.Color.A * this->ShadowOpacity);
			this->Shadow.Draw(this->X + this->Image.X + (this->ShadowX * this->Scale), this->Y + this->Image.Y + (this->ShadowY * this->Scale));
		}
		this->Image.Draw(this->X + this->Image.X, this->Y + this->Image.Y);
	}
}

void HBMElement::Update() {
	// Your code here
}

void HBMElement::SetOpacity(float value) {
	if (value <= 0) this->Visible = false;
	else if (value >= 1) {
		this->Visible = true;
		this->Image.Color.A = 255;
	} else {
		this->Visible = true;
		this->Image.Color.A = lround(255 * value);
	}
}

bool HBMElement::HitboxTouched(int chan) {
	if (this->Blocked || HBM_ExitTransition.Fade > 0 || ((HBM_Settings.Stage & HBM_STAGE_BLOCKED) == HBM_STAGE_BLOCKED))
			return false;

	if (this->Hitbox.Width < 1 || this->Hitbox.Height < 1 || HBMPointers[chan].Status == HBM_POINTER_INACTIVE) return false;

	return /* First hitbox */
			((HBMPointers[chan].X >= this->X + this->Hitbox.X && HBMPointers[chan].Y >= this->Y + this->Hitbox.Y)
			 && (HBMPointers[chan].X < this->X + this->Hitbox.X + this->Hitbox.Width && HBMPointers[chan].Y < this->Y + this->Hitbox.Y + this->Hitbox.Height))

		   /* Second hitbox */
			 || ((this->Hitbox2.Width >= 1 && this->Hitbox2.Height >= 1)
			 && (HBMPointers[chan].X >= this->X + this->Hitbox2.X && HBMPointers[chan].Y >= this->Y + this->Hitbox2.Y)
			 && (HBMPointers[chan].X < this->X + this->Hitbox2.X + this->Hitbox2.Width && HBMPointers[chan].Y < this->Y + this->Hitbox2.Y + this->Hitbox2.Height));
}

u8 HBMElement::HitboxStatus(bool conditions) {
	for (u8 i = 0; i < HBM_MAX_POINTERS; i++) {
		if (this->HitboxTouched(i)) {
			if (conditions) {
				if ((HBMPointers[i].Status == HBM_POINTER_CC && WPAD_ButtonsDown(i) & WPAD_CLASSIC_BUTTON_A)
				 || (HBMPointers[i].Status == HBM_POINTER_IR && WPAD_ButtonsDown(i) & WPAD_BUTTON_A)) {
					return ((i & 0x0F) | 0x10);
				} else {
					return ((i & 0x0F) | 0x00);
				}
			}
		}
	}

	return -1;
}