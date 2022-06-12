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
		
	public:
		CParagraph();
		~CParagraph();
		
		void cat(char *s, int n=0);
		char *get() { return buf; }
		int size() { return len; }
		int append(char *s);
		void del(int column);
		char *left(int c);
};

class CStory;

class CLine {
	public:
		int start, end, len;
		char *buf;
		CLine();
		~CLine();
		CLine(char *s);
};

class CFrameBuffer {
	private:
		std::vector<CLine *>buf;
		std::vector<SDL_Surface *>lines;
		TTF_Font *font;
		SDL_Rect rect;
	public:
		int column, row;
		SDL_Rect cursor;
		
		CFrameBuffer();
		~CFrameBuffer();
		void prepare(TTF_Font *font, SDL_Rect& r, CStory& story);
		SDL_Surface * render(SDL_Color& color);
		void freeBuffer();
		std::vector<SDL_Surface *>& getLines() { return lines; }
		void newLine();
		void moveLeft(CStory& story);
		void moveRight(CStory& story);
		void backspace(CStory& story);
};

class CStory {
	friend class CFrameBuffer;
	protected:
		std::vector<CParagraph *>text;
		int top, right, bottom, left;
		
	public:
		CStory();
		~CStory();
		
		int append(char *s);
		void newline();
		void delline(int idx);
};

#endif
