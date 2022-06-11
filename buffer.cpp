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
	printf("Destroy p\n");
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

int CParagraph::count() {
	if (len < 1) return 0;
	return len/4;
}

char *CParagraph::toUtf8(int nchar) {
	if (len < 1) return NULL;
	if ((nchar *4) > (int)len) return NULL;
	size_t inbytes = len;
	int mem = len*2;
	size_t outbytes = mem;
	char *utf8 = (char *)calloc(sizeof(char), mem);
	char *dup = NULL;
	if (nchar > 0) {
		dup = (char *)calloc(sizeof(char), len+1);
		memcpy(dup, buf, nchar *4);
		inbytes = nchar * 4;
	}
	iconv_t cd = iconv_open("UTF8", "UTF32LE");
	char *s = buf;
	if (dup) s = dup;
	char *d = utf8;
	size_t rc = iconv(cd, &s, &inbytes, &d, &outbytes);
	if (dup) free(dup);
	dup = NULL;
	if (rc == (size_t)-1) {
		free(utf8);
		iconv_close(cd);
		return NULL;
	}
	return utf8;
}

int CParagraph::appendUtf8(char *str) {
	int allocated = strlen(str)*4+2;
	char * unicode = (char *)calloc(sizeof(char), allocated);
	iconv_t cd = iconv_open("UTF32LE", "UTF8");
	char *s = str;
	char *d = unicode;
	size_t inbytes = strlen(s);
	size_t outbytes = allocated;
	size_t rc = iconv(cd, &s, &inbytes, &d, &outbytes);
	if (rc == (size_t)-1) {
		free(unicode);
		iconv_close(cd);
		return 0;
	}
	size_t nbytes = allocated - outbytes;
	iconv_close(cd);
	cat(unicode, nbytes);
	free(unicode);
	return nbytes/4;
}

void CParagraph::del(int column) {
	int idx = column * 4;
	if (len <= 4) {
		memset(buf, 0, len);
		len =0;
		return;
	}
	if (idx < (int)len) {
		memmove(buf + idx, buf + idx + 4, len - idx - 4);
	}
	len -= 4;
}

char * CParagraph::subUtf8(int start, int end) {
	if (len < 1) return NULL;
	if (end < start) return NULL;
	size_t first, last;
	
	first = start * 4;
	last = end * 4;
	if (last > len) last = len;
	int nbytes = last - first + 2;
	char *substr = (char *)calloc(sizeof(char), nbytes);
	char *converted = (char *)calloc(sizeof(char), nbytes + 10);
	char *s = buf + first;
	memcpy(substr, s, last - first);
	iconv_t cd = iconv_open("UTF32LE", "UTF8");
	s = substr;
	char *d = converted;
	size_t inbytes = strlen(s);
	size_t outbytes = nbytes + 10;
	size_t rc = iconv(cd, &s, &inbytes, &d, &outbytes);
	if (rc == (size_t)-1) {
		free(substr);
		free(converted);
		iconv_close(cd);
		return NULL;
	}
	free(substr);
	return converted;
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

int CStory::append(char *s) {
	CParagraph *p = text.back();
	int chars = p->appendUtf8(s);
	right++;
	return chars;
}

void CStory::newline() {
	CParagraph *tmp = new CParagraph();
	text.push_back(tmp);
	bottom++;
}

void CStory::delline(int idx) {
	CParagraph *p = text[idx];
	text.erase(text.begin() + idx);
	delete p;
	bottom--;
}

CLine::CLine() {
	buf = NULL;
	len = start = end = 0; 
}
CLine::CLine(char *s) { 
	buf = s; 
	start = end = 0; 
	if (s) len = strlen(s);
}

CLine::~CLine() {
	if (buf) free(buf);
	buf = NULL;
}
CFrameBuffer::CFrameBuffer() {
	buf.reserve(25);
	column = row = 0;
	cursor = { 0,0,0, 0};
	estChars =0;
}

CFrameBuffer::~CFrameBuffer() {
	freeBuffer();
}

void CFrameBuffer::freeBuffer() {
	while(!buf.empty()) {
		CLine *p = buf.back();
		buf.pop_back();
		delete p;
	}
	buf.clear();
}

// Figure out the range of text fitted into the viewing rect
void CFrameBuffer::prepare(TTF_Font *ft, SDL_Rect& r, CStory& story) {
	int txtw = 0, txth = 0, idx = row;
	freeBuffer();
	font = ft;
	rect = r;
	rect.y = 0;
	cursor.h = 0;
	int length = story.text.size();
	int height = 0;
	bool good = true;
	int lastpos = 0;
	for (int y = story.top; (y<length) && good; y++) {
		bool toFit = true;
		CParagraph *p = story.text[y];
		size_t inbytes = p->size();
		int nchars = p->count();
		for(int i = 1; i<nchars; i++) {
			char *s = p->toUtf8(i);
		}
		char *utf8 = story.text[y]->toUtf8();
		printf("column: %d y:%d row:%d bytes:%ld\n", column, y, row, inbytes);
		if (!utf8) { // Take care of blank line
			char *tmp = (char *)malloc(sizeof(char)*2);
			*tmp = ' ';
			*(tmp+1) = '\0';
			utf8 = tmp;
		}

		TTF_SizeUTF8(font, utf8, &txtw, &txth);
		height += txth;
		if (height >= r.h) {
			good = false;
			continue;
		}
		if (idx == y) {
			char *s2 = story.text[y]->toUtf8(column);
			TTF_SizeUTF8(font, s2, &txtw, &txth);
			cursor.x = txtw+2;
			cursor.w = txtw+2;
			free(s2);
		}
		if (y <= row) {
			cursor.h += txth;
			cursor.y = cursor.h - txth;
		}
		int nsize = strlen(utf8);
		char *tmp = (char *)calloc(sizeof(char), nsize+1);
		printf("w:%d h:%d x1:%d x2: %d y1: %d y2: %d idx: %d y:%d nsize:%d\n", 
			txtw, txth, cursor.x, cursor.w, cursor.y, cursor.h, idx, y, nsize);
		if (txtw < r.w) {
			memcpy(tmp, utf8, nsize);
		}
		if (utf8) free(utf8);
		buf.push_back(new CLine(tmp));
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
		char *s = buf[i]->buf;
		SDL_Surface *surface = TTF_RenderUTF8_Blended(font, s, color);
		SDL_BlitSurface(surface, NULL, area, &r);
		r.y += surface->h;
		SDL_FreeSurface(surface);
	}
	return area;
}

void CFrameBuffer::newLine() {
	cursor.x = cursor.w = 2;
	column = 0;
	row++;
	int h = cursor.h - cursor.y;
	cursor.y = cursor.h;
	cursor.h +=h;
}

void CFrameBuffer::moveLeft(CStory& story) {
	printf("Column %d\n", column);
	if (column < 1) return;
	int txtw, txth;
	column--;
	if (column == 0) {
		cursor.x = 1;
		cursor.w = 1;
		return;
	}
	char *s2 = story.text[row]->toUtf8(column);
	TTF_SizeUTF8(font, s2, &txtw, &txth);
	cursor.x = txtw+1;
	cursor.w = txtw+1;
	free(s2);
}

void CFrameBuffer::moveRight(CStory& story) {
	int txtw, txth;
	int cnt = story.text[row]->count();
	printf("Count: %d\n", cnt);
	if (column >= cnt) {
		column = cnt;
	} else {
		column++;
	}
	char *s2 = story.text[row]->toUtf8(column);
	TTF_SizeUTF8(font, s2, &txtw, &txth);
	cursor.x = txtw+1;
	cursor.w = txtw+1;
	free(s2);
}

void CFrameBuffer::backspace(CStory& story) {
	if (row >= (int)story.text.size()) return;
	if (story.text[row]->size() <= 0) {
		story.delline(row);
		if (row > 0) row--;
		column = story.text[row]->count();
		return;
	}
	story.text[row]->del(column);
	column--;
}

void CFrameBuffer::estimateLineLength(TTF_Font *ft, SDL_Rect& r) {
	char s[1024];
	memset(s, 0, sizeof(s));
	int idx = 0;
	int poll = true;
	int txtw, txth;
	while (poll) {
		s[idx] = 'A';
		s[idx+1] = '\0';
		TTF_SizeUTF8(font, s, &txtw, &txth);
		if (txtw >= r.w) poll = false;
	}
	estChars = idx;
}

