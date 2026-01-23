#include "hbm.h"

HBMRemoteDataSprite::HBMRemoteDataSprite() {
	this->Text = NULL;
	this->FlashTime = 0;

	this->BatteryIndex = 0;
	this->Battery.LoadPNG(&HBM_remoteBattery_0_png, 36, 20);
	this->BatteryDisconnected.LoadRaw(this->Battery.GetImage(), 36, 20);

	this->Battery.Visible = true;
	this->BatteryDisconnected.Visible = true;
	this->BatteryDisconnected.R = 50;
	this->BatteryDisconnected.G = 50;
	this->BatteryDisconnected.B = 50;
	this->BatteryDisconnected.A = 255;
}

HBMRemoteDataSprite::~HBMRemoteDataSprite() {
	this->Text = NULL;
	this->FlashTime = 0;
	this->BatteryDisconnected.Free();
	this->Battery.Free();
}

void HBMRemoteDataSprite::SetBatteryImage(u8 value) {
	if (this->BatteryIndex == value || this->BatteryBlocked) return;

	this->BatteryIndex = value;
	switch (value) {
		default:
		case 0:
			this->Battery.R = 255;
			this->Battery.G = 255;
			this->Battery.B = 255;
			this->Battery.LoadPNG(&HBM_remoteBattery_0_png, 36, 20);
			this->BatteryDisconnected.LoadRaw(this->Battery.GetImage(), 36, 20);
			break;

		case 1:
			this->Battery.R = 255;
			this->Battery.G = 0;
			this->Battery.B = 0;
			this->Battery.LoadPNG(&HBM_remoteBattery_0_png, 36, 20);
			this->BatteryDisconnected.LoadRaw(this->Battery.GetImage(), 36, 20);
			break;

		case 2:
			this->Battery.R = 255;
			this->Battery.G = 0;
			this->Battery.B = 0;
			this->Battery.LoadPNG(&HBM_remoteBattery_1_png, 36, 20);
			this->BatteryDisconnected.LoadRaw(this->Battery.GetImage(), 36, 20);
			break;

		case 3:
			this->Battery.R = 255;
			this->Battery.G = 255;
			this->Battery.B = 255;
			this->Battery.LoadPNG(&HBM_remoteBattery_2_png, 36, 20);
			this->BatteryDisconnected.LoadRaw(this->Battery.GetImage(), 36, 20);
			break;

		case 4:
			this->Battery.R = 255;
			this->Battery.G = 255;
			this->Battery.B = 255;
			this->Battery.LoadPNG(&HBM_remoteBattery_3_png, 36, 20);
			this->BatteryDisconnected.LoadRaw(this->Battery.GetImage(), 36, 20);
			break;

		case 5:
			this->Battery.R = 255;
			this->Battery.G = 255;
			this->Battery.B = 255;
			this->Battery.LoadPNG(&HBM_remoteBattery_4_png, 36, 20);
			this->BatteryDisconnected.LoadRaw(this->Battery.GetImage(), 36, 20);
			break;
	}
}

void HBMRemoteDataSprite::Flash(bool disconnected) {
	this->FlashTime = (f64)HBM_GETTIME / 1000.0F;
	this->SetBatteryImage(0);
	this->Disconnected = disconnected;
	this->BatteryBlocked = true;
	this->Battery.A = 255;
	this->BatteryDisconnected.A = 0;
}

void HBMRemoteDataSprite::Draw(int X, int Y) {
	if (this->FlashTime > 0) {
		f64 flashPassed = ((f64)HBM_GETTIME / 1000.0F) - this->FlashTime;
		if (flashPassed < 0.35 * 1)
			this->Battery.A = 255 - lround((flashPassed / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 2)
			this->Battery.A = lround(((flashPassed - 0.35 * 1) / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 3)
			this->Battery.A = 255 - lround(((flashPassed - 0.35 * 2) / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 4)
			this->Battery.A = lround(((flashPassed - 0.35 * 3) / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 5) {
			this->Battery.A = 255;
			this->BatteryDisconnected.A = lround(((flashPassed - 0.35 * 4) / 0.35) * 255.0F);
		} else {
			this->BatteryBlocked = false;
			this->Battery.A = 255;
			this->BatteryDisconnected.A = 255;
			this->FlashTime = 0;
		}
	}

	this->Battery.Draw(X, Y);
	HBM_DrawText
	(
		/* text */		this->Text,
		/* X */			X - 28.2,
		/* Y */			Y + 6,
		/* size */		20,
		/* scaleX */	1,
		/* scaleY */	1,
		/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
		/* serif */		false,
		/* color */		this->Battery.R, this->Battery.G, this->Battery.B, this->Battery.A,
		/* maxWidth */	48,
		/* maxHeight */	24
	);

	if (this->Disconnected && this->BatteryDisconnected.A > 0) {
		this->BatteryDisconnected.Draw(X, Y);
		HBM_DrawText
		(
			/* text */		this->Text,
			/* X */			X - 28.2,
			/* Y */			Y + 6,
			/* size */		20,
			/* scaleX */	1,
			/* scaleY */	1,
			/* align */		HBM_TEXT_CENTER, HBM_TEXT_MIDDLE,
			/* serif */		false,
			/* color */		65, 65, 65, this->BatteryDisconnected.A,
			/* maxWidth */	48,
			/* maxHeight */	24
		);
	}
}