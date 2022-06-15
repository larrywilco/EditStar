#include "buffer.h"

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
	row = -1; // Recount row based on cursor in the story
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
			printf("txtw: %d r.h:%d\n", txtw, r.w);
			if (txtw >= r.w) {
				*(dup + preserve) = '\0';
				height += txth;
				CLine *ln = new CLine(dup);
				ln->lineno = y;
				ln->segbegin = startpos;
				ln->seglen = seglen;
				ln->width = txtw;
				ln->height = txth;
				ln->empty = emptyLine;
				ln->paragraphId = p->id();
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
			CLine *ln = new CLine(dup);
			ln->lineno = y;
			ln->segbegin = startpos;
			ln->seglen = seglen;
			ln->width = txtw;
			ln->height = txth;
			ln->empty = emptyLine;
			ln->paragraphId = p->id();
			buf.push_back(ln);
			startpos += seglen;
			seglen = 0;
		}
		if (height >= r.h) {
			good = false;
			continue;
		}
		printf("w:%d h:%d x1:%d x2: %d y1: %d y2: %d row: %d y:%d \n", 
			txtw, txth, cursor.x, cursor.w, cursor.y, cursor.h, row, y);
	}
	std::vector<CLine *>::iterator it;
	int cnt = 0;
	for (it = buf.begin(); it != buf.end(); ++it) {
		CLine *obj = *it;
		if (obj->paragraphId == story.getLineId()) {
			int idx = story.getCurrentChar();			
			if ((idx > obj->segbegin) && (idx <= obj->seglen)) {
				row = cnt;
				column = obj->seglen - obj->segbegin;
				break;
			}
		}
		cnt++;
	}
}