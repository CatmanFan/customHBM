#include "HBM.h"

void HBMElement::Draw() {
	this->Shadow.Visible = this->Image.Visible = this->Visible;
	this->Shadow.Scale = this->Image.Scale = this->Scale;

	if (this->Visible) {
		if (this->ShadowOpacity > 0) {
			this->Shadow.A = lround(this->Image.A * this->ShadowOpacity);
			this->Shadow.Draw(this->X + this->Image.GetX() + (this->ShadowX * this->Scale), this->Y + this->Image.GetY() + (this->ShadowY * this->Scale));
		}
		this->Image.Draw(this->X + this->Image.GetX(), this->Y + this->Image.GetY());
	}
}

void HBMElement::Update() {
	// Your code here
}

void HBMElement::SetOpacity(float value) {
	if (value <= 0) this->Visible = false;
	else if (value >= 1) {
		this->Visible = true;
		this->Image.A = 255;
	} else {
		this->Visible = true;
		this->Image.A = lround(255 * value);
	}
}

void HBMElement::SetHitbox(int w, int h) {
	this->Hitbox.Width = w;
	this->Hitbox.Height = h;
}

void HBMElement::SetHitbox(int x, int y, int w, int h) {
	this->Hitbox.X = x;
	this->Hitbox.Y = y;
	this->Hitbox.Width = w;
	this->Hitbox.Height = h;
}

void HBMElement::SetPosition(u16 x, u16 y) {
	this->X = x;
	this->Y = y;
}

u16 HBMElement::GetX() {
	return this->X;
}

u16 HBMElement::GetY() {
	return this->Y;
}

bool HBMElement::HitboxTouched() {
	if (this->Hitbox.Width < 1 || this->Hitbox.Height < 1) return false;

	for (int i = 0; i < 4 /* WPAD_MAX_WIIMOTES */; i++) {
		if ((HBMPointers[i].X >= this->X + this->Hitbox.X && HBMPointers[i].Y >= this->Y + this->Hitbox.Y)
		 && (HBMPointers[i].X < this->X + this->Hitbox.X + this->Hitbox.Width && HBMPointers[i].Y < this->Y + this->Hitbox.Y + this->Hitbox.Height)
		 && HBMPointers[i].Status != HBM_POINTER_INACTIVE)
			return true;
	}

	return false;
}

bool HBMElement::HitboxTouched(int chan) {
	if (this->Hitbox.Width < 1 || this->Hitbox.Height < 1 || HBMPointers[chan].Status == HBM_POINTER_INACTIVE) return false;
	else return (HBMPointers[chan].X >= this->X + this->Hitbox.X && HBMPointers[chan].Y >= this->Y + this->Hitbox.Y)
			 && (HBMPointers[chan].X < this->X + this->Hitbox.X + this->Hitbox.Width && HBMPointers[chan].Y < this->Y + this->Hitbox.Y + this->Hitbox.Height);
}