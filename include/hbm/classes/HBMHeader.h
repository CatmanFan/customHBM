#ifndef __HBM__HBMHeader__
#define __HBM__HBMHeader__

class HBMHeader : public HBMElement {
	protected:
		class HBMImage Highlighted;
		float HighlightedOpacity;
		float MaskOpacity;

		// Used for the bottom header only
		float WiiRemoteSlider;

	public:
		HBMHeader();
		~HBMHeader();

		void (*Selected)();
		void (*AfterSelected)();
		bool Inverted;

		// Used for the bottom header only
		HBMImage *WiiRemote;

		const char* Text;
		float TextX;
		float TextY;
		int TextRectWidth;
		int TextRectHeight;
		bool TextCentered;
		float TextSize;
		float TextOpacity;

		void Draw() override;
		void Update() override;

		// Calls the header button function immediately.
		void Call();
};

#endif