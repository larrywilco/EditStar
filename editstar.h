#ifndef EDITSTAR_H
#define EDITSTAR_h

#include "program.h"
#include "buffer.h"

class CEditStar : public CProgram {
	private:
		CStory story;
		int cx, cy;

	protected:
		virtual void onKeyDown(SDL_Event& event);
		virtual void onTextInput(SDL_Event& event);
};

#endif
