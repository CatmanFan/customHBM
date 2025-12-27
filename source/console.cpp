#include "hbm.h"
#include "hbm/extern.h"
#include <cstring>
#include <stdarg.h>

#define HBM_CONSOLE_MSG_COUNT 1
#define HBM_CONSOLE_MSG_SIZE 256
#define HBM_CONSOLE_MSG_FONTSIZE 13
#define HBM_CONSOLE_MSG_BORDER 4
#define HBM_CONSOLE_MSG_OFFSET 10
#define HBM_CONSOLE_MSG_HEIGHT (HBM_CONSOLE_MSG_BORDER + HBM_CONSOLE_MSG_BORDER + HBM_CONSOLE_MSG_FONTSIZE)
#define HBM_CONSOLE_MSG_TIME 2.0F

#ifdef HBM_VERBOSE

	typedef struct {
		char *text;
		size_t text_size;
		bool used;
		f64 opened;
	} HBM_ConsoleMsg;

	static HBM_ConsoleMsg msg[HBM_CONSOLE_MSG_COUNT];
	/* Second console */
	static HBM_ConsoleMsg msg2;

#endif /* HBM_VERBOSE */

void HBM_ConsoleInit()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
	{
		msg[i].text = nullptr;
		msg[i].text_size = 0;
		msg[i].used = false;
		msg[i].opened = 0;
	}

#endif /* HBM_VERBOSE */
}

void HBM_ConsolePrintf(const char *string, ...)
{
#ifdef HBM_VERBOSE

	va_list argp;
	va_start(argp, string);

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
	{
		if (!msg[i].used)
		{
			msg[i].text_size = sizeof(char) * HBM_CONSOLE_MSG_SIZE;
			msg[i].text = (char *)malloc(msg[i].text_size);
			vsnprintf(msg[i].text, msg[i].text_size, string, argp);
			va_end(argp);

			msg[i].opened = (f64)ticks_to_millisecs(gettime()) / 1000.0F;
			msg[i].used = true;
			return;
		}
	}

	vsnprintf(msg[HBM_CONSOLE_MSG_COUNT - 1].text, msg[HBM_CONSOLE_MSG_COUNT - 1].text_size, string, argp);
	va_end(argp);

	msg[HBM_CONSOLE_MSG_COUNT - 1].opened = (f64)ticks_to_millisecs(gettime()) / 1000.0F;
	msg[HBM_CONSOLE_MSG_COUNT - 1].used = true;

#endif /* HBM_VERBOSE */
}

/* Second console */
void HBM_ConsolePrintf2(const char *string, ...)
{
#ifdef HBM_VERBOSE

	va_list argp;
	va_start(argp, string);
	msg2.text_size = sizeof(char) * HBM_CONSOLE_MSG_SIZE;
	msg2.text = (char *)malloc(msg2.text_size);
	vsnprintf(msg2.text, msg2.text_size, string, argp);
	va_end(argp);

	msg2.opened = (f64)ticks_to_millisecs(gettime()) / 1000.0F;
	msg2.used = true;

#endif /* HBM_VERBOSE */
}

void HBM_ConsoleDraw()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
	{
		if (msg[i].used)
		{
			HBM_DrawBlackQuad
			(
				/* x */ HBM_CONSOLE_MSG_OFFSET,
				/* y */ HBM_CONSOLE_MSG_OFFSET + (HBM_CONSOLE_MSG_HEIGHT * i),
				/* w */ HBM_MeasureText(msg[i].text, HBM_CONSOLE_MSG_FONTSIZE, false, false) + HBM_CONSOLE_MSG_OFFSET,
				/* h */ HBM_CONSOLE_MSG_HEIGHT,
				/* a */ 0.5F,
				/* noWidescreen */ true
			);
			HBM_DrawText
			(
				msg[i].text,
				/* x */ HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER,
				/* y */ HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER + (HBM_CONSOLE_MSG_HEIGHT * i),
				/* size */ HBM_CONSOLE_MSG_FONTSIZE,
				/* scaleX */ 1.0,
				/* scaleY */ 1.0,
				/* align */ HBM_TEXT_LEFT, HBM_TEXT_TOP,
				false,
				255, 255, 255, 255,
				0
			);
		}
	}

	/* Second console */
	if (msg2.used)
	{
		HBM_DrawBlackQuad
		(
			/* x */ HBM_CONSOLE_MSG_OFFSET,
			/* y */ HBM_Settings.Height - HBM_CONSOLE_MSG_OFFSET - HBM_CONSOLE_MSG_HEIGHT,
			/* w */ HBM_MeasureText(msg2.text, HBM_CONSOLE_MSG_FONTSIZE, false, false) + HBM_CONSOLE_MSG_OFFSET,
			/* h */ HBM_CONSOLE_MSG_HEIGHT,
			/* a */ 0.5F,
			/* noWidescreen */ true
		);
		HBM_DrawText
		(
			msg2.text,
			/* x */ HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER,
			/* y */ HBM_Settings.Height - HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER - HBM_CONSOLE_MSG_HEIGHT,
			/* size */ HBM_CONSOLE_MSG_FONTSIZE,
			/* scaleX */ 1.0,
			/* scaleY */ 1.0,
			/* align */ HBM_TEXT_LEFT, HBM_TEXT_TOP,
			false,
			255, 255, 255, 255,
			0
		);
	}

#endif /* HBM_VERBOSE */
}

void HBM_ConsoleUpdate()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
	{
		if (msg[i].used && (f64)ticks_to_millisecs(gettime()) / 1000.0F >= msg[i].opened + HBM_CONSOLE_MSG_TIME)
		{
			free(msg[i].text);
			msg[i].text_size = 0;
			msg[i].opened = 0;
			msg[i].used = false;
		}
	}

	/* Second console */
	if (msg2.used && (f64)ticks_to_millisecs(gettime()) / 1000.0F >= msg2.opened + HBM_CONSOLE_MSG_TIME)
	{
		free(msg2.text);
		msg2.text_size = 0;
		msg2.opened = 0;
		msg2.used = false;
	}

#endif /* HBM_VERBOSE */
}

#undef HBM_CONSOLE_MSG_COUNT
#undef HBM_CONSOLE_MSG_SIZE
#undef HBM_CONSOLE_MSG_FONTSIZE
#undef HBM_CONSOLE_MSG_BORDER
#undef HBM_CONSOLE_MSG_OFFSET
#undef HBM_CONSOLE_MSG_HEIGHT
#undef HBM_CONSOLE_MSG_TIME