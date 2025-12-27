#ifndef _HBM_GETMSGTEXT_H_
#define _HBM_GETMSGTEXT_H_

bool HBM_LoadLanguage(int lang);
int HBM_GetCurrentLanguage();
const char* HBM_gettextmsg(const char* msg);

#endif /* _GETTEXT_H_ */
