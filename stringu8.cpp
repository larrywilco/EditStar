#include <string.h>
#include <stdio.h>
#include "stringu8.h"

int StringU8::byteCount(char s) {
	int nbytes = 1;
	if ((s & 0xF0) == 0xF0) {
		nbytes = 4;
	} else if ((s & 0xE0) == 0xE0) {
		nbytes = 3;
	} else if ((s & 0xC0) == 0xC0) {
		nbytes = 2;
	}
	return nbytes;
}

int StringU8::strLen() {
	int limit = ::strlen(buf);
	char *s = buf;
	int cnt=0;
	for (int i=0; i < limit; cnt++) {
		int bytes = byteCount(*s);
		i += bytes;
		s += bytes;
	}
	return cnt;
}

StringU8& StringU8::operator= (const char *s) {
	buf = (char *)s;
	return *this;
}

char * StringU8::substr(int start, int nchar, char *dest, int maxBytes) {
	if (!buf) return NULL;
	if (nchar < 1) return NULL;
	int limit = ::strlen(buf);
	if (nchar > limit) return NULL;
	char *s = buf;
	char *d = dest;
	int cnt = 0;
	for (int i=0; i < limit; cnt++) {
		int bytes = byteCount(*s);
		if ((cnt >= start) && (nchar > 0)) {
			maxBytes -= bytes;
			if (maxBytes < 0) break;
			memcpy(d, s, bytes);
			d += bytes;
			nchar--;
		}
		s += bytes;
		i += bytes;
	}
	*d = '\0';
	return dest;
}

StringU8::iterator::iterator(StringU8 *o) {
	parent = o;
}

void StringU8::iterator::begin(int pos) {
	ptr = parent->buf;
	if (pos < 1) return;
	if (ptr == NULL) return;
	if (*ptr == '\0') return;
	int limit = ::strlen(ptr);
	for (int i=0; (i < limit) && (pos); pos--) {
		int bytes = parent->byteCount(*ptr);
		i += bytes;
		ptr += bytes;
	}
}

char *StringU8::iterator::next() {
	memset(token, 0, sizeof(token));
	if (ptr == NULL) return NULL;
	if (*ptr == '\0') return NULL;
	int bytes = parent->byteCount(*ptr);
	if (bytes > 4) {
		// Error in the string or programming error
		ptr += bytes;
		return NULL;
	}
	memcpy(token, ptr, bytes);
	ptr += bytes;
	return token;
}

StringU8::iterator StringU8::begin(int pos) {
	StringU8::iterator it(this);
	it.begin(pos);
	return it;
}
