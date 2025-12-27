#include "hbm.h"
#include "hbm/extern.h"

HBMButton::HBMButton() : HBMElement::HBMElement() {
	// Your code here
}

HBMButton::~HBMButton() {
	this->Shadow.Free();
	this->Image.Free();
}

void HBMButton::DrawText() {
	// Draw text
	if (this->Image.A > 0 && this->Visible) {
		HBM_DrawText
		(
			/* text */		this->Text,
			/* X */			this->X + (this->Image.GetWidth() / 2),
			/* Y */			this->Y + (this->Image.GetHeight() / 2.1),
			/* size */		this->TextSize,
			/* scaleX */	this->Scale,
			/* scaleY */	this->Scale,
			/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
			/* serif */		false,
			/* color */		(this->TextColor & 0xFF000000) >> 24,
							(this->TextColor & 0x00FF0000) >> 16,
							(this->TextColor & 0x0000FF00) >> 8,
							this->Image.A,
							(this->Image.GetWidth() - 40)
		);
	}
}

void HBMButton::Draw() {
	HBMElement::Draw();

	if (!this->TextOverMask) this->DrawText();

	// Draw mask
	if (this->MaskOpacity > 0) {
		this->Mask.Scale = this->Image.Scale;
		this->Mask.A = lround(255 * this->MaskOpacity);
		this->Mask.Draw(this->X + this->Image.GetX(), this->Y + this->Image.GetY());
	}

	if (this->TextOverMask) this->DrawText();
}

void HBMButton::Update() {
	// Your code here
}

f64 HBMButton::GetTime() {
	return ((f64)ticks_to_millisecs(gettime()) / 1000.0F);
}

u8 HBMButton::GetStatus() {
	return this->Status;
}