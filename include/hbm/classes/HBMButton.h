#ifndef __HBM__HBMButton__
#define __HBM__HBMButton__

class HBMButton : public HBMElement {
	protected:
		//! The status of the button based on the IR pointer's interaction with the hitbox
		u8 Status;
		u8 Substatus;
		f64 GetTime();
		f64 TimeSnapshot;

		u32 TextColor;
		float TextSize;
		bool TextOverMask;

		class HBMImage Mask;
		float MaskOpacity;
		void DrawText();

	public:
		HBMButton();
		~HBMButton();

		void (*Selected)();
		char *Text;
		bool Blocked;

		void Draw() override;
		void Update() override;
		u8 GetStatus();
};

#endif