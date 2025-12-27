#ifndef __HBM__HBMDialogButton__
#define __HBM__HBMDialogButton__

class HBMDialogButton : public HBMButton {
	public:
		HBMDialogButton();
		void Update() final;
		bool Cancel;
		bool Cancelling;
};

#endif