#include "HBM.h"
#include "HBM_Extern.h"
#include <cstring>
#include <stdarg.h>

#define HBM_CONSOLE_MSG_COUNT 12
#define HBM_CONSOLE_MSG_SIZE 512
#define HBM_CONSOLE_MSG_FONTSIZE 12
#define HBM_CONSOLE_MSG_BORDER 3
#define HBM_CONSOLE_MSG_OFFSET 8
#define HBM_CONSOLE_MSG_HEIGHT (HBM_CONSOLE_MSG_BORDER + HBM_CONSOLE_MSG_BORDER + HBM_CONSOLE_MSG_FONTSIZE)

#ifdef HBM_VERBOSE

	typedef struct {
		char text[HBM_CONSOLE_MSG_SIZE];
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
		msg[i].used = false;
		memset(msg[i].text, 0, HBM_CONSOLE_MSG_SIZE);
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
			vsnprintf(msg[i].text, sizeof(msg[i].text), string, argp);
			va_end(argp);

			msg[i].opened = (f64)ticks_to_millisecs(gettime()) / 1000.0F;
			msg[i].used = true;
			return;
		}
	}

	vsnprintf(msg[HBM_CONSOLE_MSG_COUNT - 1].text, sizeof(msg[HBM_CONSOLE_MSG_COUNT - 1].text), string, argp);
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
	vsnprintf(msg2.text, sizeof(msg2.text), string, argp);
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
				/* a */ 0.5F
			);
			HBM_DrawText
			(
				msg[i].text,
				/* x */ HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER,
				/* y */ HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER + (HBM_CONSOLE_MSG_HEIGHT * i),
				HBM_CONSOLE_MSG_FONTSIZE,
				false,
				0xFFFFFFFF
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
			/* a */ 0.5F
		);
		HBM_DrawText
		(
			msg2.text,
			/* x */ HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER,
			/* y */ HBM_Settings.Height - HBM_CONSOLE_MSG_OFFSET + HBM_CONSOLE_MSG_BORDER - HBM_CONSOLE_MSG_HEIGHT,
			HBM_CONSOLE_MSG_FONTSIZE,
			false,
			0xFFFFFFFF
		);
	}

#endif /* HBM_VERBOSE */
}

void HBM_ConsoleUpdate()
{
#ifdef HBM_VERBOSE

	for (int i = 0; i < HBM_CONSOLE_MSG_COUNT; i++)
	{
		if (msg[i].used && (f64)ticks_to_millisecs(gettime()) / 1000.0F >= msg[i].opened + 3.0F)
		{
			memset(msg[i].text, 0, HBM_CONSOLE_MSG_SIZE);
			msg[i].opened = 0;
			msg[i].used = false;
		}
	}

	/* Second console */
	if (msg2.used && (f64)ticks_to_millisecs(gettime()) / 1000.0F >= msg2.opened + 3.0F)
	{
		memset(msg2.text, 0, HBM_CONSOLE_MSG_SIZE);
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