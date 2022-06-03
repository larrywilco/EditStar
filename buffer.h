#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>

class CParagraph {
	private:
		char *buf;
		size_t len;
		size_t alloc;
		
	public:
		CParagraph();
		~CParagraph();
		
		void cat(char *s, int n);
		char *get() { return buf; }
		int getLength() { return len; }
};

class CStory;

class CFrameBuffer {
	private:
		std::vector<char *>buf;
		std::vector<SDL_Surface *>lines;
		TTF_Font *font;
		SDL_Rect rect;
	public:
		CFrameBuffer();
		~CFrameBuffer();
		void prepare(TTF_Font *font, SDL_Rect& r, CStory& story);
		SDL_Surface * render(SDL_Color& color);
		void freeBuffer();
		std::vector<SDL_Surface *>& getLines() { return lines; }
};

class CStory {
	friend class CFrameBuffer;
	protected:
		std::vector<CParagraph *>text;
		int top, right, bottom, left;
		
	public:
		CStory();
		~CStory();
		
		void append(char *s, int n);
		void newline();
};

#endif
