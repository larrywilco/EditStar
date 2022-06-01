#include "program.h"
#include <SDL_ttf.h>
#include <stdio.h>
#include <fontconfig/fontconfig.h>

CProgram::CProgram() {
	fontPath = NULL;
	fontFamily = strdup("Mono");
	fontSize = 12;
	winWidth = 640;
	winHeight = 480;
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

void CProgram::run() {
	bool cont = true;
	SDL_Event event;
	
	SDL_StartTextInput();
	while (cont) {
		SDL_WaitEvent(&event);
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
				printf("Edit %s\n", event.text.text);
				break;
		}
	}
	SDL_StopTextInput();
}
