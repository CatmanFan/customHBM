#include "hbm.h"

HBMPointerImage::HBMPointerImage() {
	this->Shadow.R = this->Shadow.G = this->Shadow.B = 0;
	this->ShadowOpacity = 0.33;
	this->ShadowX = this->ShadowY = 4;

	this->Visible = false;
	this->Image.Scale = 0.92F;
}

HBMPointerImage::~HBMPointerImage() {
	if (this->loaded) {
		// Unload from memory
		this->Image.Free();
		if (this->first) this->Shadow.Free();
		this->loaded = false;
	}
}

void HBMPointerImage::Load(int num, HBMPointerImage *first) {
	if (first == NULL) this->first = true;

	// Load images
	this->Image.LoadPNG(num == 4 ? &HBM_cursor4_png :
						num == 3 ? &HBM_cursor3_png :
						num == 2 ? &HBM_cursor2_png :
						&HBM_cursor1_png, 43, 62);
	this->Image.SetAnchorPoint(9, 27);

	// Load shadow
	if (this->first) this->Shadow.LoadPNG(&HBM_cursor_shadow_png, 44, 62);
	else this->Shadow.LoadRaw(first->Shadow.GetImage(), 44, 62);
	this->Shadow.SetAnchorPoint(9, 27);

	this->loaded = true;
}

void HBMPointerImage::Update() {
	this->Shadow.Rotation = this->Image.Rotation;
}