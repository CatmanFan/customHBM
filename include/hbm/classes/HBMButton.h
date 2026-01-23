#ifndef __HBM__HBMButton__
#define __HBM__HBMButton__

// Note that commented members have been moved to the global HBMElement class.
class HBMButton : public HBMElement {
	private:
		void DrawText();

	protected:
		// u8 Status;
		f32 AnimationProgress;

		u8 TextWidth;
		u32 TextColor;
		float TextSize;
		bool TextOverMask;

		class HBMImage Mask;
		float MaskOpacity;

	public:
		HBMButton();
		~HBMButton();

		void (*Selected)();
		const char* Text;
		// bool Blocked;

		void Draw() override;
		void Update() override;
		u8 GetStatus();
};

#endif