#ifndef BUFFER_H
#define BUFFER_H

class CBuffer {
	private:
		char *buf;
		size_t len;
		size_t alloc;
		
	public:
		CBuffer();
		~CBuffer();
		
		void cat(char *s);
		char *get() { return buf; }
		int getLength() { return len; }
};

#endif
