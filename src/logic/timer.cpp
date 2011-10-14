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
		mPool      = NULL;
	}

	Timer::Timer(tick_t tick, EventLoop *eventLoop, Event *event)
	{
		mNode.key = tick;
		mEventLoop = eventLoop;
		mEvent = event;
		mPool = NULL;
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

	void
	Timer::Lock(void)
	{
		mLock.lock();
	}

	void
	Timer::Unlock(void)
	{
		mLock.unlock();
	}

	ThreadedTimerPool::ThreadedTimerPool(tick_t startTick, int hz)
	{
		mLastTick = mTick = startTick;
		mHZ = hz;
		mThread = NULL;

		crh_init(&mCRH);
		crh_set_base(&mCRH, startTick);
	}

	ThreadedTimerPool::~ThreadedTimerPool(void)
	{
		Stop();
	}

	void
	ThreadedTimerPool::Enqueue(Timer *timer)
	{
		mLock.lock();
		timer->Lock();

		if (timer->mPool == NULL)
		{
			if (crh_insert(&mCRH, &timer->mNode))
			{
				if (timer->mEventLoop)
					timer->mEventLoop->Enqueue(timer->mEvent);
			}
			else timer->mPool = this;
		}
		
		timer->Unlock();
		mLock.unlock();
		mEventCV.notify_one();
	}

	void
	ThreadedTimerPool::Dequeue(Timer *timer)
	{
		mLock.lock();
		timer->Lock();

		if (timer->mPool == this)
		{
			crh_remove(&mCRH, &timer->mNode);
			timer->mPool = NULL;
		}
		
		timer->Unlock();
		mLock.unlock();
	}

	void
	TimerThreadHelper::operator()(void)
	{
		mPool->DoThread();
	}
	
	void
	ThreadedTimerPool::Start(void)
	{
		mThreadExit = false;
		mThreadPause = true;
		
		mThread = new std::thread(TimerThreadHelper(this));
	}

	void
	ThreadedTimerPool::Stop(void)
	{
		mThreadExit = true;
		Pause();

		mThread->join();
		delete mThread;
		mThread = NULL;
	}

	void
	ThreadedTimerPool::Pause(void)
	{
		mThreadPause = true;
		mEventCV.notify_one();
	}
	
	void
	ThreadedTimerPool::Resume(void)
	{
		mThreadPause = false;
		mEventCV.notify_one();
	}

	void
	ThreadedTimerPool::DoThread()
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

						timer->Lock();
						crh_remove(&mCRH, &timer->mNode);
						timer->mPool = NULL;
						if (timer->mEventLoop)
							timer->mEventLoop->Enqueue(timer->mEvent);

						cur->next->prev = cur->prev;
						cur->prev->next = cur->next;
						if (cur == cur->next)
						{
							timer->Unlock();
							break;
						}
						else
						{
							cur = cur->next;
							timer->Unlock();
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
	ThreadedTimerPool::GetTick(void)
	{
		tick_t tick;
		mLock.lock();
		if (mThreadPause)
		{
			tick = mTick;
		}
		else
		{
			tick = mLastTick +
				(std::chrono::system_clock::now() - mLastTime).count() *
				((double)mHZ * std::chrono::system_clock::period::num
				 / std::chrono::system_clock::period::den);
		}
		mLock.unlock();
		return tick;
	}

	MonoTimerPool::MonoTimerPool(void)
	{
		mTick = 0;
		crh_init(&mCRH);
		crh_set_base(&mCRH, mTick);
	}


	MonoTimerPool::MonoTimerPool(tick_t startTick)
	{
		mTick = startTick;
		crh_init(&mCRH);
		crh_set_base(&mCRH, mTick);
	}

	MonoTimerPool::~MonoTimerPool(void)
	{
	}

	void
	MonoTimerPool::Enqueue(Timer *timer)
	{
		timer->Lock();
		if (timer->mPool == NULL)
		{
			if (crh_insert(&mCRH, &timer->mNode))
			{
				Event *event = timer->mEvent;
				timer->Unlock();
				
				event->Activate();
				return;
			}
			else timer->mPool = this;
		}		
		timer->Unlock();
	}

	void
	MonoTimerPool::Dequeue(Timer *timer)
	{
		timer->Lock();
		if (timer->mPool == this)
		{
			crh_remove(&mCRH, &timer->mNode);
			timer->mPool = NULL;
		}
		timer->Unlock();
	}

	void
	MonoTimerPool::SetTick(tick_t tick)
	{
		tick_t tickDelta = tick - mTick;
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

				timer->Lock();
				crh_remove(&mCRH, &timer->mNode);
				timer->mPool = NULL;
				Event *event = timer->mEvent;

				cur->next->prev = cur->prev;
				cur->prev->next = cur->next;
				if (cur == cur->next)
				{
					timer->Unlock();
					event->Activate();
					break;
				}
				else
				{
					cur = cur->next;
					timer->Unlock();
					event->Activate();
				}
			}
		}

		mTick = tick;
		crh_set_base(&mCRH, mTick);
	}

	tick_t
	MonoTimerPool::GetTick(void)
	{
		return mTick;
	}
};
