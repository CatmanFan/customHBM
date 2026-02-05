#ifndef __HBM__HBMDialog__
#define __HBM__HBMDialog__

class HBMDialog : public HBMElement {
	protected:
		class HBMDialogButton Button1;
		class HBMDialogButton Button2;
		int Status;

	public:
		HBMDialog();
		~HBMDialog();

		void (*Confirm)();
		const char* Text;
		bool SlideFromTop;
		bool AltAppearance;

		void Draw() override;
		void Update() override;
		void Show();
		void UpdateText(const char* Text);
		void UpdateText(const char* Text, const char* Button1);
		void UpdateText(const char* Text, const char* Button1, const char* Button2);
		void Hide();
		void Reset();
};

#endif