#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "buffer.h"

#define DEFAULT_BLOCK_SIZE 1024

CParagraph::CParagraph(int seq) {
	buf = (char *)calloc(sizeof(char), DEFAULT_BLOCK_SIZE);
	len = 0;
	alloc = DEFAULT_BLOCK_SIZE;
	tag = seq;
}

CParagraph::~CParagraph() {
	if (buf) free(buf);
	buf = NULL;
}

void CParagraph::checkBuffer(size_t bytesRequired) {
	size_t newsize = alloc;
	while (bytesRequired >= newsize) newsize += DEFAULT_BLOCK_SIZE;
	if (bytesRequired > alloc) {
		alloc = newsize;
		char *tmp = (char *)realloc(buf, newsize);
		buf = tmp;
	}
}

void CParagraph::cat(char *s, int n) {
	if (n < 1) n = strlen(s);
	size_t size = strlen(s) + len;
	checkBuffer(size);
	strncat(buf, s, n);
	len += n;
}

int CParagraph::ins(char *s, int pos) {
	if (!s) return pos;
	if (!*s) return pos;
	if (pos >= (int)len) {
		return append(s);
	}
	size_t word = strlen(s);
	size_t size = word + len;
	checkBuffer(size);
	size_t nbytes= strlen(buf+pos);
	memmove(buf+pos+word, buf+pos, nbytes);
	memcpy(buf+pos, s, word);
	len = size;
	*(buf+len) = '\0';
	return (pos + word);
}

int CParagraph::append(char *str) {
	cat(str);
	return strlen(buf);
}

int CParagraph::del(int bytesToSkip, int column) {
	if (!buf) return 0;
	if (column < 1) return 0;
	column--;
	iterator it = begin();
	it.seek(bytesToSkip);
	int bytes = 0;
	while (char *s = it.next()) {
		if (column < 1) {
			bytes = strlen(s);
			char *src = it.peek();
			char *dest = src - bytes;
			memmove(dest, src, bytes);
		}
		if (column > 0) column--;
	}
	len = strlen(buf);
	return bytes;
}

void CParagraph::clear() {
	memset(buf, 0, len);
	len = 0;
}

CStory::CStory() {
	CParagraph *tmp = new CParagraph(1);
	text.push_back(tmp);
	top = 0;
	paragraph_seq = 2;
	editPt.line = 0;
	editPt.byte = 0;
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
	editPt.line = text.size() - 1;
	editPt.byte = p->append(s);
	StringU8 line(s);
	return line.strLen();
}

void CStory::newline() {
	CParagraph *tmp = new CParagraph(paragraph_seq++);
	text.push_back(tmp);
	editPt.line = text.size() - 1;
	editPt.byte = 0;
}

void CStory::delline(int idx) {
	printf("Delete line: %d\n", idx);
	if (text.size() < 2) {
		// Only game in town. DO not remove.
		text[idx]->clear();
		editPt.line = 0;
		editPt.byte = 0;
		return; 
	}
	if (idx > 0) {
		editPt.line = idx - 1;
		editPt.byte = text[editPt.line]->size();
	} else if (idx == 0) {
		editPt.line = 0;
		editPt.byte = text[1]->size();
	}
	CParagraph *p = text[idx];
	text.erase(text.begin() + idx);
	delete p;
}

void CStory::del(CLine *p, int col) {
	int lineno = p->lineno;
	printf("Size: %d\n", text[lineno]->size());
	if (text[lineno]->size() <= 0) {
		delline(lineno);
		return;
	}
	int bytes = text[lineno]->del(p->segbegin, col);
	if (editPt.byte > 0) editPt.byte -= bytes;
	printf("editPt line: %d byte: %d\n", editPt.line, editPt.byte);
}

void CStory::ins(char *txt) {
	int lineno = editPt.line;
	editPt.byte = text[lineno]->ins(txt, editPt.byte);
}

void CStory::moveLeft(CLine *ln) {
	CParagraph *p = text[ln->lineno];
	if (editPt.byte <= ln->segbegin) return; // Top of line. Return
	CParagraph::iterator it = p->begin();
	it.seek(ln->segbegin);
	int nbytes = 0;
	while (char *s = it.next()) {
		size_t l = strlen(s);
		if ((int)(nbytes + l) >= editPt.byte) {
			editPt.byte = nbytes;
			break;
		}
		nbytes += l;
	}

}

void CStory::moveRight(CLine *ln) {
	CParagraph *p = text[ln->lineno];
	CParagraph::iterator it = p->begin();
	it.seek(editPt.byte);
	char *s = it.next();
	if (s) editPt.byte += strlen(s);
}

CLine::CLine() {
	buf = NULL;
	len = segbegin = seglen = 0;
	width = height = 0;
	lineno = 0;
	empty = false;
}
CLine::CLine(char *s) { 
	buf = s; 
	lineno = segbegin = seglen = 0; 
	if (s) len = strlen(s);
}

CLine::~CLine() {
	if ((buf) && (!empty)) free(buf);
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
		SDL_Surface *surface;
		if (buf[i]->empty) {
			surface = TTF_RenderUTF8_Blended(font, " ", color);
		} else {
			surface = TTF_RenderUTF8_Blended(font, s, color);
		}
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
	printf("ROw:%d\n", row);
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
	story.moveLeft(buf[row]);
	if (column == 0) {
		cursor.x = SPACING;
		cursor.w = SPACING;
		return;
	}
	horizontalMove();
}

void CFrameBuffer::moveRight(CStory& story) {
	int cnt = StringU8(buf[row]->buf).strLen();
	if (column >= cnt) {
		column = cnt;
	} else {
		column++;
		story.moveRight(buf[row]);
	}
	horizontalMove();
}

void CFrameBuffer::backspace(CStory& story) {
	if (row >= (int)buf.size()) return;
	story.del(buf[row], column);
}

void CFrameBuffer::insert(CStory& story, char *text) {
	story.ins(text);
//	story.append(text);
}
