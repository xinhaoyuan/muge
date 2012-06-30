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
        friend class ThreadedTimerPool;
        friend class MonoTimerPool;
        
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

        void Lock(void);
        void Unlock(void);

        void Open(TimerPool *pool);
        void Close(void);
        bool IsClosed(void);
    };

    class ThreadedTimerPool;
    
    class TimerThreadHelper
    {
    private:
        ThreadedTimerPool *mPool;
        
    public:
        inline TimerThreadHelper(ThreadedTimerPool *pool) : mPool(pool) { };
        void operator()(void);
    };

    class TimerPool
    {
    public:
        virtual void Enqueue(Timer *timer) = 0;
        virtual void Dequeue(Timer *timer) = 0;
        virtual tick_t GetTick(void) = 0;
    };
    
    class ThreadedTimerPool : public TimerPool
    {
    private :
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
    public:

        virtual void Enqueue(Timer *timer);
        virtual void Dequeue(Timer *timer);


        ThreadedTimerPool(tick_t startTick, int hz);
        virtual ~ThreadedTimerPool(void);

        void Start(void);
        void Stop(void);

        void Pause(void);
        void Resume(void);
        
        virtual tick_t GetTick(void);
    };

    class MonoTimerPool : public TimerPool
    {
        tick_t mTick;
        crh_s mCRH;
        
    public:

        MonoTimerPool(void);
        MonoTimerPool(tick_t startTick);
        virtual ~MonoTimerPool(void);
        
        virtual void Enqueue(Timer *timer);
        virtual void Dequeue(Timer *timer);
        
        virtual tick_t GetTick(void);
        void    SetTick(tick_t tick);
    };
}

#endif
