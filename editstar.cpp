#include "editstar.h"
#include <iconv.h>
#include <errno.h>

CEditStar::CEditStar() {
	unicode = (char *)calloc(sizeof(char), 1024);
}

CEditStar::~CEditStar() {
	if (unicode) free(unicode);
	unicode = NULL;
}

void CEditStar::onKeyDown(SDL_Event& event) {
	switch (event.key.keysym.sym) {
		case SDLK_F5:
			break;
		case SDLK_RETURN:
		case SDLK_RETURN2:
			story.newline();
			break;
		default:
			break;
	}
	printf("%d\n", event.key.keysym.scancode);
}

void CEditStar::onTextInput(SDL_Event& event) {
	size_t inbytes, outbytes = 1024;
	SDL_Color color = {255, 255, 255 };
	SDL_Texture *texture = NULL;
	iconv_t cd = iconv_open("UTF8", "UTF16");
	char *dup = strdup(event.text.text); //strdup("Abc");
	char *s = dup;
	char *d = unicode;
	inbytes = strlen(s);
	size_t rc = iconv(cd, &s, &inbytes, &d, &outbytes);
	printf("%d %s %s\n", errno, strerror(errno), dup);
	iconv_close(cd);
	story.append(dup, strlen(dup)); //unicode, 1024 - outbytes);
	printf ("%s Len: %ld %ld %ld\n", dup, inbytes, outbytes, rc);
	free(dup);
	frameBuffer.prepare(font, view, story);
	SDL_Surface *area  = frameBuffer.render(color);

	SDL_Rect rect = {0, 0, 0, 0};
	texture = SDL_CreateTextureFromSurface(renderer, area);
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	SDL_Rect srcRect = rect;
	rect.x = 1;
	rect.y = 1;
	// Horizontal scrolling
	if (rect.w >= view.w) {
		srcRect.x = srcRect.w - view.w;
		rect.w = view.w;
	}

	SDL_FreeSurface(area);
	SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0x0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, &srcRect, &rect);
	SDL_RenderPresent(renderer);
	SDL_DestroyTexture(texture);
}
