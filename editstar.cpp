#include "editstar.h"

CEditStar::CEditStar() {
}

CEditStar::~CEditStar() {
}

void CEditStar::onKeyDown(SDL_Event& event) {
	switch (event.key.keysym.sym) {
		case SDLK_F5:
			break;
		case SDLK_RETURN:
		case SDLK_RETURN2:
			story.newline();
			frameBuffer.column = 0;
			frameBuffer.row++;
			break;
		default:
			break;
	}
	printf("scancode %d\n", event.key.keysym.scancode);
}

void CEditStar::render() {
	SDL_Color color = {255, 255, 255 };
	SDL_Texture *texture = NULL;
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
	SDL_SetRenderDrawColor(renderer, 255, 255,255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(renderer, frameBuffer.cursor.x, frameBuffer.cursor.y,
		frameBuffer.cursor.w, frameBuffer.cursor.h);
	SDL_RenderPresent(renderer);
	SDL_DestroyTexture(texture);
}

void CEditStar::onTextInput(SDL_Event& event) {
	int chars = story.append(event.text.text);
	frameBuffer.column += chars;
	render();
}
