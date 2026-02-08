#ifndef __HBM__HBMButton__
#define __HBM__HBMButton__

// Note that commented members have been moved to the global HBMElement class.
class HBMButton : public HBMElement {
	protected:
		// u8 Status;
		u8 Substatus;
		f32 AnimationProgress;
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
};

#endif