#ifndef __SPRITE_HPP__
#define __SPRITE_HPP__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "logic/object.hpp"

namespace Game
{
	class Sprite
	{	
	public:
		virtual void Show(int state, GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect) = 0;
	};

	class SimpleSprite : public Sprite
	{
		SDL_Rect     mRect;
		SDL_Surface *mSur;

		int  mStateCount;
		int *mFrameCount;
		int *mOffsetX;
		int *mOffsetY;
		
	public:

		int mWidth;
		int mHeight;
		
		static SimpleSprite *Load(const char *name);
		~SimpleSprite(void);

		virtual void Show(int state, GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect);
	};

}

#endif
