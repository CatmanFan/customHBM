#ifndef __HBM__ROMFS__
#define __HBM__ROMFS__

void HBM_RomfsInit();
void HBM_RomfsUninit();

class HBMRomfsFile {
	protected:
		const char *path;
		u8 *buffer;
		size_t size;

	public:
		HBMRomfsFile();
		HBMRomfsFile(const char *path);
		~HBMRomfsFile();

		u8 *Data();
		size_t Size();
		bool Load(const char *path);
		void Free();
};

#endif