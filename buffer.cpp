#include <stdlib.h>
#include <string.h>
#include <iconv.h>
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
	if (l < alloc) {
		alloc += DEFAULT_BLOCK_SIZE;
		buf = (char *)realloc(buf, alloc);
		x = buf + len;
	}
	if (l < alloc) memcpy(x, s, n);
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

char * CStory::toUnicode(char *str, int& nbytes) {
	int allocated = strlen(str)*4+2;
	char * unicode = (char *)calloc(sizeof(char), allocated);
	iconv_t cd = iconv_open("UTF32LE", "UTF8");
	char *s = str;
	char *d = unicode;
	nbytes = 0;
	size_t inbytes = strlen(s);
	size_t outbytes = allocated;
	size_t rc = iconv(cd, &s, &inbytes, &d, &outbytes);
	if (rc == (size_t)-1) {
		free(unicode);
		iconv_close(cd);
		return NULL;
	}
	nbytes = allocated - outbytes;
	iconv_close(cd);
	return unicode;
}

int CStory::append(char *s) {
	int n;
	char *unicode = toUnicode(s, n);
	CParagraph *p = text.back();
	p->cat(unicode, n);
	free(unicode);
	right++;
	return n/4;
}

void CStory::newline() {
	CParagraph *tmp = new CParagraph();
	text.push_back(tmp);
	bottom++;
}

CFrameBuffer::CFrameBuffer() {
	buf.reserve(25);
	column = row = 0;
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
	rect.y = 0;
	for (int y = story.top; y<=story.bottom; y++) {
		size_t inbytes = story.text[y]->getLength();
		if (inbytes<1) {
			char *tmp = (char *)calloc(sizeof(char), 2);
			*tmp = ' ';
			buf.push_back(tmp);
			continue;
		}
		int nbytes = inbytes*2;
		size_t outbytes = nbytes;
		char *utf8 = (char *) calloc(sizeof(char), outbytes);
		iconv_t cd = iconv_open("UTF8", "UTF32LE");
		char *s = story.text[y]->get();
		char *d = utf8;
		size_t rc = iconv(cd, &s, &inbytes, &d, &outbytes);
		int nsize = nbytes - outbytes + 1;
		if (column == y) {
			char *dup = (char *)calloc(sizeof(char), story.text[y]->getLength());
			inbytes = column * 4;
			memcpy(dup, story.text[y]->get(), inbytes);
			s = dup;
			outbytes = nbytes * 2;
			char *s2 = (char *)calloc(sizeof(char), outbytes);
			d = s2;
			iconv(cd, &s, &inbytes, &d, &outbytes);
			TTF_SizeUTF8(font, s2, &txtw, &txth);
			cursor.x = txtw;
			cursor.w = txtw + 1;
			free(s2);
			free(dup);
		}
		iconv_close(cd);

		TTF_SizeUTF8(font, utf8, &txtw, &txth);
		if (column < y) {
			cursor.y += txth;
			cursor.h = cursor.y - txth;
		}
		char *tmp = (char *)calloc(sizeof(char), nsize);
		printf("rc:%ld w:%d h:%d x1:%d x2: %d y1: %d y2: %d\n", 
			rc, txtw, txth, cursor.x, cursor.y, cursor.w, cursor.h);
		if (txtw < r.w) {
			memcpy(tmp, utf8, nsize);
		}
		free(utf8);
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
		SDL_Surface *surface = TTF_RenderUTF8_Blended(font, buf[i], color);
		SDL_BlitSurface(surface, NULL, area, &r);
		r.y += surface->h;
		SDL_FreeSurface(surface);
	}
	return area;
}

