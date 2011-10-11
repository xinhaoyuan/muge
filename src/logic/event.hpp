#ifndef __GE_EVENT_HPP__
#define __GE_EVENT_HPP__

#include <list>

#include <mutex>
#include <thread>
#include <condition_variable>

#include "object.hpp"

namespace GameEngine
{
	const int DrawHZ = 60;
	const int TickHZ = 60;

	class EventLoop
	{
	private:
		friend class EventLoopHelper;
		
		std::list<Event *> mEventQueue;

		volatile bool mToExit;
		std::mutex    mLock;
		std::condition_variable_any mMainCV;
		std::thread  *mThread;

		void Thread(void);
		
	public:

		EventLoop(void) { };

		static EventLoop sMain;
		
		void Enqueue(Event *event);
		void Start(void);
		void Stop(void);
	};
}

#endif
