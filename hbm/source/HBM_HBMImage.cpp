#include "HBM.h"
#include "HBM_Extern.h"

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

	if (!this->Visible || this->Img == NULL) return;

	float x, y, w, h, scaleX, scaleY;
	w = this->Width;
	h = this->Height;
	x = (xPos + w/2) * HBM_Settings.ScaleX;
	y = (yPos + h/2) * HBM_Settings.ScaleX;
	scaleX = this->Scale * HBM_Settings.ScaleX;
	scaleY = this->Scale * HBM_Settings.ScaleY;

	GXTexObj _texObj;
	Mtx m, m1, m2, mv;
	GX_InitTexObj(&_texObj, (void*)this->Img, this->Width, this->Height, this->ImgFmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
	// GX_InitTexObjLOD(&_texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
	GX_LoadTexObj(&_texObj,GX_TEXMAP0);
	GX_InvalidateTexAll();

	#ifdef HBM_USE_3D_RENDER
		GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
		GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
	#endif

	guMtxIdentity(m1);
	guMtxScaleApply(m1, m1, scaleX, scaleY, 1.0);
	guMtxRotDeg(m2, 'z', this->Rotation);
	// guVector axis = (guVector){0, 0, 1};
	// guMtxRotAxisDeg(m2, &axis, this->Rotation);
	guMtxConcat(m2, m1, m);

	guMtxTransApply(m, m, x, y, 0);
	guMtxConcat(HBM_GXmodelView2D, m, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	#ifdef HBM_USE_3D_RENDER
		GX_Position3f32(-w/2 + this->AnchorPointX, -h/2 + this->AnchorPointY, 0);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(0, 0);

		GX_Position3f32(w/2 + this->AnchorPointX, -h/2 + this->AnchorPointY, 0);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(1, 0);

		GX_Position3f32(w/2 + this->AnchorPointX, h/2 + this->AnchorPointY, 0);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(1, 1);

		GX_Position3f32(-w/2 + this->AnchorPointX, h/2 + this->AnchorPointY, 0);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(0, 1);
	#else
		GX_Position2f32(-w/2 + this->AnchorPointX, -h/2 + this->AnchorPointY);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(0, 0);

		GX_Position2f32(w/2 + this->AnchorPointX, -h/2 + this->AnchorPointY);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(1, 0);

		GX_Position2f32(w/2 + this->AnchorPointX, h/2 + this->AnchorPointY);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(1, 1);

		GX_Position2f32(-w/2 + this->AnchorPointX, h/2 + this->AnchorPointY);
		GX_Color4u8(this->R, this->G, this->B, this->A);
		GX_TexCoord2f32(0, 1);
	#endif
	GX_End();
	GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);

	#ifdef HBM_USE_3D_RENDER
		GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
		GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
	#endif
}

void HBMImage::Draw()
{
	this->Draw(this->X, this->Y);
}

void HBMImage::LoadRaw(void *img, u16 width, u16 height) {
	// Failsafe
	if (img == NULL) return;
	this->Free();

	this->dynamic = false;
	this->Img = img;
	this->ImgFmt = GX_TF_RGBA8;
	this->ImgSize = sizeof(img);
	this->Width = width;
	this->Height = height;
}

void HBMImage::LoadPNG(const void *img, int width, int height) {
	// Failsafe
	if (img == NULL) return;
	this->Free();

	// Load textures using PNGU
	PNGUPROP imgProp;
	IMGCTX ctx = PNGU_SelectImageFromBuffer(img);
	if (PNGU_GetImageProperties(ctx, &imgProp) == PNGU_OK) {
		this->ImgSize = imgProp.imgWidth * imgProp.imgHeight * 4;
		this->Img = PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, &width, &height);
		if (this->Img != NULL) {
			this->dynamic = true;
			this->Width = width;
			this->Height = height;
			this->ImgSize = sizeof(this->Img);
			this->ImgFmt = GX_TF_RGBA8;
			DCFlushRange(this->Img, this->ImgSize);
		}
		PNGU_ReleaseImageContext(ctx);
	}
}

#include <cstring>
static unsigned char *savebuffer[1024 * 1024 * 2];

void HBMImage::LoadEFB(u16 width, u16 height) {
	// Failsafe
	this->Free();

	// Load textures using PNGU
	IMGCTX ctx = PNGU_SelectImageFromBuffer(savebuffer);
	if (ctx != NULL) {
		this->ImgSize = PNGU_EncodeFromEFB(ctx, width, height, (1024 * 2) - width);
		PNGU_ReleaseImageContext(ctx);
		this->Img = (u8 *)malloc(this->ImgSize);
		memcpy(this->Img, savebuffer, this->ImgSize);
		this->dynamic = true;
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
	if (this->Img == NULL) return;

	// Make sure we free any saved image first
	if (this->dynamic) {
		free(this->Img);
	}

	this->dynamic = false;
	this->Img = NULL;
	this->ImgSize = 0;
}

HBMImage::HBMImage() {
	this->Img = NULL;
	this->ImgFmt = GX_TF_RGBA8;
	this->ImgSize = 0;
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
	this->Scale = 1;
	this->NoWidescreen = false;
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