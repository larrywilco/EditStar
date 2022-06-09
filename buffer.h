#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>

class CParagraph {
	private:
		char *buf; // UTF32 string
		size_t len;
		size_t alloc;
		
	public:
		CParagraph();
		~CParagraph();
		
		void cat(char *s, int n);
		char *get() { return buf; }
		int size() { return len; }
		int count();
		char *toUtf8(int nchar = 0); // Number of character. Not bytes
		int appendUtf8(char *s);
		void del(int column);
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
		int estChars;
	public:
		int column, row;
		SDL_Rect cursor;
		
		CFrameBuffer();
		~CFrameBuffer();
		void prepare(TTF_Font *font, SDL_Rect& r, CStory& story);
		SDL_Surface * render(SDL_Color& color);
		void freeBuffer();
		std::vector<SDL_Surface *>& getLines() { return lines; }
		void estimateLineLength(TTF_Font *ft, SDL_Rect& r);
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
