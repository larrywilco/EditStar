#ifndef EDITSTAR_H
#define EDITSTAR_h

#include "program.h"
#include "buffer.h"

class CEditStar : public CProgram {
	private:
		CStory story;
		CFrameBuffer frameBuffer;
		int cx, cy;
		char *unicode;
		
	protected:
		virtual void onKeyDown(SDL_Event& event);
		virtual void onTextInput(SDL_Event& event);
		
	public:
		CEditStar();
		~CEditStar();
};

#endif
