#include "buffer.h"

/**
 * Recalculate the cursor position on the view port
 * given the current edit position in the text.
 */
void CFrameBuffer::calibrate(CStory& story) {
	std::vector<CLine *>::iterator it;
	int height = 0;
	row = 0;
	for (it = buf.begin(); it != buf.end(); it++) {
		CLine *obj = *it;
		if (obj->lineno == story.getCurrentLine()) {
			cursor.y = height;
			cursor.h = height + obj->height;
			if (obj->empty) {
				column = 0;
				cursor.x = SPACING;
				cursor.w = SPACING;
				break;
			}
			int idx = story.getCurrentIndex();
			if ((idx >= obj->segbegin) && (idx <= (obj->seglen + obj->segbegin))) {
				column = obj->seglen;
				char *s= strdup(obj->buf);
				*(s+idx) = '\0';
				int txtw, txth;
				TTF_SizeUTF8(font, s, &txtw, &txth);
				column = StringU8(s).strLen();
				cursor.x = txtw;
				free(s);
				cursor.w = cursor.x;
				break;
			}
		}
		height += obj->height;
		row++;
	}
}

// Figure out the range of text fitted into the viewing rect
void CFrameBuffer::prepare(TTF_Font *ft, SDL_Rect& r, CStory& story) {
	int txtw = 0, txth = 0;
	freeBuffer();
	font = ft;
	// Save the view rectangle for later use
	rect = r;
	rect.y = 0;
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
			dup = (char *)" ";
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
			strcat(dup, s);
			TTF_SizeUTF8(font, dup, &txtw, &txth);
//			printf("txtw: %d r.h:%d\n", txtw, r.w);
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
				if (height >= r.h) cont = false;
				if (byteRemain > 0) {
					dup = (char *)calloc(sizeof(char), byteRemain+4);
					strcpy(dup, s);
				}
				else dup = NULL;
			}
			seglen += byteUsed;
			byteRemain -= byteUsed;
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
		printf("w:%d h:%d x1:%d x2: %d y1: %d y2: %d y:%d lines:%ld\n", 
			txtw, txth, cursor.x, cursor.w, cursor.y, cursor.h, y, buf.size());
	}
	calibrate(story);
}
