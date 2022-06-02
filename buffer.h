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
		
		void cat(char *s);
		char *get() { return buf; }
		int getLength() { return len; }
		SDL_Surface *toSurface(TTF_Font *ft, SDL_Color& color);
};

class CStory {
	private:
		std::vector<CParagraph *>text;
		int cx, cy;
		int sx, sy;
		
	public:
		CStory();
		~CStory();
		
		void append(char *s);
		SDL_Surface *toSurface(int idx, TTF_Font *ft, SDL_Color& color);
};

#endif
