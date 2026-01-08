#ifndef __HBM__HBMCursor__
#define __HBM__HBMCursor__

class HBMPointerImage : public HBMElement {
	protected:
		bool first;
		bool loaded;

	public:
		HBMPointerImage();
		~HBMPointerImage();
		void Load(int num, HBMPointerImage *first);
		void Update() override;
};

#endif