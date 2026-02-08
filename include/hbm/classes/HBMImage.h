#ifndef __HBM__HBMImage__
#define __HBM__HBMImage__

/**
 * This will draw an image using GX.
 **/
class HBMImage {
	protected:
		GXTexObj *TexObj;
		bool Dynamic;

	public:
		void *Img;
		u8 ImgFmt;
		f32 X;
		f32 Y;
		u16 Width;
		u16 Height;

		struct { u8 R; u8 G; u8 B; u8 A; } Color;
		struct { f32 X; f32 Y; } AnchorPoint;

		f32 Rotation;
		f32 ScaleX;
		f32 ScaleY;
		bool Visible;
		bool FixedSize;

		void Draw(f32 xPos, f32 yPos);
		void Draw();
		void LoadRaw(const void *img, u16 width, u16 height, u8 fmt = GX_TF_RGBA8);
		void LoadPNG(const void *img, int width, int height);
		void Free();

		HBMImage();
		~HBMImage();
};

#endif