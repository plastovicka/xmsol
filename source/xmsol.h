/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef xmlsolH
#define xmlsolH
#include "darray.h"
#include "lang.h"
#include "xml.h"
#include "resource.h"
//------------------------------------------------------------------
#define CLASSNAME TEXT("XMSolitaire")

#define Mcards 4*52
#define CARD_SUIT_MASK 0x30
#define CARD_SUIT_ROT 4
#define CARD_COLOR 2
#define CARD_RANK_MASK 0x0f
#define CARD_RANK_ROT 0
#define CARD_SUIT(c) (((c)&CARD_SUIT_MASK)>>CARD_SUIT_ROT)
#define CARD_RANK(c) (((c)&CARD_RANK_MASK)>>CARD_RANK_ROT)
#define MAKECARD(rank,suit) ((Tcard)(((rank)<<CARD_RANK_ROT)|((suit)<<CARD_SUIT_ROT)))
#define MAKECARD_START(d) MAKECARD((d)%13+1,((d)/13)%4)

enum{ HEART, DIAMOND, CLUB, SPADE };
enum{ RANK_ROT=0, RANK_MASK=0xff, RANK_NO=14, RANK_RANDOM, RANK_RANDOMPLUS1, RANK_SAMEBASE, RANK_KING4, RANK_AK };
enum{ CLR_ROT=8, CLR_MASK=0xf00, CLR_HEART=1, CLR_DIAMOND, CLR_CLUB, CLR_SPADE, CLR_RED, CLR_BLACK, CLR_HDCS, CLR_HSDC, CLR_CDHS };
enum{ SUIT_ROT=12, SUIT_MASK=0xf000, SUIT_SAME=1, SUIT_DIFF, SUIT_SAMECOLOR, SUIT_DIFFCOLOR, SUIT_MAX };
enum{ SEQ_ROT=16, SEQ_MASK=0xf0000, CONT_ROT=20, CONT_MASK=0x100000, SEQ1_MASK=0x1f0000, SEQ_NO=1, SEQ_DOWN, SEQ_UP, SEQ_UPDOWN, SEQ_DOWN2, SEQ_UP2, SEQ_UPDOWN2, SEQ_RANK, SEQ_UPDOWNRANK, SEQ_OSMOSIS, SEQ_SYNCH, SEQ_HIGHER, SEQ_LOWER, SEQ_UPRANK, SEQ_DOWNRANK, SEQ_MAX };
enum{ GROUP_ROT=21, GROUP_MASK=0xe00000, GROUP_NO=1, GROUP_NOEMPTY, GROUP_ANYSEQ, GROUP_ALL, GROUP_TOEMPTY, GROUP_ANYFREECELLSEQ, GROUP_PILE, GROUP_MAX };
enum{ DEST_ROT=25, DEST_MASK=0x7e000000 };
enum{ CELL_CELL, CELL_FOUNDATION, CELL_TABLEAU, CELL_RESERVE, CELL_STOCK, CELL_WASTE };
enum{ DIR_NO, DIR_DOWN, DIR_RIGHT, DIR_UP, DIR_LEFT };
enum{ PREV_DOWN=256, PREV_RIGHT, PREV_UP, PREV_LEFT };
enum{ HIDE_ALL=256, HIDE_ODD, HIDE_EVEN };
enum{ SHAPE_LEFT, SHAPE_RIGHT, SHAPE_CENTER, SHAPE_INCR, SHAPE_DECR, SHAPE_INCR2, SHAPE_DECR2, SHAPE_FIRST, SHAPE_LAST, SHAPE_PYRAMID, SHAPE_BRAID, SHAPE_BIND, SHAPE_SIDE };
enum{ STATUS_TIME, STATUS_MOVES, STATUS_DONE, STATUS_SCORE, STATUS_NCARD, STATUS_SEED, STATUS_MSG };
enum{ SECTION_NAME, SECTION_LOG, SECTION_SAVE };
enum{ DEAL_FILL=256, DEAL_GAPS, DEAL_BOTTOMTOTOP, DEAL_MAX, DEAL_START, DEAL_START_COL, DEAL_START1 };
enum{ PAIR_SUIT=256, PAIR_RANK, PAIR_RANKCOLOR, PAIR_MAX };
enum{ ANIM_NO, ANIM_RIGHTCLICK, ANIM_AUTOPLAY, ANIM_UNDO, ANIM_STOCK, ANIM_MAX };
enum{ SHUFFLE_NO, SHUFFLE_RAND };
enum{ UNDER_NEXT=257, UNDER_NEXT1, UNDER_WASTE };
enum{ MENU_CARDS, MENU_BACK, MENU_BKGND, MENU_CELL };
enum{ AUTO_NO, AUTO_YES, AUTO_BASE };
enum{ MOVE_FIRST=252, MOVE_UNHIDE, MOVE_HIDE, MOVE_SHUFFLE };
enum{ ONHID_AUTO, ONHID_BLOCKED, ONHID_ANY, ONHID_K };

typedef TCHAR TfileName[MAX_PATH];
typedef char Tcard;
typedef int (CALLBACK *TsortFunc)(LPARAM, LPARAM, LPARAM);
struct Tgame;

struct Tcell0 {
	int type, x, y, dir, spacing, fill, hideStyle, deal, redeal, shuffle, redealCount, startCount, shape, maxCount, maxShow, blockedBy[2], prevDir, autoplay, onhid;
	int inRule, outRule, sameRule, prevRule, startRule;
};

struct Tcell : public Tcell0 {
	int Ncards;
	int prevInd;
	int tmp;
	Tcard cards[Mcards];
	bool hidden[Mcards];
};

struct TcellElem : public TxmlElem, public Tcell0 {
	int w, h, repeat, lines;

	TcellElem(int type);
	void addAttr(TCHAR *name, TCHAR *value);
	void addChild(TxmlElem *elem);
	void setText(TCHAR *s);
};

struct Tstat {
	TCHAR *name;
	int played, won, lost, percent, time, moves, score, cards, date, scoreWon, timeWon, movesWon;
};

struct Tanim {
	int speed;
	int enabled;
};

struct TundoRec {
	int src, dest, ind, destInd;
	int group;
	bool unhide;
};

class Tboard {
public:
	Darray<Tcell> cells;
	Tgame *game;
	unsigned number;
	int randomRank;
	int waste;
	int score, scoreWrongMove, scoreUndo, scoreUnhide, scoreRedeal;
	int moves, doneCards, donePercent, playtime;
	bool finished;
	int undoPos, redoPos;
	Darray<TundoRec> undoRec;
	unsigned seed;

	bool dragging, revealing;
	int dragS, dragCell, dragInd, dragCount, dragX, dragY, dragDx, dragDy, revealCell, revealInd;

	Tboard(){ finished=true; dragging=revealing=false; }
	unsigned rand(unsigned n);
	unsigned randUndo(unsigned n);
	void newSeed();
	void getColumnCoord(int &x, int &y, int cell, int ind);
	int getColumnStart(Tcell *c);
	void invalidateColumn(int cell, int ind, int n);
	void animate(int dest, int src, int ind, int anim);
	int hitTest(int mx, int my);
	int hitTest(LPARAM lP);
	int testDrag();
	void lbuttondown(LPARAM lParam);
	void lbuttonup(LPARAM lParam);
	void mousemove(LPARAM lParam);
	void lbuttondblclk(LPARAM lParam);
	void rbuttondown(LPARAM lParam);
	void mbuttondown(LPARAM lParam);
	void mbuttonup(LPARAM lParam);
	int unhide(int cell, int ind);
	void paintBoard(HDC dc, RECT *rcPaint);
	int freecell(int dest);
	int king4();
	int baseRank();
	int autoBase(Tcell *t);
	int rankTest(int rule, Tcard card);
	int testMove(int dest, int src, int ind, int drag=0);
	int testStart(Tcell *c, int d, int ind);
	void move2(int dest, int src, int ind);
	void move1(int dest, int src, int ind);
	int move(int dest, int src, int ind);
	int animMove(int dest, int src, int ind, int anim);
	void animMove1(int dest, int src, int ind, int anim);
	int chooseDest(int src);
	int stockClick(int src);
	void dblclkMove(int src);
	int rightclkMove(int src, int ind);
	void saveAtExit();
	int cellFinished(Tcell *c);
	void calcDone();
	bool finishFoundation();
	bool finishFullFoundation();
	bool finishFan();
	int totalCards();
	void autoPlay(int once=0);
	bool deal(Tcell *c, bool *bitmask, int total);
	void newGame(Tgame *_game);
	void newGame(TCHAR *name);
	bool undo1();
	bool redo1();
	void undo();
	void redo();
	void undoAll();
	void redoAll();
};


struct Tgame : public TxmlElem {
	TCHAR *name;
	int pair, decks, suit, rank, autoplay;
	Darray<TcellElem*> cells;
	bool (Tboard::*checkFinish)();
	Tstat *stat;
	int cards, stocks, ruleSeq, ruleSuit, ruleGroup;

	Tgame();
	~Tgame();
	void addAttr(TCHAR *attr, TCHAR *value);
	void addChild(TxmlElem *elem);
	bool check();
	int Nsuit();
	int Nrank();
};

struct TplayerFileHeader {
	char text[11];
	BYTE Nsection;
	WORD version;
	WORD headerLen;
	DWORD nameOffset, nameLen, nameNum, nameStruct;
	DWORD logOffset, logLen, logNum, logStruct;
	DWORD saveOffset, saveLen, saveNum, saveStruct;
};

struct TgameStatRec {
	DWORD seed;
	DWORD date;
	WORD nameId;
	WORD time;
	WORD moves;
	BYTE won, wrongMove, redeal, undo, unhide, done;
	void init(Tboard *b);
};

struct TsavedMoveRec {
	BYTE src, dest, ind, moveRel;
};

struct TbestScore {
	int score;
	TgameStatRec *rec;
};

class TplayerFile {
public:
	HANDLE file;
	int error;
	DWORD fileSize;
	Darray<Tstat> gameNames;
	TfileName name;
	TplayerFileHeader pfh;
	int totalPlayed, totalWon, totalTime, totalMoves;

	TplayerFile(){ file=INVALID_HANDLE_VALUE; }
	~TplayerFile();
	bool open();
	bool create(TCHAR *fn);
	bool read(void *ptr, DWORD len);
	void write(void *ptr, DWORD len);
	void write(HANDLE h, void *ptr, DWORD len);
	void seek(DWORD pos);
	void seek(HANDLE h, DWORD pos);
	void close();
	int getNameId(TCHAR *s);
	BYTE *findPosition(BYTE *a, TCHAR *game_name, DWORD num);
	void readNames();
	void deleteNames();
	bool readStat(int action);
	bool readGameStat();
	void readPosition(Tboard *b, TCHAR *game_name, DWORD num);
	int saveName(TCHAR *s);
	void saveGameStat(Tboard *b);
	void savePosition(Tboard *b);
	void deletePosition(TCHAR *game_name, DWORD num);
	void addRecord(int section, void *ptr, DWORD len);
	void statistics();
	void bestScores(int whichGame);
};

typedef int Tstat::*TstatMember;
typedef int Tgame::*TgameMember;

//------------------------------------------------------------------
template <class T> inline void aminmax(T &x, int l, int h){
	if(x<l) x=l;
	if(x>h) x=h;
}

template <class T> inline void amax(T &x, int h){
	if(x>h) x=h;
}

template <class T> inline void amin(T &x, int l){
	if(x<l) x=l;
}

//------------------------------------------------------------------
#ifdef UNICODE 
#define convertT2W(t,w) \
    WCHAR *w=t
#define convertW2T(w,t) \
    TCHAR *t=w
#define convertA2T(a,t) \
    int cnvlen##t=strlen(a)+1;\
    TCHAR *t=(TCHAR*)_alloca(2*cnvlen##t);\
    MultiByteToWideChar(CP_ACP, 0, a, -1, t, cnvlen##t);
#define convertT2A(t,a) \
    int cnvlen##a=wcslen(t)+1;\
    char *a=(char*)_alloca(cnvlen##a);\
    WideCharToMultiByte(CP_ACP, 0, t, -1, a, cnvlen##a, 0,0);
#else
#define convertT2A(t,a) \
    char *a=t
#define convertA2T(a,t) \
    TCHAR *t=a
#define convertW2T(w,t) \
    int cnvlen##t=wcslen(w)+1;\
    TCHAR *t=(TCHAR*)_alloca(cnvlen##t);\
    WideCharToMultiByte(CP_ACP, 0, w, -1, t, cnvlen##t, 0,0);
#define convertT2W(t,w) \
    int cnvlen##w=strlen(t)+1;\
    WCHAR *w=(WCHAR*)_alloca(2*cnvlen##w);\
    MultiByteToWideChar(CP_ACP, 0, t, -1, w, cnvlen##w);
#endif

extern "C"{
	HWND WINAPI HtmlHelpA(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData);
	HWND WINAPI HtmlHelpW(HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData);
#ifdef UNICODE
#define HtmlHelp  HtmlHelpW
#else
#define HtmlHelp  HtmlHelpA
#endif
}
//------------------------------------------------------------------

extern int globalAutoplay, suspendTime, notdraw, speedAccel,
SCORE_UNDO, SCORE_WRONG_MOVE, SCORE_REDEAL, SCORE_UNHIDE, SCORE_FOUNDATION, SCORE_TIME, SCORE_BONUS;
extern bool mustLoadRules;
extern Darray<Tgame*> games;
extern TCHAR gameName[128];
extern HWND hWin, rulesWnd;
extern Tanim animation[ANIM_MAX];
extern TplayerFile playerFile;
extern Tboard board;
extern char *suitWord[SUIT_MAX], *seqWord[SEQ_MAX], *pairWord[PAIR_MAX-256], *groupWord[GROUP_MAX];
extern unsigned seedGlobal;
//------------------------------------------------------------------

//game.cpp
void readRules(TCHAR *fn);
void loadRules();
void generRules(Tgame *g);
Tgame *findGame(TCHAR *name);
void deleteGames();
int calcScore(int unhide, int done, int redeal, int undo, int wrong, int time, bool won, int Ncards);

//xmsol.cpp
void invalidate();
void status(int i, TCHAR *txt, ...);
void statusScore();
void statusDone();
void statusMoves();
void statusTime();
void statusNcard();
void statusSeed();
void updateStatus();
int dtprintf(Darray<TCHAR> &buf, TCHAR *fmt, ...);
void printDate(TCHAR *buf, int n, time_t date);
TCHAR *cutExt(TCHAR *fn);

//xml.cpp
TCHAR *dupStr(TCHAR *s);

//bitmap.cpp
HBITMAP loadBMP(TCHAR *fn, int resource);
HBITMAP readBMP(TCHAR *fn, int resource, HDC, int *pwidth, int *pheight);
bool isImage(TCHAR *file);

#endif
