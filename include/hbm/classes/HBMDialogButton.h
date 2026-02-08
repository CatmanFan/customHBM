#ifndef __HBM__HBMDialogButton__
#define __HBM__HBMDialogButton__

class HBMDialogButton : public HBMButton {
	public:
		HBMDialogButton();
		void Draw() final;
		void Update() final;

		bool CloseDialog;
		bool AltAppearance;
		int Sound;
};

#endif