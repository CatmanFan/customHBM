#include "hbm.h"
#include "hbm/extern.h"
#include <wchar.h>
#include <iostream>
#include <map>
#include <iterator>

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

// #define HBM_USE_MAP_FOR_GLYPHS

typedef struct HBM_ttfGlyph {
#ifndef HBM_USE_MAP_FOR_GLYPHS
	wchar_t myChar;
#endif /* HBM_USE_MAP_FOR_GLYPHS */
	int width;
	int rows;
	int left;
	int top;
	int advance;
	uint8_t *image;
	GXTexObj *texObj;
} HBM_Glyph;

typedef struct HBM_ttfFont {
	FT_Face /* void * */ face;     /**< A TTF face object. */
	bool kerning;   /**< true whenever a face object contains kerning data that can be accessed with FT_Get_Kerning. */
	bool serifGlyph;
	bool cached;
	float defaultSize;

	short leading;
	float spacing;

	short customSpaceWidth;
	short extraSpaceWidth;
	short shortSpaceWidth;
	short letterSpaceWidth;

	float xOffset;
	float yOffset;
} HBM_Font;

static FT_Library ftLibrary;			/* handle to library     */
static HBM_Font *sansSerif, *serif;		/* handle to face object */
#ifdef HBM_ENABLE_ROMFS
static HBMRomfsFile sansSerifFile, serifFile;
#endif

#ifdef HBM_USE_MAP_FOR_GLYPHS
	static std::map<wchar_t, HBM_Glyph> sansSerifGlyphs, serifGlyphs;
#else
	int sansSerifGlyphsUsed, serifGlyphsUsed;
	static HBM_Glyph sansSerifGlyphs[HBM_MAX_GLYPHS_SANSSERIF], serifGlyphs[HBM_MAX_GLYPHS_SERIF];
#endif /* HBM_USE_MAP_FOR_GLYPHS */

static int fontType = -1;

#define HBM_ADJUST_FONT_FOR_SPACING(font, advanceX) (font->spacing > 1 || font->spacing < 1 ? lround((advanceX) * font->spacing) : (advanceX))
#define HBM_CHECK_GLYPH_SPACE(glyph) (glyph == L' ' || glyph == L' ' /* short space */)
#define HBM_CHECK_GLYPH_LATIN(glyph) (((int)(glyph) >= 0x0041 && (int)(glyph) <= 0x007A) \
									|| ((int)(glyph) >= 0x00C6 && (int)(glyph) <= 0x017E))
#define HBM_CHECK_GLYPH_CYRILLIC_GREEK(myFont, glyph) ((myFont->serifGlyph) && (((int)(glyph) >= 0x0400 && (int)(glyph) <= 0x04FF) /* Cyrillic */ \
																		  || ((int)(glyph) >= 0x0370 && (int)(glyph) <= 0x03FF) /* Greek/Coptic */))

/******************************************************
 *                  GLYPH PROCESSING                  *
 ******************************************************/

static void HBM_freeGlyphs(HBM_Font *myFont, bool force = false) {
	if (myFont == NULL) return;
	if (!myFont->cached && !force) return;

#ifdef HBM_USE_MAP_FOR_GLYPHS
	std::map<wchar_t, HBM_Glyph> *myGlyphs = myFont->serifGlyph ? &serifGlyphs : &sansSerifGlyphs;
	if (myGlyphs == NULL) return;
	if (myGlyphs->size() <= 0) return;

	for(std::map<wchar_t, HBM_Glyph>::iterator i = myGlyphs->begin(), iEnd = myGlyphs->end(); i != iEnd; ++i) {
		if (i->second.texObj) free(i->second.texObj);
		if (i->second.image) free(i->second.image);
		i->second = (HBM_Glyph)
		{
			0,
			0,
			0,
			0,
			0,
			NULL,
			NULL
		};
	}

	myGlyphs->clear();
#else
	HBM_Glyph* myGlyphs = myFont->serifGlyph ? serifGlyphs : sansSerifGlyphs;
	int myGlyphsUsed = myFont->serifGlyph ? serifGlyphsUsed : sansSerifGlyphsUsed;

	for (int i = 0; i < myGlyphsUsed; i++) {
		if (myGlyphs[i].texObj) free(myGlyphs[i].texObj);
		if (myGlyphs[i].image) free(myGlyphs[i].image);
		myGlyphs[i] = (HBM_Glyph)
		{
			0,
			0,
			0,
			0,
			0,
			0,
			NULL,
			NULL
		};
	}

	if (myFont->serifGlyph) { serifGlyphsUsed = 0; } else { sansSerifGlyphsUsed = 0; }
#endif /* HBM_USE_MAP_FOR_GLYPHS */

	myFont->cached = false;
}

static HBM_Glyph* HBM_cacheGlyph(wchar_t Char, HBM_Font *myFont) {
	if (myFont == NULL) return NULL;

#ifdef HBM_USE_MAP_FOR_GLYPHS
	std::map<wchar_t, HBM_Glyph>* myGlyphs = myFont->serifGlyph ? &serifGlyphs : &sansSerifGlyphs;
	if (myGlyphs == NULL) return NULL;

	// Clear glyph list periodically so as to not run out of memory
	if (sansSerifGlyphs.size() >= HBM_MAX_GLYPHS_SANSSERIF) {
		HBM_ConsolePrintf("Ran out of s-serif slots");
		HBM_freeGlyphs(sansSerif);
	}
	if (serifGlyphs.size() >= HBM_MAX_GLYPHS_SERIF) {
		HBM_ConsolePrintf("Ran out of serif slots");
		HBM_freeGlyphs(serif);
	}

	// Search for existing glyph copy first
	if (myGlyphs->find(Char) != myGlyphs->end())
		return &((*myGlyphs)[Char]);
#else
	HBM_Glyph* myGlyphs = myFont->serifGlyph ? serifGlyphs : sansSerifGlyphs;

	// Clear glyph list periodically so as to not run out of memory
	if (sansSerifGlyphsUsed >= HBM_MAX_GLYPHS_SANSSERIF) {
		HBM_ConsolePrintf("Ran out of s-serif slots");
		HBM_freeGlyphs(sansSerif);
	}
	if (serifGlyphsUsed >= HBM_MAX_GLYPHS_SERIF) {
		HBM_ConsolePrintf("Ran out of serif slots");
		HBM_freeGlyphs(serif);
	}

	// Search for existing glyph copy first
	int target = myFont->serifGlyph ? serifGlyphsUsed : sansSerifGlyphsUsed;
	for (int i = 0; i < target; i++)
		if (myGlyphs[i].myChar == Char)
			return &myGlyphs[i];
#endif /* HBM_USE_MAP_FOR_GLYPHS */

	// If not, then create a new glyph.
	// Compress font size depending on screen resolution
	FT_Set_Char_Size(myFont->face, 64 * myFont->defaultSize, 64 * myFont->defaultSize, 0, 0);

	FT_UInt gIndex = FT_Get_Char_Index( myFont->face, Char );
	if (FT_Load_Glyph(myFont->face, gIndex, FT_LOAD_DEFAULT) != 0)
		return NULL;
	if (FT_Render_Glyph(myFont->face->glyph, FT_RENDER_MODE_NORMAL) != 0)
		return NULL;

	if (myFont->face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
		int cw = myFont->face->glyph->bitmap.width;
		int ch = myFont->face->glyph->bitmap.rows;
		int tw = (cw+7)/8;
		int th = (ch+3)/4;

		int tpitch = tw * 32;

#ifdef HBM_USE_MAP_FOR_GLYPHS
		(*myGlyphs)[Char] = (HBM_Glyph)
		{
			myFont->face->glyph->bitmap.width,
			myFont->face->glyph->bitmap.rows,
			myFont->face->glyph->bitmap_left,
			myFont->face->glyph->bitmap_top,
			myFont->face->glyph->advance.x,
			NULL,
			NULL
		};
#else
		myGlyphs[target] = (HBM_Glyph)
		{
			Char,
			myFont->face->glyph->bitmap.width,
			myFont->face->glyph->bitmap.rows,
			myFont->face->glyph->bitmap_left,
			myFont->face->glyph->bitmap_top,
			myFont->face->glyph->advance.x,
			NULL,
			NULL
		};
		if (myFont->serifGlyph) { serifGlyphsUsed++; } else { sansSerifGlyphsUsed++; }
#endif /* HBM_USE_MAP_FOR_GLYPHS */

		// Convert image bitmap to GX-compatible format and allocate memory to it within the glyph struct
#ifdef HBM_USE_MAP_FOR_GLYPHS
		(*myGlyphs)[Char].image = (uint8_t *)memalign(32, tw*th*32);
		memset((*myGlyphs)[Char].image, 0, tw*th*32);
#else
		myGlyphs[target].image = (uint8_t *)memalign(32, tw*th*32);
		memset(myGlyphs[target].image, 0, tw*th*32);
#endif /* HBM_USE_MAP_FOR_GLYPHS */

		int x,y;
		uint8_t *p = myFont->face->glyph->bitmap.buffer;
		for(y=0; y<ch; y++) {
			uint8_t *lp = p;
			int ty = y/4;
			int py = y%4;
#ifdef HBM_USE_MAP_FOR_GLYPHS
			uint8_t *lpix = (*myGlyphs)[Char].image + ty*tpitch + py*8;
#else
			uint8_t *lpix = myGlyphs[target].image + ty*tpitch + py*8;
#endif /* HBM_USE_MAP_FOR_GLYPHS */
			for(x=0; x<cw; x++) {
				int tx = x/8;
				int px = x%8;
				lpix[32*tx + px] = *lp++;
			}
			p += myFont->face->glyph->bitmap.pitch;
		}

#ifdef HBM_USE_MAP_FOR_GLYPHS
		DCFlushRange((*myGlyphs)[Char].image, 8*tw * 4*th);

		// Init tex obj
		(*myGlyphs)[Char].texObj = (GXTexObj *)malloc(sizeof(GXTexObj));
		DCFlushRange((*myGlyphs)[Char].texObj, sizeof(GXTexObj));
		GX_InitTexObj((*myGlyphs)[Char].texObj, (*myGlyphs)[Char].image, (*myGlyphs)[Char].width, (*myGlyphs)[Char].rows, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_InitTexObjLOD((*myGlyphs)[Char].texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_TRUE, GX_TRUE, GX_ANISO_1);

		if (!myFont->cached)
			myFont->cached = true;

		return &((*myGlyphs)[Char]);
#else
		DCFlushRange(myGlyphs[target].image, 8*tw * 4*th);

		// Init tex obj
		myGlyphs[target].texObj = (GXTexObj *)malloc(sizeof(GXTexObj));
		DCFlushRange(myGlyphs[target].texObj, sizeof(GXTexObj));
		GX_InitTexObj(myGlyphs[target].texObj, myGlyphs[target].image, myGlyphs[target].width, myGlyphs[target].rows, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_InitTexObjLOD(myGlyphs[target].texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_TRUE, GX_TRUE, GX_ANISO_1);

		if (!myFont->cached)
			myFont->cached = true;

		return &myGlyphs[target];
#endif /* HBM_USE_MAP_FOR_GLYPHS */
	}

	return NULL;
}

/******************************************************
 *                   TEXT PROCESSING                  *
 ******************************************************/

typedef struct HBM_textString {
	wchar_t* utf32;
	size_t length;
	int lines;
	enum HBM_TEXTALIGNH align_h;
	enum HBM_TEXTALIGNV align_v;
	float font_size;
	float scale_x;
	float scale_y;
} HBM_TextString;

static bool HBM_copyToTextString(HBM_TextString *myText, const char* string) {
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
	if (myFont == NULL || myFont->face == NULL || myText == NULL || myText->utf32 == NULL)
		return 0;

	FT_Face Face = (FT_Face)myFont->face;
	FT_GlyphSlot slot = Face->glyph;
	FT_UInt previousGlyph = 0;

	// Compress font size depending on screen resolution
	FT_Set_Char_Size(myFont->face, 64 * myFont->defaultSize, 64 * myFont->defaultSize, 0, 0);

	bool searchSingleLine = line >= 0 && line < myText->lines;
	int lineIndex = 0;
	int lineWidth = 0;
	int textWidth = 0;
	int textHeight = myFont->defaultSize;

	for (size_t i = 0; i < myText->length; i++) {
		if (searchSingleLine) {
			if (myText->utf32[i] == L'\n' || i == myText->length - 1) {
				if (lineIndex == line || i == myText->length - 1)
					return return_height ? textHeight : lineWidth;
				else {
					lineIndex++;
					lineWidth = 0;
					continue;
				}
			}
		} else {
			if (lineWidth > textWidth)
				textWidth = lineWidth;

			if (myText->utf32[i] == L'\n') {
				lineWidth = 0;
				textHeight += myFont->leading;
				continue;
			}
		}

		// Use FT_Load_Char?
		const FT_UInt glyphIndex = FT_Get_Char_Index(myFont->face, myText->utf32[i]);

		// Skip if a space character
		if (myText->utf32[i] == L' ' && myFont->customSpaceWidth > 0) {
			lineWidth += myFont->customSpaceWidth;
			previousGlyph = glyphIndex;
			continue;
		}
		if (myText->utf32[i] == L' ' && myFont->shortSpaceWidth > 0) { // short
			lineWidth += myFont->shortSpaceWidth;
			previousGlyph = glyphIndex;
			continue;
		}

		if (myFont->kerning && previousGlyph && glyphIndex) {
			FT_Vector delta;
			FT_Get_Kerning(myFont->face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
			lineWidth += delta.x >> 6;
		}

		if (FT_Load_Glyph(myFont->face, glyphIndex, FT_LOAD_DEFAULT) != 0)
			continue;

		if (myFont->letterSpaceWidth > 0 && i >= 1 && HBM_CHECK_GLYPH_LATIN(myText->utf32[i]))
			lineWidth += myFont->letterSpaceWidth;

		lineWidth += HBM_CHECK_GLYPH_SPACE(myText->utf32[i]) ? (slot->advance.x / 64.0f)
				   : HBM_CHECK_GLYPH_CYRILLIC_GREEK(myFont, myText->utf32[i]) ? HBM_ADJUST_FONT_FOR_SPACING(myFont, slot->bitmap.width)
				   : HBM_ADJUST_FONT_FOR_SPACING(myFont, slot->advance.x / 64.0f);

		if (myText->utf32[i] == L' ' && myFont->extraSpaceWidth > 0)
			lineWidth += myFont->extraSpaceWidth;
	}

	return return_height ? textHeight : textWidth;
}

static void HBM_TextStringDraw (HBM_TextString *myText,
								HBM_Font *myFont,
								int max_width,
								int max_height,
								float x,
								float y,
								const u8 cR,
								const u8 cG,
								const u8 cB,
								const u8 cA)
{
	if (myFont == NULL || myFont->face == NULL || myText == NULL || myText->utf32 == NULL)
		return;

	// FT_Face Face = (FT_Face)myFont->face;
	// FT_GlyphSlot slot = Face->glyph;
	FT_UInt previousGlyph = 0;
	int penX = myFont->xOffset;
	int penY = myFont->defaultSize + myFont->yOffset;
	float fontScale = myText->font_size > 0 ? myText->font_size / myFont->defaultSize : 1;

	int width = HBM_TextStringMeasure(myText, myFont, false) * fontScale;
	int height = HBM_TextStringMeasure(myText, myFont, true) * fontScale;
	if (max_width > 0 && width > max_width)
		myText->scale_x *= ((float)max_width / (float)width);
	if (max_height > 0 && height > max_height)
		myText->scale_y *= ((float)max_height / (float)height);

	int curLine = 0;
	int offsetX = myText->align_h == HBM_TEXT_CENTER ? (HBM_TextStringMeasure(myText, myFont, false, curLine) / 2)
				: myText->align_h == HBM_TEXT_RIGHT ? HBM_TextStringMeasure(myText, myFont, false, curLine)
				: 0;
	int offsetY = myText->align_v == HBM_TEXT_MIDDLE ? (HBM_TextStringMeasure(myText, myFont, true) / 2)
				: myText->align_v == HBM_TEXT_BOTTOM ? HBM_TextStringMeasure(myText, myFont, true)
				: 0;

	// Loop over each character, until the end of the string is reached, or until the pixel width is too wide
	for (size_t i = 0; i < myText->length - 1; i++) {
		if (myText->utf32[i] == L'\n') {
			penX = myFont->xOffset;
			penY += myFont->leading;

			curLine++;
			offsetX = myText->align_h == HBM_TEXT_CENTER ? (HBM_TextStringMeasure(myText, myFont, false, curLine) / 2)
					: myText->align_h == HBM_TEXT_RIGHT ? HBM_TextStringMeasure(myText, myFont, false, curLine)
					: 0;
			continue;
		}

		HBM_Glyph* glyphData = nullptr;
		if (!HBM_CHECK_GLYPH_SPACE(myText->utf32[i])) {
			glyphData = HBM_cacheGlyph(myText->utf32[i], myFont);
			if (glyphData == NULL) continue;
		}

		// Use FT_Load_Char?
		const FT_UInt glyphIndex = FT_Get_Char_Index(myFont->face, myText->utf32[i]);

		// Skip if a space character
		if (myText->utf32[i] == L' ') { // normal
			// Use custom width if found
			if (myFont->customSpaceWidth > 0) {
				penX += myFont->customSpaceWidth;
			} else {
				if (myFont->kerning && previousGlyph && glyphIndex) {
					FT_Vector delta;
					FT_Get_Kerning(myFont->face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
					penX += delta.x >> 6;
				}

				if (FT_Load_Glyph(myFont->face, glyphIndex, FT_LOAD_DEFAULT) == 0)
					penX += myFont->face->glyph->advance.x >> 6;

				if (myFont->extraSpaceWidth > 0)
					penX += myFont->extraSpaceWidth;
			}

			previousGlyph = glyphIndex;
			continue;
		}
		if (myText->utf32[i] == L' ') { // short
			// Use custom width if found
			if (myFont->shortSpaceWidth > 0) {
				penX += myFont->shortSpaceWidth;
			} else {
				if (myFont->kerning && previousGlyph && glyphIndex) {
					FT_Vector delta;
					FT_Get_Kerning(myFont->face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
					penX += delta.x >> 6;
				}

				if (FT_Load_Glyph(myFont->face, glyphIndex, FT_LOAD_DEFAULT) == 0)
					penX += myFont->face->glyph->advance.x >> 6;
			}

			previousGlyph = glyphIndex;
			continue;
		}

		if (myFont->kerning && previousGlyph && glyphIndex) {
			FT_Vector delta;
			FT_Get_Kerning(myFont->face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
			penX += delta.x >> 6;
		}

		if (myFont->letterSpaceWidth > 0 && i >= 1 && HBM_CHECK_GLYPH_LATIN(myText->utf32[i]))
			penX += myFont->letterSpaceWidth;

		/************************************/
		GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
		GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

		Mtx m,mv; // Better to use matrices for position
		guMtxIdentity (m);
		guMtxTransApply(m, m, -offsetX, -offsetY, 0);
		guMtxScaleApply(m, m, myText->scale_x * fontScale, myText->scale_y * fontScale, 1.0);
		guMtxTransApply(m, m, x, y, 0);
		guMtxScaleApply(m, m, HBM_Settings.ScaleX, HBM_Settings.ScaleY, 1.0);
		guMtxConcat(HBM_GXmodelView2D, m, mv);
		GX_LoadPosMtxImm(mv, GX_PNMTX0);

		GX_LoadTexObj(glyphData->texObj, GX_TEXMAP0);

		GX_Begin(GX_QUADS, HBM_GX_VTXFMT, 4);
			GX_Position2f32(penX + glyphData->left, penY - glyphData->top);
			GX_Color4u8(cR, cG, cB, cA);
			GX_TexCoord2f32(0, 0);
			GX_Position2f32(penX + glyphData->left + glyphData->width, penY - glyphData->top);
			GX_Color4u8(cR, cG, cB, cA);
			GX_TexCoord2f32(1, 0);
			GX_Position2f32(penX + glyphData->left + glyphData->width, penY - glyphData->top + glyphData->rows);
			GX_Color4u8(cR, cG, cB, cA);
			GX_TexCoord2f32(1, 1);
			GX_Position2f32(penX + glyphData->left, penY - glyphData->top + glyphData->rows);
			GX_Color4u8(cR, cG, cB, cA);
			GX_TexCoord2f32(0, 1);
		GX_End();
		GX_LoadPosMtxImm(HBM_GXmodelView2D, GX_PNMTX0);

		GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
		GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
		/************************************/

		penX += HBM_CHECK_GLYPH_CYRILLIC_GREEK(myFont, myText->utf32[i]) ? HBM_ADJUST_FONT_FOR_SPACING(myFont, glyphData->width + 2)
			  : HBM_ADJUST_FONT_FOR_SPACING(myFont, glyphData->advance / 64.0f);
		previousGlyph = glyphIndex;
	}
}

/******************************************************
 *               PUBLIC TEXT FUNCTIONS                *
 ******************************************************/

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
					int max_width,
					int max_height)
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
			max_height,			// max_height
			x, 					// x
			y, 					// y
			cR, cG, cB, cA		// color
		);
		HBM_freeTextString(&text);
	}
}

int HBM_MeasureText(const char* string, float size, bool use_serif, bool return_height)
{
	HBM_Font *myFont = use_serif ? serif : sansSerif;
	if (myFont == NULL || string == NULL)
		return 0;

	int result = 0;
	HBM_TextString text = {
		.align_h = HBM_TEXT_LEFT,
		.align_v = HBM_TEXT_TOP,
		.font_size = size,
		.scale_x = 1,
		.scale_y = 1
	};

	if (HBM_copyToTextString(&text, string)) {
		result = HBM_TextStringMeasure(&text, myFont, return_height) * (size / myFont->defaultSize);
		HBM_freeTextString(&text);
	}

	return result;
}

bool HBM_CheckMultilineText(const char* string)
{
	for (size_t i = 0; string[i] != '\0'; i++) {
		if (string[i] == '\n') {
			return true;
		}
	}

	return false;
}

/******************************************************
 *                    FONT LOADING                    *
 ******************************************************/

static HBM_Font* HBM_FontLoadTTF(const uint8_t *file, size_t size)
{
	FT_Face Face = NULL;
	if (FT_New_Memory_Face(ftLibrary, file, size, 0, &Face) != 0)
		return NULL;

	HBM_Font* myFont = (HBM_Font*)malloc(sizeof(HBM_Font));
	if (myFont == 0)
		return NULL;
	memset(myFont, 0, sizeof(*myFont));

	myFont->kerning = FT_HAS_KERNING(Face);
	myFont->face = Face;

	return myFont;
}

static void HBM_FontFreeTTF(HBM_Font *myFont)
{
	if (myFont != NULL) {
		HBM_freeGlyphs(myFont, true);
		free(myFont);
	}
}

/******************************************************
 *               PUBLIC FONT FUNCTIONS                *
 ******************************************************/

#ifndef HBM_ENABLE_ROMFS
extern const uint8_t HBM_font_serif_ttf[];
extern const uint8_t HBM_font_serif_ttf_end[];
extern const uint8_t HBM_font_sansserif_ttf[];
extern const uint8_t HBM_font_sansserif_ttf_end[];
extern const uint8_t HBM_font_zh_sansserif_ttf[];
extern const uint8_t HBM_font_zh_sansserif_ttf_end[];
extern const uint8_t HBM_font_ko_serif_ttf[];
extern const uint8_t HBM_font_ko_serif_ttf_end[];
extern const uint8_t HBM_font_ko_sansserif_ttf[];
extern const uint8_t HBM_font_ko_sansserif_ttf_end[];
extern const uint8_t HBM_font_zgh_serif_ttf[];
extern const uint8_t HBM_font_zgh_serif_ttf_end[];
extern const uint8_t HBM_font_zgh_sansserif_ttf[];
extern const uint8_t HBM_font_zgh_sansserif_ttf_end[];
#endif

void HBM_FontUninit()
{
	if (fontType != -1) {
		HBM_FontFreeTTF(sansSerif);
		HBM_FontFreeTTF(serif);
		#ifdef HBM_ENABLE_ROMFS
			sansSerifFile.Free();
			serifFile.Free();
		#endif
		FT_Done_FreeType(ftLibrary);
		fontType = -1;
	}
}

bool HBM_FontInit(int type)
{
	if (type != fontType && type >= 0) {
		HBM_FontUninit();

		// Init FreeType library
		if (FT_Init_FreeType(&ftLibrary) != 0)
			return false;

		// Load fonts
		switch (type) {
			default:
				break;

			case 0: // International
				#ifdef HBM_ENABLE_ROMFS
					sansSerifFile.Load("romfs:/hbm/ttf/HBM_font_sansserif.ttf");
					serifFile.Load("romfs:/hbm/ttf/HBM_font_serif.ttf");
				#else
					sansSerif = HBM_FontLoadTTF(HBM_font_sansserif_ttf, (size_t)HBM_font_sansserif_ttf_end - (size_t)HBM_font_sansserif_ttf);
					serif = HBM_FontLoadTTF(HBM_font_serif_ttf, (size_t)HBM_font_serif_ttf_end - (size_t)HBM_font_serif_ttf);
				#endif
				break;

			case 1: // Hanzi
				#ifdef HBM_ENABLE_ROMFS
					sansSerifFile.Load("romfs:/hbm/ttf/HBM_font_zh_sansserif.ttf");
					serifFile.Load("romfs:/hbm/ttf/HBM_font_serif.ttf");
				#else
					sansSerif = HBM_FontLoadTTF(HBM_font_zh_sansserif_ttf, (size_t)HBM_font_zh_sansserif_ttf_end - (size_t)HBM_font_zh_sansserif_ttf);
					serif = HBM_FontLoadTTF(HBM_font_serif_ttf, (size_t)HBM_font_serif_ttf_end - (size_t)HBM_font_serif_ttf);
				#endif
				break;

			case 2: // Korean
				#ifdef HBM_ENABLE_ROMFS
					sansSerifFile.Load("romfs:/hbm/ttf/HBM_font_ko_sansserif.ttf");
					serifFile.Load("romfs:/hbm/ttf/HBM_font_ko_serif.ttf");
				#else
					sansSerif = HBM_FontLoadTTF(HBM_font_ko_sansserif_ttf, (size_t)HBM_font_ko_sansserif_ttf_end - (size_t)HBM_font_ko_sansserif_ttf);
					serif = HBM_FontLoadTTF(HBM_font_ko_serif_ttf, (size_t)HBM_font_ko_serif_ttf_end - (size_t)HBM_font_ko_serif_ttf);
				#endif
				break;

			case 3: // Tifinagh
				#ifdef HBM_ENABLE_ROMFS
					sansSerifFile.Load("romfs:/hbm/ttf/HBM_font_zgh_sansserif.ttf");
					serifFile.Load("romfs:/hbm/ttf/HBM_font_zgh_serif.ttf");
				#else
					sansSerif = HBM_FontLoadTTF(HBM_font_zgh_sansserif_ttf, (size_t)HBM_font_zgh_sansserif_ttf_end - (size_t)HBM_font_zgh_sansserif_ttf);
					serif = HBM_FontLoadTTF(HBM_font_zgh_serif_ttf, (size_t)HBM_font_zgh_serif_ttf_end - (size_t)HBM_font_zgh_serif_ttf);
				#endif
				break;
		}

		#ifdef HBM_ENABLE_ROMFS
			sansSerif = HBM_FontLoadTTF(sansSerifFile.Data(), sansSerifFile.Size());
			serif = HBM_FontLoadTTF(serifFile.Data(), serifFile.Size());
		#endif

		if (sansSerif == NULL || serif == NULL) {
			// Failed to load either font, stop
			HBM_FontFreeTTF(sansSerif);
			HBM_FontFreeTTF(serif);
			#ifdef HBM_ENABLE_ROMFS
				sansSerifFile.Free();
				serifFile.Free();
			#endif
			FT_Done_FreeType(ftLibrary);
			return false;
		}

		/** Sans-serif options **/
		if (sansSerif != NULL) {
			sansSerif->serifGlyph = false;
			sansSerif->spacing = 1;
			sansSerif->defaultSize = 40;
			sansSerif->leading = 40;

			switch (type) {
				default:
				case 0: // International
					break;
				case 1: // Hanzi
					sansSerif->defaultSize = 36;
					// sansSerif->xOffset = 1;
					break;
				case 2: // Korean
					sansSerif->defaultSize = 29.5;
					sansSerif->shortSpaceWidth = 2;
					sansSerif->xOffset = 1;
					sansSerif->yOffset = -2.6;
					break;
				case 3: // Tifinagh
					sansSerif->defaultSize = 36;
					break;
			}
		}

		/** Serif options **/
		if (serif != NULL) {
			serif->serifGlyph = true;
			serif->kerning = false;
			serif->spacing = 1;

			switch (type) {
				default:
					break;
				case 0: // International
					serif->defaultSize = 28;
					serif->leading = 38;
					serif->customSpaceWidth = 11;
					serif->letterSpaceWidth = 1;
					break;
				case 1: // Hanzi
					serif->defaultSize = 22;
					serif->leading = 28;
					serif->customSpaceWidth = 10;
					serif->letterSpaceWidth = 0;
					break;
				case 2: // Korean
					serif->defaultSize = 22;
					serif->leading = 28;
					serif->customSpaceWidth = 9;
					serif->letterSpaceWidth = 1;
					break;
				case 3: // Tifinagh
					serif->defaultSize = 22;
					serif->leading = 28;
					serif->customSpaceWidth = 9;
					serif->letterSpaceWidth = 0;
					break;
			}
		}

		fontType = type;
	}

	return true;
}

#ifdef HBM_USE_ERROR_SCREEN
void HBM_FontSetForError()
{
	HBM_freeGlyphs(sansSerif);
	sansSerif->defaultSize = 29.5;
	sansSerif->customSpaceWidth = 14;
	sansSerif->spacing = 0.9425;
	sansSerif->leading = 39;
}
#endif

int HBM_FontType()
{
	return fontType;
}

#undef HBM_ADJUST_FONT_FOR_SPACING
#undef HBM_CHECK_GLYPH_SPACE
#undef HBM_CHECK_GLYPH_LATIN
#undef HBM_CHECK_GLYPH_CYRILLIC_GREEK