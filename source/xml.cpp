/*
	(C) 2005  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include "hdr.h"
#include "xml.h"
#include "lang.h"

WCHAR *dupStr(WCHAR *s)
{
	if(!s) return 0;
	WCHAR *result= new WCHAR[wcslen(s)+1];
	wcscpy(result, s);
	return result;
}

char *dupStr(char *s)
{
	if(!s) return 0;
	char *result= new char[strlen(s)+1];
	strcpy(result, s);
	return result;
}

void TxmlTextElem::setText(TCHAR *text)
{
	value=dupStr(text);
}

int unicode2tchar(int c)
{
#ifdef UNICODE
	return c;
#else
	WCHAR u= (WCHAR)c;
	CHAR a;
	WideCharToMultiByte(CP_ACP, 0, &u, 1, &a, 1, NULL, NULL);
	return (unsigned char)a;
#endif
}

int TxmlFileParser::getch()
{
	int c, c1, c2;
start:
	c=fgetc(f);
	if(c>127){
		if(c>=0xe0){
			c1=fgetc(f);
			c2=fgetc(f);
			c= ((((c&0xf)<<6)|(c1&0x3f))<<6)|(c2&0x3f);
			if(c==0xfeff) goto start;
		}
		else{
			c= ((c&0x1f)<<6)|(fgetc(f)&0x3f);
		}
		c=unicode2tchar(c);
	}
	return c;
}

int TxmlParser::get()
{
	int c=getch();
	column++;
	if(c=='\n'){
		line++;
		column=0;
	}
	return c;
}

void TxmlFileParser::ungetch(int c)
{
	ungetc(c, f);
}

void TxmlParser::unget(int c)
{
	ungetch(c);
	column--;
	if(c=='\n') line--;
}

int TxmlFileParser::eof()
{
	return feof(f);
}

void TxmlParser::skipSpaces()
{
	int c;
	do{
		c=get();
	} while(c=='\n' || c==' ' || c=='\t');
	unget(c);
}

void TxmlParser::readEndBracket()
{
	skipSpaces();
	if(get()!='>') error("> expected");
}

struct TcharName {
	char *name;
	char ch;
} charNames[]={
		{"amp", '&'},
		{"lt", '<'},
		{"gt", '>'},
		{"apos", '\''},
		{"quot", '\"'},
};

int TxmlParser::readAmp()
{
	int j, c;
	char amp[8];

	for(j=0;;){
		c=get();
		if(c==';') break;
		if(c==EOF || c=='\n' || c==' ' || c=='\t'){
			error("Semicolon expected after ampersand sequence");
			break;
		}
		if(j<sizeof(amp)-1) amp[j++]=(char)c;
	}
	amp[j]='\0';
	if(amp[0]=='#'){
		if(amp[1]=='x'){
			c=(int)strtol(amp+2, 0, 16);
		}
		else{
			c=atoi(amp+1);
		}
		return unicode2tchar(c);
	}
	else{
		for(TcharName *p=charNames; p<endA(charNames); p++){
			if(!strcmp(amp, p->name)){
				return p->ch;
			}
		}
		return '&';
	}
}

void TxmlParser::readStr(Darray<TCHAR> &buf)
{
	int c, i, u;

	buf.reset();
	u=get();
	if(u!='\"' && u!='\''){
		error("Value is not in quotation marks");
		unget(u);
		u='\n';
	}
	for(i=0;;){
		c=get();
		if(c==u) break;
		if(u=='\n'){
			if(c==' ' || c=='\t') break;
			if(c=='>'){ unget(c); break; }
		}
		if(/*c=='\n' ||*/ c==EOF){
			error("Unterminated string");
			break;
		}
		*buf++=(TCHAR)((c=='&') ? readAmp() : c);
	}
	*buf++='\0';
}

void TxmlParser::readName(Darray<TCHAR> &buf)
{
	buf.reset();
	for(;;){
		int c=get();
		if(c==EOF || c==' ' || c=='\t' || c=='\n') break;
		if(c=='>' || c=='/' || c=='='){ unget(c); break; }
		*buf++=(TCHAR)c;
		if(c=='>') break;
	}
	*buf++='\0';
}

int TxmlParser::readTag(Darray<TCHAR> &buf)
{
	for(;;){
		int c=get();
		if(c!='<') return c;
		readName(buf);
		if(!_tcsncmp(buf, _T("!--"), 3)){
			while((get()!='-' || get()!='-' || get()!='>') && !eof());
		}
		else if(buf[0]=='?'){
			while((get()!='?' || get()!='>') && !eof());
		}
		else if(buf[0]=='!'){
			int h=0;
			while(!eof() && h>=0){
				int c=get();
				if(c=='>') h--;
				if(c=='<') h++;
			}
		}
		else return 0;
	}
}

TxmlElem* TxmlParser::parseElem(TCHAR *name, TxmlElem *parent)
{
	int c;
	TxmlElem *elem;
	Darray<TCHAR> text, buf, buf2;

	elem=newElem(name, parent);
	if(!elem) elem=new TxmlElem(0);
	elem->parser=this;
	elem->parent=parent;

	//attributes
	for(;;){
		skipSpaces();
		readName(buf);
		skipSpaces();
		c=get();
		if(!buf[0]){
			if(c=='>' || c==EOF) break;
			if(c=='/') goto le; //empty element
		}
		if(c!='='){
			error("= expected after attribute name");
			unget(c);
		}
		else{
			skipSpaces();
			readStr(buf2);
			elem->addAttr(buf, buf2);
		}
	}
	//child elements and text
	for(;;){
		c=readTag(buf);
		if(c){
			if(c==EOF) break;
			*text++=(TCHAR)((c=='&') ? readAmp() : c);
		}
		else{
			if(buf[0]==0){
				c=get();
				if(c=='/'){
					readName(buf);
					if(_tcscmp(buf, name)) error("end tag mismatch");
					break;
				}
			}
			elem->addChild(parseElem(buf, elem));
		}
	}
le:
	readEndBracket();
	*text++='\0';
	elem->setText(text);
	return elem;
}

TxmlElem* TxmlParser::parse()
{
	Darray<TCHAR> buf;
	for(;;){
		int c=readTag(buf);
		if(!c && buf[0]) return parseElem(buf, 0);
		if(c!=' ' && c!='\n' && c!='\t') return 0;
	}
}
