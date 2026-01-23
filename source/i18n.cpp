#include <gctypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <string>
using namespace std;

#include "hbm.h"

#ifdef WII_NETTRACE
#include <network.h>
#include "net_print.h"
#endif

static map<string, string> msgmap;

/* Expand some escape sequences found in the argument string.  */
static char* expand_escape(const char* str) {
    char *retval, *rp;
    const char* cp = str;

    retval = (char*)malloc(strlen(str) + 1);
    if (retval == NULL)
        return NULL;
    rp = retval;

    while (cp[0] != '\0' && cp[0] != '\\')
        *rp++ = *cp++;
    if (cp[0] == '\0')
        goto terminate;
    do {
        /* Here cp[0] == '\\'.  */
        switch (*++cp) {
            case '\"': /* " */
                *rp++ = '\"';
                ++cp;
                break;
            case 'a': /* alert */
                *rp++ = '\a';
                ++cp;
                break;
            case 'b': /* backspace */
                *rp++ = '\b';
                ++cp;
                break;
            case 'f': /* form feed */
                *rp++ = '\f';
                ++cp;
                break;
            case 'n': /* new line */
                *rp++ = '\n';
                ++cp;
                break;
            case 'r': /* carriage return */
                *rp++ = '\r';
                ++cp;
                break;
            case 't': /* horizontal tab */
                *rp++ = '\t';
                ++cp;
                break;
            case 'v': /* vertical tab */
                *rp++ = '\v';
                ++cp;
                break;
            case '\\':
                *rp = '\\';
                ++cp;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                int ch = *cp++ - '0';

                if (*cp >= '0' && *cp <= '7') {
                    ch *= 8;
                    ch += *cp++ - '0';

                    if (*cp >= '0' && *cp <= '7') {
                        ch *= 8;
                        ch += *cp++ - '0';
                    }
                }
                *rp = ch;
            } break;
            default:
                *rp = '\\';
                break;
        }

        while (cp[0] != '\0' && cp[0] != '\\')
            *rp++ = *cp++;
    } while (cp[0] != '\0');

    /* Terminate string.  */
terminate:
    *rp = '\0';
    return retval;
}

static void setMSG(const char* msgid, const char* msgstr) {
    if (msgid && msgstr) {
        char* msg = expand_escape(msgstr);
        msgmap[msgid] = msg;
        free(msg);
    }
}

static void gettextCleanUp(void) {
    msgmap.clear();
}

static char * memfgets(char * dst, int maxlen, char * src)
{
	if(!src || !dst || maxlen <= 0)
		return NULL;

	char * newline = strchr(src, '\n');

	if(newline == NULL)
		return NULL;

	memcpy(dst, src, (newline-src));
	dst[(newline-src)] = 0;
	return ++newline;
}

static bool getLangFile(char* langfile, size_t size) {
    if (!langfile) {
        gettextCleanUp();
        return true;
    }

    char line[200];
    char* lastID = NULL;

    gettextCleanUp();

	while (langfile && langfile < langfile + size) {
		langfile = memfgets(line, sizeof(line), langfile);
		if (!langfile)
			break;

		// lines starting with # are comments
		if (line[0] == '#')
			continue;

		if (strncmp(line, "msgid \"", 7) == 0) {
			char *msgid, *end;
			if (lastID) {
				free(lastID);
				lastID = NULL;
			}
			msgid = &line[7];
			end = strrchr(msgid, '"');
			if (end && end - msgid > 1) {
				*end = 0;
				lastID = strdup(msgid);
			}
		} else if (strncmp(line, "msgstr \"", 8) == 0) {
			char *msgstr, *end;

			if (lastID == NULL)
				continue;

			msgstr = &line[8];
			end = strrchr(msgstr, '"');
			if (end && end - msgstr > 1) {
				*end = 0;
				setMSG(lastID, msgstr);
			}
			free(lastID);
			lastID = NULL;
		}
	}

    return true;
}

#ifdef HBM_ENABLE_ROMFS

	#define HBM_LANGUAGE_ROMFS_PATH(ID) "romfs:/hbm/text/HBM_" #ID ".lang"

	#define HBM_GET_LANGUAGE(ENUM, ID, NAME)	case ENUM: \
													if (!isSystem) HBM_ConsolePrintf("Language set to %d: %s", currentLanguage, #NAME ); \
													file.Load( HBM_LANGUAGE_ROMFS_PATH(ID) ); \
													return getLangFile((char *)file.Data(), file.Size());

#else

	#define HBM_LANGUAGE_EXTERN(ID) HBM_##ID##_lang
	#define HBM_LANGUAGE_EXTERN_END(ID) HBM_##ID##_lang_end

	#define HBM_GET_LANGUAGE(ENUM, ID, NAME)	case ENUM: \
													if (!isSystem) HBM_ConsolePrintf("Language set to %d: %s", currentLanguage, #NAME ); \
													extern const uint8_t HBM_LANGUAGE_EXTERN(ID)[]; \
													extern const uint8_t HBM_LANGUAGE_EXTERN_END(ID)[]; \
													return getLangFile((char *) HBM_LANGUAGE_EXTERN(ID), (size_t)HBM_LANGUAGE_EXTERN_END(ID) - (size_t)HBM_LANGUAGE_EXTERN(ID) );

#endif

static int currentLanguage = -2;

int HBM_GetCurrentLanguage() {
	return currentLanguage;
}

bool HBM_LoadLanguage(enum HBM_LANG lang) {
	if (lang != currentLanguage) {
		bool isSystem = false;

		if (lang >= HBM_LANG_COUNT || lang < HBM_LANG_SYSTEM) lang = HBM_LANG_SYSTEM;
		currentLanguage = (int)lang;

		if (lang == HBM_LANG_SYSTEM) {
			isSystem = true;
			// Narrow language selection based on region area
			switch (CONF_GetArea()) {
				default:
					lang = (enum HBM_LANG)CONF_GetLanguage();
					break;
				case CONF_AREA_CHN:
					lang = HBM_LANG_SIMP_CHINESE;
					break;
				case CONF_AREA_HKG:
				case CONF_AREA_TWN:
					lang = HBM_LANG_TRAD_CHINESE;
					break;
				case CONF_AREA_BRA:
					lang = HBM_LANG_PT_PORTUGUESE;
					break;
			}
			HBM_ConsolePrintf("Language set to System");
		}

		if (!HBM_FontInit(lang == HBM_LANG_KOREAN ? 2
						: lang == HBM_LANG_SIMP_CHINESE || lang == HBM_LANG_TRAD_CHINESE ? 1
						: 0))
			return false;

		#ifdef HBM_ENABLE_ROMFS
		HBMRomfsFile file;
		#endif

		switch (lang) {
			default:
			HBM_GET_LANGUAGE(HBM_LANG_ENGLISH,			en,			English)
			HBM_GET_LANGUAGE(HBM_LANG_JAPANESE,			ja,			Japanese)
			HBM_GET_LANGUAGE(HBM_LANG_GERMAN,			de,			Deutsch)
			HBM_GET_LANGUAGE(HBM_LANG_FRENCH,			fr,			Français)
			HBM_GET_LANGUAGE(HBM_LANG_SPANISH,			es,			Español)
			HBM_GET_LANGUAGE(HBM_LANG_ITALIAN,			it,			Italiano)
			HBM_GET_LANGUAGE(HBM_LANG_DUTCH,			nl,			Nederlands)

			#ifdef HBM_ENABLE_ROMFS
			HBM_GET_LANGUAGE(HBM_LANG_SIMP_CHINESE,		zh-Hans,	Chinese Simplified)
			HBM_GET_LANGUAGE(HBM_LANG_TRAD_CHINESE,		zh-Hant,	Chinese Traditional)
			#else
			HBM_GET_LANGUAGE(HBM_LANG_SIMP_CHINESE,		zh_Hans,	Chinese Simplified)
			HBM_GET_LANGUAGE(HBM_LANG_TRAD_CHINESE,		zh_Hant,	Chinese Traditional)
			#endif

			HBM_GET_LANGUAGE(HBM_LANG_KOREAN,			ko,			Korean)

			#ifdef HBM_ENABLE_ROMFS
			HBM_GET_LANGUAGE(HBM_LANG_PT_PORTUGUESE,	pt-PT,		Português)
			HBM_GET_LANGUAGE(HBM_LANG_BR_PORTUGUESE,	pt-BR,		Português do Brasil)
			#else
			HBM_GET_LANGUAGE(HBM_LANG_PT_PORTUGUESE,	pt_PT,		Português)
			HBM_GET_LANGUAGE(HBM_LANG_BR_PORTUGUESE,	pt_BR,		Português do Brasil)
			#endif

			HBM_GET_LANGUAGE(HBM_LANG_RUSSIAN,			ru,			Russian)
			HBM_GET_LANGUAGE(HBM_LANG_UKRAINIAN,		uk,			Ukrainian)
			HBM_GET_LANGUAGE(HBM_LANG_POLISH,			pl,			Polski)
			HBM_GET_LANGUAGE(HBM_LANG_SWEDISH,			sv,			Svenska)
			// HBM_GET_LANGUAGE(HBM_LANG_DANISH,			da,			Dansk)
			// HBM_GET_LANGUAGE(HBM_LANG_FINNISH,			fi,			Suomi)
			// HBM_GET_LANGUAGE(HBM_LANG_NORWEGIAN,		no,			Norsk bokmaal)
			// HBM_GET_LANGUAGE(HBM_LANG_GREEK,			el,			Greek)
			HBM_GET_LANGUAGE(HBM_LANG_TURKISH,			tr,			Türkçe)
			HBM_GET_LANGUAGE(HBM_LANG_WELSH,			cy,			Cymraeg)
			HBM_GET_LANGUAGE(HBM_LANG_CATALAN,			ca,			Català)
			HBM_GET_LANGUAGE(HBM_LANG_OKINAWAN,			ryu,		Uchinaaguchi)
		}
	}

	return true;
}

#undef HBM_GET_LANGUAGE
#ifdef HBM_ENABLE_ROMFS
#undef HBM_LANGUAGE_ROMFS_PATH
#else
#undef HBM_LANGUAGE_EXTERN
#undef HBM_LANGUAGE_EXTERN_END
#endif

#ifdef TRACK_UNIQUE_MSGIDS
static const char* unique_msgids[4096];
static int msgids_count = 0;

static void add_msgid(const char* msgid) {
    for (int i = 0; i < msgids_count; i++) {
        if (!strcmp(msgid, unique_msgids[i])) {
            return;
        }
    }

#ifdef WII_NETTRACE
    net_print_string(NULL, 0, "adding: %d, %s\n", msgids_count, msgid);
#endif

    unique_msgids[msgids_count++] = strdup(msgid);
}

void dump_unique_msgids() {
    for (int i = 0; i < msgids_count; i++) {
#ifdef WII_NETTRACE
        net_print_string(NULL, 0, "msgid \"%s\"\n", unique_msgids[i]);
        net_print_string(NULL, 0, "msgstr \"\"\n\n");
#endif
    }
}
#endif

const char* HBM_gettextmsg(const char* msgid) {
#ifdef TRACK_UNIQUE_MSGIDS
    add_msgid(msgid);
#endif

    map<string, string>::iterator iter = msgmap.find(msgid);
    if (iter != msgmap.end()) {
        const char* str = iter->second.c_str();
#if 0
#ifdef WII_NETTRACE
  char buf[512] = "";
  char *pbuf = buf;
  for( int i = 0; i < strlen(str) && ((pbuf - buf) < (sizeof(buf) - 4)); i++ )
  {
    sprintf( pbuf, "%x ", (u8)str[i] );
    pbuf += 3;
  }
  net_print_string( NULL, 0, "gettextmsg: %s, %s\n", str, buf );
#endif
#endif
        return str;
    }
    return msgid;
}
