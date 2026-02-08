#include "hbm/hbm.h"
#include "hbm/extern.h"
#include <cstring>
#include <stdarg.h>

#ifdef HBM_VERBOSE

	#define HBM_CONSOLE_MSG_COUNT 5
	#define HBM_CONSOLE_MSG_SIZE 72
	#define HBM_CONSOLE_MSG_FONTSIZE 15
	#define HBM_CONSOLE_MSG_BORDER 3
	#define HBM_CONSOLE_MSG_XOFFSET 10
	#define HBM_CONSOLE_MSG_YOFFSET 20
	#define HBM_CONSOLE_MSG_HEIGHT (HBM_CONSOLE_MSG_BORDER + HBM_CONSOLE_MSG_BORDER + HBM_CONSOLE_MSG_FONTSIZE)
	#define HBM_CONSOLE_MSG_TIME 3.0F

	typedef struct {
		char *text;
		size_t text_size;
		f64 opened;
	} HBM_ConsoleMsg;

	static HBM_ConsoleMsg msg[HBM_CONSOLE_MSG_COUNT];
	/* Second console */
	static HBM_ConsoleMsg msg2;

	static void HBM_ConsoleFreeMsg(HBM_ConsoleMsg *msg) {
		if (msg->text == NULL) return;

		free(msg->text);
		msg->text = NULL;
		msg->text_size = 0;
		msg->opened = 0;
	}

	// #define HBM_CONSOLE_USE_IPLROMFONT

	#ifdef HBM_CONSOLE_USE_IPLROMFONT
	struct {
		sys_fontheader* header;
		GXTexObj texture;
	} IPL_Font;

	static void HBM_IPLFontInit()
	{
		if (SYS_GetFontEncoding() == 0) {
			IPL_Font.header = (sys_fontheader *)memalign(32, SYS_FONTSIZE_ANSI);
		} else {
			IPL_Font.header = (sys_fontheader *)memalign(32, SYS_FONTSIZE_SJIS);
		}

		SYS_InitFont(IPL_Font.header);
		// IPL_Font.header->sheet_image = (IPL_Font.header->sheet_image + 31) & ~31;

		u32 texture_size;
		void *texels;

		texels = IPL_Font.header + IPL_Font.header->sheet_image;
		GX_InitTexObj(&IPL_Font.texture, texels,
					 IPL_Font.header->sheet_width,
					 IPL_Font.header->sheet_height,
					 IPL_Font.header->sheet_format,
					 GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_InitTexObjLOD(&IPL_Font.texture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_TRUE, GX_TRUE,
						 GX_ANISO_1);

		texture_size = GX_GetTexBufferSize(IPL_Font.header->sheet_width, IPL_Font.header->sheet_height,
										   IPL_Font.header->sheet_format, GX_FALSE, 0);
		DCStoreRange(texels, texture_size);
		GX_InvalidateTexAll();
	}

	static int HBM_IPLFontProcess(const char *text, bool should_draw, int text_x = 0, int text_y = 0)
	{
		void *image;
		int cellX, cellY, cellW, penX = 0, penY = 0;

		if (should_draw) {
			GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
			GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
			GX_SetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 1, 1);
			GX_LoadTexObj(&IPL_Font.texture, GX_TEXMAP0);
		}

		for (size_t i = 0; text[i] != '\0'; i++) {

			char c = text[i] - 47;
			switch(c) {
				case '\r':
					penX = 0;
					break;

				case '\n':
					penY += IPL_Font.header->cell_height;
					break;

				case ' ':
					//do not draw, because there's some crap on it
					SYS_GetFontTexture(c, &image, &cellX, &cellY, &cellW);
					penX += cellW;
					break;

				default: {
					if (c < IPL_Font.header->first_char) continue;

					SYS_GetFontTexture(c, &image, &cellX, &cellY, &cellW);
					if (should_draw) {
						int16_t penX2 = penX + IPL_Font.header->cell_width;
						int16_t penY2 = penY + IPL_Font.header->cell_height;
						int16_t cellX2 = cellX + IPL_Font.header->cell_width;
						int16_t cellY2 = cellY + IPL_Font.header->cell_height;

						Mtx m, mv;
						guMtxIdentity(m);
						guMtxTransApply(m, m, text_x, text_y, 0);
						guMtxConcat(HBM_GXmodelView2D, m, mv);
						GX_LoadPosMtxImm(mv, GX_PNMTX0);

						GX_Begin(GX_QUADS, HBM_GX_VTXFMT, 4);
							GX_Position2f32(penX, penY);
							GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
							GX_TexCoord2f32(cellX, cellY);

							GX_Position2f32(penX2, penY);
							GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
							GX_TexCoord2f32(cellX2, cellY);

							GX_Position2f32(penX2, penY2);
							GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
							GX_TexCoord2f32(cellX2, cellY2);

							GX_Position2f32(penX, penY2);
							GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
							GX_TexCoord2f32(cellX, cellY2);
						GX_End();
						GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);
					}
					penX += cellW;
				}
			}
		}

		if (should_draw) {
			GX_SetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
			GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
			GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
		}

		return penX;
	}
	#endif /* HBM_CONSOLE_USE_IPLROMFONT */

	static void HBM_ConsoleProcessString(bool use_msg2, int i)
	{
		if (use_msg2) {
			HBM_DrawQuad
			(
				/* x */ HBM_CONSOLE_MSG_XOFFSET,
				/* y */ HBM_HEIGHT - HBM_CONSOLE_MSG_YOFFSET - HBM_CONSOLE_MSG_HEIGHT,
			#ifdef HBM_CONSOLE_USE_IPLROMFONT
				/* w */ HBM_IPLFontProcess(msg2.text, false) + HBM_CONSOLE_MSG_XOFFSET,
			#else
				/* w */ HBM_MeasureText(msg2.text, HBM_CONSOLE_MSG_FONTSIZE, false, false) + HBM_CONSOLE_MSG_XOFFSET,
			#endif
				/* h */ HBM_CONSOLE_MSG_HEIGHT,
				/* c */ 25,
				/* a */ 0.5F,
			#ifdef HBM_CONSOLE_USE_IPLROMFONT
				/* noWidescreen */ true
			#else
				/* noWidescreen */ false
			#endif
			);
		} else {
			HBM_DrawQuad
			(
				/* x */ HBM_CONSOLE_MSG_XOFFSET,
				/* y */ HBM_CONSOLE_MSG_YOFFSET + (HBM_CONSOLE_MSG_HEIGHT * i),
			#ifdef HBM_CONSOLE_USE_IPLROMFONT
				/* w */ HBM_IPLFontProcess(msg[i].text, false) + HBM_CONSOLE_MSG_XOFFSET,
			#else
				/* w */ HBM_MeasureText(msg[i].text, HBM_CONSOLE_MSG_FONTSIZE, false, false) + HBM_CONSOLE_MSG_XOFFSET,
			#endif
				/* h */ HBM_CONSOLE_MSG_HEIGHT,
				/* c */ 25,
				/* a */ 0.5F,
			#ifdef HBM_CONSOLE_USE_IPLROMFONT
				/* noWidescreen */ true
			#else
				/* noWidescreen */ false
			#endif
			);
		}

		if (use_msg2) {
			#ifdef HBM_CONSOLE_USE_IPLROMFONT
				HBM_IPLFontProcess
				(
					msg2.text,
					true,
					/* x */ HBM_CONSOLE_MSG_XOFFSET + HBM_CONSOLE_MSG_BORDER,
					/* y */ HBM_HEIGHT - HBM_CONSOLE_MSG_YOFFSET + HBM_CONSOLE_MSG_BORDER - HBM_CONSOLE_MSG_HEIGHT
				);
			#else
				HBM_DrawText
				(
					msg2.text,
					/* x */ HBM_CONSOLE_MSG_XOFFSET + HBM_CONSOLE_MSG_BORDER,
					/* y */ HBM_HEIGHT - HBM_CONSOLE_MSG_YOFFSET + HBM_CONSOLE_MSG_BORDER - HBM_CONSOLE_MSG_HEIGHT,
					/* size */ HBM_CONSOLE_MSG_FONTSIZE,
					/* scaleX */ 1.0,
					/* scaleY */ 1.0,
					/* align */ HBM_TEXT_LEFT, HBM_TEXT_TOP,
					false,
					255, 255, 255, 255
				);
			#endif
		} else {
			#ifdef HBM_CONSOLE_USE_IPLROMFONT
				HBM_IPLFontProcess
				(
					msg[i].text,
					true,
					/* x */ HBM_CONSOLE_MSG_XOFFSET + HBM_CONSOLE_MSG_BORDER,
					/* y */ HBM_CONSOLE_MSG_YOFFSET + HBM_CONSOLE_MSG_BORDER + (HBM_CONSOLE_MSG_HEIGHT * i)
				);
			#else
				HBM_DrawText
				(
					msg[i].text,
					/* x */ HBM_CONSOLE_MSG_XOFFSET + HBM_CONSOLE_MSG_BORDER,
					/* y */ HBM_CONSOLE_MSG_YOFFSET + HBM_CONSOLE_MSG_BORDER + (HBM_CONSOLE_MSG_HEIGHT * i),
					/* size */ HBM_CONSOLE_MSG_FONTSIZE,
					/* scaleX */ 1.0,
					/* scaleY */ 1.0,
					/* align */ HBM_TEXT_LEFT, HBM_TEXT_TOP,
					false,
					255, 255, 255, 255
				);
			#endif
		}
	}

#endif /* HBM_VERBOSE */

void HBM_ConsoleInit()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
	{
		msg[i].text = NULL;
		msg[i].text_size = 0;
		msg[i].opened = 0;
	}

	#ifdef HBM_CONSOLE_USE_IPLROMFONT
		HBM_IPLFontInit();
	#endif

#endif /* HBM_VERBOSE */
}

void HBM_ConsoleClear()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
		HBM_ConsoleFreeMsg(&msg[i]);

	HBM_ConsoleFreeMsg(&msg2);

#endif /* HBM_VERBOSE */
}

void HBM_ConsolePrintf(const char* string, ...)
{
#ifdef HBM_VERBOSE

	// Look for empty slot
	loop:
	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
	{
		if (msg[i].text == NULL)
		{
			msg[i].text_size = sizeof(char) * HBM_CONSOLE_MSG_SIZE;
			msg[i].text = (char *)malloc(msg[i].text_size);
			memset(msg[i].text, 0, msg[i].text_size);

			va_list argp;
			va_start(argp, string);
			vsnprintf(msg[i].text, msg[i].text_size, string, argp);
			va_end(argp);

			msg[i].opened = (f64)HBM_GETTIME / 1000.0F;
			return;
		}
	}

	HBM_ConsoleFreeMsg(&msg[HBM_CONSOLE_MSG_COUNT - 1]);
	goto loop;

#endif /* HBM_VERBOSE */
}

/* Second console */
void HBM_ConsolePrintf2(const char* string, ...)
{
#ifdef HBM_VERBOSE

	HBM_ConsoleFreeMsg(&msg2);

	msg2.text_size = sizeof(char) * HBM_CONSOLE_MSG_SIZE;
	msg2.text = (char *)malloc(msg2.text_size);

	va_list argp;
	va_start(argp, string);
	vsnprintf(msg2.text, msg2.text_size, string, argp);
	va_end(argp);

	msg2.opened = (f64)HBM_GETTIME / 1000.0F;

#endif /* HBM_VERBOSE */
}

void HBM_ConsoleDraw()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++) {
		if (msg[i].text != NULL) {
			HBM_ConsoleProcessString(false, i);
		}
	}

	/* Second console */
	if (msg2.text != NULL) {
		HBM_ConsoleProcessString(true, 0);
	}

#endif /* HBM_VERBOSE */
}

void HBM_ConsoleUpdate()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++) {
		if (msg[i].text != NULL && (f64)HBM_GETTIME / 1000.0F >= msg[i].opened + HBM_CONSOLE_MSG_TIME) {
			HBM_ConsoleFreeMsg(&msg[i]);
		}
	}

	/* Second console */
	if (msg2.text != NULL && (f64)HBM_GETTIME / 1000.0F >= msg2.opened + HBM_CONSOLE_MSG_TIME) {
		HBM_ConsoleFreeMsg(&msg2);
	}

#endif /* HBM_VERBOSE */
}

#ifdef HBM_VERBOSE

	#undef HBM_CONSOLE_MSG_COUNT
	#undef HBM_CONSOLE_MSG_SIZE
	#undef HBM_CONSOLE_MSG_FONTSIZE
	#undef HBM_CONSOLE_MSG_BORDER
	#undef HBM_CONSOLE_MSG_XOFFSET
	#undef HBM_CONSOLE_MSG_YOFFSET
	#undef HBM_CONSOLE_MSG_HEIGHT
	#undef HBM_CONSOLE_MSG_TIME

#endif /* HBM_VERBOSE */