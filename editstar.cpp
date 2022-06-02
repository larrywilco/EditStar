#include "editstar.h"

void CEditStar::onKeyDown(SDL_Event& event) {
	if (event.key.keysym.sym == SDLK_F5) {
//		listallfont();
	}
	printf("%d\n", event.key.keysym.scancode);
}

void CEditStar::onTextInput(SDL_Event& event) {
	int w, h;
	std::vector<SDL_Surface *>frame;
	SDL_Color color = {255, 255, 255 };
	SDL_Texture *texture = NULL;
	
	story.append(event.text.text);
	TTF_SizeUTF8(font, event.text.text, &w, &h);
	printf("Text length: %d\n", w);
	SDL_GetRendererOutputSize(renderer, &w, &h);
	SDL_Surface *i = story.toSurface(0, font, color);
	frame.push_back(i);
	SDL_Surface *area = SDL_CreateRGBSurfaceWithFormat(0, 
			w, h, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Rect rect = {0, 0, 0, 0};
	std::vector<SDL_Surface *>::iterator it;
	for(it=frame.begin(); it != frame.end(); it++) {
		SDL_BlitSurface(*it, NULL, area, &rect);
		printf("%d\n", rect.y);
		rect.y += (*it)->h;
	}

	while(!frame.empty()) {
		SDL_Surface *p = frame.back();
		SDL_FreeSurface(p);
		frame.pop_back();
	}
	rect = {0, 0, 0, 0};
	texture = SDL_CreateTextureFromSurface(renderer, area);
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	SDL_Rect srcRect = rect;
//	rect.w /= 6;
//	rect.h /= 6;
	rect.x = 1;
	rect.y = 1;
	// Horizontal scrolling
	if (rect.w >= w) {
		srcRect.x = srcRect.w - w;
		rect.w = w;
	}

	SDL_FreeSurface(area);
	SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0x0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, &srcRect, &rect);
	SDL_RenderPresent(renderer);
	SDL_DestroyTexture(texture);
}
