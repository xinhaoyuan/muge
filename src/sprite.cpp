#include <fstream>
#include <string>

#include "sprite.hpp"

namespace Game
{
	SimpleSprite*
	SimpleSprite::Load(const char *name)
	{
		int uwidth, uheight;
		int frames;
		int keyR, keyG, keyB;
			
		std::ifstream fin(name);
			
		fin >> uwidth >> uheight;
		fin >> frames;
		fin >> keyR >> keyG >> keyB;
			
		std::string filename;
			
		fin >> filename;
			
		SimpleSprite *result = new SimpleSprite;
		
		SDL_Surface *img = SDL_LoadBMP(filename.c_str());
		result->mSur = SDL_DisplayFormat(img);
		SDL_FreeSurface(img);

		SDL_SetColorKey(result->mSur, SDL_SRCCOLORKEY,
						SDL_MapRGB(result->mSur->format, keyR, keyG, keyB));

		result->mFrames = frames;
		
		result->mRect.x = 0;
		result->mRect.y = 0;
		result->mRect.w = uwidth;
		result->mRect.h = uheight;

		return result;
	}

	SimpleSprite::~SimpleSprite(void) {
		SDL_FreeSurface(mSur);
	}

	void
	SimpleSprite::Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect)
	{
		mRect.x = mRect.w * ((tick >> 3)% mFrames);
		SDL_BlitSurface(mSur, &mRect, screen, rect);
	}

}
