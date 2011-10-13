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
	object_t mHandlerPair;
	
	Map *mMap;
	int mVPX, mVPY;
	tick_t mTick;

	std::map<tick_t, std::deque<object_t> > mTickEvent;

	Scene() { mTick = 0; }

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

		std::vector<object_t> eventArgs;
		std::vector<object_t> excall;
		std::map<tick_t, std::deque<object_t> >::iterator b;
		
		while (((b = mTickEvent.begin()) != mTickEvent.end()) &&
			   (b->first <= tick))
		{
			mTick = b->first;

			std::deque<object_t> *q = &b->second;
			std::deque<object_t>::iterator _it = q->begin();
			while (_it != q->end())
			{
				object_t container = *_it;
				mSE.Apply(SLOT_GET(container->pair.slot_car), &eventArgs, &excall);
				mSE.ObjectUnprotect(container);

				++ _it;
			}

			mTickEvent.erase(b);
		}
		
		SDL_Surface *screen = (SDL_Surface *)scene;
		SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0, 0, 0));

		SDL_Rect rect;

		rect.x = 0;
		rect.y = 0;
		mMap->UpdateMotion(tick);
		mMap->Show(tick, screen, &rect, mVPX, mVPY, 640, 480);
	}
} world;

static object_t
EXFUNC_SetCurrentMap(void *, object_t func, int argc, object_t *argv)
{
	world.SetMap((Map *)argv[0]->external.priv);
	return OBJECT_NULL;
}

static object_t
EXFUNC_GetSimpleSprite(void *, object_t func, int argc, object_t *argv)
{
	object_t result;
	
	result = world.mSE.ObjectNew();
	result->external.priv = Resource::Get<SimpleSprite>(xstring_cstr(argv[0]->string));
	result->external.enumerate = NULL;
	result->external.free = NULL;
	OBJECT_TYPE_INIT(result, OBJECT_TYPE_EXTERNAL);

	return result;
}

static object_t
EXFUNC_GetMap(void *, object_t func, int argc, object_t *argv)
{
	object_t result;
	
	result = world.mSE.ObjectNew();
	result->external.priv = Resource::Get<Map>(xstring_cstr(argv[0]->string));
	result->external.enumerate = NULL;
	result->external.free = NULL;
	OBJECT_TYPE_INIT(result, OBJECT_TYPE_EXTERNAL);

	return result;
}

static object_t
EXFUNC_AddSpriteToMap(void *, object_t func, int argc, object_t *argv)
{
	Map *map = (Map *)argv[0]->external.priv;
	Sprite *sprite = (Sprite *)argv[1]->external.priv;
	int x = INT_UNBOX(argv[2]);
	int y = INT_UNBOX(argv[3]);
	int z = INT_UNBOX(argv[4]);
	int w = INT_UNBOX(argv[5]);
	int h = INT_UNBOX(argv[6]);
	int dx = INT_UNBOX(argv[7]);
	int dy = INT_UNBOX(argv[8]);

	object_t result = world.mSE.ObjectNew();
	TileNode *node =
		map->AddMotiveSprite(sprite, w, h, dx, dy);
	result->external.priv = node;
	
	node->mMotion->mXMotion.SetConstant(x);
	node->mMotion->mYMotion.SetConstant(y);
	node->mMotion->mZMotion.SetConstant(z);
	
	result->external.enumerate = NULL;
	result->external.free = NULL;
	OBJECT_TYPE_INIT(result, OBJECT_TYPE_EXTERNAL);

	return result;
}

static object_t
EXFUNC_SpriteMoveTo(void *, object_t func, int argc, object_t *argv)
{
	TileNode *node = (TileNode *)argv[0]->external.priv;
	int x = INT_UNBOX(argv[1]);
	int y = INT_UNBOX(argv[2]);
	int z = INT_UNBOX(argv[3]);
	int l = INT_UNBOX(argv[4]);

	node->mMotion->mXMotion.SetInterval(world.mTick, node->mMotion->mXMotion.Get(world.mTick),
										world.mTick + l, x);
	node->mMotion->mYMotion.SetInterval(world.mTick, node->mMotion->mYMotion.Get(world.mTick),
										world.mTick + l, y);
	node->mMotion->mZMotion.SetInterval(world.mTick, node->mMotion->mZMotion.Get(world.mTick),
										world.mTick + l, z);

	return OBJECT_NULL;
}


static object_t
EXFUNC_SetViewPoint(void *, object_t func, int argc, object_t *argv)
{
	int x = INT_UNBOX(argv[0]);
	int y = INT_UNBOX(argv[1]);

	world.SetViewPoint(x, y);

	return OBJECT_NULL;
}

static object_t
EXFUNC_AddTickEvent(void *, object_t func, int argc, object_t *argv)
{
	int tick_delta = INT_UNBOX(argv[0]);
	object_t handler = argv[1];

	object_t container = world.mSE.ObjectNew();
	SLOT_SET(container->pair.slot_car, handler);
	SLOT_SET(container->pair.slot_cdr, OBJECT_NULL);
	OBJECT_TYPE_INIT(container, OBJECT_TYPE_PAIR);
	
	world.mTickEvent[world.mTick + tick_delta].push_back(container);

	return OBJECT_NULL;
}

class : public Event
{
public:

	void Activate(void) {
		SDL_WM_SetCaption("GAME", NULL);
		SDL_keystate = SDL_GetKeyState(NULL);

		world.mHandlerPair = world.mSE.ObjectNew();
		SLOT_SET(world.mHandlerPair->pair.slot_car, OBJECT_NULL);
		SLOT_SET(world.mHandlerPair->pair.slot_cdr, OBJECT_NULL);
		OBJECT_TYPE_INIT(world.mHandlerPair, OBJECT_TYPE_PAIR);
		
		world.mSE.LoadScript("script/test.ss");
		
		world.mSE.ExternalFuncRegister("SetCurrentMap", EXFUNC_SetCurrentMap, NULL);
		world.mSE.ExternalFuncRegister("GetSimpleSprite", EXFUNC_GetSimpleSprite, NULL);
		world.mSE.ExternalFuncRegister("GetMap", EXFUNC_GetMap, NULL);
		world.mSE.ExternalFuncRegister("AddSpriteToMap", EXFUNC_AddSpriteToMap, NULL);
		world.mSE.ExternalFuncRegister("SpriteMoveTo", EXFUNC_SpriteMoveTo, NULL);
		world.mSE.ExternalFuncRegister("SetViewPoint", EXFUNC_SetViewPoint, NULL);
		world.mSE.ExternalFuncRegister("AddTickEvent", EXFUNC_AddTickEvent, NULL);
		
		object_t exret;
		std::vector<object_t> excall;
		while (1)
		{
			int r = world.mSE.Execute(exret, &excall);
			if (r == APPLY_EXIT || r == APPLY_EXIT_NO_VALUE)
				break;

			if (r == APPLY_EXTERNAL_CALL)
			{
				exret = OBJECT_NULL;
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
