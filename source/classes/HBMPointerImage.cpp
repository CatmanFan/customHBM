#include "hbm/hbm.h"

HBMPointerImage::HBMPointerImage() {
	this->ShadowOpacity = 0.33;
	this->ShadowX = this->ShadowY = 4;

	this->Visible = false;
	this->Shadow.ScaleY = this->Shadow.ScaleX = this->Image.ScaleY = this->Image.ScaleX = 0.92F;
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
	if (!this->loaded) {
		if (first == NULL)
			this->first = true;

		// Load images
		this->Image.LoadPNG(num == 4 ? &HBM_cursor4_png :
							num == 3 ? &HBM_cursor3_png :
							num == 2 ? &HBM_cursor2_png :
							&HBM_cursor1_png, 44, 64);

		// Load shadow
		if (this->first)
			this->Shadow.LoadPNG(&HBM_cursor_shadow_png, 44, 64);
		else
			this->Shadow.LoadRaw(first->Shadow.Img, 44, 64);

		this->Image.AnchorPoint = {9, 27};
		this->Shadow.AnchorPoint = {9, 27};

		this->loaded = true;
	}
}

void HBMPointerImage::Update() {
	this->Shadow.Rotation = this->Image.Rotation;
}