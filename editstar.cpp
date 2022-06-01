#include "editstar.h"

void CEditStar::onKeyDown(SDL_Event& event) {
	if (event.key.keysym.sym == SDLK_F5) {
//		listallfont();
	}
	printf("%d\n", event.key.keysym.scancode);
}

void CEditStar::onTextInput(SDL_Event& event) {
	int w, h;
	char *s;
	
	beginRender();
	raw.cat(event.text.text);
	char s = raw.get();
	TTF_SizeUTF8(font, s, &w, &h);
	printf("Text length: %d\n", w);
	endRender();
//	render(renderer);
//	printf("Input %s\n", event.text.text);
}

void CEditStar::beginRender() {
	font = TTF_OpenFont(fontPath, fontSize);
	if (!font) {
		printf("Open font error\n");
		return;
	}
}

void CEditStar::endRender() {
	TTF_CloseFont(font);
}
