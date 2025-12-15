#ifndef __HBM__HBMButtonMain__
#define __HBM__HBMButtonMain__

class HBMButtonMain : public HBMElement {
	protected:
		//! The status of the button based on the IR pointer's interaction with the hitbox
		u8 Status;
		u8 Substatus;
		f64 GetTime();
		f64 TimeSnapshot;

		class HBMImage Mask;
		float MaskOpacity;

	public:
		HBMButtonMain();

		void (*Selected)();
		char *Text;

		void Draw() override;
		void Update() override;
		u8 GetStatus();
};

#endif