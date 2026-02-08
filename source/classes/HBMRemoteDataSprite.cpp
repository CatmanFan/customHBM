#include "hbm/hbm.h"

HBMRemoteDataSprite::HBMRemoteDataSprite() {
	this->Text = NULL;
	this->FlashTime = 0;

	this->BatteryIndex = 0;
	this->Battery.LoadPNG(&HBM_remoteBattery_0_png, 36, 20);
	this->BatteryDisconnected.LoadRaw(this->Battery.Img, 36, 20);

	this->Battery.Visible = true;
	this->BatteryDisconnected.Visible = true;
	this->BatteryDisconnected.Color = {50, 50, 50, 255};
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
			this->Battery.Color = {255, 255, 255, 255};
			this->Battery.LoadPNG(&HBM_remoteBattery_0_png, 36, 20);
			break;

		case 1:
			this->Battery.Color = {255, 0, 0, 255};
			this->Battery.LoadPNG(&HBM_remoteBattery_0_png, 36, 20);
			break;

		case 2:
			this->Battery.Color = {255, 0, 0, 255};
			this->Battery.LoadPNG(&HBM_remoteBattery_1_png, 36, 20);
			break;

		case 3:
			this->Battery.Color = {255, 255, 255, 255};
			this->Battery.LoadPNG(&HBM_remoteBattery_2_png, 36, 20);
			break;

		case 4:
			this->Battery.Color = {255, 255, 255, 255};
			this->Battery.LoadPNG(&HBM_remoteBattery_3_png, 36, 20);
			break;

		case 5:
			this->Battery.Color = {255, 255, 255, 255};
			this->Battery.LoadPNG(&HBM_remoteBattery_4_png, 36, 20);
			break;
	}
	this->BatteryDisconnected.LoadRaw(this->Battery.Img, 36, 20);
}

void HBMRemoteDataSprite::Flash(bool disconnected) {
	this->FlashTime = (f64)HBM_GETTIME / 1000.0F;
	this->SetBatteryImage(0);
	this->Disconnected = disconnected;
	this->BatteryBlocked = true;
	this->Battery.Color.A = 255;
	this->BatteryDisconnected.Color.A = 0;
}

void HBMRemoteDataSprite::Draw(int X, int Y) {
	if (this->FlashTime > 0) {
		f64 flashPassed = ((f64)HBM_GETTIME / 1000.0F) - this->FlashTime;
		if (flashPassed < 0.35 * 1)
			this->Battery.Color.A = 255 - lround((flashPassed / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 2)
			this->Battery.Color.A = lround(((flashPassed - 0.35 * 1) / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 3)
			this->Battery.Color.A = 255 - lround(((flashPassed - 0.35 * 2) / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 4)
			this->Battery.Color.A = lround(((flashPassed - 0.35 * 3) / 0.35) * 255.0F);
		else if (flashPassed < 0.35 * 5) {
			this->Battery.Color.A = 255;
			this->BatteryDisconnected.Color.A = lround(((flashPassed - 0.35 * 4) / 0.35) * 255.0F);
		} else {
			this->BatteryBlocked = false;
			this->Battery.Color.A = 255;
			this->BatteryDisconnected.Color.A = 255;
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
		/* color */		this->Battery.Color.R, this->Battery.Color.G, this->Battery.Color.B, this->Battery.Color.A,
		/* maxWidth */	48,
		/* maxHeight */	24
	);

	if (this->Disconnected && this->BatteryDisconnected.Color.A > 0) {
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
			/* color */		65, 65, 65, this->BatteryDisconnected.Color.A,
			/* maxWidth */	48,
			/* maxHeight */	24
		);
	}
}