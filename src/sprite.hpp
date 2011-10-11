#ifndef __SPRITE_HPP__
#define __SPRITE_HPP__

#include <SDL/SDL.h>

#include "logic/object.hpp"

namespace Game
{
	class Sprite
	{	
	public:
		virtual void Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect) = 0;
	};

	class SimpleSprite : public Sprite
	{
		SDL_Rect     mRect;
		SDL_Surface *mSur;
		int          mFrames;
		
	public:
		
		static SimpleSprite *Load(const char *name);
		~SimpleSprite(void);
		
		virtual void Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect);
	};

}

#endif
