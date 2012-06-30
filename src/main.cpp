#include <SDL/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <deque>

#include <cpp_binding/script.hpp>

#include "logic/event.hpp"
#include "logic/timer.hpp"
#include "io/io.hpp"

#include "sprite.hpp"
#include "map.hpp"
#include "resource.hpp"
#include "conv.hpp"

using namespace GameEngine;
using namespace Game;

Uint8 *SDL_keystate;

class Scene: public Drawable
{
public:

    ScriptEngine mSE;
    object_t mHandlerPair;
    
    Map *mMap;
    Motion mVPXMotion, mVPYMotion;
    
    Conversation *mConv;

#define TIMER_COUNT 16
#define TIMER_MAP   0
    
    tick_t        mTick[TIMER_COUNT];
    bool          mTimerPause[TIMER_COUNT];
    MonoTimerPool mTimer[TIMER_COUNT];

    Scene() {
        int i;
        for (i = 0; i < TIMER_COUNT; ++ i)
        {
            mTick[i] = 0;
            mTimerPause[i] = true;
        }
    }

    void
    SetMap(Map *map) {
        mMap = map;
    }

    void Draw(tick_t tick, void *scene) {       
        int i;
        for (i = 0; i < TIMER_COUNT; ++ i)
        {
            if (!mTimerPause[i])
            {
                mTick[i] = mTimer[i].GetTick() + 1;
                mTimer[i].SetTick(mTick[i]);
            }
        }
        
        SDL_Surface *screen = (SDL_Surface *)scene;
        SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0, 0, 0));

        SDL_Rect rect;
        
        if (mMap)
        {
            rect.x = 0;
            rect.y = 0;
            mMap->UpdateMotion(mTick[TIMER_MAP]);
            mMap->Show(tick, screen, &rect,
                       mVPXMotion.Get(mTick[TIMER_MAP]),
                       mVPYMotion.Get(mTick[TIMER_MAP]),
                       640, 480);
        }

        if (mConv)
        {
            mConv->Show(tick, screen, &rect);
        }
    }
} world;

class SEETimerEvent : public Event
{
public:
    object_t mContainer;
    Timer   *mTimer;
    
    virtual void Activate(void) {

        static std::vector<object_t> args;
        static std::vector<object_t> excall;

        args.clear();
        world.mSE.Apply(SLOT_GET(mContainer->pair.slot_car), &args);
        world.mSE.Execute(NULL, &excall);
        world.mSE.ObjectUnprotect(mContainer);

        delete mTimer;
        delete this;
    }

    virtual ~SEETimerEvent() { }
};

#define DECLARE_EXFUNC(name) \
    static object_t EXFUNC_##name(void *priv, int argc, object_t *argv)
#define EXFUNC     argv[0]
#define EXARG(idx) argv[(idx) + 1]

DECLARE_EXFUNC(CurrentMapSet)
{
    if (EXARG(1) == OBJECT_NULL)
        world.SetMap(NULL);
    else world.SetMap((Map *)EXARG(0)->external.priv);
    return OBJECT_NULL;
}

DECLARE_EXFUNC(SimpleSpriteGet)
{
    object_t result;
    
    result = world.mSE.ObjectNew();
    result->external.priv = Resource::Get<SimpleSprite>(xstring_cstr(EXARG(0)->string));
    result->external.type = &external_type_dummy;
    OBJECT_TYPE_INIT(result, OBJECT_TYPE_EXTERNAL);

    return result;
}

DECLARE_EXFUNC(MapGet)
{
    object_t result;
    
    result = world.mSE.ObjectNew();
    result->external.priv = Resource::Get<Map>(xstring_cstr(EXARG(0)->string));
    result->external.type = &external_type_dummy;
    OBJECT_TYPE_INIT(result, OBJECT_TYPE_EXTERNAL);

    return result;
}

DECLARE_EXFUNC(NodeStateSet)
{
    TileNode *node = (TileNode *)EXARG(0)->external.priv;
    int state = INT_UNBOX(EXARG(1));

    node->mState = state;
    
    return OBJECT_NULL;
}

DECLARE_EXFUNC(SpriteAdd)
{
    Map *map = (Map *)EXARG(0)->external.priv;  
    Sprite *sprite = (Sprite *)EXARG(1)->external.priv;
    int x = INT_UNBOX(EXARG(2));
    int y = INT_UNBOX(EXARG(3));
    int z = INT_UNBOX(EXARG(4));
    int w = INT_UNBOX(EXARG(5));
    int h = INT_UNBOX(EXARG(6));
    int dx = INT_UNBOX(EXARG(7));
    int dy = INT_UNBOX(EXARG(8));

    object_t result = world.mSE.ObjectNew();
    TileNode *node =
        map->AddMotiveSprite(sprite, w, h, dx, dy);
    result->external.priv = node;
    
    node->mMotion->mXMotion.SetConstant(x);
    node->mMotion->mYMotion.SetConstant(y);
    node->mMotion->mZMotion.SetConstant(z);
    
    result->external.type = &external_type_dummy;
    OBJECT_TYPE_INIT(result, OBJECT_TYPE_EXTERNAL);

    return result;
}

DECLARE_EXFUNC(NodeMove)
{
    TileNode *node = (TileNode *)EXARG(0)->external.priv;
    int x = INT_UNBOX(EXARG(1));
    int y = INT_UNBOX(EXARG(2));
    int z = INT_UNBOX(EXARG(3));
    int l = INT_UNBOX(EXARG(4));

    node->mMotion->mXMotion.SetInterval(world.mTick[TIMER_MAP], node->mMotion->mXMotion.Get(world.mTick[TIMER_MAP]),
                                        world.mTick[TIMER_MAP] + l, x);
    node->mMotion->mYMotion.SetInterval(world.mTick[TIMER_MAP], node->mMotion->mYMotion.Get(world.mTick[TIMER_MAP]),
                                        world.mTick[TIMER_MAP] + l, y);
    node->mMotion->mZMotion.SetInterval(world.mTick[TIMER_MAP], node->mMotion->mZMotion.Get(world.mTick[TIMER_MAP]),
                                        world.mTick[TIMER_MAP] + l, z);

    return OBJECT_NULL;
}


DECLARE_EXFUNC(NodeShiver)
{
    TileNode *node = (TileNode *)EXARG(0)->external.priv;
    int d = INT_UNBOX(EXARG(1));

    node->mMotion->mXMotion.SetShiver(node->mMotion->mXMotion.Get(world.mTick[TIMER_MAP]), d);
    node->mMotion->mYMotion.SetShiver(node->mMotion->mYMotion.Get(world.mTick[TIMER_MAP]), d);
    node->mMotion->mZMotion.SetConstant(node->mMotion->mZMotion.Get(world.mTick[TIMER_MAP]));

    return OBJECT_NULL;
}


DECLARE_EXFUNC(ViewPointSet)
{
    int x = INT_UNBOX(EXARG(0));
    int y = INT_UNBOX(EXARG(1));

    world.mVPXMotion.SetConstant(x);
    world.mVPYMotion.SetConstant(y);

    return OBJECT_NULL;
}

DECLARE_EXFUNC(ViewPointMove)
{
    int x = INT_UNBOX(EXARG(0));
    int y = INT_UNBOX(EXARG(1));
    int l = INT_UNBOX(EXARG(2));

    world.mVPXMotion.SetInterval(world.mTick[TIMER_MAP], world.mVPXMotion.Get(world.mTick[TIMER_MAP]),
                                 world.mTick[TIMER_MAP] + l, x);
    world.mVPYMotion.SetInterval(world.mTick[TIMER_MAP], world.mVPYMotion.Get(world.mTick[TIMER_MAP]),
                                 world.mTick[TIMER_MAP] + l, x);

    return OBJECT_NULL;
}


DECLARE_EXFUNC(ViewPointShiver)
{
    int d = INT_UNBOX(EXARG(0));

    world.mVPXMotion.SetShiver(world.mVPXMotion.Get(world.mTick[TIMER_MAP]), d);
    world.mVPYMotion.SetShiver(world.mVPYMotion.Get(world.mTick[TIMER_MAP]), d);
    
    return OBJECT_NULL;
}

DECLARE_EXFUNC(DelayEventAdd)
{
    int tick_idx = INT_UNBOX(EXARG(0));
    int delay = INT_UNBOX(EXARG(1));
    object_t handler = EXARG(2);

    object_t container = world.mSE.ObjectNew();
    SLOT_SET(container->pair.slot_car, handler);
    SLOT_SET(container->pair.slot_cdr, OBJECT_NULL);
    OBJECT_TYPE_INIT(container, OBJECT_TYPE_PAIR);

    SEETimerEvent *event = new SEETimerEvent;
    event->mContainer = container;
    event->mTimer = new Timer(world.mTick[tick_idx] + delay, NULL, event);
    event->mTimer->Open(&world.mTimer[tick_idx]);

    return OBJECT_NULL;
}

DECLARE_EXFUNC(TimerPause)
{
    int tick_idx = INT_UNBOX(EXARG(0));
    world.mTimerPause[tick_idx] = true;
    return OBJECT_NULL;
}

DECLARE_EXFUNC(TimerResume)
{
    int tick_idx = INT_UNBOX(EXARG(0));
    world.mTimerPause[tick_idx] = false;
    return OBJECT_NULL;
}

DECLARE_EXFUNC(CurrentConversationSet)
{
    if (EXARG(0) == OBJECT_NULL)
        world.mConv = NULL;
    else world.mConv = (Conversation *)EXARG(0)->external.priv;
    return OBJECT_NULL;
}

DECLARE_EXFUNC(ConversationPageSet)
{
    Conversation *conv = (Conversation *)EXARG(0)->external.priv;
    int page = INT_UNBOX(EXARG(1));

    if (conv->SetPage(page))
        return INT_BOX(0);
    else return OBJECT_NULL;
}

DECLARE_EXFUNC(ConversationGet)
{
    object_t result;
    
    result = world.mSE.ObjectNew();
    result->external.priv = Resource::Get<Conversation>(xstring_cstr(EXARG(0)->string));
    result->external.type = &external_type_dummy;
    OBJECT_TYPE_INIT(result, OBJECT_TYPE_EXTERNAL);

    return result;
}

class : public Event
{
public:

    void Activate(void) {
        SDL_WM_SetCaption("GAME", NULL);
        SDL_keystate = SDL_GetKeyState(NULL);

        world.mVPXMotion.SetConstant(0);
        world.mVPYMotion.SetConstant(0);
        
        world.mHandlerPair = world.mSE.ObjectNew();
        SLOT_SET(world.mHandlerPair->pair.slot_car, OBJECT_NULL);
        SLOT_SET(world.mHandlerPair->pair.slot_cdr, OBJECT_NULL);
        OBJECT_TYPE_INIT(world.mHandlerPair, OBJECT_TYPE_PAIR);
        
        object_t script = world.mSE.LoadScript("script/test.ss");
        
        world.mSE.ExternalFuncRegister("CurrentMapSet", EXFUNC_CurrentMapSet, NULL);
        world.mSE.ExternalFuncRegister("MapGet", EXFUNC_MapGet, NULL);

        world.mSE.ExternalFuncRegister("SimpleSpriteGet", EXFUNC_SimpleSpriteGet, NULL);
        
        world.mSE.ExternalFuncRegister("SpriteAdd", EXFUNC_SpriteAdd, NULL);
        
        world.mSE.ExternalFuncRegister("NodeStateSet", EXFUNC_NodeStateSet, NULL);
        world.mSE.ExternalFuncRegister("NodeMove", EXFUNC_NodeMove, NULL);
        world.mSE.ExternalFuncRegister("NodeShiver", EXFUNC_NodeShiver, NULL);
        
        world.mSE.ExternalFuncRegister("ViewPointSet", EXFUNC_ViewPointSet, NULL);
        world.mSE.ExternalFuncRegister("ViewPointMove", EXFUNC_ViewPointMove, NULL);
        world.mSE.ExternalFuncRegister("ViewPointShiver", EXFUNC_ViewPointShiver, NULL);

        world.mSE.ExternalFuncRegister("DelayEventAdd", EXFUNC_DelayEventAdd, NULL);
        world.mSE.ExternalFuncRegister("TimerPause", EXFUNC_TimerPause, NULL);
        world.mSE.ExternalFuncRegister("TimerResume", EXFUNC_TimerResume, NULL);

        world.mSE.ExternalFuncRegister("ConversationGet", EXFUNC_ConversationGet, NULL);
        world.mSE.ExternalFuncRegister("CurrentConversationSet", EXFUNC_CurrentConversationSet, NULL);
        world.mSE.ExternalFuncRegister("ConversationPageSet", EXFUNC_ConversationPageSet, NULL);

        object_t exret;
        std::vector<object_t> excall;
        excall.clear();
        world.mSE.Apply(script, &excall);
        while (1)
        {
            int r = world.mSE.Execute(exret, &excall);
            if (r == APPLY_EXTERNAL_CALL)
            {
                exret = OBJECT_NULL;
            }
            else break;
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
