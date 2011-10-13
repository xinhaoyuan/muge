#include <iostream>

#include "io.hpp"

namespace GameEngine
{
	
	Drawable    *IO::sRootDrawable;
	SDL_Surface *IO::sDisplay;
	TimerPool   *IO::sTimerPool;
	Timer        IO::sDrawTimer;
	DrawEvent    IO::sDrawEvent;
	
	int  IO::sScreenWidth;
	int  IO::sScreenHeight;
	int  IO::sScreenBPP;
	bool IO::sFullScreen;
	int  IO::sRefreshHZ;

	Event *IO::sInitEvent;
	Event *IO::sQuitEvent;
	std::thread *IO::sThread;

	IO::KeyboardHandler IO::sKeyboardHandler;

	void
	DrawEvent::Activate(void)
	{
		IO::sRootDrawable->Draw(IO::sTimerPool->GetTick(), IO::sDisplay);
		SDL_Flip(IO::sDisplay);
		
		IO::sDrawTimer.SetTick(IO::sTimerPool->GetTick() + 1);
		IO::sDrawTimer.Open(IO::sTimerPool);
	}

	void
	IO::SetKeyboardHandler(KeyboardHandler handler)
	{
		sKeyboardHandler = handler;
	}

	void
	IO::SetScreen(int width, int height, int bpp, bool fullScreen, int refreshHZ, Drawable *rootDrawable)
	{
		sScreenWidth = width;
		sScreenHeight = height;
		sScreenBPP = bpp;
		sFullScreen = fullScreen;
		sRefreshHZ = refreshHZ;
		sRootDrawable = rootDrawable;
	}

	void
	IO::Initialize(Event *initEvent, Event *quitEvent)
	{
		sInitEvent = initEvent;
		sQuitEvent = quitEvent;
		sThread = new std::thread(Thread);
	}

	void
	IO::Open(void)
	{
		sTimerPool = new TimerPool(0, sRefreshHZ);
		sDrawTimer.SetEventLoop(&EventLoop::sMain);
		sDrawTimer.SetEvent(&sDrawEvent);
		sDrawTimer.SetTick(1);
		sDrawTimer.Open(sTimerPool);

		sTimerPool->Start();
		sTimerPool->Resume();
	}

	void
	IO::Thread(void)
	{
		sDisplay = SDL_SetVideoMode(sScreenWidth, sScreenHeight, sScreenBPP,
									SDL_HWSURFACE | SDL_DOUBLEBUF | (sFullScreen ? SDL_FULLSCREEN : 0));
		EventLoop::sMain.Enqueue(sInitEvent);

		SDL_Event e;
		while (SDL_WaitEvent(&e))
		{
			if (e.type == SDL_USEREVENT && e.user.code == 0)
			{
				SDL_FreeSurface(sDisplay);
				SDL_Quit();
				break;
			}
			
			switch (e.type)
			{
			case SDL_QUIT:
				EventLoop::sMain.Enqueue(sQuitEvent);
				break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				
				if (sKeyboardHandler)
					sKeyboardHandler(e.type == SDL_KEYDOWN, e.key.keysym);
				
				break;
			}
		}
	}

	void
	IO::Quit(void)
	{
		SDL_Event e;
		e.type = SDL_USEREVENT;
		e.user.code = 0;
		
		SDL_PushEvent(&e);
		
		sThread->join();
		delete sThread;
		delete sTimerPool;
	}
}
