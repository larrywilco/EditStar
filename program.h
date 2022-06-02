#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <SDL.h>
#include <SDL_ttf.h>

class CProgram {
	private:
		void findFont();
		void init();
		
	protected:
		char *fontPath;
		char *fontFamily;
		int fontSize;
		int winWidth;
		int winHeight;
		SDL_Window *window;
		SDL_Renderer *renderer;
		TTF_Font *font;
		
		virtual void onKeyDown(SDL_Event& event) = 0;
		virtual void onTextInput(SDL_Event& event) = 0;
	public:
		CProgram();
		~CProgram();
		void run();
};

#endif
