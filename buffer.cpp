#include <stdlib.h>
#include <string.h>
#include "buffer.h"

#define DEFAULT_BLOCK_SIZE 1024

CParagraph::CParagraph() {
	buf = (char *)calloc(sizeof(char), DEFAULT_BLOCK_SIZE);
	len = 0;
	alloc = DEFAULT_BLOCK_SIZE;
}

CParagraph::~CParagraph() {
	if (buf) free(buf);
	buf = NULL;
}

void CParagraph::cat(char *s) {
	size_t l = len + strlen(s);
	if (l < alloc) strcat(buf, s);
	len = l;
}

SDL_Surface *CParagraph::toSurface(TTF_Font *ft, SDL_Color& color) {
	SDL_Surface *surface = TTF_RenderUTF8_Blended(ft, buf, color);
	return surface;
}

CStory::CStory() {
	CParagraph *tmp = new CParagraph();
	text.push_back(tmp);
	cy = 0; cx = 0;
	sx = 0; sy = 0;
}

CStory::~CStory() {
	while(!text.empty()) {
		CParagraph *p = text.back();
		delete p;
		text.pop_back();
	}
}

void CStory::append(char *s) {
	CParagraph *p = text.back();
	p->cat(s);
}

SDL_Surface * CStory::toSurface(int idx, TTF_Font *ft, SDL_Color& color) {
	if (idx >= (int)text.size()) return NULL;
	CParagraph *p = text[idx];
	return p->toSurface(ft, color);
}
