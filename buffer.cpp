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

void CParagraph::cat(char *s, int n) {
	char *x = buf + len;
	size_t l = len + n;
	if (l < alloc) memcpy(x, s, n);
	printf("Copy %d\n", n);
	len += n;
}

CStory::CStory() {
	CParagraph *tmp = new CParagraph();
	text.push_back(tmp);
	top = left = bottom = right = 0;
}

CStory::~CStory() {
	while(!text.empty()) {
		CParagraph *p = text.back();
		delete p;
		text.pop_back();
	}
}

void CStory::append(char *s, int n) {
	CParagraph *p = text.back();
	p->cat(s, n);
	right++;
}

void CStory::newline() {
	CParagraph *tmp = new CParagraph();
	text.push_back(tmp);
	bottom++;
}

CFrameBuffer::CFrameBuffer() {
	buf.reserve(25);
}

CFrameBuffer::~CFrameBuffer() {
	freeBuffer();
}

void CFrameBuffer::freeBuffer() {
	while(!buf.empty()) {
		char *p = buf.back();
		free(p);
		buf.pop_back();
	}
//	buf.clear();
}

// Figure out the range of text fitted into the viewing rect
void CFrameBuffer::prepare(TTF_Font *ft, SDL_Rect& r, CStory& story) {
	int txtw, txth;
	freeBuffer();
	font = ft;
	rect = r;
	for (int y = story.top; y<=story.bottom; y++) {
		char *tmp = (char *)calloc(128, sizeof(char*));
		char *s = story.text[y]->get();
		TTF_SizeUTF8(font, s, &txtw, &txth);
		printf("%d %d\n", txtw, txth);
		if (txtw < r.w) {
			strncpy(tmp, s, 128);
		}
		buf.push_back(tmp);
	}
}

/**
 * Actual render the cropped text into surface.
 * Return: SDL_Surface *; need to free by caller
 * */
SDL_Surface * CFrameBuffer::render(SDL_Color& color) {
	if (!font) return NULL;
	SDL_Surface *area = SDL_CreateRGBSurfaceWithFormat(0, 
			rect.w, rect.h, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Rect r = {0, 0, 0, 0};
	for(int i=0; i<(int)buf.size(); i++) {
		printf("Len:%ld\n", wcslen((wchar_t *)buf[i]));
		SDL_Surface *surface = TTF_RenderUNICODE_Blended(font, (Uint16*)buf[i], color);
		SDL_BlitSurface(surface, NULL, area, &r);
		r.y += surface->h;
		SDL_FreeSurface(surface);
	}
	return area;
}

