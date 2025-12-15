#include "HBM.h"
#include <romfs-ogc.h>

void HBM_RomfsInit() {
	romfsInit();
}

void HBM_RomfsUninit() {
	romfsExit();
}

HBMRomfsFile::HBMRomfsFile() {
	this->size = 0;
	this->buffer = NULL;
	this->path = NULL;
}

HBMRomfsFile::HBMRomfsFile(const char *path) {
	this->size = 0;
	this->buffer = NULL;
	this->path = NULL;
	this->Load(path);
}

HBMRomfsFile::~HBMRomfsFile() {
	this->Free();
}

void HBMRomfsFile::Load(const char *path) {
	if (path == 0 || path == NULL) {
		return;
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
		} else {
			fclose(file);
			// HBM_ConsolePrintf("Failed to get filesize: %s", path);
		}
	} else {
		// HBM_ConsolePrintf("Failed to open file: %s", path);
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