#ifndef __HBM__HBMImage__
#define __HBM__HBMImage__

/**
 * This will draw an image using GX.
 **/
class HBMImage {
	private:
		void InitTexObj();

	protected:
		void *Img;
		u8 ImgFmt;
		int ImgSize;
		GXTexObj *TexObj;

		u16 Width;
		u16 Height;
		f32 X;
		f32 Y;
		f32 AnchorPointX;
		f32 AnchorPointY;
		bool dynamic;

	public:
		u8 R;
		u8 G;
		u8 B;
		u8 A;
		float Rotation;
		float ScaleX;
		float ScaleY;
		bool Visible;
		bool FixedSize;

		HBMImage();
		~HBMImage();

		void Draw(f32 xPos, f32 yPos);
		void Draw();
		void LoadRaw(const void *img, u16 width, u16 height, u8 fmt = GX_TF_RGBA8);
		void LoadPNG(const void *img, int width, int height);
		void Free();
		void SetPosition(f32 x, f32 y);
		void SetAnchorPoint(f32 x, f32 y);

		void* GetImage();
		u8 GetImageFormat();
		u16 GetX();
		u16 GetY();
		u16 GetWidth();
		u16 GetHeight();
};

#endif