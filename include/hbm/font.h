#ifndef __HBM__Font__
#define __HBM__Font__

bool HBM_FontInit(int type);
void HBM_FontUninit();
int HBM_FontType();
void HBM_DrawText(const char* string,
					float x,
					float y,
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
					int max_width = 0,
					int max_height = 0);
int HBM_MeasureText(const char* string, float size, bool use_serif, bool return_height);

#endif