#include "event.hpp"

namespace GameEngine
{

	EventLoop EventLoop::sMain;
	
	void
	EventLoop::Enqueue(Event *event)
	{
		mLock.lock();
		if (event->mStatus == Event::Waiting)
		{
			event->mStatus == Event::InQueueWeak;
			mEventQueue.push_back(event);
		}
		else if (event->mStatus == Event::InQueueWeak)
		{
			event->mStatus = Event::InQueueStrong;
		}
		mLock.unlock();
		mMainCV.notify_one();
	};
	
	void
	EventLoop::Thread(void)
	{
		mLock.lock();
		while (!mToExit)
		{
			if (mEventQueue.empty())
			{
				mMainCV.wait(mLock);
				continue;
			}
			
			Event *event = mEventQueue.front();
			mEventQueue.pop_front();
			
			if (event->mStatus == Event::InQueueWeak)
			{
				event->mStatus = Event::Waiting;
			}
			else if (event->mStatus == Event::InQueueStrong)
			{
				event->mStatus = Event::InQueueWeak;
				mEventQueue.push_back(event);
			}
			
			mLock.unlock();			
			event->Activate();
			mLock.lock();
		}
		mLock.unlock();
	}

	class EventLoopHelper
	{
	public:
		EventLoop *mLoop;
		EventLoopHelper(EventLoop *loop) : mLoop(loop) { }

		void operator()(void) { mLoop->Thread(); }
	};

	void
	EventLoop::Start(void)
	{
		mToExit = false;

		if (this == &sMain)
		{
			mThread = NULL;
			Thread();
		}
		else mThread = new std::thread(EventLoopHelper(this));
	}

	void
	EventLoop::Stop(void)
	{
		mToExit = true;
		mMainCV.notify_one();
		
		if (this != &sMain)
		{
			mThread->join();
			delete mThread;
			mThread = NULL;
		}
	}
}
