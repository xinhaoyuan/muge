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
		};

	public:
		int Get(GameEngine::tick_t tick);

		void SetConstant(int c);
		void SetInterval(GameEngine::tick_t tick_start, int value_start,
						 GameEngine::tick_t tick_end, int value_end);
	};
}

#endif
