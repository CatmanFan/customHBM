#ifndef __HBM__Extern__
#define __HBM__Extern__

extern struct HBM_CONFIG HBM_Settings;
extern Mtx HBM_GXmodelView2D;
extern void HBM_DrawBlackQuad(int x, int y, int width, int height, float percentage);
extern void HBM_PlaySound(const void* pcm, size_t pcm_size);

#define HBM_BUTTON_TIME_CLEAR if (this->TimeSnapshot > 0) { this->TimeSnapshot = 0; }
#define HBM_BUTTON_TIME_RESET this->TimeSnapshot = this->GetTime();
#define HBM_BUTTON_TIME_WAITING(x) (this->TimeSnapshot > 0 && (this->GetTime() - this->TimeSnapshot) <= x)
#define HBM_BUTTON_TIME_PROGRESS(x) ((this->GetTime() - this->TimeSnapshot) / x)
#define HBM_BUTTON_TIME_PROGRESS_PARTIAL(x, y) ((this->GetTime() - this->TimeSnapshot - x) / (y - x))

#endif