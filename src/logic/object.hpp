#ifndef __GE_OBJECT_HPP__
#define __GE_OBJECT_HPP__

#include <iostream>

namespace GameEngine
{
    typedef unsigned long long tick_t;
    
    class Serializable
    {
    public:
        static Serializable * const sDummy;
        
        virtual void Serialize(std::ostream *s) = 0;
        virtual void Deserialize(std::istream *s) = 0;
    };

    class Drawable
    {
    public:
        static Drawable * const sDummy;
        
        virtual void Draw(tick_t tick, void *scene) = 0;
    };

    class Event
    {
    private:
        
        friend class EventLoop;
        enum { Waiting, InQueueWeak, InQueueStrong } mStatus;
        
    public:
        static Event * const sDummy;

        virtual void Activate(void) = 0;
        Event() { }
        ~Event() { }
    };  

    class Object
    {
    public:
        // The default is return the dummy
        virtual Drawable     *GetDrawable(void);
        virtual Serializable *GetSerializable(void);
    };
}

#endif
