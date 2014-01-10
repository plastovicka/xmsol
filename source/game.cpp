/*
  (C) 2005  Petr Lastovicka

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License.
*/
#include "hdr.h"
#include <richedit.h>
#include "xmsol.h"
//------------------------------------------------------------------

struct Tword {
  char *name;
  int value;
  int mask;
  int rot;
};

Tword wordA[]={
  {"a",1,RANK_MASK,RANK_ROT},
  {"2",2,RANK_MASK,RANK_ROT},
  {"3",3,RANK_MASK,RANK_ROT},
  {"4",4,RANK_MASK,RANK_ROT},
  {"5",5,RANK_MASK,RANK_ROT},
  {"6",6,RANK_MASK,RANK_ROT},
  {"7",7,RANK_MASK,RANK_ROT},
  {"8",8,RANK_MASK,RANK_ROT},
  {"9",9,RANK_MASK,RANK_ROT},
  {"10",10,RANK_MASK,RANK_ROT},
  {"j",11,RANK_MASK,RANK_ROT},
  {"q",12,RANK_MASK,RANK_ROT},
  {"k",13,RANK_MASK,RANK_ROT},
  {"no",RANK_NO,RANK_MASK,RANK_ROT},
  {"rand",RANK_RANDOM,RANK_MASK,RANK_ROT},
  {"rand+1",RANK_RANDOMPLUS1,RANK_MASK,RANK_ROT},
  {"base",RANK_SAMEBASE,RANK_MASK,RANK_ROT},
  {"anyrank",0,RANK_MASK,RANK_ROT},
  {"king4",RANK_KING4,RANK_MASK,RANK_ROT},
  {"ak",RANK_AK,RANK_MASK,RANK_ROT},

  {"anysuit",0,SUIT_MASK,SUIT_ROT},
  {"suit",SUIT_SAME,SUIT_MASK,SUIT_ROT},
  {"diffsuit",SUIT_DIFF,SUIT_MASK,SUIT_ROT},
  {"color",SUIT_SAMECOLOR,SUIT_MASK,SUIT_ROT},
  {"alter",SUIT_DIFFCOLOR,SUIT_MASK,SUIT_ROT},

  {"anyseq",0,SEQ1_MASK,SEQ_ROT},
  {"noseq",SEQ_NO,SEQ1_MASK,SEQ_ROT},
  {"rank",SEQ_RANK,SEQ1_MASK,SEQ_ROT},
  {"down",SEQ_DOWN,SEQ_MASK,SEQ_ROT},
  {"up",SEQ_UP,SEQ_MASK,SEQ_ROT},
  {"updown",SEQ_UPDOWN,SEQ_MASK,SEQ_ROT},
  {"down2",SEQ_DOWN2,SEQ_MASK,SEQ_ROT},
  {"up2",SEQ_UP2,SEQ_MASK,SEQ_ROT},
  {"updown2",SEQ_UPDOWN2,SEQ_MASK,SEQ_ROT},
  {"updownequ",SEQ_UPDOWNRANK,SEQ_MASK,SEQ_ROT},
  {"osmosis",SEQ_OSMOSIS,SEQ_MASK,SEQ_ROT},
  {"synch",SEQ_SYNCH,SEQ_MASK,SEQ_ROT},
  {"lower",SEQ_LOWER,SEQ_MASK,SEQ_ROT},
  {"higher",SEQ_HIGHER,SEQ_MASK,SEQ_ROT},
  {"upequ",SEQ_UPRANK,SEQ_MASK,SEQ_ROT},
  {"downequ",SEQ_DOWNRANK,SEQ_MASK,SEQ_ROT},
  {"cont",1,CONT_MASK,CONT_ROT},

  {"group",0,GROUP_MASK,GROUP_ROT},
  {"all",GROUP_ALL,GROUP_MASK,GROUP_ROT},
  {"nogroup",GROUP_NO,GROUP_MASK,GROUP_ROT},
  {"noempty",GROUP_NOEMPTY,GROUP_MASK,GROUP_ROT},
  {"anygroup",GROUP_ANYSEQ,GROUP_MASK,GROUP_ROT},
  {"pilegroup",GROUP_PILE,GROUP_MASK,GROUP_ROT},
  {"fillempty",GROUP_TOEMPTY,GROUP_MASK,GROUP_ROT},
  {"anygroupfreecell",GROUP_ANYFREECELLSEQ,GROUP_MASK,GROUP_ROT},

  {"stock",1,1<<(CELL_STOCK+DEST_ROT),CELL_STOCK+DEST_ROT},
  {"reserve",1,1<<(CELL_RESERVE+DEST_ROT),CELL_RESERVE+DEST_ROT},
  {"tableau",1,1<<(CELL_TABLEAU+DEST_ROT),CELL_TABLEAU+DEST_ROT},
  {"waste",1,1<<(CELL_WASTE+DEST_ROT),CELL_WASTE+DEST_ROT},
  {"foundation",1,1<<(CELL_FOUNDATION+DEST_ROT),CELL_FOUNDATION+DEST_ROT},
  {"cell",1,1<<(CELL_CELL+DEST_ROT),CELL_CELL+DEST_ROT},
  
  {"spade",CLR_SPADE,CLR_MASK,CLR_ROT},
  {"s",CLR_SPADE,CLR_MASK,CLR_ROT},
  {"club",CLR_CLUB,CLR_MASK,CLR_ROT},
  {"c",CLR_CLUB,CLR_MASK,CLR_ROT},
  {"heart",CLR_HEART,CLR_MASK,CLR_ROT},
  {"h",CLR_HEART,CLR_MASK,CLR_ROT},
  {"diamond",CLR_DIAMOND,CLR_MASK,CLR_ROT},
  {"d",CLR_DIAMOND,CLR_MASK,CLR_ROT},
  {"red",CLR_RED,CLR_MASK,CLR_ROT},
  {"black",CLR_BLACK,CLR_MASK,CLR_ROT},
  {"hdcs",CLR_HDCS,CLR_MASK,CLR_ROT},
  {"hsdc",CLR_HSDC,CLR_MASK,CLR_ROT},
  {"cdhs",CLR_CDHS,CLR_MASK,CLR_ROT},
  {"anycolor",0,CLR_MASK,CLR_ROT},
};
          
TCHAR *unknown=_T("Unknown");
int undoing;
static const int indHDCS[]={
  CLR_HDCS<<CLR_ROT, CLR_HSDC<<CLR_ROT, CLR_CDHS<<CLR_ROT};
static const int HDCS[][4]={
  {CLR_HEART<<CLR_ROT,CLR_DIAMOND<<CLR_ROT,CLR_CLUB<<CLR_ROT,CLR_SPADE<<CLR_ROT},
  {CLR_HEART<<CLR_ROT,CLR_SPADE<<CLR_ROT,CLR_DIAMOND<<CLR_ROT,CLR_CLUB<<CLR_ROT},
  {CLR_CLUB<<CLR_ROT,CLR_DIAMOND<<CLR_ROT,CLR_HEART<<CLR_ROT,CLR_SPADE<<CLR_ROT},
};

//------------------------------------------------------------------
struct TxmsolParser : public TxmlFileParser {
  int Nerr;
  TxmsolParser(FILE *f):TxmlFileParser(f),Nerr(0){};
  TxmlElem *newElem(TCHAR *name, TxmlElem *parent);
  void error(char *s);
};

struct TruleElem : public TxmlElem {
  int rule;
  TruleElem(int type):TxmlElem(0x200+type){};
  void setText(TCHAR *content);
};

struct TxmlsolElem : public TxmlElem {
  TxmlsolElem():TxmlElem(1){};
  void addChild(TxmlElem *elem);
};

void TxmsolParser::error(char *s)
{
  if(!Nerr++){
    msg("%s (line %d, column %d) ",s,line,column);
  }
}

Tgame::Tgame():TxmlElem(2)
{ 
  pair=0; 
  decks=1; 
  suit=0xf;
  rank=0x1fff;
  name=unknown; 
  autoplay=1; 
  static Tstat zeroStat;
  stat=&zeroStat;
}

Tgame::~Tgame()
{
  if(name!=unknown) free(name);
  deleteDarray(cells);
}

void Tgame::addAttr(TCHAR *name, TCHAR *value)
{
  int i=_ttoi(value);
  if(!_tcscmp(name,_T("name"))) this->name=_tcsdup(value);
  else if(!_tcscmp(name,_T("decks"))) decks=i;
  else if(!_tcscmp(name,_T("autoplay"))) autoplay=i;
  else if(!_tcscmp(name,_T("pair"))){
    pair=i;
    if(!_tcscmp(value,_T("suit"))) pair=PAIR_SUIT;
    if(!_tcscmp(value,_T("rank"))) pair=PAIR_RANK;
    if(!_tcscmp(value,_T("rankcolor"))) pair=PAIR_RANKCOLOR;
  }
  else if(!_tcscmp(name,_T("suit"))){
    suit=0;
    for(TCHAR *s=value; *s; s++){
      i=0;
      if(*s=='s') i=1<<SPADE;
      if(*s=='c') i=1<<CLUB;
      if(*s=='h') i=1<<HEART;
      if(*s=='d') i=1<<DIAMOND;
      suit|=i;
    }
  }
  else if(!_tcscmp(name,_T("rank"))){
    rank=0;
    for(TCHAR *s=value; *s; s++){
      i=0;
      if(*s=='a') i=1;
      if(*s>='2' && *s<='9') i=1<<(*s-'0'-1);
      if(*s=='1' && *(s+1)=='0'){ i=1<<9; s++; }
      if(*s=='j') i=1<<10;
      if(*s=='q') i=1<<11;
      if(*s=='k') i=1<<12;
      rank|=i;
    }
  }
  else parser->error("Unknown attribute");
}

void Tgame::addChild(TxmlElem *elem)
{
  if((elem->tag&0xfff0)==0x100){
    *cells++=(TcellElem*)elem;
  }
}

bool Tgame::check()
{
  int i,rule,suits,ranks;
  TcellElem *c;

  if(cells.len==0) return false;
  if(pair){
    for(i=0; i<cells.len; i++){
      if(cells[i]->type==CELL_FOUNDATION){
        cells[i]->inRule=(RANK_NO<<RANK_ROT)|(SEQ_NO<<SEQ_ROT);
        break;
      }
    }
  }
  suits=Nsuit();
  ranks=Nrank();
  cards=decks*ranks*suits;
  stocks=0;
  ruleSeq=ruleSuit=0;
  ruleGroup=GROUP_NO;
  for(i=0; i<cells.len; i++){
    c= cells[i];
    amax(c->w,1000-c->x);
    amax(c->h,1000-c->y);
    if(c->lines>1 && c->repeat==1) c->repeat=c->lines;
    if(c->dir==DIR_NO && (c->outRule & GROUP_MASK)==0){
      c->outRule|=(GROUP_NO<<GROUP_ROT);
    }
    if(((c->prevRule & SEQ_MASK)==(SEQ_SYNCH<<SEQ_ROT) || (c->prevRule & SEQ_MASK)==(SEQ_OSMOSIS<<SEQ_ROT)) && 
      (c->prevRule&RANK_MASK)==0){
      c->prevRule|=(RANK_NO<<RANK_ROT);
    }
    if(c->type==CELL_STOCK) stocks+=c->repeat;
    if(c->type==CELL_FOUNDATION){
      if((c->inRule&CONT_MASK) && c->repeat==decks*suits && c->maxCount==255 &&
        (decks>1 || (c->inRule&SUIT_MASK)!=(SUIT_SAME<<SUIT_ROT))){
        c->maxCount=ranks;
      }
    }else{
      rule=(c->inRule&SEQ_MASK)>>SEQ_ROT;
      if(rule && (rule!=SEQ_NO || !ruleSeq)) ruleSeq=rule;
      rule=(c->inRule&SUIT_MASK)>>SUIT_ROT;
      if(rule) ruleSuit=rule;
      rule=(c->inRule&GROUP_MASK)>>GROUP_ROT;
      if(rule!=GROUP_NOEMPTY && rule!=GROUP_NO) ruleGroup=rule;
    }
  }
  return true;
}

void TxmlsolElem::addChild(TxmlElem *elem)
{
  if(elem->tag==2 && ((Tgame*)elem)->check()){
    *games++= (Tgame*)elem;
  }else{
    delete elem;
  }
}

TcellElem::TcellElem(int type):TxmlElem(0x100+type)
{
  //default attributes
  Tcell0::type=type;
  x=y=startCount=hideStyle=onhid=redeal=shuffle=deal=fill=blockedBy[0]=blockedBy[1]=0;
  startRule=inRule=outRule=sameRule=prevRule=0;
  shape=SHAPE_LEFT;
  autoplay=1;
  dir=DIR_NO; 
  spacing=100;
  prevDir=PREV_LEFT;
  maxCount=255;
  w=h=1000;
  repeat=lines=1;
  switch(type){
  case CELL_TABLEAU:
    dir=DIR_DOWN;
    y=320;
    startCount=0x7fff;
    inRule=(SEQ_DOWN<<SEQ_ROT);
    break;
  case CELL_FOUNDATION:
    inRule=(SEQ_UP<<SEQ_ROT)|(SUIT_SAME<<SUIT_ROT)|(1<<RANK_ROT);
    break;
  case CELL_RESERVE:
    inRule=(SEQ_NO<<SEQ_ROT)|(RANK_NO<<RANK_ROT);
    break;
  case CELL_CELL:
    inRule=(SEQ_NO<<SEQ_ROT);
    break;
  case CELL_WASTE:
    inRule=(1<<(CELL_STOCK+DEST_ROT));
    break;
  case CELL_STOCK:
    startCount=0x7fff;
    inRule=(SEQ_NO<<SEQ_ROT)|(RANK_NO<<RANK_ROT);
    hideStyle=HIDE_ALL;
    onhid=ONHID_BLOCKED;
    deal=1;
    break;
  }
  inRule|=(GROUP_NO<<GROUP_ROT);
}

void TcellElem::addAttr(TCHAR *name, TCHAR *value)
{
  int i=_ttoi(value);
  if(!_tcscmp(name,_T("x"))) x=i;
  else if(!_tcscmp(name,_T("y"))) y=i;
  else if(!_tcscmp(name,_T("w"))) w=i;
  else if(!_tcscmp(name,_T("h"))) h=i;
  else if(!_tcscmp(name,_T("repeat"))) repeat=i;
  else if(!_tcscmp(name,_T("lines"))) lines=i;
  else if(!_tcscmp(name,_T("count"))) startCount=i;
  else if(!_tcscmp(name,_T("max"))) maxCount=i;
  else if(!_tcscmp(name,_T("autoplay"))){
    autoplay=i;
    if(!_tcscmp(value,_T("base"))) autoplay=AUTO_BASE;
  }else if(!_tcscmp(name,_T("deal"))){
    deal=i;
    if(!_tcscmp(value,_T("fill"))){
      deal=DEAL_FILL;
      if(!fill) fill=1;
    }
    if(!_tcscmp(value,_T("gaps"))) deal=DEAL_GAPS;
    if(!_tcscmp(value,_T("bottomtotop"))) deal=DEAL_BOTTOMTOTOP;
    if(!_tcscmp(value,_T("max"))) deal=DEAL_MAX;
    if(!_tcscmp(value,_T("start"))) deal=DEAL_START;
    if(!_tcscmp(value,_T("start1"))) deal=DEAL_START1;
    if(!_tcscmp(value,_T("start_col"))) deal=DEAL_START_COL;
  }
  else if(!_tcscmp(name,_T("fill"))) fill=i;
  else if(!_tcscmp(name,_T("redeal"))){
    redeal=i;
    if(!_tcscmp(value,_T("inf"))) redeal=0x7fffffff;
  }
  else if(!_tcscmp(name,_T("shuffle"))){
    if(!_tcscmp(value,_T("rand"))) shuffle=SHUFFLE_RAND;
  }
  else if(!_tcscmp(name,_T("dir"))){
    if(!_tcscmp(value,_T("left"))) dir=DIR_LEFT;
    if(!_tcscmp(value,_T("right"))) dir=DIR_RIGHT;
    if(!_tcscmp(value,_T("up"))) dir=DIR_UP;
    if(!_tcscmp(value,_T("down"))) dir=DIR_DOWN;
    if(!_tcscmp(value,_T("no"))) dir=DIR_NO;
  }
  else if(!_tcscmp(name,_T("spacing"))){
    spacing=i;
  }else if(!_tcscmp(name,_T("prev"))){
    prevDir=i;
    if(!_tcscmp(value,_T("left"))) prevDir=PREV_LEFT;
    if(!_tcscmp(value,_T("right"))) prevDir=PREV_RIGHT;
    if(!_tcscmp(value,_T("up"))) prevDir=PREV_UP;
    if(!_tcscmp(value,_T("down"))) prevDir=PREV_DOWN;
  }
  else if(!_tcscmp(name,_T("hide"))){
    hideStyle=i;
    if(!_tcscmp(value,_T("all"))) hideStyle=HIDE_ALL;
    if(!_tcscmp(value,_T("odd"))) hideStyle=HIDE_ODD;
    if(!_tcscmp(value,_T("even"))) hideStyle=HIDE_EVEN;
  }
  else if(!_tcscmp(name,_T("onhid"))){
    if(!_tcscmp(value,_T("any"))) onhid=ONHID_ANY;
    if(!_tcscmp(value,_T("k"))) onhid=ONHID_K;
  }
  else if(!_tcscmp(name,_T("shape"))){
    if(!_tcscmp(value,_T("incr"))) shape=SHAPE_INCR;
    if(!_tcscmp(value,_T("decr"))) shape=SHAPE_DECR;
    if(!_tcscmp(value,_T("incr2"))) shape=SHAPE_INCR2;
    if(!_tcscmp(value,_T("decr2"))) shape=SHAPE_DECR2;
    if(!_tcscmp(value,_T("pyramid"))) shape=SHAPE_PYRAMID;
    if(!_tcscmp(value,_T("braid"))) shape=SHAPE_BRAID;
    if(!_tcscmp(value,_T("bind"))) shape=SHAPE_BIND;
    if(!_tcscmp(value,_T("left"))) shape=SHAPE_LEFT;
    if(!_tcscmp(value,_T("right"))) shape=SHAPE_RIGHT;
    if(!_tcscmp(value,_T("center"))) shape=SHAPE_CENTER;
    if(!_tcscmp(value,_T("side"))) shape=SHAPE_SIDE;
    if(!_tcscmp(value,_T("first"))) shape=SHAPE_FIRST;
    if(!_tcscmp(value,_T("last"))) shape=SHAPE_LAST;
  }
  else if(!_tcscmp(name,_T("under")) || !_tcscmp(name,_T("under2"))){
    int *b= blockedBy;
    if(_tcscmp(name,_T("under2"))) b++;
    *b=i;
    if(!_tcscmp(value,_T("next"))) *b=UNDER_NEXT;
    if(!_tcscmp(value,_T("next+1"))) *b=UNDER_NEXT1;
    if(!_tcscmp(value,_T("waste"))) *b=UNDER_WASTE;
  }
  else parser->error("Unknown attribute");
}

void TcellElem::addChild(TxmlElem *elem)
{ 
  if(elem->tag==0x200) startRule=((TruleElem*)elem)->rule;
  if(elem->tag==0x202) outRule=((TruleElem*)elem)->rule;
  if(elem->tag==0x203) sameRule=((TruleElem*)elem)->rule;
  if(elem->tag==0x204) prevRule=((TruleElem*)elem)->rule;
  delete elem; 
}

int parseRule(TCHAR *s, int *pmask, TxmlParser *parser)
{
  TCHAR *b;
  int i,mask,rule;
  Tword *w;

  mask=0;
  rule=0;
  for(;;){
    while(*s==' ' || *s=='\t' || *s=='\n') s++;
    b=s;
    while(*s && *s!=' ' && *s!='\t' && *s!='\n') s++;
    if(b==s) break;
    if(*s) *s++=0;
    for(i=0; ;i++){
      if(i==sizeA(wordA)){
        parser->error("Unknown word");
        break;
      }
      w=&wordA[i];
      convertT2A(b,a);
      if(!strcmp(a,w->name)){
        rule|= w->value << w->rot;
        if(mask & w->mask) parser->error("Rules overlap");
        mask|= w->mask;
        break;
      }
    }
  }
  if(pmask) *pmask=mask;
  return rule;
}

void TcellElem::setText(TCHAR *s)
{
  int mask,rule;
  rule=parseRule(s,&mask,parser);
  inRule=(inRule&~mask)|rule;
}

void TruleElem::setText(TCHAR *s)
{
  rule= parseRule(s,0,parser);
}

TxmlElem *TxmsolParser::newElem(TCHAR *name, TxmlElem *parent)
{
  if(parent){
    if(!_tcscmp(name,_T("cell"))) return new TcellElem(CELL_CELL);
    else if(!_tcscmp(name,_T("foundation"))) return new TcellElem(CELL_FOUNDATION);
    else if(!_tcscmp(name,_T("tableau"))) return new TcellElem(CELL_TABLEAU);
    else if(!_tcscmp(name,_T("reserve"))) return new TcellElem(CELL_RESERVE);
    else if(!_tcscmp(name,_T("stock"))) return new TcellElem(CELL_STOCK);
    else if(!_tcscmp(name,_T("waste"))) return new TcellElem(CELL_WASTE);
    else if(!_tcscmp(name,_T("init"))) return new TruleElem(0);
    else if(!_tcscmp(name,_T("out"))) return new TruleElem(2);
    else if(!_tcscmp(name,_T("same"))) return new TruleElem(3);
    else if(!_tcscmp(name,_T("prev"))) return new TruleElem(4);
    else if(!_tcscmp(name,_T("game"))) return new Tgame;
    else error("Unknown element");
  }else{
    if(!_tcscmp(name,_T("xmsol"))) return new TxmlsolElem;
  }
  return 0;
}

void readRules(TCHAR *fn)
{
  FILE *f;

  f=_tfopen(fn,_T("rt"));
  if(f){
    TxmsolParser parser(f);
    delete parser.parse();
  }
  fclose(f);
}

Tgame *findGame(TCHAR *name)
{
  for(int i=0; i<games.len; i++){
    Tgame *g= games[i];
    if(!_tcsicmp(g->name,name)) return g;
  }
  return 0;
}

void deleteGames()
{
  board.game=0;
  for(int i=0; i<games.len; i++){
    delete games[i];
  }
  games.reset();
}

void loadRules()
{
  HANDLE h;
  TfileName fnRules;
  WIN32_FIND_DATA fd;

  mustLoadRules=false;
  deleteGames();
  getExeDir(fnRules,_T("rules\\*.xml"));
  h = FindFirstFile(fnRules,&fd);
  if(h!=INVALID_HANDLE_VALUE){
    do{
      if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
        _tcscpy(cutPath(fnRules),fd.cFileName);
        readRules(fnRules);
      }
    }while(FindNextFile(h,&fd));
    FindClose(h);
  }
  board.game=findGame(gameName);
  generRules(board.game);
}
//------------------------------------------------------------------

void Tboard::newSeed()
{
  seedGlobal=seedGlobal*1386674153+265620371;
  seed= seedGlobal;
}

//linear congruential generator
unsigned Tboard::rand(unsigned n)
{
  seed=seed*367413989+174680251;
  return (unsigned)(UInt32x32To64(n,seed)>>32);
}

unsigned Tboard::randUndo(unsigned n)
{
  unsigned result= (unsigned)(UInt32x32To64(n,seed)>>32);
  seed=(seed-174680251)*4253901549u;
  return result;
}


inline bool isKing4(Tcell *c)
{
  return (c->inRule&RANK_MASK)==(RANK_KING4<<RANK_ROT);
}

int Tboard::king4()
{
  int i,n;
  Tcell *c;

  n=0;
  for(i=0; i<cells.len; i++){
    c=&cells[i];
    if(isKing4(c) && c->Ncards && CARD_RANK(c->cards[0])==13) n++;
  }
  return (n==game->decks*4) ? 0 : 13;
}

int Tboard::freecell(int dest)
{
  int i,res,tab,g,seq,s,rule;
  Tcell *c;

  res=1;
  tab=0;
  seq= cells[dest].inRule & (SEQ1_MASK|SUIT_MASK);
  for(i=0; i<cells.len; i++){
    c=&cells[i];
    if(c->Ncards==0 && i!=dest && !c->prevRule){
      rule= c->inRule;
      if((rule&RANK_MASK)==(RANK_KING4<<RANK_ROT) && !king4()){
        rule&=~RANK_MASK;
      }
      if(!((rule|c->outRule|c->sameRule)&~(SEQ1_MASK|SUIT_MASK|GROUP_MASK))){
        g= rule & GROUP_MASK;
        if(g!=(GROUP_NOEMPTY<<GROUP_ROT) && g!=(GROUP_ALL<<GROUP_ROT)){
          g= c->outRule & GROUP_MASK;
          if(g!=(GROUP_NOEMPTY<<GROUP_ROT) && g!=(GROUP_ALL<<GROUP_ROT)){
            g= c->sameRule & GROUP_MASK;
            if(g!=(GROUP_NOEMPTY<<GROUP_ROT) && g!=(GROUP_ALL<<GROUP_ROT)){
              res++;
              s= rule & (SEQ1_MASK|SUIT_MASK);
              if(s==seq || s==0){
                s= c->sameRule & (SEQ1_MASK|SUIT_MASK);
                if(s==seq || s==0){
                  tab++;
                  res--;
                }
              }
            }
          }
        }
      }
    }
  }
  return res<<tab;
}

inline bool isSameBase(Tcell *c)
{
  return (c->inRule&RANK_MASK)==(RANK_SAMEBASE<<RANK_ROT);
}

int Tboard::baseRank()
{
  int i;
  Tcell *c;

  for(i=0; i<cells.len; i++){
    c=&cells[i];
    if(isSameBase(c)){
      if(c->Ncards>0) return CARD_RANK(c->cards[0]);
    }
  }
  return 0;
}

int seqTest(int rule, Tcard destCard, Tcard srcCard, Tcell *prevCell)
{
  int r,r1,j,isCont;

  r=CARD_RANK(destCard);
  r1=CARD_RANK(srcCard);
  isCont=rule&CONT_MASK;
  switch((rule&SEQ_MASK)>>SEQ_ROT){
  case SEQ_NO:
    return 0;
  case SEQ_UP:
    if(r1!=r+1) if(!isCont || r1!=r-12) return 0;
    break;
  case SEQ_DOWN:
    if(r1!=r-1) if(!isCont || r1!=r+12) return 0;
    break;
  case SEQ_UPDOWN:
    if(r1!=r+1 && r1!=r-1) if(!isCont || r1!=r-12 && r1!=r+12) return 0;
    break;
  case SEQ_UPDOWNRANK:
    if(r1!=r && r1!=r+1 && r1!=r-1) if(!isCont || r1!=r-12 && r1!=r+12) return 0;
    break;
  case SEQ_UPRANK:
    if(r1!=r+1 && r1!=r) if(!isCont || r1!=r-12) return 0;
    break;
  case SEQ_DOWNRANK:
    if(r1!=r-1 && r1!=r) if(!isCont || r1!=r+12) return 0;
    break;
  case SEQ_UP2:
    if(r1!=r+2) if(!isCont || r1!=r-11) return 0;
    break;
  case SEQ_DOWN2:
    if(r1!=r-2) if(!isCont || r1!=r+11) return 0;
    break;
  case SEQ_UPDOWN2:
    if(r1!=r+2 && r1!=r-2) if(!isCont || r1!=r-11 && r1!=r+11) return 0;
    break;
  case SEQ_HIGHER:
    if(r1<=r) return 0;
    break;
  case SEQ_LOWER:
    if(r1>=r) return 0;
    break;
  case SEQ_RANK:
    if(r1!=r) return 0;
    break;
  case SEQ_OSMOSIS:
    if(prevCell){
      for(j=prevCell->Ncards-1; ;j--){
        if(j<0) return 0;
        if(CARD_RANK(prevCell->cards[j])==r1) break;
      }
    }
    break;
  case SEQ_SYNCH:
    if(prevCell){
      if(prevCell->Ncards!=(prevCell->maxCount<255 ? prevCell->maxCount : 13)) return 0;
    }
    break;
  }
  return 1;
}

int suitTest(int rule, Tcard destCard, Tcard srcCard)
{
  int s,s1;

  s=CARD_SUIT(destCard);
  s1=CARD_SUIT(srcCard);
  switch((rule&SUIT_MASK)>>SUIT_ROT){
  case SUIT_SAME:
    if(s1!=s) return 0;
    break;
  case SUIT_DIFF:
    if(s1==s) return 0;
    break;
  case SUIT_SAMECOLOR:
    if((s1&CARD_COLOR)!=(s&CARD_COLOR)) return 0;
    break;
  case SUIT_DIFFCOLOR:
    if((s1&CARD_COLOR)==(s&CARD_COLOR)) return 0;
    break;
  }
  return 1;
}

int Tboard::rankTest(int rule, Tcard card)
{
  int r,r1;

  r=(rule&RANK_MASK)>>RANK_ROT;
  if(r){
    r1=CARD_RANK(card);
    if(r==RANK_AK){
      if(r1!=1 && r1!=13) return 0;
    }else{
      if(r==RANK_RANDOM) r=randomRank;
      if(r==RANK_RANDOMPLUS1) r=randomRank%13+1;
      if(r==RANK_SAMEBASE) r=baseRank();
      if(r==RANK_KING4) r=king4();
      assert(RANK_NO==14);
      if(r!=r1 && r>0 && r<=14) return 0;
    }
  }
  return 1;
}

int colorTest(int rule, Tcard card)
{
  int s,s1;

  s=(rule&CLR_MASK)>>CLR_ROT;
  if(s){
    s1=CARD_SUIT(card);
    switch(s){
    case CLR_HEART: 
      if(s1!=HEART) return 0;
      break;
    case CLR_DIAMOND: 
      if(s1!=DIAMOND) return 0;
      break;
    case CLR_CLUB: 
      if(s1!=CLUB) return 0;
      break;
    case CLR_SPADE: 
      if(s1!=SPADE) return 0;
      break;
    case CLR_RED:
      if(s1!=HEART && s1!=DIAMOND) return 0;
      break;
    case CLR_BLACK:
      if(s1!=SPADE && s1!=CLUB) return 0;
      break;
    }
  }
  return 1;
}

int Tboard::testMove(int dest, int src, int ind, int drag)
{
  Tcell *srcCell,*destCell,*prevCell=0,*c;
  Tcard destCard,srcCard,*srcCards;
  int i,k,count,destInd,r,r1,s,s1,maxGroup,t,pair;
  int rule;

  if(dest<0 || ind<0) return 0;
  destCell=&cells[dest];
  srcCell=&cells[src];
  srcCards=srcCell->cards+ind;
  count=srcCell->Ncards-ind;
  if(src==dest) return ind>MOVE_FIRST;
  if(srcCell->blockedBy[0]){
    i=src+srcCell->blockedBy[0];
    if(unsigned(i)<unsigned(cells.len) && cells[i].Ncards!=0) return 0;
  }
  if(srcCell->blockedBy[1]){
    i=src+srcCell->blockedBy[1];
    if(unsigned(i)<unsigned(cells.len) && cells[i].Ncards!=0) return 0;
  }
  if(destCell->blockedBy[0]){
    i=dest+destCell->blockedBy[0];
    if(unsigned(i)<unsigned(cells.len) && cells[i].Ncards!=0) return 0;
  }
  if(destCell->blockedBy[1]){
    i=dest+destCell->blockedBy[1];
    if(unsigned(i)<unsigned(cells.len) && cells[i].Ncards!=0) return 0;
  }
  if(game->pair && count==1 && destCell->type!=CELL_FOUNDATION && srcCell->type!=CELL_FOUNDATION){
    k=destCell->Ncards;
    if(k>0 && !srcCell->hidden[ind] && !destCell->hidden[k-1]){
      r=CARD_RANK(srcCards[0]);
      r1=CARD_RANK(destCell->cards[k-1]);
      s=CARD_SUIT(srcCards[0]);
      s1=CARD_SUIT(destCell->cards[k-1]);
      switch(pair=game->pair){
      case PAIR_SUIT:
        if(s==s1) return 1;
        break;
      case PAIR_RANK:
        if(r==r1) return 1;
        break;
      case PAIR_RANKCOLOR:
        if(r==r1 && (s&CARD_COLOR)==(s1&CARD_COLOR)) return 1;
        break;
      default:
        if(r+r1==pair || r>pair && r1>pair) return 1;
        break;
      }
    }
  }
  maxGroup=0;
  for(k=0; k<3; k++){
    if(k==0){
      destInd=destCell->Ncards;
      if(destInd+count > destCell->maxCount) return 0;
      rule= destCell->inRule;
    }else if(k==1){
      destInd=0;
      rule= srcCell->outRule;
    }else{
      rule= destCell->prevRule;
      if(destCell->prevInd<0) continue;
      prevCell=&cells[destCell->prevInd];
      destInd=prevCell->Ncards;
    }
    if(destCell->type==srcCell->type && destCell->sameRule){
      if(k) break;
      rule= destCell->sameRule;
    }
    if(!rule) continue;
    for(i=drag; i<count; i++){
      srcCard=srcCards[i];
      if(i==0){
        if(destInd==0){
          if(!rankTest(rule,srcCard)) return 0;
          if(!colorTest(rule,srcCard)) return 0;
          continue;
        }
        c= (k==2) ? prevCell : destCell;
        if(c->hidden[destInd-1]){
          switch(c->onhid){
          case ONHID_ANY:
            break;
          case ONHID_K:
            if(CARD_RANK(srcCard)!=13) return 0;
            break;
          default:
            return 0;
          }
          continue;
        }
        destCard=c->cards[destInd-1];
      }else{
        if(k==2 || (rule&GROUP_MASK)==(GROUP_ANYSEQ<<GROUP_ROT) ||
          (rule&GROUP_MASK)==(GROUP_PILE<<GROUP_ROT)) break;
        if((rule&GROUP_MASK)==(GROUP_ANYFREECELLSEQ<<GROUP_ROT)){
          if(!maxGroup) maxGroup=freecell(dest);
          if(count<=maxGroup) break;
        }
        destCard=srcCards[i-1];
      }
      if(!seqTest(rule,destCard,srcCard,prevCell) || !suitTest(rule,destCard,srcCard)){
        if(k==1 && (destCell->inRule&GROUP_MASK)==0){
          if(!maxGroup) maxGroup=freecell(dest);
          if(count<=maxGroup) break;
        }
        return 0;
      }
    }
    switch((rule&GROUP_MASK)>>GROUP_ROT){
    case GROUP_PILE:
      if(count==srcCell->Ncards) break;
      goto lg;
    case GROUP_TOEMPTY:
      if(count>1 && destCell->Ncards==0) break;
      goto lg;
    case GROUP_NO:
lg:   if(count>1){
        if(!maxGroup) maxGroup=freecell(dest);
        if(count>maxGroup) return 0;
      }
      break;
    case GROUP_ALL:
      if(count!=13) return 0;
      break;
    case GROUP_NOEMPTY:
      if(count==srcCell->Ncards) return 0;
      break;
    }
    t=(rule&DEST_MASK);
    if(t){
      if(( (t>>DEST_ROT) & (1<<(k==0 ? srcCell : destCell)->type) )==0) return 0;
    }
  }
  for(i=0; i<count; i++){
    if(srcCell->hidden[ind+i]) return 0;
  }
  return count;
}

void Tboard::move2(int dest, int src, int ind)
{
  Tcell *srcCell,*destCell,*c;
  int i,j,k,count,destInd;
  Tcard w;

  srcCell=&cells[src];
  count=srcCell->Ncards-ind;
  destCell=&cells[dest];
  destInd=destCell->Ncards;
  if(src==dest){
    switch(ind){
    case MOVE_SHUFFLE:
      if(destInd<=0) return;
      for(i=0; i<destInd; i++){
        if(undoing){
          j=randUndo(destInd);
          k=destInd-1-i;
        }else{
          j=rand(destInd);
          k=i;
        }
        w=srcCell->cards[j];
        srcCell->cards[j]=srcCell->cards[k];
        srcCell->cards[k]=w;
      }
      invalidateColumn(src,0,destInd);
      destInd=ind;
      break;
    case MOVE_HIDE:
      srcCell->hidden[destInd-1]=true;
      invalidateColumn(src,destInd-1,1);
      destInd=MOVE_UNHIDE;
      if(srcCell->type!=CELL_STOCK) scoreUnhide--;
      break;
    case MOVE_UNHIDE:
      srcCell->hidden[destInd-1]=false;
      invalidateColumn(src,destInd-1,1);
      destInd=MOVE_HIDE;
      if(srcCell->type!=CELL_STOCK) scoreUnhide++;
      break;
    }
  }else{
    if(count<=0) return;
    invalidateColumn(dest,destInd,count);
    invalidateColumn(src,ind,count);
    if(isSameBase(destCell) && destInd==0 || isSameBase(srcCell) && ind==0){
      for(i=0; i<cells.len; i++){
        c=&cells[i];
        if(isSameBase(c) && c->Ncards==0){
          invalidateColumn(i,0,1);
        }
      }
    }
    if((isKing4(destCell) && destInd==0 || isKing4(srcCell) && ind==0) && 
      CARD_RANK(srcCell->cards[ind])==13){
      for(i=0; i<cells.len; i++){
        c=&cells[i];
        if(isKing4(c) && c->Ncards==0){
          invalidateColumn(i,0,1);
        }
      }
    }
    if(destCell->redeal && destInd==0 && !undoing){
      destCell->redealCount++;
    }
    if(undoing && srcCell->redeal && ind==0){
      srcCell->redealCount--;
    }
    for(i=count; i>0; i--){
      destCell->cards[destCell->Ncards]= srcCell->cards[srcCell->Ncards-i];
      destCell->hidden[destCell->Ncards]= false;
      destCell->Ncards++;
    }
    srcCell->Ncards-=count;
  }
  //add undo record
  TundoRec *r= &undoRec[undoPos];
  if(!undoing){
    if(undoPos==undoRec.len) r=undoRec++;
    if(undoPos<redoPos){
      if(r->src!=src || r->dest!=dest || r->ind!=ind){
        redoPos=0;
        scoreUndo++;
      }
    }else{
      redoPos=0;
    }  
    r->src=src;
    r->dest=dest;
    r->ind=ind;
    r->destInd=destInd;
    r->group=moves;
    r->unhide=false;
    undoPos++;
  }
  if(src!=dest){
    //unhide exposed card
    if(ind>0 && srcCell->onhid==ONHID_AUTO){
      if(srcCell->hidden[ind-1]){
        srcCell->hidden[ind-1]=false;
        if(!undoing){
          r->unhide=true;
          if(srcCell->type!=CELL_STOCK) scoreUnhide++;
        }
        invalidateColumn(src,ind-1,1);
      }
    }
    //hide stock card (when undo)
    if(destCell->type==CELL_STOCK && destCell->hideStyle==HIDE_ALL){
      for(i=count; i>0; i--){
        destCell->hidden[destCell->Ncards-i]=true;
      }
    }
  }
}

void Tboard::move1(int dest, int src, int ind)
{
  Tcell *srcCell,*destCell;
  int i,count,destInd,r1,r2,s1,s2,pair;

  srcCell=&cells[src];
  count=srcCell->Ncards-ind;
  destCell=&cells[dest];
  destInd=destCell->Ncards;
  move2(dest,src,ind);
  //move pair to a foundation
  if(game->pair && count==1 && destInd>0 && !undoing && 
    destCell->type!=CELL_FOUNDATION && srcCell->type!=CELL_STOCK){
    r2=CARD_RANK(destCell->cards[destInd]);
    r1=CARD_RANK(destCell->cards[destInd-1]);
    s2=CARD_SUIT(destCell->cards[destInd]);
    s1=CARD_SUIT(destCell->cards[destInd-1]);
    switch(pair=game->pair){
    case PAIR_SUIT:
      if(s1!=s2) return;
      break;
    case PAIR_RANK:
      if(r1!=r2) return;
      break;
    case PAIR_RANKCOLOR:
      if(r1!=r2 || (s1&CARD_COLOR)!=(s2&CARD_COLOR)) return;
      break;
    default:
      if(r1+r2!=pair && (r1<=pair && r2<=pair)) return;
      break;
    }
    for(i=0; i<cells.len; i++){
      if(cells[i].type==CELL_FOUNDATION){
        move2(i,dest,destInd-1);
        break;
      }
    }
  }
}

int Tboard::move(int dest, int src, int ind)
{
  int count= testMove(dest,src,ind);
  if(count){
    move1(dest,src,ind);
    moves++;
  }
  return count;
}

int Tboard::animMove(int dest, int src, int ind, int anim)
{
  if(testMove(dest,src,ind)){
    animate(dest,src,ind,anim);
    return move(dest,src,ind);
  }else{
    return 0;
  }
}

void Tboard::animMove1(int dest, int src, int ind, int anim)
{
  animate(dest,src,ind,anim);
  move1(dest,src,ind);
}

int Tboard::chooseDest(int src)
{
  Tcell *c;
  int i,n,destType,srcType;

  c= &cells[src];
  destType=(c->outRule&DEST_MASK)>>DEST_ROT;
  if(!destType){
    srcType=c->type;
    if(waste>=0 && waste!=src){
      destType= (1<<CELL_WASTE);
    }else{ 
      n=game->cells.len;
      if(srcType!=CELL_WASTE){
        for(i=0; i<n; i++){
          if(game->cells[i]->type==CELL_WASTE) return 1<<CELL_WASTE;
        }
      }
      if(srcType!=CELL_TABLEAU){
        for(i=0; i<n; i++){
          if(game->cells[i]->type==CELL_TABLEAU) return 1<<CELL_TABLEAU;
        }
      }
      if(srcType!=CELL_CELL){
        for(i=0; i<n; i++){
          if(game->cells[i]->type==CELL_CELL) return 1<<CELL_CELL;
        }
      }
      if(srcType!=CELL_RESERVE){
        for(i=0; i<n; i++){
          if(game->cells[i]->type==CELL_RESERVE) return 1<<CELL_RESERVE;
        }
      }
    }
  }
  return destType;
}

bool testHide(Tcell *c, int ind, int N)
{
  bool b;

  switch(c->hideStyle){
  case HIDE_ALL:
    b=true;
    break;
  case HIDE_ODD:
    b=(ind&1)==0;
    break;
  case HIDE_EVEN:
    b=(ind&1)!=0;
    break;
  default:
    b= ind < ((c->hideStyle<0)? N+c->hideStyle : c->hideStyle);
    break;
  }
  return b;
}

int Tboard::stockClick(int src)
{
  int i,j,destType,r,result=0;
  Tcell *t,*c,*p;
  Tcard a,a1;
  bool change,isRedeal;

  destType=chooseDest(src);
  c= &cells[src];
  isRedeal= c->Ncards==0;
  if(isRedeal){
    //redeal
    if(c->redealCount < c->redeal || c->redeal==0x7fffffff){
      if(c->deal==DEAL_GAPS){
        for(i=0; i<cells.len; i++){
          t=&cells[i];
          if(t->Ncards>0 && t->type!=CELL_STOCK && (t->inRule&SEQ_MASK)!=(SEQ_RANK<<SEQ_ROT)){
            a1= t->cards[0];
            r= (t->inRule&RANK_MASK)>>RANK_ROT;
            if(!r || r>13 || r!=CARD_RANK(a1)){
              if(t->prevRule && t->prevInd>=0){
                p=&cells[t->prevInd];
                if(p->Ncards>0){
                  a= p->cards[p->Ncards-1];
                  if(seqTest(t->prevRule,a,a1,0) && suitTest(t->prevRule,a,a1)){
                    continue;
                  }
                }
              }
              while(t->Ncards>0){
                animMove1(src,i,t->Ncards-1,ANIM_STOCK);
              }
            }
          }
        }
      }else if(c->deal==DEAL_BOTTOMTOTOP){
        result=1;
        for(i=0; i<cells.len; i++){
          t=&cells[i];
          t->tmp=0;
          if(t->type==CELL_TABLEAU){
            t->tmp=t->Ncards;
            if(t->tmp){
              animMove1(src,i,1,ANIM_STOCK);
              animMove1(src,i,0,ANIM_STOCK);
            }
          }
        }
      }else if(waste>=0 && (destType&(1<<CELL_WASTE))){
        result=cells[waste].Ncards;
        for(i=result-1; i>=0; i--){
          move1(src,waste,i);
          c->hidden[result-1-i]=true;
        }
      }else{
        for(i=0; i<cells.len; i++){
          t=&cells[i];
          if(destType&(1<<t->type)){
            while(t->Ncards>0){
              animMove1(src,i,t->Ncards-1,ANIM_STOCK);
            }
          }
        }
      }
      if(c->shuffle==SHUFFLE_RAND) move1(src,src,MOVE_SHUFFLE); 
    }
  }
  if(c->deal==DEAL_FILL){
    //  => autoPlay()
  }else if(c->deal==DEAL_GAPS){
    for(i=0; i<cells.len; i++){
      t=&cells[i];
      t->tmp= (t->Ncards==0 && t->prevRule && 
        t->prevInd && cells[t->prevInd].Ncards<=0);
    }
    for(i=0; i<cells.len && c->Ncards>0; i++){
      t=&cells[i];
      if((destType&(1<<t->type))!=0 && t->tmp){
        animMove1(i,src,c->Ncards-1,ANIM_STOCK);
        result++;
      }
    }
  }else if(c->deal==DEAL_BOTTOMTOTOP){
    if(result){
      for(i=cells.len-1;  i>=0 && c->Ncards>0;  i--){
        t=&cells[i];
        if(t->tmp){
          animMove1(i,src,c->Ncards-t->tmp,ANIM_STOCK);
          result+=t->tmp;
        }
      }
    }
  }else if(waste>=0 && (destType&(1<<CELL_WASTE))){
    for(i=0; i<c->deal && c->Ncards>0; i++){
      animMove1(waste,src,c->Ncards-1,ANIM_STOCK);
      result++;
    }
  }else{
    i=-1;
    change=false;
    while(result<c->deal && c->Ncards>0){
      do{
        i++;
        if(i>=cells.len){
          i=0;
          if(!change) goto l1;
          change=false;
        }
        t=&cells[i];
      }while((destType&(1<<t->type))==0);
      if(c->deal!=DEAL_MAX || t->Ncards<t->maxCount){
        animMove1(i,src,c->Ncards-1,ANIM_STOCK);
        if(isRedeal && t->hideStyle) move1(i,i,MOVE_HIDE);
        change=true;
        result++;
      }
    }
    l1:
    if(isRedeal){
      for(i=0; i<cells.len; i++){
        t=&cells[i];
        if(t->hideStyle && (destType&(1<<t->type))){
          for(j=0; j<t->Ncards; j++){
            if(!testHide(t,j,t->Ncards)) move1(i,i,MOVE_UNHIDE);
          }
        }
      }
    }
  }
  if(result) moves++;
  return result;
}

//------------------------------------------------------------------

void Tboard::dblclkMove(int src)
{
  int i;
  Tcell *c,*srcCell;

  if(src<0) return;
  srcCell= &cells[src];
  if(srcCell->Ncards==0) return;
  if(srcCell->type==CELL_STOCK){
    stockClick(src);
  }else{
    for(i=0; i<cells.len; i++){
      c=&cells[i];
      if(c->type==CELL_CELL || c->type==CELL_FOUNDATION){
        if(animMove(i,src,cells[src].Ncards-1,ANIM_RIGHTCLICK)) break;
      }
    }
  }
}

int evalMove(Tcell *c, Tcell *srcCell)
{
  if(c->Ncards>0 && !c->hidden[c->Ncards-1]){
    if((c->inRule&SEQ_MASK)==0) return 3;
    if(c->type==CELL_FOUNDATION || srcCell->type==CELL_FOUNDATION){
      if(srcCell->type==c->type) return 2;
      return 0;
    }
    if(srcCell->type!=c->type) return 2;
    return 1;
  }
  if(c->type==CELL_FOUNDATION && (c->inRule&RANK_MASK)==0) return 7;
  if(c->type==CELL_CELL) return 6;
  if(c->type==CELL_FOUNDATION) return 4;
  return 5;
}

int Tboard::rightclkMove(int src, int ind)
{
  int i,k;
  Tcell *c;

  if(src<0) return 0;
  c=&cells[src];
  if(c->Ncards==0) return 0;
  if(c->type==CELL_STOCK) return stockClick(src);
  for(k=0; k<8; k++){
    for(i=src+1; ;i++){
      if(i==cells.len) i=0;
      if(i==src) break;
      if(evalMove(&cells[i],c)==k){
        if(animMove(i,src,ind,ANIM_RIGHTCLICK)) return 1;
      }
    }
  }
  scoreWrongMove++;
  return 0;
}

int calcScore(int unhide,int done,int redeal,int undo,int wrong,int time,bool won,int cards)
{
  int result;
  result= unhide*SCORE_UNHIDE + done*SCORE_FOUNDATION 
    - redeal*SCORE_REDEAL - undo*SCORE_UNDO - wrong*SCORE_WRONG_MOVE;
  if(won) result+= max(0,cards*SCORE_BONUS/52-time*SCORE_TIME);
  if(cards) result= result*52/cards;
  return result;
}

int Tboard::cellFinished(Tcell *c)
{
  int n,rule,j,seq,seqRule,suitRule;
  Tcard c1,c2;
  Tcell *p;

  rule=c->inRule;
  seq= (rule&SEQ_MASK)>>SEQ_ROT;
  n=game->Nrank();
  if(seq==SEQ_NO) n=1;
  if(seq==SEQ_RANK || seq==SEQ_UPDOWNRANK) n=4;
  amax(n,c->maxCount);
  if(n!=c->Ncards) return 0;

  seqRule=rule;
  if(seq==SEQ_UPDOWN) seqRule=SEQ_UP<<SEQ_ROT;
  if(seq==SEQ_UPDOWNRANK) seqRule=SEQ_RANK<<SEQ_ROT;
  suitRule=rule;
  if((rule&SUIT_MASK)==0 && 
    (c->outRule&SUIT_MASK)==(SUIT_SAME<<SUIT_ROT)) suitRule=SUIT_SAME<<SUIT_ROT;
  for(j=1; j<n; j++){
    c1=c->cards[j-1];
    c2=c->cards[j];
    if(!seqTest(seqRule,c1,c2,0)) return 0;
    if(!suitTest(suitRule,c1,c2)) return 0;
  }
  rule= c->prevRule;
  if(rule && c->prevInd){
    p= &cells[c->prevInd];
    if(p->Ncards!=1) return 0;
    c1=p->cards[0];
    c2=c->cards[0];
    if(!seqTest(rule,c1,c2,0) || !suitTest(rule,c1,c2)) return 0;
  }
  return 1;
}

void Tboard::calcDone()
{
  int i,t,f,n;
  Tcell *c;

  if(!game) return;
  t=f=scoreRedeal=n=0;
  for(i=0; i<cells.len; i++){
    c= &cells[i];
    if(c->type==CELL_FOUNDATION){
      if((c->inRule&SEQ_MASK)!=(SEQ_RANK<<SEQ_ROT)){
        f+=c->Ncards;
        n++;
      }
    }
    t+=c->Ncards;
    scoreRedeal+=c->redealCount;
  }
  if(!n){
    //there is no foundation
    for(i=0; i<cells.len; i++){
      c= &cells[i];
      if(cellFinished(c)) f+=c->Ncards;
    }
  }
  donePercent= (t==0) ? -1 : f*100/t;
  if(doneCards!=f){
    doneCards=f;
    statusDone();
  }
  i= calcScore(scoreUnhide,doneCards,scoreRedeal,scoreUndo,scoreWrongMove,playtime,finished,totalCards());
  if(score!=i && (!finished || score<i)){
    score=i;
    statusScore();
  }
  statusMoves();
  statusNcard();
}

bool Tboard::finishFoundation()
{
  for(int i=0; i<cells.len; i++){
    Tcell *c= &cells[i];
    if(c->Ncards!=0 && c->type!=CELL_FOUNDATION){
      return false;
    }
  }
  return true;
}

bool Tboard::finishFullFoundation()
{
  for(int i=0; i<cells.len; i++){
    Tcell *c= &cells[i];
    if(c->type==CELL_FOUNDATION && c->Ncards!=c->maxCount){
      return false;
    }
  }
  return true;
}

bool Tboard::finishFan()
{
  for(int i=0; i<cells.len; i++){
    Tcell *c= &cells[i];
    if(c->Ncards!=0 && !cellFinished(c)){
      return false;
    }
  }
  return true;
}

int Tboard::totalCards()
{
  int n=0;
  for(int i=0; i<cells.len; i++){
    n+=cells[i].Ncards;
  }
  return n;
}

int Tboard::autoBase(Tcell *t)
{
  int i,k,r,r1,rule;
  Tcell *c;

  if(t->Ncards==1){
    rule=(t->inRule&SEQ_MASK)>>SEQ_ROT;
    r=CARD_RANK(t->cards[0]);
    for(i=0; i<cells.len; i++){
      c=&cells[i];
      if(c->type==CELL_FOUNDATION || c==t) continue;
      for(k=0; k<c->Ncards; k++){
        if(k==0 && (c->inRule&RANK_MASK)==(RANK_NO<<RANK_ROT)) continue;
        r1=CARD_RANK(c->cards[k]);
        switch(rule){
        case SEQ_RANK:
          if(r==r1) return 0;
          break;
        case SEQ_UP:
          if(r==r1-1) return 0;
          break;
        case SEQ_DOWN:
          if(r==r1+1) return 0;
          break;
        }
      }
    }
  }
  return 1;
}

void Tboard::autoPlay()
{
  int i,j,k,n,destType,f;
  Tcell *c,*t;
  bool disabled;
  bool moved,moved1;
  TundoRec *u;

  if(!game || finished) return;
  disabled=false;
  if(undoPos>0){
    u=&undoRec[undoPos-1];
    if(cells[u->src].type==CELL_FOUNDATION && cells[u->dest].type!=CELL_FOUNDATION) disabled=true;
  }
  n=0;
  do{
    moved=false;
    for(i=0; i<cells.len; i++){
      c=&cells[i];
      if(c->type==CELL_FOUNDATION && game->autoplay && globalAutoplay && !disabled){
        for(j=0; j<cells.len; j++){
          t=&cells[j];
          if((t->type!=CELL_FOUNDATION || (cells[i].inRule&SEQ_MASK)==SEQ_RANK<<SEQ_ROT) &&
            t->autoplay && (t->autoplay!=AUTO_BASE || autoBase(t)) &&
            (c->Ncards>0 || (!isSameBase(c) || baseRank()) &&
            (c->inRule&RANK_MASK))){ 
              for(k=0; k<t->Ncards; k++){
                if(animMove(i,j,k,ANIM_AUTOPLAY)){
                  calcDone();
                  n++;
                  speedAccel=n/5+1;
                  moved=true;
                  goto lf;
                }
              }
          }
        }
      }
    }
lf:
    for(i=0; i<cells.len; i++){
      c=&cells[i];
      if(c->fill){
        f= max(0,c->Ncards-c->fill);
        destType=chooseDest(i);
        moved1=false;
        for(j=0; j<cells.len && c->Ncards>0; j++){
          t=&cells[j];
          if(t->Ncards==0 && (destType&(1<<t->type)) && 
            (t->type!=CELL_FOUNDATION || 
            colorTest(t->inRule,c->cards[f]) && 
            rankTest(t->inRule,c->cards[f]))){
            if(!moved1){
              if(moves>0) moves--;
              moved1=true;
            }
            animMove1(j,i,f,ANIM_STOCK);
          }
        }
        if(moved1){
          moved=true;
          moves++;
        }
      }
    }
  }while(moved);
  speedAccel=1;
  finished=(this->*(game->checkFinish))();
  calcDone();
  if(finished){
    KillTimer(hWin,128);
    playerFile.saveGameStat(this);
    if(!playerFile.error){
      playerFile.deletePosition(game->name,number);
    }
  }
}

//------------------------------------------------------------------

int Tboard::testStart(Tcell *c, int d, int ind)
{
  int k,r1,s1,rule;
  Tcard card,card1;

  r1=d%13+1;
  s1=(d/13)%4;
  card1=MAKECARD(r1,s1);
  if(ind==0){
    for(k=(c->type!=CELL_FOUNDATION || c->startRule); k<2; k++){
      rule= k ? c->startRule : c->inRule;
      if(!rankTest(rule,card1)) return 0;
      if(!colorTest(rule,card1)) return 0;
    }
  }else{
    card=c->cards[ind-1];
    if(!suitTest(c->startRule,card,card1)) return 0;
    if(!seqTest(c->startRule,card,card1,0)) return 0;
  }
  return 1;
}

bool Tboard::deal(Tcell *c, bool *bitmask, int total)
{
  int k,d,d0,Ncnt;

  k=c->Ncards;
  if(k==c->startCount) return false;
  //check rules
  Ncnt=0;
  d0=-1;
  for(d=0; d<total; d++){
    if(!bitmask[d] && testStart(c,d,k)){
      Ncnt++;
      if(!rand(Ncnt)) d0=d;
    }
  }
  if(d0<0) return false;
  bitmask[d0]=true;
  c->cards[k]= MAKECARD_START(d0);
  c->Ncards++;
  c->hidden[k]= testHide(c,k,c->startCount);
  return true;
}

void Tboard::newGame(Tgame *game)
{
  int i,j,k,n,total,row,col,Ncolumn,m,sc,d,r,f;
  Tcell *c,*c0;
  TcellElem *c1;
  bool *bitmask;
  Tcard card,card1;

  if(!game) return;
  if(!finished && (moves || redoPos)){
    playerFile.saveGameStat(this);
  }
  cells.reset();
  this->game=game;
  if(this==&board){
    _tcscpy(gameName,game->name);
    if(mustLoadRules){
      loadRules();
      game=this->game;
      if(!game) return;
    }
  }
  number= seed;
  for(i=0; i<100; i++) rand(0);
  randomRank= rand(13)+1;
  waste=-1;
  //initialize bitmask according to game suit and rank
  n=total=game->decks*13*4;
  bitmask= new bool[total];
  memset(bitmask,0,total*sizeof(bool));
  for(i=0; i<4; i++){
    if((game->suit & (1<<i))==0){
      for(j=0; j<game->decks; j++){
        for(k=0; k<13; k++){
          bitmask[(j*4+i)*13+k]=true;
          n--;
        }
      }
    }
  }
  for(k=0; k<13; k++){
    if((game->rank & (1<<k))==0){
      for(j=0; j<4*game->decks; j++){
        bool &b=bitmask[j*13+k];
        if(!b){
          b=true;
          n--;
        }
      }
    }
  }
  for(i=0; i<game->cells.len; i++){
    c1=game->cells[i];
    Ncolumn= (c1->repeat-1)/c1->lines+1;
    if(c1->shape==SHAPE_PYRAMID){
      Ncolumn+= (c1->lines+1)/2-1;
    }
    sc=c1->startCount;
    if(sc==0x7fff) sc=n;
    row=1;
    col=0;
    for(j=0; j<c1->repeat; j++){
      c=cells++;
      //copy game->cells to cells
      memcpy((Tcell0*)c,(Tcell0*)c1,sizeof(Tcell0));
      for(k=0; k<sizeA(indHDCS); k++){
        if((c->inRule&CLR_MASK)==indHDCS[k]){
          c->inRule= (c1->inRule&~CLR_MASK)|HDCS[k][j%4];
        }
        if((c->startRule&CLR_MASK)==indHDCS[k]){
          c->startRule= (c1->startRule&~CLR_MASK)|HDCS[k][j%4];
        }
      }
      for(k=0; k<2; k++){
        if(c->blockedBy[k]==UNDER_NEXT) c->blockedBy[k]=c1->repeat-j;
        if(c->blockedBy[k]==UNDER_NEXT1) c->blockedBy[k]=c1->repeat-j+1;
      }
      c->redealCount=0;
      c->Ncards=0;
      if(c->type==CELL_WASTE){
        if(waste==-1) waste=c-cells;
        else waste=-2;
      }
      //x position
      if(c1->shape==SHAPE_BIND){
        m= 2*c1->w/(2*Ncolumn-1);
        c->x+= col*m;
        k=row&1;
        if(!k) c->x+= m/2;
        if(row<c1->lines){
          if(!k || col>0) c->blockedBy[0]= Ncolumn-k;
          if(k || col<Ncolumn-1) c->blockedBy[1]= Ncolumn-k+1;
        }
      }else if(Ncolumn>1){
        c->x+= col*c1->w/(Ncolumn-1);
        if(c1->shape==SHAPE_PYRAMID){
          c->x+= (c1->lines-row)*c1->w/(Ncolumn-1)/2;
          if(row<c1->lines){
            c->blockedBy[0]= row;
            c->blockedBy[1]= row+1;
          }
        }
        if(c1->shape==SHAPE_BRAID){
          k=row&1;
          if(k) c->x+= c1->w/(Ncolumn-1)/2;
          if(row<c1->lines){
            if(k || col>0) c->blockedBy[0]= Ncolumn-1;
            if(k || col<Ncolumn-1) c->blockedBy[1]= Ncolumn;
          }
        }
      }
      col++;
      //y position
      if(c1->lines>1){
        c->y+= (row-1)*c1->h/(c1->lines-1);
        k=Ncolumn;
        if(c1->shape==SHAPE_PYRAMID) k=row;
        if(c1->shape==SHAPE_BRAID && (row&1)) k--;
        if(col==k){
          col=0;
          row++;
        }
      }
      //count of cards
      k=sc/c1->repeat; 
      if(c1->repeat>1){
        m=sc%c1->repeat;
        switch(c1->shape){
        case SHAPE_INCR:
        case SHAPE_DECR:
        case SHAPE_INCR2:
        case SHAPE_DECR2:
          k=j;
          if(c1->shape==SHAPE_DECR || c1->shape==SHAPE_DECR2){
            k=c1->repeat-j-1;
          }
          m= c1->repeat*(c1->repeat-1);
          if(c1->shape==SHAPE_INCR2 || c1->shape==SHAPE_DECR2){
            k*=2;
          }else{
            m/=2;
          }
          m=sc-m;
          if(m>0){
            d=(m-1)/c1->repeat+1;
            if(k==0) k=m-d*c1->repeat;
            k+=d;
          }
          break;
        default: //SHAPE_LEFT, SHAPE_PYRAMID, SHAPE_BRAID, SHAPE_BIND
          if(j<m) k++;
          break;
        case SHAPE_RIGHT:
          if(j>=c1->repeat-m) k++;
          break;
        case SHAPE_CENTER:
          d=(c1->repeat-m)>>1;
          if(j>=d && j<d+m) k++;
          break;
        case SHAPE_SIDE:
          d=m>>1;
          if(j<d || j>=c1->repeat-m+d) k++;
          break;
        case SHAPE_FIRST:
        case SHAPE_LAST:
          k=(sc-1)/c1->repeat+1;
          if(j== (c1->shape==SHAPE_FIRST ? 0:c1->repeat-1)){
            k+=sc-k*c1->repeat;
          }
          break;
        }
      }
      c->startCount=k;
      n-=c->startCount;
      //deal the first card
      if(c->type==CELL_FOUNDATION && c->startCount>0 && c1->repeat%4==0){
        r=(c->inRule&RANK_MASK)>>RANK_ROT;
        if(r>=1 && r<=13){
          d=(j%4)*13+(r-1);
          if(testStart(c,d,0)){
            for(k=0; k<game->decks; k++){
              if(!bitmask[d]){
                bitmask[d]=true;
                c->cards[0]= MAKECARD(r,j);
                c->hidden[0]=false;
                c->Ncards++;
                goto l1;
              }
              d+=52;
            }
          }
        }
      }
      deal(c,bitmask,total);
l1:;
    }
  }

  f=n=0;
  for(i=0; i<cells.len; i++){
    c=&cells[i];
    for(k=0; k<2; k++){
      if(c->blockedBy[k]==UNDER_WASTE && waste){
        c->blockedBy[k]=waste-i;
      }
    }
    if(c->type==CELL_FOUNDATION){
      f++;
      if((c->inRule&SEQ_MASK)==(SEQ_RANK<<SEQ_ROT)){
        amax(c->maxCount, game->Nsuit()*game->decks);
      }
      if((c->inRule&SUIT_MASK)==(SUIT_SAME<<SUIT_ROT)){
        amax(c->maxCount, game->Nrank()*game->decks);
      }
      n+=c->maxCount;
    }
    //deal
    for(k=1; k<c->startCount; k++){
      if(!deal(c,bitmask,total)) break;
    }
    //set previous cells offsets
    c->prevInd=-1;
    d= c->prevDir;
    if(d<cells.len){
      if(d>=0) c->prevInd=d;
      else if(i+d>=0) c->prevInd=i+d;
    }else{
      m=-1;
      for(j=0; j<cells.len; j++){
        c0=&cells[j];
        switch(d){
        case PREV_LEFT:
          if(c0->y==c->y && c0->x<c->x && c0->x>m){
            m=c0->x;
            c->prevInd=j;
          }
          break;
        case PREV_RIGHT:
          if(c0->y==c->y && c0->x>c->x && (c0->x<m || m<0)){
            m=c0->x;
            c->prevInd=j;
          }
          break;
        case PREV_UP:
          if(c0->x==c->x && c0->y<c->y && c0->y>m){
            m=c0->y;
            c->prevInd=j;
          }
          break;
        case PREV_DOWN:
          if(c0->x==c->x && c0->y>c->y && (c0->y<m || m<0)){
            m=c0->y;
            c->prevInd=j;
          }
          break;
        }
      }
    }
  }
  //set checkFinish
  game->checkFinish= &Tboard::finishFoundation;
  i=game->cards-n;
  if(i>0){
    game->checkFinish= &Tboard::finishFullFoundation;
    if(n==0 || i>8){
      game->checkFinish= &Tboard::finishFan;
    }
  }
  delete[] bitmask;
  //move cards to foundations during deal, if such moves are legal
  for(i=0; i<cells.len; i++){
    c=&cells[i];
    if(c->deal==DEAL_START || c->deal==DEAL_START_COL || c->deal==DEAL_START1){
      for(j=0; j<cells.len; j++){
        c0=&cells[j];
        if(c0->deal==c->deal || c0->shape==SHAPE_BRAID ||
          c->deal==DEAL_START_COL && c0->x!=c->x) continue;
        for(k=0; k<c0->Ncards; k++){
          card=c0->cards[k];
          m=c->Ncards;
          if(m){
            if(m>=c->maxCount || c->deal==DEAL_START1) break;
            card1=c->cards[m-1];
            if(!seqTest(c->inRule,card1,card,0) || 
              !suitTest(c->inRule,card1,card)) continue;
          }else{
            if(!rankTest(c->inRule,card) || 
              !colorTest(c->inRule,card)) continue;
          }
          //move card to the foundation
          c->cards[m]=card;
          c->hidden[m]=false;
          c->Ncards++;
          for(d=k+1; d<c0->Ncards; d++){
            c0->cards[d-1]=c0->cards[d];
          }
          c0->Ncards--;
          k--;
        }
      }
    }
  }
  undoRec.reset();
  undoPos=redoPos=0;
  finished=false;
  playtime=0;
  moves=0;
  scoreUnhide=scoreUndo=scoreWrongMove=scoreRedeal=0;
  if(this==&board){
    calcDone();
    statusTime();
    invalidate();
    static TCHAR buf[64];
    _sntprintf(buf,sizeA(buf)-1,_T("%s - %u"),game->name,number);
    SetWindowText(hWin,buf);
    SetTimer(hWin,128,1000,0);
  }
}

void Tboard::newGame(TCHAR *name)
{
  newGame(findGame(name));
}

//------------------------------------------------------------------

bool Tboard::undo1()
{
  int g,n;
  TundoRec *r;

  if(undoPos<=0) return false;
  if(!redoPos) redoPos=undoPos;
  undoing++;
  n=0;
  do{
    r=&undoRec[--undoPos];
    g=r->group;
    animMove1(r->src,r->dest,r->destInd,ANIM_UNDO);
    if(r->unhide){
      cells[r->src].hidden[r->ind-1]=true;
      invalidateColumn(r->src,r->ind-1,1);
      if(cells[r->src].type!=CELL_STOCK) scoreUnhide--;
    }
    n++;
    speedAccel=n/10+1;
  }while(undoPos>0 && undoRec[undoPos-1].group==g);
  speedAccel=1;
  moves=g;
  undoing--;
  return true;
}

bool Tboard::redo1()
{
  int g,n;
  TundoRec *r;

  if(!redoPos || redoPos==undoPos) return false;
  n=0;
  do{
    r=&undoRec[undoPos];
    g=r->group;
    animMove1(r->dest,r->src,r->ind,ANIM_UNDO);
    n++;
    speedAccel=n/10+1;
  }while(undoPos<redoPos && undoRec[undoPos].group==g);
  speedAccel=1;
  moves=g+1;
  return true;
}

void Tboard::undo()
{
  if(undo1()) calcDone();
}

void Tboard::redo()
{
  if(redo1()) calcDone();
}

void Tboard::undoAll()
{
  notdraw++;
  while(undo1()) ;
  notdraw--;
  calcDone();
}

void Tboard::redoAll()
{
  notdraw++;
  while(redo1()) ;
  notdraw--;
  calcDone();
}

//------------------------------------------------------------------

void dtcat(Darray<TCHAR> &buf, TCHAR *s)
{
  dtprintf(buf,_T("%s"),s);
}

bool equalCell(TcellElem *c, TcellElem *d)
{
  return c->type==d->type && c->inRule==d->inRule && c->outRule==d->outRule && c->prevRule==d->prevRule && c->sameRule==d->sameRule && c->redeal==d->redeal && c->dir==d->dir; 
}

int Tgame::Nsuit()
{
  int i=0;
  if(suit&1) i++;
  if(suit&2) i++;
  if(suit&4) i++;
  if(suit&8) i++;
  return i;
}

int Tgame::Nrank()
{
  int i,n;
  n=0;
  for(i=1; i<(1<<13); i<<=1){
    if(rank & i) n++;
  }
  return n;
}

char *rankWord[]={"any","A","2","3","4","5","6","7","8","9","10","J","Q","K","no","random base","random base + 1","same base","at first all kings and then any card","A or K"};
char *colorWord[]={"any","hearts","diamonds","clubs","spades","red","black"};
char *suitWord[]={"any","same","different","same color","alternating color"};
char *seqWord[]={"any","no","descending","ascending","up or down","descending odd/even","ascending odd/even","up or down odd/even","same rank","up or down or same","previous pile has a card of the same rank as the moved card","previous pile is fully completed","higher","lower","up or same","down or same"};
char *groupWord[]={"yes","no","not all cards","any sequence","all 13 cards","to empty cell","any sequence when free cells","entire pile regardless of sequence"};
char *cellWord[]={"cell","foundation","tableau","reserve","stock","waste"};
char *pairWord[]={"same suit","same rank","same color and rank"};

void generRules(Tgame *g)
{
  int i,j,k,n,rule;
  unsigned v;
  TcellElem *c,*d;

  if(!g) return;
  HWND w= GetDlgItem(rulesWnd,IDC_RULES);
  Darray<TCHAR> t;
  dtprintf(t,_T("%s\r\n"),g->name);
  v=g->pair;
  if(v){
    dtprintf(t,_T("%s: "),lng(900,"Pairs"));
    if(v<256) dtprintf(t,_T("%d\r\n"),v);
    else{
      v-=256;
      if(v<sizeA(pairWord)) dtprintf(t,_T("%s\r\n"),lng(1120+v,pairWord[v]));
    }
  }
  if(g->decks>1){
    dtprintf(t,_T("%s: %d\r\n"),lng(901,"Decks"),g->decks);
  }
  j=g->Nsuit();
  if(j!=4){
    dtprintf(t,_T("%s: %d\r\n"),lng(913,"Suits"),j);
  }
  j=g->rank;
  if(j!=0x1fff){
    dtprintf(t,_T("%s: "),lng(914,"Ranks"));
    for(i=0; i<13; i++){
      if(j&(1<<i)) dtprintf(t,_T("%s "),rankWord[i+1]);
    }
    dtprintf(t,_T("\r\n"));
  }
  for(i=0; i<g->cells.len; i++){
    c= g->cells[i];
    for(j=i+1; j<g->cells.len; j++){
      d = g->cells[j];
      if(equalCell(c,d)) goto l1;
    }
    n= c->repeat;
    for(j=0; j<i; j++){
      d = g->cells[j];
      if(equalCell(c,d)) n+=d->repeat;
    }
    dtprintf(t,_T("\r\n%dx  %s\r\n"),n,lng(1100+c->type,cellWord[c->type]));
    if(c->deal>1 && c->deal<256){
      dtprintf(t,_T("%s: %d\r\n"),lng(915,"Deal"),c->deal);
    }
    if(c->redeal){
      dtprintf(t,_T("%s: "),lng(902,"Redeals"));
      if(c->redeal==0x7fffffff) dtprintf(t,_T("%s\r\n"),lng(903,"unlimited"));
      else dtprintf(t,_T("%d\r\n"),c->redeal);
    }
    if(c->maxCount<255){
      dtprintf(t,_T("%s: %d\r\n"),lng(904,"Maximum cards per pile"),c->maxCount);
    }
    if(c->blockedBy[0]==UNDER_WASTE || c->blockedBy[1]==UNDER_WASTE){
      dtprintf(t,_T("%s\r\n"),lng(921,"Waste must be empty"));
    }
    for(k=0; k<4; k++){
      rule= k ? (k==3 ? c->prevRule : (k==2 ? c->sameRule : c->outRule)) : c->inRule;
      if(rule){
        if(k==1 && ((rule&~GROUP_MASK) || c->dir!=DIR_NO)){
          dtprintf(t,_T("- %s\r\n"),lng(916,"Outgoing rules"));
        }
        if(k==2){
          dtprintf(t,_T("- %s\r\n"),lng(919,"Rules for moves between cells of the same type"));
        }
        if(k==3){
          dtprintf(t,_T("- %s\r\n"),lng(920,"Rules for a previous cell"));
        }
      }
      v=(rule&RANK_MASK)>>RANK_ROT;
      if(v==RANK_NO && k!=1){
        if(c->type!=CELL_STOCK) dtprintf(t,_T("%s\r\n"),lng(905,"Spaces cannot be filled"));
      }else{
        if(v!=0 || c->type==CELL_STOCK && k==0){
          if(v<sizeA(rankWord)){
            dtprintf(t,_T("%s:  %s\r\n"),lng(906,"The first card rank"),lng(1000+v,rankWord[v]));
          }
        }
        v=(rule&CLR_MASK)>>CLR_ROT;
        if(v && v<sizeA(colorWord)) dtprintf(t,_T("%s:  %s\r\n"),lng(907,"The first card suit"),lng(1030+v,colorWord[v]));
      }
      v=(rule&SEQ_MASK)>>SEQ_ROT;
      if(v==SEQ_NO){
        if(c->type!=CELL_STOCK) dtprintf(t,_T("%s\r\n"),lng(908,"No building"));
      }else{
        if(v && v<sizeA(seqWord)){
          dtprintf(t,_T("%s  %s %s\r\n"),(v==SEQ_SYNCH)?_T(""):lng(909,"Sequence rank:"),lng(1060+v,seqWord[v]),
          (rule&CONT_MASK) ? lng(910,"continual"):_T(""));
        }
        v=(rule&SUIT_MASK)>>SUIT_ROT;
        if(v && v<sizeA(suitWord)) dtprintf(t,_T("%s:  %s\r\n"),lng(911,"Sequence suit"),lng(1050+v,suitWord[v]));
      }
      v=(rule&GROUP_MASK)>>GROUP_ROT;
      if(v<sizeA(groupWord) && (v!=0 || k==0) && (v!=GROUP_NO || k>0 && c->dir!=DIR_NO)){
        dtprintf(t,_T("%s  %s\r\n"),
        (v==GROUP_NOEMPTY) ? _T("") : lng(912,"Moving group of cards:"),
        lng(1080+v,groupWord[v]));
      }
      v=(rule&DEST_MASK)>>DEST_ROT;
      if(v && (c->type!=CELL_WASTE || (v&~(1<<CELL_STOCK)))){
        dtprintf(t,_T("%s: "),lng(917+k, (char*)(k ? "Move only to":"Move only from")));
        for(j=0; j<sizeA(cellWord); j++){
          if(v&(1<<j)) dtprintf(t,_T(" %s"),lng(1100+j,cellWord[j]));
        }
        dtprintf(t,_T("\r\n"));
      }
    }
l1:;
  }
  SetWindowText(w,t);
  /*SendMessage(w,EM_SETSEL,1,1);
  CHARFORMAT cf;
  cf.cbSize=sizeof(CHARFORMAT);
  cf.dwMask=CFM_BOLD;
  cf.dwEffects=CFE_BOLD;
  SendMessage(w,EM_SETCHARFORMAT,SCF_WORD|SCF_SELECTION,(LPARAM)&cf);
  */
}
//------------------------------------------------------------------
