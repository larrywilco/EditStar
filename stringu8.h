#ifndef _STRINGU8_H
#define _STRINGU8_H

/**
 * Simple UTF-8 string encapsulation. Allow characters counting
 * instead of just byte counting.
 * strlen() method returns characters count and not bytes count.
 * No memory allocation and management functionalities.
 */
class StringU8 {
	private:
		int byteCount(char s);
	
	protected:
		char *buf;
			
	public:
		class iterator {
			private:
				char token[5];
				StringU8 *parent;
				char *ptr;
			public:
				iterator(StringU8 *obj);
				void begin(int pos = 0);
				char *next();
				char *peek() { return ptr; };
		};
		StringU8() { buf = NULL; };
		StringU8(const char *s) { buf = (char *)s; };
		
		// Getting and setting string. No checking perform.
		char *get() { return buf; }
		void set(char *s) { buf = s; }
		operator char*() const { return buf; };
		StringU8& operator= (const char *s);
		
		int strLen(); // # of utf8 character in string
		char *substr(int start, int nchar, char *dest, int maxBytes);
		StringU8::iterator begin(int pos=0);
};

#endif
