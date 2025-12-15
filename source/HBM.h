#ifndef __HBM_library__
#define __HBM_library__

// Global libraries
// ******************************
#include <gccore.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <asndlib.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>

// Config
// ******************************
#include "HBM_Config.h"

#ifndef HBM_UI_LANGUAGE
#define HBM_UI_LANGUAGE -1
#endif

#if defined(HBM_DEBUG) && !defined(HBM_VERBOSE)
#define HBM_VERBOSE
#endif

// Utilities
// ******************************
#include <pngu.h>
#include "utils/i18n/gettext.h"

// Filelist
// ******************************
#include "HBM_Files.h"
#include "HBM_Romfs.h"

// Classes
// ******************************
#include "HBM_HBMImage.h"
#include "HBM_HBMElement.h"
#include "HBM_HBMButtonMain.h"

#include "HBM_Console.h"
#include "HBM_Font.h"
#include "HBM_WPad.h"
#include "HBM_UI.h"

#endif