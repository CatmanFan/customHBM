#ifndef __HBM__HBMRemoteDataSprite__
#define __HBM__HBMRemoteDataSprite__

class HBMRemoteDataSprite {
	private:
		HBMImage Battery;
		HBMImage BatteryDisconnected;
		u8 BatteryIndex;
		bool BatteryBlocked;
		f64 FlashTime;

	public:
		HBMRemoteDataSprite();
		~HBMRemoteDataSprite();
		const char* Text;
		bool Disconnected;

		void SetBatteryImage(u8 value);
		void Flash(bool disconnected);
		void Draw(int X, int Y);
};

#endif