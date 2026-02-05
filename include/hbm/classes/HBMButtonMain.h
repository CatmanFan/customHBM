#ifndef __HBM__HBMButtonMain__
#define __HBM__HBMButtonMain__

class HBMButtonMain : public HBMButton {
	private:
		void UpdateScale();

	protected:
		u8 Substatus;

	public:
		HBMButtonMain();
		void Draw() final;
		void Update() final;
};

#endif