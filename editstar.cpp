#include "editstar.h"

CEditStar::CEditStar() {
	blinker = false;
}

CEditStar::~CEditStar() {
}

void CEditStar::onKeyReturn() {
	story.newline();
	frameBuffer.newLine();
	render();
}

void CEditStar::onKeyLeft() {
	eraseCursor();
	frameBuffer.moveLeft(story);
	drawCursor();
	show();
}

void CEditStar::onKeyRight() {
	eraseCursor();
	frameBuffer.moveRight(story);
	drawCursor();
	show();
}

void CEditStar::onBackSpace() {
	frameBuffer.backspace(story);
	printf("x: %d w: %d\n", frameBuffer.cursor.x, frameBuffer.cursor.w);
	render();
}

void CEditStar::onDelete() {
}

void CEditStar::onKeyDown(SDL_Event& event) {
	switch (event.key.keysym.sym) {
		case SDLK_F5:
			break;
		case SDLK_RETURN:
		case SDLK_RETURN2:
			onKeyReturn();
			break;
		case SDLK_LEFT:
			onKeyLeft();
			break;
		case SDLK_RIGHT:
			onKeyRight();
			break;
		case SDLK_BACKSPACE:
			onBackSpace();
			break;
		case SDLK_DELETE:
			onDelete();
			break;
		default:
			break;
	}
	printf("scancode %d\n", event.key.keysym.scancode);
}

void CEditStar::eraseCursor() {
	setColor(false);
	SDL_RenderDrawLine(renderer, frameBuffer.cursor.x, frameBuffer.cursor.y,
		frameBuffer.cursor.w, frameBuffer.cursor.h);	
}

void CEditStar::drawCursor() {
	setColor();
	SDL_RenderDrawLine(renderer, frameBuffer.cursor.x, frameBuffer.cursor.y,
		frameBuffer.cursor.w, frameBuffer.cursor.h);	
}

void CEditStar::render() {
	SDL_Texture *texture = NULL;
	frameBuffer.prepare(font, view, story);
	SDL_Surface *area  = frameBuffer.render(fgcolor);

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
	clearAndRender(texture, &srcRect, &rect);
	drawCursor();
	show();
	SDL_DestroyTexture(texture);
}

void CEditStar::onTextInput(SDL_Event& event) {
	frameBuffer.insert(story, event.text.text);
	render();
}

void CEditStar::onIdle() {
	if (blinker) {
		drawCursor();
		blinker = false;
	} else {
		eraseCursor();
		blinker = true;
	}
	show();
}
