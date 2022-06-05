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
		SDL_Rect view;
		TTF_Font *font;
		SDL_Color bgcolor;
		SDL_Color fgcolor;
		virtual void onKeyDown(SDL_Event& event) = 0;
		virtual void onTextInput(SDL_Event& event) = 0;
		void show();
		void clear();
		void clearAndRender(SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dest);
		void setColor(bool foreground = true);
		void setColor(SDL_Color& color);
		
	public:
		CProgram();
		~CProgram();
		void run();
};

#endif
