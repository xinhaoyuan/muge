#include <fstream>
#include <string>

#include "sprite.hpp"

namespace Game
{
	SimpleSprite*
	SimpleSprite::Load(const char *name)
	{
		int uwidth, uheight;
		int states;
		int keyR, keyG, keyB;
			
		std::ifstream fin(name);
			
		fin >> uwidth >> uheight;
		fin >> states;
		fin >> keyR >> keyG >> keyB;
			
		std::string filename;
			
		fin >> filename;
			
		SimpleSprite *result = new SimpleSprite;
		
		SDL_Surface *img = SDL_LoadBMP(filename.c_str());
		result->mSur = SDL_DisplayFormat(img);
		SDL_FreeSurface(img);

		SDL_SetColorKey(result->mSur, SDL_SRCCOLORKEY,
						SDL_MapRGB(result->mSur->format, keyR, keyG, keyB));

		result->mFrameCount = new int[states];
		result->mOffsetX = new int[states];
		result->mOffsetY = new int[states];

		int i;
		for (i = 0; i < states; ++ i)
		{
			fin >> result->mFrameCount[i] >> result->mOffsetX[i] >> result->mOffsetY[i];
		}

		result->mStateCount = states;
		
		result->mRect.w = uwidth;
		result->mRect.h = uheight;

		return result;
	}

	SimpleSprite::~SimpleSprite(void) {
		delete mFrameCount;
		delete mOffsetX;
		delete mOffsetY;
		SDL_FreeSurface(mSur);
	}
	
	void
	SimpleSprite::Show(int state, GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect)
	{
		if (state < 0 || state >= mStateCount) return;
		
		mRect.x = mOffsetX[state] + mRect.w * ((tick >> 3)% mFrameCount[state]);
		mRect.y = mOffsetY[state];
		SDL_BlitSurface(mSur, &mRect, screen, rect);
	}

}
