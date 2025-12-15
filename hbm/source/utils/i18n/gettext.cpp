#include <gctypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <string>
using namespace std;

#include "HBM.h"

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

static bool LoadLanguage(char* langfile, size_t size) {
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

static bool LoadLanguageROMFS(const char* string) {
	HBMRomfsFile file(string);
	char *langfile = (char *)file.Data();

    if (!langfile) {
        gettextCleanUp();
        return true;
    }

    char line[200];
    char* lastID = NULL;

    gettextCleanUp();

	while (langfile && langfile < langfile + file.Size()) {
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

static int currentLanguage = -1;

int HBM_GetCurrentLanguage() {
	return currentLanguage;
}

bool HBM_LoadLanguage(int lang) {
	if (lang != currentLanguage) {
		bool isSystem = false;

		if (lang > 17 || lang < -1) lang = -1;
		currentLanguage = lang;

		if (lang == -1) {
			isSystem = true;
			lang = CONF_GetLanguage();
			HBM_ConsolePrintf("Language set to System");
		}

		switch (lang) {
			default:
			case CONF_LANG_ENGLISH:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (English)", currentLanguage);
				// return LoadLanguage((char *)HBM_en_lang, (char *)HBM_en_lang + HBM_en_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/en.lang");

			case CONF_LANG_JAPANESE:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Japanese)", currentLanguage);
				// if (!isSystem) HBM_ConsolePrintf("言語は%d（日本語）に設定されました。", currentLanguage);
				return LoadLanguageROMFS("romfs:/hbm/text/ja.lang");

			case CONF_LANG_GERMAN:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (German)", currentLanguage);
				return LoadLanguageROMFS("romfs:/hbm/text/de.lang");

			case CONF_LANG_FRENCH:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (French)", currentLanguage);
				// return LoadLanguage((char *)HBM_fr_lang, (char *)HBM_fr_lang + HBM_fr_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/fr.lang");

			case CONF_LANG_SPANISH:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Spanish)", currentLanguage);
				// return LoadLanguage((char *)HBM_es_lang, (char *)HBM_es_lang + HBM_es_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/es.lang");

			case CONF_LANG_ITALIAN:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Italian)", currentLanguage);
				// return LoadLanguage((char *)HBM_it_lang, (char *)HBM_it_lang + HBM_it_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/it.lang");

			case CONF_LANG_DUTCH:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Dutch)", currentLanguage);
				// return LoadLanguage((char *)HBM_nl_lang, (char *)HBM_nl_lang + HBM_nl_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/nl.lang");

			case CONF_LANG_SIMP_CHINESE:
			case CONF_LANG_TRAD_CHINESE:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Chinese)", currentLanguage);
				// return LoadLanguage((char *)HBM_zh_lang, (char *)HBM_zh_lang + HBM_zh_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/zh.lang");

			case CONF_LANG_KOREAN:
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Korean)", currentLanguage);
				// return LoadLanguage((char *)HBM_ko_lang, (char *)HBM_ko_lang + HBM_ko_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/ko.lang");

			case 10: // CONF_LANG_PT_PORTUGUESE
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Portuguese)", currentLanguage);
				// return LoadLanguage((char *)HBM_pt_PT_lang, (char *)HBM_pt_PT_lang + HBM_pt_PT_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/pt_PT.lang");

			case 11: // CONF_LANG_BR_PORTUGUESE
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Brazilian Portuguese)", currentLanguage);
				// return LoadLanguage((char *)HBM_pt_BR_lang, (char *)HBM_pt_BR_lang + HBM_pt_BR_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/pt_BR.lang");

			case 12: // CONF_LANG_RUSSIAN
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Russian)", currentLanguage);
				// return LoadLanguage((char *)HBM_ru_lang, (char *)HBM_ru_lang + HBM_ru_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/ru.lang");

			case 13: // CONF_LANG_UKRAINIAN
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Ukrainian)", currentLanguage);
				// return LoadLanguage((char *)HBM_uk_lang, (char *)HBM_uk_lang + HBM_uk_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/uk.lang");

			case 14: // CONF_LANG_POLISH
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Polish)", currentLanguage);
				// return LoadLanguage((char *)HBM_pl_lang, (char *)HBM_pl_lang + HBM_pl_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/pl.lang");

			case 15: // CONF_LANG_TURKISH
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Turkish)", currentLanguage);
				// return LoadLanguage((char *)HBM_tr_lang, (char *)HBM_tr_lang + HBM_tr_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/tr.lang");

			case 16: // CONF_LANG_SWEDISH
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Swedish)", currentLanguage);
				// return LoadLanguage((char *)HBM_sv_lang, (char *)HBM_sv_lang + HBM_sv_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/sv.lang");

			case 17: // CONF_LANG_CATALAN
				if (!isSystem) HBM_ConsolePrintf("Language set to %d (Catalan)", currentLanguage);
				// return LoadLanguage((char *)HBM_ca_lang, (char *)HBM_ca_lang + HBM_ca_lang_size);
				return LoadLanguageROMFS("romfs:/hbm/text/ca.lang");
		}
	}

	return false;
}

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
