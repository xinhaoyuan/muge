#include "timer.hpp"
#include "object.hpp"
#include "event.hpp"

namespace GameEngine
{
	Timer::Timer(void)
	{
		mNode.key  = 0;
		mEvent     = Event::sDummy;
		mEventLoop = &EventLoop::sMain;
	}

	Timer::Timer(tick_t tick, EventLoop *eventLoop, Event *event)
	{
		mNode.key = tick;
		mEventLoop = eventLoop;
		mEvent = event;
	}

	void
	Timer::SetTick(tick_t tick)
	{
		mNode.key = tick;
	}

	void
	Timer::SetEvent(Event *event)
	{
		mEvent = event;
	}

	void
	Timer::SetEventLoop(EventLoop *eventLoop)
	{
		mEventLoop = eventLoop;
	}

	void
	Timer::Open(TimerPool *pool)
	{
		pool->Enqueue(this);
	}

	void
	Timer::Close(void)
	{
		TimerPool *pool = mPool;
		if (pool == NULL) return;		
		pool->Dequeue(this);
	}

	bool
	Timer::IsClosed(void)
	{
		return mPool == NULL;
	}

	TimerPool::TimerPool(tick_t startTick, int hz)
	{
		mTick = startTick;
		mHZ = hz;
		mThread = NULL;

		crh_init(&mCRH);
		crh_set_base(&mCRH, startTick);
	}

	TimerPool::~TimerPool(void)
	{
		Stop();
	}

	void
	TimerPool::Enqueue(Timer *timer)
	{
		mLock.lock();
		timer->mLock.lock();

		if (timer->mPool == NULL)
		{
			if (crh_insert(&mCRH, &timer->mNode))
			{
				if (timer->mEventLoop)
					timer->mEventLoop->Enqueue(timer->mEvent);
			}
			else timer->mPool = this;
		}
		
		timer->mLock.unlock();
		mLock.unlock();
		mEventCV.notify_one();
	}

	void
	TimerPool::Dequeue(Timer *timer)
	{
		mLock.lock();
		timer->mLock.lock();

		if (timer->mPool == this)
		{
			crh_remove(&mCRH, &timer->mNode);
			timer->mPool = NULL;
		}
		
		timer->mLock.unlock();
		mLock.unlock();
	}

	void
	TimerThreadHelper::operator()(void)
	{
		mPool->DoThread();
	}
	
	void
	TimerPool::Start(void)
	{
		mThreadExit = false;
		mThreadPause = true;
		
		mThread = new std::thread(TimerThreadHelper(this));
	}

	void
	TimerPool::Stop(void)
	{
		mThreadExit = true;
		Pause();

		mThread->join();
		delete mThread;
		mThread = NULL;
	}

	void
	TimerPool::Pause(void)
	{
		mThreadPause = true;
		mEventCV.notify_one();
	}
	
	void
	TimerPool::Resume(void)
	{
		mThreadPause = false;
		mEventCV.notify_one();
	}

	void
	TimerPool::DoThread()
	{
		mLock.lock();
		
		mLastTime = std::chrono::system_clock::now();
		mLastTick = mTick;
		
		while (!mThreadExit)
		{
			while (!mThreadPause)
			{
				tick_t tickDelta = mLastTick +
					(std::chrono::system_clock::now() - mLastTime).count() *
					((double)mHZ * std::chrono::system_clock::period::num
					 / std::chrono::system_clock::period::den);
				tickDelta -= mTick;
				
				while (tickDelta)
				{
					tick_t step = crh_max_step(&mCRH);
					if (step > tickDelta || step == 0)
					{
						step = tickDelta;
						mTick += step;
						crh_set_base(&mCRH, mTick);
						
						break;
					}

					mTick += step;
					tickDelta -= step;
					crh_node_t node = crh_set_base(&mCRH, mTick);

					if (node == NULL) continue;

					crh_node_t cur = node;
					while (1)
					{
						Timer *timer = (Timer *)((uintptr_t)cur - (uintptr_t)(&((Timer *)0)->mNode));

						timer->mLock.lock();
						crh_remove(&mCRH, &timer->mNode);
						timer->mPool = NULL;
						if (timer->mEventLoop)
							timer->mEventLoop->Enqueue(timer->mEvent);

						cur->next->prev = cur->prev;
						cur->prev->next = cur->next;
						if (cur == cur->next)
						{
							timer->mLock.unlock();
							break;
						}
						else
						{
							cur = cur->next;
							timer->mLock.unlock();
						}
					}
				};

				tick_t waitTick = crh_max_step(&mCRH);
				if (waitTick == 0) waitTick = mHZ;
				waitTick = (mTick - mLastTick + waitTick) / (double)mHZ
					/ std::chrono::system_clock::period::num
					* std::chrono::system_clock::period::den;

				std::chrono::system_clock::time_point waitPoint =
					mLastTime + std::chrono::system_clock::duration(waitTick);
				mEventCV.wait_until(mLock, waitPoint);
			}

			mLastTime = std::chrono::system_clock::now();
			mLastTick = mTick;

			if (!mThreadExit) mEventCV.wait(mLock);
		}
		mLock.unlock();
	}

	tick_t
	TimerPool::GetTick(void)
	{
		mLock.lock();
		tick_t tick = mLastTick +
			(std::chrono::system_clock::now() - mLastTime).count() *
			((double)mHZ * std::chrono::system_clock::period::num
			 / std::chrono::system_clock::period::den);
		mLock.unlock();

		return tick;
	}
};
