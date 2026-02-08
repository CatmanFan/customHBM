#include "hbm/hbm.h"
#include "hbm/extern.h"

HBMButton::HBMButton() : HBMElement::HBMElement() {
	// Your code here
}

HBMButton::~HBMButton() {
	this->Mask.Free();
	this->Shadow.Free();
	this->Image.Free();
}

void HBMButton::Draw() {
	// Your code here
	// HBMElement::Draw();
}

void HBMButton::Update() {
	// Your code here
}