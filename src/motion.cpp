#include "motion.hpp"

#include <iostream>
#include <stdlib.h>

namespace Game
{
	int
	Motion::Get(GameEngine::tick_t tick)
	{
		switch (mType)
		{
		case MotionConstant:
			return mConstant;

		case MotionInterval:
			if (tick < mInterval.tick_start)
				return mInterval.value_start;
			else if (tick > mInterval.tick_end)
				return mInterval.value_end;
			else return mInterval.value_start +
					 (double)(tick - mInterval.tick_start) *
					 (mInterval.value_end - mInterval.value_start) /
					 (mInterval.tick_end - mInterval.tick_start);
		case MotionShiver:
			return (mShiver.origin + (double)rand() / RAND_MAX * mShiver.delta);
		}
	}

	void
	Motion::SetConstant(int c)
	{
		mType = MotionConstant;
		mConstant = c;
	}
		
	void
	Motion::SetInterval(GameEngine::tick_t tick_start, int value_start,
						GameEngine::tick_t tick_end, int value_end)
	{
		mType = MotionInterval;
		mInterval.tick_start  = tick_start;
		mInterval.value_start = value_start;
		mInterval.tick_end    = tick_end;
		mInterval.value_end   = value_end;
	}

	void
	Motion::SetShiver(int origin, int delta)
	{
		mType = MotionShiver;
		mShiver.origin = origin;
		mShiver.delta = delta;
	}

}
