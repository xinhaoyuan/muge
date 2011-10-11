#include <SDL/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <deque>

#include "logic/event.hpp"
#include "logic/timer.hpp"
#include "io/io.hpp"

#include "sprite.hpp"
#include "map.hpp"

using namespace GameEngine;
using namespace Game;

Uint8 *SDL_keystate;

class SimpleSprite : public Sprite
{
	SDL_Rect     mRect;
	SDL_Surface *mSur;
	int          mFrames;
	
public:

	SimpleSprite(const char *filename, int width, int height,
				 int frames,
				 int keyR, int keyG, int keyB) {
		SDL_Surface *img = SDL_LoadBMP(filename);
		mSur = SDL_DisplayFormat(img);
		SDL_FreeSurface(img);

		SDL_SetColorKey(mSur, SDL_SRCCOLORKEY,
						SDL_MapRGB(mSur->format, keyR, keyG, keyB));

		mFrames = frames;
		
		mRect.x = 0;
		mRect.y = 0;
		mRect.w = width;
		mRect.h = height;
	}

	~SimpleSprite(void) {
		SDL_FreeSurface(mSur);
	}

	void
	Show(tick_t tick, SDL_Surface *screen, SDL_Rect *rect) {
		mRect.x = mRect.w * ((tick >> 3)% mFrames);
		SDL_BlitSurface(mSur, &mRect, screen, rect);
	}
};

class : public Drawable
{
public:

	Map *map;
	TileNode *me;
	int meX, meY;
	tick_t lastTick;
	
	void Draw(tick_t tick, void *scene) {
		SDL_Surface *screen = (SDL_Surface *)scene;
		SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0, 0, 0));

		SDL_Rect rect;

		if (SDL_keystate[SDLK_LEFT] ^ SDL_keystate[SDLK_RIGHT])
			me->mX += (tick - lastTick) * (SDL_keystate[SDLK_RIGHT] ? 1 : -1);

		if (SDL_keystate[SDLK_DOWN] ^ SDL_keystate[SDLK_UP])
			me->mY += (tick - lastTick) * (SDL_keystate[SDLK_DOWN] ? 1 : -1);

		map->UpdateSprite(me);

		rect.x = 0;
		rect.y = 0;
		map->Show(tick, screen, &rect, meX, meY, 640, 480);

		lastTick = tick;
	}
} world;

class : public Event
{
public:

	void Activate(void) {
		SDL_WM_SetCaption("GAME", NULL);
		SDL_keystate = SDL_GetKeyState(NULL);

		world.lastTick = 0;

		MapTiles::LoadResources("data/maptiles");
		world.map = new Map();

		world.map->Load("data/map1");
		world.map->AddSprite(new SimpleSprite("gfx/budan.bmp", 32, 32, 4, 0, 0xff, 0xff), 16, -16, 0, 32, 32, 31, 31);
		world.me = world.map->AddSprite(new SimpleSprite("gfx/dango.bmp", 32, 32, 8, 0, 0xff, 0xff), -16, -16, 0, 32, 32, 31, 31);
		world.meX = -200;
		world.meY = -200;
		
		IO::Open();
	}
} initEvent;

class : public Event
{
public:

	void
	Activate(void) {
		delete world.me;
		delete world.map;
		
		IO::Quit();
		EventLoop::sMain.Stop();
	}
} quitEvent;

void
keyboardHandler(int down, SDL_keysym sym)
{ }

int
main(void)
{
	IO::SetKeyboardHandler(keyboardHandler);
	IO::SetScreen(640, 480, 32, false, 60, &world);
	IO::Initialize(&initEvent, &quitEvent);
	EventLoop::sMain.Start();
}
