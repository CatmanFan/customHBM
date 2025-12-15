#ifndef __HBM__HBMElement__
#define __HBM__HBMElement__

class HBMElement {
	protected:
		class HBMImage Shadow;
		float ShadowOpacity;
		int ShadowX;
		int ShadowY;
		float Scale = 1.0F;

		//! Hitbox is the area designated to be controlled by the IR pointer.
		struct {
			int X;
			int Y;
			int Width;
			int Height;
		} Hitbox;

	public:
		HBMElement()
		{
			// Load images
			// this->Image.LoadPNG(&..., 64, 64);
			// this->SetHitbox(0, 0, 64, 64);

			// Load shadow
			// this->Shadow.LoadRaw(this->Image.GetImage(), this->Image.GetWidth(), this->Image.GetHeight());
			// this->Shadow.R = this->Shadow.G = this->Shadow.B = 0;
			// this->ShadowOpacity = 0.58;
			// this->ShadowX = this->ShadowY = 5;
		}
		~HBMElement()
		{
			// Unload from memory
		}

		class HBMImage Image;
		bool Visible;
		u16 X;
		u16 Y;

		virtual void Draw();
		virtual void Update();
		void SetOpacity(float value);
		void SetHitbox(int w, int h);
		void SetHitbox(int x, int y, int w, int h);
		void SetPosition(u16 x, u16 y);
		u16 GetX();
		u16 GetY();

		bool HitboxTouched();
		bool HitboxTouched(int chan);
};

#endif