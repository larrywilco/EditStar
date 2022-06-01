#include "buffer.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_BLOCK_SIZE 4096

CBuffer::CBuffer() {
	buf = (char *)calloc(sizeof(char), DEFAULT_BLOCK_SIZE);
	len = 0;
	alloc = DEFAULT_BLOCK_SIZE;
}

CBuffer::~CBuffer() {
	if (buf) free(buf);
	buf = NULL;
}

void CBuffer::cat(char *s) {
	if ((len + strlen(s)) < alloc) strcat(buf, s);
}
