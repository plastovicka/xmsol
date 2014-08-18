/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef xmlH
#define xmlH

#include "darray.h"

class TxmlParser;

struct TxmlElem {
	int tag;
	TxmlElem *parent;
	TxmlParser *parser;
	TxmlElem(int tag) :tag(tag){};
	virtual void addAttr(TCHAR * /*name*/, TCHAR * /*value*/){};
	virtual void addChild(TxmlElem *elem){ delete elem; };
	virtual void setText(TCHAR *){};
};

struct TxmlTextElem : public TxmlElem {
	TCHAR *value;
	TxmlTextElem(int tag) :TxmlElem(tag){};
	void setText(TCHAR *text);
};


class TxmlParser {
public:
	TxmlParser() :line(1), column(0){};
	virtual ~TxmlParser(){};
	TxmlElem* parse();
	virtual void error(char *){};

protected:
	int line, column;

private:
	virtual int getch()=0;
	virtual void ungetch(int c)=0;
	virtual int eof()=0;
	virtual TxmlElem *newElem(TCHAR *name, TxmlElem *parent)=0;

	int get();
	void unget(int c);
	void skipSpaces();
	void readEndBracket();
	void readName(Darray<TCHAR> &buf);
	int readTag(Darray<TCHAR> &buf);
	int readAmp();
	void readStr(Darray<TCHAR> &buf);
	TxmlElem* parseElem(TCHAR *name, TxmlElem *parent);
};

class TxmlFileParser : public TxmlParser {
	FILE *f;
public:
	TxmlFileParser(FILE *f) : f(f){};
	int getch();
	void ungetch(int c);
	int eof();
};

#endif
