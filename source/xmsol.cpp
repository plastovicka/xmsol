/*
	(C) Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "hdr.h"
#include "xmsol.h"

#if defined(_DEBUG) && _MSC_VER==1700
//#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"
#endif

#pragma comment(lib, "version.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "htmlhelp.lib")

//------------------------------------------------------------------

#define Mcolumn 19
char *colNames[Mcolumn]=
{"Game name", "Played", "Won", "Percent", "Score",
	"Time", "Moves", "Cards", "Stocks", "Sequence rule",
	"Suit rule", "Pair", "Game name", "Last play", "Score won",
	"Time won", "Moves won", "Lost", "Group moves"};
short colWidth[Mcolumn]=
{190, 60, 60, 60, 60, 60, 60, 60, 60, 70, 70, 70, 190, 90, 60, 60, 60, 60, 80};
int colOrder[Mcolumn]=
{0, 1, 2, 17, 3, 5, 6, 4, 15, 16, 14, 13, 11, 7, 8, 18, 9, 10, 12};
bool colPlayed[Mcolumn]=
{0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0};
TstatMember colStat[Mcolumn]=
{0, &Tstat::played, &Tstat::won, &Tstat::percent, &Tstat::score, &Tstat::time, &Tstat::moves, 0, 0, 0, 0, 0, 0, &Tstat::date, &Tstat::scoreWon, &Tstat::timeWon, &Tstat::movesWon, &Tstat::lost, 0};
TgameMember colGame[Mcolumn]=
{0, 0, 0, 0, 0, 0, 0, &Tgame::cards, &Tgame::stocks, &Tgame::ruleSeq, &Tgame::ruleSuit, &Tgame::pair, 0, 0, 0, 0, 0, 0, &Tgame::ruleGroup};

char *toolNames[]={
	"Select game", "New game", "Change player", "",
	"Undo all", "Undo", "Redo", "Redo all", "",
	"Fast forward", "",
	"Options", "Rules"
};

const int Ntool=13;
TBBUTTON tbb[]={
		{6, ID_LIST, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{4, ID_NEWGAME, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{3, ID_PLAYER, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{0, 0, 0, TBSTYLE_SEP, {0}, 0},
		{8, ID_UNDO_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{1, ID_UNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{0, ID_REDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{7, ID_REDO_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{0, 0, 0, TBSTYLE_SEP, {0}, 0},
		{9, ID_FAST_FORWARD, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{0, 0, 0, TBSTYLE_SEP, {0}, 0},
		{2, ID_OPTIONS, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
		{5, ID_HELP_RULES, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0},
};

//------------------------------------------------------------------

int
corners=15,
 cornersW,
 globalAutoplay=1,
 optionsPage=0,
 halftone=0,
 borderW=15,
 borderH=10,
 flipx=0,
 flipy=0,
 sequenceDy=20,
 sequenceDx=18,
 seqHiddenDy=7,
 seqHiddenDx=6,
 toolBarVisible=1,
 statusBarVisible=1,
 maximized=1,
 gameNum,
 cardW, cardH,
 bmpCardW, bmpCardH,
 bmpBackW, bmpBackH,
 bmpCellW, bmpCellH,
 stretchW, stretchH,
 notdraw,
 speedAccel=1,
 suspendTime,
 currentPile,
 statusH=18,
 toolH=50,
 mainLeft=0, mainTop=0, mainW=800, mainH=570,
 multInst=0,
 noMsg=0,
 sortedCol=0,
 descending=-1,
 scrW, scrH,
 resizing=0,
 width, height,
 scrollPos,
 noSort,
 Naccel,
 SCORE_UNDO=100,
 SCORE_WRONG_MOVE=20,
 SCORE_REDEAL=20,
 SCORE_UNHIDE=10,
 SCORE_FOUNDATION=100,
 SCORE_BONUS=5000,
 SCORE_TIME=4,
 RoundRectRgnExtra,
 Ntoolbar,
 gameListX=150, gameListY=90, //game list dialog window size and position 
 gameListW, gameListH;

bool
delreg=false,
 mustLoadRules=false,
 configToRegistery,
 paused;

unsigned seedGlobal;
TCHAR gameName[128];
Darray<Tgame*> games;
Darray<TCHAR*> menuCards, menuBack, menuBkgnd, menuCell;
Tboard board, preview;
Tanim animation[ANIM_MAX]={{0, 0}, {100, 1}, {200, 1}, {150, 1}, {50, 1}};

TCHAR *title=_T("XM Solitaire");

HWND hWin, dlgWin, listBox, toolbar, statusbar, rulesWnd;
HACCEL haccel;
ACCEL accel[Maccel];
ACCEL dlgKey;
HCURSOR curMove;
HINSTANCE inst;
HFONT hFont=0;
LOGFONT font;
HBRUSH bkgndBrush;
HBITMAP bmpCards, bmpBack, bmpBkgnd, bmpCell;
HDC dcCards, dcBack, dcCell, dcStretch, dcBackStretch, dcCellStretch;

TfileName fnCards, fnBack, fnBkgnd, fnCell, fnTmp, fnIni;

const TCHAR *iniFile=_T("xmsol.ini"), *iniSection=_T("xmsol");
const TCHAR *subkey=_T("Software\\Petr Lastovicka\\xmsol");
struct Treg { char *s; int *i; } regVal[]={
		{"top", &mainTop},
		{"left", &mainLeft},
		{"width", &mainW},
		{"height", &mainH},
		{"optionsPage", &optionsPage},
		{"dx", &sequenceDx},
		{"dy", &sequenceDy},
		{"hdx", &seqHiddenDx},
		{"hdy", &seqHiddenDy},
		{"flipx", &flipx},
		{"flipy", &flipy},
		{"toolbarOn", &toolBarVisible},
		{"statusOn", &statusBarVisible},
		{"maximized", &maximized},
		{"autoplay", &globalAutoplay},
		{"borderW", &borderW},
		{"borderH", &borderH},
		{"scoreUndo", &SCORE_UNDO},
		{"scoreWrong", &SCORE_WRONG_MOVE},
		{"scoreRedeal", &SCORE_REDEAL},
		{"scoreUnhide", &SCORE_UNHIDE},
		{"scoreDone", &SCORE_FOUNDATION},
		{"scoreTime", &SCORE_TIME},
		{"scoreBonus", &SCORE_BONUS},
		{"animAuto", &animation[ANIM_AUTOPLAY].enabled},
		{"speedAuto", &animation[ANIM_AUTOPLAY].speed},
		{"animUndo", &animation[ANIM_UNDO].enabled},
		{"speedUndo", &animation[ANIM_UNDO].speed},
		{"animRclick", &animation[ANIM_RIGHTCLICK].enabled},
		{"speedRclick", &animation[ANIM_RIGHTCLICK].speed},
		{"animStock", &animation[ANIM_STOCK].enabled},
		{"speedStock", &animation[ANIM_STOCK].speed},
		{"gameNum", &gameNum},
		{"corners", &corners},
		{"halftone", &halftone},
		{"Naccel", &Naccel},
		{"Ntoolbar", &Ntoolbar},
		{"gameListX", &gameListX},
		{"gameListY", &gameListY},
		{"gameListW", &gameListW},
		{"gameListH", &gameListH},
};

struct Tregs { char *s; TCHAR *i; DWORD n; } regValS[]={
		{"language", lang, sizeof(lang)},
		{"game", gameName, sizeof(gameName)},
		{"cards", fnCards, sizeof(fnCards)},
		{"back", fnBack, sizeof(fnBack)},
		{"bkgnd", fnBkgnd, sizeof(fnBkgnd)},
		{"cell", fnCell, sizeof(fnCell)},
		{"player", playerFile.name, sizeof(playerFile.name)},
};

struct Tregb { char *s; void *i; DWORD n; } regValB[]={
		{"keys", accel, sizeof(accel)}, //must be the first item of regValB
		//{"font", &font, sizeof(LOGFONT)},
		{"colWidth", colWidth, sizeof(colWidth)},
		{"colOrder", colOrder, sizeof(colOrder)},
};

OPENFILENAME userOfn={
	OPENFILENAME_SIZE_VERSION_400, 0, 0, 0, 0, 0, 1,
	fnTmp, sizeA(fnTmp),
	0, 0, 0, 0, 0, 0, 0, _T("XOL"), 0, 0, 0
};

typedef WINUSERAPI BOOL(WINAPI *pIsHungAppWindow)(HWND hwnd);
pIsHungAppWindow isHungAppWindow;

//------------------------------------------------------------------

//append formatted text to a buffer
int dtprintf(Darray<WCHAR> &buf, WCHAR *fmt, ...)
{
	int n, len;
	va_list v;
	va_start(v, fmt);
	if(buf.len>0 && buf[buf.len-1]==0) buf--;
	for(len=max(buf.capacity-buf.len, 128);; len*=2){
		n=_vsnwprintf(buf+=len, len, fmt, v);
		if(n>=0 && n<len){
			buf -= len-n;
			break; //OK
		}
		buf-=len;
		if(len>10000000) break; //error
	}
	*buf++=0;
	va_end(v);
	return n;
}

int dtprintf(Darray<char> &buf, char *fmt, ...)
{
	int n, len;
	va_list v;
	va_start(v, fmt);
	if(buf.len>0 && buf[buf.len-1]==0) buf--;
	for(len=max(buf.capacity-buf.len, 128);; len*=2){
		n=_vsnprintf(buf+=len, len, fmt, v);
		if(n>=0 && n<len){
			buf -= len-n;
			break; //OK
		}
		buf-=len;
		if(len>10000000) break; //error
	}
	*buf++=0;
	va_end(v);
	return n;
}

void pause(bool b)
{
	paused=b;
	invalidate();
}

void dialogBox(int id, DLGPROC proc)
{
	pause(true);
	DialogBox(inst, MAKEINTRESOURCE(id), hWin, proc);
	pause(false);
}

//show message box
int vmsg(int id, char *text, int btn, va_list v)
{
	if(noMsg && btn==MB_OK) return -1;
	static const int Mbuf=4096;
	TCHAR *buf=(TCHAR*)_alloca(2*Mbuf);
	_vsntprintf(buf, Mbuf, lng(id, text), v);
	buf[Mbuf-1]=0;
	bool b=paused;
	pause(true);
	int result=MessageBox(dlgWin ? dlgWin : hWin, buf, title, btn);
	pause(b);
	return result;
}

void msglng(int id, char *text, ...)
{
	va_list ap;
	va_start(ap, text);
	vmsg(id, text, MB_OK, ap);
	va_end(ap);
}

// Create a dialog box with the standard "OK" button
void msg(char *text, ...)
{
	va_list ap;
	va_start(ap, text);
	vmsg(-1, text, MB_OK, ap);
	va_end(ap);
}

//display text in the status bar
void status(int i, TCHAR *txt, ...)
{
	TCHAR buf[256];

	if(notdraw) return;
	va_list ap;
	va_start(ap, txt);
	_vstprintf(buf, txt, ap);
	SendMessage(statusbar, SB_SETTEXT, i, (LPARAM)buf);
	va_end(ap);
}

// Returns pointer to the extention of a file
TCHAR *cutExt(TCHAR *fn)
{
	for(TCHAR *s= fn+lstrlen(fn); s>=fn; s--){
		if(*s=='.') return s;
	}
	return 0;
}

void statusPlayer()
{
	status(STATUS_MSG, _T("%s"), cutPath(playerFile.name));
}

//open or create player profile
void openUser()
{
	userOfn.hwndOwner= hWin;
	userOfn.Flags= OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	static TfileName fnDir;
	_tcscpy(fnDir, playerFile.name);
	*cutPath(fnDir)=0;
	userOfn.lpstrInitialDir=fnDir;
	*userOfn.lpstrFile=0;
	pause(true);
	BOOL result= GetOpenFileName(&userOfn);
	pause(false);
	if(result){
		if(GetFileAttributes(userOfn.lpstrFile)==INVALID_FILE_ATTRIBUTES){
			playerFile.create(userOfn.lpstrFile);
		}
		else{
			_tcscpy(playerFile.name, userOfn.lpstrFile);
			playerFile.open();
			playerFile.close();
		}
		statusPlayer();
	}
}

// Fill "s" with a filename from folder "mask", based on an item name found in the menu
void getMenuItem(TCHAR *s, TCHAR *mask, int id)
{
	//get file name from menu
	_tcscpy(s, mask);
	TCHAR* name = _tcschr(s, 0);
	GetMenuString(GetMenu(hWin), id, name, 100, MF_BYCOMMAND);
	TCHAR* ext = _tcschr(name, 0);
	_tcscpy(ext, _T(".*"));

	//find file extension
	TfileName path;
	getExeDir(path, s);
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(path, &fd);
	if(h!=INVALID_HANDLE_VALUE){
		TCHAR *t=cutExt(fd.cFileName);
		if(t) _tcscpy(ext, t);
		FindClose(h);
	}

	invalidate();
}

int __cdecl strPtrCmp(const void *a, const void *b)
{
	return _tcscmp(*(TCHAR**)a, *(TCHAR**)b);
}

//create menu items from folder content (only images)
void menuFromDir(TCHAR *mask, int id, Darray<TCHAR*> &array)
{
	int i;
	HANDLE h;
	HMENU menu;
	TCHAR *t;
	WIN32_FIND_DATA fd;
	TfileName buf;

	deleteDarray(array);
	menu=GetMenu(hWin);
	getExeDir(buf, mask);
	h = FindFirstFile(buf, &fd);
	if(h!=INVALID_HANDLE_VALUE){
		do{
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && isImage(fd.cFileName)){
				*array++= t= dupStr(fd.cFileName);
				t=cutExt(t);
				if(t) *t=0;
			}
		} while(FindNextFile(h, &fd));
		FindClose(h);
	}
	qsort(array.array, array.len, sizeof(TCHAR*), strPtrCmp);
	for(i=0; i<array.len; i++){
		InsertMenu(menu, id, MF_BYCOMMAND, id+i+1, array[i]);
	}
	DeleteMenu(menu, id, MF_BYCOMMAND);
}

//------------------------------------------------------------------
//x,y is relative position from 0 to 1000
//returns card position (top left corner) in the window client area
int scaleX(int x)
{
	if(flipx) x=1000-x;
	return (width-cardW-borderW*2)*x/1000+borderW;
}

int scaleY(int y)
{
	if(flipy) y=1000-y;
	int panelsH=0;
	if(statusBarVisible) panelsH+=statusH;
	if(toolBarVisible) panelsH+=toolH;
	return (height-cardH-borderH*2-panelsH)*y/1000 +borderH+(toolBarVisible ? toolH : 0);
}

//calculate card size according to the window size and repaint window
void calcCardW()
{
	if(!bmpCardH || !bmpCardW){
		//file not found
		cardW=71;
		cardH=96;
	}
	else{
		int w, w1, h, h1;
		w=71*width/792;
		h=96*height/518;
		h1=w*bmpCardH/bmpCardW;
		w1=h*bmpCardW/bmpCardH;
		//preserve aspect ratio
		if(w>w1){
			cardW=w1;
			cardH=h;
		}
		else{
			cardW=w;
			cardH=h1;
		}
	}
	cornersW= (cardW+cardH)*corners/200;
	invalidate();
}

//resize all images
void stretchCards(HDC dc)
{
	//cards front images
	if(!dcStretch) dcStretch=CreateCompatibleDC(dc);
	HBITMAP b=CreateCompatibleBitmap(dc, cardW*13, cardH*4);
	DeleteObject(SelectObject(dcStretch, b));
	SetStretchBltMode(dcStretch, (cardW<40 || halftone) ? HALFTONE : COLORONCOLOR);
	StretchBlt(dcStretch, 0, 0, cardW*13, cardH*4,
		dcCards, 0, 0, bmpCardW*13, bmpCardH*4, SRCCOPY);

	//card back
	if(!dcBackStretch) dcBackStretch=CreateCompatibleDC(dc);
	b=CreateCompatibleBitmap(dc, cardW, cardH);
	DeleteObject(SelectObject(dcBackStretch, b));
	SetStretchBltMode(dcBackStretch, HALFTONE);
	StretchBlt(dcBackStretch, 0, 0, cardW, cardH,
		dcBack, 0, 0, bmpBackW, bmpBackH, SRCCOPY);

	//empty cells
	if(!dcCellStretch) dcCellStretch=CreateCompatibleDC(dc);
	b=CreateCompatibleBitmap(dc, cardW*16, cardH);
	DeleteObject(SelectObject(dcCellStretch, b));
	SetStretchBltMode(dcCellStretch, HALFTONE);
	StretchBlt(dcCellStretch, 0, 0, cardW*16, cardH,
		dcCell, 0, 0, bmpCellW, bmpCellH, SRCCOPY);

	stretchW=cardW;
	stretchH=cardH;
}

//get direction and spacing of cards in a pile
void getDxDy(int &dx, int &dy, int &hdx, int &hdy, Tcell *c)
{
	dx=dy=hdx=hdy=0;
	switch(c->dir){
		case DIR_DOWN:
			dy=max(1, sequenceDy);
			hdy=max(1, seqHiddenDy);
			break;
		case DIR_RIGHT:
			dx=max(1, sequenceDx);
			hdx=max(1, seqHiddenDx);
			break;
		case DIR_UP:
			dy=-max(1, sequenceDy);
			hdy=-max(1, seqHiddenDy);
			break;
		case DIR_LEFT:
			dx=-max(1, sequenceDx);
			hdx=-max(1, seqHiddenDx);
			break;
		default:
			return;
	}
	if(flipx) dx=-dx, hdx=-hdx;
	if(flipy) dy=-dy, hdy=-hdy;
	int d=c->spacing;
	dx=dx*d*cardW/7100;
	dy=dy*d*cardH/9600;
	hdx=hdx*d*cardW/7100;
	hdy=hdy*d*cardH/9600;
}

//display card and then exclude it from current clip region
void paintCardClip(HDC dc, int x, int y, Tcard card, HRGN rgnCard, HRGN rgnClip)
{
	OffsetRgn(rgnCard, x, y);
	ExtSelectClipRgn(dc, rgnCard, RGN_AND); //exclude rounded corners

	if(card==0){
		//back side
		BitBlt(dc, x, y, cardW, cardH, dcBackStretch, 0, 0, SRCCOPY);
	}
	else{
		//front side
		BitBlt(dc, x, y, cardW, cardH, dcStretch,
			(CARD_RANK(card)-1)*stretchW,
			CARD_SUIT(card)*stretchH, SRCCOPY);
	}
	// Draw rounded corners
	if(cornersW) RoundRect(dc, x, y, x+cardW, y+cardH, cornersW, cornersW);

	CombineRgn(rgnClip, rgnClip, rgnCard, RGN_DIFF);
	ExtSelectClipRgn(dc, rgnClip, RGN_COPY);
	OffsetRgn(rgnCard, -x, -y);
}

//paint the main window or preview below game list
void Tboard::paintBoard(HDC dc, RECT *rcPaint)
{
	int i, j, j0, x, y, x0, y0, dx, dy, hdx, hdy, n;
	Tcell *c;
	HGDIOBJ oldB, oldP;
	HRGN rgn, rgnClip;

	if(stretchW!=cardW || stretchH!=cardH){
		//resize all images if window size changed
		stretchCards(dc);
	}
	oldB=SelectObject(dc, GetStockObject(NULL_BRUSH));
	oldP=SelectObject(dc, CreatePen(PS_SOLID, 0, GetPixel(dcCards, cardW/2, 0)));
	rgn=CreateRoundRectRgn(0, 0, cardW+RoundRectRgnExtra, cardH+RoundRectRgnExtra, cornersW, cornersW);
	rgnClip=CreateRectRgn(0, 0, width, height);
	bool hideAll = paused && !finished && this!=&preview;

	//dragged cards
	if(dragging){
		x=x0=dragX; y=y0=dragY;
		getDxDy(dx, dy, hdx, hdy, &cells[dragCell]);
		x+=dragCount*dx;
		y+=dragCount*dy;
		for(j=dragCount-1; j>=0; j--){
			x-=dx;
			y-=dy;
			paintCardClip(dc, x, y, cells[dragCell].cards[dragInd+j], rgn, rgnClip);
		}
	}

	//card viewed by middle mouse button
	if(revealing){
		getColumnCoord(x, y, revealCell, revealInd);
		paintCardClip(dc, x, y, cells[revealCell].cards[revealInd], rgn, rgnClip);
	}

	//cards
	for(i=cells.len-1; i>=0; i--){
		c=&cells[i];
		n=c->Ncards;
		if(dragging && i==dragCell) n-=dragCount; //exclude dragged cards
		if(n<=0) continue;
		//calculate top card position
		getDxDy(dx, dy, hdx, hdy, c);
		x=x0=scaleX(c->x);
		y=y0=scaleY(c->y);
		j0=getColumnStart(c);
		for(j=j0; j<n-1; j++){
			if(c->hidden[j]){
				x+=hdx; y+=hdy;
			}
			else{
				x+=dx; y+=dy;
			}
		}
		//paint only cards which are inside clipping region
		if(!((dx>0 ? (x0>=rcPaint->right || x+cardW<rcPaint->left) :
				(x>=rcPaint->right || x0+cardW<rcPaint->left)) ||
				(dy>0 ? (y0>=rcPaint->bottom || y+cardH<rcPaint->top) :
				(y>=rcPaint->bottom || y0+cardH<rcPaint->top)))){
			for(;;){
				paintCardClip(dc, x, y, c->hidden[j] || hideAll ? (Tcard)0 : c->cards[j], rgn, rgnClip);
				if(c->dir==DIR_NO || j==j0) break; //paint only a top card if pile is not tableau
				j--;
				if(c->hidden[j]){
					x-=hdx; y-=hdy;
				}
				else{
					x-=dx; y-=dy;
				}
			}
		}
	}
	DeleteObject(rgnClip);
	DeleteObject(rgn);
	DeleteObject(SelectObject(dc, oldP));

	//empty cells
	for(i=cells.len-1; i>=0; i--){
		c=&cells[i];
		n=c->Ncards;
		if(dragging && i==dragCell) n-=dragCount;
		if(n>0) continue;
		x=x0=scaleX(c->x);
		y=y0=scaleY(c->y);
		if(x>=rcPaint->right || x+cardW<rcPaint->left ||
			y>=rcPaint->bottom || y+cardH<rcPaint->top) continue; //cell is outside painted region
		/*
		SelectObject(dc,GetStockObject(LTGRAY_BRUSH));
		Rectangle(dc,x,y,x+cardW,y+cardH);
		SetBkMode(dc,TRANSPARENT);
		SetTextColor(dc,0x808080);
		*/
		if(c->type==CELL_STOCK && c->redealCount < c->redeal){
			//Redeal button
			BitBlt(dc, x, y, cardW, cardH, dcCellStretch, cardW*15, 0, SRCCOPY);
			/*
			SetTextAlign(dc,TA_CENTER|TA_BASELINE);
			TextOutA(dc,x+cardW/2,y+cardH/2,"Redeal", cardW>50 ? 6:1);
			*/
		}
		else{
			j=(c->inRule&RANK_MASK)>>RANK_ROT;
			if(j==RANK_RANDOM) j=randomRank;
			if(j==RANK_RANDOMPLUS1) j=randomRank%13+1;
			if(j==RANK_SAMEBASE) j=baseRank();
			if(j==RANK_KING4) j=king4();
			if(j>0 && j<=13){
				//cell is restricted to rank
				BitBlt(dc, x, y, cardW, cardH, dcCellStretch, cardW*(j-1), 0, SRCCOPY);
				/*
				TCHAR *c= _T("  A 2 3 4 5 6 7 8 9 10J Q K ") + j*2;
				//SetTextAlign(dc,TA_CENTER|TA_BASELINE);
				//TextOut(dc,x+cardW/2,y+cardH/2,c,2);
				//SetTextAlign(dc,TA_RIGHT|TA_BOTTOM);
				//TextOut(dc,x+cardW-5,y+cardH-5,c,2);
				SetTextAlign(dc,TA_LEFT|TA_TOP);
				int d=cardW/14+1;
				TextOut(dc,x+d,y+d,c,2);
				*/
			}
			else if(j==RANK_NO && (!c->sameRule || (c->sameRule&RANK_MASK)>>RANK_ROT==RANK_NO)){
				//blocked cell
				BitBlt(dc, x, y, cardW, cardH, dcCellStretch, cardW*14, 0, SRCCOPY);
				/*
				int d=cardW/8;
				x+=d; y+=d;
				MoveToEx(dc,x,y,0);
				LineTo(dc,x+cardW-2*d,y+cardH-2*d);
				MoveToEx(dc,x+cardW-2*d,y,0);
				LineTo(dc,x,y+cardH-2*d);
				*/
			}
			else{
				//free cell
				BitBlt(dc, x, y, cardW, cardH, dcCellStretch, cardW*13, 0, SRCCOPY);
			}
		}
		ExcludeClipRect(dc, x0, y0, x0+cardW, y0+cardH);
	}

	//background
	FillRect(dc, rcPaint, bkgndBrush);
	SelectObject(dc, oldB);
}

void invalidate()
{
	RECT rc;
	GetClientRect(hWin, &rc);
	if(toolBarVisible) rc.top=toolH;
	if(statusBarVisible) rc.bottom-=statusH;
	InvalidateRect(hWin, &rc, TRUE);
}

//invalidate pile of n cards at position x,y
void invalidateColumn1(int x, int y, int n, Tcell *cell)
{
	int dx, dy, dummy, dir;
	RECT rc;

	rc.left=x;
	rc.top=y;
	rc.right=rc.left+cardW+1; //+1 is needed because of wine
	rc.bottom=rc.top+cardH+1;
	//we ignore hidden cards because this function is used mainly for animation or dragging
	getDxDy(dx, dy, dummy, dummy, cell);
	dir=cell->dir;
	if(flipx){
		if(dir==DIR_LEFT) dir=DIR_RIGHT;
		if(dir==DIR_RIGHT) dir=DIR_LEFT;
	}
	if(flipy){
		if(dir==DIR_UP) dir=DIR_DOWN;
		if(dir==DIR_DOWN) dir=DIR_UP;
	}
	switch(dir){
		case DIR_DOWN:
			rc.bottom+=dy*n;
			break;
		case DIR_RIGHT:
			rc.right+=dx*n;
			break;
		case DIR_UP:
			rc.top+=dy*n;
			break;
		case DIR_LEFT:
			rc.left+=dx*n;
			break;
	}
	InvalidateRect(hWin, &rc, TRUE);
}

//position of a card at index "ind" in pile "cell"
void Tboard::getColumnCoord(int &x, int &y, int cell, int ind)
{
	int i, dx, dy, hdx, hdy;

	Tcell *c= &cells[cell];
	getDxDy(dx, dy, hdx, hdy, c);
	x=scaleX(c->x);
	y=scaleY(c->y);
	for(i=getColumnStart(c); i<ind; i++){
		if(c->hidden[i]){
			x+=hdx; y+=hdy;
		}
		else{
			x+=dx; y+=dy;
		}
	}
}

int Tboard::getColumnStart(Tcell *c)
{
	return max(0, c->Ncards - c->maxShow);
}

void Tboard::invalidateColumn(int cell, int ind, int n)
{
	int x, y;

	Tcell *c= &cells[cell];
	if(c->maxShow<255) { n+=min(ind,c->maxShow); ind=0; }
	getColumnCoord(x, y, cell, ind);
	invalidateColumn1(x, y, n-1, c);
}

void Sleep2(int ms)
{
	static int d=0;

	MSG mesg;
	if(PeekMessage(&mesg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE)){
		DispatchMessage(&mesg);
	}
	UpdateWindow(hWin);
	if(ms<20){
		d+=ms;
		if(d>20){
			Sleep(20);
			d-=20;
		}
	}
	else{
		amax(ms, 1000);
		Sleep(ms);
	}
}

void Tboard::animate(int dest, int src, int ind, int anim)
{
	int i, k, tick, x0, y0, x1, y1, count;
	Tcell *c;

	if(notdraw || src==dest) return;
	suspendTime=0;
	count=cells[src].Ncards-ind;
	if(count<=0) return;
	k=animation[anim].speed;
	k/=speedAccel;
	if(!animation[anim].enabled){
		if(anim==ANIM_AUTOPLAY) Sleep2(k);
	}
	else{
		tick=10;
		if(isHungAppWindow && isHungAppWindow(hWin)){
			k=1;
			tick=0;
		}
		else{
			k/=tick;
			aminmax(k, 1, 500);
		}
		dragging=true;
		dragCell=src;
		dragInd=ind;
		dragCount=count;
		c=&cells[src];
		getColumnCoord(x0, y0, src, ind);
		dragX=x0;
		dragY=y0;
		getColumnCoord(x1, y1, dest, cells[dest].Ncards);
		for(i=0; i<=k; i++){
			invalidateColumn1(dragX, dragY, count, c);
			dragX=x0+(x1-x0)*i/k;
			dragY=y0+(y1-y0)*i/k;
			invalidateColumn1(dragX, dragY, count, c);
			UpdateWindow(hWin);
			Sleep(tick);
		}
		dragging=false;
		invalidateColumn1(dragX, dragY, count, c);
	}
}

//returns HIWORD= cell index, LOWORD= card index in pile
int Tboard::hitTest(int mx, int my)
{
	int i, j, x, y, dx, dy, hdx, hdy, result, n;
	Tcell *c;

	if(dragging){
		mx+=dragDx+cardW/2;
		my+=dragDy+cardH/2;
	}
	result=-1;

	//test empty cells
	for(i=0; i<cells.len; i++){
		c=&cells[i];
		if(c->Ncards==0){
			x=scaleX(c->x);
			y=scaleY(c->y);
			if(mx>=x && mx<x+cardW && my>=y && my<y+cardH){
				result=(i<<16);
				if(!dragging){ dragDx=x-mx; dragDy=y-my; } //relative mouse position inside dragged card
			}
		}
	}
	//test cards
	for(i=0; i<cells.len; i++){
		c=&cells[i];
		x=scaleX(c->x);
		y=scaleY(c->y);
		getDxDy(dx, dy, hdx, hdy, c);
		n=c->Ncards;
		if(n>0){
			for(j=getColumnStart(c); j<n; j++){
				if(mx>=x && mx<x+cardW && my>=y && my<y+cardH){
					result=j|(i<<16);
					if(!dragging){ dragDx=x-mx; dragDy=y-my; }
				}
				if(c->hidden[j]){
					x+=hdx; y+=hdy;
				}
				else{
					x+=dx; y+=dy;
				}
			}
		}
	}
	currentPile= result>>16;
	return result;
}

int Tboard::hitTest(LPARAM lP)
{
	return hitTest(LOWORD(lP), HIWORD(lP));
}

int Tboard::testDrag()
{
	int i;
	if(!dragCount) return 1;
	for(i=0; i<cells.len; i++){
		if(testMove(i, dragCell, dragInd, 1)) return 0;
	}
	return 2;
}

//hidden top cards are in game Blind Patience
int Tboard::unhide(int cell, int ind)
{
	Tcell *c= &cells[cell];
	if(c->hidden[ind] && ind==c->Ncards-1 && c->onhid!=ONHID_BLOCKED){
		int i= int(c-cells);
		if(move(i, i, MOVE_UNHIDE)){
			autoPlay();
			return 1;
		}
	}
	return 0;
}

//left mouse button
void Tboard::lbuttondown(LPARAM lParam)
{
	if(finished) return;
	dragS=hitTest(lParam);
	if(dragS>=0){
		dragCell=dragS>>16;
		Tcell *c= &cells[dragCell];
		if(c->type==CELL_STOCK){
			stockClick(dragCell);
			autoPlay();
		}
		else{
			dragInd=dragS&0xffff;
			if(!unhide(dragCell, dragInd)){
				dragCount=c->Ncards-dragInd;
				dragX=LOWORD(lParam)+dragDx;
				dragY=HIWORD(lParam)+dragDy;
				if(!testDrag()){
					dragging=true;
					SetCapture(hWin);
				}
			}
		}
	}
}

void Tboard::lbuttonup(LPARAM lParam)
{
	int i;

	if(dragging){
		ReleaseCapture();
		i=hitTest(lParam)>>16;
		dragging=false;
		invalidateColumn1(dragX, dragY, dragCount, &cells[dragCell]);
		invalidateColumn(dragCell, dragInd, dragCount);
		if(move(i, dragCell, dragInd)){
			autoPlay();
		}
	}
}

void Tboard::mousemove(LPARAM lParam)
{
	int i, x, y;

	i=hitTest(lParam)>>16;
	statusNcard();
	if(dragging){
		x=LOWORD(lParam)+dragDx;
		y=HIWORD(lParam)+dragDy;
		invalidateColumn1(dragX, dragY, dragCount, &cells[dragCell]);
		invalidateColumn1(x, y, dragCount, &cells[dragCell]);
		dragX=x;
		dragY=y;
		//change mouse cursor if game rules allow to move cards to target cell
		if(testMove(i, dragCell, dragInd)){
			SetCursor(curMove);
		}
		else{
			SetCursor(LoadCursor(0, IDC_ARROW));
		}
	}
}

//double-click
void Tboard::lbuttondblclk(LPARAM lParam)
{
	if(finished) return;
	dblclkMove(hitTest(lParam)>>16);
	autoPlay();
}

//right mouse button
void Tboard::rbuttondown(LPARAM lParam)
{
	int h, i, j;

	if(finished) return;
	h=hitTest(lParam);
	if(h>=0){
		i=h>>16;
		j=h&0xffff;
		if(!unhide(i, j)){
			if(rightclkMove(i, j)) autoPlay();
			else calcDone();
		}
	}
}

//middle mouse button
void Tboard::mbuttondown(LPARAM lParam)
{
	int t=hitTest(lParam);
	if(t>=0){
		revealCell=t>>16;
		revealInd=t&0xffff;
		Tcell *c= &cells[revealCell];
		if(revealInd<c->Ncards && !c->hidden[revealInd]){
			revealing=true;
			invalidateColumn(revealCell, revealInd, 1);
			SetCapture(hWin);
		}
	}
}

void Tboard::mbuttonup(LPARAM)
{
	if(revealing){
		ReleaseCapture();
		revealing=false;
		invalidateColumn(revealCell, revealInd, 1);
	}
}

void Tboard::saveAtExit()
{
	gameNum=0;
	if(!finished && (moves || redoPos) && game){
		gameNum= number;
		playerFile.savePosition(this);
	}
}

//------------------------------------------------------------------

void statusTime()
{
	status(STATUS_TIME, _T("%s %u:%02u"), lng(680, "Time"),
		board.playtime/60, board.playtime%60);
}

void statusScore()
{
	status(STATUS_SCORE, _T("%s: %d"), lng(681, "Score"), board.score);
}

void statusMoves()
{
	status(STATUS_MOVES, _T("%s: %d"), lng(683, "Moves"), board.moves);
}

void statusDone()
{
	if(board.donePercent>=0) status(STATUS_DONE, _T("%s:%d  (%d%%)"),
		lng(682, "Done"), board.doneCards, board.donePercent);
	else status(STATUS_DONE, _T(""));
}

// Display THE CURRENT SEED in the status bar
void statusSeed()
{
	if(board.number) status(STATUS_SEED, _T("%s: %u"), lng(684, "Shuffle"), board.number);
	else status(STATUS_SEED, _T(""));
}

// Display CARDS IN THE STACK in the status bar
static int last;
void statusNcard()
{
	if(currentPile>=0 && currentPile<board.cells.len){
		int i= board.cells[currentPile].Ncards;
		if(last!=i){
			last=i;
			status(STATUS_NCARD, _T("%s: %d"), lng(685, "Pile"), i);
		}
	}
	else{
		if(last!=-1){
			last=-1;
			status(STATUS_NCARD, _T("%s:"), lng(685, "Pile"));
		}
	}
}

void updateStatus()
{
	statusTime();
	statusScore();
	statusDone();
	statusMoves();
	last=-2; statusNcard();
	statusSeed();
}

//------------------------------------------------------------------
void setBmpCards(HBITMAP h, HDC dc, int W, int H)
{
	if(!dcCards) dcCards=CreateCompatibleDC(dc);
	SelectObject(dcCards, h);
	if(bmpCards) DeleteObject(bmpCards);
	bmpCards=h;
	bmpCardW=W;
	bmpCardH=H;
}

#ifndef NDEBUG
void readCardsFromFiles(TCHAR *path)
{
	int r, s, w=0, h=0, w1, h1, err=0;
	TCHAR *fn;
	HBITMAP b;
	HGDIOBJ oldB;
	HDC dc, dc1;

	dc=GetDC(hWin);
	dc1=CreateCompatibleDC(dc);
	fn=cutPath(path);
	for(s=0; s<4; s++){
		for(r=0; r<13; r++){
			fn[0]=(TCHAR)((r+1)/10+'0');
			fn[1]=(TCHAR)((r+1)%10+'0');
			fn[2]="hdcs"[s];
			//fn[1]="a234567891jqk"[r];
			b=readBMP(path, dc, &w1, &h1);
			if(r==0 && s==0){
				w=w1; h=h1;
				setBmpCards(CreateCompatibleBitmap(dc, w*13, h*4), dc, w, h);
			}
			else{
				if(w!=w1 && h!=h1){
					if(!err) msg("Cards don't have same dimensions");
					err++;
				}
			}
			oldB=SelectObject(dc1, b);
			BitBlt(dcCards, r*w, s*h, w, h, dc1, 0, 0, SRCCOPY);
			SelectObject(dc1, oldB);
			DeleteObject(b);
		}
	}
	DeleteDC(dc1);
	ReleaseDC(hWin, dc);
}
#endif

void searchDirMenu(int which, int action)
{
	int i, id;
	TCHAR *s;
	TfileName buf;
	static int I[]={ID_CARDS, ID_BACK, ID_BKGND, ID_CELL};
	static TCHAR *F[]={fnCards, fnBack, fnBkgnd, fnCell};
	static Darray<TCHAR*> *A[]={&menuCards, &menuBack, &menuBkgnd, &menuCell};

	int firstId=I[which];
	TCHAR *file=F[which];
	Darray<TCHAR*> &array=*A[which];

	_tcscpy(buf, cutPath(file));
	s=cutExt(buf);
	if(s) *s=0;
	id=0;
	for(i=0; i<array.len; i++){
		if(!_tcsicmp(buf, array[i])){
			id=i+1;
			break;
		}
	}
	switch(action){
		case 0: //check
			CheckMenuRadioItem(GetMenu(hWin), firstId+1, firstId+array.len, firstId+id, MF_BYCOMMAND);
			break;
		case 1: //next
			if(id==array.len) id=0;
			id++;
			PostMessage(hWin, WM_COMMAND, firstId+id, 0);
			break;
		case 2: //previous
			id--;
			if(id<=0) id=array.len;
			PostMessage(hWin, WM_COMMAND, firstId+id, 0);
			break;
	}
}

void checkCardsMenu()
{
	searchDirMenu(MENU_CARDS, 0);
}

void checkBackMenu()
{
	searchDirMenu(MENU_BACK, 0);
}

void checkBkgndMenu()
{
	searchDirMenu(MENU_BKGND, 0);
}

void checkCellMenu()
{
	searchDirMenu(MENU_CELL, 0);
}

void loadCards()
{
	int w, h;
	HDC dc=GetDC(hWin);
	HBITMAP b= readBMP(fnCards, dc, &w, &h);
	if(b){
		setBmpCards(b, dc, w/13, h/4);
		calcCardW();
		stretchCards(dc);
	}
	ReleaseDC(hWin, dc);
	checkCardsMenu();
}

void loadBkgnd()
{
	HBITMAP b= loadBMP(fnBkgnd);
	if(b){
		DeleteObject(bkgndBrush);
		DeleteObject(bmpBkgnd);
		bkgndBrush = CreatePatternBrush(bmpBkgnd=b);
	}
	checkBkgndMenu();
}

void loadBack()
{
	HDC dc=GetDC(hWin);
	HBITMAP b= readBMP(fnBack, dc, &bmpBackW, &bmpBackH);
	if(b){
		if(!dcBack) dcBack=CreateCompatibleDC(dc);
		SelectObject(dcBack, b);
		if(bmpBack) DeleteObject(bmpBack);
		bmpBack=b;
		stretchCards(dc);
	}
	ReleaseDC(hWin, dc);
	checkBackMenu();
}

void loadCell()
{
	HDC dc=GetDC(hWin);
	HBITMAP b= readBMP(fnCell, dc, &bmpCellW, &bmpCellH);
	if(b){
		if(!dcCell) dcCell=CreateCompatibleDC(dc);
		SelectObject(dcCell, b);
		if(bmpCell) DeleteObject(bmpCell);
		bmpCell=b;
		stretchCards(dc);
	}
	ReleaseDC(hWin, dc);
	checkCellMenu();
}

void refresh()
{
	loadCards();
	loadBack();
	loadBkgnd();
	loadCell();
	langChanged();
	invalidate();
}

#ifndef NDEBUG
void saveBMP(TCHAR *fn, HBITMAP hBmp)
{
	BITMAPINFOHEADER bmi;
	BITMAPFILEHEADER hdr;
	BITMAP bmp;
	HANDLE hf;
	BYTE *bits;
	DWORD d;

	GetObject(hBmp, sizeof(BITMAP), &bmp);
	HDC winDC= GetDC(0);
	HDC dcb= CreateCompatibleDC(winDC);
	ReleaseDC(0, winDC);
	//initialize bitmap header
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = bmp.bmWidth;
	bmi.biHeight = bmp.bmHeight;
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = (3*(bmp.bmWidth+1)&-4)*bmp.bmHeight;
	bmi.biClrImportant = 0;
	bmi.biClrUsed=0;
	//copy bitmapu to temporary bufferu
	bits = new BYTE[bmi.biSizeImage];
	if(!GetDIBits(dcb, hBmp, 0, bmp.bmHeight, bits, (BITMAPINFO*)&bmi, DIB_RGB_COLORS)){
		msg("GetDIBits failed");
	}
	else{
		//save BMP file
		hf = CreateFile(fn, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if(hf==INVALID_HANDLE_VALUE){
			msglng(733, "Cannot create file %s", fn);
		}
		else{
			hdr.bfType = 0x4d42;  //'BM'
			hdr.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bmi.biSizeImage;
			hdr.bfReserved1 = 0;
			hdr.bfReserved2 = 0;
			hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
			WriteFile(hf, &hdr, sizeof(BITMAPFILEHEADER), &d, 0);
			WriteFile(hf, &bmi, sizeof(BITMAPINFOHEADER), &d, 0);
			WriteFile(hf, bits, bmi.biSizeImage, &d, 0);
			CloseHandle(hf);
		}
	}
	//delete bitmap
	delete[] bits;
	DeleteDC(dcb);
}
#endif
//---------------------------------------------------------------------------
/*
ACC, some details here: by default, XMSol would save the config in the registery.
I know that's the way Microsoft wants it to be, but I also know INI files and registery
are not providing the same services and I also know it annoys some people to save stuff
there without them being aware of it (especially for transportable applications, which
doesn't have any install/uninstall process and could be run from an USB Stick).

I added "configToRegistery", which is checked on startup: if the registery key exist,
then we save there. If the key doesn't exist, we use an INI file. The trick is that we
must erase the key and every subkey when we turn "configToRegistery" off.
*/

//delete settings
void deleteini()
{
	//delete registry
	HKEY key;
	DWORD i;
	if(RegDeleteKey(HKEY_CURRENT_USER, subkey)==ERROR_SUCCESS){
		if(RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Petr Lastovicka"), 0, KEY_QUERY_VALUE, &key)==ERROR_SUCCESS){
			i=1;
			RegQueryInfoKey(key, 0, 0, 0, &i, 0, 0, 0, 0, 0, 0, 0);
			RegCloseKey(key);
			if(!i){
				RegDeleteKey(HKEY_CURRENT_USER, _T("Software\\Petr Lastovicka"));
			}
		}
	}

	//delete INI file
	getExeDir(fnIni, iniFile);
	_tremove(fnIni);
}

//save settings
void writeini()
{
	regValB[0].n=Naccel*sizeof(ACCEL);
	Ntoolbar = SendMessage(toolbar, TB_BUTTONCOUNT, 0, 0);

	if(!configToRegistery)
	{
		// Erase INI file and create a new one
		getExeDir(fnIni, iniFile);
		FILE* f=_tfopen(fnIni, _T("w"));
		if(!f){
			msglng(733, "Cannot create file %s", fnIni);
			configToRegistery=true;
		}
		else{
			fputs("[xmsol]\n", f);
			for(Treg *u = regVal; u < endA(regVal); u++)
			{
				fprintf(f, "%s=%d\n", u->s, *(u->i));
			}
			for(Tregs *v = regValS; v < endA(regValS); v++)
			{
				fputs(v->s, f);
				fputc('=', f);
				_fputts(v->i, f);
				fputc('\n', f);
			}
			fclose(f);
			for(Tregb *w=regValB; w<endA(regValB); w++){
				convertA2T(w->s, t);
				WritePrivateProfileStruct(iniSection, t, w->i, w->n, fnIni);
			}

			//save toolbar
			TBBUTTON* buttons = new TBBUTTON[Ntoolbar];
			for(int i = 0; i < Ntoolbar; i++)
			{
				SendMessage(toolbar, TB_GETBUTTON, i, (LPARAM)(buttons + i));
			}
			WritePrivateProfileStruct(iniSection, _T("toolbar"), buttons, Ntoolbar*sizeof(TBBUTTON), fnIni);
			delete[] buttons;
		}
	}
	if(configToRegistery)
	{
		HKEY key;
		if(RegCreateKey(HKEY_CURRENT_USER, subkey, &key)!=ERROR_SUCCESS)
			msglng(735, "Cannot write to Windows registry");
		else{
			// Write integers
			for(Treg *u=regVal; u<endA(regVal); u++){
				RegSetValueExA(key, u->s, 0, REG_DWORD,
					(BYTE *)u->i, sizeof(int));
			}
			// Write strings
			for(Tregs *v=regValS; v<endA(regValS); v++){
				convertA2T(v->s, t);
				RegSetValueEx(key, t, 0, REG_SZ,
					(BYTE *)v->i, (DWORD) sizeof(TCHAR)*(lstrlen(v->i)+1));
			}
			// Write binary data
			for(Tregb *w=regValB; w<endA(regValB); w++){
				RegSetValueExA(key, w->s, 0, REG_BINARY, (BYTE *)w->i, w->n);
			}
			// Write toolbar
			TBSAVEPARAMS sp;
			sp.hkr=HKEY_CURRENT_USER;
			sp.pszSubKey=subkey;
			sp.pszValueName=_T("toolbar");
			SendMessage(toolbar, TB_SAVERESTORE, TRUE, (LPARAM)&sp);
			RegCloseKey(key);
		}
	}
}

//read settings
void readini()
{
	configToRegistery=true;

	HKEY key;
	DWORD d;
	if(RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_QUERY_VALUE, &key)==ERROR_SUCCESS){
		for(Treg *u=regVal; u<endA(regVal); u++){
			d=sizeof(int);
			RegQueryValueExA(key, u->s, 0, 0, (BYTE *)u->i, &d);
		}
		for(Tregs *v=regValS; v<endA(regValS); v++){
			convertA2T(v->s, t);
			d=v->n;
			RegQueryValueEx(key, t, 0, 0, (BYTE *)v->i, &d);
		}
		for(Tregb *w=regValB; w<endA(regValB); w++){
			if(w->i==colOrder){
				if(RegQueryValueExA(key, w->s, 0, 0, 0, &d)==ERROR_SUCCESS && d!=w->n) continue;
			}
			d=w->n;
			if(RegQueryValueExA(key, w->s, 0, 0, (BYTE *)w->i, &d)==ERROR_SUCCESS
				&& w==regValB) Naccel=d/sizeof(ACCEL);
		}
		RegCloseKey(key);
	}
	else{
		getExeDir(fnIni, iniFile);
		FILE* f = _tfopen(fnIni, _T("r")); // Test the IniFile
		if(f){
			fclose(f);
			configToRegistery=false;

			TCHAR buf[512];
			for(Treg *u = regVal; u < endA(regVal); u++)
			{
				convertA2T(u->s, t);
				if(GetPrivateProfileString(iniSection, t, 0, buf, sizeA(buf), fnIni) > 0)
					*u->i = _ttoi(buf);
			}
			for(Tregs *v = regValS; v < endA(regValS); v++)
			{
				convertA2T(v->s, t);
				if(GetPrivateProfileString(iniSection, t, 0, buf, sizeA(buf), fnIni) > 0)
					lstrcpyn(v->i, buf, v->n);
			}
			aminmax(Naccel, 0, Maccel);
			regValB[0].n=Naccel*sizeof(ACCEL);
			for(Tregb *w=regValB; w<endA(regValB); w++){
				convertA2T(w->s, t);
				GetPrivateProfileStruct(iniSection, t, w->i, w->n, fnIni);
			}
		}
	}
}

//------------------------------------------------------------------
void onMoved()
{
	if(IsIconic(hWin)) return;
	maximized=IsZoomed(hWin);
	if(!maximized){
		RECT rc;
		GetWindowRect(hWin, &rc);
		mainTop= rc.top;
		mainLeft= rc.left;
		mainW= rc.right-rc.left;
		mainH= rc.bottom-rc.top;
	}
}

void setFont()///
{
	HGDIOBJ oldF;
	oldF=hFont;
	hFont=CreateFontIndirect(&font);
	SendMessage(listBox, WM_SETFONT, (WPARAM)hFont, (LPARAM)MAKELPARAM(TRUE, 0));
	DeleteObject(oldF);
	TEXTMETRIC tm;
	HDC dc=GetDC(listBox);
	oldF= SelectObject(dc, hFont);
	GetTextMetrics(dc, &tm);
	SelectObject(dc, oldF);
	ReleaseDC(listBox, dc);
	SendMessage(listBox, LB_SETITEMHEIGHT, 0, MAKELPARAM(tm.tmHeight+1+tm.tmExternalLeading, 0));
}

void accelChanged()
{
	DestroyAcceleratorTable(haccel);
	haccel=0;
	if(Naccel>0 && accel[0].cmd){
		haccel= CreateAcceleratorTable(accel, Naccel);
	}
	if(!haccel){
		haccel= LoadAccelerators(inst, MAKEINTRESOURCE(IDR_ACCELERATOR));
		Naccel= CopyAcceleratorTable(haccel, accel, sizeA(accel));
	}
}

void checkFlipMenu()
{
	HMENU m=GetMenu(hWin);
	CheckMenuItem(m, ID_MIRROR_LR, flipx ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(m, ID_MIRROR_UD, flipy ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(m, ID_SHOWTOOLBAR, toolBarVisible ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(m, ID_SHOWSTATUSBAR, statusBarVisible ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(m, ID_TOGGLEREGISTERY, configToRegistery ? MF_BYCOMMAND|MF_UNCHECKED : MF_BYCOMMAND|MF_CHECKED);
}

void checkMenus()
{
	checkCardsMenu();
	checkBackMenu();
	checkBkgndMenu();
	checkCellMenu();
	checkFlipMenu();
}

static int subId[]={405, 404, 408, 403, 402, 406, 401, 400, 407};

void reloadMenu()
{
	loadMenu(hWin, MAKEINTRESOURCE(IDR_MENU), subId);
	menuFromDir(_T("cards\\*.*"), ID_CARDS, menuCards);
	menuFromDir(_T("back\\*.*"), ID_BACK, menuBack);
	menuFromDir(_T("bkgnd\\*.*"), ID_BKGND, menuBkgnd);
	menuFromDir(_T("cell\\*.*"), ID_CELL, menuCell);
	checkMenus();
	DrawMenuBar(hWin);
}

void langChanged()
{
	reloadMenu();
	updateStatus();
	SetWindowText(rulesWnd, lng(21, "Rules"));
	userOfn.lpstrFilter= lng(501, _T("Player (*.xol)\0*.xol\0All files\0*.*\0"));
	generRules(board.game);
}

//------------------------------------------------------------------

//get menu item name
void getCmdName(TCHAR *buf, int n, int cmd)
{
	TCHAR *s, *d;

	s=lng(cmd, (char*)0);
	if(s){
		lstrcpyn(buf, s, n);
	}
	else{
		MENUITEMINFO mii;
		mii.cbSize=sizeof(MENUITEMINFO);
		mii.fMask=MIIM_TYPE;
		mii.dwTypeData=buf;
		mii.cch= n;
		GetMenuItemInfo(GetMenu(hWin), cmd, FALSE, &mii);
	}
	for(s=d=buf; *s; s++){
		if(*s=='\t') *s=0; //remove keyboard shortcut
		if(*s!='&') *d++=*s; //remove underscores
	}
	*d=0;
}

void changeKey(HWND hDlg, UINT vkey)
{
	TCHAR *buf=(TCHAR*)_alloca(2*64);
	BYTE flg;
	ACCEL *a, *e;

	dlgKey.key=(USHORT)vkey;
	//read shift states
	flg=FVIRTKEY|FNOINVERT;
	if(GetKeyState(VK_CONTROL)<0) flg|=FCONTROL;
	if(GetKeyState(VK_SHIFT)<0) flg|=FSHIFT;
	if(GetKeyState(VK_MENU)<0) flg|=FALT;
	dlgKey.fVirt=flg;
	//set hotkey edit control
	printKey(buf, &dlgKey);
	SetWindowText(GetDlgItem(hDlg, IDC_KEY), buf);
	//modify accelerator table
	e=accel+Naccel;
	if(!dlgKey.cmd){
		for(a=accel; a<e; a++){
			if(a->key==vkey && a->fVirt==flg){
				PostMessage(hDlg, WM_COMMAND, a->cmd, 0);
			}
		}
	}
	else{
		for(a=accel;; a++){
			if(a==e){
				//add new hotkey
				*a=dlgKey;
				Naccel++;
				break;
			}
			if(a->cmd==dlgKey.cmd){
				//change hotkey
				a->fVirt=dlgKey.fVirt;
				a->key=dlgKey.key;
				if(vkey==0){
					//delete hotkey
					memcpy(a, a+1, (e-a-1)*sizeof(ACCEL));
					Naccel--;
				}
				break;
			}
		}
		accelChanged();
		//change the main menu
		reloadMenu();
	}
}

//------------------------------------------------------------------
LRESULT CALLBACK rulesProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM)
{
	int cmd;
	HWND w;

	switch(message){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 21);
			w=GetDlgItem(hWnd, IDC_RULES);
			SendMessage(w, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
			SendMessage(w, EM_SETLANGOPTIONS, 0, 0);
			return TRUE;

		case WM_COMMAND:
			cmd=LOWORD(wParam);
			if(cmd==IDCANCEL){
				ShowWindow(hWnd, SW_HIDE);
				return TRUE;
			}
			break;
	}
	return FALSE;
}

//------------------------------------------------------------------
DWORD getVer()
{
	HRSRC r;
	HGLOBAL h;
	void *s;
	VS_FIXEDFILEINFO *v;
	UINT i;

	r=FindResource(0, (TCHAR*)VS_VERSION_INFO, RT_VERSION);
	h=LoadResource(0, r);
	s=LockResource(h);
	if(!s || !VerQueryValueA(s, "\\", (void**)&v, &i)) return 0;
	return v->dwFileVersionMS;
}

INT_PTR CALLBACK AboutProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM)
{
	char buf[48];
	DWORD d;

	switch(message){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 11);
			d=getVer();
			sprintf(buf, "%d.%d", HIWORD(d), LOWORD(d));
			SetDlgItemTextA(hWnd, 101, buf);
			SetDlgItemInt(hWnd, IDC_TOTAL, games.len, FALSE);
			return TRUE;

		case WM_COMMAND:
			if(wParam == IDOK || wParam == IDCANCEL){
				EndDialog(hWnd, TRUE);
				return TRUE;
			}
			if(wParam==123){
				//start web browser
				GetDlgItemTextA(hWnd, wParam, buf, sizeA(buf));
				if(!_tcscmp(lang, _T("Èesky"))) strcat(buf, "/indexCS.html");
				ShellExecuteA(0, 0, buf, 0, 0, SW_SHOWNORMAL);
			}
			break;
	}
	return FALSE;
}
//------------------------------------------------------------------
INT_PTR CALLBACK NewGameNumberProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM)
{
	unsigned i;

	switch(message){
		case WM_INITDIALOG:
			setDlgTexts(hWnd, 22);
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDOK:
					i=GetDlgItemInt(hWnd, IDC_GAMENUMBER, 0, FALSE);
					if(i){
						board.seed=i;
						board.newGame(board.game);
					}
					//!
				case IDCANCEL:
					EndDialog(hWnd, TRUE);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

//------------------------------------------------------------------
inline int listLen()
{
	return games.len;
}

//adjust asc/desc image color inside a column header
void btnColor(HBITMAP bmp)
{
	int i, j;
	HDC wdc, dc;
	HGDIOBJ oldB;
	COLORREF cbtn, c;
	BITMAP o;

	wdc= GetDC(hWin);
	dc= CreateCompatibleDC(wdc);
	ReleaseDC(hWin, wdc);
	if(dc){
		oldB= SelectObject(dc, bmp);
		cbtn= (COLORREF)GetSysColor(COLOR_BTNFACE);
		c= GetPixel(dc, 0, 0);
		GetObject(bmp, sizeof(BITMAP), &o);
		for(i=0; i<o.bmWidth; i++){
			for(j=0; j<o.bmHeight; j++){
				if(GetPixel(dc, i, j)==c){
					SetPixel(dc, i, j, cbtn);
				}
			}
		}
		SelectObject(dc, oldB);
		DeleteDC(dc);
	}
}

void setCurIndex(int index)
{
	if(index>=0){
		amax(index, listLen()-1);
		ListView_SetItemState(listBox, index,
			LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
		ListView_EnsureVisible(listBox, index, FALSE);
		SetFocus(listBox);
	}
}

void setCur(int item)
{
	LV_FINDINFO lfi;
	lfi.flags=LVFI_PARAM;
	lfi.lParam=(LPARAM)item;
	setCurIndex(ListView_FindItem(listBox, -1, &lfi));
}

//incremental search in the games list
void search(UINT vkey)
{
	static TCHAR searchBuf[32];
	static int searchLen;
	static DWORD searchLastKey;

	DWORD time= GetTickCount();
	if(time-searchLastKey>1500) searchLen=0;
	searchLastKey=time;
	//convert virtual key to character
	BYTE keystate[256];
	GetKeyboardState(keystate);
	if(keystate[VK_MENU]) return;
	char ch[2];
	if(ToAscii(vkey, 0, keystate, (LPWORD)&ch, 0)!=1) return;
	//append character to the end of string
	if(searchLen==sizeof(searchBuf)) return;
	searchBuf[searchLen++]=ch[0];
	//find item which starts with searched text
	for(int i=0; i<listLen(); i++){
		if(!_tcsnicmp(games[i]->name, searchBuf, searchLen)){
			setCur(i);
			break;
		}
	}
}

void sortChanged()
{
	HD_ITEM hd;
	HWND header= ListView_GetHeader(listBox);

	for(int i=0; i<Mcolumn; i++){
		if(i==sortedCol){
			hd.mask = HDI_IMAGE | HDI_FORMAT;
			hd.fmt = HDF_STRING | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
			hd.iImage = descending==1;
		}
		else{
			hd.mask = HDI_FORMAT;
			hd.fmt = HDF_STRING;
		}
		Header_SetItem(header, i, &hd);
	}
}

struct TsortInfo {
	int desc;
	TstatMember stat;
	TgameMember game;
};

int CALLBACK sortStat(LPARAM ai, LPARAM bi, LPARAM p)
{
	Tstat *a, *b;
	a=games[ai]->stat;
	b=games[bi]->stat;
	TsortInfo *i= (TsortInfo*)p;
	if(!a->played) return b->played!=0;
	if(!b->played) return -1;
	return i->desc * (b->*(i->stat) - a->*(i->stat));
}

int CALLBACK sortGame(LPARAM ai, LPARAM bi, LPARAM p)
{
	Tgame *a, *b;
	a=games[ai];
	b=games[bi];
	TsortInfo *i= (TsortInfo*)p;
	return i->desc * (b->*(i->game) - a->*(i->game));
}

int CALLBACK sortName(LPARAM ai, LPARAM bi, LPARAM p)
{
	Tgame *a, *b;
	a=games[ai];
	b=games[bi];
	return int(p)*_tcscmp(b->name, a->name);
}

static int initializing=0;

//initialize game list
void initList()
{
	int i, bottom, n, Nitem;
	TsortFunc f;
	LV_ITEM lvi;
	TsortInfo si;

	Nitem=listLen();
	SendMessage(listBox, WM_SETREDRAW, FALSE, 0);
	bottom=ListView_GetTopIndex(listBox)+ListView_GetCountPerPage(listBox)-1;
	amax(bottom, Nitem-1);
	n=ListView_GetItemCount(listBox);

	//insert or delete items if total number of games changed
	if(n!=Nitem){
		initializing++;
		//delete rows
		while(n>Nitem){
			n--;
			ListView_DeleteItem(listBox, n);
		}
		//allocate memory
		ListView_SetItemCount(listBox, Nitem);
		//add rows
		lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		lvi.state = 0;
		lvi.stateMask = 0;
		lvi.pszText = LPSTR_TEXTCALLBACK;
		lvi.iImage = -1;
		lvi.iSubItem = 0;
		while(n<Nitem){
			lvi.iItem = n;
			ListView_InsertItem(listBox, &lvi);
			n++;
		}
		//listbox contains only indexes, data are retrieved by WM_NOTIFY
		for(i=0; i<Nitem; i++){
			lvi.iItem = i;
			lvi.mask = LVIF_PARAM;
			lvi.lParam = (LPARAM)i;
			ListView_SetItem(listBox, &lvi);
		}
		initializing--;
	}
	//sort
	i=sortedCol;
	if(i<Mcolumn){
		if(i==0 || i==12) f=sortName;
		else if(colPlayed[i]) f=sortStat;
		else f=sortGame;
		si.desc= descending;
		si.stat= colStat[i];
		si.game= colGame[i];
		ListView_SortItems(listBox, f, (f==sortStat || f==sortGame) ? (LPARAM)&si : (LPARAM)descending);
	}
	SendMessage(listBox, WM_SETREDRAW, TRUE, 0);
	ListView_EnsureVisible(listBox, bottom, TRUE);
}

void utod(unsigned i, TCHAR *s)
{
	_ultot(i, s, 10);
}

void printDate(TCHAR *buf, int n, time_t date)
{
	_tcsftime(buf, n, _T("%x %X"), localtime(&date));
}

// The following routine handles the game list.
INT_PTR CALLBACK GameListProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lP)
{
	int i, j, cmd, item, oldW, oldH, oldS, oldT, oldBW, oldBH;
	static int oldItem;
	Tgame *g;
	Tstat *t;
	DRAWITEMSTRUCT *di;
	HIMAGELIST himl;
	HBITMAP hbmp;
	LV_COLUMN lvc;
	LV_ITEM lvi;
	LVFINDINFO lfi;
	RECT rc;
	HDC dc;

	switch(message){
		case WM_INITDIALOG:
			dlgWin=hWnd;
			initializing++;
			setDlgTexts(hWnd, 12);
			listBox=GetDlgItem(hWnd, IDC_LIST);
			//set sorted column bitmap
			himl = ImageList_Create(14, 14, ILC_COLOR24, 2, 0);
			hbmp = LoadBitmap(inst, MAKEINTRESOURCE(IDB_ASCDESC));
			btnColor(hbmp);
			ImageList_Add(himl, hbmp, (HBITMAP)NULL);
			DeleteObject(hbmp);
			ListView_SetImageList(listBox, himl, LVSIL_SMALL);
			//create columns
			ListView_SetExtendedListViewStyleEx(listBox, LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT, LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
			lvc.mask = LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
			for(i=0; i<Mcolumn; i++){
				lvc.iSubItem = i;
				lvc.cx = colWidth[i];
				lvc.pszText= lng(650+i, colNames[i]);
				ListView_InsertColumn(listBox, i, &lvc);
			}
			//restore window position and size
			if(!gameListW) {
				dc = GetDC(hWnd);
				gameListW= 650*GetDeviceCaps(dc, LOGPIXELSX)/96;
				gameListH= 500*GetDeviceCaps(dc, LOGPIXELSY)/96;
				ReleaseDC(hWnd, dc);
			}
			MoveWindow(hWnd, gameListX, gameListY, gameListW, gameListH, FALSE);
			//read statistics from player's file
			for(j=0; j<games.len; j++){
				static Tstat zeroStat;
				games[j]->stat= &zeroStat;
			}
			playerFile.readGameStat();
			//initialize listbox
			initList();
			sortChanged();
			ListView_SetColumnOrderArray(listBox, Mcolumn, colOrder);
			//select current game
			lfi.flags=LVFI_STRING;
			lfi.psz=gameName;
			i=SendMessage(listBox, LVM_FINDITEM, (WPARAM)-1, (LPARAM)&lfi);
			if(i<0) i=0;
			lvi.iItem=i;
			lvi.iSubItem=0;
			lvi.mask=LVIF_PARAM;
			ListView_GetItem(listBox, &lvi);
			preview.newGame(games[(int)lvi.lParam]);
			generRules(preview.game);
			lvi.mask=LVIF_STATE;
			lvi.state=lvi.stateMask=LVIS_SELECTED|LVIS_FOCUSED;
			SendMessage(listBox, LVM_SETITEMSTATE, i, (LPARAM)&lvi);
			SendMessage(listBox, LVM_ENSUREVISIBLE, i+3, 0);
			SetFocus(listBox);
			InvalidateRect(GetDlgItem(hWnd, IDC_PREVIEW), 0, FALSE);
			//scroll
			j=-1;
			for(;;){
				i=GetScrollPos(listBox, SB_HORZ);
				if(i>=scrollPos || i==j) break;
				j=i;
				SendMessage(listBox, WM_KEYDOWN, VK_RIGHT, 0);
			}
			initializing--;
			return TRUE;

		case WM_COMMAND:
			cmd=LOWORD(wParam);
			//get focused item
			lvi.iItem= ListView_GetNextItem(listBox, -1, LVNI_FOCUSED);
			lvi.iSubItem=0;
			lvi.mask=LVIF_PARAM;
			ListView_GetItem(listBox, &lvi);
			item= (int)lvi.lParam;
			switch(cmd){
				case IDC_SHOWRULES:
					SetFocus(listBox);
					ShowWindow(rulesWnd, SW_SHOW);
					break;
				case IDC_BEST_SCORES:
					playerFile.bestScores(item);
					SetFocus(listBox);
					break;
				case IDCANCEL:
					generRules(board.game);
				case IDOK:
					ShowWindow(rulesWnd, SW_HIDE);
					//get columns order and width
					ListView_GetColumnOrderArray(listBox, Mcolumn, colOrder);
					for(i=0; i<Mcolumn; i++){
						colWidth[i]= (short)ListView_GetColumnWidth(listBox, i);
					}
					scrollPos=GetScrollPos(listBox, SB_HORZ);
					//save window position and size
					GetWindowRect(hWnd, &rc);
					gameListX=rc.left;
					gameListY=rc.top;
					gameListW=rc.right-rc.left;
					gameListH=rc.bottom-rc.top;
					//close dialog window
					dlgWin=0;
					EndDialog(hWnd, cmd);
					if(cmd==IDOK){
						board.newSeed();
						board.newGame(games[item]);
					}
					return TRUE;
			}
			break;
		case WM_DRAWITEM:
			di = (DRAWITEMSTRUCT*)lP;
			oldW=width; oldH=height;
			oldT=toolBarVisible; oldS=statusBarVisible;
			oldBH=borderH; oldBW=borderW;
			width=di->rcItem.right-di->rcItem.left;
			height=di->rcItem.bottom-di->rcItem.top;
			toolBarVisible=statusBarVisible=0;
			borderH=3; borderW=3;
			calcCardW();
			preview.paintBoard(di->hDC, &di->rcItem);
			width=oldW; height=oldH;
			toolBarVisible=oldT; statusBarVisible=oldS;
			borderH=oldBH; borderW=oldBW;
			calcCardW();
			break;
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lP)->ptMinTrackSize.x = 470;
			((MINMAXINFO*)lP)->ptMinTrackSize.y = 488;
			break;
		case WM_SIZE:
			//adjust controls positions inside window
			GetWindowRect(listBox, &rc);
			i=rc.bottom-rc.top;
			GetClientRect(hWnd, &rc);
			SetWindowPos(listBox, 0, 0, 0, rc.right, i, SWP_NOZORDER);
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lP)->code){
				case LVN_ITEMCHANGED:
					item=((NMLISTVIEW*)lP)->lParam;
					if(item>=0 && item<games.len && !initializing){
						generRules(games[item]);
						preview.newGame(games[item]);
						InvalidateRect(GetDlgItem(hWnd, IDC_PREVIEW), 0, FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_BEST_SCORES), games[item]->stat->played>0);
					}
					break;
				case LVN_GETDISPINFO:
				{
					LV_DISPINFO *pnmv= (LV_DISPINFO*)lP;
					if(pnmv->item.mask & LVIF_TEXT){
						TCHAR *buf = pnmv->item.pszText;
						*buf=0;
						if(pnmv->item.lParam>=listLen()) break;
						g = games[pnmv->item.lParam];
						int n = pnmv->item.cchTextMax;
						t= g->stat;
						i= pnmv->item.iSubItem;
						*buf=0;
						if(t->played || !colPlayed[i]){
							switch(i){
								case 1: case 2: case 4: case 6: case 14: case 16: case 17:
									utod(t->*(colStat[i]), buf);
									break;
								case 7: case 8: //number
									utod(g->*(colGame[i]), buf);
									break;
								case 0: case 12: //string
									_tcsncpy(buf, g->name, n);
									break;
								case 3: //percent
									_stprintf(buf, _T("%u%%"), t->percent);
									break;
								case 5: case 15: //time
									j=t->*(colStat[i]);
									_stprintf(buf, _T("%u:%02u"), j/60, j%60);
									break;
								case 13: //date
									printDate(buf, n, t->date);
									break;
								case 9:
									j= g->ruleSeq;
									if(j && j<sizeA(seqWord)){
										_tcscpy(buf, lng(1060+j, seqWord[j]));
									}
									break;
								case 10:
									j= g->ruleSuit;
									if(j && j<sizeA(suitWord)){
										_tcscpy(buf, lng(1050+j, suitWord[j]));
									}
									break;
								case 11:
									j= g->pair;
									if(j<256){
										if(j) utod(j, buf);
									}
									else{
										j-=256;
										if(j<sizeA(pairWord)){
											_tcscpy(buf, lng(1120+j, pairWord[j]));
										}
									}
									break;
								case 18:
									j= g->ruleGroup;
									if(j<sizeA(groupWord)){
										_tcscpy(buf, lng(1080+j, groupWord[j]));
									}
									break;
							}
						}
					}
					break;
				}
				case LVN_COLUMNCLICK:
				{
					NM_LISTVIEW *pnm= (NM_LISTVIEW *)lP;
					if(pnm->iSubItem==sortedCol){
						descending=-descending;
					}
					else{
						sortedCol=pnm->iSubItem;
						descending= sortedCol!=0 && sortedCol!=12 ? 1 : -1;
					}
					sortChanged();
					initList();
					break;
				}
				case LVN_KEYDOWN:
					search(((LV_KEYDOWN*)lP)->wVKey);
					break;
				case NM_DBLCLK:
					PostMessage(hWnd, WM_COMMAND, IDOK, 0);
					break;
			}
			break;

	}
	return FALSE;
}

//------------------------------------------------------------------
UINT APIENTRY CFHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
{
	if(message==WM_COMMAND && LOWORD(wParam)==1026){
		SendMessage(hDlg, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)&font);
		setFont();
		return 1;
	}
	return 0;
}

//------------------------------------------------------------------

static WNDPROC editWndProc;

//window procedure for hotkey edit control
LRESULT CALLBACK hotKeyClassProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
	switch(mesg){
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if(wP!=VK_CONTROL && wP!=VK_SHIFT && wP!=VK_MENU && wP!=VK_RWIN && wP!=VK_LWIN){
				if((wP==VK_BACK || wP==VK_DELETE) && dlgKey.key) wP=0;
				changeKey(GetParent(hWnd), (UINT)wP);
				return 0;
			}
			break;
		case WM_CHAR:
			return 0; //don't write character to the edit control
	}
	return CallWindowProc((WNDPROC)editWndProc, hWnd, mesg, wP, lP);
}

//------------------------------------------------------------------
INT_PTR CALLBACK KeysDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM)
{
	static const int Mbuf=64;
	TCHAR *buf=(TCHAR*)_alloca(2*Mbuf);
	int i, n;

	switch(message){
		case WM_INITDIALOG:
			editWndProc = (WNDPROC)SetWindowLong(GetDlgItem(hWnd, IDC_KEY),
				GWL_WNDPROC, (LONG)hotKeyClassProc);
			setDlgTexts(hWnd, 19);
			dlgKey.cmd=0;
			return TRUE;

		case WM_COMMAND:
			wParam=LOWORD(wParam);
			switch(wParam){
				default:
					//popup menu item
					if(wParam<500 && wParam>=200){
						//set command text
						dlgKey.cmd=(USHORT)wParam;
						getCmdName(buf, Mbuf, wParam);
						SetDlgItemText(hWnd, IDC_KEYCMD, buf);
						HWND hHotKey=GetDlgItem(hWnd, IDC_KEY);
						if(!*buf) dlgKey.cmd=0;
						else SetFocus(hHotKey);
						//change accelerator table
						dlgKey.key=0;
						*buf=0;
						for(ACCEL *a=accel+Naccel-1; a>=accel; a--){
							if(a->cmd==dlgKey.cmd){
								dlgKey.fVirt=a->fVirt;
								dlgKey.key=a->key;
								printKey(buf, &dlgKey);
								break;
							}
						}
						//set hotkey edit control
						SetWindowText(hHotKey, buf);
					}
					break;
				case IDC_MENUDROP:
				{
					RECT rc;
					GetWindowRect(GetDlgItem(hWnd, IDC_MENUDROP), &rc);
					//create popup menu from the main menu
					HMENU hMenu= CreatePopupMenu();
					MENUITEMINFO mii;
					n=GetMenuItemCount(GetMenu(hWin));
					for(i=0; i<n; i++){
						mii.cbSize=sizeof(mii);
						mii.fMask=MIIM_SUBMENU|MIIM_TYPE;
						mii.cch=Mbuf;
						mii.dwTypeData=buf;
						GetMenuItemInfo(GetMenu(hWin), i, TRUE, &mii);
						InsertMenuItem(hMenu, 0xffffffff, TRUE, &mii);
					}
					//show popup menu
					TrackPopupMenuEx(hMenu,
						TPM_RIGHTBUTTON, rc.left, rc.bottom, hWnd, NULL);
					//delete popup menu
					for(i=0; i<n; i++){
						RemoveMenu(hMenu, 0, MF_BYPOSITION);
					}
					DestroyMenu(hMenu);
					break;
				}
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd, wParam);
					return TRUE;
			}
	}
	return FALSE;
}

//------------------------------------------------------------------
int getRadioButton(HWND hWnd, int item1, int item2)
{
	for(int i=item1; i<=item2; i++){
		if(IsDlgButtonChecked(hWnd, i)){
			return i-item1;
		}
	}
	return 0;
}

struct TintValue{ int *value; WORD id; short dlgId; };
struct TboolValue{ int *value; WORD id; short dlgId; };
struct TstrValue{ TCHAR *value; int len; WORD id; short dlgId; };
struct TgroupValue{ int *value; WORD first; WORD last; short dlgId; };
struct TcomboValue{ int *value; WORD id; WORD lngId; char **strA; WORD len; short dlgId; };

static TintValue intOpts[]={
		{&animation[ANIM_AUTOPLAY].speed, IDC_AUTODELAY, 0},
		{&animation[ANIM_UNDO].speed, IDC_UNDODELAY, 0},
		{&animation[ANIM_RIGHTCLICK].speed, IDC_RIGHTCLICKDELAY, 0},
		{&animation[ANIM_STOCK].speed, IDC_STOCKDELAY, 0},
		{&borderW, IDC_BORDER_LR, 1},
		{&borderH, IDC_BORDER_UD, 1},
		{&sequenceDx, IDC_SEQDIST_X, 1},
		{&sequenceDy, IDC_SEQDIST_Y, 1},
		{&seqHiddenDx, IDC_SEQDIST_HX, 1},
		{&seqHiddenDy, IDC_SEQDIST_HY, 1},
		{&corners, IDC_CORNERS, 1},
		{&SCORE_FOUNDATION, IDC_SCORE_FOUNDATION, 2},
		{&SCORE_UNHIDE, IDC_SCORE_UNHIDE, 2},
		{&SCORE_UNDO, IDC_SCORE_UNDO, 2},
		{&SCORE_WRONG_MOVE, IDC_SCORE_WRONG_MOVE, 2},
		{&SCORE_REDEAL, IDC_SCORE_REDEAL, 2},
		{&SCORE_TIME, IDC_SCORE_TIME, 2},
		{&SCORE_BONUS, IDC_SCORE_BONUS, 2},
};

static TboolValue boolOpts[]={
		{&globalAutoplay, IDC_AUTOPLAY, 0},
		{&animation[ANIM_AUTOPLAY].enabled, IDC_ANIM_AUTOPLAY, 0},
		{&animation[ANIM_UNDO].enabled, IDC_ANIM_UNDO, 0},
		{&animation[ANIM_RIGHTCLICK].enabled, IDC_ANIM_RIGHTCLICK, 0},
		{&animation[ANIM_STOCK].enabled, IDC_ANIM_STOCK, 0},
		{&halftone, IDC_HALFTONE, 1},
};

// Initialize the correct page in the Options window
BOOL propPageInit(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP, int curPage)
{
	HWND sheet;
	TintValue *ti;
	TboolValue *tb;

	switch(mesg){
		case WM_INITDIALOG:
			setDlgTexts(hWnd);
			sheet=GetParent(hWnd);
			SetWindowLong(sheet, GWL_EXSTYLE, GetWindowLong(sheet, GWL_EXSTYLE)&~WS_EX_CONTEXTHELP);
			for(ti=intOpts; ti<endA(intOpts); ti++){
				if(ti->dlgId==curPage) SetDlgItemInt(hWnd, ti->id, *ti->value, TRUE);
			}
			for(tb=boolOpts; tb<endA(boolOpts); tb++){
				if(tb->dlgId==curPage) CheckDlgButton(hWnd, tb->id, *tb->value ? BST_CHECKED : BST_UNCHECKED);
			}
			return TRUE;
		case WM_COMMAND:
			if(HIWORD(wP)==EN_CHANGE || HIWORD(wP)==BN_CLICKED || HIWORD(wP)==CBN_SELCHANGE){
				PropSheet_Changed(GetParent(hWnd), hWnd);
			}
			break;
		case WM_NOTIFY:
			switch(((NMHDR*)lP)->code){
				case PSN_APPLY:
					for(ti=intOpts; ti<endA(intOpts); ti++){
						if(ti->dlgId==curPage) *ti->value= GetDlgItemInt(hWnd, ti->id, NULL, TRUE);
					}
					for(tb=boolOpts; tb<endA(boolOpts); tb++){
						if(tb->dlgId==curPage) *tb->value= IsDlgButtonChecked(hWnd, tb->id);
					}
					SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
					return TRUE;
				case PSN_SETACTIVE:
					optionsPage=curPage;
					break;
			}
			break;
	}
	return FALSE;
}

BOOL CALLBACK optionsAnimationProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
	BOOL result= propPageInit(hWnd, mesg, wP, lP, 0);
	switch(mesg){
		case WM_NOTIFY:
			switch(((NMHDR*)lP)->code){
				case PSN_APPLY:
					break;
			}
			break;
	}
	return result;
}

BOOL CALLBACK optionsDisplayProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
	BOOL result= propPageInit(hWnd, mesg, wP, lP, 1);
	switch(mesg){
		case WM_NOTIFY:
			switch(((NMHDR*)lP)->code){
				case PSN_APPLY:
					stretchW=0;
					calcCardW();
					break;
			}
			break;
	}
	return result;
}

BOOL CALLBACK optionsScoreProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
	BOOL result= propPageInit(hWnd, mesg, wP, lP, 2);
	switch(mesg){
		case WM_NOTIFY:
			switch(((NMHDR*)lP)->code){
				case PSN_APPLY:
					board.calcDone();
					statusScore();
					break;
			}
			break;
	}
	return result;
}

void options()
{
	int i;
	static DLGPROC P[]={
		(DLGPROC)optionsAnimationProc, (DLGPROC)optionsDisplayProc,
		(DLGPROC)optionsScoreProc,
	};
	static char *T[]={"Animation", "Display", "Score"};
	static int O[]={14, 15, 16};
	static int R[]={OPTIONS_ANIMATION, OPTIONS_DISPLAY, OPTIONS_SCORE};
	PROPSHEETPAGE psp[sizeA(P)], *p;
	PROPSHEETHEADER psh;

	for(i=0; i<sizeA(psp); i++){
		p=&psp[i];
		p->dwSize = sizeof(PROPSHEETPAGE);
		p->dwFlags = PSP_USETITLE;
		p->hInstance = inst;
		p->pszTemplate = MAKEINTRESOURCE(R[i]);
		p->pfnDlgProc = P[i];
		p->pszTitle = lng(O[i], T[i]);
	}
	// Set the property sheet
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE;
	psh.hwndParent = hWin;
	psh.hInstance = inst;
	psh.pszCaption = lng(20, "Options");
	psh.nPages = sizeA(psp);
	psh.nStartPage = optionsPage;
	psh.ppsp = psp;
	pause(true);
	PropertySheet(&psh);
	pause(false);
}

//------------------------------------------------------------------
LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, notif;
	static bool helpVisible;

	switch(message){

		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmm = (LPMINMAXINFO)lParam;
			lpmm->ptMinTrackSize.x = 300;
			lpmm->ptMinTrackSize.y = 200+toolH+statusH;
			break;
		}

		case WM_PAINT:
		{
			static PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			board.paintBoard(ps.hdc, &ps.rcPaint);
			EndPaint(hWnd, &ps);
			break;
		}

		case WM_TIMER:
			if(!paused && !IsIconic(hWin)){
				if(suspendTime>0) suspendTime--;
				else board.playtime++;
				statusTime();
			}
			break;

		case WM_MOVE:
			onMoved();
			break;

		case WM_SIZE:
			width=LOWORD(lParam);
			height=HIWORD(lParam);
			SendMessage(toolbar, TB_AUTOSIZE, 0, 0);
			SendMessage(statusbar, WM_SIZE, 0, 0);
			onMoved();
			calcCardW();
			break;

		case WM_NOTIFY:
		{
			NMTOOLBAR *nmtool;
			switch(((NMHDR*)lParam)->code){
				case TBN_QUERYDELETE:
				case TBN_QUERYINSERT:
					return TRUE;
				case TBN_GETBUTTONINFO:
					nmtool= (NMTOOLBAR*)lParam;
					i= nmtool->iItem;
					if(i<sizeA(tbb)){
						lstrcpyn(nmtool->pszText, lng(tbb[i].idCommand+1000, toolNames[i]), nmtool->cchText);
						nmtool->tbButton= tbb[i];
						return TRUE;
					}
					break;
				case TTN_NEEDTEXT:
				{
					TOOLTIPTEXT *ttt= (LPTOOLTIPTEXT)lParam;
					for(i=0; i<sizeA(tbb); i++) {
						if(tbb[i].idCommand==(int)ttt->hdr.idFrom) {
							ttt->hinst= NULL;
							ttt->lpszText= lng(ttt->hdr.idFrom+1000, toolNames[i]);
						}
					}
					break;
				}

#ifndef UNICODE
					//Linux
				case TTN_NEEDTEXTW:
				{
					TOOLTIPTEXTW *ttt= (LPTOOLTIPTEXTW)lParam;
					for(i=0; i<sizeA(tbb); i++){
						if(tbb[i].idCommand==(int)ttt->hdr.idFrom){
							ttt->hinst= NULL;
							MultiByteToWideChar(CP_ACP, 0, lng(ttt->hdr.idFrom+1000, toolNames[i]), -1, ttt->szText, sizeA(ttt->szText));
							ttt->szText[sizeA(ttt->szText)-1]=0;
						}
					}
					break;
				}
#endif

				case TBN_RESET:
					while(SendMessage(toolbar, TB_DELETEBUTTON, 0, 0));
					SendMessage(toolbar, TB_ADDBUTTONS, Ntool, (LPARAM)tbb);
					break;
			}
			break;
		}

		case WM_LBUTTONDOWN:
			board.lbuttondown(lParam);
			break;

		case WM_LBUTTONUP:
			board.lbuttonup(lParam);
			break;

		case WM_MOUSEMOVE:
			board.mousemove(lParam);
			break;

		case WM_LBUTTONDBLCLK:
			board.lbuttondblclk(lParam);
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			board.rbuttondown(lParam);
			break;

		case WM_MBUTTONDOWN:
			board.mbuttondown(lParam);
			break;

		case WM_MBUTTONUP:
			board.mbuttonup(lParam);
			break;

		case WM_DISPLAYCHANGE:
			scrW=LOWORD(lParam);
			scrH=HIWORD(lParam);
			break;

		case WM_CLOSE:
			SendMessage(hWin, WM_COMMAND, ID_EXIT, 0);
			if(helpVisible) HtmlHelp(0, NULL, 18, 0); //HH_CLOSE_ALL
			break;

		case WM_QUERYENDSESSION:
			writeini();
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_COMMAND:
			notif=HIWORD(wParam);
			wParam=LOWORD(wParam);
			if(setLang(wParam)) break;
			if(wParam>=ID_BKGND && wParam<ID_BKGND+5000){
				getMenuItem(fnBkgnd, _T("bkgnd\\"), wParam);
				loadBkgnd();
				break;
			}
			if(wParam>=ID_BACK && wParam<ID_BACK+5000){
				getMenuItem(fnBack, _T("back\\"), wParam);
				loadBack();
				break;
			}
			if(wParam>=ID_CARDS && wParam<ID_CARDS+5000){
				getMenuItem(fnCards, _T("cards\\"), wParam);
				loadCards();
				break;
			}
			if(wParam>=ID_CELL && wParam<ID_CELL+5000){
				getMenuItem(fnCell, _T("cell\\"), wParam);
				loadCell();
				break;
			}
			switch(wParam){
				case ID_LIST:
					if(games.len) dialogBox(IDD_GAMELIST, GameListProc);
					break;
				case ID_ABOUT:
					dialogBox(IDD_ABOUT, AboutProc);
					break;
				case ID_HELP_CONTENT:
				{
					TCHAR *buf=(TCHAR*)_alloca(2*MAX_PATH);
					getExeDir(buf, _T("help\\"));
					_tcscat(buf, lng(13, "en.chm"));

					TCHAR *buf2=(TCHAR*)_alloca(2*(MAX_PATH+24));
					_stprintf(buf2, _T("%s:Zone.Identifier:$DATA"), buf);
					DeleteFile(buf2); //delete only alternate data stream

					if(HtmlHelp(0, buf, 0, 0)) helpVisible=true;
					else if(ShellExecute(0, _T("open"), buf, 0, 0, SW_SHOWNORMAL)==(HINSTANCE)ERROR_FILE_NOT_FOUND){
						msglng(730, "Cannot open %s", buf);
					}
					break;
				}
				case ID_HELP_RULES:
					ShowWindow(rulesWnd, SW_SHOW);
					break;
				case ID_EXIT:
					board.saveAtExit();
					writeini();
					DestroyWindow(hWin);
					break;
				case ID_PLAYER:
					openUser();
					break;
				case ID_STATISTIC:
					playerFile.statistics();
					break;

				case ID_NEWGAME:
					if(board.game){ board.newSeed(); board.newGame(board.game); }
					else SendMessage(hWnd, WM_COMMAND, ID_LIST, 0);
					break;
				case ID_RANDOM_GAME:
					if(games.len){
						board.newGame(games[board.rand(games.len)]);
						generRules(board.game);
					}
					break;
				case ID_GAME_NUMBER:
					dialogBox(IDD_NUMBER, NewGameNumberProc);
					break;
				case ID_LASTGAME:
					loadRules();
					if(games.len){
						board.newGame(games[games.len-1]->name);
						generRules(games[games.len-1]);
					}
					break;
				case ID_RELOADGAME:
					loadRules();
					board.newGame(gameName);
					break;
				case ID_REFRESH:
					mustLoadRules=true;
					refresh();
					break;
				case ID_TOOLBAR:
					pause(true);
					SendMessage(toolbar, TB_CUSTOMIZE, 0, 0);
					pause(false);
					writeini();
					break;
				case ID_KEYS:
					dialogBox(IDD_KEYS, KeysDlgProc);
					break;
				case ID_OPTIONS:
					options();
					break;
				case ID_SHOWTOOLBAR:
					toolBarVisible=!toolBarVisible;
					ShowWindow(toolbar, toolBarVisible ? SW_SHOW : SW_HIDE);
					checkFlipMenu();
					invalidate();
					break;
				case ID_SHOWSTATUSBAR:	// ACC, it think the statusbar was meant to be toggled, so here it is.
					statusBarVisible = !statusBarVisible;
					ShowWindow(statusbar, statusBarVisible ? SW_SHOW : SW_HIDE);
					checkFlipMenu();
					invalidate();
					break;
				case ID_TOGGLEREGISTERY:	// ACC, toggle saving to registery/INI
					deleteini();
					configToRegistery = !configToRegistery;
					writeini();
					checkFlipMenu();
					break;
				case ID_DELINI:
					delreg=true;
					break;
				case ID_INCR_SEQ:
					sequenceDx++;
					seqHiddenDx++;
					sequenceDy++;
					seqHiddenDy++;
					invalidate();
					break;
				case ID_DECR_SEQ:
					if(sequenceDx > 0 && sequenceDy > 0){
						sequenceDx--;
						seqHiddenDx--;
						sequenceDy--;
						seqHiddenDy--;
						invalidate();
					}
					break;
				case ID_MIRROR_LR:
					flipx=!flipx;
					invalidate();
					checkFlipMenu();
					break;
				case ID_MIRROR_UD:
					flipy=!flipy;
					invalidate();
					checkFlipMenu();
					break;

				case ID_UNDO:
					board.undo();
					break;
				case ID_REDO:
					board.redo();
					break;
				case ID_UNDO_ALL:
					board.undoAll();
					break;
				case ID_REDO_ALL:
					board.redoAll();
					break;
				case ID_FAST_FORWARD:
					board.autoPlay(1); // ACC, added "fast forward", so it's possible to disable the global autoplay and yet get a hint when asked.
					break;
				case ID_NEXT_CARD:
					searchDirMenu(MENU_CARDS, 1);
					break;
				case ID_PREV_CARD:
					searchDirMenu(MENU_CARDS, 2);
					break;
				case ID_NEXT_BACK:
					searchDirMenu(MENU_BACK, 1);
					break;
				case ID_PREV_BACK:
					searchDirMenu(MENU_BACK, 2);
					break;
				case ID_NEXT_CELL:
					searchDirMenu(MENU_CELL, 1);
					break;
				case ID_PREV_CELL:
					searchDirMenu(MENU_CELL, 2);
					break;
				case ID_NEXT_BKGND:
					searchDirMenu(MENU_BKGND, 1);
					break;
				case ID_PREV_BKGND:
					searchDirMenu(MENU_BKGND, 2);
					break;
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);

	}
	return 0;
}

//------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	int i;
	HDC dc;
	MSG mesg;
	RECT rc;

	//detect Linux Wine
	HRGN rgn=CreateRoundRectRgn(0, 0, 10, 10, 1, 1);
	RoundRectRgnExtra = PtInRegion(rgn, 5, 9) ? 0 : 1;
	DeleteObject(rgn);

	inst = hInstance;

	//DPIAware
	typedef BOOL(WINAPI *TGetProcAddress)();
	TGetProcAddress getProcAddress = (TGetProcAddress)GetProcAddress(GetModuleHandle(_T("user32")), "SetProcessDPIAware");
	if(getProcAddress) getProcAddress();

	setlocale(LC_ALL, "");
	_tcscpy(fnCards, _T("cards\\standard.bmp"));
	_tcscpy(fnBack, _T("back\\blue.bmp"));
	_tcscpy(fnBkgnd, _T("bkgnd\\sandstone.bmp"));
	_tcscpy(fnCell, _T("cell\\simple.bmp"));
	GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &font);
	readini();
	//load common controls
	if(GetProcAddress(GetModuleHandle(_T("comctl32.dll")), "DllGetVersion")){
		INITCOMMONCONTROLSEX iccs;
		iccs.dwSize= sizeof(INITCOMMONCONTROLSEX);
		iccs.dwICC= ICC_LISTVIEW_CLASSES|ICC_BAR_CLASSES;
		InitCommonControlsEx(&iccs);
	}
	else{
		InitCommonControls();
	}

	// create the main window
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = WndMainProc;
	wc.hInstance = inst;
	wc.lpszClassName = CLASSNAME;
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
	wc.hbrBackground = 0;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon         = LoadIcon(inst, MAKEINTRESOURCE(IDI_MAINICON));
	if(!RegisterClass(&wc)){
#ifdef UNICODE
		msg("This version cannot run on Windows 95/98/ME.");
#else
		msg("RegisterClass failed");
#endif
		return 2;
	}
	scrW= GetSystemMetrics(SM_CXSCREEN);
	scrH= GetSystemMetrics(SM_CYSCREEN);
	aminmax(mainLeft, 0, scrW-300);
	aminmax(mainTop, 0, scrH-200);
	aminmax(gameListX, 0, scrW-200);
	aminmax(gameListY, 0, scrH-100);
	hWin = CreateWindow(CLASSNAME, title,
		WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
		mainLeft, mainTop, mainW, mainH, 0, 0, inst, 0);
	if(!hWin){
		msg("CreateWindow failed");
		return 3;
	}

	isHungAppWindow = (pIsHungAppWindow)GetProcAddress(GetModuleHandle(_T("user32.dll")), "IsHungAppWindow");

	//create window for game rules
	HMODULE richLib=LoadLibrary(_T("riched32.dll"));
	rulesWnd=CreateDialog(inst, MAKEINTRESOURCE(IDD_RULES), hWin, (DLGPROC)rulesProc);
	SetWindowPos(rulesWnd, 0, scrW-550, mainTop+30, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Initialize custom languages
	initLang();

	// Create status bar
	statusbar= CreateStatusWindow(WS_CHILD, 0, hWin, 1);
	static int parts[] ={100, 200, 350, 450, 540, 675, -1};
	dc=GetDC(hWin);
	for(i=0; i<sizeA(parts)-1; i++){
		parts[i]=parts[i]*GetDeviceCaps(dc, LOGPIXELSX)/96;
	}
	ReleaseDC(hWin, dc);
	SendMessage(statusbar, SB_SETPARTS, sizeA(parts), (LPARAM)parts);
	GetClientRect(statusbar, &rc);
	statusH= rc.bottom;
	if(statusBarVisible) ShowWindow(statusbar, SW_SHOW);

	//create tool bar
	i=sizeA(tbb);
	for(TBBUTTON *u=tbb; u<endA(tbb); u++){
		if(u->fsStyle==TBSTYLE_SEP) i--;
	}
	toolbar = CreateToolbarEx(hWin,
		WS_CHILD|TBSTYLE_TOOLTIPS|0x800, 2, i,
		inst, IDB_TOOLBAR, tbb, Ntool,
		16, 16, 16, 16, sizeof(TBBUTTON));
	//restore customized toolbar
	if(configToRegistery)
	{
		TBSAVEPARAMS sp;
		sp.hkr=HKEY_CURRENT_USER;
		sp.pszSubKey=subkey;
		sp.pszValueName=_T("toolbar");
		SendMessage(toolbar, TB_SAVERESTORE, FALSE, (LPARAM)&sp);
	}
	else{
		int n = Ntoolbar;
		if(n>0 && n<100){
			TBBUTTON* buttons = new TBBUTTON[n];
			if(GetPrivateProfileStruct(iniSection, _T("toolbar"), buttons, n*sizeof(TBBUTTON), fnIni)){
				while(SendMessage(toolbar, TB_DELETEBUTTON, 0, 0));
				SendMessage(toolbar, TB_ADDBUTTONS, (WPARAM)n, (LPARAM)buttons);
			}
			delete[] buttons;
		}
	}
	GetClientRect(toolbar, &rc);
	toolH= rc.bottom;
	if(toolBarVisible) ShowWindow(toolbar, SW_SHOW);

	setFont();
	accelChanged();

	/*TCHAR p[MAX_PATH];
	_tcscpy(p,_T("C:\\a\\cc\\00a.bmp"));
	readCardsFromFiles(p);
	saveBMP(_T("C:\\a\\.bmp"),bmpCards);
	return 0;*/

	//load images and rules
	refresh();
	loadRules();

	seedGlobal= time(0)+GetTickCount();

	//create default player profile
	if(!*playerFile.name){
		getExeDir(playerFile.name, _T("Default.xol"));
		if(GetFileAttributes(playerFile.name)==INVALID_FILE_ATTRIBUTES)
			playerFile.create(playerFile.name);
	}
	//read player profile from installation folder
	else if(GetFileAttributes(playerFile.name)==INVALID_FILE_ATTRIBUTES){
		getExeDir(fnTmp, cutPath(playerFile.name));
		if(GetFileAttributes(fnTmp)!=INVALID_FILE_ATTRIBUTES)
			_tcscpy(playerFile.name, fnTmp);
	}
	statusPlayer();

	//load previous game
	if(gameNum){
		playerFile.readPosition(&board, gameName, gameNum);
		playerFile.deletePosition(gameName, gameNum);
		suspendTime=10;
	}
	if(!board.number)
		PostMessage(hWin, WM_COMMAND, ID_LIST, 0);		// No game loaded, let's list some

	//show window
	curMove=LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSORMOVE));
	ShowWindow(hWin, maximized && cmdShow!=SW_SHOWMINNOACTIVE ? SW_SHOWMAXIMIZED : cmdShow);
	UpdateWindow(hWin);

	while(GetMessage(&mesg, NULL, 0, 0)==TRUE){
		if((!rulesWnd || !IsDialogMessage(rulesWnd, &mesg)) &&
			!TranslateAccelerator(hWin, haccel, &mesg)){
			TranslateMessage(&mesg);
			DispatchMessage(&mesg);
		}
	}
	if(delreg) deleteini();
	DestroyWindow(rulesWnd);
	FreeLibrary(richLib);
	DestroyAcceleratorTable(haccel);
	DeleteDC(dcCards);
	DeleteDC(dcBack);
	DeleteDC(dcStretch);
	DeleteDC(dcBackStretch);
	DeleteObject(bmpCards);
	DeleteObject(bmpBack);
	DeleteObject(hFont);
	DeleteObject(bkgndBrush);
	deleteGames();
	deleteDarray(menuCards); deleteDarray(menuBack); deleteDarray(menuBkgnd); deleteDarray(menuCell);
	cleanLang();
	return 0;
}
