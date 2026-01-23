#include "hbm.h"
#include "hbm/extern.h"

#define HBM_HEADER_FADE_TIME (this->WiiRemote != NULL ? 0.150 : 0.0667)

HBMHeader::HBMHeader() : HBMElement::HBMElement() {
	this->SetHitbox(0, 0, (HBM_Settings.Widescreen ? 832 : HBM_WIDTH), 100);

	// Load images
	this->Image.LoadPNG(&HBM_header_png, 40, 100);
	this->Highlighted.LoadPNG(&HBM_headerHighlighted_png, 40, 100);
	this->HighlightedOpacity = 0;
	this->MaskOpacity = 0;

	this->Highlighted.ScaleX = this->Image.ScaleX = 50;
	this->Highlighted.ScaleY = this->Image.ScaleY = 1;

	this->Selected = NULL;
	this->AfterSelected = NULL;
	this->Inverted = false;
	this->Blocked = false;
	this->TimerStop();
}

HBMHeader::~HBMHeader() {
	this->Highlighted.Free();
	this->Image.Free();
}

void HBMHeader::Draw() {
	if (this->Inverted && this->Image.ScaleY >= 0)
		this->Highlighted.ScaleY = this->Image.ScaleY = -1;
	if (!this->Inverted && this->Image.ScaleY < 0)
		this->Highlighted.ScaleY = this->Image.ScaleY = 1;

	if (this->Visible) {
		this->Image.Draw((HBM_Settings.Widescreen ? HBM_WIDTH * HBM_WIDESCREEN_RATIO : HBM_WIDTH) / 2, this->Y);

		// Draw highlighted sprite
		if (this->HighlightedOpacity > 0) {
			this->Highlighted.A = lround(255 * this->HighlightedOpacity);
			this->Highlighted.Draw((HBM_Settings.Widescreen ? HBM_WIDTH * HBM_WIDESCREEN_RATIO : HBM_WIDTH) / 2, this->Y);
		}

		// Draw text
		if (this->TextOpacity > 0 && this->Text != NULL) {
			HBM_DrawText
			(
				/* text */		this->Text,
				/* X */			this->X + this->TextX,
				/* Y */			this->Y + this->TextY,
				/* size */		this->TextSize,
				/* scaleX */	1.005,
				/* scaleY */	1,
				/* align */		this->TextCentered ? HBM_TEXT_CENTER : HBM_TEXT_LEFT, HBM_TEXT_MIDDLE,
				/* serif */		false,
				/* color */		255, 255, 255, lround(255 * this->TextOpacity),
				/* maxWidth */	this->TextRectWidth,
				/* maxHeight */	this->TextRectHeight
			);
		}

		// Draw selected sprite
		if (this->MaskOpacity > 0) {
			HBM_DrawQuad(this->X, this->Y, this->Hitbox.Width, this->Hitbox.Height, 255, this->MaskOpacity, true);
		}

		// Draw bottom Wii Remote
		if (this->WiiRemote != NULL) {
			this->WiiRemote->Draw(29 + (HBM_Settings.Widescreen ? 118.75 : 0),
								  this->Y - (HBM_Settings.InteractionLayer == HBM_INTERACTION_WPAD
																		   ? 32 + (300 * this->WiiRemoteSlider)
																		   : 16 + (16 * this->WiiRemoteSlider)));
		}
	}
}

void HBMHeader::Call() {
	HBM_Settings.InteractionLayer = HBM_INTERACTION_BLOCKED;

	this->Status = 2;
	this->SetAnimation(2, true);
	if (this->Inverted) {
		HBM_SOUND(HBM_sfx_menuclose_pcm, false);
	}
	else if (HBM_Settings.InteractionLayer == HBM_INTERACTION_WPAD) {
		HBM_SOUND(HBM_sfx_cancel_pcm, false);
	}
	else {
		HBM_SOUND(HBM_sfx_select_pcm, false);
	}

	if (this->Selected != NULL) this->Selected(); // Call AFTER setting interaction layer
}

void HBMHeader::Update() {
	/*******************
	 * Animation
	 *******************/
	this->UpdateAnimation();
	switch (this->Animation) {
		default:
			break;

		// Inactive state
		case 1:
			if (!this->TimerPassed(HBM_HEADER_FADE_TIME)) {
				this->HighlightedOpacity = 1 - this->TimerProgress(HBM_HEADER_FADE_TIME);
				this->WiiRemoteSlider = this->HighlightedOpacity;
			} else {
				this->HighlightedOpacity = 0;
				this->WiiRemoteSlider = 0;
				this->SetAnimation(/* stop */);
			}
			break;

		// Hovered state
		case 2:
			if (!this->TimerPassed(HBM_HEADER_FADE_TIME)) {
				this->HighlightedOpacity = this->TimerProgress(HBM_HEADER_FADE_TIME);
				this->WiiRemoteSlider = this->HighlightedOpacity;
			} else {
				this->HighlightedOpacity = 1;
				this->WiiRemoteSlider = 1;
				this->SetAnimation(/* stop */);
			}
			break;

		// Selected state
		case 3:
			this->WiiRemoteSlider = 1;
			if (!this->TimerPassed(0.1667)) {
				this->HighlightedOpacity = 1;
				this->MaskOpacity = this->TimerProgress(0.1667);
			}
			else if (!this->TimerPassed(0.3167)) {
				this->HighlightedOpacity = this->WiiRemote != NULL ? 1 : 0;
				this->MaskOpacity = 1 - this->TimerProgress(0.1667, 0.3167);
			}
			else {
				if (HBM_Settings.InteractionLayer == HBM_INTERACTION_BLOCKED)
					HBM_Settings.InteractionLayer = HBM_INTERACTION_MAIN;

				this->HighlightedOpacity = this->WiiRemote != NULL ? 1 : 0;
				this->MaskOpacity = 0;
				this->SetAnimation(/* stop */);
				this->Status = this->WiiRemote != NULL ? 1 : 0;

				if (this->AfterSelected != NULL) this->AfterSelected();
			}
			break;
	}

	/*******************
	 * Status control
	 *******************/
	u8 chan = this->HitboxStatus(this->Status != 2 && HBM_Settings.InteractionLayer == HBM_INTERACTION_MAIN);
	switch ((chan >> 4) & 0x0F) {
		case 0: // Hovering
			if (this->Status != 1) {
				this->Status = 1;
				if (!this->CheckAnimationPlaying(1))
					this->SetAnimation(1);
				HBM_SOUND(HBM_sfx_hover_pcm, false);
				HBM_PointerRumble((chan & 0x0F), 0.08, 0);
			}
			break;

		case 1: // Pressed
			if (this->Status != 2) {
				this->Call();
			}
			break;

		default: // Lost focus, set to default state
			if (this->Status == 1) {
				this->Status = 0;
				this->SetAnimation(0);
			}
			break;
	}

	if (this->Status == 1 && this->CheckAnimationStopped(0)) this->SetAnimation(1, true);
	if (this->Status == 0 && this->CheckAnimationStopped(1)) this->SetAnimation(0, true);
}

#undef HBM_HEADER_FADE_TIME