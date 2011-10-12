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
#include "resource.hpp"
#include "script.hpp"

using namespace GameEngine;
using namespace Game;

Uint8 *SDL_keystate;

class Scene: public Drawable
{
public:

	ScriptEngine mSE;

	Map *mMap;
	int mVPX, mVPY;
	tick_t mLastTick;

	Scene() { mLastTick = 0; }

	void
	SetMap(Map *map) {
		mMap = map;
	}

	void
	SetViewPoint(int vpx, int vpy) {
		mVPX = vpx;
		mVPY = vpy;
	}

	void Draw(tick_t tick, void *scene) {
		SDL_Surface *screen = (SDL_Surface *)scene;
		SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0, 0, 0));

		SDL_Rect rect;

		rect.x = 0;
		rect.y = 0;
		mMap->Show(tick, screen, &rect, mVPX, mVPY, 640, 480);

		mLastTick = tick;
	}
} world;

class : public Event
{
public:

	void Activate(void) {
		SDL_WM_SetCaption("GAME", NULL);
		SDL_keystate = SDL_GetKeyState(NULL);

		world.mSE.LoadScript("script/test");
	
		object_t exret;
		std::vector<object_t> excall;
		while (1)
		{
			int r = world.mSE.Execute(exret, &excall);
			if (r == APPLY_EXIT || r == APPLY_EXIT_NO_VALUE)
				break;
			/* An example for handling external calls: display */
			if (r == APPLY_EXTERNAL_CALL)
			{
				if (xstring_equal_cstr(excall[0]->string, "SetCurrentMap", -1))
				{
					world.SetMap((Map *)excall[1]->external.priv);
					exret = OBJECT_NULL;
				}
				else if (xstring_equal_cstr(excall[0]->string, "GetSimpleSprite", -1))
				{
					exret = world.mSE.ObjectNew();
					exret->external.priv = Resource::Get<SimpleSprite>(xstring_cstr(excall[1]->string));
					exret->external.enumerate = NULL;
					exret->external.free = NULL;
					OBJECT_TYPE_INIT(exret, OBJECT_TYPE_EXTERNAL);
				}
				else if (xstring_equal_cstr(excall[0]->string, "GetMap", -1))
				{
					exret = world.mSE.ObjectNew();
					exret->external.priv = Resource::Get<Map>(xstring_cstr(excall[1]->string));
					exret->external.enumerate = NULL;
					exret->external.free = NULL;
					OBJECT_TYPE_INIT(exret, OBJECT_TYPE_EXTERNAL);
				}
				else if (xstring_equal_cstr(excall[0]->string, "AddSpriteToMap", -1))
				{
					Map *map = (Map *)excall[1]->external.priv;
					Sprite *sprite = (Sprite *)excall[2]->external.priv;
					int x = INT_UNBOX(excall[3]);
					int y = INT_UNBOX(excall[4]);
					int z = INT_UNBOX(excall[5]);
					int w = INT_UNBOX(excall[6]);
					int h = INT_UNBOX(excall[7]);
					int dx = INT_UNBOX(excall[8]);
					int dy = INT_UNBOX(excall[9]);

					exret = world.mSE.ObjectNew();
					exret->external.priv = map->AddSprite(sprite, x, y, z, w, h, dx, dy);
					exret->external.enumerate = NULL;
					exret->external.free = NULL;
					OBJECT_TYPE_INIT(exret, OBJECT_TYPE_EXTERNAL);
				}
				else if (xstring_equal_cstr(excall[0]->string, "SetViewPoint", -1))
				{
					int x = INT_UNBOX(excall[1]);
					int y = INT_UNBOX(excall[2]);

					world.SetViewPoint(x, y);

					exret = OBJECT_NULL;
				}

				else exret = OBJECT_NULL;
			}
		}

		IO::Open();
	}
} initEvent;

class : public Event
{
public:

	void
	Activate(void) {
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
