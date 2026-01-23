#include "hbm.h"
#include "hbm/extern.h"
#include <cstring>

void HBMImage::Draw(f32 xPos, f32 yPos)
{
	/************************************************************************************************
	 * DISCLAIMER:	I would like to disclose that the code for this function was derived,			*
	 *				with slight modifications, from the following:									*
	 *				- https://github.com/dborth/libwiigui/blob/master/source/video.cpp				*
	 *					(for rendering texture; licensed under GPL)									*
	 *				- https://github.com/TheProjecter/libhomemenu/blob/master/HomeMenu.c			*
	 *					(for applying rotation/position; unknown license)							*
	 ************************************************************************************************/

	if (!this->Visible || this->Img == NULL || this->TexObj == NULL) return;

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
	GX_InvalidateTexAll();

	GX_LoadTexObj(this->TexObj, GX_TEXMAP0);

	Mtx m,m1,m2,mv;

	guMtxIdentity (m1);
	guMtxTransApply(m1, m1, -this->Width/2, -this->Height/2, 0);
	guMtxScaleApply(m1, m1, this->ScaleX, this->ScaleY, 1.0);
	guMtxRotDeg(m2, 'z', this->Rotation);
	guMtxConcat(m2, m1, m);

	guMtxTransApply(m, m, this->Width/2 + xPos, this->Height/2 + yPos, 0);
	guMtxScaleApply(m, m, this->FixedSize == true ? HBM_Settings.ScaleX * HBM_WIDESCREEN_RATIO : HBM_Settings.ScaleX, HBM_Settings.ScaleY, 1.0);
	guMtxConcat(HBM_GXmodelView2D, m, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	GX_Begin(GX_QUADS, HBM_GX_VTXFMT, 4);

		GX_Position2f32(0, 0);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(0, 0);
		GX_Position2f32(this->Width, 0);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(1, 0);
		GX_Position2f32(this->Width, this->Height);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(1, 1);
		GX_Position2f32(0, this->Height);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(0, 1);

	GX_End();
	GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
}

void HBMImage::Draw()
{
	this->Draw(this->X, this->Y);
}

void HBMImage::LoadRaw(const void *img, u16 width, u16 height, u8 fmt) {
	// Failsafe
	if (img == NULL) return;
	this->Free();

	this->dynamic = false;
	this->Img = (void *)img;
	this->ImgFmt = fmt;
	this->ImgSize = sizeof(img);
	this->Width = width;
	this->Height = height;

	// Init tex obj
	this->TexObj = (GXTexObj *)malloc(sizeof(GXTexObj));
	DCFlushRange(this->TexObj, sizeof(GXTexObj));
	GX_InitTexObj(this->TexObj, (void*)this->Img, this->Width, this->Height, this->ImgFmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
}

void HBMImage::LoadPNG(const void *img, int width, int height) {
	// Failsafe
	if (img == NULL) return;
	this->Free();

	// Load textures using PNGU
	PNGUPROP imgProp;
	IMGCTX ctx = PNGU_SelectImageFromBuffer(img);
	if (PNGU_GetImageProperties(ctx, &imgProp) == PNGU_OK) {
		this->ImgSize = (imgProp.imgWidth * imgProp.imgHeight) << 2;
		this->Img = PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, &width, &height);
		if (this->Img != NULL) {
			this->dynamic = true;
			this->Width = width;
			this->Height = height;
			this->ImgFmt = GX_TF_RGBA8;
			/*if ((int)imgProp.imgWidth != width || (int)imgProp.imgHeight != height) {
				// PNGU has resized the texture
				memset(this->Img, 0, (this->Width * this->Height) << 2);
			}*/
			DCFlushRange(this->Img, this->ImgSize);

			// Init tex obj
			this->TexObj = (GXTexObj *)malloc(sizeof(GXTexObj));
			DCFlushRange(this->TexObj, sizeof(GXTexObj));
			GX_InitTexObj(this->TexObj, (void*)this->Img, this->Width, this->Height, this->ImgFmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
		}
		PNGU_ReleaseImageContext(ctx);
	}
}

void HBMImage::SetPosition(f32 x, f32 y) {
	this->X = x;
	this->Y = y;
}

void HBMImage::SetAnchorPoint(f32 x, f32 y) {
	this->AnchorPointX = x;
	this->AnchorPointY = y;
}

void* HBMImage::GetImage() {
	return this->Img;
}

u8 HBMImage::GetImageFormat() {
	return this->ImgFmt;
}

u16 HBMImage::GetX() {
	return this->X;
}

u16 HBMImage::GetY() {
	return this->Y;
}

u16 HBMImage::GetWidth() {
	return this->Width;
}

u16 HBMImage::GetHeight() {
	return this->Height;
}

void HBMImage::Free() {
	// it is possible to just do "if (this->Img) free(this->Img);"
	if (this->Img == NULL) return;

	// Make sure we free any saved image first
	if (this->dynamic) free(this->Img);

	// Free tex obj
	if (this->TexObj) free(this->TexObj);

	this->dynamic = false;
	this->Img = NULL;
	this->ImgSize = 0;
	this->Width = 0;
	this->Height = 0;
}

HBMImage::HBMImage() {
	this->Img = NULL;
	this->ImgFmt = GX_TF_RGBA8;
	this->ImgSize = 0;
	this->TexObj = NULL;

	this->Width = 0;
	this->Height = 0;
	this->X = 0;
	this->Y = 0;
	this->AnchorPointX = 0;
	this->AnchorPointY = 0;
	this->dynamic = false;

	this->R = 255;
	this->G = 255;
	this->B = 255;
	this->A = 255;
	this->Rotation = 0;
	this->ScaleX = 1;
	this->ScaleY = 1;
	this->FixedSize = false;
	this->Visible = true;
}

HBMImage::~HBMImage() {
	this->Free();
	this->Width = 0;
	this->Height = 0;
	this->X = 0;
	this->Y = 0;
	this->AnchorPointX = 0;
	this->AnchorPointY = 0;
}