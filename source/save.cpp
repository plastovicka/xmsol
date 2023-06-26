/*
	(C) Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "hdr.h"
#include "xmsol.h"

#define MAXMEM 50000000
const int Mbest=10;
TplayerFile playerFile;

//------------------------------------------------------------------

void TplayerFile::deleteNames()
{
	for(int j=0; j<gameNames.len; j++){
		delete[] gameNames[j].name;
	}
	gameNames.reset();
}

TplayerFile::~TplayerFile()
{
	deleteNames();
}

void TplayerFile::readNames()
{
	DWORD n, i;
	BYTE *a=0, *p;
	TCHAR *s;
	Tstat *g;

	deleteNames();
	if(error) return;
	try{
		a=p=new BYTE[pfh.nameLen];
		seek(pfh.nameOffset);
		read(a, pfh.nameLen);
		for(i=pfh.nameNum; i>0; i--){
			n=*p;
			p+=pfh.nameStruct;
			g=gameNames++;
			g->name= s= new TCHAR[n+1];
#ifdef UNICODE 
			memcpy(s,p,n*2);
#else
			WideCharToMultiByte(CP_ACP, 0, (WCHAR*)p, n, s, n, 0, 0);
#endif
			p+=n*2;
			s[n]=0;
		}
	} catch(...){}
	delete[] a;
}

BYTE *TplayerFile::findPosition(BYTE *a, TCHAR *game_name, DWORD num)
{
	int i, id;
	BYTE *p, *e, *q;
	DWORD n;
	TgameStatRec *g;
	BYTE *result=0;

	id=getNameId(game_name);
	if(id>=0){
		p=a;
		e=a+pfh.saveLen;
		seek(pfh.saveOffset);
		read(a, pfh.saveLen);
		if(!error){
			for(i=0; i<(int)pfh.saveNum; i++){
				g=(TgameStatRec*)p;
				p+=pfh.logStruct;
				if(p+4>e) break;
				n=*(DWORD*)p;
				if(n>MAXMEM) break;
				p+=4;
				q=p;
				p+= n*pfh.saveStruct;
				if(p>e) break;
				if(g->seed==num && g->nameId==(WORD)id){
					result=q;
					break;
				}
			}
		}
	}
	return result;
}

void TplayerFile::readPosition(Tboard *b, TCHAR *game_name, DWORD num)
{
	int i, n, m;
	BYTE *a, *p;
	TgameStatRec *g;
	TsavedMoveRec *s;
	TundoRec *r;

	if(open()){
		if(pfh.saveLen<MAXMEM){
			a=new BYTE[pfh.saveLen];
			p=findPosition(a, game_name, num);
			if(p){
				n=*(DWORD*)(p-4);
				g=(TgameStatRec*)(p-4-pfh.logStruct);
				assert(g->seed==num);
				close();
				b->seed= g->seed;
				b->newGame(game_name);
				if(b->game){
					b->finished= g->won!=0;
					b->scoreWrongMove= g->wrongMove;
					b->scoreRedeal= g->redeal;
					b->scoreUndo= g->undo;
					b->scoreUnhide= g->unhide;
					b->doneCards= g->done;
					b->playtime= g->time;
					m=0;
					for(i=0; i<n; i++){
						s=(TsavedMoveRec*)p;
						r=b->undoRec++;
						r->src=s->src;
						r->dest=s->dest;
						r->ind=s->ind;
						if(s->moveRel) m++;
						r->group=m;
						p+= pfh.saveStruct;
					}
					b->undoPos=0;
					b->redoPos=b->undoRec.len;
					notdraw++;
					while(b->moves < g->moves && b->redo1());
					assert(b->moves==g->moves);
					notdraw--;
					b->calcDone();
				}
				updateStatus();
			}
			delete[] a;
		}
		close();
	}
}

void TplayerFile::statistics()
{
	Darray<TCHAR> t;
	int n;

	if(readGameStat()){
		n=totalPlayed;
		if(n){
			dtprintf(t, _T("%s\r\n\n%s: %u\r\n%s: %u\r\n%s: %u\r\n%s: %.2f%%\r\n\n%s: %u\r\n%s: %u:%02u\r\n%s: %u\r\n%s: %u\r\n"),
				name,
				lng(960, "Games played"), n,
				lng(961, "Games won"), totalWon,
				lng(962, "Games lost"), n-totalWon,
				lng(963, "Winning percentage"), 100*double(totalWon)/n,
				lng(964, "Total time in hours"), totalTime/3600,
				lng(965, "Time per game"), (totalTime/n)/60, (totalTime/n)%60,
				lng(966, "Total moves"), totalMoves,
				lng(967, "Moves per game"), totalMoves/n
				);
			msg("%s", (TCHAR*)t);
		}
	}
}

bool TplayerFile::readStat(int whichGame)
{
	BYTE *a=0, *p;
	int i, j, N, k, s, whichId;
	Tstat *g;
	Tgame *game;
	TgameStatRec *r;
	TbestScore *b, *best=0;
	Darray<TCHAR> t;
	bool result=false;

	if(open()){
		if(pfh.logLen<MAXMEM){
			a=p=new BYTE[pfh.logLen];
			seek(pfh.logOffset);
			read(a, pfh.logLen);
			if(!error && pfh.logNum*pfh.logStruct<=pfh.logLen){
				whichId=-1;
				N=gameNames.len;
				for(i=0; i<N; i++){
					g=&gameNames[i];
					g->played=g->won=g->score=g->time=g->moves=g->date=g->scoreWon=g->timeWon=g->movesWon=0;
					g->cards=1;
					if(whichGame>=0 && !_tcscmp(games[whichGame]->name, g->name)){
						whichId=i;
					}
					for(j=0; j<games.len; j++){
						game=games[j];
						if(!_tcscmp(game->name, g->name)){
							game->stat=g;
							g->cards=game->cards;
							break;
						}
					}
				}
				if(whichId>=0){
					best= new TbestScore[Mbest];
					for(i=0; i<Mbest; i++){
						best[i].score=0;
						best[i].rec=0;
					}
				}
				for(i=pfh.logNum; i>0; i--){
					r=(TgameStatRec*)p;
					p+=pfh.logStruct;
					if(r->nameId<N){
						g=&gameNames[r->nameId];
						g->played++;
						s=calcScore(r->unhide, r->done, r->redeal, r->undo, r->wrongMove, r->time, r->won!=0, g->cards);
						g->score+=s;
						if(r->won){
							g->scoreWon+=s;
							g->timeWon+=r->time;
							g->movesWon+=r->moves;
							g->won++;
						}
						g->time+=r->time;
						g->moves+=r->moves;
						amin(g->date, r->date);
						if(r->nameId==whichId){
							for(j=Mbest-1; j>=0; j--){
								if(s<=best[j].score) break;
							}
							j++;
							if(j<Mbest){
								for(k=Mbest-1; k>j; k--){
									best[k]=best[k-1];
								}
								best[j].score=s;
								best[j].rec=r;
							}
						}
					}
				}
				totalPlayed=totalWon=totalTime=totalMoves=0;
				for(i=0; i<N; i++){
					g=&gameNames[i];
					k=g->played;
					if(k){
						totalPlayed+=k;
						totalWon+=g->won;
						totalTime+=g->time;
						totalMoves+=g->moves;
						g->score/=k;
						g->time/=k;
						g->moves/=k;
						g->percent= 100*g->won/k;
					}
					k=g->won;
					if(k){
						g->scoreWon/=k;
						g->timeWon/=k;
						g->movesWon/=k;
					}
					g->lost= g->played-g->won;
				}
				result=true;
				if(whichId>=0){
					dtprintf(t, _T("%s\r\n\n%s"), games[whichGame]->name,
						lng(538, "Score\tTime\tDate"));
					for(i=0; i<Mbest; i++){
						b=&best[i];
						if(!b->rec) break;
						TCHAR buf[64];
						printDate(buf, sizeA(buf), b->rec->date);
						k= b->rec->time;
						dtprintf(t, _T("\r\n%d\t%u:%02u\t%s"), b->score, k/60, k%60, buf);
					}
					delete[] best;
					msg("%s", (TCHAR*)t);
				}
			}
			delete[] a;
		}
		close();
	}
	return result;
}

bool TplayerFile::readGameStat()
{
	return readStat(-1);
}

void TplayerFile::bestScores(int whichGame)
{
	readStat(whichGame);
}

//------------------------------------------------------------------
int TplayerFile::getNameId(TCHAR *s)
{
	int i;

	for(i=0; i<gameNames.len; i++){
		if(!_tcscmp(s, gameNames[i].name)) return i;
	}
	return -1;
}

int TplayerFile::saveName(TCHAR *s)
{
	int i, len, r;
	char *buf;
	WCHAR *w;

	i=getNameId(s);
	if(i>=0) return i;
	if(file==INVALID_HANDLE_VALUE) return -1;
	if(pfh.nameNum>0xffff) return -2;
	//write new game name to the file
	len=_tcslen(s);
	amax(len, 255);
	r= len*2+pfh.nameStruct;
	buf= new char[r+2];
	buf[0]=(char)len;
	buf[1]=0;
	w= (WCHAR*)(buf+pfh.nameStruct);
#ifdef UNICODE 
	memcpy(w,s,len*2);
#else
	MultiByteToWideChar(CP_ACP, 0, s, len, w, len);
#endif
	addRecord(SECTION_NAME, buf, r);
	delete[] buf;
	if(error) return -3;
	(gameNames++)->name=dupStr(s);
	return pfh.nameNum-1;
}

BYTE toByte(int i)
{
	return (BYTE)min(i, 255);
}

WORD toWord(int i)
{
	return (WORD)min(i, 65535);
}

void TgameStatRec::init(Tboard *b)
{
	date= ::time(0);
	seed= b->number;
	time= toWord(b->playtime);
	moves= toWord(b->moves);
	won= b->finished;
	wrongMove= toByte(b->scoreWrongMove);
	redeal= toByte(b->scoreRedeal);
	undo= toByte(b->scoreUndo);
	unhide= toByte(b->scoreUnhide);
	done= toByte(b->doneCards);
}

void TplayerFile::saveGameStat(Tboard *b)
{
	int i;
	TgameStatRec r;

	if(b->game){
		if(open()){
			i=saveName(b->game->name);
			if(i>=0){
				r.nameId=(WORD)i;
				r.init(b);
				addRecord(SECTION_LOG, &r, sizeof(TgameStatRec));
			}
			close();
		}
	}
}

void TplayerFile::deletePosition(TCHAR *game_name, DWORD num)
{
	BYTE *a, *p;
	DWORD n;
	int o, k;

	if(open()){
		if(pfh.saveLen<MAXMEM){
			a=new BYTE[pfh.saveLen];
			p=findPosition(a, game_name, num);
			if(p){
				p-=4;
				n= *(DWORD*)p;
				p-= pfh.logStruct;
				k= n*pfh.saveStruct + 4+pfh.logStruct;
				o= int(p-a);
				seek(pfh.saveOffset + o);
				write(p+k, pfh.saveLen-o-k);
				if(!error){
					pfh.saveLen-= k;
					pfh.saveNum--;
					seek(((char*)&pfh.saveLen)-(char*)&pfh);
					write(&pfh.saveLen, 8);
				}
			}
			delete[] a;
		}
		close();
	}
}

void TplayerFile::savePosition(Tboard *b)
{
	int i, id, n, k;
	TundoRec *r;
	TsavedMoveRec *s;
	TgameStatRec *g;
	BYTE *a, *p;
	DWORD *d;

	if(b->game){
		deletePosition(b->game->name, b->number);
		if(open()){
			id=saveName(b->game->name);
			if(id>=0){
				k=b->undoRec.len;
				n=pfh.logStruct+4+k*pfh.saveStruct;
				p= a= new BYTE[n];
				g=(TgameStatRec*)p;
				g->nameId=(WORD)id;
				g->init(b);
				p+=pfh.logStruct;
				d=(DWORD*)p;
				*d=k;
				p+=4;
				for(i=0; i<k; i++){
					r=&b->undoRec[i];
					s=(TsavedMoveRec*)p;
					s->src=(BYTE)r->src;
					s->dest=(BYTE)r->dest;
					s->ind=(BYTE)r->ind;
					s->moveRel= (BYTE)((i>0) ? r->group-(r-1)->group : 0);
					p+=pfh.saveStruct;
				}
				addRecord(SECTION_SAVE, a, n);
				delete[] a;
			}
			close();
		}
	}
}

//------------------------------------------------------------------

bool TplayerFile::create(TCHAR *fn)
{
	file=CreateFile(fn, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
	if(file==INVALID_HANDLE_VALUE){
		error=8;
		msglng(733, "Cannot create file %s", fn);
		return false;
	}
	error=0;
	_tcscpy(name, fn);
	memcpy(pfh.text, "XMSolitaire", 11);
	pfh.Nsection=3;
	pfh.version=1;
	pfh.nameStruct=2;
	pfh.logStruct=sizeof(TgameStatRec);
	assert(sizeof(TgameStatRec)==20);
	pfh.saveStruct=sizeof(TsavedMoveRec);
	assert(sizeof(TsavedMoveRec)==4);
	pfh.nameOffset=pfh.logOffset=pfh.saveOffset=pfh.headerLen=sizeof(TplayerFileHeader);
	pfh.nameLen=pfh.logLen=pfh.saveLen=pfh.nameNum=pfh.logNum=pfh.saveNum=0;
	write(&pfh, sizeof(TplayerFileHeader));
	close();
	return !error;
}

//
// Load an existing XOL (player profile)
//
bool TplayerFile::open()
{
	if(!*name) return false;
	file=CreateFile(name, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if(file==INVALID_HANDLE_VALUE){
		error=8;
		msglng(730, "Cannot open file %s", name);
	}
	else{
		error=0;
		fileSize=GetFileSize(file, 0);
		read(&pfh, sizeof(TplayerFileHeader));
		if(!error && strncmp(pfh.text, "XMSolitaire", 11)){
			error=8;
			msglng(731, "Bad format of file %s", name);
		}
		readNames();
		return true;
	}
	close();
	return false;
}

void TplayerFile::seek(HANDLE h, DWORD pos)
{
	if(SetFilePointer(h, pos, 0, FILE_BEGIN)==0xFFFFFFFF){
		if(!error) msglng(731, "Error in file %s", name);
		error|=4;
	}
}

void TplayerFile::seek(DWORD pos)
{
	seek(file, pos);
}

bool TplayerFile::read(void *ptr, DWORD len)
{
	DWORD r;
	ReadFile(file, ptr, len, &r, 0);
	if(r!=len){
		if(!error) msglng(754, "Error reading file %s", name);
		error|=1;
		return false;
	}
	return true;
}

void TplayerFile::write(HANDLE h, void *ptr, DWORD len)
{
	DWORD w;
	if(error) return;
	WriteFile(h, ptr, len, &w, 0);
	if(w!=len){
		if(!error) msglng(734, "Error writing file %s", name);
		error|=2;
	}
}

void TplayerFile::write(void *ptr, DWORD len)
{
	write(file, ptr, len);
}

void TplayerFile::close()
{
	CloseHandle(file);
	file=INVALID_HANDLE_VALUE;
}

void TplayerFile::addRecord(int section, void *ptr, DWORD len)
{
	HANDLE h;
	char *a;
	DWORD *pSect, *o;
	DWORD offset, incr;
	int i;
	TfileName fnTmp;
	const int d=(&pfh.logOffset-&pfh.nameOffset);

	if(error) return;
	pSect= &pfh.nameOffset+d*section;
	offset=pSect[0]+pSect[1];
	pSect[1]+=len;
	pSect[2]++;
	if(section+1>=pfh.Nsection || offset+len<=pSect[d]){
		seek(offset);
		write(ptr, len);
		seek(0);
		write(&pfh, pfh.headerLen);
	}
	else{
		_tcscpy(fnTmp, name);
		_tcscat(fnTmp, _T(".tmp"));
		h=CreateFile(fnTmp, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if(h==INVALID_HANDLE_VALUE){
			msglng(733, "Cannot create file %s", fnTmp);
		}
		else{
			incr=max(len, 2048);
			o=pSect;
			for(i=section+1; i<pfh.Nsection; i++){
				o+=d;
				(*o)+=incr;
			}
			if(fileSize<offset) fileSize=offset;
			a=new char[max(offset, fileSize-offset)];
			seek(0);
			read(a, offset);
			memcpy(a, &pfh, sizeof(pfh));
			write(h, a, offset);
			write(h, ptr, len);
			seek(h, offset+incr);
			read(a, fileSize-offset);
			write(h, a, fileSize-offset);
			SetEndOfFile(h);
			delete[] a;
			if(error){
				DeleteFile(fnTmp);
			}
			else{
				fileSize+=incr;
				CloseHandle(h);
				close();
				DeleteFile(name);
				MoveFile(fnTmp, name);
				open();
			}
		}
	}
	if(error){
		seek(0);
		read(&pfh, sizeof(TplayerFileHeader));
	}
}
//------------------------------------------------------------------

