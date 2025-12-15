#include "HBM.h"
#include "HBM_Extern.h"

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

#include "HBM.h"
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct HBM_ttfFont {
	FT_Face /* void * */ face;     /**< A TTF face object. */
	bool kerning;   /**< true whenever a face object contains kerning data that can be accessed with FT_Get_Kerning. */
} HBM_Font;

static FT_Library ftLibrary;			/* handle to library     */
static HBM_Font *sansSerif, *serif;		/* handle to face object */

static int fontType = -1;

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

static void DrawBitmap(FT_Bitmap *bitmap, int offset, int top, const u8 cR, const u8 cG, const u8 cB, const u8 cA) {
	const FT_Int x_max = offset + bitmap->width;
	const FT_Int y_max = top + bitmap->rows;

	for (FT_Int i = offset, p = 0; i < x_max; i++, p++ )
	{
		for (FT_Int j = top, q = 0; j < y_max; j++, q++ )
		{
			s16 alpha = bitmap->buffer[ q * bitmap->width + p ] - (0xFF - cA);
			if (alpha < 0) alpha = 0;

			GX_Begin(GX_POINTS, GX_VTXFMT0, 1);
			#ifdef HBM_USE_3D_RENDER
				GX_Position3f32(i, j, 0);
			#else
				GX_Position2f32(i, j);
			#endif
				GX_Color4u8(cR, cG, cB, alpha);
			GX_End();
		}
	}
}

static int HBM_ProcessTextWide(const wchar_t *utf32, HBM_Font *myFont, float fontSize, bool return_height, bool should_draw, int x, int y, const u8 cR, const u8 cG, const u8 cB, const u8 cA)
{
	if (myFont == NULL || utf32 == NULL)
		return 0;

	FT_Face Face = (FT_Face)myFont->face;
	int penX = 0;
	int penY = fontSize;
	FT_GlyphSlot slot = Face->glyph;
	FT_UInt previousGlyph = 0;

	// Compress font size depending on screen resolution
	float fractionalPixelSize = 64 * fontSize;
	// unsigned int fractionalPixelSize = 64 * fontSize;
	if (FT_Set_Char_Size(Face, fractionalPixelSize, fractionalPixelSize, 0, 0) != 0) {
		FT_Set_Char_Size(Face, (64 * 12) * HBM_Settings.ScaleX, (64 * 12) * HBM_Settings.ScaleY, 0, 0);
	}

	// Loop over each character, until the end of the string is reached, or until the pixel width is too wide
	while(*utf32) {
		const FT_UInt glyphIndex = FT_Get_Char_Index(myFont->face, *utf32++);

		if (myFont->kerning && previousGlyph && glyphIndex) {
			FT_Vector delta;
			FT_Get_Kerning(myFont->face, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
			penX += delta.x >> 6;
		}
		if (FT_Load_Glyph(myFont->face, glyphIndex, FT_LOAD_RENDER) != 0)
			continue;

		if (should_draw) {
			DrawBitmap(&slot->bitmap,
					   penX + slot->bitmap_left + x,
					   penY - slot->bitmap_top + y,
					   cR, cG, cB, cA);
		}
		penX += slot->advance.x >> 6;
		previousGlyph = glyphIndex;
	}

	return return_height ? penY : penX;
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
		// ttf_serif.Load("romfs:/hbm/ttf/UtrilloProGrecoStd.ttf");
		sansSerif = HBM_FontLoadTTF(ttf_sansSerif.Data(), ttf_sansSerif.Size());
		serif = HBM_FontLoadTTF(ttf_sansSerif.Data(), ttf_sansSerif.Size());
		fontType = type;
	}
}

void HBM_FontUninit()
{
	HBM_FontFreeTTF(sansSerif);
	HBM_FontFreeTTF(serif);
}

int HBM_MeasureText(const char *string, float size, bool use_serif, bool return_height)
{
	int result = 0;
	HBM_Font *myFont = use_serif ? serif : sansSerif;

	if (myFont == NULL || string == NULL)
		return result;

	size_t length = strlen(string) + 1;
	wchar_t *utf32 = (wchar_t*)malloc(length * sizeof(wchar_t));
	if (utf32 != NULL) {
		length = mbstowcs(utf32, string, length);
		if (length > 0) {
			utf32[length] = L'\0';
			result = HBM_ProcessTextWide(utf32, myFont, size, return_height, false, 0, 0, 0, 0, 0, 0);
		}
		free(utf32);
	}

	return result;
}

void HBM_DrawText(const char *string, int x, int y, float size, bool use_serif, const u8 cR, const u8 cG, const u8 cB, const u8 cA)
{
	HBM_Font *myFont = use_serif ? serif : sansSerif;

	if (myFont == NULL || string == NULL)
		return;

	size_t length = strlen(string) + 1;
	wchar_t *utf32 = (wchar_t*)malloc(length * sizeof(wchar_t));
	if (utf32 != NULL) {
		length = mbstowcs(utf32, string, length);
		if (length > 0) {
			utf32[length] = L'\0';
			HBM_ProcessTextWide(utf32, myFont, size * HBM_Settings.ScaleY, false, true, lround(x / HBM_Settings.ScaleX), lround(y / HBM_Settings.ScaleY), cR, cG, cB, cA);
		}
		free(utf32);
	}
}

void HBM_DrawText(const char *string, int x, int y, float size, bool use_serif, const u32 color)
{
	HBM_DrawText(string, x, y, size, use_serif, (((color) >>24) & 0xFF), (((color) >>16) & 0xFF), (((color) >> 8) & 0xFF), ((color) & 0xFF));
}