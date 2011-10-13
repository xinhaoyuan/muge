#ifndef __CONV_HPP__
#define __CONV_HPP__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <vector>

#include "logic/object.hpp"

namespace Game
{
	class Font
	{
		TTF_Font *mFont;
	public:

		static Font *Load(const char *name);

		void         SetStyle(int style);
		void         UnicodeTextSize(const char16_t *text, int *w, int *h);
		SDL_Surface *UnicodeTextRender(const char16_t *text, SDL_Color color);
		int          UnicodeTextMaxAdvance(const char16_t *text, int w);
	};

	class Conversation
	{
		std::vector<SDL_Surface *> mPages;
		int mPage;
	public:
		
		int mHeight;
		int mWidth;

		static Conversation *Load(const char *name);
		bool SetPage(int page);
		void Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect);
	};
}

#endif
