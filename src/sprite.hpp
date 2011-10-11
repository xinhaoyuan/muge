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
}

#endif
