#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "buffer.h"

#define DEFAULT_BLOCK_SIZE 1024
#define SPACING 1

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
	if (n < 1) n = strlen(s);
	size_t size = strlen(s) + len;
	size_t newsize = alloc;
	while (size >= newsize) newsize += DEFAULT_BLOCK_SIZE;
	if (size > alloc) {
		alloc = newsize;
		char *tmp = (char *)realloc(buf, newsize);
		buf = tmp;
	}
	strncat(buf, s, n);
	len += n;
}

int CParagraph::append(char *str) {
	cat(str);
	StringU8 tmp(str);
	return tmp.strLen();
}

void CParagraph::del(int column) {
	if (!buf) return;
	if (column < 1) return;
	int limit = ::strlen(buf);
	char *src = buf, *dest = buf;
	int cnt=1;
	src = buf; dest = buf;
	for (int i=0; i < limit; cnt++) {
		int bytes = byteCount(*dest);
		i += bytes;
		if (cnt < column) {
			dest += bytes;
		} else if (cnt == column) {
			src = dest + bytes;
			memmove(dest, src, bytes);
		}
	}
	len = strlen(buf);
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
	int chars = p->append(s);
	right++;
	return chars;
}

void CStory::newline() {
	CParagraph *tmp = new CParagraph();
	text.push_back(tmp);
//	bottom++;
}

void CStory::delline(int idx) {
	CParagraph *p = text[idx];
	text.erase(text.begin() + idx);
	delete p;
//	bottom--;
}

CLine::CLine() {
	buf = NULL;
	len = segbegin = seglen = 0; 
}
CLine::CLine(char *s) { 
	buf = s; 
	lineno = segbegin = seglen = 0; 
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
	int txtw = 0, txth = 0;
	freeBuffer();
	font = ft;
	rect = r;
	rect.y = 0;
	cursor.h = 0;
	int length = story.text.size();
	int height = 0;
	bool good = true;
	for (int y = story.top; (y<length) && good; y++) {
	bool emptyLine = false;
		int startpos = 0;
		int seglen = 0;
		CParagraph *p = story.text[y];
		char *utf8 = p->get();
		int byteRemain = p->size();
		char *dup = NULL;
		if ((!utf8) || (*utf8 == '\0')) { // Take care of blank line
			emptyLine = true;
			dup = strdup(" ");
		} else {
			emptyLine = false;
			dup = (char *)calloc(sizeof(char), byteRemain+2);
		}
		CParagraph::iterator it = p->begin();
		bool cont = true;
		char *s;
		while ((s = it.next()) && cont) {
			int preserve = strlen(dup);
			int byteUsed = strlen(s);
			byteRemain -= byteUsed;
			seglen += byteUsed;
			strcat(dup, s);
			TTF_SizeUTF8(font, dup, &txtw, &txth);
			printf("Row:%d y:%d\n", row, y);
			if (txtw >= r.w) {
				*(dup + preserve) = '\0';
				height += txth;
				if (row == y) {
					cursor.x = txtw+SPACING;
					cursor.w = txtw+SPACING;
				}
				if (y <= row) {
					cursor.h += txth;
					cursor.y = cursor.h - txth;
				}
				CLine *ln = new CLine(dup);
				ln->lineno = y;
				ln->segbegin = startpos;
				ln->seglen = seglen;
				buf.push_back(ln);
				startpos += seglen;
				seglen = 0;
				y++;
				if (height >= r.h) cont = false;
				if (byteRemain > 0)
					dup = (char *)calloc(sizeof(char), byteRemain+4);
				else dup = NULL;
			}
		}
		if ((dup) && (*dup)) {
			TTF_SizeUTF8(font, dup, &txtw, &txth);
			height += txth;
			if (row == y) {
				if (emptyLine) {
					cursor.x = SPACING;
					cursor.w = SPACING;
				} else {
					cursor.x = txtw+SPACING;
					cursor.w = txtw+SPACING;
				}
			}
			CLine *ln = new CLine(dup);
			ln->lineno = y;
			ln->segbegin = startpos;
			ln->seglen = seglen;
			buf.push_back(ln);
			startpos += seglen;
			seglen = 0;
			if (y <= row) {
				cursor.h += txth;
				cursor.y = cursor.h - txth;
			}
		}
		if (height >= r.h) {
			good = false;
			continue;
		}
		printf("w:%d h:%d x1:%d x2: %d y1: %d y2: %d row: %d y:%d \n", 
			txtw, txth, cursor.x, cursor.w, cursor.y, cursor.h, row, y);
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
	cursor.x = cursor.w = SPACING;
	column = 0;
	row++;
	int h = cursor.h - cursor.y;
	cursor.y = cursor.h;
	cursor.h +=h;
}

void CFrameBuffer::horizontalMove() {
	if (column < 1) return;
	int txtw, txth;
	StringU8 s(buf[row]->buf);
	char *s2 = strdup((char *)s);
	s.substr(0, column, s2, strlen(s));
	TTF_SizeUTF8(font, s2, &txtw, &txth);
	cursor.x = txtw+SPACING;
	cursor.w = txtw+SPACING;
	free(s2);
}

void CFrameBuffer::moveLeft(CStory& story) {
	if (column < 1) return;
	column--;
	if (column == 0) {
		cursor.x = SPACING;
		cursor.w = SPACING;
		return;
	}
	horizontalMove();
}

void CFrameBuffer::moveRight(CStory& story) {
	int cnt = story.text[row]->strLen();
	if (column >= cnt) {
		column = cnt;
	} else {
		column++;
	}
	horizontalMove();
}

void CFrameBuffer::backspace(CStory& story) {
	if (row >= (int)buf.size()) return;
	int lineno = buf[row]->lineno;
	if (story.text[lineno]->size() <= 0) {
		story.delline(lineno);
		if (row > 0) row--;
		column = story.text[lineno]->strLen();
		return;
	}
	story.text[lineno]->del(column);
	if (column > 0) column--;
}

void CFrameBuffer::insert(CStory& story, char *text) {
	int chars = story.append(text);
	column += chars;
}
