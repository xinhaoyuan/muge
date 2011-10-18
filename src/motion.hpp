#ifndef __MOTION_HPP__
#define __MOTION_HPP__

#include "logic/object.hpp"

namespace Game
{
	class Motion
	{
	private:

		enum
		{
			MotionInterval,
			MotionConstant,
			MotionShiver,
		} mType;
		
		union
		{
			int mConstant;
			
			struct
			{
				GameEngine::tick_t tick_start;
				GameEngine::tick_t tick_end;
				
				int value_start;
				int value_end;
			} mInterval;

			struct
			{
				int origin;
				int delta;
			} mShiver;
		};

	public:
		int Get(GameEngine::tick_t tick);

		void SetConstant(int c);
		void SetInterval(GameEngine::tick_t tick_start, int value_start,
						 GameEngine::tick_t tick_end, int value_end);
		void SetShiver(int origin, int delta);
	};
}

#endif
