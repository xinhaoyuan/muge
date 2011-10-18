#ifndef __GE_IO_H__
#define __GE_IO_H__

#include "../logic/object.hpp"
#include "../logic/timer.hpp"

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <thread>

namespace GameEngine
{
	class DrawEvent : public Event
	{
	public:
		void Activate(void);
	};
	
	class IO
	{
	public:
		typedef void(*KeyboardHandler)(int down, SDL_keysym sym);
		
	private:

		friend class DrawEvent;
		
		static Drawable    *sRootDrawable;
		static SDL_Surface *sDisplay;

		static ThreadedTimerPool *sTimerPool;
		static Timer      sDrawTimer;
		static DrawEvent  sDrawEvent;

		static int  sScreenWidth;
		static int  sScreenHeight;
		static int  sScreenBPP;
		static bool sFullScreen;
		static int  sRefreshHZ;

		static Event *sInitEvent;
		static Event *sQuitEvent;
		static std::thread *sThread;

		static KeyboardHandler sKeyboardHandler;
		
	public:

		static void SetKeyboardHandler(KeyboardHandler handler);
		static void SetScreen(int width, int height, int bpp, bool fullScreen, int refreshHZ, Drawable *rootDrawable);
		static void Initialize(Event *initEvent, Event *quitEvent);
		static void Open(void);
		static void Quit(void);

		static void Thread(void);
	};
}

#endif
