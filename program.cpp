#include "program.h"
#include <SDL_ttf.h>
#include <stdio.h>
#include <fontconfig/fontconfig.h>

CProgram::CProgram() {
	fontPath = NULL;
//	fontFamily = strdup("Glass TTY VT220");
	fontFamily = strdup("Px IBM VGA9");
//	fontFamily = strdup("Mono");
	fontSize = 32;
	winWidth = 640;
	winHeight = 480;
	eventTimeout = 1000; // 1 second
	font = NULL;
//	bgcolor = {0x0, 0x0, 0x00, SDL_ALPHA_OPAQUE};
	bgcolor = {40, 40, 40, SDL_ALPHA_OPAQUE};
	fgcolor = {0xFF, 0xB0, 0x0, SDL_ALPHA_OPAQUE};
	init();
}

CProgram::~CProgram() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
	
	if (fontPath) free(fontPath);
	fontPath = NULL;
	if (fontFamily) free(fontFamily);
	fontFamily = NULL;
}

void CProgram::init() {
	Uint32 render_flags = SDL_RENDERER_ACCELERATED;

	findFont();
	SDL_Init(SDL_INIT_VIDEO);
	if (TTF_Init() == -1) {
		printf("TTF init failed\n");
		return ;
	}
	window = SDL_CreateWindow("Untitled", SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED, winWidth, winHeight, 0);
	renderer = SDL_CreateRenderer(window, -1, render_flags);
	view.x = 1;
	view.y = 1;
	view.w = winWidth - 2;
	view.h = winHeight - 2;
}

void CProgram::findFont() {
	FcInit();
	FcConfig *conf = FcConfigGetCurrent();
	
	// Load a default font first
	FcPattern *pat = FcNameParse((const FcChar8*)fontFamily);
//	FcPattern *pat = FcPatternBuild(0, FC_LANG, FcTypeString, "zh-cn", (char *)0);
	if (pat == NULL) {
		printf("Error creating pattern\n");
	}
	FcConfigSubstitute(0, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
	FcResult rc;
	FcPattern *font = FcFontMatch(0, pat, &rc);
	if (font) {
		FcChar8 *path = NULL;
		FcPatternGetString(font, FC_FILE, 0, &path);
		printf("%s\n", path);
		fontPath = strdup((char *)path);
	}
	FcPatternDestroy(font);
	FcPatternDestroy(pat);
	FcConfigDestroy(conf);
	FcFini();
}

void CProgram::setFont(char *pattern) {
	if (!pattern) return;
	if (font) TTF_CloseFont(font);
	if (fontFamily) free(fontFamily);
	fontFamily = strdup(pattern);
	findFont();
	font = TTF_OpenFont(fontPath, fontSize);
}

void CProgram::run() {
	bool cont = true;
	SDL_Event event;
	
	font = TTF_OpenFont(fontPath, fontSize);
	if (!font) {
		printf("Open font error\n");
		return;
	}
	SDL_StartTextInput();
	while (cont) {
		SDL_WaitEventTimeout(&event, eventTimeout);
		switch (event.type) {
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN: // capture special key
				onKeyDown(event);
				break;
			case SDL_TEXTINPUT: // Actual text
				onTextInput(event);
				break;
			case SDL_TEXTEDITING: // During character composition
//				printf("Edit %s\n", event.text.text);
				break;
		}
		onIdle();
	}
	SDL_StopTextInput();
	if (font) TTF_CloseFont(font);
}

void CProgram::show() {
	SDL_RenderPresent(renderer);
}

void CProgram::setColor(bool foreground) {
	if (foreground) {
		SDL_SetRenderDrawColor(renderer, fgcolor.r, fgcolor.g, fgcolor.b, 
				fgcolor.a);
	} else {
		SDL_SetRenderDrawColor(renderer, bgcolor.r, bgcolor.g, bgcolor.b, 
				SDL_ALPHA_OPAQUE);
	}
}

void CProgram::setColor(SDL_Color& color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void CProgram::clear() {
	SDL_SetRenderDrawColor(renderer, bgcolor.r, bgcolor.g, bgcolor.b, 
				SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
}

void CProgram::clearAndRender(SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dest) {
	clear();
	SDL_RenderCopy(renderer, texture, src, dest);
}
