#ifndef __GE_TIMER_HPP_
#define __GE_TIMER_HPP_

#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>

#include "object.hpp"
#include "event.hpp"

extern "C" {
#include "crh.h"
}

namespace GameEngine
{
	class TimerPool;
	
	class Timer
	{
	private:
		friend class TimerPool;

		crh_node_s mNode;

		std::mutex mLock;
		TimerPool *mPool;
		EventLoop *mEventLoop;
		Event     *mEvent;

	public:
		Timer(void);
		Timer(tick_t tick, EventLoop *eventLoop, Event *event);

		void SetEvent(Event *event);
		void SetEventLoop(EventLoop *eventLoop);
		void SetTick(tick_t tick);

		void Open(TimerPool *pool);
		void Close(void);
		bool IsClosed(void);
	};

	class TimerThreadHelper
	{
	private:
		TimerPool *mPool;
		
	public:
		inline TimerThreadHelper(TimerPool *pool) : mPool(pool) { };
		void operator()(void);
	};
	
	class TimerPool
	{
	private:
		friend class Timer;
		friend class TimerThreadHelper;

		int mHZ;
		
		std::chrono::system_clock::time_point mLastTime;
		tick_t       mTick, mLastTick;
		std::thread *mThread;
		std::mutex   mLock;
		std::condition_variable_any mEventCV;

		bool mThreadExit, mThreadPause;

		crh_s mCRH;

		void DoThread(void);

		void Enqueue(Timer *timer);
		void Dequeue(Timer *timer);
		
	public:

		TimerPool(tick_t startTick, int hz);
		~TimerPool(void);

		void Start(void);
		void Stop(void);

		void Pause(void);
		void Resume(void);
		
		tick_t GetTick(void);
	};
}

#endif
