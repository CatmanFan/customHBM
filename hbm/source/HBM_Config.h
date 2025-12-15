#ifndef __HBM_CONFIG_H__
#define __HBM_CONFIG_H__

/**
 * Language setting.
 *
 * -1: System (auto)		7:  Chinese	(Simplified)	15: Turkish
 * 0:  Japanese				8:  Chinese (Traditional)	16: Swedish
 * 1:  English				9:  Korean					17: Catalan
 * 2:  German				10: Portuguese
 * 3:  French				11: Portuguese (Brazil)
 * 4:  Spanish				12: Russian
 * 5:  Italian				13: Ukrainian
 * 6:  Dutch				14: Polish
 **/
#define HBM_UI_LANGUAGE -1

/**
 * Render textures and elements in three-dimensional instead of two-dimensional vertices.
 * Set this to ON if using GRRLIB, OFF if using Libwiisprite.
 * The wrong setting is likely to freeze the console.
 **/
#define HBM_USE_3D_RENDER

#define HBM_ENABLE_SOUND

/**
 * Verbose & debug options
 **/
#define HBM_VERBOSE
// #define HBM_DEBUG

#endif