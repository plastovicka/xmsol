/*
  (C) 2004-2014  Petr Lastovicka

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License.
*/
#include "hdr.h"
#include "lang.h"
/*
Usage:

menu:               submenu "Language", item 29999
registry:           item "language", value lang[64]
WinMain:            readini(); initLang(); ...; langChanded();
case WM_COMMAND:    setLang(cmd);
case WM_INITDIALOG: setDlgTexts(hDlg,titleId);
void langChanged(): LoadMenu, file filters, invalidate, modeless dialogs

ID ranges:
0    reserved for standard dialog buttons
20   window titles
100  commands
400  submenus and popup menus
500  file filters
510  dialog controls
650  column names
700  common error messages
800  application messages
1100 tooltips
1500 end
*/

#ifndef MAXLNGSTR
 #define MAXLNGSTR 1500 //maximum string ID
#endif

#ifndef MAXLNG_PARALLEL
 #define MAXLNG_PARALLEL 16
#endif

const int MAXLANG=60;
TCHAR lang[64];            //name of the current language

static TCHAR *langFile;     //file content (\n are replaced by \0)
static TCHAR *lngstr[MAXLNGSTR];//pointers to lines in the langFile
static TCHAR *lngNames[MAXLANG+1];//names of all foundlanguages
//-------------------------------------------------------------------------

TCHAR *lng(int i, char *s)
{
  if(i>=0 && i<sizeA(lngstr) && lngstr[i]) return lngstr[i];
#ifdef UNICODE
  if(!s) return 0;
  static TCHAR *recycl[MAXLNG_PARALLEL];
  static int ind;
  int len=strlen(s)+1;
  TCHAR *w= new TCHAR[len];
  MultiByteToWideChar(CP_ACP, 0, s, -1, w, len);
  delete[] recycl[ind];
  recycl[ind]=w;
  ind++;
  if(ind==sizeA(recycl)) ind=0;
  return w;
#else
  return s;
#endif
}

#ifdef UNICODE
WCHAR *lng(int i, WCHAR *s)
{
  if(i>=0 && i<sizeA(lngstr) && lngstr[i]) return lngstr[i];
  return s;
}
#endif

//return pointer to the file name after the path
TCHAR *cutPath(TCHAR *s)
{
 TCHAR *t;
 t=s+lstrlen(s);
 while(t>=s && *t!='\\' && *t!='/') t--;
 t++;
 return t;
}

//write working dir to fn and append string e
void getExeDir(TCHAR *fn, TCHAR *e)
{
 GetModuleFileName(0,fn,192);
 lstrcpy(cutPath(fn), e);
}
//-------------------------------------------------------------------------
static BOOL CALLBACK enumControls(HWND hwnd,LPARAM )
{
 int i=GetDlgCtrlID(hwnd);
 if((i>=300 && i<sizeA(lngstr) || i<11 && i>0) && lngstr[i]){
   SetWindowText(hwnd,lngstr[i]);
 }
 return TRUE;
}

void setDlgTexts(HWND hDlg)
{
 EnumChildWindows(hDlg,(WNDENUMPROC)enumControls,0);
}

void setDlgTexts(HWND hDlg, int id)
{
 TCHAR *s=lng(id,(char*)0);
 if(s) SetWindowText(hDlg,s);
 setDlgTexts(hDlg);
}

//reload modeless dialog or create a new dialog at the position x,y
void changeDialog(HWND &wnd, int x,int y,LPCTSTR dlgTempl, DLGPROC dlgProc)
{
 HWND a,w;

 a=GetActiveWindow();
 w=CreateDialog(inst,dlgTempl,0,dlgProc);
 if(wnd){
   RECT rc;
   GetWindowRect(wnd,&rc);
   MoveWindow(w,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
   if(IsWindowVisible(wnd)) ShowWindow(w,SW_SHOW);
   DestroyWindow(wnd);
 }else{
   SetWindowPos(w,0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 }
 wnd=w;
 if(a) SetActiveWindow(a);
}

//-------------------------------------------------------------------------
void printKey(TCHAR *s, ACCEL *a)
{
  *s=0;
  if(a->key==0) return;
  if(a->fVirt&FCONTROL) lstrcat(s,lng(97,"Ctrl+"));
  if(a->fVirt&FSHIFT) lstrcat(s,lng(98,"Shift+"));
  if(a->fVirt&FALT) lstrcat(s,lng(99,"Alt+"));
  s+=lstrlen(s);
  if((a->fVirt&FVIRTKEY) && (a->key<'0' || a->key>'9')){
   UINT scan=MapVirtualKey(a->key,0);
   if(a->key==VK_DIVIDE){
     lstrcpy(s,TEXT("Num /"));
   }else{
     if(a->key>0x20 && a->key<0x2f || a->key==VK_NUMLOCK){
       scan+=256; //Ins,Del,Home,End,PgUp,PgDw,Left,Right,Up,Down
     }
     GetKeyNameText(scan<<16,s,16);
   }
  }else{
   *s++= (TCHAR)a->key;
   *s=0;
  }
}
//-------------------------------------------------------------------------
static int *subPtr;

//change names in the menu h, recursively
static void fillPopup(HMENU h)
{
 int i,id,j;
 TCHAR *s;
 HMENU sub;
 ACCEL *a;
 MENUITEMINFO mii;
 TCHAR buf[128];

 for(i=GetMenuItemCount(h)-1; i>=0; i--){
   id=GetMenuItemID(h,i);
   if(id==29999){
     for(j=0; lngNames[j]; j++){
       InsertMenu(h,0xFFFFFFFF,
         MF_BYPOSITION|(lstrcmpi(lngNames[j],lang)?0:MF_CHECKED),
         30000+j,lngNames[j]);
     }
     DeleteMenu(h,0,MF_BYPOSITION);
   }else{
     if(id<0 || id>=0xffffffff){
       sub=GetSubMenu(h,i);
       if(sub){
         id=*subPtr++;
         fillPopup(sub);
       }
     }
     mii.cbSize=sizeof(MENUITEMINFO);
     mii.fMask=MIIM_TYPE;
     mii.dwTypeData=buf;
     mii.cch= sizeA(buf);
     GetMenuItemInfo(h,i,TRUE,&mii);
     if(mii.fType&MFT_SEPARATOR) continue;
     s=lng(id,(char*)0);
     if(s) lstrcpyn(buf,s,sizeA(buf)-32);
     for(a=accel+Naccel-1;  a>=accel; a--){
       if(a->cmd==id){
         s=buf+lstrlen(buf);
         *s++='\t';
         printKey(s,a);
         break;
       }
     }
     mii.fMask=MIIM_TYPE|MIIM_STATE;
     mii.fType=MFT_STRING;
     mii.fState=MFS_ENABLED;
     mii.dwTypeData=buf;
     mii.cch= (UINT) lstrlen(buf);
     SetMenuItemInfo(h,i,TRUE,&mii);
   }
 }
}

//load menu from resources
//subId are id numbers of names for submenus
HMENU loadMenu(TCHAR *name, int *subId)
{
 HMENU hMenu= LoadMenu(inst,name);
 subPtr=subId;
 fillPopup(hMenu);
 return hMenu;
}

void loadMenu(HWND hwnd, TCHAR *name, int *subId)
{
 if(!hwnd) return;
 HMENU m= GetMenu(hwnd);
 SetMenu(hwnd,loadMenu(name,subId));
 DestroyMenu(m);
}
//-------------------------------------------------------------------------
static void parseLng()
{
 TCHAR *s,*d,*e;
 int id,err=0,line=1;

 for(s=langFile; *s; s++){
   if(*s==';' || *s=='#' || *s=='\n' || *s=='\r'){
     //comment
   }else{
     id=(int)_tcstol(s,&e,10);
     if(s==e){
       if(!err) msglng(755,"Error in %s\nLine %d", lang,line);
       err++;
     }else if(id<0 || id>=sizeA(lngstr)){
       if(!err) msglng(756,"Error in %s\nMessage number %d is too big", lang,id);
       err++;
     }else if(lngstr[id]){
       if(!err) msglng(757,"Error in %s\nDuplicated number %d", lang,id);
       err++;
     }else{
       s=e;
       while(*s==' ' || *s=='\t') s++;
       if(*s=='=') s++;
       lngstr[id]=s;
     }
   }
   for(d=s; *s!='\n' && *s!='\r'; s++){
     if(*s=='\\'){
       s++;
       if(*s=='\r'){
         line++;
         if(s[1]=='\n') s++;
         continue;
       }else if(*s=='\n'){
         line++;
         continue;
       }else if(*s=='0'){
         *s='\0';
       }else if(*s=='n'){
         *s='\n';
       }else if(*s=='r'){
         *s='\r';
       }else if(*s=='t'){
         *s='\t';
       }
     }
     *d++=*s;
   }
   if(*s!='\r' || s[1]!='\n') line++;
   *d='\0';
 }
}
//-------------------------------------------------------------------------
void scanLangDir()
{
 int n;
 HANDLE h;
 WIN32_FIND_DATA fd;
 TCHAR buf[256];

 lngNames[0]=TEXT("English");
 getExeDir(buf,TEXT("language\\*.lng"));
 h = FindFirstFile(buf,&fd);
 if(h!=INVALID_HANDLE_VALUE){
  n=1;
  do{
   if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
     int len= (int) lstrlen(fd.cFileName)-4;
     if(len>0){
       TCHAR *s= new TCHAR[len+1];
       memcpy(s,fd.cFileName,len*sizeof(TCHAR));
       s[len]='\0';
       lngNames[n++]=s;
     }
   }
  }while(FindNextFile(h,&fd) && n<MAXLANG);
  FindClose(h);
 }
}
//-------------------------------------------------------------------------
static void loadLang()
{
 memset(lngstr,0,sizeof(lngstr));
 TCHAR buf[256];
 GetModuleFileName(0,buf,sizeA(buf)-lstrlen(lang)-14);
 lstrcpy(cutPath(buf), TEXT("language\\"));
 TCHAR *fn= buf + lstrlen(buf);
 lstrcpy(fn,lang);
 lstrcat(buf,TEXT(".lng"));
 HANDLE f=CreateFile(buf,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
 if(f!=INVALID_HANDLE_VALUE){
   DWORD len=GetFileSize(f,0);
   if(len>10000000){
     msglng(753,"File %s is too long",fn);
   }else{
     delete[] langFile;
     char *langFileA= new char[len+3];
     DWORD r;
     ReadFile(f,langFileA,len,&r,0);
     if(r<len){
       msglng(754,"Error reading file %s",fn);
     }else{
#ifdef UNICODE
       langFile= new TCHAR[len+3];
       MultiByteToWideChar(CP_ACP, 0, langFileA, len, langFile, len);
       delete[] langFileA;
#else
       langFile= langFileA;
#endif
       langFile[len]='\n';
       langFile[len+1]='\n';
       langFile[len+2]='\0';
       parseLng();
     }
   }
   CloseHandle(f);
 }
}
//---------------------------------------------------------------------------
int setLang(int cmd)
{
 if(cmd>=30000 && cmd<30000+MAXLANG && lngNames[cmd-30000]){
   lstrcpy(lang,lngNames[cmd-30000]);
   loadLang();
   langChanged();
   return 1;
 }
 return 0;
}
//---------------------------------------------------------------------------
void initLang()
{
	scanLangDir();
	if(!lang[0]){
		//language detection
		const char* s;
		switch(PRIMARYLANGID(GetUserDefaultLangID()))
		{
			case LANG_CZECH: s="Èesky"; break;
			case LANG_FRENCH: s="French"; break;
			case LANG_POLISH: s="Polski"; break;
			case LANG_ITALIAN: s= "Italiano"; break;
			default: s="English"; break;
		}
		strcpy(lang, s);
	}
	loadLang();
}
//---------------------------------------------------------------------------
