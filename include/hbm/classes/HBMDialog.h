#ifndef __HBM__HBMDialog__
#define __HBM__HBMDialog__

class HBMDialog : public HBMElement {
	protected:
		class HBMDialogButton Button1;
		class HBMDialogButton Button2;
		int Status;
		bool Active;
		f64 TransitionStart;
		f64 Transition;

	public:
		HBMDialog();
		~HBMDialog();

		void (*Confirm)();
		char *Text;

		void Draw() override;
		void Update() override;
		void Show();
		void UpdateText(char *Text, char *Button1, char *Button2);
		void Hide();
};

#endif