#ifndef __HBM__Extern__
#define __HBM__Extern__

extern struct HBM_CONFIG HBM_Settings;
extern struct HBM_EXITTRANSITION HBM_ExitTransition;
extern Mtx HBM_GXmodelView2D;

extern void HBM_DrawQuad(int x, int y, int width, int height, u8 shade, float alpha, bool noWidescreen);
extern void HBM_PlaySound(const void* pcm, const void* pcm_end, bool lowRate);

#define HBM_SOUND(FILE, LOWRATE) HBM_PlaySound(FILE, FILE##_end, LOWRATE);

#endif