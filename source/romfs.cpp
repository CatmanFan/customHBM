#include "hbm.h"

#ifdef HBM_ENABLE_ROMFS

#include <romfs-ogc.h>

HBMRomfsFile::HBMRomfsFile() {
	this->size = 0;
	this->buffer = NULL;
	this->path = NULL;
}

HBMRomfsFile::HBMRomfsFile(const char* path) {
	this->size = 0;
	this->buffer = NULL;
	this->path = NULL;
	this->Load(path);
}

HBMRomfsFile::~HBMRomfsFile() {
	this->Free();
}

bool HBMRomfsFile::Load(const char* path) {
	if (path == 0 || path == NULL) {
		return false;
	}

	if (this->buffer) {
		this->Free();
	}

	FILE *file = fopen(path, "rb");

	if (file) {
		fseeko(file, 0, SEEK_END);
		size_t fileSize = ftello(file);

		if (fileSize > 0) {
			this->size = fileSize;
			this->path = path;
			this->buffer = (u8 *)malloc(this->size);
			fseeko(file, 0, SEEK_SET);
			fread(this->buffer, 1, this->size, file);
			fclose(file);

			// HBM_ConsolePrintf("Loaded file: %s", path);
			return true;
		} else {
			fclose(file);
			HBM_ConsolePrintf("Failed to get filesize: %s", path);
			return false;
		}
	} else {
		HBM_ConsolePrintf("Failed to open file: %s", path);
		return false;
	}
}

void HBMRomfsFile::Free() {
	if (this->buffer) {
		// HBM_ConsolePrintf("Freeing file: %s", this->path);
		free(this->buffer);
		this->size = 0;
		this->buffer = NULL;
		this->path = NULL;
	}
}

u8 * HBMRomfsFile::Data() {
	return this->buffer;
}

size_t HBMRomfsFile::Size() {
	return this->size;

}

#endif

void HBM_RomfsInit() {
#ifdef HBM_ENABLE_ROMFS
	romfsInit();
#endif
}

void HBM_RomfsUninit() {
#ifdef HBM_ENABLE_ROMFS
	romfsExit();
#endif
}