/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef langH
#define langH

#define Maccel 96

extern TCHAR lang[64];

extern ACCEL accel[Maccel];
extern int Naccel;

TCHAR *lng(int i, char *s);
WCHAR *lng(int i, WCHAR *s);
void initLang();
void cleanLang();
int setLang(int cmd);
HMENU loadMenu(TCHAR *name, int *subId);
void loadMenu(HWND hwnd, TCHAR *name, int *subId);
void changeDialog(HWND &wnd, int x, int y, LPCTSTR dlgTempl, DLGPROC dlgProc);
void setDlgTexts(HWND hDlg);
void setDlgTexts(HWND hDlg, int id);
void getExeDir(TCHAR *fn, const TCHAR *e);
TCHAR *cutPath(TCHAR *s);
void printKey(TCHAR *s, ACCEL *a);


extern void langChanged();
extern void msg(char *text, ...);
extern void msglng(int id, char *text, ...);
extern HINSTANCE inst;

#define sizeA(A) (sizeof(A)/sizeof(*A))
#define endA(a) (a+(sizeof(a)/sizeof(*a)))

#endif
