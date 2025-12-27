#ifndef __HBM__Font__
#define __HBM__Font__

void HBM_FontInit();
void HBM_FontReload(int type);
void HBM_FontUninit();
void HBM_DrawText(const char *string,
					int x,
					int y,
					float size,
					float scaleX,
					float scaleY,
					enum HBM_TEXTALIGNH align_h,
					enum HBM_TEXTALIGNV align_v,
					bool use_serif,
					const u8 cR,
					const u8 cG,
					const u8 cB,
					const u8 cA,
					int max_width = -1);
int HBM_MeasureText(const char *string, float size, bool use_serif, bool return_height);

#endif