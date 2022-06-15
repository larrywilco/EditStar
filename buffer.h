#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include "stringu8.h"

class CParagraph : public StringU8 {
	private:
		size_t len;
		size_t alloc;
		int tag;
		
	public:
		CParagraph(int seq);
		~CParagraph();
		
		int id() { return tag; }
		void cat(char *s, int n=0);
		char *get() { return buf; }
		int size() { return len; }
		int append(char *s);
		int del(int bytesToSkip, int column);
		void ins(char *s, int pos);
		void clear();
};

class CStory;

class CLine {
	public:
		int width, height;
		int lineno, segbegin, seglen, len;
		int paragraphId;
		bool empty;
		char *buf;
		CLine();
		~CLine();
		CLine(char *s);
};

class CFrameBuffer {
	private:
		const int SPACING = 1;
		std::vector<CLine *>buf;
		std::vector<SDL_Surface *>lines;
		TTF_Font *font;
		SDL_Rect rect;
		void horizontalMove();
		
	public:
		int column, row;
		SDL_Rect cursor;
		
		CFrameBuffer();
		~CFrameBuffer();
		void prepare(TTF_Font *font, SDL_Rect& r, CStory& story);
		SDL_Surface * render(SDL_Color& color);
		void freeBuffer();
		std::vector<SDL_Surface *>& getLines() { return lines; }
		int size() { return buf.size(); }
		void calibrate(CStory& story);
		void newLine();
		void moveLeft(CStory& story);
		void moveRight(CStory& story);
		void backspace(CStory& story);
		void insert(CStory& story, char *txt);
};

class CStory {
	friend class CFrameBuffer;
	private:
		int paragraph_seq;
		struct {
			int line;
			int byte;
		} editPt;
		
	protected:
		std::vector<CParagraph *>text;
		int top;
		
	public:
		CStory();
		~CStory();
		
		int getCurrentLine() { return editPt.line; }
		int getCurrentIndex() { return editPt.byte; }
		int append(char *s);
		void newline();
		void delline(int idx);
		void del(CLine *p, int col);
};

#endif
