#include "hbm.h"
#include "hbm/extern.h"
#include <wchar.h>

/*------------------------------------------------------------------------------
Copyright (c) 2009-2025 The GRRLIB Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
------------------------------------------------------------------------------*/

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct HBM_ttfFont {
	FT_Face /* void * */ face;     /**< A TTF face object. */
	bool kerning;   /**< true whenever a face object contains kerning data that can be accessed with FT_Get_Kerning. */
	float lineHeight;
} HBM_Font;

static FT_Library ftLibrary;			/* handle to library     */
static HBM_Font *sansSerif, *serif;		/* handle to face object */

static int fontType = -1;

/******************************************************
 *                  GLYPH PROCESSING                  *
 ******************************************************/

uint8_t *HBM_Glyphs[HBM_MAXGLYPHS];

static uint8_t *HBM_glyphToTexture(FT_Bitmap *bmp) {
	int cw = bmp->width;
	int ch = bmp->rows;
	int tw = (cw+7)/8;
	int th = (ch+3)/4;

	int tpitch = tw * 32;

	int slot;
	for (slot = 0; slot < HBM_MAXGLYPHS; slot++)
		if (HBM_Glyphs[slot] == NULL) {
			HBM_Glyphs[slot] = (uint8_t *)memalign(32, tw*th*32);
			goto copy;
		}

	slot = HBM_MAXGLYPHS - 1;
	if (HBM_Glyphs[slot] != NULL) {
		free(HBM_Glyphs[slot]);
		HBM_Glyphs[slot] = NULL;
	}
	HBM_Glyphs[slot] = (uint8_t *)memalign(32, tw*th*32);

	copy:
	memset(HBM_Glyphs[slot], 0, tw*th*32);

	int x,y;
	uint8_t *p = bmp->buffer;
	for(y=0; y<ch; y++) {
		uint8_t *lp = p;
		int ty = y/4;
		int py = y%4;
		uint8_t *lpix = HBM_Glyphs[slot] + ty*tpitch + py*8;
		for(x=0; x<cw; x++) {
			int tx = x/8;
			int px = x%8;
			lpix[32*tx + px] = *lp++;
		}
		p += bmp->pitch;
	}

	DCFlushRange(HBM_Glyphs[slot], 8*tw * 4*th);
	return HBM_Glyphs[slot];
}

/******************************************************
 *                   TEXT PROCESSING                  *
 ******************************************************/

typedef struct HBM_textString {
	wchar_t *utf32;
	size_t length;
	int lines;
	enum HBM_TEXTALIGNH align_h;
	enum HBM_TEXTALIGNV align_v;
	float font_size;
	float scale_x;
	float scale_y;
} HBM_TextString;
 
static bool HBM_copyToTextString(HBM_TextString *myText, const char *string) {
	myText->length = strlen(string) + 1;
	myText->lines = 1;
	myText->utf32 = (wchar_t*)malloc(myText->length * sizeof(wchar_t));

	if (myText->utf32 != NULL) {
		myText->length = mbstowcs(myText->utf32, string, myText->length);
		if (myText->length > 0) {
			myText->utf32[myText->length] = L'\0';
		}

		for (size_t i = 0; i < myText->length; i++)
			if (myText->utf32[i] == L'\n')
				myText->lines++;

		myText->length++;
		return true;
	}

	myText->length = 0;
	return false;
}
 
static void HBM_freeTextString(HBM_TextString *myText) {
	free(myText->utf32);
	myText->length = 0;
	myText->lines = 0;
}

static int HBM_TextStringMeasure(HBM_TextString *myText, HBM_Font *myFont, bool return_height, int line = -1)
{
	FT_Face Face = (FT_Face)myFont->face;
	FT_GlyphSlot slot = Face->glyph;
	FT_UInt previousGlyph = 0;

	while (FT_Set_Char_Size(Face, 64 * myText->font_size, 64 * myText->font_size, 0, 0) != 0)
		myText->font_size = 12;

	bool searchSingleLine = line >= 0 && line < myText->lines;
	int lineIndex = 0;
	int textWidth = 0;
	int textWidth_max = 0;
	int textHeight = myText->font_size;

	for (size_t i = 0; i < myText->length; i++) {
		if (searchSingleLine) {
			if (myText->utf32[i] == L'\n' || i == myText->length - 1) {
				if (lineIndex == line || i == myText->length - 1)
					return return_height ? textHeight : textWidth;
				else {
					lineIndex++;
					textWidth = 0;
					// textHeight = myText->font_size;
					continue;
				}
			}
		} else {
			if (textWidth > textWidth_max)
				textWidth_max = textWidth;

			if (myText->utf32[i] == L'\n') {
				textWidth = 0;
				textHeight += lround(myFont->lineHeight * myText->font_size);
				continue;
			}
		}

		// Use FT_Load_Char?
		const FT_UInt glyphIndex = FT_Get_Char_Index(myFont->face, myText->utf32[i]);

		if (myFont->kerning && previousGlyph && glyphIndex) {
			FT_Vector delta;
			FT_Get_Kerning(myFont->face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
			textWidth += delta.x >> 6;
		}
		if (FT_Load_Glyph(myFont->face, glyphIndex, FT_LOAD_DEFAULT) != 0)
			continue;

		textWidth += slot->advance.x >> 6;
		previousGlyph = glyphIndex;
	}

	return return_height ? textHeight : textWidth_max;
}

static void HBM_TextStringDraw (HBM_TextString *myText,
								HBM_Font *myFont,
								int max_width,
								int x,
								int y,
								const u8 cR,
								const u8 cG,
								const u8 cB,
								const u8 cA)
{
	if (myFont == NULL || myText == NULL || myText->utf32 == NULL)
		return;

	FT_Face Face = (FT_Face)myFont->face;
	FT_GlyphSlot slot = Face->glyph;
	FT_UInt previousGlyph = 0;
	int penX = 0;
	int penY = myText->font_size;

	int width = HBM_TextStringMeasure(myText, myFont, false);
	int height = HBM_TextStringMeasure(myText, myFont, true);
	if (max_width > 0 && width > max_width)
		myText->scale_x *= ((float)max_width / (float)width);

	// Compress font size depending on screen resolution
	while (FT_Set_Char_Size(Face, 64 * myText->font_size * 2, 64 * myText->font_size * 2, 0, 0) != 0)
		myText->font_size = 12;

	int curLine = 0;
	int offsetX = myText->align_h == HBM_TEXT_CENTER ? HBM_TextStringMeasure(myText, myFont, false, curLine) / -2 : 0;
	int offsetY = myText->align_v == HBM_TEXT_MIDDLE ? height / -2 : 0;

	Mtx m, mv; // Better to use matrices for position

	// Loop over each character, until the end of the string is reached, or until the pixel width is too wide
	for (size_t i = 0; i < wcslen(myText->utf32); i++) {
		if (myText->utf32[i] == L'\n') {
			penX = 0;
			penY += lround(myFont->lineHeight * myText->font_size);

			curLine++;
			offsetX = myText->align_h == HBM_TEXT_CENTER ? HBM_TextStringMeasure(myText, myFont, false, curLine) / -2 : 0;
			continue;
		}

		// Use FT_Load_Char?
		const FT_UInt glyphIndex = FT_Get_Char_Index(myFont->face, myText->utf32[i]);

		if (myFont->kerning && previousGlyph && glyphIndex) {
			FT_Vector delta;
			FT_Get_Kerning(myFont->face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
			penX += delta.x >> 6;
		}
		if (FT_Load_Glyph(myFont->face, glyphIndex, FT_LOAD_RENDER) != 0)
			continue;

		uint8_t *glyphTexture = HBM_glyphToTexture(&slot->bitmap);

		/************************************/
		guMtxIdentity(m);
		guMtxScaleApply(m, m, myText->scale_x * HBM_Settings.ScaleX, myText->scale_y * HBM_Settings.ScaleY, 1.0);
		guMtxTransApply(m, m, x * HBM_Settings.ScaleX, y * HBM_Settings.ScaleY, 0);
		guMtxConcat(HBM_GXmodelView2D, m, mv);
		GX_LoadPosMtxImm(mv, GX_PNMTX0);

		GXTexObj _texObj;
		GX_InitTexObj(&_texObj, (void*)glyphTexture, slot->bitmap.width, slot->bitmap.rows, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&_texObj, GX_TEXMAP0);

		#if (HBM_DRAW_METHOD == 1) /*** GRRLIB **/
			GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
			GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

			GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
				GX_Position3f32(penX + slot->bitmap_left + offsetX, penY - slot->bitmap_top + offsetY, 0);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(0, 0);
				GX_Position3f32(penX + slot->bitmap_left + slot->bitmap.width + offsetX, penY - slot->bitmap_top + offsetY, 0);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(1, 0);
				GX_Position3f32(penX + slot->bitmap_left + slot->bitmap.width + offsetX, penY - slot->bitmap_top + slot->bitmap.rows + offsetY, 0);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(1, 1);
				GX_Position3f32(penX + slot->bitmap_left + offsetX, penY - slot->bitmap_top + slot->bitmap.rows + offsetY, 0);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(0, 1);
			GX_End();
			GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);

			GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
			GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
		#endif

		#if (HBM_DRAW_METHOD == 0) /*** Libwiisprite **/
			GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
				GX_Position2f32(penX + slot->bitmap_left + offsetX, penY - slot->bitmap_top + offsetY);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(0, 0);
				GX_Position2f32(penX + slot->bitmap_left + slot->bitmap.width + offsetX, penY - slot->bitmap_top + offsetY);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(1, 0);
				GX_Position2f32(penX + slot->bitmap_left + slot->bitmap.width + offsetX, penY - slot->bitmap_top + slot->bitmap.rows + offsetY);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(1, 1);
				GX_Position2f32(penX + slot->bitmap_left + offsetX, penY - slot->bitmap_top + slot->bitmap.rows + offsetY);
				GX_Color4u8(cR, cG, cB, cA);
				GX_TexCoord2f32(0, 1);
			GX_End();
			GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);
		#endif

		/*
		GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
		GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

		GX_Begin(GX_POINTS, GX_VTXFMT0, slot->bitmap.width * slot->bitmap.rows);
		for (FT_Int i = 0; i < slot->bitmap.width; i++)
		{
			for (FT_Int j = 0; j < slot->bitmap.rows; j++)
			{
				s16 alpha = slot->bitmap.buffer[ j * slot->bitmap.width + i ] - (0xFF - cA);
				if (alpha < 0) alpha = 0;

				#if (HBM_DRAW_METHOD == 0)
					GX_Position2f32(penX + slot->bitmap_left + i + offsetX, penY - slot->bitmap_top + j + offsetY);
				#else
					GX_Position3f32(penX + slot->bitmap_left + i + offsetX, penY - slot->bitmap_top + j + offsetY, 0);
				#endif
				GX_Color4u8(cR, cG, cB, alpha);
			}
		}
		GX_End();

		GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);*/

		// free(glyphTexture);
		/************************************/

		penX += slot->advance.x >> 6;
		previousGlyph = glyphIndex;
	}
}

/******************************************************
 *               PUBLIC TEXT FUNCTIONS                *
 ******************************************************/

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
					int max_width)
{
	HBM_Font *myFont = use_serif ? serif : sansSerif;

	if (myFont == NULL || string == NULL)
		return;

	HBM_TextString text = {
		.align_h = align_h,
		.align_v = align_v,
		.font_size = size,
		.scale_x = scaleX,
		.scale_y = scaleY
	};

	if (HBM_copyToTextString(&text, string)) {
		HBM_TextStringDraw(
			&text,				// string
			myFont,				// myFont
			max_width,			// max_width
			x, 					// x
			y, 					// y
			cR, cG, cB, cA		// color
		);
		HBM_freeTextString(&text);
	}
}

int HBM_MeasureText(const char *string, float size, bool use_serif, bool return_height)
{
	HBM_Font *myFont = use_serif ? serif : sansSerif;
	if (myFont == NULL || string == NULL)
		return 0;

	int result = 0;
	HBM_TextString text = { .align_h = HBM_TEXT_LEFT, .align_v = HBM_TEXT_TOP, .font_size = size, .scale_x = 1, .scale_y = 1 };
	if (HBM_copyToTextString(&text, string)) {
		result = HBM_TextStringMeasure(&text, myFont, return_height);
		HBM_freeTextString(&text);
	}

	return result;
}

/******************************************************
 *                    FONT LOADING                    *
 ******************************************************/

static HBM_Font* HBM_FontLoadTTF(const u8* file_base, s32 file_size)
{
	FT_Face Face;
	if (FT_New_Memory_Face(ftLibrary, file_base, file_size, 0, &Face) != 0)
		return NULL;

	HBM_Font* myFont = (HBM_Font*)malloc(sizeof(HBM_Font));
	myFont->kerning = FT_HAS_KERNING(Face);

	myFont->face = Face;
	return myFont;
}

static void HBM_FontFreeTTF(HBM_Font *myFont)
{
	if (myFont != NULL) {
		FT_Done_Face(myFont->face);
		free(myFont);
	}
}

/******************************************************
 *               PUBLIC FONT FUNCTIONS                *
 ******************************************************/

void HBM_FontInit()
{
	// Init FreeType library
	if (FT_Init_FreeType(&ftLibrary) != 0)
		return;
}

static HBMRomfsFile ttf_sansSerif;
static HBMRomfsFile ttf_serif;

void HBM_FontReload(int type)
{
	if (type != fontType) {
		HBM_FontFreeTTF(sansSerif);
		HBM_FontFreeTTF(serif);

		// Load fonts
		// sansSerif = HBM_FontLoadTTF(nintendo_NTLG_DB_002_ttf, nintendo_NTLG_DB_002_ttf_size);
		// serif = HBM_FontLoadTTF(nintendo_NTLG_DB_002_ttf, nintendo_NTLG_DB_002_ttf_size);
		// serif = HBM_FontLoadTTF(UtrilloProGrecoStd_ttf, UtrilloProGrecoStd_ttf_size);

		// Load fonts
		ttf_sansSerif.Load("romfs:/hbm/ttf/nintendo_NTLG-DB_002.ttf");
		ttf_serif.Load("romfs:/hbm/ttf/UtrilloProGrecoStd.ttf");
		sansSerif = HBM_FontLoadTTF(ttf_sansSerif.Data(), ttf_sansSerif.Size());
		serif = HBM_FontLoadTTF(ttf_serif.Data(), ttf_serif.Size());
		sansSerif->lineHeight = 0.95;
		serif->lineHeight = 0.95;
		fontType = type;
	}
}

void HBM_FontUninit()
{
	HBM_FontFreeTTF(sansSerif);
	HBM_FontFreeTTF(serif);
}