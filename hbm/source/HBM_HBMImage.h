#ifndef __HBM__HBMImage__
#define __HBM__HBMImage__

/*
 * This will draw an image using GX.
 */
class HBMImage {
	protected:
		void *Img;
		u8 ImgFmt;
		int ImgSize;
		u16 Width;
		u16 Height;
		f32 X;
		f32 Y;
		f32 AnchorPointX;
		f32 AnchorPointY;
		bool dynamic;

	public:
		u16 R;
		u16 G;
		u16 B;
		u16 A;
		float Rotation;
		float Scale;
		bool Visible;
		bool NoWidescreen;

		HBMImage();
		~HBMImage();

		void Draw(f32 xPos, f32 yPos);
		void Draw();
		void LoadRaw(void *img, u16 width, u16 height);
		void LoadPNG(const void *img, int width, int height);
		void LoadEFB(u16 width, u16 height);
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